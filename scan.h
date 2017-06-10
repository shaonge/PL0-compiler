#pragma once
#include <string>

enum Token
{
	NUL, Id, IntNum, RealNum,
	//�ؼ���
	If, Then, Else, Begin, End, While, Do, Exit, Call, Write, Read, Procedure, Function, Var,
	Type, Const, Integer, Real, Boolean, Array, Of, True, False, Odd, Not, Or, And, Div, Mod,
	//���ţ�LA('['), SC(';'), COMMA(','), LB('{'), LP('('), DOT('.')
	PLUS, MINUS, MULT, DIV, EQ, LT, GT, LE, GE, NE, COLON, ASSIGN, LB, RB, LP, RP, LA, RA, SC, COMMA, DOT
};

extern std::string token_val;//��ǰtokenֵ
extern std::size_t line_num;//�Ѿ���ȡ����Դ�����к�
extern std::fstream source_file;//Դ�ļ�
extern Token token;//��ǰtoken

Token getToken();//�õ���һ��token