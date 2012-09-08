/*
* $Id: c_generator.c 52 2006-11-08 03:28:22Z kulibali $
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

#include "c_generator.h"
#include "internal.h"
#include "narwhal_utils.h"

#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>

/*************************************************/

#define BUF_LEN 1024

static void to_upper(wchar_t *s)
{
    wchar_t *ch;
    for (ch = s; *ch; ++ch)
    {
        if (*ch >= L'a' && *ch <= L'z')
            *ch -= 32;
        else if (!iswalnum(*ch))
            *ch = L'_';
    }
}

static void to_lower(wchar_t *s)
{
    wchar_t *ch;
    for (ch = s; *ch; ++ch)
    {
        if (*ch >= L'A' && *ch <= L'Z')
            *ch += 32;
        else if (!iswalnum(*ch))
            *ch = L'_';
    }
}

static void get_node_type_labels(const wchar_t *prefix, const array_t *rule_records, array_t *node_type_labels)
{
    wchar_t buf[BUF_LEN], *temp;
    int i, len;

    /* make null first node type */
    swprintf(buf, BUF_LEN, L"%ls_NULL_NODE", prefix);
    to_upper(buf);
    temp = wcsdup(buf);
    array_add(node_type_labels, &temp);

    /* make other node types */
    len = array_size(rule_records);

    for (i = 0; i < len; ++i)
    {
        rule_rec_t *rec = *(rule_rec_t **) array_item(rule_records, i);

        swprintf(buf, BUF_LEN, L"%ls_%ls_NODE", prefix, rec->rule_name);
        to_upper(buf);
        temp = wcsdup(buf);
        array_add(node_type_labels, &temp);
    }
} /* get_node_type_labels() */

static void get_node_function_names(const wchar_t *prefix, const array_t *rule_records, array_t *node_function_names)
{
    wchar_t buf[BUF_LEN], *temp;
    int i, len;

    if (!prefix || !prefix[0])
        prefix = L"GRAMMAR";

    len = array_size(rule_records);

    for (i = 0; i < len; ++i)
    {
        rule_rec_t *rec = *(rule_rec_t **) array_item(rule_records, i);

        swprintf(buf, BUF_LEN, L"parse_%ls_%ls", prefix, rec->rule_name);
        to_lower(buf);
        temp = wcsdup(buf);
        array_add(node_function_names, &temp);
    }
}

/*************************************************/

static void generate_header(const wchar_t *prefix, const char *header_fname, FILE *header_file, const array_t *node_type_labels)
{
    wchar_t buf[BUF_LEN];
    time_t cur_time;
    int i, len;

    /* header guard */
    swprintf(buf, BUF_LEN, L"%ls_%hs", prefix, header_fname);
    to_upper(buf);

    fprintf(header_file, "#ifndef %ls\n", buf);
    fprintf(header_file, "#define %ls\n", buf);
    fprintf(header_file, "\n");

    /* timestamp */
    time(&cur_time);
    fprintf(header_file, "/*\n * generated %s */\n", ctime(&cur_time));
    fprintf(header_file, "\n");

    fprintf(header_file, "#ifdef WIN32\n#pragma warning(disable : 4996)\n#define _CRT_SECURE_NO_DEPRECATE\n#define snprintf _snprintf\n#endif\n\n");


    fprintf(header_file, "#include <wchar.h>\n\n");

    fprintf(header_file, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n");

    /* type enum */
    fprintf(header_file, "/* node types in the abstract syntax tree */\n\n");

    swprintf(buf, BUF_LEN, L"%ls_node_type", prefix);
    to_lower(buf);
    fprintf(header_file, "enum %ls \n{\n", buf);
    len = array_size(node_type_labels);
    for (i = 0; i < len; ++i)
    {
        wchar_t *type = *(wchar_t **) array_item(node_type_labels, i);
        fprintf(header_file, "\t%ls = %d,", type, i);
        fprintf(header_file, "\n");
    }
    swprintf(buf, BUF_LEN, L"%ls", prefix);
    to_upper(buf);
    fprintf(header_file, "\t%ls_NUM_NODE_TYPES = %d\n", buf, len);
    fprintf(header_file, "};\n\n");
   

    swprintf(buf, BUF_LEN, L"%ls_node_names", prefix);
    to_lower(buf);
    fprintf(header_file, "extern const char *%ls[%d];\n",buf,len);



    /* syntax tree types */
    wcscpy(buf, prefix);
    to_lower(buf);

    fprintf(header_file, "/* nodes in the abstract syntax tree */\n\n");
    fprintf(header_file, "typedef struct _%ls_syntax_node_t\n"
        "{\n"
        "    int type;                  /* type of node; this is defined above.                    */\n"
        "    int begin;                 /* input position before the first character of the match  */\n"
        "    int end;                   /* input position after the last character of the match    */\n"
        "    int first_line;            /* line on which the match begins                          */\n"
        "    int last_line;             /* line on which the match ends                            */\n"
	"    int children;            /* number of children*/                                      \n"
        "    struct _%ls_syntax_node_t **child; /* null-terminated array of child nodes                    */\n"
        "    void *data;\n"
		"    void *ib; /*Pointer to text buffer*/\n"
        "}\n"
        "%ls_syntax_node_t;\n\n", buf, buf, buf);

    fprintf(header_file, "extern %ls_syntax_node_t *%ls_syntax_node_create(int type, int begin, int end, void* ib);\n", buf, buf);
    fprintf(header_file, "extern %ls_syntax_node_t *%ls_syntax_node_copy(%ls_syntax_node_t *node);\n", buf, buf, buf);
    fprintf(header_file, "extern void %ls_syntax_node_destroy(%ls_syntax_node_t *node);\n", buf, buf);
    fprintf(header_file, "extern int %ls_syntax_node_children(%ls_syntax_node_t *node); /*Returns number of children this node has*/\n", buf, buf);
    fprintf(header_file, "extern %ls_syntax_node_t *%ls_syntax_node_child(%ls_syntax_node_t *node,int idx);/*Returns child indicated by idx*/\n", buf, buf, buf);
    fprintf(header_file, "typedef int (*%ls_syntax_node_process_ft)(%ls_syntax_node_t *node, void *data);\n", buf, buf);
    fprintf(header_file, "extern void %ls_syntax_node_traverse_preorder(%ls_syntax_node_t *root, void *data, %ls_syntax_node_process_ft entry_func,%ls_syntax_node_process_ft exit_func);\n", buf, buf, buf, buf);
   fprintf(header_file, "extern  %ls_syntax_node_process_ft %ls_dispatch[%d];\n\n", buf, buf, len);

    /* error handling */
    fprintf(header_file, "/* error handling */\n\n");
    fprintf(header_file, "typedef struct _%ls_error_rec_t\n"
        "{\n"
        "    int pos;\n"
        "    wchar_t *str;\n"
        "}\n"
        "%ls_error_rec_t;\n\n", buf, buf);
    fprintf(header_file, "extern int %ls_wish_node;\n",buf);

    fprintf(header_file, "extern void %ls_destroy_error_list(void *error_list); /* free a list of errors */\n", buf);
    fprintf(header_file, "extern void %ls_add_error(void *error_list, int pos, wchar_t *str); /* add an error to the list of errors */\n", buf);
    fprintf(header_file, "extern int %ls_num_errors(void *error_list); /* returns the number of errors in the list */\n", buf);
    fprintf(header_file, "extern %ls_error_rec_t *%ls_get_error(void *error_list, int index);\n\n", buf, buf);

    /* main function */
    fprintf(header_file, "/* input buffers */\n\n");
    fprintf(header_file, "extern wchar_t *%ls_get_wstr(%ls_syntax_node_t *node);\n", buf, buf);
    fprintf(header_file, "extern char *%ls_get_str(%ls_syntax_node_t *node);\n", buf, buf);
    fprintf(header_file, "extern void %ls_destroy_input_buffer(void *ib);\n\n", buf);

    fprintf(header_file, "/* main parse function */\n\n");
    fprintf(header_file, "extern int %ls_parse(char *fname, %ls_syntax_node_t **parse_tree, void **input_buffer, void **error_list);\n\n", buf, buf);

    /* end guard */
    fprintf(header_file, "#ifdef __cplusplus\n}\n#endif\n\n");
    fprintf(header_file, "#endif\n");
} /* generate_header() */

/*************************************************/

static void print_utility_source(const wchar_t *prefix, FILE *src_file,const array_t* node_type_labels)
{
    wchar_t buf[BUF_LEN];
    wchar_t cbuf[BUF_LEN];
int len,i;
    len = array_size(node_type_labels);
    swprintf(buf, BUF_LEN, L"%ls_node_names", prefix);
    to_lower(buf);
    fprintf(src_file, "const char *%ls[%d] = {\n",buf,len);
    for (i = 0; i < len; ++i)
    {
        wchar_t *type = *(wchar_t **) array_item(node_type_labels, i);
        fprintf(src_file, "\"%ls\",", type);
        fprintf(src_file, "\n");
    }
    fprintf(src_file, "};\n\n");
    
    swprintf(buf, BUF_LEN, L"%ls", prefix);
    to_lower(buf);

    fprintf(src_file, "%ls_syntax_node_process_ft %ls_dispatch[%d] = {\n",buf,buf,len);
    for (i = 0; i < len; ++i)
    {
        fprintf(src_file, "NULL,\n");
    }
    fprintf(src_file, "};\n\n");

    fprintf(src_file, "wchar_t *wcs_dup(wchar_t *str)\n"
	"{\n"
	"    wchar_t *res;\n"
	"    int len = (int) wcslen(str) + 1;\n"
	"\n"
	"    res = (wchar_t *) calloc(len, sizeof(wchar_t));\n"
	"    memcpy(res, str, len * sizeof(wchar_t));\n"
	"\n"
	"    return res;\n"
        "} /* wcs_dup() */\n");

    fprintf(src_file, "char *str_dup(char *str)\n"
	"{\n"
	"    char *res;\n"
	"    int len = (int) strlen(str) + 1;\n"
	"\n"
	"    res = (char *) calloc(len, sizeof(char));\n"
	"    memcpy(res, str, len * sizeof(char));\n"
	"\n"
	"    return res;\n"
        "} /* str_dup() */\n");

    
    /* dynamic array functions */
    fprintf(src_file, "/* dynamic arrays */\n\n");
    fprintf(src_file, "typedef struct _array_t\n"
        "{\n"
        "    int data_size;\n"
        "    int num, cap;\n"
        "    void *data;\n"
        "}\n"
        "array_t;\n\n");

    fprintf(src_file, "static void array_init(array_t *da, int data_size, int num_items)\n"
        "{\n"
        "    assert(da);\n"
        "\n"
        "    da->data_size = data_size;\n"
        "    da->num       = num_items;\n"
        "    da->cap       = num_items;\n"
        "\n"
        "    if (num_items)\n"
        "        da->data  = calloc(num_items, data_size);\n"
        "    else\n"
        "        da->data  = 0;\n"
        "} /* array_init() */\n\n");

    fprintf(src_file, "static int array_size(array_t *da)\n"
        "{\n"
        "    assert(da);\n"
        "\n"
        "    return da->num;\n"
        "} /* array_size() */\n\n");

    fprintf(src_file, "static void *array_item(array_t *da, int index)\n"
        "{\n"
        "    char *cp;\n"
        "\n"
        "    assert(da);\n"
        "    assert(index < da->num);\n"
        "\n"
        "    cp = (char *) da->data;\n"
        "    return cp + (da->data_size * index);\n"
        "} /* array_item() */\n\n");

    fprintf(src_file, "static void array_resize(array_t *da, int num_items)\n"
        "{\n"
        "    assert(da);\n"
        "    assert(num_items >= 0);\n"
        "\n"
        "    if (num_items >= da->cap)\n"
        "    {\n"
        "        da->cap = num_items * 3 / 2 + 1;\n"
        "\n"
        "        if (da->data)\n"
        "            da->data = realloc(da->data, da->cap * da->data_size);\n"
        "        else\n"
        "            da->data = calloc(da->cap, da->data_size);\n"
        "    }\n"
        "\n"
        "    da->num = num_items;\n"
        "} /* array_resize() */\n\n");

    fprintf(src_file, "static void array_add(array_t *da, void *item)\n"
        "{\n"
        "    char *cp;\n"
        "    int new_index;\n"
        "\n"
        "    assert(da);\n"
        "\n"
        "    new_index = da->num;\n"
        "\n"
        "    if (da->num == da->cap)\n"
        "        array_resize(da, da->num + 1);\n"
        "    else\n"
        "        da->num++;\n"
        "\n"
        "    cp = (char *) da->data;\n"
        "    memcpy(cp + (da->data_size * new_index), item, da->data_size);\n"
        "} /* array_add() */\n\n");

    fprintf(src_file, "static void array_deinit(array_t *da)\n"
        "{\n"
        "    assert(da);\n"
        "\n"
        "    free(da->data);\n"
        "    da->data = 0;\n"
        "    da->cap = da->num = 0;\n"
        "} /* array_destroy() */\n\n");

    fprintf(src_file, "static void array_copy(array_t *dest, array_t *src)\n"
        "{\n"
        "    assert(dest);\n"
        "    assert(src);\n"
        "    assert(dest->data_size == src->data_size);\n"
        "\n"
        "    array_deinit(dest);\n"
        "    array_init(dest, dest->data_size, src->num);\n"
        "    memcpy(dest->data, src->data, src->num * src->data_size);\n"
        "} /* array_copy() */\n\n");

    fprintf(src_file, "static void array_clear(array_t *da)\n"
        "{\n"
        "    assert(da);\n"
        "\n"
        "    da->num = 0;\n"
        "} /* array_clear() */\n\n");

    /* syntax node functions */
    swprintf(buf, BUF_LEN, L"%ls", prefix);
    to_lower(buf);
    swprintf(cbuf, BUF_LEN, L"%ls", prefix);
    to_upper(cbuf);

    fprintf(src_file, "/* syntax node functions */\n\n");
    fprintf(src_file, "%ls_syntax_node_t *%ls_syntax_node_create(int type, int begin, int end,void* ib)\n"
        "{\n"
        "    %ls_syntax_node_t *res = (%ls_syntax_node_t *) calloc(1, sizeof(%ls_syntax_node_t));\n"
        "    res->type  = type;\n"
        "    res->begin = begin;\n"
        "    res->end   = end;\n"
        "    res->first_line = -1;\n"
        "    res->last_line  = -1;\n"
        "    res->child = 0;\n"
	"    res->children = 0;\n"
	"    res->ib = ib;\n"
        "    return res;\n"
        "} /* syntax_node_create() */\n\n", buf, buf, buf, buf, buf);

    fprintf(src_file, "%ls_syntax_node_t *%ls_syntax_node_copy(%ls_syntax_node_t *node)\n"
        "{\n"
        "    %ls_syntax_node_t *copy = 0, **cur;\n"
        "    int i, len;\n"
        "\n"
        "    if (node)\n"
        "    {\n"
        "        copy = %ls_syntax_node_create(node->type, node->begin, node->end, node->ib);\n"
        "        copy->first_line = node->first_line;\n"
        "        copy->last_line = node->last_line;\n"
        "\n", buf, buf, buf, buf, buf);

    fprintf(src_file, 
        "        len = node->children;\n"
        "        copy->children = node->children;\n"  
        "        if (len)\n"
        "        {\n"
        "            copy->child = (%ls_syntax_node_t **) calloc(len+1, sizeof(%ls_syntax_node_t *));\n"
        "            for (i = 0; i < len; ++i)\n"
        "                copy->child[i] = %ls_syntax_node_copy(node->child[i]);\n"
        "        }\n"
        "    }\n"
        "\n"
        "    return copy;\n"
        "} /* syntax_node_copy() */\n\n", buf, buf, buf);

    fprintf(src_file, "void %ls_syntax_node_destroy(%ls_syntax_node_t *node)\n"
        "{\n"
        "    %ls_syntax_node_t **cur;\n"
        "\n"
        "    assert(node);\n"
        "\n"
        "    if (node->child)\n"
        "        for (cur = node->child; *cur; ++cur)\n"
        "            %ls_syntax_node_destroy(*cur);\n"
        "\n"
        "    free(node);\n"
        "} /* syntax_node_destroy() */\n\n", buf, buf, buf, buf);


    fprintf(src_file, "void %ls_syntax_node_traverse_preorder(%ls_syntax_node_t *root, void *data, %ls_syntax_node_process_ft entry_func, %ls_syntax_node_process_ft exit_func)\n"
        "{\n"
        "    if (root)\n"
        "    {\n"
        "        %ls_syntax_node_t **cur;\n"
        "        if (entry_func != NULL)\n"
        "          if (entry_func(root, data))\n"
        "            return;\n"
        "\n"
        "        if (root->child)\n"
        "            for (cur = root->child; *cur; ++cur)\n"
        "                %ls_syntax_node_traverse_preorder(*cur, data, entry_func, exit_func);\n"
		"        if (exit_func != NULL)\n"
		"          exit_func(root,data);\n"
        "    }\n"
        "} /* syntax_node_traverse_inorder() */\n\n", buf, buf, buf, buf, buf, buf);

    /* error handling functions */
    fprintf(src_file, "/* error handling functions */\n\n");
    fprintf(src_file, "void *%ls_create_error_list()\n"
        "{\n"
        "    array_t *ptr = (array_t *) calloc(1, sizeof(array_t));\n"
        "    array_init(ptr, sizeof(%ls_error_rec_t), 0);\n"
        "    return ptr;\n"
        "}\n\n", buf, buf);

    fprintf(src_file, "void %ls_destroy_error_list(void *error_list)\n"
        "{\n"
        "    int i, len = array_size((array_t *) error_list);\n"
        "    for (i = 0; i < len; ++i)\n"
        "    {\n"
        "        %ls_error_rec_t *er = (%ls_error_rec_t *) array_item((array_t *) error_list, i);\n"
        "        free(er->str);\n"
        "    }\n"
        "    array_deinit((array_t *) error_list);\n"
        "    free((array_t *) error_list);\n"
        "}\n\n", buf, buf, buf);

    fprintf(src_file, "void %ls_add_error(void *error_list, int pos, wchar_t *str)\n"
        "{\n"
        "    %ls_error_rec_t er;\n"
        "    er.pos = pos;\n"
        "    er.str = wcs_dup(str);\n"
        "    array_add((array_t *) error_list, &er);\n"
        "}\n\n", buf, buf);

    fprintf(src_file, "int %ls_num_errors(void *error_list)\n"
        "{\n"
        "    return array_size((array_t *) error_list);\n"
        "}\n\n", buf);

    fprintf(src_file, "%ls_error_rec_t *%ls_get_error(void *error_list, int index)\n"
        "{\n"
        "    return (%ls_error_rec_t *) array_item((array_t *) error_list, index);\n"
        "}\n\n", buf, buf, buf);

    fprintf(src_file, "static void delete_errors(array_t *errors, int start_index)\n"
"{\n"
"    int i, len = array_size(errors);\n"
"    for (i = start_index; i < len; ++i)\n"
"    {\n"
"        %ls_error_rec_t *rec = (%ls_error_rec_t *) array_item(errors, i);\n"
"        free(rec->str);\n"
"    }\n"
"    errors->num = start_index;\n"
"}\n\n", buf, buf);

    fprintf(src_file, "static void add_error(array_t *errs, int pos, wchar_t *str)\n"
"{\n"
"    %ls_error_rec_t rec;\n"
"    rec.pos = pos;\n"
"    rec.str = wcs_dup(str);\n"
"\n"
"    array_add(errs, &rec);\n"
"} /* add_error() */\n\n", buf);

    fprintf(src_file, "static void dump_errors(array_t *errs){\n"
		    "int cnt,max;\n"
		    "%ls_error_rec_t *this_error;\n"
		    "\n"
		    "for (cnt = %ls_num_errors(errs)-1;cnt >= 0; cnt--){\n"
		    "  this_error = %ls_get_error(errs,cnt);\n"
		    "  printf(\"WishError %%d:%%d %%ls\\n\",cnt,this_error->pos,this_error->str);\n"
		    "}\n"
		    "}\n"
		    "/*Debug dump of errors to stdout*/\n",buf,buf,buf);



    /* input buffer functions */
    fprintf(src_file, "/* input buffers */\n\n");
    fprintf(src_file, "enum input_buffer_flags_et\n"
        "    {\n"
        "        INPUT_BUFFER_NULL_FLAGS       = 0,\n"
        "        INPUT_BUFFER_UNICODE_BOM_READ = 1\n"
        "    };\n"
        "\n"
        "    enum input_buffer_unicode_mode_et\n"
        "    {\n"
        "        INPUT_BUFFER_ANSI     = 0,\n"
        "        INPUT_BUFFER_UTF8     = 1,\n"
        "        INPUT_BUFFER_UTF16_LE = 2,\n"
        "        INPUT_BUFFER_UTF16_BE = 3,\n"
        "        INPUT_BUFFER_UTF32_LE = 4,\n"
        "        INPUT_BUFFER_UTF32_BE = 5\n"
        "    };\n"
        "\n"
        "    typedef struct _input_buffer_t\n"
        "    {\n"
        "        char *name;\n"
        "        FILE *f;\n"
        "        char *buf;\n"
        "        int buf_size;\n"
        "        int bytes_read;\n"
        "        int current_pos;\n"
        "\n"
        "        unsigned int flags;\n"
        "        int unicode_mode;\n"
        "    }\n"
        "    input_buffer_t;\n\n");

    fprintf(src_file, "static void input_buffer_get_unicode_mode(input_buffer_t *ib)\n"
        "{\n"
        "    assert(ib);\n"
        "\n"
        "    ib->unicode_mode = INPUT_BUFFER_ANSI;\n"
        "    ib->flags |= INPUT_BUFFER_UNICODE_BOM_READ;\n"
        "} /* input_buffer_get_unicode_mode() */\n\n");

    fprintf(src_file, "static input_buffer_t *input_buffer_create(char *name, FILE *f)\n"
        "{\n"
        "    input_buffer_t *ib;\n"
        "\n"
        "    assert(f);\n"
        "\n"
        "    ib = (input_buffer_t *) calloc(1, sizeof(input_buffer_t));\n"
        "    ib->name = str_dup(name);\n"
        "    ib->f = f;\n"
        "\n"
        "    input_buffer_get_unicode_mode(ib);\n"
        "\n"
        "    return ib;\n"
        "} /* input_buffer_create() */\n\n");

    fprintf(src_file, "static void input_buffer_destroy(input_buffer_t *ib)\n"
        "{\n"
        "    assert(ib);\n"
        "\n"
        "    fclose(ib->f);\n"
        "    free(ib->name);\n"
        "    free(ib->buf);\n"
        "    free(ib);\n"
        "} /* input_buffer_destroy() */\n\n");

    fprintf(src_file, "static void input_buffer_setpos(input_buffer_t *ib, int pos)\n"
        "{\n"
        "    assert(ib);\n"
        "    assert(pos >= 0);\n"
        "\n"
        "    ib->current_pos = pos;\n"
        "} /* input_buffer_setpos() */\n\n");

    fprintf(src_file, "static int input_buffer_getpos(input_buffer_t *ib)\n"
        "{\n"
        "    assert(ib);\n"
        "\n"
        "    return ib->current_pos;\n"
        "} /* input_buffer_getpos() */\n\n");

    fprintf(src_file, "#define INPUT_BUFFER_SIZE_INCREMENT 4096\n\n");

    fprintf(src_file, "static wchar_t input_buffer_read_char(input_buffer_t *ib)\n"
        "{\n"
        "    wchar_t ch;\n"
        "    int prev_pos, new_pos;\n"
        "\n"
        "    assert(ib);\n"
        "    \n"
        "    /* resize buffer if necessary, and read */\n"
        "    while (ib->current_pos >= ib->bytes_read)\n"
        "    {\n"
        "        size_t num_bytes_read;\n"
        "\n"
        "        ib->buf = (char *) realloc(ib->buf, ib->buf_size += INPUT_BUFFER_SIZE_INCREMENT);\n"
        "\n"
        "        if (!ib->buf)\n"
        "        {\n"
        "            return WEOF;\n"
        "            ib->buf_size -= INPUT_BUFFER_SIZE_INCREMENT;\n"
        "        }\n"
        "\n"
        "        num_bytes_read = fread(ib->buf + ib->bytes_read, 1, INPUT_BUFFER_SIZE_INCREMENT, ib->f);\n"
        "\n"
        "        if (num_bytes_read)\n"
        "        {\n"
        "            ib->bytes_read += (int) num_bytes_read;\n"
        "        }\n"
        "        else\n"
        "        {\n"
        "            return WEOF;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    /* get next code point and check for eol */\n"
        "    prev_pos = ib->current_pos;\n"
        "\n"
        "    ch = (wchar_t) ib->buf[ib->current_pos++]; /* just for now */\n"
        "\n"
        "    new_pos = ib->current_pos;\n"
        "\n"
        "    return ch;\n"
        "} /* input_buffer_read_char() */\n\n");

    fprintf(src_file, "static void input_buffer_read_wstring(input_buffer_t *ib, int begin, int end, array_t *str)\n"
        "{\n"
        "    wchar_t ch = WEOF;\n"
        "\n"
        "    assert(ib);\n"
        "    assert(str);\n"
        "\n"
        "    array_clear(str);\n"
        "    ib->current_pos = begin;\n"
        "    while (ib->current_pos < end)\n"
        "    {\n"
        "        ch = input_buffer_read_char(ib);\n"
        "\n"
        "        if (ch != WEOF)\n"
        "            array_add(str, &ch);\n"
        "    }\n"
        "\n"
        "    ch = 0;\n"
        "    array_add(str, &ch);\n"
        "} /* input_buffer_read_string() */\n\n");


    fprintf(src_file, "static void input_buffer_read_string(input_buffer_t *ib, int begin, int end, array_t *str)\n"
        "{\n"
        "    char ch = WEOF;\n"
        "\n"
        "    assert(ib);\n"
        "    assert(str);\n"
        "\n"
        "    array_clear(str);\n"
        "    ib->current_pos = begin;\n"
        "    while (ib->current_pos < end)\n"
        "    {\n"
        "        ch = (char)wctob(input_buffer_read_char(ib));\n"
        "\n"
        "        if (ch != WEOF)\n"
        "            array_add(str, &ch);\n"
        "    }\n"
        "\n"
        "    ch = 0;\n"
        "    array_add(str, &ch);\n"
        "} /* input_buffer_read_string() */\n\n");

    /* external input buffer functions */
    fprintf(src_file, "/* external input buffer functions */\n");
    fprintf(src_file, "wchar_t *%ls_get_wstr(%ls_syntax_node_t *node)\n"
        "{\n"
        "    array_t str;\n"
        "    wchar_t *res;\n"
		"    void *ib;\n"
        "\n"
		"    ib = node->ib;\n"
        "    array_init(&str, sizeof(wchar_t), 0);\n"
        "    input_buffer_read_wstring((input_buffer_t *) ib, node->begin, node->end, &str);\n"
        "    res = wcs_dup(str.data);\n"
        "    return res;\n"
        "}\n\n", buf, buf);
    fprintf(src_file, "char *%ls_get_str(%ls_syntax_node_t *node)\n"
        "{\n"
        "    array_t str;\n"
        "    char *res;\n"
		"    void *ib;\n"
        "\n"
		"    ib = node->ib;\n"
        "    array_init(&str, sizeof(char), 0);\n"
        "    input_buffer_read_string((input_buffer_t *) ib, node->begin, node->end, &str);\n"
        "    res = str_dup(str.data);\n"
        "    return res;\n"
        "}\n\n", buf, buf);


    fprintf(src_file, "void %ls_destroy_input_buffer(void *ib)\n"
        "{\n"
        "    input_buffer_destroy((input_buffer_t *) ib);\n"
        "}\n\n", buf);
    

    /* memory map functions */
    fprintf(src_file, "/* memo map functions */\n\n");
    fprintf(src_file, "typedef struct _memo_rec_t\n"
        "{\n"
        "    int start_offset, end_offset;\n"
        "    %ls_syntax_node_t *parse_tree;\n"
        "}\n"
        "memo_rec_t;\n\n", buf);

    fprintf(src_file, "typedef struct _memo_map_t\n"
        "{\n"
        "    array_t records[%ls_NUM_NODE_TYPES]; /* a array_t of memo_rec_t structures */\n"
        "}\n"
        "memo_map_t;\n\n", cbuf);

    fprintf(src_file, "static memo_map_t *memo_map_create()\n"
        "{\n"
        "    int i;\n"
        "    memo_map_t *map;\n"
        "    \n"
        "    map = (memo_map_t *) calloc(1, sizeof(memo_map_t));\n"
        "\n"
        "    for (i = 0; i < %ls_NUM_NODE_TYPES; ++i)\n"
        "    {\n"
        "        array_init(&map->records[i], sizeof(memo_rec_t), 0);\n"
        "    }\n"
        "\n"
        "    return map;\n"
        "} /* memo_map_create() */\n\n", cbuf);

    fprintf(src_file, "static void memo_map_destroy(memo_map_t *map)\n"
        "{\n"
        "    int i;\n"
        "\n"
        "    assert(map);\n"
        "\n"
        "    for (i = 0; i < %ls_NUM_NODE_TYPES; ++i)\n"
        "    {\n"
        "        int j, count = array_size(&map->records[i]);\n"
        "        for (j = 0; j < count; ++j)\n"
        "        {\n"
        "            memo_rec_t *mr = (memo_rec_t *) array_item(&map->records[i], j);\n"
        "            if (mr->parse_tree)\n"
        "                %ls_syntax_node_destroy(mr->parse_tree);\n"
        "        }\n"
        "        array_deinit(&map->records[i]);\n"
        "    }\n"
        "\n"
        "    free(map);\n"
        "} /* memo_map_destroy() */\n\n", cbuf, buf);

    fprintf(src_file, "static int is_memoized(memo_map_t *map, int type, int start_offset, %ls_syntax_node_t **res, int *end_offset)\n"
        "{\n"
        "    int i, len;\n"
        "    array_t *da;\n"
        "\n"
        "    assert(map);\n"
        "    assert(type < %ls_NUM_NODE_TYPES);\n"
        "\n"
        "    da = &map->records[type];\n"
        "    len = array_size(da);\n"
        "\n"
        "    for (i = 0; i < len; ++i)\n"
        "    {\n"
        "        memo_rec_t *rec = (memo_rec_t *) array_item(da, i);\n"
        "\n"
        "        if (rec->start_offset == start_offset)\n"
        "        {\n"
        "            *res = %ls_syntax_node_copy(rec->parse_tree);\n"
        "            *end_offset = rec->end_offset;\n"
        "            return 1;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    return 0;\n"
        "} /* is_memoized() */\n\n", buf, cbuf, buf);

    fprintf(src_file, "static void memoize(memo_map_t *map, int type, int start_offset, int end_offset, %ls_syntax_node_t *node)\n"
        "{\n"
        "    memo_rec_t rec;\n"
        "\n"
        "    assert(map);\n"
        "    assert(type < %ls_NUM_NODE_TYPES);\n"
        "\n"
        "    rec.start_offset = start_offset;\n"
        "    rec.end_offset = end_offset;\n"
        "    rec.parse_tree = %ls_syntax_node_copy(node);\n"
        "\n"
        "    array_add(&map->records[type], &rec);\n"
        "} /* memoize() */\n\n", buf, cbuf, buf);

    fprintf(src_file, "static void delete_children(array_t *children, int start_index)\n"
        "{\n"
        "    int i, len;\n"
        "    \n"
        "    assert(children);\n"
        "    \n"
        "    len = array_size(children);\n"
        "\n"
        "    for (i = start_index; i < len; ++i)\n"
        "    {\n"
        "        %ls_syntax_node_t *child = *(%ls_syntax_node_t **) array_item(children, i);\n"
        "        %ls_syntax_node_destroy(child);\n"
        "    }\n"
        "\n"
        "    children->num = start_index;\n"
        "} /* delete_children() */\n\n", buf, buf, buf);

    /* line number utilities */
    fprintf(src_file, "static void find_line_endings(array_t *end_offsets, input_buffer_t *ib, int start_pos)\n"
"{\n"
"    wchar_t ch, eol_pos = 0;\n"
"    int pos;\n"
"\n"
"    input_buffer_setpos(ib, start_pos);\n"
"\n"
"    do\n"
"    {\n"
"        ch = input_buffer_read_char(ib);\n"
"\n"
"        if (ch == L'\\n')\n"
"        {\n"
"            pos = input_buffer_getpos(ib);\n"
"            array_add(end_offsets, &pos);\n"
"            eol_pos = 0;\n"
"        }\n"
"        else if (ch == L'\\r')\n"
"        {\n"
"            eol_pos = input_buffer_getpos(ib);\n"
"        }\n"
"        else\n"
"        {\n"
"            if (eol_pos > 0)\n"
"            {\n"
"                pos = eol_pos;\n"
"                array_add(end_offsets, &pos);\n"
"            }\n"
"\n"
"            eol_pos = 0;\n"
"        }\n"
"    }\n"
"    while (ch != WEOF);\n"
"\n"
"    pos = input_buffer_getpos(ib);\n"
"    array_add(end_offsets, &pos);\n"
"} /* find_line_lengths() */\n"
"\n"
"static int find_line(int pos, array_t *line_endings)\n"
"{\n"
"    int n, i, len = array_size(line_endings);\n"
"    for (i = 0; i < len; ++i)\n"
"    {\n"
"        n = * (int *) array_item(line_endings, i);\n"
"        if (pos < n)\n"
"            return i+1;\n"
"    }\n"
"\n"
"    if (pos == n)\n"
"        return len;\n"
"    else\n"
"        return -1;\n"
"} /* find_line() */\n"
"\n"
"static int assign_line_number(%ls_syntax_node_t *node, void *data)\n"
"{\n"
"    array_t *line_endings = (array_t *) data;\n"
"\n"
"    if (node->begin >= node->end)\n"
"    {\n"
"        node->first_line = node->last_line = find_line(node->begin, line_endings);\n"
"    }\n"
"    else\n"
"    {\n"
"        node->first_line = find_line(node->begin, line_endings);\n"
"        node->last_line = find_line(node->end-1, line_endings);\n"
"    }\n"
"\n"
"    return 0;\n"
"} /* assign_line_number() */\n"
"\n"
"static void assign_line_numbers(input_buffer_t *ib, %ls_syntax_node_t *node, array_t *line_endings)\n"
"{\n"
"    /* assign line numbers to the nodes */\n"
"    %ls_syntax_node_traverse_preorder(node, line_endings, assign_line_number,NULL);\n"
"} /* assign_line_numbers() */\n", buf, buf, buf);

} /* print_utility_source() */

static void print_function_prototypes(const wchar_t *prefix, FILE *src_file, const array_t *node_function_names)
{
    wchar_t pbuf[128];
    int i, len;

    swprintf(pbuf, 128, L"%ls", prefix);
    to_lower(pbuf);

    fprintf(src_file, "/* function prototypes */\n\n");

    len = array_size(node_function_names);
    for (i = 0; i < len; ++i)
    {
        fprintf(src_file, "static int %ls(input_buffer_t *ib, int start_offset, int *end_ofset, %ls_syntax_node_t **node, memo_map_t *map, array_t *error_stack);\n", 
            *(wchar_t **) array_item(node_function_names, i), pbuf);
    }

    fprintf(src_file, "\n");
} /* print_function_prototypes() */

static void print_macros(const wchar_t *prefix, FILE *src_file)
{
    wchar_t pbuf[128];

    swprintf(pbuf, 128, L"%ls", prefix);
    to_lower(pbuf);

    fprintf(src_file, "/* parsing macros */\n\n");

    fprintf(src_file, "#define SEQ(A, B)                           \\\n"
        "{                                           \\\n"
        "    int orig_stack_size = child_stack.num;  \\\n"
        "                                            \\\n"
        "    A;                                      \\\n"
        "                                            \\\n"
        "    if (res)                                \\\n"
        "    {                                       \\\n"
        "        B;                                  \\\n"
        "    }                                       \\\n"
        "                                            \\\n"
        "    if (res)                                \\\n"
        "        cur_start_pos = cur_end_pos;        \\\n"
        "    else                                    \\\n"
        "        delete_children(&child_stack, orig_stack_size);   \\\n"
        "                                            \\\n"
        "}\n\n");

    fprintf(src_file, "#define DISJ(A, B)                          \\\n"
        "{                                           \\\n"
        "    int orig_stack_size = child_stack.num;  \\\n"
        "    int orig_start_pos = cur_start_pos;     \\\n"
        "                                            \\\n"
        "    A;                                      \\\n"
        "                                            \\\n"
        "    if (!res)                               \\\n"
        "    {                                       \\\n"
        "        delete_errors(error_stack, 0);      \\\n"
        "        cur_start_pos = orig_start_pos;     \\\n"
        "                                            \\\n"
        "        B;                                  \\\n"
        "    }                                       \\\n"
        "                                            \\\n"
        "    if (res)                                \\\n"
        "        cur_start_pos = cur_end_pos;        \\\n"
        "    else                                    \\\n"
        "        delete_children(&child_stack, orig_stack_size);   \\\n"
        "}\n\n");

    fprintf(src_file, "#define STAR(A)                             \\\n"
        "{                                           \\\n"
        "                                            \\\n"
        "    do                                      \\\n"
        "    {                                       \\\n"
        "        A;                                  \\\n"
        "                                            \\\n"
        "        if (res)                            \\\n"
        "            cur_start_pos = cur_end_pos;    \\\n"
        "    }                                       \\\n"
        "    while(res);                             \\\n"
        "    res = 1;                                \\\n"
        "                                            \\\n"
        "    delete_errors(error_stack, 0);          \\\n"
        "}\n\n");

    fprintf(src_file, "#define PLUS(A)                             \\\n"
        "{                                           \\\n"
        "    int orig_stack_size = child_stack.num;  \\\n"
        "    int count = 0;                          \\\n"
        "    int error_stack_size = error_stack->num; \\\n"
        "                                            \\\n"
        "    do                                      \\\n"
        "    {                                       \\\n"
        "        A;                                  \\\n"
        "                                            \\\n"
        "        if (res)                            \\\n"
        "        {                                   \\\n"
        "            count++;                        \\\n"
        "            cur_start_pos = cur_end_pos;    \\\n"
        "        }                                   \\\n"
        "    }                                       \\\n"
        "    while (res);                            \\\n"
        "    res = count > 0;                        \\\n"
        "                                            \\\n"
        "    if (res)                                \\\n"
        "        delete_errors(error_stack, error_stack_size); \\\n"
        "    else                                    \\\n"
        "        delete_children(&child_stack, orig_stack_size); \\\n"
        "}\n\n");

    fprintf(src_file, "#define QUES(A)                             \\\n"
        "{                                           \\\n"
        "    int error_stack_size = error_stack->num; \\\n"
        "                                            \\\n"
        "    A;                                      \\\n"
        "                                            \\\n"
        "    if (res)                                \\\n"
        "        cur_start_pos = cur_end_pos;        \\\n"
        "                                            \\\n"
        "    res = 1;                                \\\n"
        "    delete_errors(error_stack, error_stack_size); \\\n"
        "}\n\n");

    fprintf(src_file, "#define HIDE(A)                                     \\\n"
        "{                                                   \\\n"
        "    int orig_stack_size = child_stack.num;          \\\n"
        "                                                    \\\n"
        "    A;                                              \\\n"
        "                                                    \\\n"
        "    delete_children(&child_stack, orig_stack_size); \\\n"
        "    cur_start_pos = cur_end_pos;                    \\\n"
        "}\n\n");



    fprintf(src_file, "#define BANG(A)                             \\\n"
        "{                                           \\\n"
        "    int orig_start_pos = cur_start_pos;     \\\n"
        "    int orig_stack_size = child_stack.num;  \\\n"
        "    int orig_error_size = error_stack->num; \\\n"
        "                                            \\\n"
        "    A;                                      \\\n"
        "                                            \\\n"
        "    res = !res;                             \\\n"
        "                                            \\\n"
        "    cur_start_pos = cur_end_pos = orig_start_pos;         \\\n"
        "                                            \\\n"
        "    delete_children(&child_stack, orig_stack_size); \\\n"
        "    delete_errors(error_stack, orig_error_size); \\\n"
        "}\n\n");

    fprintf(src_file, "#define AMP(A)                              \\\n"
        "{                                           \\\n"
        "    int orig_start_pos = cur_start_pos;     \\\n"
        "                                            \\\n"
        "                                            \\\n"
        "    A;                                      \\\n"
        "                                            \\\n"
        "    cur_start_pos = cur_end_pos = orig_start_pos;         \\\n"
        "                                            \\\n"
        "}\n\n");

    fprintf(src_file, "#define T(func)                                                     \\\n"
        "{                                                                   \\\n"
        "    %ls_syntax_node_t *child = 0;                                         \\\n"
        "    if ((res = func(ib, cur_start_pos, &cur_end_pos, &child, map, error_stack))) \\\n"
        "    {                                                               \\\n"
        "        if (child)                                                  \\\n"
        "            array_add(&child_stack, &child);                        \\\n"
        "        cur_start_pos = cur_end_pos;                                \\\n"
        "    }                                                               \\\n"
        "}\n\n", pbuf);

    fprintf(src_file, "#define S(str)                                  \\\n"
        "{                                               \\\n"
        "    wchar_t ch, *ptr = str;                     \\\n"
        "                                                \\\n"
        "    input_buffer_setpos(ib, cur_start_pos);     \\\n"
        "                                                \\\n"
        "    for (res = 1; *ptr; ++ptr)                  \\\n"
        "    {                                           \\\n"
        "        ch = input_buffer_read_char(ib);        \\\n"
        "        if (!(res = (ch == *ptr)))              \\\n"
        "            break;                              \\\n"
        "    }                                           \\\n"
        "                                                \\\n"
        "    if (res)                                    \\\n"
        "        cur_start_pos = cur_end_pos = input_buffer_getpos(ib);  \\\n"
        "}\n\n");

    fprintf(src_file, "#define C(str)                                  \\\n"
        "{                                               \\\n"
        "    wchar_t ch, *ptr = str;                     \\\n"
        "    input_buffer_setpos(ib, cur_start_pos);     \\\n"
        "    ch = input_buffer_read_char(ib);            \\\n"
        "    for (res = 0; *ptr; ++ptr)                  \\\n"
        "    {                                           \\\n"
        "        if ((res = (ch == *ptr)))               \\\n"
        "            break;                              \\\n"
        "    }                                           \\\n"
        "    if (res)                                    \\\n"
        "        cur_start_pos = cur_end_pos = input_buffer_getpos(ib);  \\\n"
        "}\n\n");

    fprintf(src_file, "#define DOT                                     \\\n"
        "{                                               \\\n"
        "    wchar_t ch;                                 \\\n"
        "    input_buffer_setpos(ib, cur_start_pos);     \\\n"
        "    ch = input_buffer_read_char(ib);            \\\n"
        "    if (res = (ch != WEOF))                     \\\n"
        "        cur_start_pos = cur_end_pos = input_buffer_getpos(ib);  \\\n"
        "}\n\n");

    fprintf(src_file, "#define PEG_PARSE(FUNCTION, NODE_TYPE, NODE_NAME, EXP)                                                                 \\\n"
        "static int FUNCTION(input_buffer_t *ib, int start_offset, int *end_offset, %ls_syntax_node_t **node, memo_map_t *map, array_t *error_stack) \\\n"
        "{                                                                                                           \\\n"
        "    int res = 0;                                                                                            \\\n"
        "    int cur_start_pos = start_offset, cur_end_pos = start_offset;                                           \\\n"
        "                                                                                                            \\\n"
        "    array_t child_stack;                                                                                    \\\n"
        "    array_init(&child_stack, sizeof(%ls_syntax_node_t *), 0);                                               \\\n"
        "                                                                                                            \\\n"
        "    if (is_memoized(map, NODE_TYPE, start_offset, node, end_offset))                                        \\\n"
        "        return *node != 0;                                                                                  \\\n"
        "                                                                                                            \\\n"
        "    EXP;                                                                                                    \\\n"
        "                                                                                                            \\\n"
        "    if (res)                                                                                                \\\n"
        "    {                                                                                                       \\\n"
        "        int i, len;                                                                                         \\\n"
        "        *end_offset = cur_end_pos;                                                                          \\\n"
        "        *node = %ls_syntax_node_create(NODE_TYPE, start_offset, cur_end_pos, ib);                           \\\n"
        "        len = array_size(&child_stack);                                                                     \\\n"
        "        (*node)->child = (%ls_syntax_node_t **) calloc(len+1, sizeof(%ls_syntax_node_t *));                 \\\n"
        "                                                                                                            \\\n"
        "        for (i = 0; i < len; ++i)                                                                           \\\n"
        "        {                                                                                                   \\\n"
        "            (*node)->child[i] = *(%ls_syntax_node_t **) array_item(&child_stack, i);                        \\\n"
        "        }                                                                                                   \\\n"
	"        (*node)->children = len;                                                                            \\\n"
        "        delete_children(&child_stack, len);                                                                 \\\n"
        "        array_deinit(&child_stack);                                                                         \\\n"
        "        if (%ls_dispatch[NODE_TYPE] != NULL) (%ls_dispatch[NODE_TYPE])(*node,NULL);                         \\\n"
        "        res = 1;                                                                                            \\\n"
        "    }                                                                                                       \\\n"
        "    else                                                                                                    \\\n"
        "    {                                                                                                       \\\n"
        "        int len = (int) wcslen(NODE_NAME) + 64;                                                             \\\n"
        "        wchar_t *buf = (wchar_t *) calloc(len, sizeof(wchar_t));                                            \\\n"
        "        swprintf(buf, len, L\"syntax error: expected %%ls\", NODE_NAME);                                    \\\n"
        "        add_error(error_stack, start_offset, buf);                                                          \\\n"
	"        if(%ls_wish_node == NODE_TYPE) dump_errors(error_stack);					     \\\n"
        "        free(buf);                                                                                          \\\n"
        "        *node = 0;                                                                                          \\\n"
        "    }                                                                                                       \\\n"
        "                                                                                                            \\\n"
        "    memoize(map, NODE_TYPE, start_offset, res ? *end_offset : start_offset, *node);                         \\\n"
        "                                                                                                            \\\n"
        "    return res;                                                                                             \\\n"
        "}\n\n", pbuf, pbuf, pbuf, pbuf, pbuf, pbuf, pbuf, pbuf, pbuf);
} /* print_macros() */


static void print_escape(wchar_t *buf, wchar_t *str)
{
    wchar_t *ch;
    wchar_t temp[3];
    temp[1] = 0;
    temp[2] = 0;

    for (ch = str; *ch; ++ch)
    {
        temp[0] = '\\';

        switch (*ch)
        {
        case L'\'':
            temp[1] = '\'';
            break;
        case L'\"':
            temp[1] = '\"';
            break;
        case L'\?':
            temp[1] = '?';
            break;
        case L'\\':
            temp[1] = '\\';
            break;
        case L'\a':
            temp[1] = 'a';
            break;
        case L'\b':
            temp[1] = 'b';
            break;
        case L'\f':
            temp[1] = 'f';
            break;
        case L'\n':
            temp[1] = 'n';
            break;
        case L'\r':
            temp[1] = 'r';
            break;
        case L'\t':
            temp[1] = 't';
            break;
        case L'\v':
            temp[1] = 'v';
            break;
        default:
            temp[0] = *ch;
            temp[1] = 0;
            break;
        }

        wcscat(buf, temp);
    }
} /* print_escape() */


static void print_rule_exp(wchar_t *buf, const rule_exp_t *exp, const array_t *rule_records, const array_t *node_function_names)
{
    int i, len;

    switch (exp->type)
    {
    case RULE_EXP_SEQ:
        wcsncat(buf, L"SEQ(", BUF_LEN);
        print_rule_exp(buf, exp->left, rule_records, node_function_names);
        wcsncat(buf, L", ", BUF_LEN);
        print_rule_exp(buf, exp->right, rule_records, node_function_names);
        wcsncat(buf, L")", BUF_LEN);
        break;
    case RULE_EXP_DISJ:
        wcsncat(buf, L"DISJ(", BUF_LEN);
        print_rule_exp(buf, exp->left, rule_records, node_function_names);
        wcsncat(buf, L", ", BUF_LEN);
        print_rule_exp(buf, exp->right, rule_records, node_function_names);
        wcsncat(buf, L")", BUF_LEN);
        break;
    case RULE_EXP_STAR:
        wcsncat(buf, L"STAR(", BUF_LEN);
        print_rule_exp(buf, exp->left, rule_records, node_function_names);
        wcsncat(buf, L")", BUF_LEN);
        break;
    case RULE_EXP_PLUS:
        wcsncat(buf, L"PLUS(", BUF_LEN);
        print_rule_exp(buf, exp->left, rule_records, node_function_names);
        wcsncat(buf, L")", BUF_LEN);
        break;
    case RULE_EXP_QUES:
        wcsncat(buf, L"QUES(", BUF_LEN);
        print_rule_exp(buf, exp->left, rule_records, node_function_names);
        wcsncat(buf, L")", BUF_LEN);
        break;
    case RULE_EXP_BANG:
        wcsncat(buf, L"BANG(", BUF_LEN);
        print_rule_exp(buf, exp->left, rule_records, node_function_names);
        wcsncat(buf, L")", BUF_LEN);
        break;
    case RULE_EXP_AMP:
        wcsncat(buf, L"AMP(", BUF_LEN);
        print_rule_exp(buf, exp->left, rule_records, node_function_names);
        wcsncat(buf, L")", BUF_LEN);
        break;
    case RULE_EXP_HIDE:
        wcsncat(buf, L"HIDE(", BUF_LEN);
        print_rule_exp(buf, exp->left, rule_records, node_function_names);
        wcsncat(buf, L")", BUF_LEN);
        break;
    case RULE_EXP_CALL:
        wcsncat(buf, L"T(", BUF_LEN);
        len = array_size(rule_records);
        for (i = 0; i < len; ++i)
        {
            rule_rec_t *rec = *(rule_rec_t **) array_item(rule_records, i);
            if (wcscmp(rec->rule_name, exp->data.str) == 0)
            {
                wcsncat(buf, *(wchar_t **) array_item(node_function_names, i), BUF_LEN);
            }
        }
        wcsncat(buf, L")", BUF_LEN);
        break;
    case RULE_EXP_STR:
        wcsncat(buf, L"S(L\"", BUF_LEN);
        print_escape(buf, exp->data.str);
        wcsncat(buf, L"\")", BUF_LEN);
        break;
    case RULE_EXP_DOT:
        wcsncat(buf, L"DOT", BUF_LEN);
        break;
    case RULE_EXP_CLASS:
        wcsncat(buf, L"C(L\"", BUF_LEN);
        print_escape(buf, exp->data.str);
        wcsncat(buf, L"\")", BUF_LEN);
        break;
    }
} /* print_rule_exp() */

static void print_function_bodies(const wchar_t *prefix, FILE *src_file, const array_t *rule_records, const array_t *node_type_labels, const array_t *node_function_names)
{
    wchar_t pbuf[128];
    wchar_t buf[BUF_LEN];
    int i, len;

    swprintf(pbuf, 128, L"%ls", prefix);
    to_lower(pbuf);

    fprintf(src_file, "/* parsing functions */\n\n");

    len = array_size(rule_records);
    for (i = 0; i < len; ++i)
    {
        rule_rec_t *rec = *(rule_rec_t **) array_item(rule_records, i);

        buf[0] = 0;
        print_rule_exp(buf, rec->rule_spec, rule_records, node_function_names);
        fprintf(src_file, "PEG_PARSE(%ls, %ls, L\"%ls\", %ls)\n\n", 
            *(wchar_t **) array_item(node_function_names, i),
            *(wchar_t **) array_item(node_type_labels, i+1),
            ((rec->rule_desc && rec->rule_desc[0]) ? rec->rule_desc : *(wchar_t **) array_item(node_function_names, i)),
            buf);
    }


} /* print_function_bodies() */

static void generate_source(const wchar_t *prefix, const char *header_fname, const char *src_fname, FILE *src_file, 
                            const array_t *rule_records, input_buffer_t *ib, const array_t *line_endings, 
                            const array_t *node_type_labels, const array_t *node_function_names)
{
    wchar_t buf[BUF_LEN];
    time_t cur_time;

    /* timestamp */
    time(&cur_time);
    fprintf(src_file, "/*\n * generated %s */\n", ctime(&cur_time));
    fprintf(src_file, "\n");

    /* utility code */
    fprintf(src_file, "#include \"%s\"\n\n", header_fname);
    fprintf(src_file, "#include <assert.h>\n#include <stdlib.h>\n#include <stdio.h>\n\n#include <malloc.h>\n#include <string.h>\n#include <wchar.h>\n\n");

    print_utility_source(prefix, src_file, node_type_labels);

    /* function prototypes */
    print_function_prototypes(prefix, src_file, node_function_names);

    /* macros */
    print_macros(prefix, src_file);

    /* functions */
    print_function_bodies(prefix, src_file, rule_records, node_type_labels, node_function_names);

    /* main function */
    swprintf(buf, BUF_LEN, L"%ls", prefix);
    to_lower(buf);

    fprintf(src_file,"/* _wish_node is a useful last-ditch grammar debugging tool. \n"
		    "Set it to the node you expected to be recognized but wasn't. Dumps error list to stdout. */\n"
		     
		    "int %ls_wish_node = 32000;\n\n",buf);
    fprintf(src_file, "/* main function */\n\n");

    fprintf(src_file, "int %ls_parse(char *fname, %ls_syntax_node_t **parse_tree, void **input_buf, void **error_list)\n"
        "{\n"
        "    FILE *f;\n"
        "    input_buffer_t *ib;\n"
        "    int start_offset, end_offset;\n"
        "    array_t line_endings;\n"
        "    memo_map_t *map;\n"
        "    %ls_syntax_node_t *root = 0;\n"
        "\n"
        "    f = fopen(fname, \"r\");\n"
        "    if (!f) return 0;\n"
        "\n"
        "    ib = input_buffer_create(fname, f);\n"
        "    assert(ib);\n"
        "    *input_buf = ib;\n"
        "\n"
        "    map = memo_map_create();\n"
        "    *error_list = (array_t *) calloc(sizeof(array_t), 1);\n"
        "    array_init(*error_list, sizeof(%ls_error_rec_t), 0);\n"
        "\n"
        "    start_offset = input_buffer_getpos(ib);\n"
        "    %ls(ib, start_offset, &end_offset, &root, map, *error_list);\n"
        "    memo_map_destroy(map);\n"
        "\n"
        "    if (root)\n"
        "    {\n"
        "        array_init(&line_endings, sizeof(int), 0);\n"
        "        find_line_endings(&line_endings, ib, root->begin);\n"
        "        assign_line_numbers(ib, root, &line_endings);\n"
        "        array_deinit(&line_endings);\n"
        "    }\n"
        "\n"
        "    *parse_tree = root;\n"
        "    return root != 0;\n"
        "}\n\n", buf, buf, buf, buf, *(wchar_t **) array_item(node_function_names, 0));
        
} /* generate_source() */

/*************************************************/

/**
* Generates a parser in C from a PEG grammar.
*/

void generate_c_parser(const wchar_t *prefix,
                       const char *header_fname, FILE *header_file, 
                       const char *src_fname, FILE *src_file, 
                       array_t *rule_records, input_buffer_t *ib, 
                       array_t *line_endings)
{
    array_t node_type_labels;    /* wchar_t * */
    array_t node_function_names; /* wchar_t * */
    int i, len;

    if (!prefix || !prefix[0])
        prefix = L"PEG";

    /* get names */
    array_init(&node_type_labels, sizeof(wchar_t *), 0);
    array_init(&node_function_names, sizeof(wchar_t *), 0);

    get_node_type_labels(prefix, rule_records, &node_type_labels);
    get_node_function_names(prefix, rule_records, &node_function_names);

    /* assemble header with node types */
    generate_header(prefix, header_fname, header_file, &node_type_labels);

    /* assemble rule code */
    generate_source(prefix, header_fname, src_fname, src_file, rule_records, ib, line_endings, &node_type_labels, &node_function_names);

    /* clean up */
    len = array_size(&node_type_labels);
    for (i = 0; i < len; ++i)
        free(*(wchar_t **) array_item(&node_type_labels, i));
    array_deinit(&node_type_labels);

    len = array_size(&node_function_names);
    for (i = 0; i < len; ++i)
        free(*(wchar_t **) array_item(&node_function_names, i));
    array_deinit(&node_function_names);
} /* generate_c_parser() */
