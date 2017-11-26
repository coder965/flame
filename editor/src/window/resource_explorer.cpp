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

void load_resource()
{
	//tke::AttributeTree at("data", "data.xml");
	//for (auto &c : at.children)
	//{
	//	if (c->name == "texture")
	//	{
	//		auto a = c->firstAttribute("filename");
	//		auto i = tke::createImage(a->value, false, true);
	//		if (i)
	//		{
	//			tke::addUiImage(i.get());
	//		}
	//	}
	//	else if (c->name == "model")
	//	{
	//		auto a = c->firstAttribute("filename");
	//		tke::createModel(a->value);
	//	}
	//	else if (c->name == "animation")
	//	{
	//		auto a = c->firstAttribute("filename");
	//		tke::createAnimation(a->value);
	//	}
	//	else if (c->name == "scene")
	//	{
	//		auto a = c->firstAttribute("filename");
	//		auto s = new tke::Scene;
	//		s->load(a->value);
	//		s->camera.setMode(tke::CameraMode::targeting);
	//		s->camera.setCoord(0.f, 5.f, 0.f);
	//		tke::addScene(s);
	//	}
	//}
}

ResourceExplorerFileListItem::~ResourceExplorerFileListItem()
{
	if (image && image->index != 0)
		tke::removeUiImage(image.get());
}

ResourceExplorer::ResourceExplorer()
	:Window(&resourceExplorerClass)
{
	path = project_path;
	refresh();
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
				auto ext = it->path().extension().string();
				const char *prefix;
				if (tke::isImageFile(ext))
				{
					i->file_type = ResourceExplorerFileListItem::FileTypeImage;
					prefix = ICON_FA_FILE_IMAGE_O" ";
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

	if (project_path == "")
		ImGui::Text("No project opened.");
	else
	{
		const float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
		bool need_refresh = false;

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

		ImGui::BeginChild("list", ImVec2(0, 0), true);
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
						i->image = tke::createImage((path / i->value).string());
						tke::addUiImage(i->image);
					}
					break;
				}
			}
			index++;
		}
		ImGui::EndChild();

		ImGui::EndChild();

		ImGui::SameLine();
		if (list_index != -1 && list_index >= dir_list.size())
		{
			auto i = file_list[list_index - dir_list.size()].get();
			switch (i->file_type)
			{
			case ResourceExplorerFileListItem::FileTypeImage:
				ImGui::Image((ImTextureID)i->image->index, ImVec2(i->image->levels[0].cx, i->image->levels[0].cy));
				break;
			}
		}

		if (need_refresh)
			refresh();

		//if (ImGui::TreeNode("Textures"))
		//{
		//	for (int i = 0; i < tke::textures.size(); i++)
		//	{
		//		auto t = tke::textures[i].get();
		//		if (ImGui::Selectable(t->filename.c_str(), lastItemType == lastItemTypeTexture && itemIndex == i))
		//		{
		//			lastItemType = lastItemTypeTexture;
		//			itemIndex = i;
		//		}
		//	}
		//	ImGui::TreePop();
		//}
		//if (ImGui::TreeNode("Animations"))
		//{
		//	for (int i = 0; i < tke::animations.size(); i++)
		//	{
		//		auto a = tke::animations[i].get();
		//		if (ImGui::Selectable(a->filename.c_str(), lastItemType == lastItemTypeAnimation && itemIndex == i, ImGuiSelectableFlags_AllowDoubleClick))
		//		{
		//			lastItemType = lastItemTypeAnimation;
		//			itemIndex = i;
		//			if (ImGui::IsMouseDoubleClicked(0))
		//				;
		//		}
		//	}
		//	ImGui::TreePop();
		//}
		//if (ImGui::TreeNode("Models"))
		//{
		//	for (int i = 0; i < tke::models.size(); i++)
		//	{
		//		auto m = tke::models[i].get();
		//		if (ImGui::Selectable(m->filename.c_str(), lastItemType == lastItemTypeModel && itemIndex == i, ImGuiSelectableFlags_AllowDoubleClick))
		//		{
		//			lastItemType = lastItemTypeModel;
		//			itemIndex = i;
		//		}
		//	}
		//	ImGui::TreePop();
		//}
		//if (ImGui::TreeNode("Scenes"))
		//{
		//	for (int i = 0; i < tke::scenes.size(); i++)
		//	{
		//		auto s = tke::scenes[i].get();
		//		if (ImGui::Selectable(s->name.c_str(), lastItemType == lastItemTypeScene && itemIndex == i, ImGuiSelectableFlags_AllowDoubleClick))
		//		{
		//			lastItemType = lastItemTypeScene;
		//			itemIndex = i;
		//			if (ImGui::IsMouseDoubleClicked(0))
		//				windows.push_back(std::move(std::make_unique<SceneEditor>(s)));
		//		}
		//	}
		//	ImGui::TreePop();
		//}
	}

	ImGui::End();
}
