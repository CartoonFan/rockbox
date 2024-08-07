/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2005 Dave Chapman
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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>

#include "metadata.h"
#include "metadata_common.h"
#include "metadata_parsers.h"

#define APETAG_HEADER_LENGTH        32
#define APETAG_ITEM_TYPE_MASK       3

struct apetag_header 
{
    char id[8];
    uint32_t version;
    uint32_t length;
    uint32_t item_count;
    uint32_t flags;
    char reserved[8];
};

struct apetag_item_header 
{
    int32_t length;
    uint32_t flags;
};

/* Read the items in an APEV2 tag. Only looks for a tag at the end of a 
 * file. Returns true if a tag was found and fully read, false otherwise.
 */
bool read_ape_tags(int fd, struct mp3entry* id3)
{
    struct apetag_header header;

    if (lseek(fd, -APETAG_HEADER_LENGTH, SEEK_END) < 0)
        return false;

    if (read(fd, &header, sizeof(header)) != APETAG_HEADER_LENGTH)
        return false;

    if (memcmp(header.id, "APETAGEX", sizeof(header.id)))
        return false;

    /* APE tag is little endian - convert to native byte order if needed */
    if (IS_BIG_ENDIAN)
    {
        header.version = swap32(header.version);
        header.length = swap32(header.version);
        header.item_count = swap32(header.version);
        header.flags = swap32(header.version);
    }

    if ((header.version == 2000) && (header.item_count > 0)
        && (header.length > APETAG_HEADER_LENGTH)) 
    {
        char *buf = id3->id3v2buf;
        unsigned int buf_remaining = sizeof(id3->id3v2buf) 
            + sizeof(id3->id3v1buf);
        unsigned int tag_remaining = header.length - APETAG_HEADER_LENGTH;
        unsigned int i;

        if (lseek(fd, -((int)header.length), SEEK_END) < 0)
        {
            return false;
        }

        for (i = 0; i < header.item_count; i++)
        {
            struct apetag_item_header item;
            char name[TAG_NAME_LENGTH];
            char value[TAG_VALUE_LENGTH];
            long r;

            if (tag_remaining < sizeof(item))
            {
                break;
            }

            if (read(fd, &item, sizeof(item)) != sizeof(item))
                return false;

            if (IS_BIG_ENDIAN)
            {
                item.length = swap32(item.length);
                item.flags = swap32(item.flags);
            }

            tag_remaining -= sizeof(item);
            r = read_string(fd, name, sizeof(name), 0, tag_remaining);
            
            if (r == -1)
            {
                return false;
            }

            tag_remaining -= r + item.length;

            if ((item.flags & APETAG_ITEM_TYPE_MASK) == 0) 
            {
                long len;

                if (read_string(fd, value, sizeof(value), -1, item.length) 
                    != item.length)
                {
                    return false;
                }

                if (strcasecmp(name, "cuesheet") == 0)
                {
                    id3->has_embedded_cuesheet = true;
                    id3->embedded_cuesheet.pos = lseek(fd, 0, SEEK_CUR)-item.length;
                    id3->embedded_cuesheet.size = item.length;
                    id3->embedded_cuesheet.encoding = CHAR_ENC_UTF_8;
                }
                else
                {
                    len = parse_tag(name, value, id3, buf, buf_remaining,
                    TAGTYPE_APE);
                    buf += len;
                    buf_remaining -= len;
                }
            }
            else
            {
#ifdef HAVE_ALBUMART
                if (strcasecmp(name, "cover art (front)") == 0)
                {
                    /* Skip any file name. */
                    r = read_string(fd, value, sizeof(value), 0, -1);
                    r += read_string(fd, value, sizeof(value), -1, 4);

                    if (r == -1)
                    {
                        return false;
                    }

                    /* Gather the album art format from the magic number of the embedded binary. */
                    id3->albumart.type = AA_TYPE_UNKNOWN;
                    if (memcmp(value, "\xFF\xD8\xFF", 3) == 0)
                    {
                        id3->albumart.type = AA_TYPE_JPG;
                    }
                    else if (memcmp(value, "\x42\x4D", 2) == 0)
                    {
                        id3->albumart.type = AA_TYPE_BMP;
                    }
                    else if (memcmp(value, "\x89\x50\x4E\x47", 4) == 0)
                    {
                        id3->albumart.type = AA_TYPE_PNG;
                    }

                    /* Set the album art size and position. */
                    if (id3->albumart.type != AA_TYPE_UNKNOWN)
                    {
                        id3->albumart.pos  = lseek(fd, - 4, SEEK_CUR);
                        id3->albumart.size = item.length - r;
                        id3->has_embedded_albumart = true;
                    }
                    
                    /* Seek back to this APE items begin. */
                    if (lseek(fd, -r, SEEK_CUR) < 0)
                    {
                        return false;
                    }
                }
#endif
                /* Seek to the next APE item. */
                if (lseek(fd, item.length, SEEK_CUR) < 0)
                {
                    return false;
                }
            }
        }
    }

    return true;
}
