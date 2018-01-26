#include <stdio.h>
#include <string>
#include <filesystem>
#include <regex>
#include <sstream>

#include "../../src/file_utils.h"

#include <windows.h>

// THIS PROJECT IS SEALED SINCE 2018-01-11

int main(int argc, char **argv)
{
	auto up_to_date = true;

	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (argv[i][0] == '-')
			{
				if (strcmp(argv[i], "-recreate") == 0)
					up_to_date = false;
			}
		}
	}

	auto outputFilename = "../src/REFLECT.cpp";
	std::vector<std::string> inputFilenames;

	auto outputFilenameExist = std::experimental::filesystem::exists(outputFilename);
	std::experimental::filesystem::file_time_type output_last_modification_time;
	if (outputFilenameExist)
		output_last_modification_time = std::experimental::filesystem::last_write_time(outputFilename);

	{
		std::experimental::filesystem::recursive_directory_iterator end_it;
		for (std::experimental::filesystem::recursive_directory_iterator it("../src/"); it != end_it; it++)
		{
			if (!std::experimental::filesystem::is_directory(it->status()) && it->path().extension().string() == ".h")
			{
				if (up_to_date && outputFilenameExist)
				{
					if (output_last_modification_time <= std::experimental::filesystem::last_write_time(it->path()))
						up_to_date = false;
				}
				inputFilenames.push_back(it->path().string());
			}
		}
	}

	if (up_to_date)
		return 0;

	int current = -1;
	std::string defineString, enumString, implString, currentStructName, currentEnumName;
	std::vector<std::string> structNames;
	static const auto structExist = [&](const std::string &name){
		for (auto n : structNames)
		{
			if (n == name)
				return true;
		}
		return false;
	};

	for (int i = 0; i < inputFilenames.size(); i++)
	{
		auto file_content = tke::get_file_content(inputFilenames[i]);

		std::stringstream ss(file_content.first.get());
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
					defineString += "tke::ReflectionBank *" + currentStructName + "::b = tke::addReflectionBank(\"" + currentStructName + "\");\n";
					implString += "currentBank = " + currentStructName + "::b;\n";
					if (match[6].matched) // parent
					{
						if (structExist(match[6].str()))
							implString += "currentBank->parents.emplace_back(" + match[6].str() + "::b, TK_DERIVE_OFFSET(" + currentStructName + ", " + match[6].str() + "));\n";
						line = match.suffix();
						while (std::regex_search(line, match, pattern = R"(([\w_]+))"))
						{
							if (structExist(match[1].str()))
								implString += "currentBank->parents.emplace_back(" + match[1].str() + "::b, TK_DERIVE_OFFSET(" + currentStructName + ", " + match[1].str() + "));\n";
							line = match.suffix();
						}
					}

				}
				else
				{
					current = 1;
					currentEnumName = match[4].str();
					printf("current enum: %s\n", currentEnumName.c_str());
					enumString += "currentEnumType = tke::addReflectEnumType(\"" + currentEnumName + "\");\n";
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
					enumString += "currentEnumType->items.emplace_back(\"" + name + "\", (int)" + currentEnumName + "::" + name + ");\n";
				}
				else if (current == 0)
				{
					std::string eName = match[1].str();
					std::string name = match[3].str();

					printf("reflect: %s %s\n", eName.c_str(), name.c_str());
					implString += "currentBank->addE(\"" + eName + "\", \"" + name + "\", offsetof(" + currentStructName + ", " + name + "));\n";
				}
			}
			else if (std::regex_search(line, match, pattern = R"(IMPL\s*\(\s*([\w_\-\.:\(\)\,]+)?\s*\)\s*([\w_:\<\>\*\,]+)\s+([\w_\*\[\]\(\)]+))"))
			{
				defineString += match[2].str() + " " + match[3].str();
				if (match[1].matched)
					defineString += " = " + match[1].str();
				defineString += ";\n";
			}
		}
	}

	std::ofstream outFile(outputFilename);
	outFile << "//This file is auto generated by Project Reflect, please do not edit this file.\n";
	for (auto &fn : inputFilenames)
		outFile << "#include \"" + fn + "\"\n";
	outFile << "namespace tke{\n";
	outFile << "//define:\n";
	outFile << defineString;
	outFile << "struct ReflectInit{ReflectInit(){\n";
	outFile << "tke::EnumType *currentEnumType = nullptr;\n";
	outFile << "tke::ReflectionBank *currentBank = nullptr;\n";
	outFile << "//enum:\n";
	outFile << enumString;
	outFile << "//impl:\n";
	outFile << implString;
	outFile << "}};static ReflectInit _init;";
	outFile << "\n}";
	
	return 0;
}