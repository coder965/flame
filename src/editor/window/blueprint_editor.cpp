#include "blueprint_editor.h"

BlueprintEditor::BlueprintEditor() :
	Window("")
{
}

BlueprintEditor::~BlueprintEditor()
{
	blueprint_editor = nullptr;
}

void BlueprintEditor::on_show()
{
}

BlueprintEditor *blueprint_editor = nullptr;
