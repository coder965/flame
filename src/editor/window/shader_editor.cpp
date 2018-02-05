#include "shader_editor.h"

ShaderEditor::ShaderEditor() :
	FileSelector("Shader Editor", FileSelectorOpen)
{
	splitter.size[0] = 300;


}

ShaderEditor::~ShaderEditor()
{
	shader_editor = nullptr;
}

ShaderEditor *shader_editor = nullptr;
