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
			cmd_str += " -o " + spvFilename;
			system(cmd_str.c_str());
			if (!std::experimental::filesystem::exists(spvFilename))
			{
				assert(0); // shader compile error
				return;
			}
		}

		OnceFileBuffer spvFile(spvFilename);

		{
			VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
			shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderModuleCreateInfo.codeSize = spvFile.length;
			shaderModuleCreateInfo.pCode = (uint32_t*)spvFile.data;

			device.mtx.lock();
			auto res = vkCreateShaderModule(device.v, &shaderModuleCreateInfo, nullptr, &vkModule);
			assert(res == VK_SUCCESS);
			device.mtx.unlock();
		}

		auto resFilename = spvFilename + ".res";
		if (std::experimental::filesystem::exists(resFilename) && 
			std::experimental::filesystem::last_write_time(resFilename) > std::experimental::filesystem::last_write_time(spvFilename))
		{
			std::ifstream resFile(resFilename, std::ios::binary);

			int uboCount;
			resFile & uboCount;
			for (int i = 0; i < uboCount; i++)
			{
				int set;
				resFile & set;
				if (set >= descriptors.size())
					descriptors.resize(set + 1);

				Descriptor d;
				d.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				resFile > d.name;
				resFile & d.binding;
				resFile & d.count;
				descriptors[set].push_back(d);
			}
			int imageCount;
			resFile & imageCount;
			for (int i = 0; i < imageCount; i++)
			{
				int set;
				resFile & set;
				if (set >= descriptors.size())
					descriptors.resize(set + 1);

				Descriptor d;
				d.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				resFile > d.name;
				resFile & d.binding;
				resFile & d.count;
				descriptors[set].push_back(d);
			}
			int pcSize;
			resFile & pcSize;
			if (pcSize > 0)
			{
				PushConstantRange p;
				p.offset = 0; // 0 always
				p.size = pcSize;
				pushConstantRanges.push_back(p);
			}
		}
		else
		{
			// do reflection
			std::vector<unsigned int> spv_vec(spvFile.length / sizeof(unsigned int));
			memcpy(spv_vec.data(), spvFile.data, spvFile.length);

			spirv_cross::CompilerGLSL glsl(std::move(spv_vec));

			spirv_cross::ShaderResources resources = glsl.get_shader_resources();

			std::ofstream resFile(resFilename, std::ios::binary);

			int uboCount = resources.uniform_buffers.size();
			resFile & uboCount;
			for (auto &r : resources.uniform_buffers)
			{
				int set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				if (set >= descriptors.size())
					descriptors.resize(set + 1);
				resFile & set;

				Descriptor d;
				d.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				d.name = r.name;
				d.binding = glsl.get_decoration(r.id, spv::DecorationBinding);
				auto type = glsl.get_type(r.type_id);
				d.count = type.array.size() > 0 ? type.array[0] : 1;
				descriptors[set].push_back(d);

				resFile < d.name;
				resFile & d.binding;
				resFile & d.count;
			}
			int imageCount = resources.sampled_images.size();
			resFile & imageCount;
			for (auto &r : resources.sampled_images)
			{
				int set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				if (set >= descriptors.size())
					descriptors.resize(set + 1);
				resFile & set;

				Descriptor d;
				d.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				d.name = r.name;
				d.binding = glsl.get_decoration(r.id, spv::DecorationBinding);
				auto type = glsl.get_type(r.type_id);
				d.count = type.array.size() > 0 ? type.array[0] : 1;
				descriptors[set].push_back(d);

				resFile < d.name;
				resFile & d.binding;
				resFile & d.count;
			}

			int pcSize = 0;
			for (auto &r : resources.push_constant_buffers)
			{
				PushConstantRange p;
				p.offset = 0; // 0 always
				p.size = glsl.get_declared_struct_size(glsl.get_type(r.type_id));
				pushConstantRanges.push_back(p);

				pcSize = p.size;
			}
			resFile & pcSize;
		}
	}

	Shader::~Shader()
	{
		device.mtx.lock();
		vkDestroyShaderModule(device.v, vkModule, nullptr);
		device.mtx.unlock();
	}
}
