#ifndef PEG_PARSER_H
#define PEG_PARSER_H

/*
 * $Id: peg_parser.h 54 2006-12-14 22:56:04Z kulibali $
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

/** \file peg_parser.h
 * 
 * This file specified the definitions for a hand-coded parser for the PEG specification in 
 * Bryan Ford, "Parsing Expression Grammars: A Recognition-Based Syntactic Foundation".
 *
 * A PEG specification has the following structure:
 *
 * \verbatim
 *
 *     # Grammar
 *     Grammar <- Spacing Rule* EndOfFile
 *     Rule <- Identifier Literal? LEFTARROW Disjunction
 *     
 *     Disjunction <- Conjunction (SLASH Conjunction)*
 *     Conjunction <- PrefixExp+
 *     PrefixExp <- (AND / NOT / HIDE)? SuffixExp
 *     SuffixExp <- Term (QUESTION / STAR / PLUS)?
 *     Term <- ( Identifier !(Literal? LEFTARROW / EndOfLine '#')
 *           / OPEN Disjunction CLOSE
 *           / Literal / Class / DOT )
 *
 *     # lexicon
 *     Identifier <- IStart IBody* Spacing
 *     IStart <- [a-zA-Z_]
 *     IBody <- IStart / [0-9]
 *     
 *     Literal <- [’] (![’\n] Char)+ [’] Spacing
 *              / ["] (!["\n] Char)+ ["] Spacing
 *     Class <- ’[’ (![\n\]] Range)+ ’]’ Spacing
 *     Range <- (Char / "\]" / "\[") ’-’ (Char / "\]"/ "\[") / (Char / "\]" / "\[")
 *     Char <- ’\\’ ['"?\\abfnrtv]
 *           / ’\\’ [0-2][0-7][0-7]
 *           / ’\\’ [0-7][0-7]?
 *           / .
 *     LEFTARROW <- ’<-’ Spacing
 *     SLASH <- ’/’ Spacing
 *     AND <- ’&’ Spacing
 *     NOT <- ’!’ Spacing
 *     QUESTION <- ’?’ Spacing
 *     STAR <- ’*’ Spacing
 *     PLUS <- ’+’ Spacing
 *     OPEN <- ’(’ Spacing
 *     CLOSE <- ’)’ Spacing
 *     DOT <- ’.’ Spacing
 *     Spacing <- (Space / Comment)*
 *     Comment <- ’#’ (!EndOfLine .)* EndOfLine
 *     Space <- ’ ’ / ’\t’ / EndOfLine
 *     Hide <- '~' Spacing
 *     EndOfLine <- ’\r\n’ / ’\n’ / ’\r’
 *     EndOfFile <- !.
 *
 * \endverbatim
 */

#include "parsergen.h"

#ifdef __cplusplus
extern "C" {
#endif

    enum peg_node_type
    {
        PEG_NO_NODE         = 0,
        PEG_GRAMMAR_NODE    = 1,
        PEG_RULE_NODE       = 2,
        PEG_DISJ_NODE       = 3,
        PEG_CONJ_NODE       = 4,
        PEG_PREFIX_EXP_NODE = 5,
        PEG_SUFFIX_EXP_NODE = 6,
        PEG_TERM_NODE       = 7,
        PEG_IDENT_NODE      = 8,
        PEG_ISTART_NODE     = 9,
        PEG_IBODY_NODE      = 10,
        PEG_LITERAL_NODE    = 11,
        PEG_CLASS_NODE      = 12,
        PEG_RANGE_NODE      = 13,
        PEG_CHAR_NODE       = 14,
        PEG_LEFT_ARROW_NODE = 15,
        PEG_SLASH_NODE      = 16,
        PEG_AND_NODE        = 17,
        PEG_NOT_NODE        = 18,
        PEG_QUESTION_NODE   = 19,
        PEG_STAR_NODE       = 20,
        PEG_PLUS_NODE       = 21,
        PEG_OPEN_NODE       = 22,
        PEG_CLOSE_NODE      = 23,
        PEG_DOT_NODE        = 24,
        PEG_SPACING_NODE    = 25,
        PEG_COMMENT_NODE    = 26,
        PEG_SPACE_NODE      = 27,
        PEG_EOL_NODE        = 28,
        PEG_EOF_NODE        = 29,
        PEG_SUCCEED_BLOCK_NODE = 30,
        PEG_HIDE_NODE       = 31,
        PEG_NUM_NODE_TYPES  = 32
    };

    /** This is the main parsing function. */
    syntax_node_t *parse_peg_spec(input_buffer_t *ib, array_t *errors);

    void syntax_node_print(input_buffer_t *ib, syntax_node_t *node, int indent);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
