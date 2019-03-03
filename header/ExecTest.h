#ifndef EXECTEST_H
#define EXECTEST_H

#include"Eval.h"

void test() {
	Evaluator evaluator("Test.txt");
	evaluator.exec();
}

#endif // !EXECTEST_H
