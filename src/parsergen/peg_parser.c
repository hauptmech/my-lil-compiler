/*
 * $Id: peg_parser.c 54 2006-12-14 22:56:04Z kulibali $
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

#include "peg_parser.h"

#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <wchar.h>
#include <stdio.h>

/** \name Memoization */
/*@{*/

/**
 * Records a memoized parse tree.
 */
typedef struct _memo_rec_t
{
    int start_offset, end_offset;
    syntax_node_t *parse_tree;
}
memo_rec_t;

/**
 * Records memoizations for all node types.
 */
typedef struct _memo_map_t
{
    array_t records[PEG_NUM_NODE_TYPES]; /* a array_t of memo_rec_t structures */
}
memo_map_t;

/************************/

/**
 * Creates a memoization map.
 */
static memo_map_t *memo_map_create()
{
    int i;
    memo_map_t *map;
    
    map = (memo_map_t *) calloc(1, sizeof(memo_map_t));

    for (i = 0; i < PEG_NUM_NODE_TYPES; ++i)
    {
        array_init(&map->records[i], sizeof(memo_rec_t), 0);
    }

    return map;
} /* memo_map_create() */

/**
 * Destroys a memoization map.
 */
static void memo_map_destroy(memo_map_t *map)
{
    int i;

    assert(map);

    for (i = 0; i < PEG_NUM_NODE_TYPES; ++i)
    {
        int j, count = array_size(&map->records[i]);
        for (j = 0; j < count; ++j)
        {
            memo_rec_t *mr = (memo_rec_t *) array_item(&map->records[i], j);

            if (mr->parse_tree)
                syntax_node_destroy(mr->parse_tree);
        }

        array_deinit(&map->records[i]);
    }

    free(map);
} /* memo_map_destroy() */

/**
 * Returns true (and returns a copy of the memoized node) if there is a memoized value for the node type at the current position.
 */
static int is_memoized(memo_map_t *map, int type, int start_offset, syntax_node_t **res, int *end_offset)
{
    int i, len;
    array_t *da;

    assert(map);
    assert(type < PEG_NUM_NODE_TYPES);

    da = &map->records[type];
    len = array_size(da);

    for (i = 0; i < len; ++i)
    {
        memo_rec_t *rec = (memo_rec_t *) array_item(da, i);

        if (rec->start_offset == start_offset)
        {
            *res = syntax_node_copy(rec->parse_tree);
            *end_offset = rec->end_offset;
            return 1;
        }
    }

    return 0;
} /* is_memoized() */

/**
 * Copies the tree and records it in the memoization map.
 * \todo Sort these so find_memo_rec() can use a binary search.
 */
static void memoize(memo_map_t *map, int type, int start_offset, int end_offset, syntax_node_t *node)
{
    memo_rec_t rec;

    assert(map);
    assert(type < PEG_NUM_NODE_TYPES);

    rec.start_offset = start_offset;
    rec.end_offset = end_offset;
    rec.parse_tree = syntax_node_copy(node);

    array_add(&map->records[type], &rec);
} /* memoize() */

/*@}*/

static void delete_children(array_t *children, int start_index)
{
    int i, len;
    
    assert(children);
    
    len = array_size(children);

    for (i = start_index; i < len; ++i)
    {
        syntax_node_t *child = *(syntax_node_t **) array_item(children, i);
        syntax_node_destroy(child);
    }

    children->num = start_index;
} /* delete_children() */

/*************************************************/

#if 0
static int s_debug_indent = 0;

static void print_debug_indent(int indent, char *prefix, char *what, char *suffix, int pos)
{
    int i;
    for (i = 0; i < indent; ++i)
        fprintf(stdout, "  ");
    fprintf(stdout, "%s %s %s: %d\n", prefix, what, suffix, pos);
}

#define ENTER_DEBUG(X) print_debug_indent(s_debug_indent++, ">", #X, "", cur_start_pos)
#define EXIT_DEBUG(X) print_debug_indent(--s_debug_indent, "<", #X, res ? "ok" : "FAIL", cur_start_pos)
#else
#define ENTER_DEBUG(X) 
#define EXIT_DEBUG(X) 
#endif

#define SEQ(A, B)                           \
{                                           \
    int orig_stack_size = child_stack.num;  \
                                            \
    ENTER_DEBUG(seq);                       \
                                            \
    A;                                      \
                                            \
    if (res)                                \
    {                                       \
        B;                                  \
    }                                       \
                                            \
    if (res)                                \
        cur_start_pos = cur_end_pos;        \
    else                                    \
        delete_children(&child_stack, orig_stack_size);   \
                                            \
    EXIT_DEBUG(seq);                        \
}

#define DISJ(A, B)                          \
{                                           \
    int orig_stack_size = child_stack.num;  \
    int orig_start_pos = cur_start_pos;     \
                                            \
    ENTER_DEBUG(disj);                      \
                                            \
    A;                                      \
                                            \
    if (!res)                               \
    {                                       \
        delete_errors(error_stack, 0);      \
        cur_start_pos = orig_start_pos;     \
                                            \
        B;                                  \
    }                                       \
                                            \
    if (res)                                \
        cur_start_pos = cur_end_pos;        \
    else                                    \
        delete_children(&child_stack, orig_stack_size);   \
                                            \
    EXIT_DEBUG(disj);                       \
}

#define STAR(A)                             \
{                                           \
    ENTER_DEBUG(star);                      \
                                            \
    do                                      \
    {                                       \
        A;                                  \
                                            \
        if (res)                            \
            cur_start_pos = cur_end_pos;    \
    }                                       \
    while(res);                             \
    res = 1;                                \
                                            \
    delete_errors(error_stack, 0);          \
                                            \
    EXIT_DEBUG(star);                       \
}

#define PLUS(A)                             \
{                                           \
    int orig_stack_size = child_stack.num;  \
    int count = 0;                          \
    int error_stack_size = error_stack->num; \
                                            \
    ENTER_DEBUG(plus);                      \
                                            \
    do                                      \
    {                                       \
        A;                                  \
                                            \
        if (res)                            \
        {                                   \
            count++;                        \
            cur_start_pos = cur_end_pos;    \
        }                                   \
    }                                       \
    while (res);                            \
    res = count > 0;                        \
                                            \
    if (res)                                \
        delete_errors(error_stack, error_stack_size); \
    else                                    \
        delete_children(&child_stack, orig_stack_size); \
                                            \
    EXIT_DEBUG(plus);                       \
}

#define QUES(A)                             \
{                                           \
    int error_stack_size = error_stack->num; \
    ENTER_DEBUG(ques);                      \
                                            \
    A;                                      \
                                            \
    if (res)                                \
        cur_start_pos = cur_end_pos;        \
                                            \
    res = 1;                                \
    delete_errors(error_stack, error_stack_size); \
    EXIT_DEBUG(ques);                       \
}

#define BANG(A)                             \
{                                           \
    int orig_start_pos = cur_start_pos;     \
    int orig_stack_size = child_stack.num;  \
    int orig_error_size = error_stack->num; \
                                            \
    ENTER_DEBUG(bang);                      \
                                            \
    A;                                      \
                                            \
    res = !res;                             \
                                            \
    cur_start_pos = cur_end_pos = orig_start_pos; \
                                            \
    delete_children(&child_stack, orig_stack_size); \
    delete_errors(error_stack, orig_error_size); \
                                            \
    EXIT_DEBUG(bang);                       \
}

#define HIDE(A)                                     \
{                                                   \
    ENTER_DEBUG(hide);                              \
                                                    \
    int orig_stack_size = child_stack.num;          \
                                                    \
    A;                                              \
                                                    \
    delete_children(&child_stack, orig_stack_size); \
    cur_start_pos = cur_end_pos;                    \
                                                    \
    EXIT_DEBUG(hide);                               \
}

#define AMP(A)                              \
{                                           \
    int orig_start_pos = cur_start_pos;     \
                                            \
    ENTER_DEBUG(amp);                       \
                                            \
    A;                                      \
                                            \
    cur_start_pos = cur_end_pos = orig_start_pos; \
                                            \
    EXIT_DEBUG(amp);                        \
}

#define T(func)                                                     \
{                                                                   \
    syntax_node_t *child = 0;                                         \
    ENTER_DEBUG(func);                                              \
    if ((res = func(ib, cur_start_pos, &cur_end_pos, &child, map, error_stack))) \
    {                                                               \
        if (child)                                                  \
            array_add(&child_stack, &child);                        \
        cur_start_pos = cur_end_pos;                                \
    }                                                               \
    EXIT_DEBUG(func);                                               \
}

#define S(str)                                  \
{                                               \
    wchar_t ch, *ptr = str;                     \
                                                \
    ENTER_DEBUG(string);                        \
                                                \
    input_buffer_setpos(ib, cur_start_pos);     \
                                                \
    for (res = 1; *ptr; ++ptr)                  \
    {                                           \
        ch = input_buffer_read_char(ib);        \
        if (!(res = (ch == *ptr)))              \
            break;                              \
    }                                           \
                                                \
    if (res)                                    \
        cur_start_pos = cur_end_pos = input_buffer_getpos(ib);  \
                                                \
    EXIT_DEBUG(string);                         \
}

#define C(str)                                  \
{                                               \
    wchar_t ch, *ptr = str;                     \
    ENTER_DEBUG(class);                         \
    input_buffer_setpos(ib, cur_start_pos);     \
    ch = input_buffer_read_char(ib);            \
    for (res = 0; *ptr; ++ptr)                  \
    {                                           \
        if ((res = (ch == *ptr)))               \
            break;                              \
    }                                           \
    if (res)                                    \
        cur_start_pos = cur_end_pos = input_buffer_getpos(ib);  \
    EXIT_DEBUG(class);                          \
}

#define DOT                                     \
{                                               \
    wchar_t ch;                                 \
    ENTER_DEBUG(dot);                           \
    input_buffer_setpos(ib, cur_start_pos);     \
    ch = input_buffer_read_char(ib);            \
    if ((res = (ch != WEOF)))                     \
        cur_start_pos = cur_end_pos = input_buffer_getpos(ib);  \
    EXIT_DEBUG(dot);                            \
}

#define PEG_PARSE(FUNCTION, NODE_TYPE, NODE_NAME, EXP)                                                                 \
static int FUNCTION(input_buffer_t *ib, int start_offset, int *end_offset, syntax_node_t **node, memo_map_t *map, array_t *error_stack) \
{                                                                                                           \
    int res = 0;                                                                                            \
    int cur_start_pos = start_offset, cur_end_pos = start_offset;                                           \
                                                                                                            \
    array_t child_stack;                                                                                   \
    array_init(&child_stack, sizeof(syntax_node_t *), 0);                                                     \
                                                                                                            \
    if (is_memoized(map, NODE_TYPE, start_offset, node, end_offset))                                        \
        return *node != 0;                                                                                  \
                                                                                                            \
    EXP;                                                                                                    \
                                                                                                            \
    if (res)                                                                                                \
    {                                                                                                       \
        int i, len;                                                                                         \
        *end_offset = cur_end_pos;                                                                          \
        *node = syntax_node_create(NODE_TYPE, start_offset, cur_end_pos);                                   \
        len = array_size(&child_stack);                                                                     \
        (*node)->children = (syntax_node_t **) calloc(len+1, sizeof(syntax_node_t *));                          \
                                                                                                            \
        for (i = 0; i < len; ++i)                                                                           \
        {                                                                                                   \
            (*node)->children[i] = *(syntax_node_t **) array_item(&child_stack, i);                           \
        }                                                                                                   \
        delete_children(&child_stack, len);                                                                 \
        array_deinit(&child_stack);                                                                         \
                                                                                                            \
        res = 1;                                                                                            \
    }                                                                                                       \
    else                                                                                                    \
    {                                                                                                       \
        int len = (int) wcslen(NODE_NAME) + 64;                                                             \
        wchar_t *buf = (wchar_t *) calloc(len, sizeof(wchar_t));                                            \
        swprintf(buf, len, L"syntax error: expected %ls", NODE_NAME);                                        \
        add_error(error_stack, start_offset, buf);                                                          \
        free(buf);                                                                                          \
        *node = 0;                                                                                          \
        delete_children(&child_stack, 0);                                                                   \
        array_deinit(&child_stack);                                                                         \
    }                                                                                                       \
                                                                                                            \
    memoize(map, NODE_TYPE, start_offset, res ? *end_offset : start_offset, *node);                         \
                                                                                                            \
    return res;                                                                                             \
}

#define PEG_FUNC(F) static int F(input_buffer_t *ib, int start_offset, int *end_offset, syntax_node_t **node, memo_map_t *map, array_t *error_stack)

/*************************************************/

PEG_FUNC(parse_peg_grammar);
PEG_FUNC(parse_peg_rule);
PEG_FUNC(parse_peg_disjunction);
PEG_FUNC(parse_peg_conjunction);
PEG_FUNC(parse_peg_prefix_exp);
PEG_FUNC(parse_peg_suffix_exp);
PEG_FUNC(parse_peg_term);
PEG_FUNC(parse_peg_identifier);
PEG_FUNC(parse_peg_literal);
PEG_FUNC(parse_peg_class);
PEG_FUNC(parse_peg_range);
PEG_FUNC(parse_peg_char);
PEG_FUNC(parse_peg_left_arrow);
PEG_FUNC(parse_peg_slash);
PEG_FUNC(parse_peg_and);
PEG_FUNC(parse_peg_not);
PEG_FUNC(parse_peg_question);
PEG_FUNC(parse_peg_star);
PEG_FUNC(parse_peg_plus);
PEG_FUNC(parse_peg_open);
PEG_FUNC(parse_peg_close);
PEG_FUNC(parse_peg_dot);
PEG_FUNC(parse_peg_spacing);
PEG_FUNC(parse_peg_comment);
PEG_FUNC(parse_peg_space);
PEG_FUNC(parse_peg_eol);
PEG_FUNC(parse_peg_eof);
PEG_FUNC(parse_peg_hide);

/*************************************************/

/* grammar */

PEG_PARSE(parse_peg_grammar,        PEG_GRAMMAR_NODE,               L"PEG grammar",             SEQ(T(parse_peg_spacing),
                                                                                                    SEQ(PLUS(T(parse_peg_rule)),
                                                                                                        T(parse_peg_eof))));

PEG_PARSE(parse_peg_rule,           PEG_RULE_NODE,                  L"PEG rule",                SEQ(T(parse_peg_identifier),
                                                                                                    SEQ(QUES(T(parse_peg_literal)),
                                                                                                        SEQ(T(parse_peg_left_arrow),
                                                                                                            T(parse_peg_disjunction)))));

PEG_PARSE(parse_peg_disjunction,    PEG_DISJ_NODE,                  L"expression",              SEQ(T(parse_peg_conjunction), STAR(SEQ(T(parse_peg_slash), T(parse_peg_conjunction)))));

PEG_PARSE(parse_peg_conjunction,    PEG_CONJ_NODE,                  L"term",                    PLUS(T(parse_peg_prefix_exp)));

PEG_PARSE(parse_peg_prefix_exp,     PEG_PREFIX_EXP_NODE,            L"term",                    SEQ(QUES(DISJ(T(parse_peg_and), DISJ(T(parse_peg_not), T(parse_peg_hide)))), T(parse_peg_suffix_exp)));

PEG_PARSE(parse_peg_suffix_exp,     PEG_SUFFIX_EXP_NODE,            L"term",                    SEQ(T(parse_peg_term),
                                                                                                    QUES(DISJ(DISJ(T(parse_peg_question), 
                                                                                                                   T(parse_peg_star)),
                                                                                                                   T(parse_peg_plus)))));

PEG_PARSE(parse_peg_term, PEG_TERM_NODE, L"term", DISJ(SEQ(T(parse_peg_identifier), 
                                                           BANG(SEQ(QUES(T(parse_peg_literal)), 
                                                                    T(parse_peg_left_arrow)))),
                                                       DISJ(SEQ(T(parse_peg_open), 
                                                                SEQ(T(parse_peg_disjunction), 
                                                                    T(parse_peg_close))),
                                                            DISJ(T(parse_peg_literal),
                                                                 DISJ(T(parse_peg_class), 
                                                                      T(parse_peg_dot))))));

/* lexicon */

PEG_PARSE(parse_peg_identifier, PEG_IDENT_NODE, L"identifier", SEQ(SEQ(C(L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"), STAR(DISJ(C(L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"), C(L"0123456789")))), T(parse_peg_spacing)));

PEG_PARSE(parse_peg_literal,    PEG_LITERAL_NODE, L"literal string",      DISJ( SEQ(C(L"'"), SEQ(PLUS(SEQ(BANG(C(L"'\n")), T(parse_peg_char))), SEQ(C(L"'"), T(parse_peg_spacing)))),
                                                                          SEQ(C(L"\""), SEQ(PLUS(SEQ(BANG(C(L"\"\n")), T(parse_peg_char))), SEQ(C(L"\""), T(parse_peg_spacing)))) ));

PEG_PARSE(parse_peg_class,      PEG_CLASS_NODE, L"character class",        SEQ(S(L"["), SEQ(PLUS(SEQ(BANG(C(L"]\n")), T(parse_peg_range))), SEQ(S(L"]"), T(parse_peg_spacing)))));
PEG_PARSE(parse_peg_range,      PEG_RANGE_NODE, L"character range",        DISJ( SEQ( DISJ(T(parse_peg_char), DISJ(S(L"\\["), S(L"\\]"))), SEQ( S(L"-"), DISJ(T(parse_peg_char), DISJ(S(L"\\["), S(L"\\]"))) ) ), DISJ(T(parse_peg_char), DISJ(S(L"\\["), S(L"\\]"))) ) );

PEG_PARSE(parse_peg_char,       PEG_CHAR_NODE, L"character",         DISJ(SEQ(S(L"\\"), C(L"\'\"?\\abfnrtv")) , DISJ( SEQ(S(L"\\"), SEQ(C(L"012"), SEQ(C(L"01234567"), C(L"01234567")))), DISJ(SEQ(S(L"\\"), SEQ(C(L"01234567"), QUES(C(L"01234567")))), SEQ(BANG(S(L"\\")), DOT)))));

PEG_PARSE(parse_peg_left_arrow, PEG_LEFT_ARROW_NODE,    L"left arrow", SEQ(S(L"<-"), T(parse_peg_spacing)));
PEG_PARSE(parse_peg_slash,      PEG_SLASH_NODE,         L"'/'", SEQ(S(L"/"), T(parse_peg_spacing)));
PEG_PARSE(parse_peg_and,        PEG_AND_NODE,           L"'&'", SEQ(S(L"&"), T(parse_peg_spacing)));
PEG_PARSE(parse_peg_not,        PEG_NOT_NODE,           L"'!'", SEQ(S(L"!"), T(parse_peg_spacing)));
PEG_PARSE(parse_peg_question,   PEG_QUESTION_NODE,      L"'?'", SEQ(S(L"?"), T(parse_peg_spacing)));
PEG_PARSE(parse_peg_star,       PEG_STAR_NODE,          L"'*'",   SEQ(S(L"*"), T(parse_peg_spacing)));
PEG_PARSE(parse_peg_plus,       PEG_PLUS_NODE,          L"'+'", SEQ(S(L"+"), T(parse_peg_spacing)));
PEG_PARSE(parse_peg_open,       PEG_OPEN_NODE,          L"'('", SEQ(S(L"("), T(parse_peg_spacing)));
PEG_PARSE(parse_peg_close,      PEG_CLOSE_NODE,         L"')'", SEQ(S(L")"), T(parse_peg_spacing)));
PEG_PARSE(parse_peg_dot,        PEG_DOT_NODE,           L"'.'", SEQ(S(L"."), T(parse_peg_spacing)));

PEG_PARSE(parse_peg_spacing,    PEG_SPACING_NODE,       L"whitespace", STAR(DISJ(T(parse_peg_space), T(parse_peg_comment))));
PEG_PARSE(parse_peg_comment,    PEG_COMMENT_NODE,       L"comment", SEQ(S(L"#"), SEQ(STAR(SEQ(BANG(T(parse_peg_eol)), DOT)), T(parse_peg_eol))));
PEG_PARSE(parse_peg_space,      PEG_SPACE_NODE,         L"space character", DISJ(S(L" "), DISJ(S(L"\t"), T(parse_peg_eol))));

PEG_PARSE(parse_peg_eol, PEG_EOL_NODE, L"end-of-line", DISJ(S(L"\r\n"), DISJ(S(L"\n"), S(L"\r"))));
PEG_PARSE(parse_peg_eof, PEG_EOF_NODE, L"end-of-file", BANG(DOT));

PEG_PARSE(parse_peg_hide, PEG_HIDE_NODE, L"~", SEQ(S(L"~"), T(parse_peg_spacing)));

/*************************************************/

syntax_node_t *parse_peg_spec(input_buffer_t *ib, array_t *errors)
{
    int start_offset, end_offset;
    memo_map_t *map = memo_map_create();
    syntax_node_t *grammar = 0;

    start_offset = input_buffer_getpos(ib);
    parse_peg_grammar(ib, start_offset, &end_offset, &grammar, map, errors);
    
    memo_map_destroy(map);

    return grammar;
} /* parse_peg_spec() */

/*************************************************/

static void print_indent(FILE *out, int indent)
{
    int i;
    for (i = 0; i < indent; ++i)
        fprintf(out, "  ");
} /* print_indent() */

void syntax_node_print(input_buffer_t *ib, syntax_node_t *node, int indent)
{
    syntax_node_t **cur;
    array_t str;
    FILE *out = stdout;

    assert(node);

    array_init(&str, sizeof(wchar_t), 128);

    print_indent(out, indent);

    switch (node->type)
    {
    case PEG_NO_NODE:
        fprintf(out, "ERROR: NO NODE TYPE");

        break;
    case PEG_GRAMMAR_NODE:
        fprintf(out, "grammar");

        break;
    case PEG_RULE_NODE:
        fprintf(out, "rule");

        break;
    case PEG_DISJ_NODE:
        fprintf(out, "disj");

        break;
    case PEG_CONJ_NODE:
        fprintf(out, "conj");

        break;
    case PEG_PREFIX_EXP_NODE:
        fprintf(out, "prefix exp");

        break;
    case PEG_SUFFIX_EXP_NODE:
        fprintf(out, "suffix exp");

        break;
    case PEG_TERM_NODE:
        fprintf(out, "term");

        break;
    case PEG_IDENT_NODE:
        input_buffer_read_string(ib, node->begin, node->end, &str);
        fprintf(out, "identifier: %ls", (wchar_t *) str.data);

        break;
    case PEG_ISTART_NODE:
        fprintf(out, "istart");

        break;
    case PEG_IBODY_NODE:
        fprintf(out, "ibody");

        break;
    case PEG_LITERAL_NODE:
        input_buffer_read_string(ib, node->begin, node->end, &str);
        fprintf(out, "literal: %ls", (wchar_t *) str.data);

        break;
    case PEG_CLASS_NODE:
        input_buffer_read_string(ib, node->begin, node->end, &str);
        fprintf(out, "class: %ls", (wchar_t *) str.data);

        break;
    case PEG_RANGE_NODE:
        input_buffer_read_string(ib, node->begin, node->end, &str);
        fprintf(out, "range: %ls", (wchar_t *) str.data);

        break;
    case PEG_CHAR_NODE:
        input_buffer_read_string(ib, node->begin, node->end, &str);
        fprintf(out, "char: %ls", (wchar_t *) str.data);

        break;
    case PEG_LEFT_ARROW_NODE:
        input_buffer_read_string(ib, node->begin, node->end, &str);
        fprintf(out, "arrow: %ls", (wchar_t *) str.data);

        break;
    case PEG_SLASH_NODE:
        fprintf(out, "slash");

        break;
    case PEG_AND_NODE:
        fprintf(out, "and");

        break;
    case PEG_NOT_NODE:
        fprintf(out, "not");

        break;
    case PEG_QUESTION_NODE:
        fprintf(out, "question");

        break;
    case PEG_STAR_NODE:
        fprintf(out, "star");

        break;
    case PEG_PLUS_NODE:
        fprintf(out, "plus");

        break;
    case PEG_OPEN_NODE:
        fprintf(out, "open");

        break;
    case PEG_CLOSE_NODE:
        fprintf(out, "close");

        break;
    case PEG_DOT_NODE:
        fprintf(out, "dot");

        break;
    case PEG_SPACING_NODE:
        fprintf(out, "spacing");

        break;
    case PEG_COMMENT_NODE:
        input_buffer_read_string(ib, node->begin, node->end, &str);
        fprintf(out, "comment: %ls", (wchar_t *) str.data);

        break;
    case PEG_SPACE_NODE:
        fprintf(out, "space");

        break;
    case PEG_EOL_NODE:
        fprintf(out, "EOL");

        break;
    case PEG_EOF_NODE:
        fprintf(out, "EOF");

        break;
    case PEG_SUCCEED_BLOCK_NODE:
        input_buffer_read_string(ib, node->begin, node->end, &str);
        fprintf(out, "block: \n%ls", (wchar_t *) str.data);
        break;
    case PEG_NUM_NODE_TYPES:
        /* fall through */
    default:
        fprintf(out, "ERROR: INVALID NODE TYPE %d", node->type);

        break;
    }

    fprintf(out, " (%d-%d, %d-%d)\n", node->begin, node->end, node->first_line, node->last_line);

    /* print children */
    if (node->children)
    {
        for (cur = node->children; *cur; ++cur)
            syntax_node_print(ib, *cur, indent+1);
    }
} /* syntax_node_print() */
