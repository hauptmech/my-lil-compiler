#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "internal.h"
#include "avl.h"
/********************************* rArray ************************************/
rArray* rA_new(size_t size,size_t count)
{
  rArray* p;

  p = (rArray*)malloc(sizeof(rArray)+(count+2)*size);
  p->buffer = count;
  p->element_size = size;
  p->data = p + sizeof(rArray) + size; //leave one element before start of array for cornercase stuff
  p->length = 0; 
}

void* rA_get(rArray* rA, int idx)
{
  if (idx<rA->buffer)
    return rA->data + idx*rA->element_size;
  else 
    return NULL;
}
void rA_put(rArray* rA, void* data, int idx)
{
  void* dest;
  
  dest = rA_get(rA,idx);
  if (dest != NULL)
    memcpy(dest,data,rA->element_size);
  
}
int rA_add(rArray* rA,void* data)
{
  if (rA->length < rA->buffer){
    rA_put(rA,data,rA->length);
    rA->length++;
    return rA->length - 1;
  }
  else{
    return -1;
  }
}
void rA_free(rArray* rA)
{
  if(rA != NULL)
		free(rA);
}
/****************************** rSymbolTable *********************************/
rSymbolTable* rST_new()
{ 
  rSymbolTable* st;
  st = (rSymbolTable*)malloc(sizeof(rSymbolTable));
  st->ST = avl_create(rSymbol_cmp,NULL,NULL);

  rName a = {"",0};
  rSymbol* yygdrasil;
  yygdrasil = rS_new("",NULL,NULL);
  avl_insert(st->ST,yygdrasil);
  st->yygdrasil = yygdrasil;
  rName b = {"symbol",0};
  rST_add(st,b);
  return st;
}

void rST_dump(rSymbolTable* st)
{
  rSymbol* s;
  int idx = 0;
	struct avl_traverser trav;
 
  s = avl_t_first(&trav,st->ST);	

  while (s){
		rSymbol_print(s,NULL);
		s = avl_t_next(&trav);
		idx++;
	}
}	

rSymbol* rST_find(rSymbolTable* st,rName name)
{
	rSymbol *prev,*ret;
	rSymbol targ;
	int idx = 0;

	prev = st->yygdrasil;
	while(name[idx] != 0){
		targ.Name = name[idx];
		targ.Rent = prev;

		ret = avl_find(st->ST,&targ);
		if (ret == NULL) break;

		prev = ret;
		idx++;
	}
	return ret;
}

rSymbol* rST_add(rSymbolTable* st,rName name)
{
	rSymbol *s,*prev;
	rSymbol serch;
	rSymbol **ret;

  int nidx = 0;
  int idx;
	int last_idx = 0;
	prev = st->yygdrasil;

	while(name[nidx] != 0){
		s = rS_new(name[nidx],prev,0); 
    ret = (rSymbol**)avl_probe(st->ST,s);
		if (ret && *ret != s){//already there, free s
	    free(s);
	  }
    else if (!ret) { //error! do something
      fprintf(stderr,"Symbol Table malloc error - rST_add");
		}		
		
    prev = *ret;
		nidx++;
	}
	return prev;

}
void rST_free(rSymbolTable* st)
{
	avl_destroy(st->ST,NULL);
	free(st);
}
//-----------------------------------------------------
rSymbol* rS_new(char* Name,rSymbol* Rent,rSymbol* Type)
{
  rSymbol* self = (rSymbol*)calloc(1,sizeof(rSymbol));
	self->Name = strdup(Name);
	self->Rent = Rent;
	self->Type = Type;
	return self;
}
void rSymbol_print(rSymbol* self,void* dat)
{
  if (self)
	  printf("%X[%X]<%X>%s.\n",self,self->Rent,self->Type,self->Name);
}
int rSymbol_cmp(const void* a,const void *b,void* data)
{
  int res;
  res = strcmp(((rSymbol*)a)->Name,((rSymbol*)b)->Name);
  if (!res) {
    return (((rSymbol*)a)->Rent - ((rSymbol*)b)->Rent);
  }
  return res;
}
//-----------------------------------
int rTypeEntry_cmp(const void *a,const void *b, void* data)
{
	return (int)(a-b);
}

rTypeTable* rTT_new()
{
	rTypeTable *tt;
  tt = (rTypeTable*)malloc(sizeof(rTypeTable));
  tt->TT = avl_create(rTypeEntry_cmp,NULL,NULL);
  return tt;
}

int main()
{
  	rSymbolTable *st;
	struct avl_table *tree;
	rSymbol* sp;
	tree = avl_create(rSymbol_cmp,NULL,NULL);

  

	rName a = {"module","class1","name1",0};
	rName b = {"module","class1","name2",0};
	rName c = {"module","class2","name2","name1",0};
	rName d = {"bubba",0};
	rName e = {"==",0};

	char* s1 = "bubba";
	char* s2 = "chubba";
	char* s3 = "dharma";
	char* s4 = "erg";
	st = rST_new();
  
	rST_add(st,a);
	rST_add(st,b);
	rST_add(st,c);
	rST_add(st,d);
	rST_add(st,e);
	rST_dump(st); 
  
	sp = rST_find(st,c);
	rSymbol_print(sp,NULL);
  	rST_free(st);
	return 0;
}


/*Symbol table design*/
/* 
 * find_or_add() //
 * find() //if symbol not found, return null
 * unique() //figure out if subsymbol is a unique one
 * find_all() // find all instances of a symbol
 * add() // return null if it already exists
 */

/* Type Table
 *
 * Add new type
 */



