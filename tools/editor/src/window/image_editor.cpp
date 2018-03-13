#include <flame/global.h>
#include <flame/engine/core/application.h>
#include <flame/engine/graphics/command_buffer.h>

#include "image_editor.h"

static int _magic_number = 0;

ImageEditor::ImageEditor(std::shared_ptr<flame::Texture> _texture) :
	ImageViewer(_texture->filename != "" ? _texture->filename : "Image - " + std::to_string(_magic_number++), _texture),
	penID(-1)
{
}

void ImageEditor::on_menu_bar()
{
	if (ImGui::BeginMenu("Filter"))
	{
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Special Save"))
	{
		if (ImGui::MenuItem("Save As Raw Bit RGBA32"))
		{
			auto dialog = new flame::ui::FileSelector("Save Image", flame::ui::FileSelectorSave, "", flame::ui::WindowModal | flame::ui::WindowNoSavedSettings);
			dialog->first_cx = 800;
			dialog->first_cy = 600;
			dialog->callback = [&](const std::string &filename) {
				auto pixel = (unsigned char*)staging_buffer->map(0, texture->get_size());
				flame::Image img(texture->get_cx(), texture->get_cy(), texture->channel, texture->bpp, pixel, false);
				img.save_as_raw_bit_rgba32(filename);
				staging_buffer->unmap();
				return true;
			};
		}

		ImGui::EndMenu();
	}
}

void ImageEditor::on_top_area()
{
	enum Mode
	{
		ModeTerrainBlendMap = 0
	};

	const char *modeNames[] = {
		"terrain blend map"
	};
	static int mode = ModeTerrainBlendMap;
	ImGui::Combo("mode", &mode, modeNames, TK_ARRAYSIZE(modeNames));

	switch (mode)
	{
		case ModeTerrainBlendMap:
			if (penID < -1 || penID > 3)
				penID = -1;
			ImGui::RadioButton("Null", &penID, -1);
			ImGui::SameLine();
			ImGui::RadioButton("R", &penID, 0);
			ImGui::SameLine();
			ImGui::RadioButton("G", &penID, 1);
			ImGui::SameLine();
			ImGui::RadioButton("B", &penID, 2);
			ImGui::SameLine();
			ImGui::RadioButton("A", &penID, 3);
			break;
	}
}

void ImageEditor::on_mouse_overing_image(ImVec2 image_pos)
{
	if (flame::app->mouse_button[0].pressing && penID != -1)
	{
		auto x = flame::app->mouseX - image_pos.x;
		auto y = flame::app->mouseY - image_pos.y;

		auto pixel = (unsigned char*)staging_buffer->map(texture->get_linear_offset(x, y), texture->bpp / 8);
		switch (penID)
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
				//pixel[0] = 0; pixel[1] = 0; pixel[2] = 0; pixel[3] = 0;
				pixel[3] = 0;
				break;
		}
		staging_buffer->unmap();

		texture->copy_from_buffer(staging_buffer.get(), 0, x, y, 1, 1, -1);
	}
}
