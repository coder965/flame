#include <flame/global.h>
#include <flame/engine/application.h>
#include <flame/graphics/command_buffer.h>

#include "image_editor.h"

ImageEditor::ImageEditor(const std::string &filename) :
	Window(filename, tke::ui::WindowHasMenu | tke::ui::WindowNoSavedSettings)
{
	first_cx = 800;
	first_cy = 600;

	texture = tke::get_or_create_texture(filename);
	staging_buffer = std::make_unique<tke::Buffer>(tke::BufferTypeStaging, texture->get_size());
	texture->copy_to_buffer(staging_buffer.get());
}

void ImageEditor::on_show()
{
	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Save"))
			/*tke::save_image_file(image->filename, image->levels[0], image->bpp)*/;
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Filter"))
	{
		if (ImGui::MenuItem("Signed Distance Field"))
		{
			auto pixel = (unsigned char*)staging_buffer->map(0, texture->get_size());
			//tke::create_and_save_image_distance_transform(pixel, texture->get_cy(), texture->get_cx(), 0, texture->bpp / 8, "sdf.png");
			staging_buffer->unmap();
		}

		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();

	enum Mode
	{
		ModeTerrainBlendMap = 0
	};

	const char *modeNames[] = {
		"terrain blend map"
	};
	static int mode = ModeTerrainBlendMap;
	ImGui::Combo("mode", &mode, modeNames, TK_ARRAYSIZE(modeNames));

	static int penId = -1;
	switch (mode)
	{
		case ModeTerrainBlendMap:
			if (penId < -1 || penId > 3)
				penId = -1;
			ImGui::RadioButton("Null", &penId, -1);
			ImGui::SameLine();
			ImGui::RadioButton("R", &penId, 0);
			ImGui::SameLine();
			ImGui::RadioButton("G", &penId, 1);
			ImGui::SameLine();
			ImGui::RadioButton("B", &penId, 2);
			ImGui::SameLine();
			ImGui::RadioButton("A", &penId, 3);
			break;
	}

	ImVec2 image_pos = ImGui::GetCursorScreenPos();
	ImVec2 image_size = ImVec2(texture->get_cx(), texture->get_cy());
	ImGui::InvisibleButton("canvas", image_size);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(ImGui::ImageID(texture), image_pos, image_pos + image_size);
	if (ImGui::IsItemHovered())
	{
		if (tke::app->mouse_button[0].pressing && penId != -1)
		{
			auto x = tke::app->mouseX - image_pos.x;
			auto y = tke::app->mouseY - image_pos.y;

			auto pixel = (unsigned char*)staging_buffer->map(texture->get_linear_offset(x, y), texture->bpp / 8);
			switch (penId)
			{
				case 0:
					pixel[0] = 0; pixel[1] = 0; pixel[2] = 255; pixel[3] = 255;
					break;
				case 1:
					pixel[0] = 0; pixel[1] = 255; pixel[2] = 0; pixel[3] = 255;
					break;
				case 2:
					pixel[0] = 255; pixel[1] = 0; pixel[2] = 0; pixel[3] = 255;
					break;
				case 3:
					pixel[0] = 0; pixel[1] = 0; pixel[2] = 0; pixel[3] = 255;
					break;
			}
			staging_buffer->unmap();

			texture->copy_from_buffer(staging_buffer.get(), 0, x, y, 1, 1, -1);
		}
	}
}
