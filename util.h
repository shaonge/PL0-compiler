#pragma once
#include "scan.h"
#include <vector>
#include <memory>
#include <unordered_map>

enum ScopeType
{
	 Global, Param, Local
};

struct ArrayType//数组类型专用
{
	Token base_type;//元素类型
	std::size_t dimension;//维度
	std::vector<std::size_t> size_per_dimension;//每维元素数

	ArrayType(Token t, std::size_t s, const std::vector<std::size_t>& v) :
		base_type(t), dimension(s), size_per_dimension(v) {}
};

struct IdNode//符号表节点
{
	Token token_type;//节点类型
	Token val_type;//元素类型，如果是Function，则为返回值类型
	std::string val;//变量、常数的值
	std::size_t line_num;//所在源代码行号
	std::size_t val_size;//变量所占内存大小，如果为数组，则为数组全部元素大小之和
	int addr_offset;//变量、常数和函数的地址
	ScopeType scope_type;//作用域
	std::shared_ptr<ArrayType> array_type;//如果val_type不为Array，则此指针为空
};

struct ScopeTreeNode//作用域树节点
{
	std::shared_ptr<ScopeTreeNode> parentScope;//上一级作用域
	std::unordered_map<std::string, IdNode> symtab;//本作用域的符号表
	bool is_global_scope;//是否是全局作用域

	ScopeTreeNode(std::shared_ptr<ScopeTreeNode> parent) :
		parentScope(parent) {}
};

using tree_node = std::shared_ptr<ScopeTreeNode>;
using pos_type = std::streampos;

extern tree_node current_scope;
extern int code_line_num;//目标代码当前行号
extern std::ofstream dest_file;//目标文件
extern bool error_flag;//源代码是否有错误

tree_node createNewScope();//创建一个新作用域

void errorPrint(const std::string& message);//输出错误信息

void emitCode(const std::string& code);//在目标文件中输出代码
pos_type emitCodeRollback(const std::string& code);//在目标文件中输出代码,并暂存当前位置以便回溯使用
void rollback(const std::string& code, pos_type pos);//目标代码回溯

IdNode& lookup(const std::string& id);//在符号表中查找指定符号

void printToken();
void printSymtab();