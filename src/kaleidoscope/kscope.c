/*
 * generated Mon Mar 30 21:04:44 2009
 */

#include "kscope.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <malloc.h>
#include <string.h>
#include <wchar.h>

const char *kscope_node_names[49] = {
"KSCOPE_NULL_NODE",
"KSCOPE_FILE_NODE",
"KSCOPE_STATEMENT_NODE",
"KSCOPE_DEFN_NODE",
"KSCOPE_EXTERN_NODE",
"KSCOPE_PROTO_NODE",
"KSCOPE_EXPR_NODE",
"KSCOPE_BINOPRHS_NODE",
"KSCOPE_UNARY_NODE",
"KSCOPE_PRIMARY_NODE",
"KSCOPE_VAREXPR_NODE",
"KSCOPE_FOREXPR_NODE",
"KSCOPE_IFEXPR_NODE",
"KSCOPE_PAREN_NODE",
"KSCOPE_IDEXPR_NODE",
"KSCOPE_IDPROTO_NODE",
"KSCOPE_BINPROTO_NODE",
"KSCOPE_UNIPROTO_NODE",
"KSCOPE_PROTOARG_NODE",
"KSCOPE_CALL_NODE",
"KSCOPE_EQEXPR_NODE",
"KSCOPE_OPERATOR_NODE",
"KSCOPE_OPERATOR_STR_NODE",
"KSCOPE_UNKNOWN_NODE",
"KSCOPE_IDENTIFIER_NODE",
"KSCOPE_IDENTIFIER_STR_NODE",
"KSCOPE_NUMBER_NODE",
"KSCOPE_NUMBER_STR_NODE",
"KSCOPE_LETTER_NODE",
"KSCOPE_LEX_NODE",
"KSCOPE_SEP_NODE",
"KSCOPE_OPEQL_NODE",
"KSCOPE_OP_NODE",
"KSCOPE_CP_NODE",
"KSCOPE_DEF_KW_NODE",
"KSCOPE_EXTERN_KW_NODE",
"KSCOPE_IF_NODE",
"KSCOPE_THEN_NODE",
"KSCOPE_ELSE_NODE",
"KSCOPE_FOR_NODE",
"KSCOPE_IN_NODE",
"KSCOPE_BINARY_KW_NODE",
"KSCOPE_UNARY_KW_NODE",
"KSCOPE_VAR_NODE",
"KSCOPE___NODE",
"KSCOPE_WS_NODE",
"KSCOPE_COMMENT_NODE",
"KSCOPE_WHITESPACE_NODE",
"KSCOPE_EOF_NODE",
};

kscope_syntax_node_process_ft kscope_dispatch[49] = {
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
};

wchar_t *wcs_dup(wchar_t *str)
{
    wchar_t *res;
    int len = (int) wcslen(str) + 1;

    res = (wchar_t *) calloc(len, sizeof(wchar_t));
    memcpy(res, str, len * sizeof(wchar_t));

    return res;
} /* wcs_dup() */
char *str_dup(char *str)
{
    char *res;
    int len = (int) strlen(str) + 1;

    res = (char *) calloc(len, sizeof(char));
    memcpy(res, str, len * sizeof(char));

    return res;
} /* str_dup() */
/* dynamic arrays */

typedef struct _array_t
{
    int data_size;
    int num, cap;
    void *data;
}
array_t;

static void array_init(array_t *da, int data_size, int num_items)
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

static int array_size(array_t *da)
{
    assert(da);

    return da->num;
} /* array_size() */

static void *array_item(array_t *da, int index)
{
    char *cp;

    assert(da);
    assert(index < da->num);

    cp = (char *) da->data;
    return cp + (da->data_size * index);
} /* array_item() */

static void array_resize(array_t *da, int num_items)
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

static void array_add(array_t *da, void *item)
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

static void array_deinit(array_t *da)
{
    assert(da);

    free(da->data);
    da->data = 0;
    da->cap = da->num = 0;
} /* array_destroy() */

static void array_copy(array_t *dest, array_t *src)
{
    assert(dest);
    assert(src);
    assert(dest->data_size == src->data_size);

    array_deinit(dest);
    array_init(dest, dest->data_size, src->num);
    memcpy(dest->data, src->data, src->num * src->data_size);
} /* array_copy() */

static void array_clear(array_t *da)
{
    assert(da);

    da->num = 0;
} /* array_clear() */

/* syntax node functions */

kscope_syntax_node_t *kscope_syntax_node_create(int type, int begin, int end,void* ib)
{
    kscope_syntax_node_t *res = (kscope_syntax_node_t *) calloc(1, sizeof(kscope_syntax_node_t));
    res->type  = type;
    res->begin = begin;
    res->end   = end;
    res->first_line = -1;
    res->last_line  = -1;
    res->child = 0;
    res->children = 0;
    res->ib = ib;
    return res;
} /* syntax_node_create() */

kscope_syntax_node_t *kscope_syntax_node_copy(kscope_syntax_node_t *node)
{
    kscope_syntax_node_t *copy = 0, **cur;
    int i, len;

    if (node)
    {
        copy = kscope_syntax_node_create(node->type, node->begin, node->end, node->ib);
        copy->first_line = node->first_line;
        copy->last_line = node->last_line;

        len = node->children;
        copy->children = node->children;
        if (len)
        {
            copy->child = (kscope_syntax_node_t **) calloc(len+1, sizeof(kscope_syntax_node_t *));
            for (i = 0; i < len; ++i)
                copy->child[i] = kscope_syntax_node_copy(node->child[i]);
        }
    }

    return copy;
} /* syntax_node_copy() */

void kscope_syntax_node_destroy(kscope_syntax_node_t *node)
{
    kscope_syntax_node_t **cur;

    assert(node);

    if (node->child)
        for (cur = node->child; *cur; ++cur)
            kscope_syntax_node_destroy(*cur);

    free(node);
} /* syntax_node_destroy() */

void kscope_syntax_node_traverse_preorder(kscope_syntax_node_t *root, void *data, kscope_syntax_node_process_ft entry_func, kscope_syntax_node_process_ft exit_func)
{
    if (root)
    {
        kscope_syntax_node_t **cur;
        if (entry_func != NULL)
          if (entry_func(root, data))
            return;

        if (root->child)
            for (cur = root->child; *cur; ++cur)
                kscope_syntax_node_traverse_preorder(*cur, data, entry_func, exit_func);
        if (exit_func != NULL)
          exit_func(root,data);
    }
} /* syntax_node_traverse_inorder() */

/* error handling functions */

void *kscope_create_error_list()
{
    array_t *ptr = (array_t *) calloc(1, sizeof(array_t));
    array_init(ptr, sizeof(kscope_error_rec_t), 0);
    return ptr;
}

void kscope_destroy_error_list(void *error_list)
{
    int i, len = array_size((array_t *) error_list);
    for (i = 0; i < len; ++i)
    {
        kscope_error_rec_t *er = (kscope_error_rec_t *) array_item((array_t *) error_list, i);
        free(er->str);
    }
    array_deinit((array_t *) error_list);
    free((array_t *) error_list);
}

void kscope_add_error(void *error_list, int pos, wchar_t *str)
{
    kscope_error_rec_t er;
    er.pos = pos;
    er.str = wcs_dup(str);
    array_add((array_t *) error_list, &er);
}

int kscope_num_errors(void *error_list)
{
    return array_size((array_t *) error_list);
}

kscope_error_rec_t *kscope_get_error(void *error_list, int index)
{
    return (kscope_error_rec_t *) array_item((array_t *) error_list, index);
}

static void delete_errors(array_t *errors, int start_index)
{
    int i, len = array_size(errors);
    for (i = start_index; i < len; ++i)
    {
        kscope_error_rec_t *rec = (kscope_error_rec_t *) array_item(errors, i);
        free(rec->str);
    }
    errors->num = start_index;
}

static void add_error(array_t *errs, int pos, wchar_t *str)
{
    kscope_error_rec_t rec;
    rec.pos = pos;
    rec.str = wcs_dup(str);

    array_add(errs, &rec);
} /* add_error() */

static void dump_errors(array_t *errs){
int cnt,max;
kscope_error_rec_t *this_error;

for (cnt = kscope_num_errors(errs)-1;cnt >= 0; cnt--){
  this_error = kscope_get_error(errs,cnt);
  printf("Error %d:%d %ls\n",cnt,this_error->pos,this_error->str);
}
}
/*Debug dump of errors to stdout*/
/* input buffers */

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

static void input_buffer_get_unicode_mode(input_buffer_t *ib)
{
    assert(ib);

    ib->unicode_mode = INPUT_BUFFER_ANSI;
    ib->flags |= INPUT_BUFFER_UNICODE_BOM_READ;
} /* input_buffer_get_unicode_mode() */

static input_buffer_t *input_buffer_create(char *name, FILE *f)
{
    input_buffer_t *ib;

    assert(f);

    ib = (input_buffer_t *) calloc(1, sizeof(input_buffer_t));
    ib->name = str_dup(name);
    ib->f = f;

    input_buffer_get_unicode_mode(ib);

    return ib;
} /* input_buffer_create() */

static void input_buffer_destroy(input_buffer_t *ib)
{
    assert(ib);

    fclose(ib->f);
    free(ib->name);
    free(ib->buf);
    free(ib);
} /* input_buffer_destroy() */

static void input_buffer_setpos(input_buffer_t *ib, int pos)
{
    assert(ib);
    assert(pos >= 0);

    ib->current_pos = pos;
} /* input_buffer_setpos() */

static int input_buffer_getpos(input_buffer_t *ib)
{
    assert(ib);

    return ib->current_pos;
} /* input_buffer_getpos() */

#define INPUT_BUFFER_SIZE_INCREMENT 4096

static wchar_t input_buffer_read_char(input_buffer_t *ib)
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

static void input_buffer_read_wstring(input_buffer_t *ib, int begin, int end, array_t *str)
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

static void input_buffer_read_string(input_buffer_t *ib, int begin, int end, array_t *str)
{
    char ch = WEOF;

    assert(ib);
    assert(str);

    array_clear(str);
    ib->current_pos = begin;
    while (ib->current_pos < end)
    {
        ch = (char)wctob(input_buffer_read_char(ib));

        if (ch != WEOF)
            array_add(str, &ch);
    }

    ch = 0;
    array_add(str, &ch);
} /* input_buffer_read_string() */

/* external input buffer functions */
wchar_t *kscope_get_wstr(kscope_syntax_node_t *node)
{
    array_t str;
    wchar_t *res;
    void *ib;

    ib = node->ib;
    array_init(&str, sizeof(wchar_t), 0);
    input_buffer_read_wstring((input_buffer_t *) ib, node->begin, node->end, &str);
    res = wcs_dup(str.data);
    return res;
}

char *kscope_get_str(kscope_syntax_node_t *node)
{
    array_t str;
    char *res;
    void *ib;

    ib = node->ib;
    array_init(&str, sizeof(char), 0);
    input_buffer_read_string((input_buffer_t *) ib, node->begin, node->end, &str);
    res = str_dup(str.data);
    return res;
}

void kscope_destroy_input_buffer(void *ib)
{
    input_buffer_destroy((input_buffer_t *) ib);
}

/* memo map functions */

typedef struct _memo_rec_t
{
    int start_offset, end_offset;
    kscope_syntax_node_t *parse_tree;
}
memo_rec_t;

typedef struct _memo_map_t
{
    array_t records[KSCOPE_NUM_NODE_TYPES]; /* a array_t of memo_rec_t structures */
}
memo_map_t;

static memo_map_t *memo_map_create()
{
    int i;
    memo_map_t *map;
    
    map = (memo_map_t *) calloc(1, sizeof(memo_map_t));

    for (i = 0; i < KSCOPE_NUM_NODE_TYPES; ++i)
    {
        array_init(&map->records[i], sizeof(memo_rec_t), 0);
    }

    return map;
} /* memo_map_create() */

static void memo_map_destroy(memo_map_t *map)
{
    int i;

    assert(map);

    for (i = 0; i < KSCOPE_NUM_NODE_TYPES; ++i)
    {
        int j, count = array_size(&map->records[i]);
        for (j = 0; j < count; ++j)
        {
            memo_rec_t *mr = (memo_rec_t *) array_item(&map->records[i], j);
            if (mr->parse_tree)
                kscope_syntax_node_destroy(mr->parse_tree);
        }
        array_deinit(&map->records[i]);
    }

    free(map);
} /* memo_map_destroy() */

static int is_memoized(memo_map_t *map, int type, int start_offset, kscope_syntax_node_t **res, int *end_offset)
{
    int i, len;
    array_t *da;

    assert(map);
    assert(type < KSCOPE_NUM_NODE_TYPES);

    da = &map->records[type];
    len = array_size(da);

    for (i = 0; i < len; ++i)
    {
        memo_rec_t *rec = (memo_rec_t *) array_item(da, i);

        if (rec->start_offset == start_offset)
        {
            *res = kscope_syntax_node_copy(rec->parse_tree);
            *end_offset = rec->end_offset;
            return 1;
        }
    }

    return 0;
} /* is_memoized() */

static void memoize(memo_map_t *map, int type, int start_offset, int end_offset, kscope_syntax_node_t *node)
{
    memo_rec_t rec;

    assert(map);
    assert(type < KSCOPE_NUM_NODE_TYPES);

    rec.start_offset = start_offset;
    rec.end_offset = end_offset;
    rec.parse_tree = kscope_syntax_node_copy(node);

    array_add(&map->records[type], &rec);
} /* memoize() */

static void delete_children(array_t *children, int start_index)
{
    int i, len;
    
    assert(children);
    
    len = array_size(children);

    for (i = start_index; i < len; ++i)
    {
        kscope_syntax_node_t *child = *(kscope_syntax_node_t **) array_item(children, i);
        kscope_syntax_node_destroy(child);
    }

    children->num = start_index;
} /* delete_children() */

static void find_line_endings(array_t *end_offsets, input_buffer_t *ib, int start_pos)
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

static int find_line(int pos, array_t *line_endings)
{
    int n, i, len = array_size(line_endings);
    for (i = 0; i < len; ++i)
    {
        n = * (int *) array_item(line_endings, i);
        if (pos < n)
            return i+1;
    }

    if (pos == n)
        return len;
    else
        return -1;
} /* find_line() */

static int assign_line_number(kscope_syntax_node_t *node, void *data)
{
    array_t *line_endings = (array_t *) data;

    if (node->begin >= node->end)
    {
        node->first_line = node->last_line = find_line(node->begin, line_endings);
    }
    else
    {
        node->first_line = find_line(node->begin, line_endings);
        node->last_line = find_line(node->end-1, line_endings);
    }

    return 0;
} /* assign_line_number() */

static void assign_line_numbers(input_buffer_t *ib, kscope_syntax_node_t *node, array_t *line_endings)
{
    /* assign line numbers to the nodes */
    kscope_syntax_node_traverse_preorder(node, line_endings, assign_line_number,NULL);
} /* assign_line_numbers() */
/* function prototypes */

static int parse_kscope_file(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_statement(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_defn(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_extern(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_proto(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_expr(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_binoprhs(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_unary(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_primary(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_varexpr(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_forexpr(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_ifexpr(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_paren(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_idexpr(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_idproto(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_binproto(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_uniproto(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_protoarg(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_call(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_eqexpr(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_operator(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_operator_str(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_unknown(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_identifier(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_identifier_str(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_number(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_number_str(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_letter(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_lex(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_sep(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_opeql(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_op(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_cp(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_def_kw(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_extern_kw(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_if(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_then(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_else(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_for(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_in(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_binary_kw(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_unary_kw(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_var(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope__(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_ws(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_comment(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_whitespace(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);
static int parse_kscope_eof(input_buffer_t *ib, int start_offset, int *end_ofset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack);

/* parsing macros */

#define SEQ(A, B)                           \
{                                           \
    int orig_stack_size = child_stack.num;  \
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
}

#define DISJ(A, B)                          \
{                                           \
    int orig_stack_size = child_stack.num;  \
    int orig_start_pos = cur_start_pos;     \
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
}

#define STAR(A)                             \
{                                           \
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
}

#define PLUS(A)                             \
{                                           \
    int orig_stack_size = child_stack.num;  \
    int count = 0;                          \
    int error_stack_size = error_stack->num; \
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
}

#define QUES(A)                             \
{                                           \
    int error_stack_size = error_stack->num; \
                                            \
    A;                                      \
                                            \
    if (res)                                \
        cur_start_pos = cur_end_pos;        \
                                            \
    res = 1;                                \
    delete_errors(error_stack, error_stack_size); \
}

#define HIDE(A)                                     \
{                                                   \
    int orig_stack_size = child_stack.num;          \
                                                    \
    A;                                              \
                                                    \
    delete_children(&child_stack, orig_stack_size); \
    cur_start_pos = cur_end_pos;                    \
}

#define BANG(A)                             \
{                                           \
    int orig_start_pos = cur_start_pos;     \
    int orig_stack_size = child_stack.num;  \
    int orig_error_size = error_stack->num; \
                                            \
    A;                                      \
                                            \
    res = !res;                             \
                                            \
    cur_start_pos = cur_end_pos = orig_start_pos;         \
                                            \
    delete_children(&child_stack, orig_stack_size); \
    delete_errors(error_stack, orig_error_size); \
}

#define AMP(A)                              \
{                                           \
    int orig_start_pos = cur_start_pos;     \
                                            \
                                            \
    A;                                      \
                                            \
    cur_start_pos = cur_end_pos = orig_start_pos;         \
                                            \
}

#define T(func)                                                     \
{                                                                   \
    kscope_syntax_node_t *child = 0;                                         \
    if ((res = func(ib, cur_start_pos, &cur_end_pos, &child, map, error_stack))) \
    {                                                               \
        if (child)                                                  \
            array_add(&child_stack, &child);                        \
        cur_start_pos = cur_end_pos;                                \
    }                                                               \
}

#define S(str)                                  \
{                                               \
    wchar_t ch, *ptr = str;                     \
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
}

#define C(str)                                  \
{                                               \
    wchar_t ch, *ptr = str;                     \
    input_buffer_setpos(ib, cur_start_pos);     \
    ch = input_buffer_read_char(ib);            \
    for (res = 0; *ptr; ++ptr)                  \
    {                                           \
        if ((res = (ch == *ptr)))               \
            break;                              \
    }                                           \
    if (res)                                    \
        cur_start_pos = cur_end_pos = input_buffer_getpos(ib);  \
}

#define DOT                                     \
{                                               \
    wchar_t ch;                                 \
    input_buffer_setpos(ib, cur_start_pos);     \
    ch = input_buffer_read_char(ib);            \
    if (res = (ch != WEOF))                     \
        cur_start_pos = cur_end_pos = input_buffer_getpos(ib);  \
}

#define PEG_PARSE(FUNCTION, NODE_TYPE, NODE_NAME, EXP)                                                                 \
static int FUNCTION(input_buffer_t *ib, int start_offset, int *end_offset, kscope_syntax_node_t **node, memo_map_t *map, array_t *error_stack) \
{                                                                                                           \
    int res = 0;                                                                                            \
    int cur_start_pos = start_offset, cur_end_pos = start_offset;                                           \
                                                                                                            \
    array_t child_stack;                                                                                    \
    array_init(&child_stack, sizeof(kscope_syntax_node_t *), 0);                                               \
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
        *node = kscope_syntax_node_create(NODE_TYPE, start_offset, cur_end_pos, ib);                           \
        len = array_size(&child_stack);                                                                     \
        (*node)->child = (kscope_syntax_node_t **) calloc(len+1, sizeof(kscope_syntax_node_t *));                 \
                                                                                                            \
        for (i = 0; i < len; ++i)                                                                           \
        {                                                                                                   \
            (*node)->child[i] = *(kscope_syntax_node_t **) array_item(&child_stack, i);                        \
        }                                                                                                   \
        (*node)->children = len;                                                                            \
        delete_children(&child_stack, len);                                                                 \
        array_deinit(&child_stack);                                                                         \
        if (kscope_dispatch[NODE_TYPE] != NULL) (kscope_dispatch[NODE_TYPE])(*node,NULL);                         \
        res = 1;                                                                                            \
    }                                                                                                       \
    else                                                                                                    \
    {                                                                                                       \
        int len = (int) wcslen(NODE_NAME) + 64;                                                             \
        wchar_t *buf = (wchar_t *) calloc(len, sizeof(wchar_t));                                            \
        swprintf(buf, len, L"syntax error: expected %ls", NODE_NAME);                                    \
        add_error(error_stack, start_offset, buf);                                                          \
        if(kscope_wish_node == NODE_TYPE) dump_errors(error_stack);					     \
        free(buf);                                                                                          \
        *node = 0;                                                                                          \
    }                                                                                                       \
                                                                                                            \
    memoize(map, NODE_TYPE, start_offset, res ? *end_offset : start_offset, *node);                         \
                                                                                                            \
    return res;                                                                                             \
}

/* parsing functions */

PEG_PARSE(parse_kscope_file, KSCOPE_FILE_NODE, L"parse_kscope_file", SEQ(T(parse_kscope__), SEQ(STAR(SEQ(T(parse_kscope_statement), T(parse_kscope__))), T(parse_kscope_unknown))))

PEG_PARSE(parse_kscope_statement, KSCOPE_STATEMENT_NODE, L"parse_kscope_statement", DISJ(T(parse_kscope_defn), DISJ(T(parse_kscope_extern), T(parse_kscope_expr))))

PEG_PARSE(parse_kscope_defn, KSCOPE_DEFN_NODE, L"parse_kscope_defn", SEQ(T(parse_kscope_def_kw), SEQ(T(parse_kscope_proto), T(parse_kscope_expr))))

PEG_PARSE(parse_kscope_extern, KSCOPE_EXTERN_NODE, L"parse_kscope_extern", SEQ(T(parse_kscope_extern_kw), T(parse_kscope_proto)))

PEG_PARSE(parse_kscope_proto, KSCOPE_PROTO_NODE, L"parse_kscope_proto", DISJ(T(parse_kscope_idproto), DISJ(T(parse_kscope_binproto), T(parse_kscope_uniproto))))

PEG_PARSE(parse_kscope_expr, KSCOPE_EXPR_NODE, L"parse_kscope_expr", SEQ(T(parse_kscope_unary), T(parse_kscope_binoprhs)))

PEG_PARSE(parse_kscope_binoprhs, KSCOPE_BINOPRHS_NODE, L"parse_kscope_binoprhs", STAR(SEQ(T(parse_kscope_operator), T(parse_kscope_unary))))

PEG_PARSE(parse_kscope_unary, KSCOPE_UNARY_NODE, L"parse_kscope_unary", DISJ(T(parse_kscope_primary), SEQ(T(parse_kscope_operator), T(parse_kscope_unary))))

PEG_PARSE(parse_kscope_primary, KSCOPE_PRIMARY_NODE, L"parse_kscope_primary", DISJ(T(parse_kscope_varexpr), DISJ(T(parse_kscope_forexpr), DISJ(T(parse_kscope_ifexpr), DISJ(T(parse_kscope_paren), DISJ(T(parse_kscope_idexpr), T(parse_kscope_number)))))))

PEG_PARSE(parse_kscope_varexpr, KSCOPE_VAREXPR_NODE, L"parse_kscope_varexpr", SEQ(T(parse_kscope_var), SEQ(T(parse_kscope_identifier), SEQ(QUES(T(parse_kscope_eqexpr)), SEQ(STAR(SEQ(T(parse_kscope_sep), SEQ(T(parse_kscope_identifier), QUES(T(parse_kscope_eqexpr))))), SEQ(T(parse_kscope_in), T(parse_kscope_expr)))))))

PEG_PARSE(parse_kscope_forexpr, KSCOPE_FOREXPR_NODE, L"parse_kscope_forexpr", SEQ(T(parse_kscope_for), SEQ(T(parse_kscope_identifier), SEQ(T(parse_kscope_eqexpr), SEQ(T(parse_kscope_sep), SEQ(T(parse_kscope_expr), SEQ(QUES(SEQ(T(parse_kscope_sep), T(parse_kscope_expr))), SEQ(T(parse_kscope_in), T(parse_kscope_expr)))))))))

PEG_PARSE(parse_kscope_ifexpr, KSCOPE_IFEXPR_NODE, L"parse_kscope_ifexpr", SEQ(T(parse_kscope_if), SEQ(T(parse_kscope_expr), SEQ(T(parse_kscope_then), SEQ(T(parse_kscope_expr), SEQ(T(parse_kscope_else), T(parse_kscope_expr)))))))

PEG_PARSE(parse_kscope_paren, KSCOPE_PAREN_NODE, L"parse_kscope_paren", SEQ(T(parse_kscope_op), SEQ(T(parse_kscope_expr), T(parse_kscope_cp))))

PEG_PARSE(parse_kscope_idexpr, KSCOPE_IDEXPR_NODE, L"parse_kscope_idexpr", DISJ(SEQ(T(parse_kscope_identifier), T(parse_kscope_call)), T(parse_kscope_identifier)))

PEG_PARSE(parse_kscope_idproto, KSCOPE_IDPROTO_NODE, L"parse_kscope_idproto", SEQ(T(parse_kscope_identifier), SEQ(T(parse_kscope_op), SEQ(T(parse_kscope_protoarg), SEQ(STAR(T(parse_kscope_protoarg)), T(parse_kscope_cp))))))

PEG_PARSE(parse_kscope_binproto, KSCOPE_BINPROTO_NODE, L"parse_kscope_binproto", SEQ(T(parse_kscope_binary_kw), SEQ(T(parse_kscope_operator), SEQ(QUES(T(parse_kscope_number)), SEQ(T(parse_kscope_op), SEQ(T(parse_kscope_protoarg), SEQ(T(parse_kscope_protoarg), T(parse_kscope_cp))))))))

PEG_PARSE(parse_kscope_uniproto, KSCOPE_UNIPROTO_NODE, L"parse_kscope_uniproto", SEQ(T(parse_kscope_unary_kw), SEQ(T(parse_kscope_operator), SEQ(T(parse_kscope_op), SEQ(T(parse_kscope_protoarg), T(parse_kscope_cp))))))

PEG_PARSE(parse_kscope_protoarg, KSCOPE_PROTOARG_NODE, L"parse_kscope_protoarg", T(parse_kscope_identifier))

PEG_PARSE(parse_kscope_call, KSCOPE_CALL_NODE, L"parse_kscope_call", SEQ(T(parse_kscope_op), SEQ(T(parse_kscope_expr), SEQ(STAR(SEQ(T(parse_kscope_sep), T(parse_kscope_expr))), T(parse_kscope_cp)))))

PEG_PARSE(parse_kscope_eqexpr, KSCOPE_EQEXPR_NODE, L"parse_kscope_eqexpr", SEQ(T(parse_kscope_opeql), T(parse_kscope_expr)))

PEG_PARSE(parse_kscope_operator, KSCOPE_OPERATOR_NODE, L"parse_kscope_operator", SEQ(T(parse_kscope_operator_str), T(parse_kscope__)))

PEG_PARSE(parse_kscope_operator_str, KSCOPE_OPERATOR_STR_NODE, L"parse_kscope_operator_str", C(L"-+=*/:<>"))

PEG_PARSE(parse_kscope_unknown, KSCOPE_UNKNOWN_NODE, L"parse_kscope_unknown", SEQ(STAR(DOT), T(parse_kscope_eof)))

PEG_PARSE(parse_kscope_identifier, KSCOPE_IDENTIFIER_NODE, L"parse_kscope_identifier", SEQ(T(parse_kscope_identifier_str), HIDE(T(parse_kscope__))))

PEG_PARSE(parse_kscope_identifier_str, KSCOPE_IDENTIFIER_STR_NODE, L"parse_kscope_identifier_str", SEQ(C(L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"), STAR(C(L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"))))

PEG_PARSE(parse_kscope_number, KSCOPE_NUMBER_NODE, L"parse_kscope_number", SEQ(T(parse_kscope_number_str), T(parse_kscope__)))

PEG_PARSE(parse_kscope_number_str, KSCOPE_NUMBER_STR_NODE, L"parse_kscope_number_str", DISJ(SEQ(PLUS(C(L"0123456789")), QUES(SEQ(S(L"."), STAR(C(L"0123456789"))))), SEQ(S(L"."), PLUS(C(L"0123456789")))))

PEG_PARSE(parse_kscope_letter, KSCOPE_LETTER_NODE, L"parse_kscope_letter", SEQ(BANG(C(L"0123456789. ()\t\n")), DOT))

PEG_PARSE(parse_kscope_lex, KSCOPE_LEX_NODE, L"parse_kscope_lex", S(L"Lexer stuff below"))

PEG_PARSE(parse_kscope_sep, KSCOPE_SEP_NODE, L"parse_kscope_sep", SEQ(S(L","), T(parse_kscope__)))

PEG_PARSE(parse_kscope_opeql, KSCOPE_OPEQL_NODE, L"parse_kscope_opeql", SEQ(S(L"="), T(parse_kscope__)))

PEG_PARSE(parse_kscope_op, KSCOPE_OP_NODE, L"parse_kscope_op", SEQ(S(L"("), T(parse_kscope__)))

PEG_PARSE(parse_kscope_cp, KSCOPE_CP_NODE, L"parse_kscope_cp", SEQ(S(L")"), T(parse_kscope__)))

PEG_PARSE(parse_kscope_def_kw, KSCOPE_DEF_KW_NODE, L"parse_kscope_def_kw", SEQ(S(L"def"), T(parse_kscope__)))

PEG_PARSE(parse_kscope_extern_kw, KSCOPE_EXTERN_KW_NODE, L"parse_kscope_extern_kw", SEQ(S(L"extern"), T(parse_kscope__)))

PEG_PARSE(parse_kscope_if, KSCOPE_IF_NODE, L"parse_kscope_if", SEQ(S(L"if"), T(parse_kscope__)))

PEG_PARSE(parse_kscope_then, KSCOPE_THEN_NODE, L"parse_kscope_then", SEQ(S(L"then"), T(parse_kscope__)))

PEG_PARSE(parse_kscope_else, KSCOPE_ELSE_NODE, L"parse_kscope_else", SEQ(S(L"else"), T(parse_kscope__)))

PEG_PARSE(parse_kscope_for, KSCOPE_FOR_NODE, L"parse_kscope_for", SEQ(S(L"for"), T(parse_kscope__)))

PEG_PARSE(parse_kscope_in, KSCOPE_IN_NODE, L"parse_kscope_in", SEQ(S(L"in"), T(parse_kscope__)))

PEG_PARSE(parse_kscope_binary_kw, KSCOPE_BINARY_KW_NODE, L"parse_kscope_binary_kw", SEQ(S(L"binary"), T(parse_kscope__)))

PEG_PARSE(parse_kscope_unary_kw, KSCOPE_UNARY_KW_NODE, L"parse_kscope_unary_kw", SEQ(S(L"unary"), T(parse_kscope__)))

PEG_PARSE(parse_kscope_var, KSCOPE_VAR_NODE, L"parse_kscope_var", SEQ(S(L"var"), T(parse_kscope__)))

PEG_PARSE(parse_kscope__, KSCOPE___NODE, L"parse_kscope__", STAR(T(parse_kscope_ws)))

PEG_PARSE(parse_kscope_ws, KSCOPE_WS_NODE, L"parse_kscope_ws", DISJ(T(parse_kscope_whitespace), T(parse_kscope_comment)))

PEG_PARSE(parse_kscope_comment, KSCOPE_COMMENT_NODE, L"parse_kscope_comment", SEQ(S(L"#"), SEQ(STAR(SEQ(BANG(S(L"\n")), DOT)), S(L"\n"))))

PEG_PARSE(parse_kscope_whitespace, KSCOPE_WHITESPACE_NODE, L"parse_kscope_whitespace", C(L" ;\t\n"))

PEG_PARSE(parse_kscope_eof, KSCOPE_EOF_NODE, L"parse_kscope_eof", BANG(DOT))

/* _wish_node is a useful last-ditch grammar debugging tool. 
Set it to the node you expected to be recognized but wasn't. Dumps error list to stdout. */
int kscope_wish_node = 32000;

/* main function */

int kscope_parse(char *fname, kscope_syntax_node_t **parse_tree, void **input_buf, void **error_list)
{
    FILE *f;
    input_buffer_t *ib;
    int start_offset, end_offset;
    array_t line_endings;
    memo_map_t *map;
    kscope_syntax_node_t *root = 0;

    f = fopen(fname, "r");
    if (!f) return 0;

    ib = input_buffer_create(fname, f);
    assert(ib);
    *input_buf = ib;

    map = memo_map_create();
    *error_list = (array_t *) calloc(sizeof(array_t), 1);
    array_init(*error_list, sizeof(kscope_error_rec_t), 0);

    start_offset = input_buffer_getpos(ib);
    parse_kscope_file(ib, start_offset, &end_offset, &root, map, *error_list);
    memo_map_destroy(map);

    if (root)
    {
        array_init(&line_endings, sizeof(int), 0);
        find_line_endings(&line_endings, ib, root->begin);
        assign_line_numbers(ib, root, &line_endings);
        array_deinit(&line_endings);
    }

    *parse_tree = root;
    return root != 0;
}

