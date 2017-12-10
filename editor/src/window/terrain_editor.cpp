#include "../../../src/ui/ui.h"

#include "terrain_editor.h"

std::string TerrainEditorClass::getName()
{
	return "terrain editor";
}

Window *TerrainEditorClass::load(tke::AttributeTreeNode *n)
{
	auto w = new TerrainEditor;
	return w;
}

TerrainEditorClass terrainEditorClass;

TerrainEditor::TerrainEditor()
	:Window(&terrainEditorClass)
{
}

void TerrainEditor::show()
{
	ImGui::Begin("Terrain -", &opened);
	ImGui::End();
}
