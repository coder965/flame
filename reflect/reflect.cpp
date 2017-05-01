#include <stdio.h>
#include <string.h>
#include <string>

#include "def.h"

//#include "extra0.h"
#include "extra1.h"

#ifndef EXTRA
#define EXTRA "\n"
#endif

extern "C" {
	extern FILE *yyin;
	extern int yylex();
	extern int yylineno;
	extern char *yytext;
}

int current = -1;
char currentStructName[100];
char currentEnumName[100];

int main(int argc, char **argv)
{
	if (argc < 3)
		return 1;

	yyin = fopen(argv[1], "rb");
	if (!yyin)
		return 1;

	FILE *fout = fopen(argv[2], "wb");
	if (!fout) return 1;

	std::string declString;
	std::string implString;

	char line[200];

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
					strcpy(currentStructName, yytext);
					printf("current struct: %s\n", yytext);
					sprintf(line, "tke::ReflectionBank *%s::b = tke::addReflectionBank(\"%s\");\n", currentStructName, currentStructName);
					declString += line;
					sprintf(line, "currentBank = %s::b;\n", currentStructName);
					implString += line;
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
					strcpy(currentEnumName, yytext);
					printf("current enum: %s\n", yytext);
					sprintf(line, "currentEnum = tke::addReflectEnum(\"%s\");\n", currentEnumName);
					implString += line;
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

				char typeName[100];
				char valueName[100];

				int token1 = yylex();
				if (token1 != IDENTIFIER)
					return 1;
				strcpy(typeName, yytext);
				int token2 = yylex();
				if (token2 != IDENTIFIER)
					return 1;
				strcpy(valueName, yytext);

				printf("reflect: %s %s\n", typeName, valueName);
				sprintf(line, "currentBank->addV<%s>(\"%s\", offsetof(%s, %s));\n", typeName, valueName, currentStructName, valueName);
				implString += line;
			}
				break;
			case REFLe:
			{
				if (current == -1)
					return 1;

				if (current == 1)
				{
					char name[100];

					int token1 = yylex();
					if (token1 != IDENTIFIER)
						return 1;
					strcpy(name, yytext);

					printf("reflect: %s\n", name);
					sprintf(line, "currentEnum->items.emplace_back(\"%s\", (int)%s::%s);\n", name, currentEnumName, name);
					implString += line;
				}
				else if (current == 0)
				{
					char eName[100];
					char name[100];

					int token1 = yylex();
					if (token1 != IDENTIFIER)
						return 1;
					strcpy(eName, yytext);

					int token2 = yylex();
					if (token2 != IDENTIFIER)
						return 1;
					strcpy(name, yytext);

					printf("reflect: %s %s\n", eName, name);
					sprintf(line, "currentBank->addE(\"%s\", \"%s\", offsetof(%s, %s));\n", eName, name, currentStructName, name);
					implString += line;
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