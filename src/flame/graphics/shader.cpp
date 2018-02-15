#include <sstream>
#include <stack>
#include <regex>

#include "../../SPIRV-Cross/spirv_glsl.hpp"
#include "../file_utils.h"
#include "../global.h"
#include "../graphics/descriptor.h"
#include "shader.h"

namespace tke
{
	Shader::Shader(const std::string &_filename, const std::vector<std::string> &_defines) :
		filename(_filename),
		defines(_defines),
		v(0)
	{
		auto ext = std::fs::path(filename).extension().string();
		if (ext == ".vert")
			stage = VK_SHADER_STAGE_VERTEX_BIT;
		else if (ext == ".tesc")
			stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		else if (ext == ".tese")
			stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		else if (ext == ".geom")
			stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		else if (ext == ".frag")
			stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		create();
	}

	Shader::~Shader()
	{
		vkDestroyShaderModule(vk_device.v, v, nullptr);
	}

	void Shader::create()
	{
		std::fs::path path(filename);

		auto shader_file_timestamp = std::fs::last_write_time(path);

		auto spv_filename = filename;
		for (auto &d : defines)
			spv_filename += "." + d;
		spv_filename += ".spv";

		bool spv_up_to_date = false;
		if (std::fs::exists(spv_filename))
		{
			if (std::fs::last_write_time(spv_filename) > shader_file_timestamp)
				spv_up_to_date = true;
			else
				std::fs::remove(spv_filename); // glslc cannot write to an existed file
		}

		if (!spv_up_to_date)
		{
			std::string cmd_str("glslc ");
			cmd_str += filename + " ";
			for (auto &d : defines)
				cmd_str += "-D" + d + " ";
			cmd_str += " -flimit-file ";
			cmd_str += engine_path + "shader/my_config.conf";
			cmd_str += " -o " + spv_filename;
			system(cmd_str.c_str());
			if (!std::experimental::filesystem::exists(spv_filename))
			{
				assert(0); // shader compile error
				return;
			}
		}

		auto spv_file = get_file_content(spv_filename);

		{
			VkShaderModuleCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.codeSize = spv_file.second;
			info.pCode = (uint32_t*)spv_file.first.get();

			if (v)
				vkDestroyShaderModule(vk_device.v, v, nullptr);
			auto res = vkCreateShaderModule(vk_device.v, &info, nullptr, &v);
			assert(res == VK_SUCCESS);
		}

		auto res_filename = spv_filename + ".res";
		if (std::fs::exists(res_filename) &&
			std::fs::last_write_time(res_filename) > std::fs::last_write_time(spv_filename))
		{
			std::ifstream resFile(res_filename, std::ios::binary);

			auto ubo_count = read_int(resFile);
			for (int i = 0; i < ubo_count; i++)
			{
				auto set = read_int(resFile);
				if (set >= descriptor_sets.size())
					descriptor_sets.resize(set + 1);

				auto d = new Descriptor;
				d->type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				d->name = read_string(resFile);
				d->binding = read_int(resFile);
				d->count = read_int(resFile);
				descriptor_sets[set].emplace_back(d);
			}
			auto image_count = read_int(resFile);
			for (int i = 0; i < image_count; i++)
			{
				auto set = read_int(resFile);
				if (set >= descriptor_sets.size())
					descriptor_sets.resize(set + 1);

				auto d = new Descriptor;
				d->type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				d->name = read_string(resFile);
				d->binding = read_int(resFile);
				d->count = read_int(resFile);
				descriptor_sets[set].emplace_back(d);
			}
			push_constant_size = read_int(resFile);
		}
		else
		{
			// do reflection
			std::vector<unsigned int> spv_vec(spv_file.second / sizeof(unsigned int));
			memcpy(spv_vec.data(), spv_file.first.get(), spv_file.second);

			spirv_cross::CompilerGLSL glsl(std::move(spv_vec));

			spirv_cross::ShaderResources resources = glsl.get_shader_resources();

			std::ofstream res_file(res_filename, std::ios::binary);

			auto _process_descriptor_resource = [&](VkDescriptorType desc_type, spirv_cross::Resource &r) {
				auto set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				if (set >= descriptor_sets.size())
					descriptor_sets.resize(set + 1);
				write_int(res_file, set);

				auto d = new Descriptor;
				d->type = desc_type;
				d->name = r.name;
				d->binding = glsl.get_decoration(r.id, spv::DecorationBinding);
				auto type = glsl.get_type(r.type_id);
				d->count = type.array.size() > 0 ? type.array[0] : 1;
				descriptor_sets[set].emplace_back(d);

				write_string(res_file, d->name);
				write_int(res_file, d->binding);
				write_int(res_file, d->count);
			};

			write_int(res_file, resources.uniform_buffers.size());
			for (auto &r : resources.uniform_buffers)
				_process_descriptor_resource(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, r);
			write_int(res_file, resources.sampled_images.size());
			for (auto &r : resources.sampled_images)
				_process_descriptor_resource(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, r);

			for (auto &r : resources.push_constant_buffers)
				push_constant_size = glsl.get_declared_struct_size(glsl.get_type(r.type_id));
			write_int(res_file, push_constant_size);
		}
	}

	static std::vector<std::weak_ptr<Shader>> _shaders;

	std::shared_ptr<Shader> get_or_create_shader(const std::string &filename, const std::vector<std::string> &defines, Pipeline *pipeline)
	{
		for (auto it = _shaders.begin(); it != _shaders.end(); )
		{
			auto s = it->lock();
			if (s)
			{
				if (s->filename == filename && s->defines == defines)
				{
					if (pipeline)
						s->pipelines_use_this.push_back(pipeline);
					return s;
				}
				it++;
			}
			else
				it = _shaders.erase(it);
		}
		auto s = std::make_shared<Shader>(filename, defines);
		if (pipeline)
			s->pipelines_use_this.push_back(pipeline);
		_shaders.push_back(s);
		return s;
	}
}
