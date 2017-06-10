#include "stdafx.h"
#include "parse-emit-code.h"
#include "util.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		std::cerr << "������Ҫ�����Դ�ļ�..." << std::endl;
	}
	else
	{
		std::string src = argv[1];
		source_file.open(src);
		if (!source_file)
		{
			std::cerr << "��������ЧԴ�ļ�..." << std::endl;
		}
		else
		{
			if (argc == 2)
			{
				dest_file.open(src.substr(0, src.find_first_of('.')) + ".as");
			}
			if (argc == 3)
			{
				dest_file.open(argv[2]);
			}
			program();
		}
	}

	system("pause");
	return 0;
}