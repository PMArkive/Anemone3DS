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

#ifndef WRAPPERS_H
#define WRAPPERS_H

#include "common.h"
#include "loading.h"

extern Entry_List_s lists[LIST_AMOUNT];

extern bool installed_themes;
extern bool quit;

extern Entry_List_s * current_list;
extern EntryMode current_mode;
extern EntryMode current_list_mode;
extern int preview_offset;

extern Thread iconLoadingThread;
extern Thread_Arg_s iconLoadingThread_arg;
extern Handle update_icons_handle;

extern Thread installCheckThreads[LIST_AMOUNT];
extern Thread_Arg_s installCheckThreads_arg[LIST_AMOUNT];

extern int __stacksize__;
extern Result archive_result;
extern u32 old_time_limit;

// Startup and exit
void free_lists(void);
void load_lists(Entry_List_s * lists);

// Changing selected entry
void move_up(void * void_arg);
void move_down(void * void_arg);
void move_up_large(void * void_arg);
void move_down_large(void * void_arg);

void move_up_fast(void * void_arg);
void move_down_fast(void * void_arg);
void move_up_large_fast(void * void_arg);
void move_down_large_fast(void * void_arg);

// Changing modes
void change_to_theme_install_mode(void * void_arg);
void change_to_preview_mode(void * void_arg);
void change_to_qr_mode(void * void_arg);
void change_to_splashes_mode(void * void_arg);
void change_to_themes_mode(void * void_arg);

void exit_qr_mode(void * void_arg);
void exit_preview_mode(void * void_arg);
void exit_theme_install_mode(void * void_arg);

// Actions
void do_delete_from_sd(void * void_arg);

void start_pressed(void * void_arg);

void do_install_splash(void * void_arg);
void do_delete_installed_splash(void * void_arg);

void do_toggle_shuffle_theme(void * void_arg);

void do_install_theme(void * void_arg);
void do_install_theme_no_bgm(void * void_arg);
void do_install_theme_bgm_only(void * void_arg);
void do_install_shuffle_themes(void * void_arg);

// Touchscreen
void handle_touch(void * void_arg);

#endif