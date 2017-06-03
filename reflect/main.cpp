#include <stdio.h>
#include <string>

#include <windows.h>
#include <experimental\filesystem>

#include "def.h"

#define EXTRA \
"#include \"utils.h\"\n \
#include \"render.h\"\n"

#define INPUT "../src/render.h"

#define OUTPUT "../src/reflect.cpp"

#ifndef EXTRA
#define EXTRA "\n"
#endif

const std::string defaultSkip[] = {""};

#ifndef SKIP
#define SKIP defaultSkip
#endif

#ifndef INPUT
#define INPUT ""
#endif

#ifndef OUTPUT
#define OUTPUT ""
#endif

bool needSkip(const std::string &name)
{
	for (auto i = 0; i < ARRAYSIZE(SKIP); i++)
	{
		if (SKIP[i] == name)
			return true;
	}
	return false;
}

extern "C" {
	extern FILE *yyin;
	extern int yylex();
	extern int yylineno;
	extern char *yytext;
}

int current = -1;
std::string currentStructName;
std::string currentEnumName;

int main(int argc, char **argv)
{
	{
		if (!std::experimental::filesystem::exists(INPUT))
			return 1;
		if (!std::experimental::filesystem::exists(OUTPUT))
			return 1;

		auto input_last_modification_time = std::experimental::filesystem::last_write_time(INPUT);
		auto output_last_modification_time = std::experimental::filesystem::last_write_time(OUTPUT);
		if (output_last_modification_time > input_last_modification_time)
			return 0;
	}

	yyin = fopen(INPUT, "rb");

	FILE *fout = fopen(OUTPUT, "wb");

	std::string declString;
	std::string implString;

	int token = yylex();
	while(token)
	{
		switch (token)
		{
			case REFLECTABLE:
			{
				int token0 = yylex();
				if (token0 == STRUCT)
				{
					int token1 = yylex();
					if (token1 != IDENTIFIER)
						return 1;

					current = 0;
					currentStructName = yytext;
					if (!needSkip(currentStructName))
					{
						printf("current struct: %s\n", yytext);
						declString += "tke::ReflectionBank *" + currentStructName + "::b = tke::addReflectionBank(\"" + currentStructName + "\");\n";
						implString += "currentBank = " + currentStructName + "::b;\n";
					}
				}
				else if (token0 == ENUM)
				{
					int token1 = yylex();
					if (token1 != IDENTIFIER)
						return 1;

					token1 = yylex();
					if (token1 != IDENTIFIER)
						return 1;

					current = 1;
					currentEnumName = yytext;
					if (!needSkip(currentEnumName))
					{
						printf("current enum: %s\n", yytext);
						implString += "currentEnum = tke::addReflectEnum(\"" + currentEnumName + "\");\n";
					}
				}
				else
				{
					return 1;
				}
			}
				break;
			case REFLv:
			{
				if (current != 0)
					return 1;

				std::string typeName;
				std::string valueName;

				int token1 = yylex();
				if (token1 != IDENTIFIER)
					return 1;
				typeName = yytext;
				int token2 = yylex();
				if (token2 != IDENTIFIER)
					return 1;
				valueName = yytext;

				if (!needSkip(currentStructName))
				{
					printf("reflect: %s %s\n", typeName.c_str(), valueName.c_str());
					implString += "currentBank->addV<" + typeName + ">(\"" + valueName + "\", offsetof(" + currentStructName + ", " + valueName + "));\n";
				}
			}
				break;
			case REFLe:
			{
				if (current == -1)
					return 1;

				if (current == 1)
				{
					std::string name;

					int token1 = yylex();
					if (token1 != IDENTIFIER)
						return 1;
					name = yytext;

					if (!needSkip(currentEnumName))
					{
						printf("reflect: %s\n", name.c_str());
						implString += "currentEnum->items.emplace_back(\"" + name + "\", (int)" + currentEnumName + "::" + name + ");\n";
					}
				}
				else if (current == 0)
				{
					std::string eName;
					std::string name;

					int token1 = yylex();
					if (token1 != IDENTIFIER)
						return 1;
					eName = yytext;

					int token2 = yylex();
					if (token2 != IDENTIFIER)
						return 1;
					name = yytext;

					if (!needSkip(currentStructName))
					{
						printf("reflect: %s %s\n", eName.c_str(), name.c_str());
						implString += "currentBank->addE(\"" + eName + "\", \"" + name + "\", offsetof(" + currentStructName + ", " + name + "));\n";
					}
				}
			}
				break;
		}
		token = yylex();
	}

	fprintf(fout, EXTRA);
	fprintf(fout, "#include <string>\n");
	fprintf(fout, "namespace tke{\n");

	fprintf(fout, declString.c_str());

	fprintf(fout, "struct ReflectInit{ReflectInit(){\n");
	fprintf(fout, "tke::Enum *currentEnum = nullptr;\n");
	fprintf(fout, "tke::ReflectionBank *currentBank = nullptr;\n");
	fprintf(fout, implString.c_str());
	fprintf(fout, "}};static ReflectInit init;");

	fprintf(fout, "\n}");

	fclose(fout);
	
	return 0;
}