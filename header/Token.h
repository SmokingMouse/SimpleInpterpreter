#ifndef TOKEN_H
#define TOKEN_H
#include<string>
#include<memory>


enum Token_Type {
	k_if, k_else, k_while, k_return, k_int, k_double,k_write,//reserved key
	s_equal, s_not_equal, s_left_comment, s_right_comment, identifier,
	s_plus, s_sub, s_mul, s_div, s_less,  s_left_paren, s_right_paren, s_assign, s_greater,
	s_end, s_left_brace, s_right_brace,  s_comma,

	v_double, v_int,
	endId
};

struct Val {
	union {
		char sym;
		std::string key;
		int intLit;
		double doubleLit;
	};
	Val():sym('\0'){}
	~Val(){}
	Val(char c):sym(c){}
	Val(const std::string& str) :key(str){}
	Val(int i):intLit(i){}
	Val(double d):doubleLit(d){}
	Val(const Val& v){}
};

class Token{
public:
	Token_Type type;
	Val value;
	size_t lineNo;

	Token() = default;

	Token(Token_Type t, char c,size_t l) :type(t) ,value(c),lineNo(l){}

	Token(Token_Type t, std::string str, size_t l) :type(t), value(str), lineNo(l) {}

	Token(Token_Type t, int i,size_t l) :type(t),value(i),lineNo(l){}

	Token(Token_Type t, double d,size_t l) :type(t),value(d) ,lineNo(l){}

	~Token() {}
};
#endif