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

