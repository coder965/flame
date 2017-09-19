#include <sstream>
#include <stack>
#include <regex>

#include "../src/utils.h"
#include "../../SPIRV-Cross/spirv_glsl.hpp"

namespace fs = std::experimental::filesystem;

int main(int argc, char **argv)
{
	tke::iterateDirectory("../pipeline/", [&](const fs::path &name, bool is_dir) {
		if (!is_dir)
		{
			auto path = name.parent_path().string();

			auto ext = name.extension().string();
			if (ext == ".xml")
			{
				std::vector<std::string> defines;

				tke::AttributeTree at_pipeline("pipeline", name.string());
				for (auto &n : at_pipeline.children)
				{
					if (n->name == "define")
						defines.push_back(n->value);
					else if (n->name == "stage")
					{
						auto stage_filename = n->firstAttribute("filename")->value;
						auto input_filename = path + "/" + stage_filename;
						auto output_filename = input_filename;
						for (auto &d : defines)
							output_filename += "." + d;
						output_filename += ".spv";

						std::string cmd_str("glslc ");
						cmd_str += input_filename;
						cmd_str += " ";
						for (auto &d : defines)
							cmd_str += "-D" + d + " ";
						cmd_str += " -flimit-file ";
						cmd_str += "my_config.conf";
						cmd_str += " -fauto-bind-uniforms ";
						cmd_str += " -o " + output_filename;

						system(cmd_str.c_str());

						if (fs::exists(output_filename))
						{
							std::ifstream spv_file(output_filename, std::ios::binary);
							auto size = tke::file_length(spv_file);
							std::vector<unsigned int> spv_data(size / sizeof(unsigned int));
							spv_file.read((char*)spv_data.data(), size); 

							spirv_cross::CompilerGLSL glsl(std::move(spv_data));

							spirv_cross::ShaderResources resources = glsl.get_shader_resources();

							for (auto &resource : resources.sampled_images)
							{
								unsigned int set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
								unsigned int binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

								int cut = 1;
							}

							int cut = 1;
						}
					}
				}
			}
		}
	});

	system("pause");

    return 0;
}

