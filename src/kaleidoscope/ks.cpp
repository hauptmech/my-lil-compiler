#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "kscope.h"
#include <errno.h>
#include <string>
#include <map>
#include <vector>

#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/PassManager.h"
#include "llvm/CallingConv.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
Module* makeLLVMModule();
static Module* MyMod;
static IRBuilder<> Builder;
static std::map<std::string, Value*> NamedValues;
typedef struct _peg_options
{
    char *input_fname;
    char *output_prefix;
}
peg_options;

static void get_options(int argc, char **argv, peg_options *ops)
{
    char buf[128];

    if (argc == 2)
    {
        snprintf(buf, 128, "%s", argv[1]);
        ops->input_fname = strdup(buf);

        snprintf(buf, 128, "%s", argv[1]);
        ops->output_prefix = strdup(buf);
    }
    else
    {
        fprintf(stderr, "usage: cyc file\n");
        exit(1);
    }
} /* get_options() */

static int open_file(char *fname, char *mode, FILE **file)
{
    assert(fname);
    assert(mode);
    assert(file);

    *file = fopen(fname, mode);

    if (!*file)
    {
        fprintf(stderr, "unable to open %s: %s\n", fname, strerror(errno));
        return 0;
    }
    else
    {
        return 1;
    }
} /* open_file() */

int test_dispatch(kscope_syntax_node_t *node, void* data)
{
	printf("Got Called!!\n");
}
int codegen(kscope_syntax_node_t *node, void* data)
{
}
int print_node(kscope_syntax_node_t *node, void* data)
{ int cnt;
	if (node != NULL){
		if (node->type < KSCOPE_LEX_NODE){
			for (cnt = 0;cnt<node->type;cnt++) printf(" ");
           printf("%s ",kscope_node_names[node->type],node->begin,node->end);
	   if (node->type > KSCOPE_STATEMENTS_NODE){
		   printf(">%ls\n",kscope_get_input_string(node,data));
	   }
	   else printf("\n");
	}
	}

	return 0;
}
void print_errors(void* error_list)
{
  int cnt,max;
  kscope_error_rec_t *this_error;

  max = kscope_num_errors(error_list);
  for(cnt = 0;cnt<max;cnt++){
	  this_error = kscope_get_error(error_list,cnt);
	  if (this_error != NULL){
		  printf("\nError %d:%d: %ls",cnt,this_error->pos,this_error->str);
	  }
  }

}
int main(int argc, char* argv[])
{
  
    peg_options ops;
    FILE *input_file = 0;
    int i, len, res = 0;

    kscope_syntax_node_t *root;
    void *input_buffer;
    void *error_list;

    /* get options and open files */
    get_options(argc, argv, &ops);
    printf("FILE:%s",ops.input_fname);
    kscope_dispatch[KSCOPE_STATEMENTS_NODE] = test_dispatch;
    if (kscope_parse(ops.input_fname,&root,&input_buffer,&error_list)){
    printf("\n---------------nodes------------------\n");
    kscope_syntax_node_traverse_preorder(root,input_buffer,print_node);
    }
    printf("\n---------------errors-----------------\n");
    print_errors(error_list);
    printf("\n---------------raw------------------\n");

    kscope_destroy_error_list(error_list);
    kscope_destroy_input_buffer(input_buffer);
   

    Module* Mod = makeLLVMModule();
    verifyModule(*Mod, PrintMessageAction);
    PassManager PM;
    PM.add(createPrintModulePass(&(llvm::outs())));
    PM.run(*Mod);
    delete Mod;

    return 1;
}


Module* makeLLVMModule() {
	Module* mod = new Module("tut2");
/*
	Constant* c = mod->getOrInsertFunction("gcd",IntegerType::get(32),
			IntegerType::get(32),
			IntegerType::get(32), NULL);

	Function* gcd = cast<Function>(c);

	Function::arg_iterator args = gcd->arg_begin();
	Value* x = args++;
	x->setName("x");
	Value* y = args++;
	y->setName("y");

	BasicBlock* entry = BasicBlock::Create("entry",gcd);
        BasicBlock* ret = BasicBlock::Create("return",gcd);
   	BasicBlock* cond_false = BasicBlock::Create("cond_false",gcd);
        BasicBlock* cond_true = BasicBlock::Create("cond_true",gcd);
        BasicBlock* cond_false_2 = BasicBlock::Create("cond_false",gcd);

        IRBuilder<> builder(entry);
        Value* xEqualsY = builder.CreateICmpEQ(x, y, "tmp");
        builder.CreateCondBr(xEqualsY, ret, cond_false);
	
	builder.SetInsertPoint(ret);
	builder.CreateRet(x);

	builder.SetInsertPoint(cond_false);
        Value* xLessThanY = builder.CreateICmpULT(x,y,"tmp");
        builder.CreateCondBr(xLessThanY, cond_true, cond_false_2);

	builder.SetInsertPoint(cond_true);
	Value* yMinusX = builder.CreateSub(y,x,"tmp");
	std::vector<Value*> args1;
	args1.push_back(x);
	args1.push_back(yMinusX);
	Value* recur_1 = builder.CreateCall(gcd, args1.begin(),args1.end(),"tmp");
	builder.CreateRet(recur_1);

	builder.SetInsertPoint(cond_false_2);
	Value* xMinusY = builder.CreateSub(x,y,"tmp");
	std::vector<Value*> args2;
	args2.push_back(xMinusY);
	args2.push_back(y);
	Value* recur_2 = builder.CreateCall(gcd, args2.begin(), args2.end(), "tmp");
	builder.CreateRet(recur_2);

*/	
	

	return mod;
}

