#include <filesystem>

#include "../../../src/ui/ui.h"

#include "resource_explorer.h"
#include "../editor.h"

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
	:FileSelector(&resourceExplorerClass, true)
{
	path = project_path;
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

FileSelector::FileItem *ResourceExplorer::on_new_file_item()
{
	return new ResourceExplorerFileItem;
}

bool ResourceExplorer::on_window_begin()
{
	auto open = ImGui::Begin("Resource Explorer", &opened);
	if (project_path == "")
	{
		ImGui::Text("No project opened.");
		return false;
	}
	return open;
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
				i->image = tke::getImage((path / i->value).string());
				if (i->image)
					tke::addUiImage(i->image.get());
			}
			break;
		case tke::FileTypeScene:
			if (ImGui::IsMouseDoubleClicked(0))
			{
				auto s = tke::getScene((path / i->value).string());
				if (s)
				{
					s->camera.setMode(tke::CameraMode::targeting);
					auto w = new SceneEditor(s);
					windows.push_back(std::move(std::unique_ptr<Window>(w)));
				}
			}
			break;
	}
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
