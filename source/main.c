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

#include "common.h"

#include "fs.h"
#include "draw.h"

#include "controls.h"
#include "wrappers.h"

#include "pp2d/pp2d/pp2d.h"
#include <time.h>

Entry_List_s lists[LIST_AMOUNT] = {0};

bool homebrew = false;
bool installed_themes = false;
bool quit = false;

Entry_List_s * current_list = NULL;
EntryMode current_mode = MODE_THEMES;
EntryMode current_list_mode = MODE_THEMES;
int preview_offset = 0;

Thread iconLoadingThread = {0};
Thread_Arg_s iconLoadingThread_arg = {0};
Handle update_icons_handle;

Thread installCheckThreads[LIST_AMOUNT] = {0};
Thread_Arg_s installCheckThreads_arg[LIST_AMOUNT] = {0};

int __stacksize__ = 64 * 1024;
Result archive_result;
u32 old_time_limit;

const char * main_paths[LIST_AMOUNT] = {
    "/Themes/",
    "/Splashes/",
};

static void init_services(void)
{
    consoleDebugInit(debugDevice_SVC);
    cfguInit();
    ptmuInit();
    acInit();
    APT_GetAppCpuTimeLimit(&old_time_limit);
    APT_SetAppCpuTimeLimit(30);
    httpcInit(0);
    archive_result = open_archives();
    if(envIsHomebrew())
    {
        s64 out;
        svcGetSystemInfo(&out, 0x10000, 0);
        homebrew = !out;
    }
}

void init_function(void)
{
    init_services();
    init_screens();
    svcCreateEvent(&update_icons_handle, RESET_ONESHOT);
}

static void exit_services(void)
{
    close_archives();
    cfguExit();
    ptmuExit();
    if (old_time_limit != UINT32_MAX) APT_SetAppCpuTimeLimit(old_time_limit);
    httpcExit();
    acExit();
}

void exit_function(bool power_pressed)
{
    free_lists();
    svcCloseHandle(update_icons_handle);
    exit_screens();
    exit_services();

    if(!power_pressed && installed_themes)
    {
        if(homebrew)
        {
            APT_HardwareResetAsync();
        }
        else
        {
            srvPublishToSubscriber(0x202, 0);
        }
    }

    quit = true;
}

int main(void)
{
    srand(time(NULL));
    init_function();

    void * iconLoadingThread_args_void[] = {
        &current_list,
        &update_icons_handle,
    };
    iconLoadingThread_arg.thread_arg = iconLoadingThread_args_void;
    iconLoadingThread_arg.run_thread = false;

    load_lists(lists);

    u32 kDown = 0;
    u32 kHeld = 0;
    u32 kUp = 0;

    Button_s base_modes_buttons[MODE_AMOUNT][BUTTONS_AMOUNT] = {
        { // theme mode
            {
                L"\uE000 Hold to install",
                change_to_theme_install_mode,
                NULL,
                BUTTONS_X_LEFT,
                BUTTONS_Y_LINE_1,
                KEY_A,
                &kDown,
            },
            {
                L"\uE001 Queue shuffle theme",
                do_toggle_shuffle_theme,
                &current_list,
                BUTTONS_X_RIGHT,
                BUTTONS_Y_LINE_1,
                KEY_B,
                &kDown,
            },
            {
                NULL,
                NULL,
                NULL,
                BUTTONS_X_LEFT,
                BUTTONS_Y_LINE_2,
                KEY_X,
                &kDown,
            },
            {
                L"\uE003 Preview theme",
                change_to_preview_mode,
                &current_list,
                BUTTONS_X_RIGHT,
                BUTTONS_Y_LINE_2,
                KEY_Y,
                &kDown,
            },

            {
                L"\uE004 Switch to splashes",
                change_to_splashes_mode,
                NULL,
                BUTTONS_X_LEFT,
                BUTTONS_Y_LINE_3,
                KEY_L,
                &kDown,
            },
            {
                L"\uE005 Scan QR code",
                change_to_qr_mode,
                NULL,
                BUTTONS_X_RIGHT,
                BUTTONS_Y_LINE_3,
                KEY_R,
                &kDown,
            },

            {
                L"Exit",
                start_pressed,
                NULL,
                BUTTONS_X_LEFT,
                BUTTONS_Y_LINE_4,
                KEY_START,
                &kDown,
            },
            {
                L"Delete from SD",
                do_delete_from_sd,
                &current_list,
                BUTTONS_X_RIGHT,
                BUTTONS_Y_LINE_4,
                KEY_SELECT,
                &kDown,
            },

            {
                NULL,
                move_up,
                &current_list,
                0,
                0,
                KEY_DUP,
                &kDown,
            },
            {
                NULL,
                move_down,
                &current_list,
                0,
                0,
                KEY_DDOWN,
                &kDown,
            },
            {
                NULL,
                move_up_large,
                &current_list,
                0,
                0,
                KEY_DLEFT,
                &kDown,
            },
            {
                NULL,
                move_down_large,
                &current_list,
                0,
                0,
                KEY_DRIGHT,
                &kDown,
            },

            {
                NULL,
                move_up_fast,
                &current_list,
                0,
                0,
                KEY_CPAD_UP,
                &kHeld,
            },
            {
                NULL,
                move_down_fast,
                &current_list,
                0,
                0,
                KEY_CPAD_DOWN,
                &kHeld,
            },
            {
                NULL,
                move_up_large_fast,
                &current_list,
                0,
                0,
                KEY_CPAD_LEFT,
                &kHeld,
            },
            {
                NULL,
                move_down_large_fast,
                &current_list,
                0,
                0,
                KEY_CPAD_RIGHT,
                &kHeld,
            },

            {
                NULL,
                handle_touch,
                NULL,
                0,
                0,
                KEY_TOUCH,
                &kDown,
            },
        },
        { // splashes mode
            {
                L"\uE000 Install splash",
                do_install_splash,
                &current_list,
                BUTTONS_X_LEFT,
                BUTTONS_Y_LINE_1,
                KEY_A,
                &kDown,
            },
            {
                L"\uE001 Delete installed splash",
                do_delete_installed_splash,
                NULL,
                BUTTONS_X_RIGHT,
                BUTTONS_Y_LINE_1,
                KEY_B,
                &kDown,
            },
            {
                NULL,
                NULL,
                NULL,
                BUTTONS_X_LEFT,
                BUTTONS_Y_LINE_2,
                KEY_X,
                &kDown,
            },
            {
                L"\uE003 Preview splash",
                change_to_preview_mode,
                &current_list,
                BUTTONS_X_RIGHT,
                BUTTONS_Y_LINE_2,
                KEY_Y,
                &kDown,
            },

            {
                L"\uE004 Switch to themes",
                change_to_themes_mode,
                NULL,
                BUTTONS_X_LEFT,
                BUTTONS_Y_LINE_3,
                KEY_L,
                &kDown,
            },
            {
                L"\uE005 Scan QR code",
                change_to_qr_mode,
                NULL,
                BUTTONS_X_RIGHT,
                BUTTONS_Y_LINE_3,
                KEY_R,
                &kDown,
            },

            {
                L"Exit",
                start_pressed,
                NULL,
                BUTTONS_X_LEFT,
                BUTTONS_Y_LINE_4,
                KEY_START,
                &kDown,
            },
            {
                L"Delete from SD",
                do_delete_from_sd,
                &current_list,
                BUTTONS_X_RIGHT,
                BUTTONS_Y_LINE_4,
                KEY_SELECT,
                &kDown,
            },

            {
                NULL,
                move_up,
                &current_list,
                0,
                0,
                KEY_DUP,
                &kDown,
            },
            {
                NULL,
                move_down,
                &current_list,
                0,
                0,
                KEY_DDOWN,
                &kDown,
            },
            {
                NULL,
                move_up_large,
                &current_list,
                0,
                0,
                KEY_DLEFT,
                &kDown,
            },
            {
                NULL,
                move_down_large,
                &current_list,
                0,
                0,
                KEY_DRIGHT,
                &kDown,
            },

            {
                NULL,
                move_up_fast,
                &current_list,
                0,
                0,
                KEY_CPAD_UP,
                &kHeld,
            },
            {
                NULL,
                move_down_fast,
                &current_list,
                0,
                0,
                KEY_CPAD_DOWN,
                &kHeld,
            },
            {
                NULL,
                move_up_large_fast,
                &current_list,
                0,
                0,
                KEY_CPAD_LEFT,
                &kHeld,
            },
            {
                NULL,
                move_down_large_fast,
                &current_list,
                0,
                0,
                KEY_CPAD_RIGHT,
                &kHeld,
            },

            {
                NULL,
                handle_touch,
                NULL,
                0,
                0,
                KEY_TOUCH,
                &kDown,
            },
        },
        { // theme install mode
            {L"\uE000 Go back", exit_theme_install_mode, &current_list, BUTTONS_X_LEFT, BUTTONS_Y_LINE_1, KEY_A, &kDown},
            {L"\uE001 Go back", exit_theme_install_mode, &current_list, BUTTONS_X_RIGHT, BUTTONS_Y_LINE_1, KEY_B, &kDown},

            {L"\uE079 Normal install", do_install_theme, &current_list, BUTTONS_X_LEFT, BUTTONS_Y_LINE_2, KEY_UP, &kDown},
            {L"\uE07A Shuffle install", do_install_shuffle_themes, &current_list, BUTTONS_X_RIGHT, BUTTONS_Y_LINE_2, KEY_DOWN, &kDown},
            {L"\uE07B BGM-only install", do_install_theme_bgm_only, &current_list, BUTTONS_X_LEFT, BUTTONS_Y_LINE_3, KEY_LEFT, &kDown},
            {L"\uE07C No-BGM install", do_install_theme_no_bgm, &current_list, BUTTONS_X_RIGHT, BUTTONS_Y_LINE_3, KEY_RIGHT, &kDown},
        },
        { // preview mode
            {NULL, exit_preview_mode, NULL, 0, 0, KEY_A, &kDown},
            {NULL, exit_preview_mode, NULL, 0, 0, KEY_B, &kDown},
            {NULL, exit_preview_mode, NULL, 0, 0, KEY_Y, &kDown},

            {NULL, exit_preview_mode, NULL, 0, 0, KEY_START, &kDown},

            {NULL, exit_preview_mode, NULL, 0, 0, KEY_TOUCH, &kDown},
        },
    };
    Button_s * current_mode_controls = NULL;

    while(aptMainLoop())
    {
        if(quit)
        {
            exit_function(false);
            return 0;
        }

        #ifndef CITRA_MODE
        if(R_FAILED(archive_result) && current_list_mode == MODE_THEMES)
        {
            throw_error("Theme extdata does not exist!\nSet a default theme from the home menu.", ERROR_LEVEL_ERROR);
            quit = true;
        }
        #endif

        hidScanInput();
        kDown = hidKeysDown();
        kHeld = hidKeysHeld();
        kUp = hidKeysUp();

        current_list = &lists[current_list_mode];
        current_mode_controls = base_modes_buttons[current_mode];

        switch(current_mode)
        {
            case MODE_PREVIEW:
                draw_preview(preview_offset);
                break;
            default:
                if(!iconLoadingThread_arg.run_thread)
                {
                    handle_scrolling(current_list);
                    current_list->previous_scroll = current_list->scroll;
                }
                else
                {
                    svcSignalEvent(update_icons_handle);
                    svcSleepThread(1e6);
                }
                draw_interface(current_list, current_mode_controls);
                svcSleepThread(1e7);
                break;
        }
        pp2d_end_draw();

        for(int i = 0; i < BUTTONS_AMOUNT; i++)
        {
            Button_s current_button = current_mode_controls[i];

            u32 button_key = current_button.key;
            u32 * button_checked = current_button.pressed_check;

            if(button_checked != NULL && ((*button_checked & button_key) == button_key))
            {
                void (*button_function)(void*) = current_button.function;
                if(button_function != NULL)
                {
                    void * button_arg = current_button.arg;
                    button_function(button_arg);
                    break;
                }
            }
        }
    }

    exit_function(true);

    return 0;
}
