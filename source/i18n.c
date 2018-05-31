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
//TODO MAKE THIS WORK
#define BUFFER_LENGTH 128

void i18n_instructions(void);
void i18n_strings(void);

// Reads in lines until not a comment, then returns silently
inline void parse_next_line(FILE* file, char* buffer);

void i18n_set(u8 language)
{
    FILE* file;
    char text_buf[BUFFER_LENGTH];
    char filename[19] = "romfs:/lang/"; // 19 = length of "romfs:/lang/cc.txt" + NULL

    switch(language)
    {
        case CFG_LANGUAGE_EN:
        default:
            strcat(filename, "en.txt");
    }

    file = fopen(filename, "r");
    if(file == NULL)
    {
        DEBUG("%s missing\n", filename);
        return;
    }

    // Static C2D_Text objects using the Text enum
    for(int i = 0; i < TEXT_AMOUNT; i++)
    {
        if(i == TEXT_VERSION)
            continue;

        parse_next_line(file, text_buf);

        text_buf[strlen(text_buf) - 1] = '\0'; // set \n to null char
        C2D_TextParse(&text[i], staticBuf, text_buf);
        C2D_TextOptimize(&text[i]);
    }

    if(fclose(file) != 0)
        DEBUG("Error closing %s", filename);
}

inline void parse_next_line(FILE* file, char* buffer)
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
