#pragma once
#include <string>

enum Token
{
	NUL, Id, IntNum, RealNum,
	//关键字
	If, Then, Else, Begin, End, While, Do, Exit, Call, Write, Read, Procedure, Function, Var,
	Type, Const, Integer, Real, Boolean, Array, Of, True, False, Odd, Not, Or, And, Div, Mod,
	//符号，LA('['), SC(';'), COMMA(','), LB('{'), LP('('), DOT('.')
	PLUS, MINUS, MULT, DIV, EQ, LT, GT, LE, GE, NE, COLON, ASSIGN, LB, RB, LP, RP, LA, RA, SC, COMMA, DOT
};

extern std::string token_val;//当前token值
extern std::size_t line_num;//已经读取到的源代码行号
extern std::fstream source_file;//源文件
extern Token token;//当前token

Token getToken();//得到下一个token