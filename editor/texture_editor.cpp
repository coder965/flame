#include "../src/ui/ui.h"

#include "editor.h"
#include "texture_editor.h"

static void _texture_editor_remove_image(TextureEditor *e)
{
	if (e->image)
	{
		tke::removeGuiImage(e->image);
		delete e->image;
		e->image = nullptr;
	}
}

void TextureEditor::show()
{
	ImGui::Begin("Texture Editor", &opened);

	if (ImGui::Button("New"))
		ImGui::OpenPopup("Image Attribute");
	if (ImGui::BeginPopupModal("Image Attribute", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static int cx = 512;
		static int cy = 512;
		const char *typeNames[] = {
			"color R8G8B8A8"
		};
		static int type = 0;
		ImGui::Combo("type", &type, typeNames, TK_ARRAYSIZE(typeNames));

		if (ImGui::Button("Create"))
		{
			_texture_editor_remove_image(this);
			image = new tke::Image(cx, cy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			auto cb = tke::begineOnceCommandBuffer();
			VkClearColorValue clearValue = { 0.f, 0.f, 0.f, 1.f };
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;
			vkCmdClearColorImage(cb->v, image->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &range);
			tke::endOnceCommandBuffer(cb);
			tke::addGuiImage(image);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Load"))
	{

	}
	ImGui::SameLine();
	if (ImGui::Button("Save"))
		ImGui::OpenPopup("Save Attribute");
	if (ImGui::BeginPopupModal("Save Attribute", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char filename[260];
		ImGui::InputText("Filename", filename, TK_ARRAYSIZE(filename));

		if (ImGui::Button("Save"))
		{
			auto cb = tke::begineOnceCommandBuffer();
			VkBufferImageCopy range = {};
			range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.imageSubresource.layerCount = 1;
			range.imageOffset.x = 0;
			range.imageOffset.y = 0;
			range.imageExtent.width = image->levels[0].cx;
			range.imageExtent.height = image->levels[0].cy;
			range.imageExtent.depth = 1;
			vkCmdCopyImageToBuffer(cb->v, image->v, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, tke::stagingBuffer->v, 1, &range);
			tke::endOnceCommandBuffer(cb);

			auto pixel = (unsigned char*)tke::stagingBuffer->map(0, image->levels[0].size);
			tke::saveImageFile(filename, pixel, image->levels[0].cx, image->levels[0].cy, 4);
			tke::stagingBuffer->unmap();

			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
	}

	if (image)
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
		ImVec2 image_size = ImVec2(image->levels[0].cx, image->levels[0].cy);
		ImGui::InvisibleButton("canvas", image_size);
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddImage(ImTextureID(image->index), image_pos, image_pos + image_size);
		if (ImGui::IsItemHovered())
		{
			if (tke::mouseLeft.pressing && penId != -1)
			{
				auto x = tke::mouseX - image_pos.x;
				auto y = tke::mouseY - image_pos.y;

				auto pixel = (unsigned char*)tke::stagingBuffer->map(0, 4);
				memset(pixel, 0, 4);
				if (penId < 3) 
				{
					pixel[penId] = 255;
					pixel[3] = 255;
				}
				pixel[3] = 255;
				tke::stagingBuffer->unmap();

				auto cb = tke::begineOnceCommandBuffer();
				VkBufferImageCopy range = {};
				range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				range.imageSubresource.layerCount = 1;
				range.imageOffset.x = x;
				range.imageOffset.y = y;
				range.imageExtent.width = 1;
				range.imageExtent.height = 1;
				range.imageExtent.depth = 1;
				vkCmdCopyBufferToImage(cb->v, tke::stagingBuffer->v, image->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &range);
				tke::endOnceCommandBuffer(cb);
			}
		}
		if (ImGui::Button("Remove"))
			_texture_editor_remove_image(this);
	}
	else
	{
		ImGui::Text("[No Image]");
	}

	ImGui::End();
}

TextureEditor *textureEditor = nullptr;