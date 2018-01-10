#include <filesystem>

#include "../../ui/ui.h"
#include "../../graphics/descriptor.h"

#include "resource_explorer.h"
#include "image_editor.h"
#include "model_editor.h"
#include "terrain_editor.h"
#include "scene_editor.h"

ResourceExplorer *resourceExplorer = nullptr;

ResourceExplorer::ResourceExplorer()
	:FileSelector("Resource Explorer", false, true, true, 0)
{
}

ResourceExplorer::~ResourceExplorer()
{
	resourceExplorer = nullptr;
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
				i->image = tke::get_image(i->filename);
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
					s->name = "scene";
					s->camera.setMode(tke::CameraMode::targeting);
					if (!scene_editor)
						scene_editor = std::make_unique<SceneEditor>(s);
					else
						scene_editor->scene = s;
				}
			}
			break;
	}
}

void ResourceExplorer::on_top_area_show()
{
}

void ResourceExplorer::on_bottom_area_show()
{
	if (list_index != -1 && list_index >= dir_list.size())
	{
		auto i = file_list[list_index - dir_list.size()].get();
		ImGui::Text("size: %d byte", i->file_size);
	}
}

void ResourceExplorer::on_right_area_show()
{
	if (list_index != -1 && list_index >= dir_list.size())
	{
		auto i = (ResourceExplorerFileItem*)file_list[list_index - dir_list.size()].get();
		switch (i->file_type)
		{
			case tke::FileTypeImage:
			{
				auto w = right_region_width;
				auto h = ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() * 3;
				ImGui::Text("%d x %d", i->image->levels[0].cx, i->image->levels[0].cy);
				auto image_rect = tke::fit_rect_no_zoom_in(glm::vec2(w, h), glm::vec2(i->image->levels[0].cx, i->image->levels[0].cy));
				auto pos = ImGui::GetCursorPos();
				ImGui::SetCursorPos(pos + ImVec2(image_rect.x, image_rect.y));
				ImGui::Image((ImTextureID)tke::get_ui_image_index(i->image), ImVec2(image_rect.z, image_rect.w));
				break;
			}
		}
	}
}
