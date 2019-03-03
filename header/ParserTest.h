#ifndef PARSERTEST_H
#define PARDERTEST_H

#include"Parser.h"
#include<stdio.h>


int tabSize = 0;

void addTab() {
	for (int i = 0; i < tabSize; i++) {
		printf("\t");
	}
}

void printTree(AST* tree) {
	addTab();
	if (!tree) {
		printf("\n"); return;
	}
	switch (tree->node)
	{
	case Program: {
		printf("Program\n");
		tabSize++;
		auto subTrees = &CONCRETE_AST_MEM(ProgramAST, tree, stmts);
		for (auto i = subTrees->begin(); i != subTrees->end(); i++) {
			printTree(*i);
		}
		tabSize--;
		break;
	}
		
	case FuncDef:{
		printf("FuncDef\n");
		tabSize++;
		addTab();
		printf("Type:%d\n",CONCRETE_AST_MEM(FuncDefAST,tree,returnType));
		printTree(CONCRETE_AST_MEM(FuncDefAST, tree, para));
		printTree(CONCRETE_AST_MEM(FuncDefAST, tree, body));
		tabSize--;
		break; 
	}
		
	case Assign:{
		printf("Assign\n");
		tabSize++;
		addTab();
		printf("Var:%s\n", CONCRETE_AST_MEM(AssignStmtAST, tree, id).data());
		printTree(CONCRETE_AST_MEM(AssignStmtAST, tree, value));
		tabSize--;
		break; 
	}
		
	case If:{
		printf("If\n");
		tabSize++;
		printTree(CONCRETE_AST_MEM(IfStmtAST, tree, condition));
		printTree(CONCRETE_AST_MEM(IfStmtAST, tree, if_body));
		printTree(CONCRETE_AST_MEM(IfStmtAST, tree, else_body));
		tabSize--;
		break; 
	}
		
	case While:{
		printf("While\n");
		tabSize++;
		printTree(CONCRETE_AST_MEM(WhileStmtAST, tree, condition));
		printTree(CONCRETE_AST_MEM(WhileStmtAST, tree, body));
		tabSize--;
		break; 
	}
		
	case Para: {
		printf("Para\n");
		tabSize++;
		auto strs = &CONCRETE_AST_MEM(ParaAST, tree, para);
		for (auto i = strs->begin(); i != strs->end(); i++) {
			addTab();
			printf("var:%s\n",std::get<1>(*i).data());
		}
		tabSize--;
		break;
	}
		
	case Return:{
		printf("Return\n");
		tabSize++;
		printTree(CONCRETE_AST_MEM(ReturnStmtAST, tree, expr));
		tabSize--;
		break;
	}
		
	case Block:{
		printf("Block\n");
		tabSize++;
		auto subTrees = &CONCRETE_AST_MEM(BlockStmtAST, tree, stmts);
		for (auto i = subTrees->begin(); i != subTrees->end(); i++) {
			printTree(*i);
		}
		tabSize--;
		break;
	}
		
	case IntNumExpr: {
		printf("intLit\n");
		tabSize++;
		addTab();
		printf("%d\n", CONCRETE_AST_MEM(IntNumAST, tree, value));
		tabSize--;
		break;
	}
		
	case DoubleNumExpr:{
		printf("doubleLit\n");
		tabSize++;
		addTab();
		printf("%f\n", CONCRETE_AST_MEM(DoubleNumAST, tree, value));
		tabSize--;
		break; 
	}
		
	case Id: {
		printf("VarExpr\n");
		tabSize++;
		addTab();
		printf("var:%s\n", CONCRETE_AST_MEM(VarAST, tree, id).data());
		tabSize--;
		break;
	}
		
	case FuncInvokeExpr:{
		printf("FuncInvoke\n");
		tabSize++;
		addTab();
		printf("funcName:%s\n", CONCRETE_AST_MEM(FuncInvokeAST, tree, id).data());
		auto subTrees = &CONCRETE_AST_MEM(FuncInvokeAST, tree, args);
		for (auto i = subTrees->begin(); i != subTrees->end(); i++) {
			printTree(*i);
		}
		tabSize--;
		break;
	}
		
	case BinaryOpExpr: {
		printf("BinaryOp:%d\n",CONCRETE_AST_MEM(BinaryOpAST,tree,op));
		tabSize++;
		printTree(CONCRETE_AST_MEM(BinaryOpAST, tree, lhs));
		printTree(CONCRETE_AST_MEM(BinaryOpAST, tree, rhs));
		tabSize--;
		break;
	}

	case WriteStmt: {
		printf("Write\n");
		tabSize++;
		printTree(CONCRETE_AST_MEM(WriteAST, tree, expr));
		tabSize--;
		break;
	}
		
	default:
		break;
	}
}

void test() {
	Parser p("Test.txt");
	AST* ast = p.parseProgram();
	printTree(ast);
}
#endif

