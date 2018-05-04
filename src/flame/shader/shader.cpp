#include "shader.h"

#include <flame/filesystem.h>
#include <flame/system.h>

#include <spirv_glsl.hpp>
#include <assert.h>

namespace flame
{
	static const char additional_lines_graphics[] =
		"#version 450 core\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"; // Allows the setting of Uniform Buffer Object and sampler binding points directly from GLSL

	static const char additional_lines_compute[] =
		"#version 450 core\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"; // Allows the setting of Uniform Buffer Object and sampler binding points directly from GLSL

	void compile_shader(const char *glsl_file_in, int shader_define_count,
		ShortString *shader_defines, const char *config_file, const char *spv_file_out, void(*compile_output_callback)(const char *filename, int line, const char *what))
	{
		std::string vk_sdk_path(getenv("VK_SDK_PATH"));
		assert(vk_sdk_path != "");

		std::filesystem::path glsl_path(glsl_file_in);

		const char *additional_lines;
		if (glsl_path.extension().string() == ".comp")
			additional_lines = additional_lines_compute;
		else
			additional_lines = additional_lines_graphics;
		auto additional_lines_len = strlen(additional_lines);
		auto temp_filename = glsl_path.parent_path().string() + "/temp." + glsl_path.filename().string();
		{
			std::ofstream ofile(temp_filename);
			auto file = get_file_content(glsl_file_in);
			ofile.write(additional_lines, additional_lines_len);
			ofile.write(file.first.get(), file.second);
			ofile.close();
		}
		std::string command_line(" " + temp_filename + " ");
		for (auto i = 0; i < shader_define_count; i++)
			command_line += "-D" + std::string(shader_defines[i].data) + " ";
		if (config_file)
		{
			command_line += " -flimit-file ";
			command_line += config_file;
		}
		command_line += " -o temp.spv";
		LongString output;
		exec((vk_sdk_path + "/Bin/glslc.exe").c_str(), command_line.c_str(), &output);
		std::filesystem::remove(temp_filename);
		if (!std::filesystem::exists("temp.spv"))
		{
			auto additional_lines_count = std::count(additional_lines, additional_lines + additional_lines_len, '\n');
			compile_output_callback(nullptr, -1, "##start"); // this tag means this is the start
			auto p = (char*)output.data;
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
				auto filename = p;
				auto line = std::atoi(p0 + 1) - additional_lines_count;
				p = std::strstr(p1 + 1, "\n");
				if (p)
					*p = 0;
				auto what = p1 + 1;
				compile_output_callback(filename, line, what);
				if (!p)
					break;
			}
			compile_output_callback(nullptr, -1, "##end"); // this tag means this is the end
		}
		else
		{
			std::filesystem::path spv_path(spv_file_out);
			std::filesystem::path spv_dir = spv_path.parent_path();
			if (!std::filesystem::exists(spv_dir))
				std::filesystem::create_directories(spv_dir);
			std::filesystem::copy_file("temp.spv", spv_path, std::filesystem::copy_options::overwrite_existing);
			std::filesystem::remove("temp.spv");
		}
	}

	void produce_shader_resource_file(const char *spv_file_in, const char *res_file_out)
	{
		auto spv_file = get_file_content(spv_file_in);

		std::vector<unsigned int> spv_vec(spv_file.second / sizeof(unsigned int));
		memcpy(spv_vec.data(), spv_file.first.get(), spv_file.second);

		spirv_cross::CompilerGLSL glsl(std::move(spv_vec));

		spirv_cross::ShaderResources resources = glsl.get_shader_resources();

		std::ofstream res_file(res_file_out, std::ios::binary);

		auto _write_resource = [&](spirv_cross::Resource &r) {
			write<int>(res_file, (int)glsl.get_decoration(r.id, spv::DecorationDescriptorSet));

			auto _type = glsl.get_type(r.type_id);
			int count = _type.array.size() > 0 ? _type.array[0] : 1;

			write_string(res_file, r.name);
			write<int>(res_file, (int)glsl.get_decoration(r.id, spv::DecorationBinding));
			write<int>(res_file, count);
		};

		write<int>(res_file, resources.uniform_buffers.size());
		for (auto &r : resources.uniform_buffers)
			_write_resource(r);
		write<int>(res_file, resources.storage_buffers.size());
		for (auto &r : resources.storage_buffers)
			_write_resource(r);
		write<int>(res_file, resources.sampled_images.size());
		for (auto &r : resources.sampled_images)
			_write_resource(r);
		write<int>(res_file, resources.storage_images.size());
		for (auto &r : resources.storage_images)
			_write_resource(r);

		int push_constant_size;
		for (auto &r : resources.push_constant_buffers)
			push_constant_size = glsl.get_declared_struct_size(glsl.get_type(r.type_id));
		write<int>(res_file, push_constant_size);
	}
}

