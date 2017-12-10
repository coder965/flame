#include <filesystem>

#include "../../../src/ui/ui.h"

#include "../editor.h"
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
	:FileSelector(&resourceExplorerClass, true, 0)
{
	current_path = project_path;
}

ResourceExplorer::~ResourceExplorer()
{
	resourceExplorer = nullptr;
}

int ResourceExplorer::on_left_area_width()
{
	return 300;
}

bool ResourceExplorer::on_refresh()
{
	return project_path != "";
}

bool ResourceExplorer::on_parent_path()
{
	return project_path != current_path;
}

FileSelector::FileItem *ResourceExplorer::on_new_file_item()
{
	return new ResourceExplorerFileItem;
}

bool ResourceExplorer::on_window_begin()
{
	if (ImGui::Begin("Resource Explorer", &opened))
	{
		if (project_path == "")
		{
			ImGui::Text("No project opened.");
			return false;
		}
		return true;
	}
	return false;
}

void ResourceExplorer::on_window_end()
{
	ImGui::End();
}

void ResourceExplorer::on_file_item_selected(FileItem *_i, bool doubleClicked)
{
	auto i = (ResourceExplorerFileItem*)_i;

	switch (i->file_type)
	{
		case tke::FileTypeImage:
			if (!i->image)
			{
				i->image = tke::getImage((current_path / i->value).string());
				if (i->image)
					tke::addUiImage(i->image.get());
			}
			if (ImGui::IsMouseDoubleClicked(0))
				new ImageEditor(i->image);
			break;
		case tke::FileTypeModel:
			if (ImGui::IsMouseDoubleClicked(0))
			{
				auto m = tke::getModel((current_path / i->value).string());
				if (m)
					new ModelEditor(m);
			}
			break;
		case tke::FileTypeTerrain:
			if (ImGui::IsMouseDoubleClicked(0))
				new TerrainEditor;
			break;
		case tke::FileTypeScene:
			if (ImGui::IsMouseDoubleClicked(0))
			{
				auto s = tke::getScene((current_path / i->value).string());
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
