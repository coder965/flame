#include <flame/global.h>
#include <flame/engine/application.h>
#include <flame/graphics/command_buffer.h>

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
		if (ImGui::MenuItem("Signed Distance Field"))
		{
			auto pixel = (unsigned char*)staging_buffer->map(0, texture->get_size());
			flame::Image img(texture->get_cx(), texture->get_cy(), texture->channel, texture->bpp, pixel, false);
			auto sdf = img.create_distance_transform(0);
			staging_buffer->unmap();
			auto t = std::make_shared<flame::Texture>(texture->get_cx(), texture->get_cy(),
				VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, false);
			t->fill_data(0, sdf->data);
			new ImageEditor(t);
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
				pixel[0] = 0; pixel[1] = 0; pixel[2] = 0; pixel[3] = 255;
				break;
		}
		staging_buffer->unmap();

		texture->copy_from_buffer(staging_buffer.get(), 0, x, y, 1, 1, -1);
	}
}
