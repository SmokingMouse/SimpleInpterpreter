
#ifndef AST_H
#define AST_H

#include<string>
#include<vector>
#include<tuple>
#include"Token.h"

enum Semantic_Type:int;
struct SymTable_Entry;
class SymTable;


using std::tuple;

enum AST_Node_Type {
	Assign, If, While, FuncDef, Return, Block, Para,Id, BinaryOpExpr,WriteStmt,
	Program, FuncInvokeExpr, IntNumExpr,DoubleNumExpr,ErrorT
};

class AST {
public:
	AST(AST_Node_Type type):node(type){}
	AST_Node_Type node;
	virtual ~AST() = 0 {}
};


class AssignStmtAST:public AST {
public:
	AssignStmtAST(AST_Node_Type n,const std::string& s,AST* a):
		AST(Assign),id(s),value(a){}
	std::string id;
	AST* value;
};

class IfStmtAST :public AST {
public:
	IfStmtAST():AST(If){}
	AST* condition;
	AST* if_body;
	AST* else_body;
};

class WhileStmtAST :public AST {
public:
	WhileStmtAST():AST(While){}
	AST* condition;
	AST* body;
};

class ParaAST :public AST {
public:
	ParaAST():AST(Para){}
	std::vector<tuple<Semantic_Type,std::string>> para;
};

class FuncDefAST :public AST {
public:
	FuncDefAST(Semantic_Type r,AST* p,AST* b):AST(FuncDef),returnType(r),para(p),body(b){}
	Semantic_Type returnType;
	AST* para;
	AST* body;
};

class ReturnStmtAST :public AST {
public:
	ReturnStmtAST():AST(Return){}
	AST* expr;
};

class BlockStmtAST :public AST {
public:
	BlockStmtAST():AST(Block){}
	std::vector<AST*> stmts;
};

class ProgramAST :public AST {
public:
	ProgramAST():AST(Program){}
	std::vector<AST*> stmts;
};

class IntNumAST :public AST {
public:
	IntNumAST(int v):AST(IntNumExpr),value(v){}
	int value;
};

class DoubleNumAST :public AST {
public:
	DoubleNumAST(double v) :AST(DoubleNumExpr),value(v) {}
	double value;
};

class VarAST :public AST {
public:
	VarAST(const std::string& i):AST(Id),id(i){}
	std::string id;
};

class FuncInvokeAST :public AST {
public:
	FuncInvokeAST(const std::vector<AST*>& a,const std::string& s):
		args(a),id(s),AST(FuncInvokeExpr){}
	std::string id;
	std::vector<AST*> args;
};

class BinaryOpAST :public AST {
public:
	Token_Type op;
	AST* lhs;
	AST* rhs;
	BinaryOpAST(Token_Type t,AST* l,AST* r):op(t),lhs(l),rhs(r),AST(BinaryOpExpr){}
};

class WriteAST:public AST {
public:
	AST* expr;
	WriteAST(AST* subtree):expr(subtree),AST(WriteStmt){}
};

class Error :public AST {
public:
	Error(const std::string& str):errorInfo(str),AST(ErrorT){}
	std::string errorInfo;
};
#endif