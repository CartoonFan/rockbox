/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2005 Ray Lambert
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
#ifndef _ABREPEAT_H_
#define _ABREPEAT_H_
#include <stdbool.h>

#ifndef AB_REPEAT_ENABLE /* Dummy functions */
static inline bool ab_repeat_mode_enabled(void)
{
    return false;
}
static inline bool ab_bool_dummy_marker(unsigned int song_position)
{
    (void) song_position;
    return false;
}
static inline void ab_void_dummy_marker(unsigned int song_position)
{
    (void) song_position;
}
static inline void ab_dummy_voidfn(void){}

#define ab_before_A_marker     ab_bool_dummy_marker
#define ab_after_A_marker      ab_bool_dummy_marker
#define ab_jump_to_A_marker    ab_dummy_voidfn
#define ab_reset_markers       ab_dummy_voidfn
#define ab_set_A_marker        ab_void_dummy_marker
#define ab_set_B_marker        ab_void_dummy_marker
#define ab_get_A_marker        ab_bool_dummy_marker
#define ab_get_B_marker        ab_bool_dummy_marker
#define ab_end_of_track_report ab_dummy_voidfn
#define ab_reached_B_marker    ab_bool_dummy_marker
#define ab_position_report     ab_void_dummy_marker

#else /*def AB_REPEAT_ENABLE*/
#include "audio.h"
#include "kernel.h" /* needed for HZ */

#define AB_MARKER_NONE 0

#include "settings.h"

bool ab_before_A_marker(unsigned int song_position);
bool ab_after_A_marker(unsigned int song_position);
void ab_jump_to_A_marker(void);
void ab_reset_markers(void);
void ab_set_A_marker(unsigned int song_position);
void ab_set_B_marker(unsigned int song_position);
/* These return whether the marker are actually set.
 * The actual positions are returned via output parameter */
bool ab_get_A_marker(unsigned int *song_position);
bool ab_get_B_marker(unsigned int *song_position);
void ab_end_of_track_report(void);

/* These functions really need to be inlined for speed */
extern unsigned int ab_A_marker;
extern unsigned int ab_B_marker;

static inline bool ab_repeat_mode_enabled(void)
{
    return global_settings.repeat_mode == REPEAT_AB;
}

static inline bool ab_reached_B_marker(unsigned int song_position)
{
/* following is the size of the window in which we'll detect that the B marker
was hit; it must be larger than the frequency (in milliseconds) at which this 
function is called otherwise detection of the B marker will be unreliable */
/* On swcodec, the worst case seems to be 9600kHz with 1024 samples between
 * calls, meaning ~9 calls per second, look within 1/5 of a second */
#define B_MARKER_DETECT_WINDOW 200
    if (ab_B_marker != AB_MARKER_NONE)
    {
        if ( (song_position >= ab_B_marker) 
        && (song_position <= (ab_B_marker+B_MARKER_DETECT_WINDOW)) )
            return true;
    }
    return false;
}

static inline void ab_position_report(unsigned long position)
{
    if (ab_repeat_mode_enabled())
    {
        if ( !(audio_status() & AUDIO_STATUS_PAUSE) && 
                ab_reached_B_marker(position) )
        {
            ab_jump_to_A_marker();
        }
    }
}

#endif

#endif /* _ABREPEAT_H_ */
