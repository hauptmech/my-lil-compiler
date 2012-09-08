/*
 * $Id: internal.c 54 2006-12-14 22:56:04Z kulibali $
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

#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <assert.h>
#include "internal.h"
#include "peg_parser.h"

/*************************************************/

/**
 * Stores information about a rule call, for convenience in checking invalid identifiers.
 */
typedef struct _rule_call
{
    wchar_t *rule_name;
    syntax_node_t *node;
    rule_exp_t *exp;
}
rule_call;

/**
 * Stores context information for assembling rules.
 */

typedef struct _rule_context
{
    input_buffer_t *ib;

    array_t *rule_records; /* array of rule_rec_t * */
    array_t *rule_calls;   /* array of rule_call */
}
rule_context;

/*************************************************/

static syntax_node_t *find_first_spacing_node(syntax_node_t *node)
{
    if (node->type == PEG_SPACING_NODE)
        return node;

    if (node->children)
    {
        syntax_node_t **cur, *temp;
        for (cur = node->children; *cur; ++cur)
        {
            if ((temp = find_first_spacing_node(*cur)))
                return temp;
        }
    }

    return 0;
} /* find_first_spacing_node() */

static wchar_t *get_string_without_spacing(syntax_node_t *node, input_buffer_t *ib)
{
    wchar_t *res = 0;
    array_t dest;
    int begin, end;
    syntax_node_t *space;
    
    begin = node->begin;
    end = node->end;

    if ((space = find_first_spacing_node(node)))
    {
        end = space->begin;
    }

    if (end > begin)
    {
        array_init(&dest, sizeof(wchar_t), 1+end-begin);
        input_buffer_read_string(ib, begin, end, &dest);
        res = wcsdup(dest.data);        
        array_deinit(&dest);
    }
    else
    {
        res = wcsdup(L"");
    }

    wcs_trim(&res);

    return res;
} /* get_string_without_spacing() */

/*************************************************/

static wchar_t *get_unquoted(wchar_t *str)
{
    wchar_t *cur = str;
    int len;

    assert(str);

    if ((len = (int) wcslen(str)))
    {
        if (str[0] == '"')
        {
            ++cur;

            if (str[len-1] == '"')
                str[len-1] = 0;
        }
        else if (str[0] == '\'')
        {
            ++cur;

            if (str[len-1] == '\'')
                str[len-1] = 0;
        }
    }

    return cur;
} /* get_unquoted() */


static void collect_rule_name(rule_rec_t *rec, syntax_node_t *node, input_buffer_t *ib)
{
    syntax_node_t **cur;
    int i, len;

    assert(node->children);

    len = 0;
    for (cur = node->children; *cur; ++cur)
        ++len;

    // get name
    if (len)
    {
        syntax_node_t *id_node = node->children[0];

        if (id_node->type == PEG_IDENT_NODE)
        {
            rec->rule_name = get_string_without_spacing(id_node, ib);
        }
    }

    // get description, if any
    if (len > 1)
    {
        for (i = 0; i < len; ++i)
        {
            syntax_node_t *child = node->children[i];
            if (child->type == PEG_LITERAL_NODE)
            {
                wchar_t *str = get_string_without_spacing(child, ib);
                rec->rule_desc = wcsdup(get_unquoted(str));
                free(str);

                break;
            }
        }
    }
} /* collect_rule_name() */


static rule_exp_t *collect_rule_exp(syntax_node_t *node, rule_context *context);

static rule_exp_t *collect_rule_children(syntax_node_t **children, int exp_type, rule_context *context)
{
    syntax_node_t **child;
    rule_exp_t *first = 0;
    rule_exp_t **cur = &first;

    assert(children);

    for (child = children; *child && *(child+1); ++child)
    {
        if ((*child)->type != PEG_SLASH_NODE)
        {
            *cur = (rule_exp_t *) calloc(1, sizeof(rule_exp_t));
            (*cur)->type = exp_type;
            (*cur)->left = collect_rule_exp(*child, context);
            cur = &(*cur)->right;
        }
    }

    *cur = collect_rule_exp(*child, context);

    return first;
} /* collect_rule_children() */


static rule_exp_t *collect_prefix_exp(syntax_node_t **children, rule_context *context)
{
    syntax_node_t **cur;
    int len;

    assert(children);

    len = 0;
    for (cur = children; *cur; ++cur)
        ++len;

    if (len == 2)
    {
        int rule_type = 0;
        syntax_node_t *pref = children[0];

        if (pref->type == PEG_AND_NODE)
            rule_type = RULE_EXP_AMP;
        else if (pref->type == PEG_NOT_NODE)
            rule_type = RULE_EXP_BANG;
        else if (pref->type == PEG_HIDE_NODE)
            rule_type = RULE_EXP_HIDE;

        if (rule_type)
        {
            rule_exp_t *exp = (rule_exp_t *) calloc(1, sizeof(rule_exp_t));
            exp->type = rule_type;
            exp->left = collect_rule_exp(children[1], context);
            return exp;
        }
    }
    else if (len == 1)
    {
        return collect_rule_exp(children[0], context);
    }

    /* can't happen */
    assert(0);
    return 0;
} /* collect_prefix_exp() */


static rule_exp_t *collect_suffix_exp(syntax_node_t **children, rule_context *context)
{
    syntax_node_t **cur;
    int len;

    assert(children);

    len = 0;
    for (cur = children; *cur; ++cur)
        ++len;

    if (len == 2)
    {
        int rule_type = 0;
        syntax_node_t *op = children[1];

        if (op->type == PEG_QUESTION_NODE)
            rule_type = RULE_EXP_QUES;
        else if (op->type == PEG_STAR_NODE)
            rule_type = RULE_EXP_STAR;
        else if (op->type == PEG_PLUS_NODE)
            rule_type = RULE_EXP_PLUS;

        if (rule_type)
        {
            rule_exp_t *exp = (rule_exp_t *) calloc(1, sizeof(rule_exp_t));
            exp->type = rule_type;
            exp->left = collect_rule_exp(children[0], context);
            return exp;
        }
    }
    else if (len == 1)
    {
        return collect_rule_exp(children[0], context);
    }

    /* can't happen */
    assert(0);
    return 0;
} /* collect_suffix_exp() */


static rule_exp_t *collect_rule_call_exp(syntax_node_t *node, rule_context *context)
{
    rule_exp_t *exp;
    rule_call call;

    /* initialize exp */
    exp = (rule_exp_t *) calloc(1, sizeof(rule_exp_t));
    exp->type = RULE_EXP_CALL;

    /* get name */
    exp->data.str = get_string_without_spacing(node, context->ib);

    /* record call */
    call.node = node;
    call.rule_name = exp->data.str;
    call.exp = exp;
    array_add(context->rule_calls, &call);

    /* return */
    return exp;
} /* collect_rule_call_exp() */


static wchar_t get_escaped_char(wchar_t *str, int index, int len, int *span)
{
    wchar_t ch = str[index];
    *span = 1;

    if (ch == L'\\')
    {
        /* check for octal-coded characters */
        if (index < (len-3))
        {
            wchar_t d[3];

            d[0] = str[index+1];
            d[1] = str[index+2];
            d[2] = (index < (len+4)) ? str[index+3] : 0;

            if (iswdigit(d[0]) && iswdigit(d[1]))
            {
                if (iswdigit(d[2]))
                {
                    *span = 4;
                    return (wchar_t) (((d[0] - L'0') * 64) + ((d[1] - L'0') * 8) + (d[2] - L'0'));
                }
                else
                {
                    *span = 3;
                    return (wchar_t) (((d[0] - L'0') * 8) + (d[1] - L'0'));
                }
            }
        }

        /* check for hexadecimal-coded characters */

        /* check for unicode characters */

        /* check for other escaped chars */
        if (index < (len-1))
        {
            wchar_t e = str[index+1];

            switch (e)
            {
            case L'\'':
                ch = L'\'';
                break;
            case L'\"':
                ch = L'\"';
                break;
            case L'?':
                ch = L'\?';
                break;
            case L'\\':
                ch = L'\\';
                break;
            case L'a':
                ch = L'\a';
                break;
            case L'b':
                ch = L'\b';
                break;
            case L'f':
                ch = L'\f';
                break;
            case L'n':
                ch = L'\n';
                break;
            case L'r':
                ch = L'\r';
                break;
            case L't':
                ch = L'\t';
                break;
            case L'v':
                ch = L'\v';
                break;
            default:
                break;
            }

            *span = 2;
        }
    }

    return ch;
} /* get_escaped_char() */


static rule_exp_t *collect_string_exp(syntax_node_t *node, rule_context *context)
{
    rule_exp_t *exp = 0;
    wchar_t *str, *cur, zero = 0;
    int i, len, span = 1;
    array_t res;

    assert(node->children);

    /* initialize exp */
    exp = (rule_exp_t *) calloc(1, sizeof(rule_exp_t));
    exp->type = RULE_EXP_STR;

    /* get string */
    str = get_string_without_spacing(node, context->ib);
    cur = get_unquoted(str);
    len = (int) wcslen(cur);
    array_init(&res, sizeof(wchar_t), 0);

    for (i = 0; i < len; i += span)
    {
        wchar_t ch;
        
        span = 1;
        ch = get_escaped_char(cur, i, len, &span);
        array_add(&res, &ch);
    }

    array_add(&res, &zero);
    exp->data.str = wcsdup(res.data);
    array_deinit(&res);
    free(str);

    return exp;
} /* collect_string_exp() */


static rule_exp_t *collect_class_exp(syntax_node_t *node, rule_context *context)
{
    rule_exp_t *exp = 0;
    array_t cc;
    wchar_t *str, *quoted;
    wchar_t ch = 0;
    int i, len;

    assert(node->children);

    /* initialize exp */
    exp = (rule_exp_t *) calloc(1, sizeof(rule_exp_t));
    exp->type = RULE_EXP_CLASS;

    array_init(&cc, sizeof(wchar_t), 0);

    /* get string */
    str = quoted = get_string_without_spacing(node, context->ib);
    
    /* assemble char class */
    if (str && (len = (int) wcslen(str)))
    {
        int span = 1;

        if (str[len-1] == ']')
        {
            str[--len] = 0;
        }

        if (str[0] == '[')
        {
            ++quoted;
            --len;
        }

        /* check for range */
        for (i = 0; i < len; i += span)
        {
            wchar_t first;

            span = 1;

            first = get_escaped_char(quoted, i, len, &span);

            if (i < (len-span) && quoted[i+span] == L'-' && i < (len-(span+1)))
            {
                int next_span = 1;
                wchar_t cur, last = get_escaped_char(quoted, i+span+1, len, &next_span);

                if (first <= last)
                {
                    for (cur = first; cur <= last; ++cur)
                        array_add(&cc, &cur);
                }
                else
                {
                    array_add(&cc, &first);
                    array_add(&cc, &last);
                }

                span += next_span+1;
            }
            else
            {
                array_add(&cc, &first);
            }
        }
    }

    /* terminate and copy */
    array_add(&cc, &ch);
    exp->data.str = wcsdup(cc.data);

    /* clean up and return */
    array_deinit(&cc);
    free(str);

    return exp;
} /* collect_class_exp() */


static rule_exp_t *collect_term_exp(syntax_node_t **children, rule_context *context)
{
    syntax_node_t **cur;
    rule_exp_t *res = 0;
    int hide_next = 0;

    assert(children);

    for (cur = children; *cur && !res; ++cur)
    {
        syntax_node_t *node = *cur;

        switch (node->type)
        {
        case PEG_OPEN_NODE:
        case PEG_CLOSE_NODE:
            break;
        case PEG_IDENT_NODE:
            res = collect_rule_call_exp(node, context);
            break;
        case PEG_LITERAL_NODE:
            res = collect_string_exp(node, context);
            break;
        case PEG_CLASS_NODE:
            res = collect_class_exp(node, context);
            break;
        case PEG_DOT_NODE:
            res = (rule_exp_t *) calloc(1, sizeof(rule_exp_t));
            res->type = RULE_EXP_DOT;
            break;
        case PEG_DISJ_NODE:
            res = collect_rule_exp(node, context);
            break;
        default:
            /* can't happen */
            assert(0);
            break;
        }
    }

    assert(res);

    if (hide_next)
    {
        rule_exp_t *next = (rule_exp_t *) calloc(1, sizeof(rule_exp_t));
        next->type = RULE_EXP_HIDE;
        next->left = res;

        res = next;
    }

    return res;
} /* collect_term_exp() */


static rule_exp_t *collect_rule_exp(syntax_node_t *node, rule_context *context)
{
    assert(node);
    assert(context);

    switch (node->type)
    {
    case PEG_DISJ_NODE:
        return collect_rule_children(node->children, RULE_EXP_DISJ, context);
    case PEG_CONJ_NODE:
        return collect_rule_children(node->children, RULE_EXP_SEQ, context);
    case PEG_PREFIX_EXP_NODE:
        return collect_prefix_exp(node->children, context);
    case PEG_SUFFIX_EXP_NODE:
        return collect_suffix_exp(node->children, context);
    case PEG_TERM_NODE:
        return collect_term_exp(node->children, context);
    default:
        break;
    }

    /* can't happen */
    assert(0);
    return 0;
} /* collect_rule_exp() */


static int collect_rule_data(syntax_node_t *node, void *data)
{
    assert(node);
    assert(data);

    if (node->type == PEG_RULE_NODE)
    {
        rule_context *context = (rule_context *) data;
        rule_rec_t *rec = (rule_rec_t *) calloc(1, sizeof(rule_rec_t));
        syntax_node_t **cur;
       
        /* get rule name */
        rec->node = (syntax_node_t *) node;
        collect_rule_name(rec, node, context->ib);

        /* get rule expression */
        for (cur = node->children; *cur; ++cur)
        {
            syntax_node_t *child = *cur;
            if (child->type == PEG_DISJ_NODE)
            {
                rec->rule_spec = collect_rule_exp(child, context);
                break;
            }
        }

        /* add to the list of rules */
        array_add(context->rule_records, &rec);

        return 1;
    }
    else
    {
        return 0;
    }
} /* collect_rule_data() */


/*************************************************/

static void cleanup_rule_exp(rule_exp_t *exp)
{
    assert(exp);

    switch (exp->type)
    {
    case RULE_EXP_CALL:
    case RULE_EXP_STR:
    case RULE_EXP_CLASS:
        free(exp->data.str);
        break;
    }

    if (exp->left)
        cleanup_rule_exp(exp->left);
    if (exp->right)
        cleanup_rule_exp(exp->right);
    free(exp);
} /* cleanup_rule_exp() */


void cleanup_rule(rule_rec_t *rec)
{
    assert(rec);

    free((wchar_t *) rec->rule_name);
    free((wchar_t *) rec->rule_desc);

    if (rec->rule_spec)
        cleanup_rule_exp((rule_exp_t *) rec->rule_spec);

    free(rec);
} /* cleanup_rule() */

/*************************************************/

static void resolve_rule_calls(array_t *rule_records, array_t *rule_calls, array_t *errors)
{
    int i, j, num_calls, num_rules;

    num_calls = array_size(rule_calls);
    num_rules = array_size(rule_records);

    /* go through the rule calls */
    for (i = 0; i < num_calls; ++i)
    {
        int found = 0;
        rule_rec_t *rec = 0;

        /* find in the rule definitions */
        rule_call *rc = (rule_call *) array_item(rule_calls, i);
        for (j = 0; j < num_rules; ++j)
        {
            rec = *(rule_rec_t **) array_item(rule_records, j);

            if (wcscmp(rc->rule_name, rec->rule_name) == 0)
            {
                found = 1;
                break;
            }
        }

        /* record error if not found */
        if (!found)
        {
            static wchar_t *msg = L"unknown grammar rule '%ls'";
            int len = (int) (wcslen(msg) + wcslen(rc->rule_name) + 32);
            wchar_t *buf = (wchar_t *) calloc(len+1, sizeof(wchar_t));
            swprintf(buf, len, msg, rc->rule_name);
            add_error(errors, rc->node->begin, buf);
            free(buf);
        }
    }
} /* resolve_rule_calls() */

/*************************************************/

static void print_rule_exp(rule_exp_t *exp)
{
    switch (exp->type)
    {
    case RULE_EXP_SEQ:
        fprintf(stdout, "SEQ(");
        print_rule_exp(exp->left);
        fprintf(stdout, ", ");
        print_rule_exp(exp->right);
        fprintf(stdout, ")");
        break;
    case RULE_EXP_DISJ:
        fprintf(stdout, "DISJ(");
        print_rule_exp(exp->left);
        fprintf(stdout, ", ");
        print_rule_exp(exp->right);
        fprintf(stdout, ")");
        break;
    case RULE_EXP_STAR:
        fprintf(stdout, "STAR(");
        print_rule_exp(exp->left);
        fprintf(stdout, ")");
        break;
    case RULE_EXP_PLUS:
        fprintf(stdout, "PLUS(");
        print_rule_exp(exp->left);
        fprintf(stdout, ")");
        break;
    case RULE_EXP_QUES:
        fprintf(stdout, "QUES(");
        print_rule_exp(exp->left);
        fprintf(stdout, ")");
        break;
    case RULE_EXP_BANG:
        fprintf(stdout, "BANG(");
        print_rule_exp(exp->left);
        fprintf(stdout, ")");
        break;
    case RULE_EXP_AMP:
        fprintf(stdout, "AMP(");
        print_rule_exp(exp->left);
        fprintf(stdout, ")");
        break;
    case RULE_EXP_HIDE:
        fprintf(stdout,"HIDE(");
        print_rule_exp(exp->left);
        fprintf(stdout,")");
        break;
    case RULE_EXP_CALL:
        fprintf(stdout, "T('%ls')", exp->data.str);
        break;
    case RULE_EXP_STR:
        fprintf(stdout, "S(\"%ls\")", exp->data.str);
        break;
    case RULE_EXP_DOT:
        fprintf(stdout, "DOT");
        break;
    case RULE_EXP_CLASS:
        fprintf(stdout, "C([%ls])", exp->data.str);
        break;
    }
} /* print_rule_exp() */

void print_rules(array_t *rule_records)
{
    int i, len = array_size(rule_records);
    
    for (i = 0; i < len; ++i)
    {
        rule_rec_t *rec = *(rule_rec_t **) array_item(rule_records, i);

        /* name and description */
        fprintf(stdout, "rule '%ls'", rec->rule_name);
        if (rec->rule_desc && *rec->rule_desc)
            fprintf(stdout, " ('%ls')", rec->rule_desc);
        fprintf(stdout, ": ");

        if (rec->rule_spec)
            print_rule_exp(rec->rule_spec);

        /* end of line */
        fprintf(stdout, "\n");
    }
} /* print_rules() */


/*************************************************/

void get_internal_representation(syntax_node_t **parsed_spec, input_buffer_t *ib, array_t *rule_records, array_t *line_endings, array_t *errors)
{
    array_t rule_calls;   /* array of rule_call */
    rule_context context;

    /* get rule names and expressions */
    if (*parsed_spec)
    {
        array_init(&rule_calls, sizeof(rule_call), 0);

        context.ib = ib;
        context.rule_records = rule_records;
        context.rule_calls = &rule_calls;
        syntax_node_traverse_inorder(*parsed_spec, &context, collect_rule_data);

        /* error check */
        resolve_rule_calls(rule_records, &rule_calls, errors);

        /* clean up */
        array_deinit(&rule_calls);
    }
    else
    {
        add_error(errors, 0, L"no visible nodes");
    }
} /* get_internal_representation() */
