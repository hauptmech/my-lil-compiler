/*
* $Id: narwhal_utils.h 23 2006-07-06 23:49:52Z kulibali $
*
* Copyright (c) 2006, The Narwhal Project 
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*    * Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*    * Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
*    * Neither the name of the Narwhal Project nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#include "narwhal_utils.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

/*************************************************/

char *str_dup(const char *str)
{
    char *res;
    int len = (int) strlen(str) + 1;

    res = (char *) calloc(len, sizeof(char));
    memcpy(res, str, len * sizeof(char));

    return res;
} /* str_dup() */

wchar_t *wcs_dup(const wchar_t *str)
{
    wchar_t *res;
    int len = (int) wcslen(str) + 1;

    res = (wchar_t *) calloc(len, sizeof(wchar_t));
    memcpy(res, str, len * sizeof(wchar_t));

    return res;
} /* wcs_dup() */

void wcs_trim(wchar_t **str)
{
    wchar_t *orig;
    wchar_t *first;
    wchar_t *last;

    assert(str);
    assert(*str);

    orig = *str;
    first = *str;

    /* get the first non-space character */
    while (*first && iswspace(*first))
        ++first;

    /* truncate the end */
    if (*first)
    {
        last = first + (wcslen(first) - 1);
        while (last > first && *last && iswspace(*last))
            *last-- = 0;

        /* replace original string */
        *str = wcsdup(first);
        free(orig);
    }
} /* wcs_trim() */


void wcs_trim_char(wchar_t **str, wchar_t delim)
{
    wchar_t *orig;
    wchar_t *first;
    wchar_t *last;

    assert(str);
    assert(*str);

    orig = *str;
    first = *str;

    /* get the first non-space character */
    while (*first && *first == delim)
        ++first;

    /* truncate the end */
    if (*first)
    {
        last = first + (wcslen(first) - 1);
        while (last > first && *last && *last == delim)
            *last-- = 0;

        /* replace original string */
        *str = wcsdup(first);
        free(orig);
    }
} /* wcs_trim_char() */


wchar_t *str2wcs(const char *str)
{
    int len = (int) strlen(str);
    wchar_t *wcs = (wchar_t *) malloc(sizeof(wchar_t) * (len+2));
    swprintf(wcs, len+1, L"%hs", str);
    return wcs;
} /* str2wcs() */


char *wcs2str(const wchar_t *wcs)
{
    int len = (int) wcslen(wcs) * 2;
    char *str = (char *) malloc(sizeof(char) * (len+2));
    snprintf(str, len+1, "%ls", wcs);
    return str;
} /* wcs2str() */
