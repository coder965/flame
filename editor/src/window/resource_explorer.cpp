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

ResourceExplorerFileListItem::~ResourceExplorerFileListItem()
{
	if (image && image->index != 0)
		tke::removeUiImage(image.get());
}

ResourceExplorer::ResourceExplorer()
	:Window(&resourceExplorerClass)
{
	path = project_path;
}

ResourceExplorer::~ResourceExplorer()
{
	resourceExplorer = nullptr;
}

void ResourceExplorer::refresh()
{
	dir_list.clear();
	file_list.clear();
	list_index = -1;

	if (project_path != "")
	{
		std::experimental::filesystem::directory_iterator end_it;
		for (std::experimental::filesystem::directory_iterator it(path); it != end_it; it++)
		{
			auto str = it->path().filename().string();
			if (std::experimental::filesystem::is_directory(it->status()))
			{
				auto i = std::make_unique<ResourceExplorerDirListItem>();
				i->value = str;
				i->name = ICON_FA_FOLDER_O" " + str;
				dir_list.push_back(std::move(i));
			}
			else
			{
				auto i = std::make_unique<ResourceExplorerFileListItem>();

				i->file_size = std::experimental::filesystem::file_size(it->path());

				auto ext = it->path().extension().string();
				const char *prefix;
				if (tke::isTextFile(ext))
				{
					i->file_type = ResourceExplorerFileListItem::FileTypeText;
					prefix = ICON_FA_FILE_TEXT_O" ";
				}
				else if (tke::isImageFile(ext))
				{
					i->file_type = ResourceExplorerFileListItem::FileTypeImage;
					prefix = ICON_FA_FILE_IMAGE_O" ";
				}
				else if (tke::isModelFile(ext))
				{
					i->file_type = ResourceExplorerFileListItem::FileTypeModel;
					prefix = ICON_FA_FILE_O" ";
				}
				else if (tke::isSceneFile(ext))
				{
					i->file_type = ResourceExplorerFileListItem::FileTypeScene;
					prefix = ICON_FA_FILE_O" ";
				}
				else
					prefix = ICON_FA_FILE_O" ";
				i->value = str;
				i->name = prefix + str;

				file_list.push_back(std::move(i));
			}
		}
	}
}

void ResourceExplorer::show()
{
	ImGui::Begin("Resource Explorer", &opened);

	if (need_refresh)
	{
		refresh();
		need_refresh = false;
	}

	if (project_path == "")
		ImGui::Text("No project opened.");
	else
	{
		const float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

		ImGui::BeginChild("left", ImVec2(300, 0));

		ImGui::Text(path.string().c_str());
		ImGui::SameLine();
		{
			static float buttonWidth = 100.f;
			float pos = buttonWidth + itemSpacing;
			ImGui::SameLine(ImGui::GetWindowWidth() - pos);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::Button(ICON_FA_CHEVRON_UP))
			{
				if (path != project_path)
				{
					path = path.parent_path();
					need_refresh = true;
				}
			}
			buttonWidth = ImGui::GetItemRectSize().x;
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Parent Path");
		ImGui::PopStyleColor();
		ImGui::Separator();

		ImGui::BeginChild("list", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing() - 1), true);
		int index = 0;
		for (auto &i : dir_list)
		{
			if (ImGui::Selectable(i->name.c_str(), index == list_index, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick))
			{
				list_index = index;
				if (ImGui::IsMouseDoubleClicked(0))
				{
					path /= i->value;
					need_refresh = true;
				}
			}
			index++;
		}
		for (auto &i : file_list)
		{
			if (ImGui::Selectable(i->name.c_str(), index == list_index, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick))
			{
				list_index = index;
				switch (i->file_type)
				{
					case ResourceExplorerFileListItem::FileTypeImage:
						if (!i->image)
						{
							i->image = tke::getImage((path / i->value).string());
							if (i->image)
								tke::addUiImage(i->image.get());
						}
						break;
					case ResourceExplorerFileListItem::FileTypeScene:
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
			index++;
		}
		ImGui::EndChild();

		if (list_index != -1 && list_index >= dir_list.size())
		{
			auto i = file_list[list_index - dir_list.size()].get();
			ImGui::Text("size: %d byte", i->file_size);
		}

		ImGui::EndChild();

		ImGui::SameLine();
		if (list_index != -1 && list_index >= dir_list.size())
		{
			auto i = file_list[list_index - dir_list.size()].get();
			switch (i->file_type)
			{
				case ResourceExplorerFileListItem::FileTypeImage:
					ImGui::Text("%d x %d", i->image->levels[0].cx, i->image->levels[0].cy);
					ImGui::Image((ImTextureID)i->image->index, ImVec2(i->image->levels[0].cx, i->image->levels[0].cy));
					break;
			}
		}
	}

	ImGui::End();
}
