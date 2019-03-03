#ifndef SYMTABLE_H
#define SYMTABLE_H

#include<string>
#include<vector>
#include"AST.h"

enum Semantic_Type;
struct SymTable_Entry;
class SymTable;

enum Semantic_Type {
	INT,DOUBLE,BOOL,FUNC,NONE
};


struct SymTable_Entry {
	Semantic_Type type;
	std::string name;
	AST* ast;
};

class SymTable {
public:
	SymTable_Entry* getEntry(const std::string& str) {
		for (auto entry_ptr : _table) {
			if (entry_ptr->name == str) {
				return entry_ptr;
			}
		}
		return nullptr;
	}

	void pushEntry(SymTable_Entry* e) {
		_table.push_back(e);
	}

	void updateEntry(const std::string& id, AST* new_ast) {
		auto ptr = getEntry(id);
		if (ptr) {
			ptr->ast = new_ast;
		}
	}
private:
	std::vector<SymTable_Entry*> _table;
};
#endif