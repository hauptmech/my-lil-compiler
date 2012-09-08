/*
 * $Id: narwhal_arrays.c 50 2006-09-21 19:57:58Z kulibali $
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
#include <malloc.h>
#include <string.h>
#include <wchar.h>

void array_init(array_t *da, const int data_size, const int num_items)
{
    assert(da);

    da->data_size = data_size;
    da->num       = num_items;
    da->cap       = num_items;

    if (num_items)
        da->data  = calloc(num_items, data_size);
    else
        da->data  = 0;
} /* array_init() */


#ifdef _DEBUG

int array_size(const array_t *da)
{
    assert(da);

    return da->num;
} /* array_size() */

#endif

void *array_item(const array_t *da, const int index)
{
    char *cp;

    assert(da);
    assert(index < da->num);

    cp = (char *) da->data;
    return cp + (da->data_size * index);
} /* array_item() */

void array_resize(array_t *da, const int num_items)
{
    assert(da);
    assert(num_items >= 0);

    if (num_items >= da->cap)
    {
        da->cap = num_items * 3 / 2 + 1;

        if (da->data)
            da->data = realloc(da->data, da->cap * da->data_size);
        else
            da->data = calloc(da->cap, da->data_size);
    }

    da->num = num_items;
} /* array_resize() */


void array_add(array_t *da, void *item)
{
    char *cp;
    int new_index;

    assert(da);

    new_index = da->num;

    if (da->num == da->cap)
        array_resize(da, da->num + 1);
    else
        da->num++;

    cp = (char *) da->data;
    memcpy(cp + (da->data_size * new_index), item, da->data_size);
} /* array_add() */


void array_copy(array_t *dest, const array_t *src)
{
    assert(dest);
    assert(src);
    assert(dest->data_size == src->data_size);

    array_deinit(dest);
    array_init(dest, dest->data_size, src->num);
    memcpy(dest->data, src->data, src->num * src->data_size);
} /* array_copy() */


void array_deinit(array_t *da)
{
    assert(da);

    free(da->data);
    da->data = 0;
    da->cap = da->num = 0;
} /* array_destroy() */


void array_clear(array_t *da)
{
    assert(da);

    da->num = 0;
} /* array_clear() */
