#ifndef LEXERTEST_H
#define LECERTEST_H

#include"Token.h"
#include"Tokenizer.h"
#include<stdio.h>
#include<iostream>

void getValue(const Token& t) {
	if (t.type == endId) return;
	if (t.type <= 11) {
		std::cout << t.value.key << '\t';
		return;
	}
	if (t.type <= 24) std::cout << t.value.sym << '\t';
	if (t.type == 25) std::cout << t.value.doubleLit << '\t';
	if (t.type == 26) std::cout << t.value.intLit << '\t';
}

void test() {
	Tokenizer t("Test.txt");
	t.tokenize();
	size_t lineNo = 0;
	for (auto i : *t.getTokens()) {
		if (i->lineNo != lineNo) {
			lineNo = i->lineNo;
			std::cout <<std::endl << "Line" << lineNo<<":  ";
		}
		getValue(*i);
	}

	for (auto i : *t.getErrorInfo()) {
	printf("line %d : undefined sym %c in", i->lineNo, i->errorTokenStr[0]);
	std::cout << i->errorTokenStr<<std::endl;
	}
}

#endif