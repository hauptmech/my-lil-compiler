#ifndef PARSER_GEN_H
#define PARSER_GEN_H

/*
 * $Id: parsergen.h 50 2006-09-21 19:57:58Z kulibali $
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

#ifdef __cplusplus
extern "C" {
#endif

    /** Stores a production in the sytnax tree. */
    typedef struct _syntax_node_t
    {
        int type;               /**< The type of node; this is defined entirely by the grammar.  */
        int begin;              /**< The input position before the first character of the match. */
        int end;                /**< The input position after the last character of the match.   */
        int first_line;         /**< The line on which the match begins.                         */
        int last_line;          /**< The line on which the match ends.                           */
        struct _syntax_node_t **children; /**< Null-terminated array of child nodes.               */
    }
    syntax_node_t;

    /** Allocate and initialize a syntax node. */
    syntax_node_t *syntax_node_create(const int type, const int begin, const int end);

    /** Make a deep copy of a syntax node tree. */
    syntax_node_t *syntax_node_copy(const syntax_node_t *node);

    /** Deallocate a syntax node tree. */
    void syntax_node_destroy(syntax_node_t *node);

    /** A function type for a function that processes a single node. */
    typedef int (*syntax_node_process_ft)(syntax_node_t *node, void *data);

    /** Traverse a syntax node tree in infix order, processing each node with the given function. 
     *  If the function returns non-null for a given node, the traversal stops.
     */
    void syntax_node_traverse_inorder(syntax_node_t *root, void *data, syntax_node_process_ft process);

    /*************************************************/

    /** Records an error in the parse. */
    typedef struct _error_rec
    {
        int pos;
        wchar_t *str;
    }
    error_rec;

    /** Adds an error record to the array. */
    void add_error(array_t *errs, const int pos, const wchar_t *str);

    /** Deletes error records from the end of the array, starting with \code start_index. */
    void delete_errors(array_t *errs, const int start_index);

    /** Deletes all but the last error, and moves it up to the beginning of the array. */
    void strip_errors(array_t *errs);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
