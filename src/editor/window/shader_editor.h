#pragma once

#include "file_selector.h"

struct Shader
{
	std::string filename;
	std::string name;
};

struct ShaderEditor : FileSelector
{
	std::vector<std::unique_ptr<Shader>> shaders;

	ShaderEditor();
	~ShaderEditor();
};

extern ShaderEditor *shader_editor;
