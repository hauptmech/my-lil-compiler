#ifndef KSCOPE_KSCOPE_H
#define KSCOPE_KSCOPE_H

/*
 * generated Mon Mar 30 21:04:44 2009
 */

#ifdef WIN32
#pragma warning(disable : 4996)
#define _CRT_SECURE_NO_DEPRECATE
#define snprintf _snprintf
#endif

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

/* node types in the abstract syntax tree */

enum kscope_node_type 
{
	KSCOPE_NULL_NODE = 0,
	KSCOPE_FILE_NODE = 1,
	KSCOPE_STATEMENT_NODE = 2,
	KSCOPE_DEFN_NODE = 3,
	KSCOPE_EXTERN_NODE = 4,
	KSCOPE_PROTO_NODE = 5,
	KSCOPE_EXPR_NODE = 6,
	KSCOPE_BINOPRHS_NODE = 7,
	KSCOPE_UNARY_NODE = 8,
	KSCOPE_PRIMARY_NODE = 9,
	KSCOPE_VAREXPR_NODE = 10,
	KSCOPE_FOREXPR_NODE = 11,
	KSCOPE_IFEXPR_NODE = 12,
	KSCOPE_PAREN_NODE = 13,
	KSCOPE_IDEXPR_NODE = 14,
	KSCOPE_IDPROTO_NODE = 15,
	KSCOPE_BINPROTO_NODE = 16,
	KSCOPE_UNIPROTO_NODE = 17,
	KSCOPE_PROTOARG_NODE = 18,
	KSCOPE_CALL_NODE = 19,
	KSCOPE_EQEXPR_NODE = 20,
	KSCOPE_OPERATOR_NODE = 21,
	KSCOPE_OPERATOR_STR_NODE = 22,
	KSCOPE_UNKNOWN_NODE = 23,
	KSCOPE_IDENTIFIER_NODE = 24,
	KSCOPE_IDENTIFIER_STR_NODE = 25,
	KSCOPE_NUMBER_NODE = 26,
	KSCOPE_NUMBER_STR_NODE = 27,
	KSCOPE_LETTER_NODE = 28,
	KSCOPE_LEX_NODE = 29,
	KSCOPE_SEP_NODE = 30,
	KSCOPE_OPEQL_NODE = 31,
	KSCOPE_OP_NODE = 32,
	KSCOPE_CP_NODE = 33,
	KSCOPE_DEF_KW_NODE = 34,
	KSCOPE_EXTERN_KW_NODE = 35,
	KSCOPE_IF_NODE = 36,
	KSCOPE_THEN_NODE = 37,
	KSCOPE_ELSE_NODE = 38,
	KSCOPE_FOR_NODE = 39,
	KSCOPE_IN_NODE = 40,
	KSCOPE_BINARY_KW_NODE = 41,
	KSCOPE_UNARY_KW_NODE = 42,
	KSCOPE_VAR_NODE = 43,
	KSCOPE___NODE = 44,
	KSCOPE_WS_NODE = 45,
	KSCOPE_COMMENT_NODE = 46,
	KSCOPE_WHITESPACE_NODE = 47,
	KSCOPE_EOF_NODE = 48,
	KSCOPE_NUM_NODE_TYPES = 49
};

extern const char *kscope_node_names[49];
/* nodes in the abstract syntax tree */

typedef struct _kscope_syntax_node_t
{
    int type;                  /* type of node; this is defined above.                    */
    int begin;                 /* input position before the first character of the match  */
    int end;                   /* input position after the last character of the match    */
    int first_line;            /* line on which the match begins                          */
    int last_line;             /* line on which the match ends                            */
    int children;            /* number of children*/                                      
    struct _kscope_syntax_node_t **child; /* null-terminated array of child nodes                    */
    void *data;
    void *ib; /*Pointer to text buffer*/
}
kscope_syntax_node_t;

extern kscope_syntax_node_t *kscope_syntax_node_create(int type, int begin, int end, void* ib);
extern kscope_syntax_node_t *kscope_syntax_node_copy(kscope_syntax_node_t *node);
extern void kscope_syntax_node_destroy(kscope_syntax_node_t *node);
extern int kscope_syntax_node_children(kscope_syntax_node_t *node); /*Returns number of children this node has*/
extern kscope_syntax_node_t *kscope_syntax_node_child(kscope_syntax_node_t *node,int idx);/*Returns child indicated by idx*/
typedef int (*kscope_syntax_node_process_ft)(kscope_syntax_node_t *node, void *data);
extern void kscope_syntax_node_traverse_preorder(kscope_syntax_node_t *root, void *data, kscope_syntax_node_process_ft entry_func,kscope_syntax_node_process_ft exit_func);
extern  kscope_syntax_node_process_ft kscope_dispatch[49];

/* error handling */

typedef struct _kscope_error_rec_t
{
    int pos;
    wchar_t *str;
}
kscope_error_rec_t;

extern int kscope_wish_node;
extern void kscope_destroy_error_list(void *error_list); /* free a list of errors */
extern void kscope_add_error(void *error_list, int pos, wchar_t *str); /* add an error to the list of errors */
extern int kscope_num_errors(void *error_list); /* returns the number of errors in the list */
extern kscope_error_rec_t *kscope_get_error(void *error_list, int index);

/* input buffers */

extern wchar_t *kscope_get_wstr(kscope_syntax_node_t *node);
extern char *kscope_get_str(kscope_syntax_node_t *node);
extern void kscope_destroy_input_buffer(void *ib);

/* main parse function */

extern int kscope_parse(char *fname, kscope_syntax_node_t **parse_tree, void **input_buffer, void **error_list);

#ifdef __cplusplus
}
#endif

#endif
