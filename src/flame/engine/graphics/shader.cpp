#include <sstream>
#include <stack>
#include <regex>

#include <spirv_glsl.hpp>
#include <flame/global.h>
#include <flame/common/filesystem.h>
#include <flame/common/system.h>
#include <flame/engine/core/core.h>
#include <flame/engine/core/application.h>
#include <flame/engine/graphics/descriptor.h>
#include <flame/engine/graphics/shader.h>
#include <flame/engine/graphics/pipeline.h>

namespace flame
{
	static std::string shader_path;
	static std::string vk_sdk_path;
	static const char additional_lines[] =
		"#version 450 core\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
	;
	static const auto additional_lines_count = std::count(std::begin(additional_lines), std::end(additional_lines), '\n');
	static bool watch_shader_file;

	Shader::Shader(const std::string &_filename, const std::vector<std::string> &_defines) :
		filename(_filename),
		defines(_defines),
		v(0)
	{
		auto ext = std::filesystem::path(filename).extension().string();
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
		std::filesystem::remove("temp.spv"); // glslc cannot write to an existed file

		auto _filename = engine_path + shader_path + "src/" + filename;
		auto shader_file_timestamp = std::filesystem::last_write_time(_filename);

		auto spv_filename = filename;
		for (auto &d : defines)
			spv_filename += "." + d;
		spv_filename += ".spv";
		spv_filename = engine_path + shader_path + "bin/" + spv_filename;

		bool spv_up_to_date = false;
		if (std::filesystem::exists(spv_filename))
		{
			if (std::filesystem::last_write_time(spv_filename) > shader_file_timestamp)
				spv_up_to_date = true;
		}

		if (!spv_up_to_date)
		{
			{
				std::ofstream ofile("temp.glsl");
				auto file = get_file_content(_filename);
				ofile.write(additional_lines, sizeof(additional_lines) - 1);
				ofile.write(file.first.get(), file.second);
			}
			std::string command_line(" temp.glsl ");
			for (auto &d : defines)
				command_line += "-D" + d + " ";
			command_line += " -flimit-file ";
			command_line += engine_path + "src/shader/src/shader_compile_config.conf";
			command_line += " -o temp.spv";
			auto output = create_process_and_get_output(vk_sdk_path + "/Bin/glslc.exe", command_line);
			std::filesystem::remove("temp.glsl");
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
			if (!v) // if we are running first time and no previous spv
				assert(0);
			return;
		}

		VkShaderModuleCreateInfo shader_info;
		shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_info.flags = 0;
		shader_info.pNext = nullptr;
		shader_info.codeSize = spv_file.second;
		shader_info.pCode = (uint32_t*)spv_file.first.get();

		if (v)
			vkDestroyShaderModule(vk_device.v, v, nullptr);
		chk_vk_res(vkCreateShaderModule(vk_device.v, &shader_info, nullptr, &v));

		auto res_filename = spv_filename + ".res";
		if (std::filesystem::exists(res_filename) &&
			std::filesystem::last_write_time(res_filename) > std::filesystem::last_write_time(spv_filename))
		{
			std::ifstream resFile(res_filename, std::ios::binary);

			auto ubo_count = read_int(resFile);
			for (auto i = 0; i < ubo_count; i++)
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

		for (auto p : referencing_pipelines)
			p->create();
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
						s->referencing_pipelines.push_back(pipeline);
					return s;
				}
				it++;
			}
			else
				it = _shaders.erase(it);
		}
		auto s = std::make_shared<Shader>(filename, defines);
		if (pipeline)
			s->referencing_pipelines.push_back(pipeline);
		_shaders.push_back(s);
		return s;
	}

	std::unique_ptr<FileWatcherHandler> shader_change_watcher;

	void init_shader(bool _watch_shader_file)
	{
		shader_path = "src/shader/";
		vk_sdk_path = getenv("VK_SDK_PATH");
		assert(vk_sdk_path != "");
		watch_shader_file = _watch_shader_file;

		if (_watch_shader_file)
		{
			shader_change_watcher = add_file_watcher(FileWatcherModeContent, engine_path + shader_path + "src/", [](const std::vector<FileChangeInfo> &infos) {
				std::vector<Shader*> changed_shaders;
				for (auto &i : infos)
				{
					Shader *shader = nullptr;
					for (auto &t : _shaders)
					{
						auto s = t.lock();
						if (s && s->filename == i.filename)
						{
							shader = s.get();
							break;
						}
					}
					if (!shader)
						continue;
					auto exist = false;
					for (auto &t : changed_shaders)
					{
						if (t == shader)
						{
							exist = true;
							break;
						}
					}
					if (!exist)
						changed_shaders.push_back(shader);
				}
				add_after_frame_event([=]() {
					for (auto s : changed_shaders)
						s->create();
				});
			});
		}
	}
}
