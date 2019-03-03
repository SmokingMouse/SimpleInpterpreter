#ifndef PARSER_H
#define PARSER_H

#include"Tokenizer.h"
#include"AST.h"
#include"SymTable.h"
#include<stdarg.h>
#include<map>


#define CONCRETE_AST_MEM(type,obj,mem) (dynamic_cast<type*>(obj)->mem)
#define TYPE (tokens[curr]->type)
#define TO(type,obj) (dynamic_cast<type*>(obj))

std::map<Token_Type, int>op_prior{ {s_equal,10},
									{s_not_equal,10},
									{s_less,20	},
									{s_greater,20},
									{s_plus,30},
									{s_sub,30},
									{s_mul,40},
									{s_div,40} };

bool isIn(Token_Type t, ...) {
	va_list args = NULL;
	va_start(args, t);
	Token_Type arg = va_arg(args, Token_Type);
	while (arg != 0) {
		if (arg == t) return true;
		arg = va_arg(args, Token_Type);
	}
	return false;
}

int getOpPrior(Token_Type t) {
	auto iter = op_prior.find(t);
	if (iter!=op_prior.end()) {
		return iter->second;
	}
	return -1;
}

/*
Grammar:
	prior: Equal = NotEqual 1 < Less = Greater 2 < Sub = Add  3< Mul = Div 4
	Program = Stmts
	
	
	LogicOp = Equal | NotEqual | Less | Greater
	Num = Int | Double
	Expr = AritExpr | LogicExpr
	LogicExpr = AritExpr LogicOp AritExpr | (LogicExpr)

	AritExpr = Term FirstOp 
	FirstOp = Null | + Term FirstOp | - Term FirstOp
	Term = Fact SecondOp
	SecondOp = Null | * Fact SecondOp | / Fact SecondOp
	Fact = Num | FuncInvoke | (AritExpr) | Id

	FuncInvoke = Func ( Arg )					Function Invoke.
	Arg = Expr | Expr , Arg | Null		
	
	
	Block = { Stmts }
	Type = K_Int | K_Double				
	Stmts = Stmt Stmts | Null			
	Para = Type Id | Type Id , Para | Null		
	Stmt = Type Id = Expr End					Var Assignment.
	Stmt = Type Id ( Para ) Block				Function Defination
	Stmt = Block								
	Stmt = If ( Expr ) Stmt Else Stmt			If
	Stmt = While ( Expr £© Stmt					While
	Stmt = Return Expr End						Return
*/

//1. Ambiguous
//2. Left Recurse  finish!

class Parser {
public:
	Parser(std::string path) :tokenizer(new Tokenizer(path)) {}
	
	AST* parseProgram() {
		if (!prepare()) return NULL;
		tokens = *tokenizer->getTokens();
		AST* program = new ProgramAST();
		AST* temp;
		while ((temp = parseStmt())) {
			CONCRETE_AST_MEM(ProgramAST, program, stmts).push_back(temp);
		}
		return program;
	}

	SymTable* getTable() {
		return &table;
	}

private:
	std::vector<Token*> tokens;
	Tokenizer* tokenizer;
	size_t curr;
	SymTable table;
	std::vector<Parser_Error_Info> error;

	bool prepare() {
		tokenizer->tokenize();
		if (tokenizer->getErrorInfo()->empty()) return true;
		return false;
	}

	bool typeCheck(Token_Type t) {
		if (TYPE != t) return false;
		curr++;
		return true;
	}

	

	AST* parsePrimary() {
		switch (TYPE)
		{
		case v_int:
		case v_double:
			return parseNum();
		case identifier:
			return parseIdentifier();
		case s_left_paren:
			return parseParenExpr();
		default:
			throw Parser_Error_Info(tokens[curr]->lineNo, "lack primary.");
		}
	}

	AST* parseNum() {
		AST* num_ast = NULL;
		if (isIn(TYPE, v_int, v_double)) {
			if (TYPE == v_int) {
				num_ast = new IntNumAST(tokens[curr]->value.intLit);
			}
			else {
				num_ast = new DoubleNumAST(tokens[curr]->value.doubleLit);
			}
			curr++;
		}
		return num_ast;
	}

	AST* parseIdentifier() {
		std::string idName = tokens[curr]->value.key;
		curr++;
		if (!typeCheck(s_left_paren))
			return new VarAST(idName);
		std::vector<AST*> args;
		if (!typeCheck(s_right_paren)) {
			while (1) {
				AST* arg = parseExpr();
				
				if (arg)
					args.push_back(arg);
				if (typeCheck(s_right_paren))
					return new FuncInvokeAST(args, idName);
				if (!typeCheck(s_comma))
					throw Parser_Error_Info(tokens[curr]->lineNo,"Expect ')' or ','");
			}
		}
		return new FuncInvokeAST(args, idName);
	}

	AST* parseParenExpr() {
		AST* expr_ast = NULL;
		if (typeCheck(s_left_paren)) {
			expr_ast = parseExpr();
		}
		if (typeCheck(s_right_paren))
			return expr_ast;
		else {
			throw Parser_Error_Info(tokens[curr]->lineNo, "expect ')'");
		}
	}


	AST* parseExpr() {
		AST* lhs = parsePrimary();
		if (!lhs) return NULL;
		return parseBinOpRhs(0, lhs);
	}

	AST* parseBinOpRhs(int prior, AST* lhs) {
		while (1) {
			int cur_prior = getOpPrior(TYPE);

			if (cur_prior < prior) return lhs;

			Token_Type op = tokens[curr]->type;
			curr++;
			AST* rhs = parsePrimary();
			int next_prior = getOpPrior(TYPE);
			if (next_prior > cur_prior) {
				rhs = parseBinOpRhs(cur_prior + 1, rhs);
			}
			lhs = new BinaryOpAST(op, lhs, rhs);
			
		}
	}

	
	AST* parseStmt() {
		try {
			AST* ast = NULL;
			switch (TYPE)
			{
			case s_left_brace:
			{
				AST* block = parseBlock();
				if (block) return block;
				break;
			}
			case k_if: {
				AST* if_ast = parseIf();
				if (if_ast) return if_ast;
				break;
			}
			case k_while: {
				AST* while_ast = parseWhile();
				if (while_ast) return while_ast;
				break;
			}

			case k_return: {
				AST* return_ast = parseReturn();
				if (return_ast) return return_ast;
				break;
			}

			case k_int:
			case k_double: {
				Token_Type t = TYPE;
				curr++;
				if (TYPE == identifier) {
					std::string id = tokens[curr]->value.key;
					curr++;
					if (typeCheck(s_assign)) {                         //parse assign stmt.
						AST* exp = parseExpr();
						if (typeCheck(s_end)) {
							ast = new AssignStmtAST(Assign, id, exp);
							table.pushEntry(new SymTable_Entry{ t == k_int ? INT : DOUBLE, id, exp });
							return ast;
						}
						else {
							throw Parser_Error_Info(tokens[curr]->lineNo, "Expect ';'");
						}
					}
					else if (typeCheck(s_left_paren)) {                 //parse function define stmt.
						AST* para = NULL;
						if (TYPE == k_int || TYPE == k_double) {
							para = parsePara();
						}
						if (typeCheck(s_right_paren)) {
							AST* block = parseBlock();
							ast = new FuncDefAST(t == k_int? INT:DOUBLE, para, block);
							table.pushEntry(new SymTable_Entry{ FUNC,id,ast });
							return ast;
						}
						else {
							throw Parser_Error_Info(tokens[curr]->lineNo, "Expect ')'");
						}
					}
					else {
						throw Parser_Error_Info{ tokens[curr]->lineNo, "the prefix means assign or funcDef,but not." };
					}
				}
				else {
					throw Parser_Error_Info(tokens[curr]->lineNo,"After typename should be identifier.");
				}
				break;
			}
			case k_write: {
				AST* write_ast = parseWrite();
				if (write_ast) return write_ast;
				break;
			}
			case identifier:
			{
				std::string id = tokens[curr]->value.key;
				curr++;
				if (typeCheck(s_assign)) {
					AST* exp = parseExpr();
					if (typeCheck(s_end)) {
						ast = new AssignStmtAST(Assign, id, exp);
						table.updateEntry(id, exp);
						return ast;
					}
					else {
						throw Parser_Error_Info(tokens[curr]->lineNo, "Expect ';'");
					}
				}
				else {
					throw Parser_Error_Info(tokens[curr]->lineNo, "in a stmt,after the id should be assign.");
				}
				break;
			}
			default:
				return nullptr;
			}
		}
		catch (Parser_Error_Info errorInfo) {
			error.push_back(errorInfo);
			while (!isIn(TYPE,s_end,s_right_brace))
			{
				if (TYPE == endId)
					return NULL;
				curr++;
			}
			curr++;
			if (TYPE == endId) return NULL;
			return parseStmt();
		}
	}

	AST* parseWrite() {
		AST* rtn;
		if (typeCheck(k_write)) {
			AST* expr_ast = parseExpr();
			if (expr_ast) {
				rtn = new WriteAST(expr_ast);
				if (typeCheck(s_end)) {
					return rtn;
				}
				else {
					throw Parser_Error_Info(tokens[curr]->lineNo, "Expect ';'");
				}
			}
			return NULL;
			
		}
	}

	AST* parseBlock() {
		AST* block_ast = new BlockStmtAST();
		AST* stmt;
		if (typeCheck(s_left_brace)) {
			while (stmt = parseStmt()) {
				CONCRETE_AST_MEM(BlockStmtAST, block_ast, stmts).push_back(stmt);
			}
			if (typeCheck(s_right_brace))
				return block_ast;
			else {
				throw Parser_Error_Info(tokens[curr]->lineNo, "Expect '}'");
			}
		}
		else {
			throw Parser_Error_Info(tokens[curr]->lineNo, "Expect '{'");
		}
		return block_ast;
	}

	AST* parseIf() {
		AST* if_ast = new IfStmtAST();
		if (typeCheck(k_if)) {
			if (typeCheck(s_left_paren)) {
				AST* con = parseExpr();
				CONCRETE_AST_MEM(IfStmtAST, if_ast, condition) = con;
				if (typeCheck(s_right_paren)) {
					AST* if_b;
					if(TYPE == s_left_brace)
						if_b = parseBlock();
					else
						if_b = parseStmt();
					CONCRETE_AST_MEM(IfStmtAST, if_ast, if_body) = if_b;
					
					if (typeCheck(k_else)) {
						AST* else_b;
						if (TYPE == s_left_brace)
							else_b = parseBlock();
						else
							else_b = parseStmt();
						CONCRETE_AST_MEM(IfStmtAST, if_ast, else_body) = else_b;
					}
				}
				else {
					throw Parser_Error_Info(tokens[curr]->lineNo, "expect ')'");
				}
			
			}
			else {
				throw Parser_Error_Info(tokens[curr]->lineNo, "after if should be '('");
			}
		}
		else {
			throw Parser_Error_Info(tokens[curr]->lineNo, "Expect if key,but not.");
		}
		return if_ast;

	}

	AST* parseWhile() {
		AST* while_stmt = new WhileStmtAST();
		if (typeCheck(k_while)) {
			if (typeCheck(s_left_paren)) {
				AST* con = parseExpr();
				CONCRETE_AST_MEM(WhileStmtAST, while_stmt, condition) = con;
				if (typeCheck(s_right_paren)) {
					AST* b;
					if (TYPE == s_left_brace)
						b = parseBlock();
					else
						b = parseStmt();
					CONCRETE_AST_MEM(WhileStmtAST, while_stmt, body) = b;
				}
				else {
					throw Parser_Error_Info(tokens[curr]->lineNo, "expect ')'");
				}
			}
			else {
				throw Parser_Error_Info(tokens[curr]->lineNo, "After while should be '('");
			}
		}
		else {
			throw Parser_Error_Info(tokens[curr]->lineNo, "Expect while,but not.");
		}
		return while_stmt;
	}

	AST* parseReturn() {
		AST* return_stmt = new ReturnStmtAST();
		if (typeCheck(k_return)) {
			AST* e = parseExpr();
			CONCRETE_AST_MEM(ReturnStmtAST, return_stmt, expr) = e;
			if (typeCheck(s_end)) {
				return return_stmt;
			}
			else {
				throw Parser_Error_Info(tokens[curr]->lineNo, "expect ';'.");
			}
		}
		else {
			throw Parser_Error_Info(tokens[curr]->lineNo, "expect return,but not.");
		}
		return return_stmt;
	}

	AST* parsePara() {
		AST* p = new ParaAST();
		bool hasRemain = true;
		while (hasRemain) {
			Token_Type t = TYPE;
			if (typeCheck(k_int) || typeCheck(k_double)) {
				if (TYPE == identifier) {
					CONCRETE_AST_MEM(ParaAST, p, para).push_back({ t == k_int? INT:DOUBLE,tokens[curr]->value.key });
//					SymTable_Entry* entry = new SymTable_Entry
//						{ t == k_int ? INT : DOUBLE,tokens[curr]->value.key,nullptr };
//					table.pushEntry(entry);
					curr++;
					if (!typeCheck(s_comma))
						hasRemain = false;
				}
			}
			else {
				throw Parser_Error_Info(tokens[curr]->lineNo, "in the para,expect a type.");
			}
		}
		return p;
	}

	

	
};
#endif