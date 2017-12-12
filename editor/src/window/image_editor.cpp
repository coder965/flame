#include "../../../src/ui/ui.h"
#include "../../../src/core.h"

#include "../editor.h"
#include "image_editor.h"

std::string ImageEditorClass::getName()
{
	return "image editor";
}

Window *ImageEditorClass::load(tke::AttributeTreeNode *n)
{
	auto a = n->firstAttribute("filename");
	if (a)
	{
		auto i = tke::getImage(a->value);
		if (i)
		{
			auto w = new ImageEditor(i);
			return w;
		}
	}
	return nullptr;
}

ImageEditorClass imageEditorClass;

ImageEditor::ImageEditor(std::shared_ptr<tke::Image> _image)
	:Window(&imageEditorClass), image(_image)
{
	tke::addUiImage(image.get());
}

ImageEditor::~ImageEditor()
{
	tke::removeUiImage(image.get());
}

void ImageEditor::do_show()
{
	ImGui::Begin(("Image - " + image->filename).c_str(), &opened, ImGuiWindowFlags_MenuBar);

	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Save"))
			tke::saveImageFile(image->filename, image->levels[0], image->bpp);
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

	ImGui::End();
}
