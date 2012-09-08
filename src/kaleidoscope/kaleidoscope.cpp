#include "llvm/DerivedTypes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <map>
#include <vector>
#include "kscope.h"
#include <deque>

using namespace llvm;
using namespace std;

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {
  tok_eof = -1,

  // commands
  tok_def = -2, tok_extern = -3,

  // primary
  tok_identifier = -4, tok_number = -5,
  
  // control
  tok_if = -6, tok_then = -7, tok_else = -8,
  tok_for = -9, tok_in = -10,
  
  // operators
  tok_binary = -11, tok_unary = -12,
  
  // var definition
  tok_var = -13
};

static std::string IdentifierStr;  // Filled in if tok_identifier
static double NumVal;              // Filled in if tok_number

/// gettok - Return the next token from standard input.
static int gettok() {
  static int LastChar = ' ';

  // Skip any whitespace.
  while (isspace(LastChar))
    LastChar = getchar();

  if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
    IdentifierStr = LastChar;
    while (isalnum((LastChar = getchar())))
      IdentifierStr += LastChar;

    if (IdentifierStr == "def") return tok_def;
    if (IdentifierStr == "extern") return tok_extern;
    if (IdentifierStr == "if") return tok_if;
    if (IdentifierStr == "then") return tok_then;
    if (IdentifierStr == "else") return tok_else;
    if (IdentifierStr == "for") return tok_for;
    if (IdentifierStr == "in") return tok_in;
    if (IdentifierStr == "binary") return tok_binary;
    if (IdentifierStr == "unary") return tok_unary;
    if (IdentifierStr == "var") return tok_var;
    return tok_identifier;
  }

  if (isdigit(LastChar) || LastChar == '.') {   // Number: [0-9.]+
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = getchar();
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(NumStr.c_str(), 0);
    return tok_number;
  }

  if (LastChar == '#') {
    // Comment until end of line.
    do LastChar = getchar();
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
    
    if (LastChar != EOF)
      return gettok();
  }
  
  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF)
    return tok_eof;

  // Otherwise, just return the character as its ascii value.
  int ThisChar = LastChar;
  LastChar = getchar();
  return ThisChar;
}

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
  virtual ~ExprAST() {}
  virtual Value *Codegen() = 0;
  virtual void Print() {}
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
  double Val;
public:
  NumberExprAST(double val) : Val(val) {}
  virtual Value *Codegen();
  void Print(){
	  printf("Val[%lf]",Val);
  }
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
  std::string Name;
public:
  VariableExprAST(const std::string &name) : Name(name) {}
  const std::string &getName() const { return Name; }
  virtual Value *Codegen();
  void Print(){
	  printf("Var[%s]",Name.c_str());
  }
};

/// UnaryExprAST - Expression class for a unary operator.
class UnaryExprAST : public ExprAST {
  char Opcode;
  ExprAST *Operand;
public:
  UnaryExprAST(char opcode, ExprAST *operand) 
    : Opcode(opcode), Operand(operand) {}
  virtual Value *Codegen();
  void Print(){
	  printf(" Unary[%c]",Opcode);
	  Operand->Print();
  }
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
  char Op;
  ExprAST *LHS, *RHS;
public:
  BinaryExprAST(char op, ExprAST *lhs, ExprAST *rhs) 
    : Op(op), LHS(lhs), RHS(rhs) {}
  virtual Value *Codegen();
  void Print(){
	  printf(" Bin[%c](",Op);
	  if (LHS)
	  LHS->Print();
	  printf(" - ");
	  if (RHS)
	  RHS->Print();
	  printf(")");
  }
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<ExprAST*> Args;
public:
  CallExprAST(const std::string &callee, std::vector<ExprAST*> &args)
    : Callee(callee), Args(args) {}
  virtual Value *Codegen();
  void Print(){
	  printf("CallExpr:");
	  int cnt;
	  for(cnt = 0; cnt<Args.size();cnt++)
		  Args[cnt]->Print();
	  printf("\n");
  }
};

/// IfExprAST - Expression class for if/then/else.
class IfExprAST : public ExprAST {
  ExprAST *Cond, *Then, *Else;
public:
  IfExprAST(ExprAST *cond, ExprAST *then, ExprAST *_else)
  : Cond(cond), Then(then), Else(_else) {}
  virtual Value *Codegen();
  void Print(){
	  printf("If:\n");
	  Cond->Print();
	  printf("Then:\n");
	  Then->Print();
	  printf("Else:\n");
	  Else->Print();
  }
};

/// ForExprAST - Expression class for for/in.
class ForExprAST : public ExprAST {
  std::string VarName;
  ExprAST *Start, *End, *Step, *Body;
public:
  ForExprAST(const std::string &varname, ExprAST *start, ExprAST *end,
	     ExprAST *step, ExprAST *body)
    : VarName(varname), Start(start), End(end), Step(step), Body(body) {}
  virtual Value *Codegen();
  void Print(){
	  printf("For:\n");
	  Start->Print();
	  printf("end:\n");
	  End->Print();
	  printf("step:\n");
	  if(Step) Step->Print();
	  printf("Body:\n");
	  Body->Print();
  }
};

/// VarExprAST - Expression class for var/in
class VarExprAST : public ExprAST {
  std::vector<std::pair<std::string, ExprAST*> > VarNames;
  ExprAST *Body;
public:
  VarExprAST(const std::vector<std::pair<std::string, ExprAST*> > &varnames,
	     ExprAST *body)
  : VarNames(varnames), Body(body) {}
  
  virtual Value *Codegen();
  void Print(){
	  printf("VarExpr:\n");
	  int cnt;
	  for(cnt = 0; cnt<VarNames.size();cnt++){
		  printf("(%s)=",VarNames[cnt].first.c_str());
		  if (VarNames[cnt].second) VarNames[cnt].second->Print();
	  }
	  printf("\nBody:\n");
	  Body->Print();
  }
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes), as well as if it is an operator.
class PrototypeAST {
  std::string Name;
  std::vector<std::string> Args;
  bool isOperator;
  unsigned Precedence;  // Precedence if a binary op.
public:
  PrototypeAST(const std::string &name, const std::vector<std::string> &args,
	       bool isoperator = false, unsigned prec = 0)
  : Name(name), Args(args), isOperator(isoperator), Precedence(prec) {}
  
  bool isUnaryOp() const { return isOperator && Args.size() == 1; }
  bool isBinaryOp() const { return isOperator && Args.size() == 2; }
  
  char getOperatorName() const {
    assert(isUnaryOp() || isBinaryOp());
    return Name[Name.size()-1];
  }
  
  unsigned getBinaryPrecedence() const { return Precedence; }
  
  Function *Codegen();
  
  void CreateArgumentAllocas(Function *F);
  void Print(){
	  printf("Prototype:[%s|%d]",Name.c_str(),Precedence);
	  int cnt;
	  for(cnt = 0; cnt<Args.size();cnt++)
		  printf("(%s)",Args[cnt].c_str());
	  printf("\n");
  }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
  PrototypeAST *Proto;
  ExprAST *Body;
public:
  FunctionAST(PrototypeAST *proto, ExprAST *body)
    : Proto(proto), Body(body) {}
  
  Function *Codegen();

  void Print(){
	  printf("Function:");
	  Proto->Print();
	  printf("Body:{");
	  Body->Print();
	  printf("}\n\n");
  }
};

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
static int CurTok;
static int getNextToken() {
  return CurTok = gettok();
}

/// BinopPrecedence - This holds the precedence for each binary operator that is
/// defined.
static std::map<char, int> BinopPrecedence;

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence(int ct) {
  if (!isascii(ct))
    return -1;
  
  // Make sure it's a declared binop.
  int TokPrec = BinopPrecedence[ct];
  if (TokPrec <= 0) return -1;
  return TokPrec;
}

/// Error* - These are little helper functions for error handling.
ExprAST *Error(const char *Str) { fprintf(stderr, "Error: %s\n", Str);return 0;}
PrototypeAST *ErrorP(const char *Str) { Error(Str); return 0; }
FunctionAST *ErrorF(const char *Str) { Error(Str); return 0; }

static ExprAST *ParseExpression(kscope_syntax_node_t *root,void* data) ;

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
static ExprAST *ParseIdentifierExpr(kscope_syntax_node_t *root,void* data) {

  std::string IdName = kscope_get_str(root->child[0]->child[0]);
  
  
  if (root->children == 1) // Simple variable ref.
    return new VariableExprAST(IdName);
  
  // Call.
  std::vector<ExprAST*> Args;
  if (root->child[1]->child[1]->type != KSCOPE_CP_NODE) {
	  int idx = 1;
    while (1) {
      ExprAST *Arg = ParseExpression(root->child[1]->child[idx],data);
      if (!Arg){
		 fprintf(stderr,"Error: Parse Arg");
		 return 0;
	  }
      Args.push_back(Arg);
      
      if (root->child[1]->child[idx+1]->type == KSCOPE_CP_NODE) break;
      idx+=2;  
    }
  }

  // Eat the ')'.
  
  return new CallExprAST(IdName, Args);
}

/// numberexpr ::= number
static ExprAST *ParseNumberExpr(kscope_syntax_node_t *root,void* data) {
  double Val = strtod(kscope_get_str(root->child[0]),NULL);
  ExprAST *Result = new NumberExprAST(Val);
  return Result;
}

/// parenexpr ::= '(' expression ')'
static ExprAST *ParseParenExpr(kscope_syntax_node_t *root,void* data) {
  ExprAST *V = ParseExpression(root->child[1],data);
  if (!V) return 0;
  return V;
}

/// ifexpr ::= 'if' expression 'then' expression 'else' expression
static ExprAST *ParseIfExpr(kscope_syntax_node_t *root,void* data) {
  
  // condition.
  ExprAST *Cond = ParseExpression(root->child[1],data);
  if (!Cond) return 0;
  
  ExprAST *Then = ParseExpression(root->child[3],data);
  if (Then == 0) return 0;
  
  ExprAST *Else = ParseExpression(root->child[5],data);
  if (!Else) return 0;
  
  return new IfExprAST(Cond, Then, Else);
}

/// forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
static ExprAST *ParseForExpr(kscope_syntax_node_t *root,void* data) {

  kscope_syntax_node_t *node;
	
  std::string IdName = kscope_get_str(root->child[1]->child[0]);
  
  ExprAST *Start = ParseExpression(root->child[2]->child[1],data);
  if (Start == 0) return 0;
  
  ExprAST *End = ParseExpression(root->child[4],data);
  if (End == 0) return 0;
  
  // The step value is optional.
  ExprAST *Step = 0;
  if (root->child[5]->type == KSCOPE_SEP_NODE) {
    Step = ParseExpression(root->child[6],data);
	node = root->child[8];
    if (Step == 0) return 0;
  }
  else node = root->child[6];
  
  ExprAST *Body = ParseExpression(node,data);
  if (Body == 0) return 0;

  return new ForExprAST(IdName, Start, End, Step, Body);
}

/// varexpr ::= 'var' identifier ('=' expression)? 
//                    (',' identifier ('=' expression)?)* 'in' expression
static ExprAST *ParseVarExpr(kscope_syntax_node_t *root,void* data) {

  std::vector<std::pair<std::string, ExprAST*> > VarNames;

  
  int idx = 1;
  while (1) {
    std::string Name = kscope_get_str(root->child[idx]->child[0]);

    ExprAST *Init = 0;
    if (root->child[idx+1]->type == KSCOPE_EQEXPR_NODE) {
      Init = ParseExpression(root->child[idx+1]->child[1],data);
      if (Init == 0) return 0;
	  idx++;
    }
    
    VarNames.push_back(std::make_pair(Name, Init));
    
    if (root->child[idx+1]->type!=KSCOPE_SEP_NODE) break;
    idx+=2;  
  }
   
  
  ExprAST *Body = ParseExpression(root->child[idx+2],data);
  if (Body == 0) return 0;
  
  return new VarExprAST(VarNames, Body);
}


/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
///   ::= ifexpr
///   ::= forexpr
///   ::= varexpr
static ExprAST *ParsePrimary(kscope_syntax_node_t *root,void* data) {
	kscope_syntax_node_t *node;
	node = root->child[0];
  switch (node->type) {
	  default: fprintf(stderr,"Parse Primary Got:%s ",kscope_node_names[node->type]);return 0;
  case KSCOPE_IDEXPR_NODE: return ParseIdentifierExpr(node,data);
  case KSCOPE_NUMBER_NODE:     return ParseNumberExpr(node,data);
  case KSCOPE_PAREN_NODE:            return ParseParenExpr(node,data);
  case KSCOPE_IFEXPR_NODE:         return ParseIfExpr(node,data);
  case KSCOPE_FOREXPR_NODE:        return ParseForExpr(node,data);
  case KSCOPE_VAREXPR_NODE:        return ParseVarExpr(node,data);
  }
}

/// unary
///   ::= primary
///   ::= '!' unary
static ExprAST *ParseUnary(kscope_syntax_node_t *root,void* data) {
  // If the current token is not an operator, it must be a primary expr.
  
  if (root->child[0]->type == KSCOPE_PRIMARY_NODE)
    return ParsePrimary(root->child[0],data);
  
  // If this is a unary operator, read it.
  if (ExprAST *Operand = ParseUnary(root->child[1],data))
    return new UnaryExprAST(kscope_get_str(root->child[0]->child[0])[0], Operand);
  fprintf(stderr,"Error: ParseUnary\n");
  return 0;
}

/// binoprhs
///   ::= ('+' unary)*
static ExprAST *ParseBinOpRHS(int ExprPrec, ExprAST *LHS,kscope_syntax_node_t *root,void* data) {
  // If this is a binop, find its precedence.
 // if (root->children == 0)
//	  return LHS;
 // if (root->children < idx+2)
//	  return 0;

  if (root->children == 0) return LHS;

  deque<int> Ops;
  int Op;
  int ct;
  deque<ExprAST*> Unis;
  ExprAST *Right,*Left;

  Unis.push_back(LHS);
  Right = ParseUnary(root->child[1],data);
  Unis.push_back(Right);
  

  ct = kscope_get_str(root->child[0]->child[0])[0];

  Ops.push_back(ct);

  int idx = 2;
  int ThisPrec,NextPrec;

  ThisPrec = GetTokPrecedence(ct);
  if (ThisPrec < 1) fprintf(stderr,"Operator precedence unexpectedly low!!!\n");

  //Move forward through expression, consolidating terms when we can.
  while (idx < root->children){
    
      ct = kscope_get_str(root->child[idx]->child[0])[0];
      NextPrec = GetTokPrecedence(ct);
        
	  if (ThisPrec >= NextPrec){
         int ThisOp = Ops.back();Ops.pop_back();
		 while (Ops.size() && GetTokPrecedence(ThisOp) > GetTokPrecedence(Ops.back())) {
    		 Right = Unis.back();Unis.pop_back();
	    	 Left = Unis.back();Unis.pop_back();
    		 Op = ThisOp;
    		Left = new BinaryExprAST(Op,Left,Right);
    	    Unis.push_back(Left);	
			ThisOp = Ops.back();Ops.pop_back();
		}
		 Ops.push_back(ThisOp);
	  }

	  Ops.push_back(ct);
	  Right = ParseUnary(root->child[idx+1],data);
      Unis.push_back(Right);


	  ThisPrec = NextPrec;
      idx+=2;


  }
  //Consolidate the last term if necessary 
  int ThisOp = Ops.back();Ops.pop_back();
  while (Ops.size() && GetTokPrecedence(ThisOp) > GetTokPrecedence(Ops.back())) {
 	 Right = Unis.back();Unis.pop_back();
   	 Left = Unis.back();Unis.pop_back();
	 Op = ThisOp;
     Left = new BinaryExprAST(Op,Left,Right);
     Unis.push_back(Left);	
     ThisOp = Ops.back();Ops.pop_back();
  }
  Ops.push_back(ThisOp);
  //There should only be top level terms left. Do them left to right.
  while (Ops.size()){

    Left = Unis.front();Unis.pop_front();
    Right = Unis.front();Unis.pop_front();
    Op = Ops.front();Ops.pop_front();
    Left =  new BinaryExprAST(Op,Left,Right);
    Unis.push_front(Left);
  }
  Right = Unis.front();
  return Right;
}

/// expression
///   ::= unary binoprhs
///
static ExprAST *ParseExpression(kscope_syntax_node_t *root,void* data) {
  ExprAST *LHS = ParseUnary(root->child[0],data);
  if (!LHS) {
	  fprintf(stderr,"Error: ParseExpression LHS\n");
	  return 0;
  }
  
  return ParseBinOpRHS(0, LHS,root->child[1],data);
}

/// prototype
///   ::= id '(' id* ')'
///   ::= binary LETTER number? (id, id)
///   ::= unary LETTER (id)
static PrototypeAST *ParsePrototype(kscope_syntax_node_t *root,void* data) {
  std::string FnName;
  
  int Kind = 0;  // 0 = identifier, 1 = unary, 2 = binary.
  unsigned BinaryPrecedence = 30;
  kscope_syntax_node_t *node;
  node = root->child[0];
  switch (node->type) {
  default:
    return ErrorP("Expected function name in prototype");
  case KSCOPE_IDPROTO_NODE:
    FnName = kscope_get_str(node->child[0]->child[0]);
    Kind = 0;
    break;
  case KSCOPE_UNIPROTO_NODE:
    FnName = "unary";
    FnName += kscope_get_str(node->child[1]->child[0]);
    Kind = 1;
    break;
  case KSCOPE_BINPROTO_NODE:
    FnName = "binary";
    FnName += kscope_get_str(node->child[1]->child[0]);
    Kind = 2;
    
    if (node->child[2]->type == KSCOPE_NUMBER_NODE){
      BinaryPrecedence = (unsigned)strtod(kscope_get_str(node->child[2]->child[0]),NULL);
    }
    break;
  }
  
  
  std::vector<std::string> ArgNames;
  int cnt;
  for (cnt = 0; cnt < node->children;cnt++){
    if (node->child[cnt]->type == KSCOPE_PROTOARG_NODE){
      ArgNames.push_back(kscope_get_str(node->child[cnt]->child[0]->child[0]));
    }
  }
  
// Verify right number of names for operator.
  if (Kind && ArgNames.size() != Kind)
    return ErrorP("Invalid number of operands for operator");
  
  return new PrototypeAST(FnName, ArgNames, Kind != 0, BinaryPrecedence);
}

/// definition ::= 'def' prototype expression
static FunctionAST *ParseDefinition(kscope_syntax_node_t *root,void* data) {
  PrototypeAST *Proto = ParsePrototype(root->child[1],data);
  if (Proto == 0) {
	  	fprintf(stderr,"Error: ParseDefinition-proto\n");
		return 0;
  }

  if (ExprAST *E = ParseExpression(root->child[2],data))
    return new FunctionAST(Proto, E);
  
  fprintf(stderr,"Error: ParseDefinition\n");
  return 0;
}

/// toplevelexpr ::= expression
static FunctionAST *ParseTopLevelExpr(kscope_syntax_node_t *root,void* data) {
  if (ExprAST *E = ParseExpression(root,data)) {
    // Make an anonymous proto.
    PrototypeAST *Proto = new PrototypeAST("", std::vector<std::string>());
    return new FunctionAST(Proto, E);
  }
  return 0;
}

/// external ::= 'extern' prototype
static PrototypeAST *ParseExtern(kscope_syntax_node_t *root,void* data)  {
  return ParsePrototype(root->child[1],data);
}

//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

static Module *TheModule;
static IRBuilder<> Builder(getGlobalContext());
static std::map<std::string, AllocaInst*> NamedValues;
static FunctionPassManager *TheFPM;

Value *ErrorV(const char *Str) { Error(Str); return 0; }

/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
/// the function.  This is used for mutable variables etc.
static AllocaInst *CreateEntryBlockAlloca(Function *TheFunction,
					  const std::string &VarName) {
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
		 TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(Type::getDoubleTy(getGlobalContext()), 0,
			   VarName.c_str());
}


Value *NumberExprAST::Codegen() {
  return ConstantFP::get(getGlobalContext(), APFloat(Val));
}

Value *VariableExprAST::Codegen() {
  // Look this variable up in the function.
  Value *V = NamedValues[Name];
  if (V == 0){
	  fprintf(stderr,"Unknown Var Name:[%s]",Name.c_str());
	  return 0;
  }

  // Load the value.
  return Builder.CreateLoad(V, Name.c_str());
}

Value *UnaryExprAST::Codegen() {
  Value *OperandV = Operand->Codegen();
  if (OperandV == 0) return 0;
  
  Function *F = TheModule->getFunction(std::string("unary")+Opcode);
  if (F == 0)
    return ErrorV("Unknown unary operator");
  
  return Builder.CreateCall(F, OperandV, "unop");
}


Value *BinaryExprAST::Codegen() {
  // Special case '=' because we don't want to emit the LHS as an expression.
  if (Op == '=') {
    // Assignment requires the LHS to be an identifier.
    VariableExprAST *LHSE = static_cast<VariableExprAST*>(LHS);
    if (!LHSE)
      return ErrorV("destination of '=' must be a variable");
    // Codegen the RHS.
    Value *Val = RHS->Codegen();
    if (Val == 0) return 0;

    // Look up the name.
    Value *Variable = NamedValues[LHSE->getName()];
    if (Variable == 0) return ErrorV("Unknown variable name");

    Builder.CreateStore(Val, Variable);
    return Val;
  }
  
  
  Value *L = LHS->Codegen();
  Value *R = RHS->Codegen();
  if (L == 0 || R == 0) return 0;
  
  switch (Op) {
  case '+': return Builder.CreateFAdd(L, R, "addtmp");
  case '-': return Builder.CreateFSub(L, R, "subtmp");
  case '*': return Builder.CreateFMul(L, R, "multmp");
  case '<':
    L = Builder.CreateFCmpULT(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()),
				"booltmp");
  default: break;
  }
  
  // If it wasn't a builtin binary operator, it must be a user defined one. Emit
  // a call to it.
  Function *F = TheModule->getFunction(std::string("binary")+Op);
  assert(F && "binary operator not found!");
  
  Value *Ops[2] = { L, R };
  return Builder.CreateCall(F, Ops, "binop");
}

Value *CallExprAST::Codegen() {
  // Look up the name in the global module table.
  Function *CalleeF = TheModule->getFunction(Callee);
  if (CalleeF == 0)
    return ErrorV("Unknown function referenced");
  
  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return ErrorV("Incorrect # arguments passed");

  std::vector<Value*> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->Codegen());
    if (ArgsV.back() == 0) return 0;
  }
  
  return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

Value *IfExprAST::Codegen() {
  Value *CondV = Cond->Codegen();
  if (CondV == 0) return 0;
  
  // Convert condition to a bool by comparing equal to 0.0.
  CondV = Builder.CreateFCmpONE(CondV, 
			      ConstantFP::get(getGlobalContext(), APFloat(0.0)),
				"ifcond");
  
  Function *TheFunction = Builder.GetInsertBlock()->getParent();
  
  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "then", TheFunction);
  BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "else");
  BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");
  
  Builder.CreateCondBr(CondV, ThenBB, ElseBB);
  
  // Emit then value.
  Builder.SetInsertPoint(ThenBB);
  
  Value *ThenV = Then->Codegen();
  if (ThenV == 0) return 0;
  
  Builder.CreateBr(MergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  ThenBB = Builder.GetInsertBlock();
  
  // Emit else block.
  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder.SetInsertPoint(ElseBB);
  
  Value *ElseV = Else->Codegen();
  if (ElseV == 0) return 0;
  
  Builder.CreateBr(MergeBB);
  // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
  ElseBB = Builder.GetInsertBlock();
  
  // Emit merge block.
  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder.SetInsertPoint(MergeBB);
  PHINode *PN = Builder.CreatePHI(Type::getDoubleTy(getGlobalContext()),2,
				  "iftmp");
  
  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);
  return PN;
}

Value *ForExprAST::Codegen() {
  // Output this as:
  //   var = alloca double
  //   ...
  //   start = startexpr
  //   store start -> var
  //   goto loop
  // loop: 
  //   ...
  //   bodyexpr
  //   ...
  // loopend:
  //   step = stepexpr
  //   endcond = endexpr
  //
  //   curvar = load var
  //   nextvar = curvar + step
  //   store nextvar -> var
  //   br endcond, loop, endloop
  // outloop:
  
  Function *TheFunction = Builder.GetInsertBlock()->getParent();

  // Create an alloca for the variable in the entry block.
  AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
  
  // Emit the start code first, without 'variable' in scope.
  Value *StartVal = Start->Codegen();
  if (StartVal == 0) return 0;
  
  // Store the value into the alloca.
  Builder.CreateStore(StartVal, Alloca);
  
  // Make the new basic block for the loop header, inserting after current
  // block.
  BasicBlock *LoopBB = BasicBlock::Create(getGlobalContext(), "loop", TheFunction);
  
  // Insert an explicit fall through from the current block to the LoopBB.
  Builder.CreateBr(LoopBB);

  // Start insertion in LoopBB.
  Builder.SetInsertPoint(LoopBB);
  
  // Within the loop, the variable is defined equal to the PHI node.  If it
  // shadows an existing variable, we have to restore it, so save it now.
  AllocaInst *OldVal = NamedValues[VarName];
  NamedValues[VarName] = Alloca;
  
  // Emit the body of the loop.  This, like any other expr, can change the
  // current BB.  Note that we ignore the value computed by the body, but don't
  // allow an error.
  if (Body->Codegen() == 0)
    return 0;
  
  // Emit the step value.
  Value *StepVal;
  if (Step) {
    StepVal = Step->Codegen();
    if (StepVal == 0) return 0;
  } else {
    // If not specified, use 1.0.
    StepVal = ConstantFP::get(getGlobalContext(), APFloat(1.0));
  }
  
  // Compute the end condition.
  Value *EndCond = End->Codegen();
  if (EndCond == 0) return EndCond;
  
  // Reload, increment, and restore the alloca.  This handles the case where
  // the body of the loop mutates the variable.
  Value *CurVar = Builder.CreateLoad(Alloca, VarName.c_str());
  Value *NextVar = Builder.CreateFAdd(CurVar, StepVal, "nextvar");
  Builder.CreateStore(NextVar, Alloca);
  
  // Convert condition to a bool by comparing equal to 0.0.
  EndCond = Builder.CreateFCmpONE(EndCond, 
			      ConstantFP::get(getGlobalContext(), APFloat(0.0)),
				  "loopcond");
  
  // Create the "after loop" block and insert it.
  BasicBlock *AfterBB = BasicBlock::Create(getGlobalContext(), "afterloop", TheFunction);
  
  // Insert the conditional branch into the end of LoopEndBB.
  Builder.CreateCondBr(EndCond, LoopBB, AfterBB);
  
  // Any new code will be inserted in AfterBB.
  Builder.SetInsertPoint(AfterBB);
  
  // Restore the unshadowed variable.
  if (OldVal)
    NamedValues[VarName] = OldVal;
  else
    NamedValues.erase(VarName);

  
  // for expr always returns 0.0.
  return Constant::getNullValue(Type::getDoubleTy(getGlobalContext()));
}

Value *VarExprAST::Codegen() {
  std::vector<AllocaInst *> OldBindings;
  
  Function *TheFunction = Builder.GetInsertBlock()->getParent();

  // Register all variables and emit their initializer.
  for (unsigned i = 0, e = VarNames.size(); i != e; ++i) {
    const std::string &VarName = VarNames[i].first;
    ExprAST *Init = VarNames[i].second;
    
    // Emit the initializer before adding the variable to scope, this prevents
    // the initializer from referencing the variable itself, and permits stuff
    // like this:
    //  var a = 1 in
    //    var a = a in ...   # refers to outer 'a'.
    Value *InitVal;
    if (Init) {
      InitVal = Init->Codegen();
      if (InitVal == 0) return 0;
    } else { // If not specified, use 0.0.
      InitVal = ConstantFP::get(getGlobalContext(), APFloat(0.0));
    }
    
    AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
    Builder.CreateStore(InitVal, Alloca);

    // Remember the old variable binding so that we can restore the binding when
    // we unrecurse.
    OldBindings.push_back(NamedValues[VarName]);
    
    // Remember this binding.
    NamedValues[VarName] = Alloca;
  }
  
  // Codegen the body, now that all vars are in scope.
  Value *BodyVal = Body->Codegen();
  if (BodyVal == 0) return 0;
  
  // Pop all our variables from scope.
  for (unsigned i = 0, e = VarNames.size(); i != e; ++i)
    NamedValues[VarNames[i].first] = OldBindings[i];

  // Return the body computation.
  return BodyVal;
}


Function *PrototypeAST::Codegen() {
  // Make the function type:  double(double,double) etc.
  std::vector<Type*> Doubles(Args.size(),
				   Type::getDoubleTy(getGlobalContext()));
  FunctionType *FT = FunctionType::get(Type::getDoubleTy(getGlobalContext()),
				       Doubles, false);
  
  Function *F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule);
  
  // If F conflicted, there was already something named 'Name'.  If it has a
  // body, don't allow redefinition or reextern.
  if (F->getName() != Name) {
    // Delete the one we just made and get the existing one.
    F->eraseFromParent();
    F = TheModule->getFunction(Name);
    
    // If F already has a body, reject this.
    if (!F->empty()) {
      ErrorF("redefinition of function");
      return 0;
    }
    
    // If F took a different number of args, reject.
    if (F->arg_size() != Args.size()) {
      ErrorF("redefinition of function with different # args");
      return 0;
    }
  }
  
  // Set names for all arguments.
  unsigned Idx = 0;
  for (Function::arg_iterator AI = F->arg_begin(); Idx != Args.size();
       ++AI, ++Idx)
    AI->setName(Args[Idx]);
    
  return F;
}

/// CreateArgumentAllocas - Create an alloca for each argument and register the
/// argument in the symbol table so that references to it will succeed.
void PrototypeAST::CreateArgumentAllocas(Function *F) {
  Function::arg_iterator AI = F->arg_begin();
  for (unsigned Idx = 0, e = Args.size(); Idx != e; ++Idx, ++AI) {
    // Create an alloca for this variable.
    AllocaInst *Alloca = CreateEntryBlockAlloca(F, Args[Idx]);

    // Store the initial value into the alloca.
    Builder.CreateStore(AI, Alloca);

    // Add arguments to variable symbol table.
    NamedValues[Args[Idx]] = Alloca;
  }
}


Function *FunctionAST::Codegen() {
  NamedValues.clear();
  
  Function *TheFunction = Proto->Codegen();
  if (TheFunction == 0)
    return 0;
  
   
  // If this is an operator, install it.
  if (Proto->isBinaryOp()){
    BinopPrecedence[Proto->getOperatorName()] = Proto->getBinaryPrecedence();
   /* std::map<char,int>::iterator it;
    fprintf(stderr,"BinopPrecedence----\n");
    for (it = BinopPrecedence.begin(); it != BinopPrecedence.end(); it++){
	fprintf(stderr,"[%c]%d\n",(*it).first,(*it).second);    
    }*/
  }
  // Create a new basic block to start insertion into.
  BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", TheFunction);
  Builder.SetInsertPoint(BB);
  
  // Add all arguments to the symbol table and create their allocas.
  Proto->CreateArgumentAllocas(TheFunction);
  
  if (Value *RetVal = Body->Codegen()) {
    // Finish off the function.
    Builder.CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    verifyFunction(*TheFunction);

    // Optimize the function.
//090330H Debug    TheFPM->run(*TheFunction);
    
    return TheFunction;
  }
  
  // Error reading body, remove function.
  TheFunction->eraseFromParent();

  if (Proto->isBinaryOp())
    BinopPrecedence.erase(Proto->getOperatorName());
  return 0;
}

//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//

static ExecutionEngine *TheExecutionEngine;

static void HandleDefinition(kscope_syntax_node_t *root,void* data) {
  if (FunctionAST *F = ParseDefinition(root,data)) {
    if (Function *LF = F->Codegen()) {
      fprintf(stderr, "Read function definition:");
      LF->dump();
    }
  } else {
    // Skip token for error recovery.
	fprintf(stderr,"Error: Definition\n");
  }

}

static void HandleExtern (kscope_syntax_node_t *root,void* data) {
  if (PrototypeAST *P = ParseExtern(root,data)) {
    if (Function *F = P->Codegen()) {
      F->dump();
    }
  } else {
    fprintf(stderr,"Error: Extern=n");// Skip token for error recovery.
  }
}

static void HandleTopLevelExpression(kscope_syntax_node_t *root,void* data) {
  // Evaluate a top level expression into an anonymous function.
  if (FunctionAST *F = ParseTopLevelExpr(root,data)) {
    if (Function *LF = F->Codegen()) {
      // JIT the function, returning a function pointer.
      void *FPtr = TheExecutionEngine->getPointerToFunction(LF);
      
      // Cast it to the right type (takes no arguments, returns a double) so we
      // can call it as a native function.
      double (*FP)() = (double (*)())(intptr_t)FPtr;
      fprintf(stderr, "Evaluated to %f\n", FP());
    }
  } else {
    // Skip token for error recovery.
		fprintf(stderr,"Error: Expression\n");
  }
}

/// top ::= definition | external | expression | ';'
static void MainLoop(kscope_syntax_node_t *root,void* data) {
  int cnt;

  kscope_syntax_node_t *node;

  for (cnt = 0; cnt < root->children;cnt++){
	  node = root->child[cnt];
	  if (node->children){
	  //fprintf(stderr,"Node: %s!\n",kscope_node_names[node->child[0]->type]);
      switch (node->child[0]->type) {
        case KSCOPE_DEFN_NODE:  HandleDefinition(node->child[0],data); break;
        case KSCOPE_EXTERN_NODE: HandleExtern(node->child[0],data); break;
        case KSCOPE_EXPR_NODE:   HandleTopLevelExpression(node->child[0],data); break;
	    case KSCOPE___NODE: fprintf(stderr,"[SP]");
		default:; //fprintf(stderr,"Error: Unexpected top level node");
	  }
	  }
  }
  fprintf(stderr,"Done building!!!!!\n");
}

int dump_node(kscope_syntax_node_t *node, void* data)
{ 
	int cnt;
	if (node != NULL && node->type < KSCOPE_LEX_NODE){
      printf("%s|",kscope_node_names[node->type]);
//	    	   printf("%s\n",kscope_get_str(node,data));
	}

	return 0;
}

void* ib;
int depth = 0;
void kscope_print_traverse(kscope_syntax_node_t *root, void *data)
{
	int cnt;
    if (root)
    {
        kscope_syntax_node_t **cur;
	
	if (root->type < 1000){// KSCOPE_LEX_NODE){
        for (cnt = 0;cnt < depth; cnt++)
			printf(".");

        printf("%s #%d %d:%d",kscope_node_names[root->type],root->children,root->begin,root->end);
	if (root->end - root->begin < 8) printf(" [%s]",kscope_get_str(root));
	printf("\n");}
        if (root->child){
			depth++;
            for (cur = root->child; *cur; ++cur){
                kscope_print_traverse(*cur, data);
			}
			depth--;
		}
    }
} /* syntax_node_traverse_inorder() */
//===----------------------------------------------------------------------===//
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===//

/// putchard - putchar that takes a double and returns 0.
extern "C" 
double putchard(double X) {
  putchar((char)X);
  return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" 
double printd(double X) {
  printf("%f\n", X);
  return 0;
}
int term_node(kscope_syntax_node_t *node, void* data)
{
  if (node->type < KSCOPE_LEX_NODE){
	printf("]\n");
  }
}
int print_node(kscope_syntax_node_t *node, void* data)
{ int cnt;
	if (node != NULL){
		if (node->type < KSCOPE_LEX_NODE){
		printf("[");
		//	for (cnt = 0;cnt<node->type;cnt++) printf(" ");
	       //if (node->type == KSCOPE_STATEMENT_NODE || node->type > KSCOPE_PRIMARY_NODE){
               printf("%s",kscope_node_names[node->type]);
               printf("%d:%d",node->children,node->end);
	    	   //printf("%s\n",kscope_get_str(node));
	    	//}
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


//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main(int argc, char** argv) {
  InitializeNativeTarget();
  LLVMContext &Context = getGlobalContext();
  char *input_file;

  if (argc == 2){
	  input_file = strdup(argv[1]);
	  printf("Input:%s\n",argv[1]);
	  if (!input_file) exit(1);
  }
  else exit(1);
	
  kscope_syntax_node_t *root;
  void* error_list;  

  // Install standard binary operators.
  // 1 is lowest precedence.
  BinopPrecedence['='] = 2;
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;  // highest.

 
  // Prime the first token.
  //fprintf(stderr, "ready> ");
  //getNextToken();

  // Make the module, which holds all the code.
  TheModule = new Module("my cool jit", Context);
  
  // Create the JIT.  This takes ownership of the module.
  std::string ErrStr;
  TheExecutionEngine = EngineBuilder(TheModule).setErrorStr(&ErrStr).create();
  if (!TheExecutionEngine) {
    fprintf(stderr, "Could not create ExecutionEngine: %s\n", ErrStr.c_str());
    exit(1);
  }

  FunctionPassManager OurFPM(TheModule);

  // Set up the optimizer pipeline.  Start with registering info about how the
  // target lays out data structures.
  OurFPM.add(new TargetData(*TheExecutionEngine->getTargetData()));
  // Provide basic AliasAnalysis support for GVN.
  OurFPM.add(createBasicAliasAnalysisPass());
  // Promote allocas to registers.
  OurFPM.add(createPromoteMemoryToRegisterPass());
  // Do simple "peephole" optimizations and bit-twiddling optzns.
  OurFPM.add(createInstructionCombiningPass());
  // Reassociate expressions.
  OurFPM.add(createReassociatePass());
  // Eliminate Common SubExpressions.
  OurFPM.add(createGVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  OurFPM.add(createCFGSimplificationPass());

  OurFPM.doInitialization();

  // Set the global so the code gen can use this.
  TheFPM = &OurFPM;

    // Run the main "interpreter loop" now.
    // Parse file 
	if (kscope_parse(input_file,&root,&ib,&error_list)){
          kscope_print_traverse(root,ib);
		printf("\n");
	}
	else{
	        print_errors(error_list);
	}

  MainLoop(root,ib);
	
  kscope_syntax_node_destroy(root);
  kscope_destroy_error_list(error_list);
  kscope_destroy_input_buffer(ib);



  TheFPM = 0;

  // Print out all of the generated code.
//  TheModule->dump();
  return 0;
}

