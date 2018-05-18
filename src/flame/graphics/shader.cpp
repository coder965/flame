//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "shader_private.h"
#include "device_private.h"

#include <flame/filesystem.h>
#include <flame/system.h>
#include <flame/shader/shader.h>

namespace flame
{
	namespace graphics
	{
		static std::string shader_path("shaders/");

		void Shader::build()
		{
			std::filesystem::remove("temp.spv"); // glslc cannot write to an existed file. well we did delete it when we finish compiling, but there can be one somehow

			auto glsl_filename = shader_path + "src/" + filename.data;

			std::string spv_filename(filename.data);
			for (auto &d : defines)
				spv_filename += std::string(".") + d.data;
			spv_filename += ".spv";
			spv_filename = shader_path + "bin/" + spv_filename;

			if (!std::filesystem::exists(spv_filename) ||
				std::filesystem::last_write_time(spv_filename) <= std::filesystem::last_write_time(glsl_filename))
			{
				compile_shader(glsl_filename.c_str(), defines.size(), defines.data(),
					(shader_path + "src/shader_compile_config.conf").c_str(), spv_filename.c_str(), [](const char *filename, int line, const char *what){
					// shader compile error, try to use previous spv file, we are not going to return here
					if (filename == nullptr && line == -1)
					{
						if (strcmp(what, "##start"))
							printf("\n=====Shader Compile Error=====\n");
						else if (strcmp(what, "##end"))
							printf("=============================\n");
					}
					else
						printf("%s:%d:%s\n", filename, line, what);
				});
			}

			auto spv_file = get_file_content(spv_filename);
			if (!spv_file.first)  // missing spv file!
			{
				if (!_priv->v) // if we are running first time and no previous spv
					assert(0); // we should not having bad shader TODO : maybe we should add a default shader?
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
			if (!std::filesystem::exists(res_filename) ||
				std::filesystem::last_write_time(res_filename) <= std::filesystem::last_write_time(spv_filename))
				produce_shader_resource_file(spv_filename.c_str(), res_filename.c_str());

			std::ifstream res_file(res_filename, std::ios::binary);

			auto _read_resource = [&](ShaderResourceType type) {
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

			_read_resource(ShaderResourceUniformbuffer);
			_read_resource(ShaderResourceStoragebuffer);
			_read_resource(ShaderResourceTexture);
			_read_resource(ShaderResourceStorageTexture);
			_priv->push_constant_size = read<int>(res_file);
		}

		void Shader::release()
		{
			if (_priv->v)
			{
				vkDestroyShaderModule(_priv->d->_priv->device, _priv->v, nullptr);
				_priv->v = 0;
			}
		}

		Shader *create_shader(Device *d, const char *filename)
		{
			auto s = new Shader;
			strcpy(s->filename.data, filename);

			auto ext = std::filesystem::path(filename).extension().string();
			if (ext == ".vert")
				s->type = ShaderVert;
			else if (ext == ".tesc")
				s->type = ShaderTesc;
			else if (ext == ".tese")
				s->type = ShaderTese;
			else if (ext == ".geom")
				s->type = ShaderGeom;
			else if (ext == ".frag")
				s->type = ShaderFrag;
			else if (ext == ".comp")
				s->type = ShaderComp;

			s->_priv = new ShaderPrivate;
			s->_priv->push_constant_size = 0;
			s->_priv->d = d;
			s->_priv->v = 0;

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
