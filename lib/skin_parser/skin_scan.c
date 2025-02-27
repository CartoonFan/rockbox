/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2010 Robert Bieber
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "skin_scan.h"
#include "skin_debug.h"
#include "symbols.h"
#include "skin_parser.h"
#include "tag_table.h"

/* Scanning Functions */

/* Simple function to advance a char* past a comment */
void skip_comment(const char** document)
{
    while(**document != '\n' && **document != '\0')
        (*document)++;
    if(**document == '\n')
        (*document)++;
}

static void skip_arglist(const char** document)
{
    if(**document == ARGLISTOPENSYM)
        (*document)++;
    while(**document && **document != ARGLISTCLOSESYM)
    {
        if(**document == TAGSYM)
            skip_tag(document);
        else if(**document == COMMENTSYM)
            skip_comment(document);
        else
            (*document)++;
    }
    if(**document == ARGLISTCLOSESYM)
        (*document)++;
}

static void skip_enumlist(const char** document)
{
    if(**document == ENUMLISTOPENSYM)
        (*document)++;
    while(**document && **document != ENUMLISTCLOSESYM)
    {
        if(**document == TAGSYM)
            skip_tag(document);
        else if(**document == COMMENTSYM)
            skip_comment(document);
        else
            (*document)++;
    }

    if(**document == ENUMLISTCLOSESYM)
        (*document)++;
}

void skip_tag(const char** document)
{
    bool qmark;
    const struct tag_info *tag;

    if(**document == TAGSYM)
        (*document)++;
    qmark = (**document == CONDITIONSYM);
    if (qmark)
        (*document)++;

    if (!qmark && find_escape_character(**document))
    {
        (*document)++;
    }
    else
    {
        /* Checking the tag name */
        tag = find_tag(*document);
        if (tag)
        {
            *document += tag->param_pos - 1; /*strlen(tag->name)*/
        }
    }
    if (**document == ARGLISTOPENSYM)
        skip_arglist(document);

    if (**document == ENUMLISTOPENSYM)
        skip_enumlist(document);
}

char* scan_string(const char** document)
{

    const char* cursor = *document;
    int length = 0;
    char* buffer = NULL;
    int i;

    while(*cursor != ARGLISTSEPARATESYM && *cursor != ARGLISTCLOSESYM &&
          *cursor != '\0')
    {
        if(*cursor == COMMENTSYM)
        {
            skip_comment(&cursor);
            continue;
        }

        if(*cursor == TAGSYM)
            cursor++;

        if(*cursor == '\n')
        {
            skin_error(UNEXPECTED_NEWLINE, cursor);
            return NULL;
        }

        length++;
        cursor++;
    }

    /* Copying the string */
    cursor = *document;
    buffer = skin_alloc_string(length);
    if (!buffer)
        return NULL;
    buffer[length] = '\0';
    for(i = 0; i < length; i++)
    {
        if(*cursor == TAGSYM)
            cursor++;

        if(*cursor == COMMENTSYM)
        {
            skip_comment(&cursor);
            i--;
            continue;
        }

        buffer[i] = *cursor;
        cursor++;
    }

    *document = cursor;
    return buffer;
}

int scan_int(const char** document)
{

    const char *cursor = *document;
    int retval = 0;
    int sign = 1;

    while(true)
    {
        if(*cursor == COMMENTSYM)
        {
            skip_comment(&cursor);
            if (retval > 0) /* || sign < 0 already read a number */
            {
                break;
            }
            continue;
        }
        else if (*cursor == '-')
        {
            if (retval != 0)  /* only allow negative prior to numbers */
                break;
            sign = -1;
        }
        else if (isdigit(*cursor)) /* is digit*/
        {
            retval = (retval * 10) + (*cursor - '0');
        }
        else
            break;
        cursor++;
    }
    *document = cursor;

    return sign * retval;
}

int check_viewport(const char* document)
{
    if(document[0] != TAGSYM)
        return 0;

    if(document[1] != 'V')
        return 0;

    if(document[2] != ARGLISTOPENSYM
       && document[2] != 'l'
       && document[2] != 'i')
        return 0;

    if (document[3] == '\0')
        return 0;

    return 1;
}
