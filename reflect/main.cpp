#include <stdio.h>
#include <string>
#include <experimental\filesystem>
#include <regex>
#include <sstream>

#include "../src/utils.h"

const std::vector<std::string> defaultSkip = {};

#define INPUT "../src/render.h"
#define OUTPUT "../src/reflect.cpp"

#ifndef SKIP
#define SKIP defaultSkip
#endif

bool needSkip(const std::string &name)
{
	for (auto i = 0; i < SKIP.size(); i++)
	{
		if (SKIP[i] == name)
			return true;
	}
	return false;
}

int main(int argc, char **argv)
{
	if (!std::experimental::filesystem::exists(INPUT))
		return 1;

	if (std::experimental::filesystem::exists(OUTPUT))
	{
		auto input_last_modification_time = std::experimental::filesystem::last_write_time(INPUT);
		auto output_last_modification_time = std::experimental::filesystem::last_write_time(OUTPUT);
		if (output_last_modification_time > input_last_modification_time)
			return 0;
	}

	std::string declString;
	std::string implString;

	int current = -1;
	std::string currentStructName;
	std::string currentEnumName;

	{
		tke::OnceFileBuffer file(INPUT);

		std::stringstream ss(file.data);

		std::string line;
		while (!ss.eof())
		{
			std::getline(ss, line);

			std::regex pattern;
			std::smatch match;

			if (std::regex_search(line, match, pattern = R"(REFLECTABLE\s+((struct)|(enum\s+class))\s+([\w_]+))"))
			{
				if (match[2].matched)
				{
					current = 0;
					currentStructName = match[4].str();
					if (!needSkip(currentStructName))
					{
						printf("current struct: %s\n", currentStructName.c_str());
						declString += "tke::ReflectionBank *" + currentStructName + "::b = tke::addReflectionBank(\"" + currentStructName + "\");\n";
						implString += "currentBank = " + currentStructName + "::b;\n";
					}
				}
				else
				{
					current = 1;
					currentEnumName = match[4].str();
					if (!needSkip(currentEnumName))
					{
						printf("current enum: %s\n", currentEnumName.c_str());
						implString += "currentEnum = tke::addReflectEnum(\"" + currentEnumName + "\");\n";
					}
				}
			}
			else if (std::regex_search(line, match, pattern = R"(REFLv\s+([\w_:]+)\s+([\w_]+))"))
			{
				if (current != 0)
					return 1;

				std::string tName = match[1].str();
				std::string vName = match[2].str();

				if (!needSkip(currentStructName))
				{
					printf("reflect: %s %s\n", tName.c_str(), vName.c_str());
					implString += "currentBank->addV<" + tName + ">(\"" + vName + "\", offsetof(" + currentStructName + ", " + vName + "));\n";
				}
			}
			else if (std::regex_search(line, match, pattern = R"(REFLe\s+([\w_:]+)(\s+([\w_]+))?)"))
			{
				if (current != 0 && current != 1)
					return 1;

				if (current == 1)
				{
					std::string name = match[1].str();

					if (!needSkip(currentEnumName))
					{
						printf("reflect: %s\n", name.c_str());
						implString += "currentEnum->items.emplace_back(\"" + name + "\", (int)" + currentEnumName + "::" + name + ");\n";
					}
				}
				else if (current == 0)
				{
					std::string eName = match[1].str();
					std::string name = match[3].str();

					if (!needSkip(currentStructName))
					{
						printf("reflect: %s %s\n", eName.c_str(), name.c_str());
						implString += "currentBank->addE(\"" + eName + "\", \"" + name + "\", offsetof(" + currentStructName + ", " + name + "));\n";
					}
				}
			}
		}
	}

	auto fout = fopen(OUTPUT, "wb");
	fprintf(fout, "#include \"utils.h\"\n");
	fprintf(fout, "#include \"render.h\"\n");
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