/***************************************************************************
*             __________               __   ___.
*   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
*   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
*   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
*   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
*                     \/            \/     \/    \/            \/
* $Id$
*
* JPEG image viewer
* (This is a real mess if it has to be coded in one single C file)
*
* File scrolling addition (C) 2005 Alexander Spyridakis
* Copyright (C) 2004 Jörg Hohensohn aka [IDC]Dragon
* Heavily borrowed from the IJG implementation (C) Thomas G. Lane
* Small & fast downscaling IDCT (C) 2002 by Guido Vollbeding  JPEGclub.org
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

#include "resize.h"
#include "bmp.h"
#include "jpeg_common.h"

#ifndef _JPEG_LOAD_H
#define _JPEG_LOAD_H

/* Approximate memory overhead required for JPEG decoding. This memory is
 * taken from the bitmap buffer so you must ensure the buffer is big enough
 * to contain all decoded pixel data plus decoder overhead, otherwise the
 * image cannot be loaded. After the image is loaded this extra memory can
 * be freed. */
extern const size_t JPEG_DECODE_OVERHEAD;

int read_jpeg_file(const char* filename,
                   struct bitmap *bm,
                   int maxsize,
                   int format,
                   const struct custom_format *cformat);

int read_jpeg_fd(int fd,
                 struct bitmap *bm,
                 int maxsize,
                 int format,
                 const struct custom_format *cformat);

/**
 * read embedded jpeg files as above. Needs an offset and length into
 * the file
 **/
int clip_jpeg_file(const char* filename,
                   int offset,
                   unsigned long jpeg_size,
                   struct bitmap *bm,
                   int maxsize,
                   int format,
                   const struct custom_format *cformat);

/**
 * read embedded jpeg files as above. Needs an open file descripter, and
 * assumes the caller has lseek()'d to the start of the jpeg blob
 **/
int clip_jpeg_fd(int fd, int flags,
                 unsigned long jpeg_size,
                 struct bitmap *bm,
                 int maxsize,
                 int format,
                 const struct custom_format *cformat);

#endif /* _JPEG_JPEG_DECODER_H */
