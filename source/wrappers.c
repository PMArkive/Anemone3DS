/*
*   This file is part of Anemone3DS
*   Copyright (C) 2016-2017 Alex Taber ("astronautlevel"), Dawid Eckert ("daedreth")
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
*       * Requiring preservation of specified reasonable legal notices or
*         author attributions in that material or in the Appropriate Legal
*         Notices displayed by works containing it.
*       * Prohibiting misrepresentation of the origin of that material,
*         or requiring that modified versions of such material be marked in
*         reasonable ways as different from the original version.
*/
#include "wrappers.h"
#include "loading.h"
#include "themes.h"
#include "splashes.h"
#include "camera.h"
#include "draw.h"

#define FASTSCROLL_WAIT 1e8


// Startup and exit
static void stop_install_check(void)
{
    for(int i = 0; i < LIST_AMOUNT; i++)
    {
        if(installCheckThreads_arg[i].run_thread)
        {
            installCheckThreads_arg[i].run_thread = false;
        }
    }
}

static void exit_thread(void)
{
    if(iconLoadingThread_arg.run_thread)
    {
        DEBUG("exiting thread\n");
        iconLoadingThread_arg.run_thread = false;
        svcSignalEvent(update_icons_handle);
        threadJoin(iconLoadingThread, U64_MAX);
        threadFree(iconLoadingThread);
    }
}

void free_lists(void)
{
    stop_install_check();
    for(int i = 0; i < LIST_AMOUNT; i++)
    {
        Entry_List_s * current_list = &lists[i];
        free(current_list->entries);
        current_list->entries = NULL;
    }
    exit_thread();
}

static void start_thread(void)
{
    if(iconLoadingThread_arg.run_thread)
    {
        DEBUG("starting thread\n");
        iconLoadingThread = threadCreate(load_icons_thread, &iconLoadingThread_arg, __stacksize__, 0x38, -2, false);
    }
}

void load_lists(Entry_List_s * lists)
{
    free_lists();

    ssize_t texture_id_offset = TEXTURE_ICON;

    for(int i = 0; i < LIST_AMOUNT; i++)
    {
        InstallType loading_screen = INSTALL_NONE;
        if(i == MODE_THEMES)
            loading_screen = INSTALL_LOADING_THEMES;
        else if(i == MODE_SPLASHES)
            loading_screen = INSTALL_LOADING_SPLASHES;

        draw_install(loading_screen);

        Entry_List_s * list = &lists[i];
        free(list->entries);
        memset(list, 0, sizeof(Entry_List_s));
        Result res = load_entries(main_paths[i], list, i);
        if(R_SUCCEEDED(res))
        {
            if(list->entries_count > ENTRIES_PER_SCREEN*ICONS_OFFSET_AMOUNT)
                iconLoadingThread_arg.run_thread = true;

            DEBUG("total: %i\n", list->entries_count);

            list->texture_id_offset = texture_id_offset;
            load_icons_first(list, false);

            texture_id_offset += ENTRIES_PER_SCREEN*ICONS_OFFSET_AMOUNT;

            void (*install_check_function)(void*) = NULL;
            if(i == MODE_THEMES)
                install_check_function = themes_check_installed;
            else if(i == MODE_SPLASHES)
                install_check_function = splash_check_installed;

            Thread_Arg_s * current_arg = &installCheckThreads_arg[i];
            current_arg->run_thread = true;
            current_arg->thread_arg = (void**)list;

            installCheckThreads[i] = threadCreate(install_check_function, current_arg, __stacksize__, 0x3f, -2, true);
            svcSleepThread(1e8);
        }
    }
    start_thread();
}
// ------------------------------------------


// Changing selected entry
static void change_selected(Entry_List_s * list, int change_value)
{
    if(abs(change_value) >= list->entries_count) return;

    list->selected_entry += change_value;
    if(list->selected_entry < 0)
        list->selected_entry += list->entries_count;
    list->selected_entry %= list->entries_count;
}

void move_up(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    change_selected(list, -1);
}
void move_down(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    change_selected(list, 1);
}
void move_up_large(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    change_selected(list, -ENTRIES_PER_SCREEN);
}
void move_down_large(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    change_selected(list, ENTRIES_PER_SCREEN);
}

void move_up_fast(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    change_selected(list, -1);
    svcSleepThread(FASTSCROLL_WAIT);
}
void move_down_fast(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    change_selected(list, 1);
    svcSleepThread(FASTSCROLL_WAIT);
}
void move_up_large_fast(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    change_selected(list, -ENTRIES_PER_SCREEN);
    svcSleepThread(FASTSCROLL_WAIT);
}
void move_down_large_fast(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    change_selected(list, ENTRIES_PER_SCREEN);
    svcSleepThread(FASTSCROLL_WAIT);
}

static SwkbdCallbackResult jump_menu_callback(void* entries_count, const char** ppMessage, const char* text, size_t textlen)
{
    int typed_value = atoi(text);
    if(typed_value > *(int*)entries_count)
    {
        *ppMessage = "The new position has to be\nsmaller or equal to the\nnumber of entries!";
        return SWKBD_CALLBACK_CONTINUE;
    }
    else if(typed_value == 0)
    {
        *ppMessage = "The new position has to\nbe positive!";
        return SWKBD_CALLBACK_CONTINUE;
    }
    return SWKBD_CALLBACK_OK;
}

static void jump_menu(Entry_List_s * list)
{
    if(list == NULL) return;

    char numbuf[64] = {0};

    SwkbdState swkbd;

    sprintf(numbuf, "%i", list->entries_count);
    int max_chars = strlen(numbuf);
    swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 2, max_chars);

    sprintf(numbuf, "%i", list->selected_entry);
    swkbdSetInitialText(&swkbd, numbuf);

    sprintf(numbuf, "Where do you want to jump to?\nMay cause icons to reload.");
    swkbdSetHintText(&swkbd, numbuf);

    swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Cancel", false);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "Jump", true);
    swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, max_chars);
    swkbdSetFilterCallback(&swkbd, jump_menu_callback, &list->entries_count);

    memset(numbuf, 0, sizeof(numbuf));
    SwkbdButton button = swkbdInputText(&swkbd, numbuf, sizeof(numbuf));
    if(button == SWKBD_BUTTON_CONFIRM)
    {
        list->selected_entry = atoi(numbuf) - 1;
        list->scroll = list->selected_entry;
        if(list->scroll >= list->entries_count - ENTRIES_PER_SCREEN)
            list->scroll = list->entries_count - ENTRIES_PER_SCREEN - 1;
    }
}
// ------------------------------------------

// Changing mode
void change_to_theme_install_mode(void * void_arg)
{
    current_mode = MODE_INSTALL_THEMES;
}
void change_to_preview_mode(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    if(load_preview(*list, &preview_offset))
    {
        current_mode = MODE_PREVIEW;;
    }

}
void change_to_qr_mode(void * void_arg)
{
    if(R_SUCCEEDED(camInit()))
    {
        camExit();
        u32 out;
        ACU_GetWifiStatus(&out);
        if(out)
        {
            if(init_qr(current_list_mode))
            {
                load_lists(lists);
            }
        }
        else
        {
            throw_error("Please connect to Wi-Fi before scanning QRs", ERROR_LEVEL_WARNING);
        }
    }
    else
    {
        throw_error("Your camera seems to have a problem, unable to scan QRs.", ERROR_LEVEL_WARNING);
    }
}
void change_to_splashes_mode(void * void_arg)
{
    current_list_mode = current_mode = MODE_SPLASHES;
}
void change_to_themes_mode(void * void_arg)
{
    current_list_mode = current_mode = MODE_THEMES;
}

void exit_preview_mode(void * void_arg)
{
    current_mode = current_list_mode;
}
void exit_theme_install_mode(void * void_arg)
{
    current_list_mode = current_mode = MODE_THEMES;
}
// ------------------------------------------

// Actions
void do_delete_from_sd(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    if(list != NULL && list->entries != NULL)
    {
        Entry_s entry = list->entries[list->selected_entry];
        draw_install(INSTALL_ENTRY_DELETE);
        delete_entry(entry);
        load_lists(lists);
    }
}

void start_pressed(void * void_arg)
{
    quit = true;
}

void do_install_splash(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    if(list != NULL && list->entries != NULL)
    {
        Entry_s splash = list->entries[list->selected_entry];
        draw_install(INSTALL_SPLASH);
        splash_install(splash);
        load_lists(lists);
    }
}
void do_delete_installed_splash(void * void_arg)
{
    draw_install(INSTALL_SPLASH_DELETE);
    splash_delete();
}

void do_toggle_shuffle_theme(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    if(list != NULL && list->entries != NULL)
    {
        Entry_s * current_entry = &list->entries[list->selected_entry];
        current_entry->in_shuffle = !current_entry->in_shuffle;
    }
}

void do_install_theme(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    if(list != NULL && list->entries != NULL)
    {
        Entry_s * theme = &list->entries[list->selected_entry];
        draw_install(INSTALL_SINGLE);
        if(R_SUCCEEDED(theme_install(*theme)))
        {
            for(int i = 0; i < list->entries_count; i++)
            {
                theme = &list->entries[i];
                if(i == list->selected_entry)
                    theme->installed = true;
                else
                    theme->installed = false;
            }
        }
    }
}
void do_install_theme_no_bgm(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    if(list != NULL && list->entries != NULL)
    {
        Entry_s * theme = &list->entries[list->selected_entry];
        draw_install(INSTALL_NO_BGM);
        if(R_SUCCEEDED(no_bgm_install(*theme)))
        {
            for(int i = 0; i < list->entries_count; i++)
            {
                theme = &list->entries[i];
                if(i == list->selected_entry)
                    theme->installed = true;
                else
                    theme->installed = false;
            }
        }
    }
}
void do_install_theme_bgm_only(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    if(list != NULL && list->entries != NULL)
    {
        Entry_s * theme = &list->entries[list->selected_entry];
        draw_install(INSTALL_BGM);
        if(R_SUCCEEDED(bgm_install(*theme)))
        {
            for(int i = 0; i < list->entries_count; i++)
            {
                theme = &list->entries[i];
                if(i == list->selected_entry)
                    theme->installed = true;
                else
                    theme->installed = false;
            }
        }
    }
}
void do_install_shuffle_themes(void * void_arg)
{
    Entry_List_s * list = *(Entry_List_s **)void_arg;
    if(list != NULL && list->entries != NULL)
    {
        draw_install(INSTALL_SHUFFLE);
        if(R_SUCCEEDED(shuffle_install(*list)))
        {

        }
    }
}
// ------------------------------------------

// Touchscreen
void handle_touch(void * void_arg)
{
    touchPosition touch = {0};
    hidTouchRead(&touch);

    u16 x = touch.px;
    u16 y = touch.py;

    u16 arrowStartX = 152;
    u16 arrowEndX = arrowStartX+16;

    #define BETWEEN(min, x, max) (min < x && x < max)

    if(y < 24)
    {
        if(BETWEEN(arrowStartX, x, arrowEndX) && current_list->scroll > 0)
        {
            move_down_large(&current_list);
        }
        else if(BETWEEN(320-24, x, 320))
        {
            switch(current_list_mode)
            {
                case MODE_THEMES:
                    change_to_splashes_mode(NULL);
                    break;
                case MODE_SPLASHES:
                    change_to_themes_mode(NULL);
                    break;
                default:
                    break;
            }
        }
        else if(BETWEEN(320-48, x, 320-24))
        {
            change_to_qr_mode(NULL);
        }
        else if(BETWEEN(320-72, x, 320-48))
        {
            change_to_preview_mode(&current_list);
        }
        else if(BETWEEN(320-96, x, 320-72))
        {
            load_icons_first(current_list, false);
        }
        else if(BETWEEN(320-120, x, 320-96) && current_mode == MODE_THEMES)
        {
            do_toggle_shuffle_theme(&current_list);
        }
    }
    else if(y >= 216)
    {
        if(BETWEEN(arrowStartX, x, arrowEndX) && current_list->scroll < current_list->entries_count - ENTRIES_PER_SCREEN)
        {
            move_up_large(&current_list);
        }
        else if(BETWEEN(176, x, 320))
        {
            jump_menu(current_list);
        }
    }

    if(BETWEEN(24, y, 216))
    {
        for(int i = 0; i < ENTRIES_PER_SCREEN; i++)
        {
            u16 miny = 24 + 48*i;
            u16 maxy = miny + 48;
            int new_selected = current_list->scroll + i;
            if(BETWEEN(miny, y, maxy) && new_selected < current_list->entries_count)
            {
                current_list->selected_entry = new_selected;
                break;
            }
        }
    }

    #undef BETWEEN
}
// ------------------------------------------
