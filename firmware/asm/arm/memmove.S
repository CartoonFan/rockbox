/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2006 Free Software Foundation, Inc.
 * This file was originally part of the GNU C Library
 * Contributed to glibc by MontaVista Software, Inc. (written by Nicolas Pitre)
 * Adapted for Rockbox by Daniel Ankers
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

#include "config.h"

/*
 * Endian independent macros for shifting bytes within registers.
 */
#ifndef __ARMEB__
#define pull            lsr
#define push            lsl
#else
#define pull            lsl
#define push            lsr
#endif

	.syntax unified

        .text

/*
 * Prototype: void *memmove(void *dest, const void *src, size_t n);
 *
 * Note:
 *
 * If the memory regions don't overlap, we simply branch to memcpy which is
 * normally a bit faster. Otherwise the copy is done going downwards.
 */

    .section    .icode,"ax",%progbits

    .align      2
    .global     memmove
    .type       memmove,%function

memmove:

        subs    ip, r0, r1
        cmphi   r2, ip
        bls memcpy

        stmfd   sp!, {r0, r4, lr}
        add r1, r1, r2
        add r0, r0, r2
        subs    r2, r2, #4
        blt 8f
        ands    ip, r0, #3
        bne 9f
        ands    ip, r1, #3
        bne 10f

1:      subs    r2, r2, #(28)
        stmfd   sp!, {r5 - r8}
        blt 5f

2:
3:
4:      ldmdb   r1!, {r3, r4, r5, r6, r7, r8, ip, lr}
        subs    r2, r2, #32
        stmdb   r0!, {r3, r4, r5, r6, r7, r8, ip, lr}
        bge 3b

5:      ands    ip, r2, #28
        rsb ip, ip, #32
        addne   pc, pc, ip      @ C is always clear here
        b   7f
6:      nop
        ldr r3, [r1, #-4]!
        ldr r4, [r1, #-4]!
        ldr r5, [r1, #-4]!
        ldr r6, [r1, #-4]!
        ldr r7, [r1, #-4]!
        ldr r8, [r1, #-4]!
        ldr lr, [r1, #-4]!

        add pc, pc, ip
        nop
        nop
        str r3, [r0, #-4]!
        str r4, [r0, #-4]!
        str r5, [r0, #-4]!
        str r6, [r0, #-4]!
        str r7, [r0, #-4]!
        str r8, [r0, #-4]!
        str lr, [r0, #-4]!

7:      ldmfd   sp!, {r5 - r8}

8:      movs    r2, r2, lsl #31
        ldrbne  r3, [r1, #-1]!
        ldrbcs  r4, [r1, #-1]!
        ldrbcs  ip, [r1, #-1]
        strbne  r3, [r0, #-1]!
        strbcs  r4, [r0, #-1]!
        strbcs  ip, [r0, #-1]
        ldmpc   regs="r0, r4"

9:      cmp ip, #2
        ldrbgt  r3, [r1, #-1]!
        ldrbge  r4, [r1, #-1]!
        ldrb    lr, [r1, #-1]!
        strbgt  r3, [r0, #-1]!
        strbge  r4, [r0, #-1]!
        subs    r2, r2, ip
        strb    lr, [r0, #-1]!
        blt 8b
        ands    ip, r1, #3
        beq 1b

10:     bic r1, r1, #3
        cmp ip, #2
        ldr r3, [r1, #0]
        beq 17f
        blt 18f


        .macro  backward_copy_shift push pull

        subs    r2, r2, #28
        blt 14f

11:     stmfd   sp!, {r5 - r9}

12:
13:     ldmdb   r1!, {r7, r8, r9, ip}
        mov     lr, r3, push #\push
        subs    r2, r2, #32
        ldmdb   r1!, {r3, r4, r5, r6}
        orr     lr, lr, ip, pull #\pull
        mov     ip, ip, push #\push
        orr     ip, ip, r9, pull #\pull
        mov     r9, r9, push #\push
        orr     r9, r9, r8, pull #\pull
        mov     r8, r8, push #\push
        orr     r8, r8, r7, pull #\pull
        mov     r7, r7, push #\push
        orr     r7, r7, r6, pull #\pull
        mov     r6, r6, push #\push
        orr     r6, r6, r5, pull #\pull
        mov     r5, r5, push #\push
        orr     r5, r5, r4, pull #\pull
        mov     r4, r4, push #\push
        orr     r4, r4, r3, pull #\pull
        stmdb   r0!, {r4 - r9, ip, lr}
        bge 12b

        ldmfd   sp!, {r5 - r9}

14:     ands    ip, r2, #28
        beq 16f

15:     mov     lr, r3, push #\push
        ldr r3, [r1, #-4]!
        subs    ip, ip, #4
        orr lr, lr, r3, pull #\pull
        str lr, [r0, #-4]!
        bgt 15b

16:     add r1, r1, #(\pull / 8)
        b   8b

        .endm


        backward_copy_shift push=8  pull=24

17:     backward_copy_shift push=16 pull=16

18:     backward_copy_shift push=24 pull=8


