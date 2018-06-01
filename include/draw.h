/*
*   This file is part of Anemone3DS
*   Copyright (C) 2016-2018 Contributors in CONTRIBUTORS.md
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

#ifndef DRAW_H
#define DRAW_H

#include "common.h"
#include "loading.h"
#include "colors.h"
#include "i18n.h"

#define MAX_LINES 10

typedef enum {
    INSTALL_LOADING_THEMES,
    INSTALL_LOADING_SPLASHES,
    INSTALL_LOADING_ICONS,

    INSTALL_SPLASH,
    INSTALL_SPLASH_DELETE,

    INSTALL_SINGLE,
    INSTALL_SHUFFLE,
    INSTALL_BGM,
    INSTALL_NO_BGM,

    INSTALL_DOWNLOAD,
    INSTALL_CHECKING_DOWNLOAD,
    INSTALL_ENTRY_DELETE,

    INSTALL_LOADING_REMOTE_THEMES,
    INSTALL_LOADING_REMOTE_SPLASHES,
    INSTALL_LOADING_REMOTE_PREVIEW,
    INSTALL_LOADING_REMOTE_BGM,

    INSTALL_NONE,
} InstallType;

typedef enum {
    ERROR_TYPE_NO_THEME_EXTDATA,

    ERROR_TYPE_NO_WIFI_QR,
    ERROR_TYPE_NO_QR_HBL,
    ERROR_TYPE_OTHER_QR_ERROR,

    ERROR_TYPE_DOWNLOADED_NOT_SPLASH_THEME,
    ERROR_TYPE_DOWNLOADED_NOT_ZIP,
    ERROR_TYPE_DOWNLOAD_FAILED,

    ERROR_TYPE_INVALID_PREVIEW_PNG,
    ERROR_TYPE_NO_PREVIEW,

    ERROR_TYPE_SHUFFLE_NOT_ENOUGH,
    ERROR_TYPE_SHUFFLE_TOO_MANY,

    ERROR_TYPE_THEMEPLAZA_COULDNT_LOAD,
    ERROR_TYPE_THEMEPLAZA_NO_RESULT,

    ERROR_TYPE_TARGET_NOT_VALID,

    ERROR_TYPE_SPLASH_WRONG,
    ERROR_TYPE_SPLASH_LUMA_DISABLED,

    ERROR_TYPE_THEME_NO_BODY,

    ERROR_TYPE_NONE,
} ErrorType;

typedef enum {
    CONFIRM_DELETE_INSTALLED_SPLASH,
    CONFIRM_DELETE_SELECTED_ENTRY,

    CONFIRM_NONE,
} ConfirmType;

typedef enum {
    // InstallType text
    TEXT_INSTALLS_START, // indicator

    TEXT_INSTALL_LOADING_THEMES = TEXT_INSTALLS_START,
    TEXT_INSTALL_LOADING_SPLASHES,
    TEXT_INSTALL_LOADING_ICONS,

    TEXT_INSTALL_SPLASH,
    TEXT_INSTALL_SPLASH_DELETE,

    TEXT_INSTALL_SINGLE,
    TEXT_INSTALL_SHUFFLE,
    TEXT_INSTALL_BGM,
    TEXT_INSTALL_NO_BGM,

    TEXT_INSTALL_DOWNLOAD,
    TEXT_INSTALL_CHECKING_DOWNLOAD,
    TEXT_INSTALL_ENTRY_DELETE,

    TEXT_INSTALL_LOADING_REMOTE_THEMES,
    TEXT_INSTALL_LOADING_REMOTE_SPLASHES,
    TEXT_INSTALL_LOADING_REMOTE_PREVIEW,
    TEXT_INSTALL_LOADING_REMOTE_BGM,

    TEXT_INSTALLS_END, // indicator

    // ErrorType text
    TEXT_ERRORS_START = TEXT_INSTALLS_END, // indicator

    TEXT_ERROR_TYPE_NO_THEME_EXTDATA = TEXT_ERRORS_START,

    TEXT_ERROR_TYPE_NO_WIFI_QR,
    TEXT_ERROR_TYPE_NO_QR_HBL,
    TEXT_ERROR_TYPE_OTHER_QR_ERROR,

    TEXT_ERROR_TYPE_DOWNLOADED_NOT_SPLASH_THEME,
    TEXT_ERROR_TYPE_DOWNLOADED_NOT_ZIP,
    TEXT_ERROR_TYPE_DOWNLOAD_FAILED,

    TEXT_ERROR_TYPE_INVALID_PREVIEW_PNG,
    TEXT_ERROR_TYPE_NO_PREVIEW,

    TEXT_ERROR_TYPE_SHUFFLE_NOT_ENOUGH,
    TEXT_ERROR_TYPE_SHUFFLE_TOO_MANY,

    TEXT_ERROR_TYPE_THEMEPLAZA_COULDNT_LOAD,
    TEXT_ERROR_TYPE_THEMEPLAZA_NO_RESULT,

    TEXT_ERROR_TYPE_TARGET_NOT_VALID,

    TEXT_ERROR_TYPE_SPLASH_WRONG,
    TEXT_ERROR_TYPE_SPLASH_LUMA_DISABLED,

    TEXT_ERROR_TYPE_THEME_NO_BODY,

    TEXT_ERRORS_END, // indicator

    // ConfirmType text
    TEXT_CONFIRMS_START = TEXT_ERRORS_END,

    TEXT_CONFIRM_DELETE_INSTALLED_SPLASH = TEXT_CONFIRMS_START,
    TEXT_CONFIRM_DELETE_SELECTED_ENTRY,

    TEXT_CONFIRMS_END,

    // Other text
    TEXT_NORMAL_START = TEXT_CONFIRMS_END, // indicator

    TEXT_THEME_MODE = TEXT_NORMAL_START,
    TEXT_SPLASH_MODE,

    TEXT_NO_THEME_FOUND,
    TEXT_NO_SPLASH_FOUND,

    TEXT_DOWNLOAD_FROM_QR,

    TEXT_SWITCH_TO_SPLASHES,
    TEXT_SWITCH_TO_THEMES,

    TEXT_OR_START_TO_QUIT,

    TEXT_BY_AUTHOR,
    TEXT_SELECTED,
    TEXT_SELECTED_SHORT,

    TEXT_THEMEPLAZA_THEME_MODE,
    TEXT_THEMEPLAZA_SPLASH_MODE,

    TEXT_SEARCH,
    TEXT_PAGE,

    TEXT_ERROR_QUIT,
    TEXT_ERROR_CONTINUE,

    TEXT_CONFIRM_YES_NO,

    TEXT_NORMAL_END, // indicator

    TEXT_VERSION = TEXT_NORMAL_END,

    TEXT_AMOUNT
} Text;

typedef enum {
    ERROR_LEVEL_ERROR,
    ERROR_LEVEL_WARNING,
} ErrorLevel;

#define BUTTONS_START_Y 130
#define BUTTONS_STEP 22
#define BUTTONS_INFO_LINES 4
#define BUTTONS_INFO_COLUNMNS 2

enum {
    BUTTONS_Y_INFO = BUTTONS_START_Y+5,

    BUTTONS_Y_LINE_1 = BUTTONS_START_Y + BUTTONS_STEP*1,
    BUTTONS_Y_LINE_2 = BUTTONS_START_Y + BUTTONS_STEP*2,
    BUTTONS_Y_LINE_3 = BUTTONS_START_Y + BUTTONS_STEP*3,
    BUTTONS_Y_LINE_4 = BUTTONS_START_Y + BUTTONS_STEP*4,

    BUTTONS_X_LEFT = 20,
    BUTTONS_X_RIGHT = 200,
    BUTTONS_X_MAX = 380,
} ButtonPos;

typedef struct {
    const char * info_line;
    const char * instructions[BUTTONS_INFO_LINES][BUTTONS_INFO_COLUNMNS];
} Instructions_s;

extern C3D_RenderTarget* top;
extern C3D_RenderTarget* bottom;
extern C2D_TextBuf staticBuf, dynamicBuf;

extern C2D_Text text[TEXT_AMOUNT];

void init_screens(void);
void exit_screens(void);

void start_frame(void);
void end_frame(void);
void set_screen(C3D_RenderTarget * screen);

void draw_preview(C2D_Image preview, int preview_offset);

void draw_install(InstallType type);
void throw_error(ErrorType error, ErrorLevel level);
bool draw_confirm(ConfirmType type, Entry_List_s* list);
void draw_loading_bar(u32 current, u32 max, InstallType type);

void draw_text(float x, float y, float z, float scaleX, float scaleY, Color color, const char * text);
void draw_text_wrap(float x, float y, float z, float scaleX, float scaleY, Color color, const char * text, float max_width);
void draw_text_wrap_scaled(float x, float y, float z, Color color, const char * text, float max_scale, float min_scale, float max_width);
void draw_text_center(gfxScreen_t target, float y, float z, float scaleX, float scaleY, Color color, const char * text);

void draw_base_interface(void);
void draw_grid_interface(Entry_List_s* list, Instructions_s instructions);
void draw_interface(Entry_List_s* list, Instructions_s instructions);

#endif