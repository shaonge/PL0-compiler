#include "stdafx.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include <algorithm>

tree_node current_scope = nullptr;
int code_line_num = 1;
std::ofstream dest_file;
bool error_flag = false;

void errorPrint(const std::string& message)
{
	std::cerr << "error(line: " << line_num << "): " << message << std::endl;
	token = getToken();
	error_flag = true;
	dest_file.();
}

tree_node createNewScope()
{
	tree_node ptr = std::make_shared<ScopeTreeNode>(current_scope);
	return ptr;
}

void emitCode(const std::string& code)
{
	dest_file << code_line_num++ << ": " << code << std::endl;
}

pos_type emitCodeRollback(const std::string& code)
{
	dest_file << code_line_num++ << ": " << code;
	pos_type pos = dest_file.tellp();
	dest_file << "\t\t\t" << std::endl;
	return pos;
}

void rollback(const std::string& code, pos_type pos)
{
	pos_type p = dest_file.tellp();
	dest_file.seekp(pos);
	dest_file << code;
	dest_file.seekp(p);
}

IdNode& lookup(const std::string& id)
{
	tree_node s1 = current_scope;
	tree_node s2 = nullptr;
	while (current_scope != nullptr)
	{
		if (current_scope->symtab.find(id) !=
			current_scope->symtab.end())
		{
			s2 = current_scope;
			current_scope = s1;
			return s2->symtab[id];
		}
		current_scope = current_scope->parentScope;
	}
	errorPrint("no definition: " + id);
	return *(new IdNode());
}

void printToken()
{
	Token token = getToken();
	std::cout << token << "(line: " << line_num << "): " << token_val << std::endl;
}

void printSymtab()
{
	for (const auto& x : current_scope->symtab)
	{
		std::cout << x.first << "(line: " << x.second.line_num << "): "
			<< x.second.addr_offset << " " << x.second.val_size << std::endl;
	}
}