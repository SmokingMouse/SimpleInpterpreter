#ifndef ERRORINFO_H
#define ERRORINFO_H
#include<string>
#include"AST.h"
enum Token_Error_Type {
	undef_sym,num_form_error
};

struct Token_Error_Info {
	size_t lineNo;
	std::string errorTokenStr;
	Token_Error_Type errorType;
};

class Parser_Error_Info {
public:
	Parser_Error_Info(size_t l,const std::string& str):lineNo(l),errorStr(str){}
	size_t lineNo;
	std::string errorStr;
};

class Exec_Error_Info {
public:
	Exec_Error_Info(AST* tree,std::string str):errorTree(tree),errorStr(str){}
	AST* errorTree;
	std::string errorStr;
};

#endif