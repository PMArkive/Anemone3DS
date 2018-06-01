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

#include "i18n.h"
#include "fs.h"
#include "draw.h"
#include "colors.h"

#include "instructions.h"

Instructions_s normal_instructions[MODE_AMOUNT] = {0};
Instructions_s install_instructions = {0};
Instructions_s extra_instructions[3] = {0};

Instructions_s remote_instructions[MODE_AMOUNT] = {0};
Instructions_s remote_extra_instructions = {0};

//TODO MAKE THIS WORK
#define BUFFER_LENGTH 128

// Reads in lines until not a comment, then returns silently
static inline void parse_next_line(FILE* file, char* buffer);

static void i18n_instructions_line(const char ** line, FILE * file, char * text_buf)
{
    parse_next_line(file, text_buf);
    text_buf[strlen(text_buf) - 1] = '\0'; // set \n to null char
    if(strlen(text_buf) != 0) // if the line is empty, leave it to NULL
    {
        *line = strdup(text_buf);
    }
}

static void i18n_instructions(Instructions_s * instructions, const char * filename)
{
    FILE* file = fopen(filename, "r");
    if(file == NULL)
    {
        DEBUG("%s missing\n", filename);
        return;
    }

    char text_buf[BUFFER_LENGTH];
    i18n_instructions_line(&instructions->info_line, file, text_buf);

    for(int i = 0; i < BUTTONS_INFO_LINES; i++)
    {
        i18n_instructions_line(&instructions->instructions[i][0], file, text_buf);
        i18n_instructions_line(&instructions->instructions[i][1], file, text_buf);
    }

    if(fclose(file) != 0)
        DEBUG("Error closing %s", filename);
}

static void i18n_load_instructions(const char * base_folder)
{
    char filename[BUFFER_LENGTH];

    // Main instructions
    sprintf(filename, "%s/instructions/main/themes.txt", base_folder);
    i18n_instructions(&normal_instructions[MODE_THEMES], filename);
    sprintf(filename, "%s/instructions/main/splashes.txt", base_folder);
    i18n_instructions(&normal_instructions[MODE_SPLASHES], filename);

    // Install instructions
    sprintf(filename, "%s/instructions/main/install.txt", base_folder);
    i18n_instructions(&install_instructions, filename);

    // Extra instructions
    sprintf(filename, "%s/instructions/main/extra.txt", base_folder);
    i18n_instructions(&extra_instructions[1], filename);
    sprintf(filename, "%s/instructions/main/extra_L.txt", base_folder);
    i18n_instructions(&extra_instructions[0], filename);
    sprintf(filename, "%s/instructions/main/extra_R.txt", base_folder);
    i18n_instructions(&extra_instructions[2], filename);

    // Remote instructions
    sprintf(filename, "%s/instructions/remote/themes.txt", base_folder);
    i18n_instructions(&normal_instructions[MODE_THEMES], filename);
    sprintf(filename, "%s/instructions/remote/splashes.txt", base_folder);
    i18n_instructions(&normal_instructions[MODE_SPLASHES], filename);
    sprintf(filename, "%s/instructions/remote/extra.txt", base_folder);
    i18n_instructions(&remote_extra_instructions, filename);
}

void i18n_strings(void);

static void i18n_load_text(const char * filename, int starti, int endi)
{
    FILE* file = fopen(filename, "r");
    if(file == NULL)
    {
        DEBUG("%s missing\n", filename);
        return;
    }

    char text_buf[BUFFER_LENGTH];
    // Static C2D_Text objects using the Text enum
    for(int i = starti; i != endi; i++)
    {
        parse_next_line(file, text_buf);

        text_buf[strlen(text_buf) - 1] = '\0'; // set \n to null char
        C2D_TextParse(&text[i], staticBuf, text_buf);
        C2D_TextOptimize(&text[i]);
    }

    if(fclose(file) != 0)
        DEBUG("Error closing %s", filename);
}

void i18n_set(u8 language)
{
    char base_folder[12 + 2 + 1] = "romfs:/lang/"; // 15 = length of "romfs:/lang/cc" + NULL

    switch(language)
    {
        case CFG_LANGUAGE_EN:
        default:
            strcat(base_folder, "en");
    }

    DEBUG("TEXT_AMOUNT: %i\n", TEXT_AMOUNT);
    char filename[12 + 2 + 1 + 12 + 1];
    sprintf(filename, "%s/%s", base_folder, "installs.txt");
    i18n_load_text(filename, TEXT_INSTALLS_START, TEXT_INSTALLS_END);

    sprintf(filename, "%s/%s", base_folder, "errors.txt");
    i18n_load_text(filename, TEXT_ERRORS_START, TEXT_ERRORS_END);

    sprintf(filename, "%s/%s", base_folder, "confirm.txt");
    i18n_load_text(filename, TEXT_CONFIRMS_START, TEXT_CONFIRMS_END);

    sprintf(filename, "%s/%s", base_folder, "text.txt");
    i18n_load_text(filename, TEXT_NORMAL_START, TEXT_NORMAL_END);

    i18n_load_instructions(base_folder);
}

static inline void parse_next_line(FILE* file, char* buffer)
{
    char* buf = calloc(BUFFER_LENGTH, sizeof(*buf));
    char* temp = buf; 

    while(*fgets(temp, BUFFER_LENGTH, file) == '|')
        memset(temp, 0, BUFFER_LENGTH);

    memset(buffer, 0, BUFFER_LENGTH);

    char ch[2];
    ch[1] = '\0';
    *ch = *temp;

    do {
        char* to_append = ch;

        if(*ch == '#')
        {
            char next = *(++temp);
            switch(next)
            {
                case 'n':
                    to_append = "\n";
                    break;
                case 'A':
                    to_append = "\uE000";
                    break;
                case 'B':
                    to_append = "\uE001";
                    break;
                case 'X':
                    to_append = "\uE002";
                    break;
                case 'Y':
                    to_append = "\uE003";
                    break;
                case 'L':
                    to_append = "\uE004";
                    break;
                case 'R':
                    to_append = "\uE005";
                    break;
                case 'S':
                    // five spaces
                    to_append = "     ";
                    break;
                case 'D':
                    next = *(++temp);
                    switch(next)
                    {
                        case 'u':
                            to_append = "\uE079";
                            break;
                        case 'd':
                            to_append = "\uE07A";
                            break;
                        case 'l':
                            to_append = "\uE07B";
                            break;
                        case 'r':
                            to_append = "\uE07C";
                            break;
                        case 'n':
                            to_append = "\uE006";
                            break;
                        default:
                            temp -= 2;
                    }
                    break;
                default:
                    temp -= 1;
            }
        }
        strcat(buffer, to_append);
    } while((ch[0] = *(++temp)));

    free(buf);
}
