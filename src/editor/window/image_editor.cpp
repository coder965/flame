#include "../../global.h"
#include "../../input.h"
#include "../../graphics/buffer.h"
#include "../../graphics/command_buffer.h"
#include "../../ui/ui.h"

#include "image_editor.h"

ImageEditor::ImageEditor(const std::string &filename) :
	Window(filename, tke::ui::WindowHasMenu | tke::ui::WindowNoSavedSettings)
{
	first_cx = 800;
	first_cy = 600;

	image = tke::get_image(filename);
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
	ImVec2 image_size = ImVec2(image->get_cx(), image->get_cy());
	ImGui::InvisibleButton("canvas", image_size);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(ImGui::ImageID(image), image_pos, image_pos + image_size);
	if (ImGui::IsItemHovered())
	{
		if (tke::mouseLeft.pressing && penId != -1)
		{
			auto x = tke::mouseX - image_pos.x;
			auto y = tke::mouseY - image_pos.y;

			auto pixel = (unsigned char*)tke::defalut_staging_buffer->map(0, 4);
			memset(pixel, 0, 4);
			if (penId < 3)
			{
				pixel[penId] = 255;
				pixel[3] = 255;
			}
			pixel[3] = 255;
			tke::defalut_staging_buffer->unmap();

			auto cb = tke::begineOnceCommandBuffer();
			VkBufferImageCopy range = {};
			range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.imageSubresource.layerCount = 1;
			range.imageOffset.x = x;
			range.imageOffset.y = y;
			range.imageExtent.width = 1;
			range.imageExtent.height = 1;
			range.imageExtent.depth = 1;
			vkCmdCopyBufferToImage(cb->v, tke::defalut_staging_buffer->v, image->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &range);
			tke::endOnceCommandBuffer(cb);
		}
	}
}
