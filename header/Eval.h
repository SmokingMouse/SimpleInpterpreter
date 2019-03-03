#ifndef EVAL_H
#define EVAL_H

#include"AST.h"
#include"Environment.h"
#include"Parser.h"
#include<iostream>

using std::cout;
using std::endl;

class Evaluator {
public:
	Evaluator(string path):_parser(new Parser(path)),
							_env(new Env()){}

	void exec();
private:
	Env * _env;
	Parser* _parser;
	vector<Exec_Error_Info> _error_info;

	void execStmt(AST* stmt);

	void execAssign(AssignStmtAST*);
	void execIf(IfStmtAST*);
	void execWhile(WhileStmtAST*);
//	void execFuncDef(FuncDefAST*);
	void execReturn(ReturnStmtAST*);
	void execBlock(BlockStmtAST*);
	void execWrite(WriteAST*);

	Env_Entry eval(AST* expr);

	Env_Entry evalId(VarAST* expr);
	Env_Entry evalBinaryOpExpr(BinaryOpAST* expr);
	Env_Entry evalFuncInvokeExpr(FuncInvokeAST* expr);
	Env_Entry evalInt(IntNumAST* expr);
	Env_Entry evalDouble(DoubleNumAST* expr);
};

void Evaluator::exec() {
	AST* ast = _parser->parseProgram();
	ProgramAST* program = TO(ProgramAST,ast);
	for (auto iter = program->stmts.begin(); iter != program->stmts.end(); iter++) {
		execStmt(*iter);
	}
}

void Evaluator::execStmt(AST* stmt) {
	try {
		switch (stmt->node) {
		case Assign:
			execAssign(TO(AssignStmtAST, stmt));
			break;
		case If:
			execIf(TO(IfStmtAST, stmt));
			break;
		case While:
			execWhile(TO(WhileStmtAST, stmt));
			break;
			/*
			case FuncDef:
			execFuncDef(TO(FuncDefAST, stmt));
			*/
		case Return:
			execReturn(TO(ReturnStmtAST, stmt));
			break;
		case Block:
			execBlock(TO(BlockStmtAST, stmt));
			break;
		case WriteStmt:
			execWrite(TO(WriteAST, stmt));
			break;
		default:
			break;
		}
	}
	catch (Exec_Error_Info e) {
		_error_info.push_back(e);
	}
	
}

void Evaluator::execAssign(AssignStmtAST* ast)
{
	Env_Entry val = eval(ast->value);
	Env_Entry* find_val = _env->findEntry(ast->id);
	if (find_val) {
		if (find_val->type != val.type)
			throw Exec_Error_Info(ast, "assign type unmatched.");
		find_val->val.dbl_val = val.val.dbl_val;
	}
	else
	{
		if (_parser->getTable()->getEntry(ast->id)->type != val.type)
			throw Exec_Error_Info(ast, "assign type unmatched.");
		_env->pushValue(ast->id, val);
	}
}

void Evaluator::execIf(IfStmtAST* ast) {
	Env_Entry condition = eval(ast->condition);
	if (condition.type != BOOL) throw Exec_Error_Info(ast->condition, "the value type is not bool");
	_env->enterScope();
	if (condition.val.bool_val) execStmt(ast->if_body);
	else execStmt(ast->else_body);
	if (_env->isRtnState()) return;
	_env->exitScope();
}

void Evaluator::execWhile(WhileStmtAST* ast) {
	Env_Entry condition = eval(ast->condition);
	if (condition.type != BOOL) throw Exec_Error_Info(ast->condition, "the value type is not bool");
	_env->enterScope();
	while (condition.type == BOOL && condition.val.bool_val) {
		execStmt(ast->body);
		if (_env->isRtnState()) return;
		condition = eval(ast->condition);
	} 
	_env->exitScope();
}

/*
void Evaluator::execFuncDef(FuncDefAST* ast) {

}
*/

void Evaluator::execReturn(ReturnStmtAST* ast) {//check if in Func.
	Env_Entry rtn_val = eval(ast->expr);
	_env->pushValue(RETURN_ID,rtn_val);
	_env->setRtnState(1);
}

void Evaluator::execBlock(BlockStmtAST* ast) {

	for (auto iter = ast->stmts.begin(); iter != ast->stmts.end(); iter++)
	{
		execStmt(*iter);
		if (_env->isRtnState()) break;
	}

}

void Evaluator::execWrite(WriteAST* ast) {
	Env_Entry value = eval(ast->expr);
	switch (value.type) {
	case DOUBLE:
		cout << value.val.dbl_val << endl;
		break;
	case INT:
		cout << value.val.int_val << endl;
		break;
	default:
		break;
	}
}

Env_Entry Evaluator::eval(AST* expr)
{
	switch (expr->node) {
	case Id:
		return evalId(TO(VarAST, expr));
	case BinaryOpExpr:
		return evalBinaryOpExpr(TO(BinaryOpAST, expr));
	case FuncInvokeExpr:
		return evalFuncInvokeExpr(TO(FuncInvokeAST, expr));
	case IntNumExpr:
		return evalInt(TO(IntNumAST, expr));
	case DoubleNumExpr:
		return evalDouble(TO(DoubleNumAST, expr));
	default:
		return Env_Entry();
	}
}

Env_Entry Evaluator::evalId(VarAST * expr)
{
	Env_Entry* ptr = _env->findEntry(expr->id);
	if (ptr) {
		return *ptr;
	}
	return Env_Entry();
}

Env_Entry Evaluator::evalBinaryOpExpr(BinaryOpAST * expr)
{
	Env_Entry left_val = eval(expr->lhs);
	Env_Entry right_val = eval(expr->rhs);
	if (left_val.type != right_val.type)
		throw Exec_Error_Info(expr, "BinaryOp type unmatched.");
	if (left_val.type == NONE || right_val.type == NONE)
		throw Exec_Error_Info(expr, "part of the BinaryOp can't be calculated.");
	
	switch (expr->op){
	case s_equal:
		if (left_val.val.dbl_val == right_val.val.dbl_val)
			return Env_Entry(true);
		return Env_Entry(false);
	case s_not_equal:
		if (left_val.val.dbl_val == right_val.val.dbl_val)
			return Env_Entry(false);
		return Env_Entry(true);
	case s_plus:
		if (left_val.type == DOUBLE)
			return Env_Entry(left_val.val.dbl_val + right_val.val.dbl_val);
		else if (right_val.type == INT)
			return Env_Entry(left_val.val.int_val + right_val.val.int_val);
		else break;
	case s_sub:
		if (left_val.type == DOUBLE)
			return Env_Entry(left_val.val.dbl_val - right_val.val.dbl_val);
		else if (right_val.type == INT)
			return Env_Entry(left_val.val.int_val - right_val.val.int_val);
		else break;
	case s_mul:
		if (left_val.type == DOUBLE)
			return Env_Entry(left_val.val.dbl_val * right_val.val.dbl_val);
		else if (right_val.type == INT)
			return Env_Entry(left_val.val.int_val * right_val.val.int_val);
		else break;
	case s_div:
		if (left_val.type == DOUBLE)
			return Env_Entry(left_val.val.dbl_val * right_val.val.dbl_val);
		else if (right_val.type == INT)
			return Env_Entry(left_val.val.int_val * right_val.val.int_val);
		else break;
	case s_less:
		if (left_val.type == DOUBLE)
			return Env_Entry(left_val.val.dbl_val < right_val.val.dbl_val);
		else if (right_val.type == INT)
			return Env_Entry(left_val.val.int_val < right_val.val.int_val);
		else break;
	case s_greater:
		if (left_val.type == DOUBLE)
			return Env_Entry(left_val.val.dbl_val > right_val.val.dbl_val);
		else if (right_val.type == INT)
			return Env_Entry(left_val.val.int_val > right_val.val.int_val);
		else break;
	default:
		return Env_Entry();
	}
	return Env_Entry();
}

Env_Entry Evaluator::evalFuncInvokeExpr(FuncInvokeAST* expr)
{
	
	string func_id = expr->id;
	SymTable_Entry* func_entry = _parser->getTable()->getEntry(func_id);
	if (func_entry->type != FUNC) return Env_Entry();
	FuncDefAST* func_ast = TO(FuncDefAST,func_entry->ast);
	ParaAST* para_ast = TO(ParaAST, func_ast->para);
	BlockStmtAST* block_ast = TO(BlockStmtAST, func_ast->body);

	vector<tuple<Semantic_Type, string>>& para = para_ast->para;
	if (expr->args.size() != para.size())
		throw Exec_Error_Info(expr, "the count of args unmatched.");

	_env->enterFuncScope();

	size_t idx;

	for (idx = 0; idx < expr->args.size(); idx++) {
		Env_Entry val = eval(expr->args[idx]);
		if (val.type != std::get<0>(para[idx]))
			throw Exec_Error_Info(expr, "type unmatched between arg and para.");
		_env->pushValue(std::get<1>(para[idx]), val);
	}

	_env->insertFuncId(idx);
	execBlock(block_ast);

	Env_Entry return_val = _env->getRtnEntry();
	_env->exitFuncScope();
	if (return_val.type == NONE) throw Exec_Error_Info(expr, "don't return in func.");
	return return_val;
}

Env_Entry Evaluator::evalInt(IntNumAST * expr)
{
	return Env_Entry(expr->value);
}

Env_Entry Evaluator::evalDouble(DoubleNumAST * expr)
{
	return Env_Entry(expr->value);
}





#endif
