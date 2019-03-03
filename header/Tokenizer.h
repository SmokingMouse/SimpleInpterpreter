#ifndef TOKENIZER_H
#define TOKENIZER_H
#include"Token.h"
#include"ErrorInfo.h"
#include"FileHandler.h"

#include<string>
#include<vector>

class Tokenizer {
public:
	Tokenizer(std::string filepath):io(new IOHandler(filepath)),lineNo(0),
		errorInfo(), position(0), lineStr(&io->getStr()),commentState(false)
	{
		lineLength = lineStr->size();
		gotoEnd = !lineStr->compare("@end__");
	}
	~Tokenizer() { 
		delete io;
		for (Token_Error_Info* i : errorInfo) {
			delete i;
		}
		for (Token* i : tokens)
			delete i;
	}
	
	std::vector<Token_Error_Info*>* getErrorInfo() { return &errorInfo; }
	Token* getNextToken();
	bool gotoEnd;

	void tokenize() {
		while (!gotoEnd) {
			tokens.push_back(getNextToken());
		}
		return;
	}

	std::vector<Token*>* getTokens() {
		return &tokens;
	}
private:
	size_t lineNo;
	IOHandler* io;
	const std::string* lineStr;
	size_t lineLength;
	size_t position;
	bool commentState;
	std::vector<Token*> tokens;
	
	std::vector<Token_Error_Info*> errorInfo;
	
	void clearSpace() {
		while (position == lineLength && !gotoEnd)//first type: goto end of line.
			getNextLine(); //may set gotoEnd.
		if (gotoEnd) return;//second type:goto end of file.
		while (isspace(lineStr->at(position))) {//third type:
			position++;
			clearSpace();
		}
	}

	//the only way to set goto end.
	void getNextLine() {
		lineStr = &io->getLine();
		if (!lineStr->compare("@end__")) {
			gotoEnd = true;
			return;
		}
		position = 0;
		lineNo++;
		lineLength = lineStr->size();
	}

	char nextChar() {
		if (position < lineLength - 1)
			return lineStr->at(position + 1);
		else return '\0';
	}

	void forwardPosition(size_t n) {
		if (position + n < lineLength)
			position += n;
		getNextLine();
		if(!gotoEnd)
			forwardPosition(lineLength - 1 - position);
	}


	void gotoNextPosition() {
		if (position < lineLength - 1)
			position++;
		else
			getNextLine();
	}
};

typedef size_t State_No;

bool isSym(char c) {
	return !isalnum(c);
}

bool beginWithKey(const std::string& str, const std::string& key) {
	size_t str_len = str.length(), key_len = key.length();
	if (str_len < key_len) return false;
	if (str_len == key_len) return str == key;
	return str.substr(0,key_len) == key&&!isalnum(str[key_len]);
}

State_No transform(State_No curr, char c) {
	switch (curr)
	{
	case 0:
		if (c == '0') return 1;
		if (isdigit(c)) return 2;
	case 1:
		if (c == '.') return 3;
	case 2:
		if (isdigit(c)) return 2;
		if (c == '.') return 3;
	case 3:
		if (isdigit(c)) return 3;
	default:
		break;
	}
	return 6;
}

bool canTransform(State_No i, char c) {
	if (i == 0 && isdigit(c)) return true;
	if (i == 1 && c == '.')return true;
	if (i == 2 && (isdigit(c) || c == '.')) return true;
	if (i == 3 && isdigit(c)) return true;
	return false;
}


Token* Tokenizer::getNextToken() {
	clearSpace();//deal with the space.

	if (gotoEnd) 
		return new Token(endId,'\0',lineNo);

	char curr = lineStr->at(position);
	char next = nextChar();

	if (commentState == true) {
		if (curr != '*'||next != '/'){
			gotoNextPosition();
			return getNextToken();
		}
		commentState = false;
		gotoNextPosition();
		gotoNextPosition();
		return getNextToken();
	}

	//匹配关键字
	if (isalpha(curr) || curr == '_') {
		std::string sub = this->lineStr->substr(position);

		if (beginWithKey(sub, "if")) {
			position += 2;
			return new Token{ Token_Type::k_if,"if" ,lineNo};
		}

		if (beginWithKey(sub, "else")) {
			position += 4;
			return new Token{ Token_Type::k_else,"else",lineNo };
		}

		if (beginWithKey(sub, "while")) {
			position += 5;
			return new Token{ Token_Type::k_while,"while" ,lineNo };
		}

		if (beginWithKey(sub, "return")) {
			position += 6;
			return new Token{ Token_Type::k_return,"return",lineNo };
		}

		if (beginWithKey(sub, "int")) {
			position += 3;
			return new Token{ Token_Type::k_int,"int" ,lineNo };
		}

		if (beginWithKey(sub, "double")) {
			position += 6;
			return new Token{ Token_Type::k_double,"double" ,lineNo };
		}

		if (beginWithKey(sub, "write")) {
			position += 5;
			return new Token{ Token_Type::k_write,"write",lineNo };
		}

		size_t index = position;
		while (position<lineStr->size()&&(isalnum(lineStr->at(position)) || lineStr->at(position) == '_'))
			++position;
		return new Token{ Token_Type::identifier,lineStr->substr(index, position - index) ,lineNo };
	}

//匹配符号
	if (isSym(curr)) {
		switch (curr) {
		case '+':++position; return new Token{ Token_Type::s_plus, '+' ,lineNo };
		case '-':++position; return new Token{ Token_Type::s_sub,'-',lineNo };
		case '(':++position; return new Token{ Token_Type::s_left_paren,'(' ,lineNo };
		case ')':++position; return new Token{ Token_Type::s_right_paren,')' ,lineNo };
		case ';':++position; return new Token{ Token_Type::s_end,';' ,lineNo };
		case '{':++position; return new Token{ Token_Type::s_left_brace,'{' ,lineNo };
		case '}':++position; return new Token{ Token_Type::s_right_brace,'}' ,lineNo };
		case '>':++position; return new Token{ Token_Type::s_greater,'>' ,lineNo };
		case '*':++position; return new Token{ Token_Type::s_mul,'*' ,lineNo };
		case ',':++position; return new Token{ Token_Type::s_comma,',' ,lineNo };

		case '/':
			if (next == '*') {
				position += 2;
				commentState = true;
				return getNextToken();
			}
			else {
				++position;
				return new Token{ Token_Type::s_div,'/' ,lineNo };
			}
		case '=':
			if (next == '=') {
				position += 2;
				return new Token{ Token_Type::s_equal,"==" ,lineNo };
			}
			else {
				++position;
				return new Token{ Token_Type::s_assign,'=',lineNo };
			}
		case '<':
			if (next == '>') {
				position += 2;
				return new Token{ Token_Type::s_not_equal,"<>",lineNo };
			}
			else {
				++position;
				return new Token{ Token_Type::s_less,'<',lineNo };
			}
		default:
			errorInfo.push_back(new Token_Error_Info{ lineNo,lineStr->substr(position),undef_sym });
			gotoNextPosition();
			return getNextToken();
		}
		
	}

	
//匹配数字
	if (isdigit(curr)) {
		size_t index = position;

		State_No state = 0;
		for (; position<lineStr->size()&&canTransform(state, lineStr->at(position));
			state = transform(state, lineStr->at(position)), position++) {
		}
		
		if (state == 2 || state == 1)
			return new Token{ v_int,std::stoi(lineStr->substr(index, position - index)) ,lineNo };
		if (state == 3)
			return new Token{ v_double,std::stod(lineStr->substr(index, position - index)),lineNo };
	}
} 

#endif


