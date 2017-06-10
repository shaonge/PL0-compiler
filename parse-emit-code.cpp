#include "stdafx.h"
#include "parse-emit-code.h"
#include "util.h"
#include <stack>
#include <queue>
#include <utility>
#include <numeric>
#include <iostream>
#include <fstream>

namespace
{
	std::vector<std::size_t> array_type_aux;
	Token current_exp_type = Integer;//当前表达式类型
	int current_var_offset = 1024;//当前变量偏移量，全局变量从1024开始
	int current_const_offset = 0;//当前常量偏移量，常量从0开始
	int current_local_offset = 0;//当前局部变量相对于bp寄存器的偏移量
	Token array_base_type = NUL;//数组元素类型
	pos_type main_function_pos;
	bool in_while_stmt = false;
	bool has_exit_stmt = false;
	std::string type_ident;
	Token type_aux = NUL;
}

static void match(Token t);
static void factor();
static void term();
static void simpleExp();
static void exp();
static void actParal();
static void identRef();
static void stmt();
static void forParal();
static void funcDec();
static void funcDecl();
static void type();
static void varDec();
static void varDecl();
static std::size_t typeExp();
static void typeDef();
static void typeDecl();
static void constDef();
static void constDecl();
static void block();

static void match(Token t)
{
	if (SC == t && SC != token) { return; }
	else if (token != t) { errorPrint("unexpected symbol: " + token_val); }
	else { token = getToken(); }
	if (token == NUL)
	{
		if (!error_flag)
		{
			std::cout << "编译完成..." << std::endl;
			dest_file.close();
		}
		else { std::cout << "存在错误..." << std::endl; }
		system("pause");
		exit(0);
	}
}

static void factor()
{
	current_exp_type = Integer;
	std::string id_tmp;

	switch (token)
	{
	case Id:
		if (lookup(token_val).token_type == Procedure)
		{
			errorPrint("no return value in procedure");
		}
		else if (lookup(token_val).token_type == Function)
		{
			id_tmp = token_val;
			match(Id);
			if (token == LP)
			{
				match(LP);
				actParal();
				match(RP);
			}
			emitCode("CALL " + std::to_string(lookup(id_tmp).addr_offset));
		}
		else
		{
			identRef();
			current_exp_type == Real ? emitCode("LD bx (ax)") :
				emitCode("LD ax (ax)");
		}
		break;
	case IntNum:
		emitCode("PUSH ax");
		emitCode("IMM ax " + token_val);
		match(IntNum);
		break;
	case RealNum:
		emitCode("PUSH bx");
		emitCode("IMM bx " + token_val);
		match(RealNum);
		break;
	case LP:
		match(LP);
		exp();
		match(RP);
		break;
	case Not:
		match(Not);
		factor();
		emitCode("NOT");
		break;
	case Odd:
		match(Odd);
		match(LP);
		simpleExp();
		match(RP);
		current_exp_type = Boolean;
		emitCode("ODD");
		break;
	case True:
		current_exp_type = Boolean;
		match(True);
		emitCode("PUSH ax");
		emitCode("IMM ax 1");
		break;
	case False:
		current_exp_type = Boolean;
		match(False);
		emitCode("PUSH ax");
		emitCode("IMM ax 0");
		break;
	default:
		break;
	}
}

static void term()
{
	factor();
	while (token == MULT || token == Div || token == DIV ||
		token == Mod || token == And)
	{
		if (token == MULT)
		{
			match(MULT);
			factor();
			current_exp_type == Real ? emitCode("MUL bx (sp)") :
				emitCode("MUL ax (sp)");
		}
		else if (token == Div)
		{
			match(Div);
			factor();
			if (current_exp_type == Real) { errorPrint("div can't be used to real operators"); }
			else { emitCode("DIV ax (sp)"); }
		}
		else if (token == DIV)
		{
			match(DIV);
			factor();
			if (current_exp_type != Real)
			{
				emitCode("PUSH ax");
				emitCode("POP bx");
			}
			emitCode("DIV bx (sp)");
		}
		else if (token == Mod)
		{
			match(Mod);
			factor();
			current_exp_type == Real ? emitCode("MOD bx (sp)") :
				emitCode("MOD ax (sp)");
		}
		else
		{
			pos_type pos = emitCodeRollback("JZ ");
			match(And);
			factor();
			emitCode("AND ax (sp)");
			rollback(std::to_string(code_line_num), pos);
		}
	}
}

static void simpleExp()
{
	if (token == MINUS)
	{
		match(MINUS);
		term();
		current_exp_type == Real ? emitCode("NEG bx") :
			emitCode("NEG ax");
	}
	else if (token == PLUS)
	{
		match(PLUS);
		term();
	}
	else
	{
		term();
	}

	while (token == PLUS || token == MINUS || token == Or)
	{
		if (token == PLUS)
		{
			match(PLUS);
			term();
			current_exp_type == Real ? emitCode("ADD bx (sp)") :
				emitCode("ADD ax (sp)");
		}
		else if (token == MINUS)
		{
			match(MINUS);
			term();
			current_exp_type == Real ? emitCode("SUB bx (sp)") :
				emitCode("SUB ax (sp)");
		}
		else
		{
			pos_type pos = emitCodeRollback("JNZ ");
			match(Or);
			term();
			emitCode("OR ax (sp)");
			rollback(std::to_string(code_line_num), pos);
		}
	}
}

static void exp()
{
	simpleExp();
	if (token == EQ)
	{
		match(EQ);
		simpleExp();
		current_exp_type == Real ? emitCode("EQ bx (sp)") :
			emitCode("EQ ax (sp)");
	}
	else if (token == NE)
	{
		match(NE);
		simpleExp();
		current_exp_type == Real ? emitCode("NE bx (sp)") :
			emitCode("NE ax (sp)");
	}
	else if (token == LT)
	{
		match(LT);
		simpleExp();
		current_exp_type == Real ? emitCode("LT bx (sp)") :
			emitCode("LT ax (sp)");
	}
	else if (token == GT)
	{
		match(GT);
		simpleExp();
		current_exp_type == Real ? emitCode("GT bx (sp)") :
			emitCode("GT ax (sp)");
	}
	else if (token == LE)
	{
		match(LE);
		simpleExp();
		current_exp_type == Real ? emitCode("LE bx (sp)") :
			emitCode("LE ax (sp)");
	}
	else if (token == GE)
	{
		match(GE);
		simpleExp();
		current_exp_type == Real ? emitCode("GE bx (sp)") :
			emitCode("GE ax (sp)");
	}
}

static void actParal()
{
	exp();
	current_exp_type == Real ? emitCode("PUSH bx") :
		emitCode("PUSH ax");
	while (token != RP)
	{
		match(COMMA);
		exp();
		current_exp_type == Real ? emitCode("PUSH bx") :
			emitCode("PUSH ax");
	}
}

static void identRef()
{
	IdNode& in = lookup(token_val);
	match(Id);

	if (in.val_type == Array)
	{
		std::vector<std::size_t> vec = in.array_type->size_per_dimension;
		std::size_t d1 = in.array_type->dimension;
		std::size_t d2 = d1;
		std::size_t index_tmp = 0;

		while (d1-- > 1)
		{
			match(LA);
			exp();
			emitCode("PUSH ax");
			emitCode("IMM ax " + std::to_string(vec.at(index_tmp++)));
			emitCode("MUL ax (sp)");
			match(RA);
		}
		match(LA);
		exp();
		while (d2-- > 1) { emitCode("ADD ax (sp)"); }
		match(RA);

		current_exp_type = in.array_type->base_type;
		if (current_exp_type == Integer || current_exp_type == Real)
		{
			emitCode("PUSH ax");
			emitCode("IMM ax 4");
			emitCode("MUL ax (sp)");
		}
		if (in.scope_type == Global)
		{
			emitCode("LDA ax ax(" + std::to_string(in.addr_offset) + ")");
		}
		else
		{
			if (in.scope_type == Param) { emitCode("NEG ax"); }
			emitCode("LDA ax bp(ax(" + std::to_string(in.addr_offset) + "))");
		}
	}

	else
	{
		current_exp_type == Real ? emitCode("PUSH bx") :
			emitCode("PUSH ax");
		if (in.scope_type == Global)
		{
			emitCode("IMM ax " + std::to_string(in.addr_offset));
		}
		else
		{
			emitCode("LDA ax bp(" + std::to_string(in.addr_offset) + ")");
		}
		current_exp_type = in.val_type;
	}
}

static void stmt()
{
	std::string id_tmp;
	pos_type pos1, pos2, pos3;
	std::size_t line_num_tmp = 0;
	switch (token)
	{
	case Id:
		identRef();
		match(ASSIGN);
		exp();
		current_exp_type == Real ? emitCode("STO (sp) bx") :
			emitCode("STO (sp) ax");
		break;
	case If:
		match(If);
		exp();
		pos1 = emitCodeRollback("JZ ");
		match(Then);
		stmt();
		if (token == Else)
		{
			pos_type p = emitCodeRollback("JMP ");
			rollback(std::to_string(code_line_num), pos1);
			match(Else);
			stmt();
			rollback(std::to_string(code_line_num), p);
		}
		else
		{
			rollback(std::to_string(code_line_num), pos1);
		}
		break;
	case Begin:
		match(Begin);
		stmt();
		while (token != End)
		{
			match(SC);
			stmt();
		}
		match(End);
		break;
	case While:
		in_while_stmt = true;
		match(While);
		line_num_tmp = code_line_num;
		exp();
		pos2 = emitCodeRollback("JZ ");
		match(Do);
		stmt();
		emitCode("JMP " + std::to_string(line_num_tmp));
		rollback(std::to_string(code_line_num), pos2);
		if (has_exit_stmt)
		{
			rollback(std::to_string(code_line_num), pos3);
			has_exit_stmt = false;
		}
		in_while_stmt = false;
		break;
	case Exit:
		if (!in_while_stmt)
		{
			errorPrint("exit statement can't be used to other statements except while");
		}
		match(Exit);
		has_exit_stmt = true;
		pos3 = emitCodeRollback("JMP ");
		break;
	case Call:
		match(Call);
		id_tmp = token_val;
		match(Id);
		if (token == LP)
		{
			match(LP);
			actParal();
			match(RP);
		}
		if (lookup(id_tmp).token_type == Procedure)
		{
			emitCode("CALL " + std::to_string(lookup(id_tmp).addr_offset));
		}
		else { errorPrint("call statement can only be used to invoke procedure"); }
		break;
	case Write:
		match(Write);
		match(LP);
		exp();
		if (current_exp_type == Real) { emitCode("WRITE bx"); }
		else if (current_exp_type == Integer) { emitCode("WRITE ax"); }
		else { errorPrint("type of expression in write statement can't be other types except real and integer"); }

		while (token != RP)
		{
			match(COMMA);
			exp();
			if (current_exp_type == Real) { emitCode("WRITE bx"); }
			else if (current_exp_type == Integer) { emitCode("WRITE ax"); }
			else { errorPrint("type of expression in write statement can't be other types except real and integer"); }
		}
		match(RP);
		break;
	case Read:
		match(Read);
		match(LP);
		identRef();
		if (current_exp_type == Boolean)
		{
			errorPrint("type of expression in read statement can't be boolean");
		}
		emitCode("PUSH ax");
		emitCode("READ (sp)");

		while (token != RP)
		{
			match(COMMA);
			identRef();
			if (current_exp_type == Boolean)
			{
				errorPrint("type of expression in read statement can't be boolean");
			}
			emitCode("PUSH ax");
			emitCode("READ (sp)");
		}
		match(RP);
		break;
	case Else:
	case End:
	case SC:
		break;
	default:
		errorPrint("unexpected token");
		match(token);
		break;
	}
}

static void forParal()
{
	current_local_offset = 0;
	bool flag = false;
	std::stack<std::pair<std::string, std::size_t>> params;

	while (token != RP)
	{
		if (flag) { match(SC); }
		flag = true;
		std::string id_tmp = token_val;
		match(Id);
		match(COLON);
		type();
		std::size_t type_size = 0;
		if (type_aux != Array)
		{
			type_size = (type_aux == Boolean ? 1 : 4);
			current_scope->symtab.insert({ id_tmp, { Var, type_aux,
				std::string(), line_num, type_size, 0, Param, nullptr } });
		}
		else
		{
			std::shared_ptr<ArrayType> array_tmp = lookup(type_ident).array_type;
			type_size = lookup(type_ident).val_size;
			current_scope->symtab.insert({ id_tmp, { Var, Array,
				std::string(), line_num, type_size, 0, Param, array_tmp } });
		}
		params.push(std::make_pair(id_tmp, type_size));
	}

	current_local_offset -= 8;
	while (!params.empty())
	{
		lookup(params.top().first).addr_offset = current_local_offset;
		current_local_offset -= static_cast<int>(params.top().second);
		params.pop();
	}
}

static void funcDec()
{
	current_local_offset = 0;
	if (token != Function && token != Procedure)
	{
		errorPrint("unexpected token");
		match(token);
	}
	else
	{
		tree_node scope_tmp = nullptr;
		Token token_tmp = token;
		match(token);
		std::string id_tmp = token_val;
		match(Id);
		if (token == LP)
		{
			match(LP);
			current_scope = createNewScope();
			current_scope->is_global_scope = false;
			forParal();
			scope_tmp = current_scope;
			current_scope = current_scope->parentScope;
			match(RP);
		}
		if (token_tmp == Function)
		{
			match(COLON);
			type();
		}

		match(SC);
		if (token_tmp == Function)
		{
			if (type_aux != Array)
			{
				current_scope->symtab.insert({ id_tmp, { Function, type_aux, std::string(),
					line_num, 0, code_line_num, current_scope->is_global_scope ? Global : Local, nullptr } });
			}
			else
			{
				std::shared_ptr<ArrayType> array_tmp = lookup(type_ident).array_type;
				current_scope->symtab.insert({ id_tmp, { Function, Array, std::string(), line_num,
					0, code_line_num, current_scope->is_global_scope ? Global : Local, array_tmp } });
			}
		}
		else
		{
			current_scope->symtab.insert({ id_tmp, { Procedure, NUL, std::string(), line_num,
				0, code_line_num, current_scope->is_global_scope ? Global : Local, nullptr } });
		}

		tree_node s = current_scope;
		current_scope = (scope_tmp == nullptr ?
			createNewScope() : scope_tmp);
		current_scope->is_global_scope = false;
		current_local_offset = 0;
		block();
		current_scope = s;
	}
}

static void funcDecl()
{
	do
	{
		funcDec();
		match(SC);

	} while (token != Begin);
}

static void type()
{
	if (token != Integer && token != Real &&
		token != Boolean && token != Id)
	{
		errorPrint("unexpected token");
	}

	if (token != Id)
	{
		type_aux = token;
	}
	else
	{
		type_aux = lookup(token_val).val_type;
		type_ident = token_val;
	}
	match(token);
}

static void varDec()
{
	std::queue<std::string> id_tmps;
	id_tmps.push(token_val);
	match(Id);
	while (token != COLON)
	{
		match(COMMA);
		id_tmps.push(token_val);
		match(Id);
	}
	match(COLON);
	type();
	if (type_aux != Array)
	{
		if (current_scope->is_global_scope)
		{
			std::size_t type_size = (type_aux == Boolean ? 1 : 4);
			while (!id_tmps.empty())
			{
				current_scope->symtab.insert({ id_tmps.front(), { Var, type_aux, std::string(),
					line_num, type_size, current_var_offset, Global, nullptr } });
				current_var_offset += type_size;
				id_tmps.pop();
			}
		}
		else
		{
			if (type_aux == Real)
			{
				while (!id_tmps.empty())
				{
					current_scope->symtab.insert({ id_tmps.front(), { Var, type_aux, std::string(),
						line_num, 4, current_local_offset, Local, nullptr } });
					emitCode("IMM bx 0");
					emitCode("PUSH bx");
					current_local_offset += 4;
					id_tmps.pop();
				}
			}
			else
			{
				while (!id_tmps.empty())
				{
					current_scope->symtab.insert({ id_tmps.front(), { Var, type_aux, std::string(),
						line_num, 4, current_local_offset, Local, nullptr } });
					emitCode("IMM ax 0");
					emitCode("PUSH ax");
					current_local_offset += 4;
					id_tmps.pop();
				}
			}
		}
	}
	else
	{
		std::shared_ptr<ArrayType> array_tmp = lookup(type_ident).array_type;
		std::size_t type_size = lookup(type_ident).val_size;
		if (current_scope->is_global_scope)
		{
			while (!id_tmps.empty())
			{
				current_scope->symtab.insert({ id_tmps.front(), { Var, Array, std::string(), line_num,
					type_size,  current_var_offset, Global, array_tmp } });
				current_var_offset += type_size;
				id_tmps.pop();
			}
		}
		else
		{
			std::size_t t = std::accumulate(array_tmp->size_per_dimension.begin(),
				array_tmp->size_per_dimension.end(), 1, std::multiplies<std::size_t>());
			if (type_aux == Real)
			{
				while (!id_tmps.empty())
				{
					current_scope->symtab.insert({ id_tmps.front(), { Var, type_aux, std::string(),
						line_num, type_size, current_local_offset, Local, nullptr } });
					while (t-- > 0)
					{
						emitCode("IMM bx 0");
						emitCode("PUSH bx");
					}			
					current_local_offset += type_size;
					id_tmps.pop();
				}
			}
			else
			{
				type_size = (type_aux == Boolean ? 4 * type_size : type_size);
				while (!id_tmps.empty())
				{
					current_scope->symtab.insert({ id_tmps.front(), { Var, type_aux, std::string(),
						line_num, type_size, current_local_offset, Local, nullptr } });
					while (t-- > 0)
					{
						emitCode("IMM ax 0");
						emitCode("PUSH ax");
					}
					current_local_offset += type_size;
					id_tmps.pop();
				}
			}
		}
	}
}

static void varDecl()
{
	match(Var);
	do
	{
		varDec();
		match(SC);

	} while (token != Procedure && token != Function && token != Begin);
}

static std::size_t typeExp()
{
	type_aux = token;

	if (token == Integer || token == Real)
	{
		array_base_type = token;
		match(token);
		return 4;
	}
	else if (token == Boolean)
	{
		array_base_type = Boolean;
		match(Boolean);
		return 1;
	}
	else
	{
		match(Array);
		match(LA);
		std::string num_tmp1 = token_val;
		match(IntNum);
		match(DOT);
		match(DOT);
		std::string num_tmp2 = token_val;
		match(IntNum);
		match(RA);
		match(Of);
		std::size_t exp_size = typeExp();
		type_aux = Array;
		std::size_t size_tmp = std::stoi(num_tmp2)
			- std::stoi(num_tmp1) + 1;
		array_type_aux.push_back(size_tmp);
		return exp_size * size_tmp;
	}
}

static void typeDef()
{
	std::string id_tmp = token_val;
	match(Id);
	match(EQ);
	std::size_t size_tmp = typeExp();
	if (type_aux == Array)
	{
		current_scope->symtab.insert({ id_tmp,{ Type, Array, std::string(),
			line_num, size_tmp, 0, current_scope->is_global_scope ? Global : Local,
			std::make_shared<ArrayType>(array_base_type, array_type_aux.size(), array_type_aux) } });
		array_type_aux.clear();
	}
	else { errorPrint("type in type definition can't be other types except array"); }
}

static void typeDecl()
{
	match(Type);
	do
	{
		typeDef();
		match(SC);

	} while (token != Var && token != Procedure
		&& token != Function && token != Begin);
}

static void constDef()
{
	std::string id_tmp = token_val;
	match(Id);
	match(EQ);
	if (current_scope->is_global_scope)
	{
		current_scope->symtab.insert({ id_tmp, { Const, token == IntNum ? Integer : Real,
			token_val, line_num, 4, current_const_offset, Global, nullptr } });
		current_const_offset += 4;
	}
	else
	{
		if (token == IntNum)
		{
			current_scope->symtab.insert({ id_tmp,{ Const, Integer, token_val,
				line_num, 4, current_local_offset, Local, nullptr } });
			emitCode("IMM ax " + token_val);
			emitCode("PUSH ax");
		}
		else if (token == RealNum)
		{
			current_scope->symtab.insert({ id_tmp,{ Const, Real, token_val,
				line_num, 4, current_local_offset, Local, nullptr } });
			emitCode("IMM bx " + token_val);
			emitCode("PUSH bx");
		}
		current_local_offset += 4;
	}

	match(token);
}

static void constDecl()
{
	match(Const);
	do
	{
		constDef();
		match(SC);

	} while (token != Type && token != Var && token != Procedure
		&& token != Function && token != Begin);
}

static void block()
{
	if (!current_scope->is_global_scope)
	{
		emitCode("ENT");
	}
	
	switch (token)
	{
	case Const:
		constDecl();
		switch (token)
		{
		case Type:
			typeDecl();
			if (token == Var)
			{
				varDecl();
				if (token == Procedure || token == Function)
				{
					funcDecl();
				}
			}
			else if (token == Procedure || token == Function)
			{
				funcDecl();
			}
			break;
		case Var:
			varDecl();
			if (token == Procedure || token == Function)
			{
				funcDecl();
			}
			break;
		case Procedure:
		case Function:
			funcDecl();
			break;
		default:
			break;
		}
		break;
	case Type:
		typeDecl();
		if (token == Var)
		{
			varDecl();
			if (token == Procedure || token == Function)
			{
				funcDecl();
			}
		}
		else if (token == Procedure || token == Function)
		{
			funcDecl();
		}
		break;
	case Var:
		varDecl();
		if (token == Procedure || token == Function)
		{
			funcDecl();
		}
		break;
	case Procedure:
	case Function:
		funcDecl();
		break;
	default:
		break;
	}

	match(Begin);
	if (current_scope->is_global_scope)
	{
		rollback(std::to_string(code_line_num), main_function_pos);
		emitCode("ENT");
	}
	stmt();
	while (token != End)
	{
		match(SC);
		stmt();
	}
	emitCode("LEV");
	match(End);
}

void program()
{
	current_scope = createNewScope();
	current_scope->is_global_scope = true;
	token = getToken();
	main_function_pos = emitCodeRollback("CALL ");
	emitCode("HLT");
	block();
	match(DOT);
}