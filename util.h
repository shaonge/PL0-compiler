#pragma once
#include "scan.h"
#include <vector>
#include <memory>
#include <unordered_map>

enum ScopeType
{
	 Global, Param, Local
};

struct ArrayType//��������ר��
{
	Token base_type;//Ԫ������
	std::size_t dimension;//ά��
	std::vector<std::size_t> size_per_dimension;//ÿάԪ����

	ArrayType(Token t, std::size_t s, const std::vector<std::size_t>& v) :
		base_type(t), dimension(s), size_per_dimension(v) {}
};

struct IdNode//���ű�ڵ�
{
	Token token_type;//�ڵ�����
	Token val_type;//Ԫ�����ͣ������Function����Ϊ����ֵ����
	std::string val;//������������ֵ
	std::size_t line_num;//����Դ�����к�
	std::size_t val_size;//������ռ�ڴ��С�����Ϊ���飬��Ϊ����ȫ��Ԫ�ش�С֮��
	int addr_offset;//�����������ͺ����ĵ�ַ
	ScopeType scope_type;//������
	std::shared_ptr<ArrayType> array_type;//���val_type��ΪArray�����ָ��Ϊ��
};

struct ScopeTreeNode//���������ڵ�
{
	std::shared_ptr<ScopeTreeNode> parentScope;//��һ��������
	std::unordered_map<std::string, IdNode> symtab;//��������ķ��ű�
	bool is_global_scope;//�Ƿ���ȫ��������

	ScopeTreeNode(std::shared_ptr<ScopeTreeNode> parent) :
		parentScope(parent) {}
};

using tree_node = std::shared_ptr<ScopeTreeNode>;
using pos_type = std::streampos;

extern tree_node current_scope;
extern int code_line_num;//Ŀ����뵱ǰ�к�
extern std::ofstream dest_file;//Ŀ���ļ�
extern bool error_flag;//Դ�����Ƿ��д���

tree_node createNewScope();//����һ����������

void errorPrint(const std::string& message);//���������Ϣ

void emitCode(const std::string& code);//��Ŀ���ļ����������
pos_type emitCodeRollback(const std::string& code);//��Ŀ���ļ����������,���ݴ浱ǰλ���Ա����ʹ��
void rollback(const std::string& code, pos_type pos);//Ŀ��������

IdNode& lookup(const std::string& id);//�ڷ��ű��в���ָ������

void printToken();
void printSymtab();