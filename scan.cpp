#include "stdafx.h"
#include "scan.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cctype>

std::string token_val;
std::size_t line_num = 0;
Token token = NUL;
std::fstream source_file;

static std::string line_buffer;
static std::string last_line_buffer;
static std::size_t line_pos = 0;
static bool back_to_last_line = false;

std::unordered_map<std::string, Token> reserved_words
{
	{ "if", If }, { "then", Then }, { "else", Else }, { "begin", Begin }, { "end", End },
	{ "while", While }, { "do", Do }, { "exit", Exit }, { "call", Call }, { "write", Write },
	{ "read", Read }, { "procedure", Procedure }, { "function", Function }, { "var", Var },
	{ "type", Type }, { "const", Const }, { "integer", Integer }, { "real", Real },
	{ "boolean", Boolean }, { "array", Array }, { "of", Of }, { "true", True },
	{ "false", False }, { "odd", Odd }, { "or", Or }, { "not", Not },
	{ "and", And }, { "div", Div }, { "mod", Mod},
};

static int getNextChar()
{
	if (line_pos >= line_buffer.size())
	{
		last_line_buffer = line_buffer;
		while (std::getline(source_file, line_buffer))
		{
			++line_num;
			if (line_buffer.empty()) { continue; }
			line_pos = 0;
			line_buffer.push_back('\n');
			return line_buffer.at(line_pos++);
		}
		source_file.close();
		return EOF;
	}

	else if (back_to_last_line && !last_line_buffer.empty())
	{
		return last_line_buffer.back();
	}
	return line_buffer.at(line_pos++);
}

static void ungetChar(int c)
{
	if (line_pos == 0 && last_line_buffer.back() == c) { back_to_last_line = true; }
	else if (line_buffer.at(line_pos - 1)) { --line_pos; }
	else { std::cerr << "failing to rollback to last char" << std::endl; }
}

static Token reservedLookup(const std::string& str)
{
	if (reserved_words.find(str) != reserved_words.end())
	{
		return reserved_words[str];
	}

	return Id;
}

Token getToken()
{
	token_val.clear();
	token_val.reserve(10);
	int c = 0;
	while ((c = getNextChar()) != EOF)
	{
		if (isspace(c)) { continue; }
		else if (isalpha(c) || '_' == c)
		{
			token_val.push_back(c);
			while (isalnum(c = getNextChar()) || '_' == c)
			{
				token_val.push_back(c);
			}
			ungetChar(c);
			return reservedLookup(token_val);
		}
		else if (isdigit(c))
		{
			do
			{
				token_val.push_back(c);
			} while (isdigit(c = getNextChar()));

			if ('.' == c)
			{
				if (isdigit(c = getNextChar()))
				{
					token_val.push_back('.');
					do
					{
						token_val.push_back(c);
					} while (isdigit(c = getNextChar()));
					ungetChar(c);
					return RealNum;
				}
				ungetChar(c);
				ungetChar('.');
				return IntNum;
			}

			ungetChar(c);
			return IntNum;
		}
		else if ('/' == c)
		{
			if ((c = getNextChar()) == '/')
			{
				line_buffer.clear();
				continue;
			}
			else if ('*' == c)
			{
				while (true)
				{
					while ((c = getNextChar()) != '*') {}
					if ((c = getNextChar()) != '/') { ungetChar(c); }
					else { break; }
				}
				continue;
			}
			else
			{
				ungetChar(c);
				token_val.push_back('/');
				return DIV;
			}
			break;
		}
		else if ('<' == c)
		{
			token_val.push_back('<');
			if ((c = getNextChar()) == '=')
			{
				token_val.push_back('=');
				return LE;
			}
			else if ('>' == c)
			{
				token_val.push_back('>');
				return NE;
			}
			ungetChar(c);
			return LT;
		}
		else if ('>' == c)
		{
			token_val.push_back('>');
			if ((c = getNextChar()) == '=')
			{
				token_val.push_back('=');
				return GE;
			}
			ungetChar(c);
			return GT;
		}
		else if (':' == c)
		{
			token_val.push_back(':');
			if ((c = getNextChar()) == '=')
			{
				token_val.push_back('=');
				return ASSIGN;
			}
			ungetChar(c);
			return COLON;
		}
		else
		{
			token_val.push_back(c);
			switch (c)
			{
			case '+':
				return PLUS;
			case '-':
				return MINUS;
			case '*':
				return MULT;
			case '=':
				return EQ;
			case '{':
				return LB;
			case '}':
				return RB;
			case '(':
				return LP;
			case ')':
				return RP;
			case '[':
				return LA;
			case ']':
				return RA;
			case ';':
				return SC;
			case ',':
				return COMMA;
			case '.':
				return DOT;
			default:
				break;
			}
		}
	}

	return NUL;
}