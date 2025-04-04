/* Return the offset of one string within another.
   Copyright (C) 1994,1996,1997,1998,1999,2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/*
 * My personal strstr() implementation that beats most other algorithms.
 * Until someone tells me otherwise, I assume that this is the
 * fastest implementation of strstr() in C.
 * I deliberately chose not to comment it.  You should have at least
 * as much fun trying to understand it, as I had to write it :-).
 *
 * Stephen R. van den Berg, berg@pool.informatik.rwth-aachen.de*/

/* Faster looping by precalculating bl, bu, cl, cu before looping.
 * 2004 Apr 08 Jose Da Silva, digital@joescat@com */

#include <string.h>
#include <ctype.h>

typedef unsigned chartype;
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
char* strcasestr (const char* haystack, const char* needle)
{
    const char* match, *needle_search;
    chartype needle_ch, ch_h;
    chartype first_ch, first_upch;
    if (*needle == 0) /* if needle empty return haystack */
        return (char*)haystack;

    first_ch = tolower(*needle++); /*store first & step to 2nd needle character */
    first_upch = toupper(first_ch);

    while(1)
    {
        /* find the first matching character */
        do
        {
            ch_h = *haystack++;
            if (ch_h == 0) /* end of haystack no match.. */
                return NULL;
        } while (ch_h != first_ch && ch_h != first_upch);
        match = haystack; /* keep progress thru haystack */
        needle_search = needle; /* reset needle to second character */
        goto case_match;

        /* find the first non-matching character */
        while (ch_h == needle_ch)
        {
case_match:
            ch_h = *match++;
            needle_ch = tolower(*needle_search++);
            if (needle_ch == 0) /* end of needle, found match.. */
                return (char*)haystack - 1;
        }
        needle_ch = toupper(needle_ch);
        if (ch_h == needle_ch)
            goto case_match;
    }
}
#else
char* strcasestr (const char* phaystack, const char* pneedle)
{
    const unsigned char *haystack, *needle;
    chartype bl, bu, cl, cu;

    haystack = (const unsigned char *) phaystack;
    needle = (const unsigned char *) pneedle;

    bl = tolower (*needle);
    if (bl != '\0')
    {
        bu = toupper (bl);
        haystack--;/* possible ANSI violation */
        do
        {
            cl = *++haystack;
            if (cl == '\0')
                goto ret0;
        }
        while ((cl != bl) && (cl != bu));

        cl = tolower (*++needle);
        if (cl == '\0')
            goto foundneedle;
        cu = toupper (cl);
        ++needle;
        goto jin;

        for (;;)
        {
            chartype a;
            const unsigned char *rhaystack, *rneedle;

            do
            {
                a = *++haystack;
                if (a == '\0')
                    goto ret0;
                if ((a == bl) || (a == bu))
                    break;
                a = *++haystack;
                if (a == '\0')
                    goto ret0;
              shloop:
                ;
            }
            while ((a != bl) && (a != bu));

          jin:  a = *++haystack;
            if (a == '\0')
                goto ret0;

            if ((a != cl) && (a != cu))
                goto shloop;

            rhaystack = haystack-- + 1;
            rneedle = needle;
            a = tolower (*rneedle);

            if (tolower (*rhaystack) == (int) a)
                do
                {
                    if (a == '\0')
                        goto foundneedle;
                    ++rhaystack;
                    a = tolower (*++needle);
                    if (tolower (*rhaystack) != (int) a)
                        break;
                    if (a == '\0')
                        goto foundneedle;
                    ++rhaystack;
                    a = tolower (*++needle);
                }
                while (tolower (*rhaystack) == (int) a);

            needle = rneedle;/* took the register-poor approach */

            if (a == '\0')
                break;
        }
    }
  foundneedle:
    return (char*) haystack;
  ret0:
    return 0;
}
#endif
