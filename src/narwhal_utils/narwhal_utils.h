#ifndef NARWHAL_UTILS_H
#define NARWHAL_UTILS_H

/*
* $Id: narwhal_utils.h 54 2006-12-14 22:56:04Z kulibali $
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

#include <stdio.h>
#include <wchar.h>

#ifdef WIN32
#ifdef NARWHALUTILS_EXPORTS
#define NU_API __declspec(dllexport)
#else
#define NU_API __declspec(dllimport)
#endif
#else
#define NU_API
#endif

/** \name Platform Macros. */
/*@{*/

#ifdef WIN32

#pragma warning(disable : 4996)
#define _CRT_SECURE_NO_DEPRECATE
#define snprintf _snprintf

#else

#define strdup(s) str_dup(s)
#define wcsdup(s) wcs_dup(s)

#endif

/*@}*/

#ifdef __cplusplus
extern "C" {
#endif

    /** \name Dynamic array utilities. */
    /*@{*/

    typedef struct _array_t
    {
        int data_size;
        int num, cap;
        void *data;
    }
    array_t;

    /**
    * Initializes the dynamic array.
    */
    NU_API void array_init(array_t *da, const int data_size, const int num_items);

    /**
    * Returns the length of the dynamic array.
    */

#ifdef _DEBUG
    NU_API int array_size(const array_t *da);
#else
#define array_size(da) ((da)->num)
#endif

    /**
    * Returns the address of an item in the dynamic array.
    */

    NU_API void *array_item(const array_t *da, const int index);

    /**
    * Resizes the array.
    */
    NU_API void array_resize(array_t *da, const int num_items);

    /**
    * Adds the specified item to the array.
    */
    NU_API void array_add(array_t *da, void *item);

    /**
    * Copies the contents of the source array into the destination array.
    */
    NU_API void array_copy(array_t *dest, const array_t *src);

    /**
    * Frees the data held by the array.  Does not free the array pointer itself!
    */
    NU_API void array_deinit(array_t *da);

    /**
    * Clears the array and sets its size to zero.
    */
    NU_API void array_clear(array_t *da);

    /*@}*/



    /** \name File IO utilities. */
    /*@{*/

    NU_API wchar_t fgetc_utf8(FILE *file);

    /*@}*/


    /** \name Input Buffers */
    /*@{*/

    enum input_buffer_flags_et
    {
        INPUT_BUFFER_NULL_FLAGS       = 0,
        INPUT_BUFFER_UNICODE_BOM_READ = 1
    };

    enum input_buffer_unicode_mode_et
    {
        INPUT_BUFFER_ANSI     = 0,
        INPUT_BUFFER_UTF8     = 1,
        INPUT_BUFFER_UTF16_LE = 2,
        INPUT_BUFFER_UTF16_BE = 3,
        INPUT_BUFFER_UTF32_LE = 4,
        INPUT_BUFFER_UTF32_BE = 5
    };

    typedef struct _input_buffer_t
    {
        char *name;
        FILE *f;
        char *buf;
        int buf_size;
        int bytes_read;
        int current_pos;

        unsigned int flags;
        int unicode_mode;
    }
    input_buffer_t;

    NU_API input_buffer_t *input_buffer_create(const char *name, FILE *f);
    NU_API void input_buffer_destroy(input_buffer_t *ib);
    NU_API void input_buffer_setpos(input_buffer_t *ib, int pos);
    NU_API int input_buffer_getpos(const input_buffer_t *ib);
    NU_API void input_buffer_get_unicode_mode(input_buffer_t *ib);
    NU_API wchar_t input_buffer_read_char(input_buffer_t *ib);
    NU_API void input_buffer_read_string(input_buffer_t *ib, const int begin, const int end, array_t *str);

    NU_API void input_buffer_find_line_endings(array_t *end_offsets, input_buffer_t *ib, int start_pos);
    NU_API int input_buffer_find_line(int pos, array_t *end_offsets);

    /*@}*/


    /** \name Text Utilities */
    /*@{*/

    NU_API char *str_dup(const char *str);
    
    NU_API wchar_t *wcs_dup(const wchar_t *str);
    
    /** Trims whitespace from the beginning and end of a string. */
    NU_API void wcs_trim(wchar_t **str);
    NU_API void wcs_trim_char(wchar_t **str, wchar_t delim);

    NU_API wchar_t *str2wcs(const char *str);
    NU_API char *wcs2str(const wchar_t *wcs);

    /*@}*/

#ifdef __cplusplus
} // extern "C"
#endif

#endif
