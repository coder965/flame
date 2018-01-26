#include <filesystem>

#include "../../global.h"
#include "../../ui/ui.h"
#include "../../graphics/descriptor.h"

#include "../select.h"
#include "resource_explorer.h"
#include "inspector.h"
#include "image_editor.h"
#include "model_editor.h"
#include "terrain_editor.h"
#include "scene_editor.h"

ResourceExplorer *resourceExplorer = nullptr;

ResourceExplorer::ResourceExplorer()
	:FileSelector("Resource Explorer", false, true, true, false, 0, 0, true)
{
}

ResourceExplorer::~ResourceExplorer()
{
	resourceExplorer = nullptr;
}

void ResourceExplorer::on_right_area_show()
{
	static char filter[260];
	ImGui::InputText(ICON_FA_SEARCH, filter, TK_ARRAYSIZE(filter));
	if (select_dir)
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		const ImVec2 img_size(64.f, 64.f);
		const ImVec2 widget_size = img_size + ImVec2(0.f, 20.f);
		auto &style = ImGui::GetStyle();
		int column_count = (right_region_width - style.WindowPadding.x * 2.f) / (widget_size.x + style.FramePadding.x + style.ItemSpacing.x);
		ImGui::Columns(column_count < 1 ? 1 : column_count, nullptr, false);
		static const auto fShow = [&](ItemData *d, bool is_folder) {
			auto pos = ImGui::GetCursorScreenPos();

			//draw_list->PushClipRect(pos, pos + widget_size, true);
			if (column_count > 1)
				pos.x += (ImGui::GetColumnWidth() - widget_size.x) * 0.5f;
			ImGui::SetCursorScreenPos(pos);
			ImGui::InvisibleButton(d->filename.c_str(), widget_size);
			int sel = 0;
			if (ImGui::IsItemHovered())
			{
				if (ImGui::IsMouseClicked(1))
					selected = d->filename;
				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (is_folder)
						select_dir = (DirItem*)d;
					else
					{
						auto f = (FileItem*)d;
						switch (f->file_type)
						{
							case tke::FileTypeImage:
							{
								new ImageEditor(f->filename);
								break;
							}
							case tke::FileTypeModel:
							{
								auto m = tke::getModel(f->filename);
								if (m)
									new ModelEditor(m);
								break;
							}
							case tke::FileTypeTerrain:
							{
								new TerrainEditor;
								break;
							}
							case tke::FileTypeScene:
							{
								auto s = tke::create_scene(f->filename);
								if (s)
								{
									s->name = "scene";
									if (!scene_editor)
										scene_editor = new SceneEditor(s);
									else
										scene_editor->scene = s;
								}
								break;
							}
						}
					}
				}
				sel = 1;
			}
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("file", d->filename.c_str(), d->filename.size() + 1);
				ImGui::TextUnformatted(d->filename.c_str());
				ImGui::EndDragDropSource();
			}
			if (selected.get_filename() == d->filename)
				sel = 2;
			if (sel > 0)
				draw_list->AddRectFilled(pos, pos + widget_size, ImColor(255, 122, 50, sel == 1 ? 60 : 255));
			std::string img_name;
			if (is_folder)
				img_name = "folder.png";
			else
			{
				auto f = (FileItem*)d;
				if (f->file_type == tke::FileTypeImage)
					img_name = f->filename;
				else
					img_name = "file.png";
			}
			{
				auto i = tke::get_image(img_name);
				if (i)
					draw_list->AddImage(ImGui::ImageID(i), pos, pos + img_size);
			}
			draw_list->AddText(pos + ImVec2(0, img_size.y), ImColor(0, 0, 0), d->value.c_str());
			//draw_list->PopClipRect();
			if (column_count > 1)
				ImGui::NextColumn();
		};
		for (auto &d : select_dir->dir_list)
			fShow(d.get(), true);
		for (auto &f : select_dir->file_list)
			fShow(f.get(), false);
		ImGui::Columns(1);
	}
}
