#include <stdio.h>
#include <string>
#include <filesystem>
#include <regex>
#include <sstream>

#include "../src/utils.h"

#include "windows.h"

std::vector<std::string> structNames;

int main(int argc, char **argv)
{
	auto outputFilename = "../src/reflect.cpp";

	char *inputFilenames[] = {
		"../src/transformer.h",
		"../src/controller.h",
		"../src/model.h",
		"../src/object.h",
		"../src/terrain.h",
		"../src/water.h",
		"../src/scene.h",
		"../src/descriptor.h",
		"../src/push_constant.h",
		"../src/stage.h",
		"../src/sampler.h",
		"../src/pipeline.h",
	};

	if (std::experimental::filesystem::exists(outputFilename))
	{
		auto output_last_modification_time = std::experimental::filesystem::last_write_time(outputFilename);
		auto up_to_date = true;
		for (int i = 0; i < TK_ARRAYSIZE(inputFilenames); i++)
		{
			if (output_last_modification_time <= std::experimental::filesystem::last_write_time(inputFilenames[i]))
			{
				up_to_date = false;
				break;
			}
		}
		if (up_to_date)
			return 0;
	}

	int current = -1;
	std::string declString, implString, currentStructName, currentEnumName;
	auto structExist = [](const std::string &name){
		for (auto n : structNames)
		{
			if (n == name)
				return true;
		}
		return false;
	};

	for (int i = 0; i < TK_ARRAYSIZE(inputFilenames); i++)
	{
		tke::OnceFileBuffer file(inputFilenames[i]);

		std::stringstream ss(file.data);
		std::string line;
		while (!ss.eof())
		{
			std::getline(ss, line);

			std::regex pattern;
			std::smatch match;

			if (std::regex_search(line, match, pattern = R"(REFLECTABLE\s+((struct)|(enum\s+class))\s+([\w_]+)(\s*:\s*([\w_]+))?)"))
			{
				if (match[2].matched)
				{
					current = 0;
					currentStructName = match[4].str();
					structNames.push_back(currentStructName);
					printf("current struct: %s\n", currentStructName.c_str());
					declString += "tke::ReflectionBank *" + currentStructName + "::b = tke::addReflectionBank(\"" + currentStructName + "\");\n";
					implString += "currentBank = " + currentStructName + "::b;\n";
					if (match[6].matched) // parent
					{
						if (structExist(match[6].str()))
							implString += "currentBank->parents.emplace_back(" + match[6].str() + "::b, TK_STRUCT_OFFSET(" + currentStructName + ", " + match[6].str() + "));\n";
						line = match.suffix();
						while (std::regex_search(line, match, pattern = R"(([\w_]+))"))
						{
							if (structExist(match[1].str()))
								implString += "currentBank->parents.emplace_back(" + match[1].str() + "::b, TK_STRUCT_OFFSET(" + currentStructName + ", " + match[1].str() + "));\n";
							line = match.suffix();
						}
					}

				}
				else
				{
					current = 1;
					currentEnumName = match[4].str();
					printf("current enum: %s\n", currentEnumName.c_str());
					implString += "currentEnum = tke::addReflectEnum(\"" + currentEnumName + "\");\n";
				}
			}
			else if (std::regex_search(line, match, pattern = R"(REFLv\s+([\w_:]+)\s+([\w_]+))"))
			{
				if (current != 0)
					return 1;

				std::string tName = match[1].str();
				std::string vName = match[2].str();

				printf("reflect: %s %s\n", tName.c_str(), vName.c_str());
				implString += "currentBank->addV<" + tName + ">(\"" + vName + "\", offsetof(" + currentStructName + ", " + vName + "));\n";
			}
			else if (std::regex_search(line, match, pattern = R"(REFLe\s+([\w_:]+)(\s+([\w_]+))?)"))
			{
				if (current != 0 && current != 1)
					return 1;

				if (current == 1)
				{
					std::string name = match[1].str();

					printf("reflect: %s\n", name.c_str());
					implString += "currentEnum->items.emplace_back(\"" + name + "\", (int)" + currentEnumName + "::" + name + ");\n";
				}
				else if (current == 0)
				{
					std::string eName = match[1].str();
					std::string name = match[3].str();

					printf("reflect: %s %s\n", eName.c_str(), name.c_str());
					implString += "currentBank->addE(\"" + eName + "\", \"" + name + "\", offsetof(" + currentStructName + ", " + name + "));\n";
				}
			}
		}
	}

	auto fout = fopen(outputFilename, "wb");
	fprintf(fout, "#include \"utils.h\"\n");
	fprintf(fout, "#include \"transformer.h\"\n");
	fprintf(fout, "#include \"controller.h\"\n");
	fprintf(fout, "#include \"model.h\"\n");
	fprintf(fout, "#include \"object.h\"\n");
	fprintf(fout, "#include \"terrain.h\"\n");
	fprintf(fout, "#include \"water.h\"\n");
	fprintf(fout, "#include \"scene.h\"\n");
	fprintf(fout, "#include \"descriptor.h\"\n");
	fprintf(fout, "#include \"push_constant.h\"\n");
	fprintf(fout, "#include \"stage.h\"\n");
	fprintf(fout, "#include \"sampler.h\"\n");
	fprintf(fout, "#include \"pipeline.h\"\n");
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