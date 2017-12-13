#include <filesystem>

#include "../../../src/ui/ui.h"

#include "resource_explorer.h"
#include "image_editor.h"
#include "model_editor.h"
#include "terrain_editor.h"
#include "scene_editor.h"

std::string ResourceExplorerClass::getName()
{
	return "resource explorer";
}

Window *ResourceExplorerClass::load(tke::AttributeTreeNode *n)
{
	resourceExplorer = new ResourceExplorer;
	return resourceExplorer;
}

ResourceExplorerClass resourceExplorerClass;

ResourceExplorer *resourceExplorer = nullptr;

ResourceExplorerFileItem::~ResourceExplorerFileItem()
{
	if (image && image->index != 0)
		tke::removeUiImage(image.get());
}

ResourceExplorer::ResourceExplorer()
	:FileSelector(&resourceExplorerClass, "Resource Explorer", false, true, 0)
{
	set_current_path("d:\\TK_Engine\\editor");
}

ResourceExplorer::~ResourceExplorer()
{
	resourceExplorer = nullptr;
}

int ResourceExplorer::on_left_area_width()
{
	return 300;
}

FileSelector::FileItem *ResourceExplorer::on_new_file_item()
{
	return new ResourceExplorerFileItem;
}

void ResourceExplorer::on_file_item_selected(FileItem *_i, bool doubleClicked)
{
	auto i = (ResourceExplorerFileItem*)_i;

	switch (i->file_type)
	{
		case tke::FileTypeImage:
			if (!i->image)
			{
				i->image = tke::getImage(i->filename);
				if (i->image)
					tke::addUiImage(i->image.get());
			}
			if (doubleClicked)
				new ImageEditor(i->image);
			break;
		case tke::FileTypeModel:
			if (doubleClicked)
			{
				auto m = tke::getModel(i->filename);
				if (m)
					new ModelEditor(m);
			}
			break;
		case tke::FileTypeTerrain:
			if (doubleClicked)
				new TerrainEditor;
			break;
		case tke::FileTypeScene:
			if (doubleClicked)
			{
				auto s = tke::getScene(i->filename);
				if (s)
				{
					s->camera.setMode(tke::CameraMode::targeting);
					new SceneEditor(s);
				}
			}
			break;
	}
}

void ResourceExplorer::on_top_area_begin()
{
}

void ResourceExplorer::on_bottom_area_begin()
{
	if (list_index != -1 && list_index >= dir_list.size())
	{
		auto i = file_list[list_index - dir_list.size()].get();
		ImGui::Text("size: %d byte", i->file_size);
	}
}

void ResourceExplorer::on_right_area_begin()
{
	ImGui::SameLine();
	ImGui::BeginGroup();
	if (list_index != -1 && list_index >= dir_list.size())
	{
		auto i = (ResourceExplorerFileItem*)file_list[list_index - dir_list.size()].get();
		switch (i->file_type)
		{
			case tke::FileTypeImage:
				ImGui::Text("%d x %d", i->image->levels[0].cx, i->image->levels[0].cy);
				ImGui::Image((ImTextureID)i->image->index, ImVec2(i->image->levels[0].cx, i->image->levels[0].cy));
				break;
		}
	}
	ImGui::EndGroup();
}
