#ifndef PARSERGEN_INTERNAL_H
#define PARSERGEN_INTERNAL_H

/*
* $Id: internal.h 54 2006-12-14 22:56:04Z kulibali $
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

#include "parsergen.h"

#ifdef __cplusplus
extern "C" {
#endif

    enum rule_exp_type_et
    {
        RULE_EXP_SEQ   = 1,
        RULE_EXP_DISJ  = 2,
        RULE_EXP_STAR  = 3,
        RULE_EXP_PLUS  = 4,
        RULE_EXP_QUES  = 5,
        RULE_EXP_BANG  = 6,
        RULE_EXP_AMP   = 7,
        RULE_EXP_CALL  = 8,
        RULE_EXP_STR   = 9,
        RULE_EXP_DOT   = 10,
        RULE_EXP_CLASS = 11,
        RULE_EXP_HIDE  = 12,

        NUM_RULE_EXP   = 13
    };

    typedef struct _rule_exp_t
    {
        int type;
        struct _rule_exp_t *left, *right;

        union 
        {
            wchar_t *str;
            struct _rule_rec_t *target;
        }
        data;
    }
    rule_exp_t;

    /** 
    * Contains data for a rule in a grammar file.
    */
    typedef struct _rule_rec_t
    {
        wchar_t *rule_name;
        wchar_t *rule_desc;
        syntax_node_t *node;

        rule_exp_t *rule_spec;
    }
    rule_rec_t;

    /*************************************************/

    void get_internal_representation(syntax_node_t **parsed_spec, input_buffer_t *ib, array_t *rule_records, array_t *line_endings, array_t *errors);

    void print_rules(array_t *rule_records);

    void cleanup_rule(rule_rec_t *rec);

    /*************************************************/

#ifdef __cplusplus
} // extern "C"
#endif

#endif
