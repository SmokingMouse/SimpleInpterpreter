#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#define SCOPE_ID "***"
#define RETURN_ID "%RTN"
#define FUNC_ID "%FUNC"

#include"SymTable.h"
#include<map>

using std::pair;
using std::map;
using std::string;
using std::vector;

//1.普通作用域与函数作用域不同，普通作用域可以访问到上级作用域，
//		函数作用域能访问到普通作用域，不能单纯访问上级作用域
//让普通作用域和函数作用域的findEntry有相同的结果
//		1,在同一表中插入多个标志符。以体现出当前在普通还是函数。
//		2,采用两个表，用额外的状态标记。
// 我采用的是第二种办法。
class Env_Entry {
public:
	Semantic_Type type;
	union {
		double dbl_val;
		int int_val;
		bool bool_val;
	} val;

	Env_Entry() :type(NONE), val{ 0 } {}
	Env_Entry(bool v) :type(BOOL) { val.bool_val = v; }
	Env_Entry(double v) :val{ v }, type(DOUBLE){}
	Env_Entry(int v) :type(INT) { val.int_val = v; }
};


class Env {
private:
	vector <pair<string, Env_Entry>> _table;
	vector <pair<string, Env_Entry>> _stack;
	//map<string, Env_Entry> _table;
	int _is_in_func;
	bool _rtn_state;

	Env_Entry* _search_in_table(const string& id) {
		if (_table.empty()) return NULL;
		for (auto iter = _table.rbegin(); iter != _table.rend(); iter++) {
			if (iter->first == id) return &iter->second;
		}
		return NULL;
	}

	void _clear(bool clearFunc) {
		vector<pair<string, Env_Entry>>* env;
		string id = clearFunc ? FUNC_ID : SCOPE_ID;
		if (_is_in_func) env = &_stack;
		else env = &_table;
		int idx;
		for (idx = env->size() - 1; idx >= 0; idx--) {
			if (env->at(idx).first == id);
				break;
		}
		env->resize(idx);
	}
public:
	bool isRtnState() {
		return _rtn_state;
	}

	void setRtnState(bool s) {
		_rtn_state = s;
	}

	Env():_stack(),_table(),_is_in_func(0){}

	void pushValue(const string& id, const Env_Entry& val) {
		if (_is_in_func) _stack.push_back({ id,val });
		else _table.push_back({ id,val });
	}

	Env_Entry* findEntry(const string& id) {
		if (_is_in_func) {
			if (_stack.empty()) return _search_in_table(id);
			for (auto iter = _stack.rbegin(); iter != _stack.rend(); iter++) {
				if (iter->first == FUNC_ID) return _search_in_table(id);
				if (iter->first == id) return &iter->second;
			}
		}
		return _search_in_table(id);
	}

	void enterFuncScope() {
		_is_in_func++;
		
//		pushValue(FUNC_ID, Env_Entry());
	}

	void insertFuncId(size_t idx) {
		_stack.insert(_stack.end() - idx, { FUNC_ID,Env_Entry() });
	}

	void exitFuncScope() {
		_clear(1);
		_rtn_state = false;
		_is_in_func--;
	}

	void enterScope() {
		pushValue(SCOPE_ID, Env_Entry());
	}

	void exitScope() {
		_clear(0);
	}

	Env_Entry getRtnEntry() {
		if (!_stack.empty()&&_stack.back().first == RETURN_ID) return _stack.back().second;
		return Env_Entry();
	}
};



#endif
