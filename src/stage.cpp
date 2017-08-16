#include <sstream>
#include <stack>
#include <regex>

#include "define.h"
#include "stage.h"
#include "pipeline.h"

namespace tke
{
	ShaderModule::~ShaderModule()
	{
		device.mtx.lock();
		vkDestroyShaderModule(device.v, v, nullptr);
		device.mtx.unlock();
	}

	static std::vector<ShaderModule*> shaderModules;

	Stage::Stage(Pipeline *_parent)
	{
		parent = _parent;
	}

	static bool _findDefine(const std::vector<std::string> &vec, const std::string &def, bool b)
	{
		if (b)
		{
			for (auto &v : vec)
			{
				if (v == def)
					return true;
			}
			return false;
		}
		else
		{
			for (auto &v : vec)
			{
				if (v == def)
					return false;
			}
			return true;
		}
	}

	static std::string _last_compiled_stage_text;

	void Stage::create()
	{
		std::vector<std::string> defines;
		for (auto &m : parent->shaderMacros)
		{
			if (((int)m.stage & (int)type))
				defines.push_back(m.value);
		}

		// format the shader path, so that they can reuse if they refer the same one
		auto stageFilename = std::experimental::filesystem::canonical(parent->filepath + "/" + filename).string();
		for (auto m : shaderModules)
		{
			if (m->filename == stageFilename)
			{
				if (defines.size() != m->defines.size()) continue;

				bool same = true;
				for (int i = 0; i < defines.size(); i++)
				{
					if (defines[i] != m->defines[i])
					{
						same = false;
						break;
					}
				}

				if (same)
				{
					m->refCount++;
					module = m;
					return;
				}
			}
		}

		module = new ShaderModule;
		module->filename = stageFilename;
		module->defines.insert(module->defines.begin(), defines.begin(), defines.end());
		shaderModules.push_back(module);

		// Warnning:push constants in different stages must be merged, or else they would not reflect properly.

		{
			auto file_path = std::experimental::filesystem::path(stageFilename).parent_path().string();
			tke::OnceFileBuffer file(stageFilename);

			std::stringstream ss(file.data);

			int lineNum = 0;
			std::string stageText = "";
			stageText += "#version 450 core\n"; lineNum++;
			stageText += "#extension GL_ARB_separate_shader_objects : enable\n"; lineNum++;
			stageText += "#extension GL_ARB_shading_language_420pack : enable\n\n"; lineNum++;
			for (auto &m : defines)
			{
				stageText += "#define " + m + "\n";
				lineNum++;
			}
			int fullLineNum = lineNum;

			std::stack<std::pair<bool, bool>> states; // first current accept, second this con accepted
			states.push({ true, false });

			std::vector<std::tuple<int, int, int>> includeFileDatas;

			std::string line;
			while (!ss.eof())
			{
				std::getline(ss, line);

				std::regex pattern;
				std::smatch match;

				if (std::regex_search(line, match, pattern = R"(#(el)?if\s+(\!)?defined\(([\w_]*)\)[\s&]*)"))
				{
					bool isElse = match[1].matched;
					bool ok;
					if ((isElse && !states.top().second) || (!isElse && states.top().first))
					{
						std::vector<std::pair<std::string, bool>> cons;
						if (match[2].matched)
							cons.emplace_back(match[3].str(), false);
						else
							cons.emplace_back(match[3].str(), true);
						std::string str = match.suffix();
						while (std::regex_search(str, match, pattern = R"((\!)?defined\(([\w_]*)\)[\s&]*)"))
						{
							if (match[1].matched)
								cons.emplace_back(match[2].str(), false);
							else
								cons.emplace_back(match[2].str(), true);
							str = match.suffix();
						}

						ok = true;
						for (auto &c : cons)
						{
							if (!_findDefine(defines, c.first, c.second))
							{
								ok = false;
								break;
							}
						}
					}

					if (isElse)
					{
						if (states.top().second)
						{
							states.top().first = false;
						}
						else
						{
							states.top().first = ok;
							states.top().second = ok;
						}
					}
					else
					{
						if (states.top().first)
							states.push({ ok, ok });
						else
							states.push({ false, true });
					}
				}
				else if (std::regex_search(line, match, pattern = R"(#else)"))
				{
					states.top().first = !states.top().first;
					states.top().second = true;
				}
				else if (std::regex_search(line, match, pattern = R"(#endif)"))
				{
					states.pop();
				}
				else if (states.top().first && std::regex_search(line, match, pattern = R"(#define\s+([\w_]*))"))
				{
					defines.push_back(match[1].str());

					stageText += line + "\n";

					fullLineNum += 1;
					lineNum++;
				}
				else if (states.top().first && std::regex_search(line, match, pattern = R"(#include\s+\"([\w\.\\]*)\")"))
				{
					tke::OnceFileBuffer includeFile(file_path + "/" + match[1].str());
					stageText += includeFile.data;
					stageText += "\n";

					auto includeFileLineNum = tke::lineNumber(includeFile.data);
					includeFileDatas.emplace_back(lineNum, fullLineNum, includeFileLineNum);

					fullLineNum += includeFileLineNum;
					lineNum++;
				}
				else if (states.top().first && std::regex_search(line, match, pattern = R"(layout\s*\((\s*set\s*=\s*([0-9]+)\s*,\s*)?\s*binding\s*=\s*((TKE_UBO_BINDING)|([0-9]+))\s*\)\s*uniform\s+((sampler2D)\s+)?([\w_]+)\s*(\[\s*([0-9]+)\s*\])?)"))
				{
					auto set = match[2].matched ? std::stoi(match[2].str()) : 0;
					if (set >= parent->descriptor_set_bindings.size())
						parent->descriptor_set_bindings.resize(set + 1);
					if (set >= module->descriptors.size())
						module->descriptors.resize(set + 1);

					Descriptor d;
					d.name = match[8].str();
					d.binding = -1;
					for (int i = 0; i < 5; i++)
					{
						if (parent->stages[i])
						{
							auto s = parent->stages[i];
							if (s->type != type && s->module && s->module->descriptors.size() > set)
							{
								for (auto &_d : s->module->descriptors[set])
								{
									if (_d.name == d.name)
									{
										d.binding = _d.binding;
										d.type = _d.type;
										d.count = _d.count;
										break;
									}
								}
								if (d.binding != -1)
									break;
							}
						}
					}

					if (d.binding == -1)
					{
						d.binding = parent->descriptor_set_bindings[set];
						d.type = match[7].matched ? DescriptorType::image_n_sampler : DescriptorType::uniform_buffer;
						d.count = match[10].matched ? std::stoi(match[10].str()) : 1;
						module->descriptors[set].push_back(d);
						parent->descriptor_set_bindings[set]++;
					}
					else
					{
						module->descriptors[set].push_back(d);
					}
					line = std::regex_replace(line, pattern = R"(TKE_UBO_BINDING)", std::to_string(d.binding));

					stageText += line + "\n";

					fullLineNum += 1;
					lineNum++;
				}
				else if (states.top().first)
				{
					stageText += line + "\n";

					fullLineNum += 1;
					lineNum++;
				}
			}

			{
				_last_compiled_stage_text = stageText;
				std::ofstream file("temp.glsl");
				file.write(stageText.c_str(), stageText.size());
				file.close();
			}

			tke::exec("cmd", std::string("/C glslangValidator ") + enginePath + "src/my_glslValidator_config.conf -V temp.glsl -S " + tke::StageNameByType(type) + " -q -o temp.spv > output.txt");

			bool error = false;

			std::string output;
			{
				tke::OnceFileBuffer outputFile("output.txt");
				output = outputFile.data;
				output += "\n";

				// analyzing the reflection

				enum ReflectionType
				{
					eNull = -1,
					eUniform = 0,
					eUniformBlock = 1,
					eVertexAttribute = 2
				};

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

				ReflectionType currentReflectionType = eNull;

				int error_count = 0;

				std::stringstream ss(outputFile.data);
				std::string line;
				while (!ss.eof())
				{
					std::getline(ss, line);

					std::regex pattern;
					std::smatch match;
					if (std::regex_search(line, match, pattern = R"(glslangValidator: Error unable to open input file)"))
					{
						error = true;
					}
					else if (std::regex_search(line, match, pattern = R"(ERROR: temp.glsl:([-0-9][-a-zA-Z0-9]*))"))
					{
						error = true;

						auto n = std::stoi(match[1].str());

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

						error_count++;
						output += std::string("The ") + std::to_string(error_count) + "th Error, redirect line num:" + std::to_string(n) + "\n";
					}
					else if (std::regex_search(line, match, pattern = R"(Uniform reflection:)"))
					{
						currentReflectionType = eUniform;
					}
					else if (std::regex_search(line, match, pattern = R"(Uniform block reflection:)"))
					{
						currentReflectionType = eUniformBlock;
					}
					else if (std::regex_search(line, match, pattern = R"(Vertex attribute reflection:)"))
					{
						currentReflectionType = eVertexAttribute;
					}
					else if (std::regex_search(line, match, pattern = R"(([_a-zA-Z][_a-zA-Z0-9.]*)[\[\]0-9]*: offset ([-0-9][-a-zA-Z0-9]*), type ([_a-zA-Z0-9.]*), size ([-0-9][-a-zA-Z0-9]*), index ([-0-9][-a-zA-Z0-9]*), binding ([-0-9][-a-zA-Z0-9]*))"))
					{
						Reflection reflection;
						reflection.reflectionType = currentReflectionType;
						reflection.name = match[1].str();
						reflection.offset = std::stoi(match[2].str());
						reflection.type = match[3].str();
						reflection.size = std::stoi(match[4].str());
						reflection.index = std::stoi(match[5].str());
						reflection.binding = std::stoi(match[6].str());
						reflections.add(reflection);
					}
				}

				for (auto &r : reflections.rs)
				{
					switch (r.reflectionType)
					{
					case eUniform:
						if (r.binding != -1 && r.type == "8b5e") // SAMPLER
						{
							// processed above
						}
						break;
					case eUniformBlock:
						if (r.binding != -1) // UBO
						{
							// processed above
						}
						else // PC
						{
							tke::PushConstantRange p;
							p.offset = 0; // 0 always
							p.size = r.size;
							module->pushConstantRanges.push_back(p);
						}
						break;
					}
				}
			}

			if (!error)
			{
				OnceFileBuffer file("temp.spv");
				VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
				shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shaderModuleCreateInfo.codeSize = file.length;
				shaderModuleCreateInfo.pCode = (uint32_t*)file.data;

				device.mtx.lock();
				auto res = vkCreateShaderModule(device.v, &shaderModuleCreateInfo, nullptr, &module->v);
				assert(res == VK_SUCCESS);
				device.mtx.unlock();

				DeleteFileA("temp.spv");
			}
			else
			{
				assert(false);
				MessageBox(NULL, output.c_str(), stageFilename.c_str(), 0);
				exit(1);
			}

			DeleteFileA("output.txt");
			DeleteFileA("temp.glsl");
		}
	}

	Stage::~Stage()
	{
		module->refCount--;
		if (module->refCount == 0)
		{
			for (auto it = shaderModules.begin(); it != shaderModules.end(); it++)
			{
				if (*it == module)
				{
					shaderModules.erase(it);
					delete module;
					return;
				}
			}
		}
	}
}
