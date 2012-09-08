/*Symbol table, type table, etc */
#include "avl.h"

/***************************** rArray *****************************/
typedef struct rArray_{
  int buffer;
  int length;
  int element_size;
  void* data;
}rArray;
rArray* rA_new(size_t size,size_t count); //Create a new array
int rA_add(rArray* rA, void* data); //Add something to the end
void* rA_get(rArray* rA, int idx);
void rA_put(rArray* rA, void* data, int idx);
void rA_free(rArray* rA);


/*************************** rSymbol ****************************/
typedef char *rName[256];

typedef struct rSymbol_{
  char* Name; 
  struct rSymbol_ *Rent; //We can follow the chain back to get the full name of this symbol.
  struct rTypeEntry_ *Type;
}rSymbol;

int rSymbol_cmp(const void* a,const void* b,void* data);
rSymbol* rS_new(char* name,rSymbol* rent,rSymbol* type);
void rSymbol_print(rSymbol* self,void* dat);
/** Symbol heirarchy
 * 
 * A symbol is a actually a chain of individual symbols all the way back to the top level.
 * You can think of it as a tree of namespaces, whose leaves are the symbols.
 *
 * mymodule.myclass.mymember
 *
 * mymodule <-rent- myclass <-rent- mymember
 */
typedef struct rSymbolTable_{ //Location zero is a special null type
  struct avl_table *ST;
	rSymbol* yygdrasil; //pointer to the parent of all top level symbols
}rSymbolTable;

rSymbolTable* rST_new();
void rST_dump(rSymbolTable *st); //printf table contents
rSymbol* rST_find(rSymbolTable *st,rName name); //find the symbol / parent combo given
rSymbol* rST_add(rSymbolTable* st, rName name);
void rST_free(rSymbolTable* st);


typedef struct rVariable_{ 
  rSymbol* symb; //points to a symbol, from which type can be inferred
  void* value;
}rVariable;


typedef struct rPList_{
	int N;
	rVariable* data;
}rPList;

rPList* rPL_new();
rVariable* rPL_add(rPList* rPL,rVariable* rV);
rVariable* rPL_addnew(rPList* rPL,rSymbol* rS,void* value);

typedef struct rTypeEntry_{
  rSymbol* Name;
  struct rTypeEntry_* Pointee; //0 if this is a value type, else points to type this pointer points to
  size_t size; //number of memory units this type occupies
  rPList* Plist; //parameter list
}rTypeEntry;

typedef struct rTypeTable_{
	struct avl_table *TT;
	rTypeEntry* yygdrasil;
}rTypeTable;

rTypeTable * rTT_new();
void rTT_dump(rTypeTable *tt);
int rTT_add(rTypeTable *tt,rTypeEntry *te);
int rTypeEntry_cmp(const void *a,const void *c,void* data);

typedef struct rUniverse_{ //The state of this execution universe
  rSymbolTable* ST;
  rTypeTable* TT;	
}rUniverse;


