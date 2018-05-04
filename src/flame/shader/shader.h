#pragma once

#ifdef _FLAME_SHADER_EXPORTS
#define FLAME_SHADER_EXPORTS __declspec(dllexport)
#else
#define FLAME_SHADER_EXPORTS __declspec(dllimport)
#endif

#include <flame/string.h>

namespace flame
{
	FLAME_SHADER_EXPORTS void compile_shader(const char *glsl_file_in, int shader_define_count, 
		ShortString *shader_defines, const char *config_file, const char *spv_file_out, 
		void(*compile_output_callback)(const char *filename, int line, const char *what));

	/*  == compile_shader ==
		
		in the callback, if the filename == nullptr and line == -1, this means it is a special 
		entry, when what == "##start" it is the start of block, what == "##end" it is the end of block
	*/

	FLAME_SHADER_EXPORTS void produce_shader_resource_file(const char *spv_file_in, const char *res_file_out);
}

