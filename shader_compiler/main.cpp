#include <vector>
#include <tuple>
#include <string>
#include <sstream>
#include <fstream>
#include <regex>
#include <experimental\filesystem>
#include <iomanip>

#include "../src/utils.h"
#include "../src/render.h"

#include "yy_def.h"
extern "C" {
	extern FILE *yyin;
	extern FILE *yyout;
	extern int yylex();
	int yylex_destroy();
	extern char *yytext;
}

int main(int argc, char** argv)
{
	bool show_all = false;

	for (int i = 0; i < argc; i++)
	{
		if (argv[i] == std::string("-show-all"))
			show_all = true;
	}

	int succeeded = 0;
	int failed = 0;
	int up_to_date = 0;

	auto yy_out_file = fopen("yy_out", "wb");

	{
		auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::stringstream ss;
		ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
		printf("%s\nWarnning:push constants in different stages must be merged, or else they would not reflect properly.\n\n", ss.str().c_str());
	}

	auto at = tke::createAttributeTreeFromXML("stages", "stages.xml");
	if (at)
	{
		for (auto c : at->children)
		{
			if (c->name == "stage")
			{
				auto a = c->firstAttribute("filename");
				if (a)
				{
					if (!std::experimental::filesystem::exists(a->second))
					{
						printf("%s Not Found!\n\n", a->second.c_str());
						continue;
					}

					if (std::experimental::filesystem::exists(a->second + ".spv"))
					{
						auto stage_file_last_modification_time = std::experimental::filesystem::last_write_time(a->second);
						auto spv_file_last_modification_time = std::experimental::filesystem::last_write_time(a->second + ".spv");
						if (spv_file_last_modification_time > stage_file_last_modification_time)
						{
							if (show_all) printf("%s Up To Date\n\n", a->second.c_str());
							up_to_date++;
							continue;
						}
					}

					printf("Now Processing:%s\n", a->second.c_str());

					auto stage_type = tke::StageFlagByExt(std::experimental::filesystem::path(a->second).extension().string());
					auto file_path = std::experimental::filesystem::path(a->second).parent_path().string();
					tke::OnceFileBuffer file(a->second);

					std::stringstream ss(file.data);
					std::string stageText = "";

					std::vector<std::tuple<int, int, int>> includeFileDatas;
					std::string line;
					int lineNum = 0;
					int fullLineNum = 0;
					while (!ss.eof())
					{
						std::getline(ss, line);

						std::regex pat(R"(#include\s+\"([\w\.\\]+)\")");
						std::smatch sm;
						if (std::regex_search(line, sm, pat))
						{
							tke::OnceFileBuffer includeFile(file_path + "/" + sm[1].str());
							stageText += includeFile.data;
							stageText += "\n";

							auto includeFileLineNum = tke::lineNumber(includeFile.data);
							includeFileDatas.emplace_back(lineNum, fullLineNum, includeFileLineNum);

							fullLineNum += includeFileLineNum;
						}
						else
						{
							stageText += line + "\n";

							fullLineNum += 1;
						}

						lineNum++;
					}

					{
						std::ofstream file("temp.glsl");
						file.write(stageText.c_str(), stageText.size());
						file.close();
					}

					std::experimental::filesystem::path spv(a->second + ".spv");

					tke::exec("cmd", "/C glslangValidator my_glslValidator_config.conf -V temp.glsl -S " + tke::StageNameByType(stage_type) + " -q -o " + spv.string() + " > output.txt");

					std::string output;

					{
						tke::OnceFileBuffer outputFile("output.txt");
						output = outputFile.data;
						output += "\n";
					}

					bool error = false;

					{
						// analyzing the reflection

						enum ReflectionType
						{
							eNull = -1,
							eUniform = 0,
							eUniformBlock = 1,
							eVertexAttribute = 2
						};

						ReflectionType currentReflectionType = eNull;

						struct Reflection
						{
							int COUNT = 1; // special for UBO

							ReflectionType reflectionType;
							std::string name;
							int offset;
							std::string type;
							int size;
							int index;
							int binding;
						};
						struct ReflectionManager
						{
							std::vector<Reflection> rs;
							void add(Reflection &_r)
							{
								if (_r.reflectionType == eUniformBlock && _r.binding != -1)
								{
									for (auto &r : rs)
									{
										if (r.binding == _r.binding)
										{
											r.COUNT++;
											return;
										}
									}
								}
								rs.push_back(_r);
							}
						};
						ReflectionManager reflections;
						Reflection currentReflection;

						yylex_destroy();
						yyout = yy_out_file;

						yyin = fopen("output.txt", "rb");
						int token = yylex();
						std::string last_string;
						while (token)
						{
							switch (token)
							{
							case YY_ERROR:
							{
								error = true;
								token = yylex();
								if (token == YY_VALUE)
								{
									auto n = std::stoi(yytext);

									for (auto it = includeFileDatas.rbegin(); it != includeFileDatas.rend(); it++)
									{
										if (n > std::get<1>(*it) + std::get<2>(*it))
										{
											n = n - std::get<1>(*it) + std::get<0>(*it) - std::get<2>(*it) - 1;
											break;
										}
										else if (n > std::get<1>(*it))
										{
											n = std::get<0>(*it);
											break;
										}
									}

									output += std::string("Error, line num:") + std::to_string(n) + "\n";
								}
							}
								break;
							case YY_COLON:
								if (currentReflectionType != eNull)
								{
									if (currentReflection.name != "") reflections.add(currentReflection);
									currentReflection.reflectionType = currentReflectionType;
									currentReflection.name = last_string;
									last_string = "";
								}
								break;
							case YY_IDENTIFIER:
							{
								std::string string(yytext);
								last_string = string;
							}
								break;
							case YY_VALUE:
							{
								std::string string(yytext);
								if (currentReflectionType != eNull)
								{
									if (last_string == "offset")
										currentReflection.offset = std::stoi(string);
									else if (last_string == "type")
										currentReflection.type = string;
									else if (last_string == "size")
										currentReflection.size = std::stoi(string);
									else if (last_string == "index")
										currentReflection.index = std::stoi(string);
									else if (last_string == "binding")
										currentReflection.binding = std::stoi(string);
								}
							}
								break;
							case YY_UR_MK:
								currentReflectionType = eUniform;
								break;
							case YY_UBR_MK:
								currentReflectionType = eUniformBlock;
								break;
							case YY_VAR_MK:
								currentReflectionType = eVertexAttribute;
								break;
							}
							token = yylex();
						}
						if (currentReflection.name != "") reflections.add(currentReflection);
						fclose(yyin);
						yyin = NULL;

						std::vector<tke::Descriptor> descriptors;
						std::vector<tke::PushConstantRange> pushConstantRanges;

						for (auto &r : reflections.rs)
						{
							switch (r.reflectionType)
							{
							case eUniform:
								if (r.binding != -1 && r.type == "8b5e") // SAMPLER
								{
									tke::Descriptor d;
									d.type = tke::DescriptorType::image_n_sampler;
									d.name = r.name;
									d.binding = r.binding;
									d.count = r.size;
									descriptors.push_back(d);
								}
								break;
							case eUniformBlock:
								if (r.binding != -1) // UBO
								{
									tke::Descriptor d;
									d.type = tke::DescriptorType::uniform_buffer;
									d.name = r.name;
									d.binding = r.binding;
									d.count = r.COUNT;
									descriptors.push_back(d);
								}
								else // PC
								{
									tke::PushConstantRange p;
									p.offset = 0; // 0 always
									p.size = r.size;
									pushConstantRanges.push_back(p);
								}
								break;
							}
						}

						if (!error)
						{
							tke::AttributeTree at("stage");

							for (auto &d : descriptors)
							{
								auto n = new tke::AttributeTreeNode("descriptor");
								n->addAttributes(&d, d.b);
								at.children.push_back(n);
							}
							for (auto &p : pushConstantRanges)
							{
								auto n = new tke::AttributeTreeNode("push_constant");
								n->addAttributes(&p, p.b);
								at.children.push_back(n);
							}

							at.saveXML(a->second + ".xml");

							failed++;
						}
						else
						{
							succeeded++;
						}
					}

					DeleteFileA("output.txt");
					DeleteFileA("temp.glsl");

					printf("%s\n", output.c_str());
				}
			}
		}

		printf("succeeded:%d, failed:%d, up-to-date:%d\n", succeeded, failed, up_to_date);
	}
	else
	{
		printf("stages.xml : Not Found!\n");
	}

	fclose(yy_out_file);

	return 0;
}
