#include "../../../src/ui/ui.h"

#include "terrain_editor.h"

std::string TerrainEditorClass::getName()
{
	return "terrain editor";
}

Window *TerrainEditorClass::load(tke::AttributeTreeNode *n)
{
	terrainEditor = new TerrainEditor;
	return terrainEditor;
}

TerrainEditorClass terrainEditorClass;

TerrainEditor *terrainEditor;

TerrainEditor::TerrainEditor()
	:Window(&terrainEditorClass)
{
}

void TerrainEditor::show()
{
	ImGui::Begin("Terrain Editor", &opened);
	ImGui::End();
}
