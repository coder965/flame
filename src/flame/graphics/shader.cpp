#include "shader_private.h"
#include "device_private.h"

#include <flame/filesystem.h>
#include <flame/system.h>

#include <spirv_glsl.hpp>

namespace flame
{
	namespace graphics
	{
		static const char additional_lines[] =
			"#version 450 core\n"
			"#extension GL_ARB_separate_shader_objects : enable\n"
			"#extension GL_ARB_shading_language_420pack : enable\n"
			;
		static const auto additional_lines_count = std::count(std::begin(additional_lines), std::end(additional_lines), '\n');

		void Shader::build()
		{
			std::string shader_path("shaders/");
			std::string vk_sdk_path(getenv("VK_SDK_PATH"));
			assert(vk_sdk_path != "");

			std::filesystem::remove("temp.spv"); // glslc cannot write to an existed file

			auto _filename = shader_path + "src/" + filename;
			std::filesystem::path path(_filename);

			auto spv_filename = filename;
			for (auto &d : defines)
				spv_filename += "." + d;
			spv_filename += ".spv";
			spv_filename = shader_path + "bin/" + spv_filename;

			auto spv_up_to_date = std::filesystem::exists(spv_filename) && 
				std::filesystem::last_write_time(spv_filename) > std::filesystem::last_write_time(path);

			if (!spv_up_to_date)
			{
				auto temp_filename = path.parent_path().string() + "/temp." + path.filename().string();
				{
					std::ofstream ofile(temp_filename);
					auto file = get_file_content(_filename);
					ofile.write(additional_lines, sizeof(additional_lines) - 1);
					ofile.write(file.first.get(), file.second);
					ofile.close();
				}
				std::string command_line(" " + temp_filename + " ");
				for (auto &d : defines)
					command_line += "-D" + d + " ";
				command_line += " -flimit-file ";
				command_line += shader_path + "src/shader_compile_config.conf";
				command_line += " -o temp.spv";
				auto output = exec_and_get_output(vk_sdk_path + "/Bin/glslc.exe", command_line);
				std::filesystem::remove(temp_filename);
				if (!std::filesystem::exists("temp.spv"))
				{
					// shader compile error, try to use previous spv file
					printf("\n=====Shader Compile Error=====\n", output.c_str());
					auto p = (char*)output.c_str();
					while (true)
					{
						auto p0 = std::strstr(p, ":");
						if (!p0)
							break;
						auto p1 = std::strstr(p0 + 1, ":");
						if (!p1)
							break;
						*p0 = 0;
						*p1 = 0;
						printf("%s:%d:", p, std::atoi(p0 + 1) - additional_lines_count);
						p = std::strstr(p1 + 1, "\n");
						if (p)
							*p = 0;
						printf("%s\n", p1 + 1);
						if (!p)
							break;
					}
					printf("=============================\n", output.c_str());
				}
				else
				{
					std::filesystem::path spv_dir = std::filesystem::path(spv_filename).parent_path();
					if (!std::filesystem::exists(spv_dir))
						std::filesystem::create_directories(spv_dir);
					std::filesystem::copy_file("temp.spv", spv_filename, std::filesystem::copy_options::overwrite_existing);
				}
			}

			auto spv_file = get_file_content(spv_filename);
			if (!spv_file.first)  // missing spv file!
			{
				if (!_priv->v) // if we are running first time and no previous spv
					assert(0);
				return;
			}

			release();

			VkShaderModuleCreateInfo shader_info;
			shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_info.flags = 0;
			shader_info.pNext = nullptr;
			shader_info.codeSize = spv_file.second;
			shader_info.pCode = (uint32_t*)spv_file.first.get();
			vk_chk_res(vkCreateShaderModule(_priv->d->_priv->device, &shader_info, nullptr, &_priv->v));

			auto res_filename = spv_filename + ".res";
			if (std::filesystem::exists(res_filename) &&
				std::filesystem::last_write_time(res_filename) > std::filesystem::last_write_time(spv_filename))
			{
				std::ifstream res_file(res_filename, std::ios::binary);

				auto _process_resource = [&](ShaderResourceType type) {
					auto count = read<int>(res_file);
					for (auto i = 0; i < count; i++)
					{
						auto set = read<int>(res_file);
						if (set >= _priv->resources.size())
							_priv->resources.resize(set + 1);

						ShaderResource r;
						r.type = type;
						r.name = read_string(res_file);
						r.binding = read<int>(res_file);
						r.count = read<int>(res_file);
						_priv->resources[set].push_back(r);
					}
				};

				_process_resource(ShaderResourceUniformBuffer);
				_process_resource(ShaderResourceTexture);
				_priv->push_constant_size = read<int>(res_file);
			}
			else
			{
				// do reflection
				std::vector<unsigned int> spv_vec(spv_file.second / sizeof(unsigned int));
				memcpy(spv_vec.data(), spv_file.first.get(), spv_file.second);

				spirv_cross::CompilerGLSL glsl(std::move(spv_vec));

				spirv_cross::ShaderResources resources = glsl.get_shader_resources();

				std::ofstream res_file(res_filename, std::ios::binary);

				auto _process_resource = [&](ShaderResourceType type, spirv_cross::Resource &r) {
					auto set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
					if (set >= _priv->resources.size())
						_priv->resources.resize(set + 1);
					write<int>(res_file, set);

					ShaderResource sr;
					sr.type = type;
					sr.name = r.name;
					sr.binding = glsl.get_decoration(r.id, spv::DecorationBinding);
					auto _type = glsl.get_type(r.type_id);
					sr.count = _type.array.size() > 0 ? _type.array[0] : 1;
					_priv->resources[set].emplace_back(sr);

					write_string(res_file, sr.name);
					write<int>(res_file, sr.binding);
					write<int>(res_file, sr.count);
				};

				write<int>(res_file, resources.uniform_buffers.size());
				for (auto &r : resources.uniform_buffers)
					_process_resource(ShaderResourceUniformBuffer, r);
				write<int>(res_file, resources.sampled_images.size());
				for (auto &r : resources.sampled_images)
					_process_resource(ShaderResourceTexture, r);

				for (auto &r : resources.push_constant_buffers)
					_priv->push_constant_size = glsl.get_declared_struct_size(glsl.get_type(r.type_id));
				write<int>(res_file, _priv->push_constant_size);
			}

			//for (auto p : referencing_pipelines)
			//	p->create();
		}

		void Shader::release()
		{
			//filename.clear();
			//defines.clear();
			//type = ShaderTypeNone;
			//_priv->resources.clear();
			//_priv->push_constant_size = 0;

			if (_priv->v)
			{
				vkDestroyShaderModule(_priv->d->_priv->device, _priv->v, nullptr);
				_priv->v = 0;
			}
		}

		Shader *create_shader(Device *d, const std::string &filename, const std::vector<std::string> &defines)
		{
			auto s = new Shader;
			s->filename = filename;
			s->defines = defines;

			auto ext = std::filesystem::path(filename).extension().string();
			if (ext == ".vert")
				s->type = ShaderTypeVert;
			else if (ext == ".tesc")
				s->type = ShaderTypeTesc;
			else if (ext == ".tese")
				s->type = ShaderTypeTese;
			else if (ext == ".geom")
				s->type = ShaderTypeGeom;
			else if (ext == ".frag")
				s->type = ShaderTypeFrag;

			s->_priv = new ShaderPrivate;
			s->_priv->push_constant_size = 0;
			s->_priv->d = d;
			s->_priv->v = 0;
			s->build();

			return s;
		}

		void destroy_shader(Device *d, Shader *s)
		{
			assert(d == s->_priv->d);

			s->release();

			delete s->_priv;
			delete s;
		}
	}
}
