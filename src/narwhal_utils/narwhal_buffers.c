/*
 * $Id: narwhal_buffers.c 52 2006-11-08 03:28:22Z kulibali $
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
#include <string.h>
#include <wchar.h>

#define INPUT_BUFFER_SIZE_INCREMENT 8192

input_buffer_t *input_buffer_create(const char *name, FILE *f)
{
    input_buffer_t *ib;

    assert(f);

    ib = (input_buffer_t *) calloc(1, sizeof(input_buffer_t));
    ib->name = strdup(name);
    ib->f = f;

    input_buffer_get_unicode_mode(ib);

    return ib;
} /* input_buffer_create() */

void input_buffer_destroy(input_buffer_t *ib)
{
    assert(ib);

    free(ib->name);
    free(ib->buf);
    free(ib);
} /* input_buffer_destroy() */

void input_buffer_setpos(input_buffer_t *ib, int pos)
{
    assert(ib);
    assert(pos >= 0);

    ib->current_pos = pos;
} /* input_buffer_setpos() */

int input_buffer_getpos(const input_buffer_t *ib)
{
    assert(ib);

    return ib->current_pos;
} /* input_buffer_getpos() */

/** \todo Implement unicode reading. */
void input_buffer_get_unicode_mode(input_buffer_t *ib)
{
    assert(ib);

    ib->unicode_mode = INPUT_BUFFER_ANSI;
    ib->flags |= INPUT_BUFFER_UNICODE_BOM_READ;
} /* input_buffer_get_unicode_mode() */

wchar_t input_buffer_read_char(input_buffer_t *ib)
{
    wchar_t ch;
    int prev_pos, new_pos;

    assert(ib);
    
    /* resize buffer if necessary, and read */
    while (ib->current_pos >= ib->bytes_read)
    {
        size_t num_bytes_read;

        ib->buf = (char *) realloc(ib->buf, ib->buf_size += INPUT_BUFFER_SIZE_INCREMENT);

        if (!ib->buf)
        {
            return WEOF;
            ib->buf_size -= INPUT_BUFFER_SIZE_INCREMENT;
        }

        num_bytes_read = fread(ib->buf + ib->bytes_read, 1, INPUT_BUFFER_SIZE_INCREMENT, ib->f);

        if (num_bytes_read)
        {
            ib->bytes_read += (int) num_bytes_read;
        }
        else
        {
            return WEOF;
        }
    }

    /* get next code point and check for eol */
    prev_pos = ib->current_pos;

    ch = (wchar_t) ib->buf[ib->current_pos++]; /* just for now */

    new_pos = ib->current_pos;

    return ch;
} /* input_buffer_read_char() */


void input_buffer_read_string(input_buffer_t *ib, const int begin, const int end, array_t *str)
{
    wchar_t ch = WEOF;

    assert(ib);
    assert(str);

    array_clear(str);
    ib->current_pos = begin;
    while (ib->current_pos < end)
    {
        ch = input_buffer_read_char(ib);

        if (ch != WEOF)
            array_add(str, &ch);
    }

    ch = 0;
    array_add(str, &ch);
} /* input_buffer_read_string() */




/** 
 * Goes through and finds the lengths of all the line endings in the input buffer (in bytes). 
 * \param end_offsets A pre-initialized array of int to store line offsets.
 * \param ib The input buffer to search.
 * \param start_pos The position in the input buffer in which to start.
 */
void input_buffer_find_line_endings(array_t *end_offsets, input_buffer_t *ib, int start_pos)
{
    wchar_t ch, eol_pos = 0;
    int pos;

    input_buffer_setpos(ib, start_pos);

    do
    {
        ch = input_buffer_read_char(ib);

        if (ch == L'\n')
        {
            pos = input_buffer_getpos(ib);
            array_add(end_offsets, &pos);
            eol_pos = 0;
        }
        else if (ch == L'\r')
        {
            eol_pos = input_buffer_getpos(ib);
        }
        else
        {
            if (eol_pos > 0)
            {
                pos = eol_pos;
                array_add(end_offsets, &pos);
            }

            eol_pos = 0;
        }
    }
    while (ch != WEOF);

    pos = input_buffer_getpos(ib);
    array_add(end_offsets, &pos);
} /* find_line_lengths() */

/** Given a particular character offset, figures out which lines it's on. */
int input_buffer_find_line(int pos, array_t *end_offsets)
{
    int n, i, len = array_size(end_offsets);
    for (i = 0; i < len; ++i)
    {
        n = * (int *) array_item(end_offsets, i);
        if (pos < n)
            return i+1;
    }

    if (pos == n)
        return len;
    else
        return -1;
} /* find_line() */

