#include <sstream>
#include <stack>
#include <regex>

#include "shader.h"
#include "../core.h"
#include "../../SPIRV-Cross/spirv_glsl.hpp"

namespace tke
{
	Shader::Shader(const std::string &_filename, const std::vector<std::string> &_defines)
	{
		std::experimental::filesystem::path path(_filename);
		// format the shader path, so that they can reuse if they refer the same one
		filename = std::experimental::filesystem::canonical(path).string();
		defines = _defines;

		{
			auto ext = path.extension().string();
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
		}

		auto shaderFileLastModificationTime = std::experimental::filesystem::last_write_time(path);

		auto spvFilename = filename;
		for (auto &d : defines)
			spvFilename += "." + d;
		spvFilename += ".spv";

		bool spvUpToDate = false;
		if (std::experimental::filesystem::exists(spvFilename))
		{
			if (std::experimental::filesystem::last_write_time(spvFilename) > shaderFileLastModificationTime)
				spvUpToDate = true;
			else
				std::experimental::filesystem::remove(spvFilename);
		}

		if (!spvUpToDate)
		{
			std::string cmd_str("glslc ");
			cmd_str += filename + " ";
			for (auto &d : defines)
				cmd_str += "-D" + d + " ";
			cmd_str += " -flimit-file ";
			cmd_str += enginePath + "shader/my_config.conf";
			cmd_str += " -fauto-bind-uniforms ";
			cmd_str += " -o " + spvFilename;
			system(cmd_str.c_str());
			if (!std::experimental::filesystem::exists(spvFilename))
			{
				assert(0); // shader compile error
				return;
			}
		}

		{
			OnceFileBuffer spvFile(spvFilename);
			VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
			shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderModuleCreateInfo.codeSize = spvFile.length;
			shaderModuleCreateInfo.pCode = (uint32_t*)spvFile.data;

			device.mtx.lock();
			auto res = vkCreateShaderModule(device.v, &shaderModuleCreateInfo, nullptr, &vkModule);
			assert(res == VK_SUCCESS);
			device.mtx.unlock();
		}

		// do reflection
		{
			std::ifstream spv_file(spvFilename, std::ios::binary);
			auto size = tke::file_length(spv_file);
			std::vector<unsigned int> spv_data(size / sizeof(unsigned int));
			spv_file.read((char*)spv_data.data(), size);

			spirv_cross::CompilerGLSL glsl(std::move(spv_data));

			spirv_cross::ShaderResources resources = glsl.get_shader_resources();

			for (auto &r : resources.uniform_buffers)
			{
				auto set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				if (set >= descriptors.size())
					descriptors.resize(set + 1);

				Descriptor d;
				d.name = r.name;
				d.binding = glsl.get_decoration(r.id, spv::DecorationBinding);
				d.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				d.count = glsl.get_type(r.type_id).array[0];
				descriptors[set].push_back(d);
			}
			for (auto &r : resources.sampled_images)
			{
				auto set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				if (set >= descriptors.size())
					descriptors.resize(set + 1);

				Descriptor d;
				d.name = r.name;
				d.binding = glsl.get_decoration(r.id, spv::DecorationBinding);
				d.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				d.count = glsl.get_type(r.type_id).array[0];
				descriptors[set].push_back(d);
			}
			for (auto &r : resources.push_constant_buffers)
			{
				tke::PushConstantRange p;
				p.offset = 0; // 0 always
				p.size = glsl.get_declared_struct_size(glsl.get_type(r.type_id));
				pushConstantRanges.push_back(p);
			}
		}
	}

	Shader::~Shader()
	{
		device.mtx.lock();
		vkDestroyShaderModule(device.v, vkModule, nullptr);
		device.mtx.unlock();
	}
}
