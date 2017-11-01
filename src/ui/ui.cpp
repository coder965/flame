#include <process.h>

#include "ui.h"
#include "../core.h"
#include "../render/image.h"

namespace tke
{
	static bool need_clear = false;

	static void _gui_renderer(ImDrawData* draw_data)
	{
		ImGuiIO& io = ImGui::GetIO();
		if ((int)(io.DisplaySize.x * io.DisplayFramebufferScale.x) == 0 || (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y) == 0)
			return;
		draw_data->ScaleClipRects(io.DisplayFramebufferScale);

		static OnceVertexBuffer	*vertexBuffer = nullptr;
		size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
		if (!vertexBuffer || vertexBuffer->size < vertex_size)
		{
			if (vertexBuffer) delete vertexBuffer;
			vertexBuffer = new OnceVertexBuffer(vertex_size);
		}

		static OnceIndexBuffer *indexBuffer = nullptr;
		size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
		if (!indexBuffer || indexBuffer->size < index_size)
		{
			if (indexBuffer) delete indexBuffer;
			indexBuffer = new OnceIndexBuffer(index_size);
		}

		{
			auto vtx_dst = (ImDrawVert*)vertexBuffer->map(0, vertex_size);
			auto idx_dst = (ImDrawIdx*)indexBuffer->map(0, index_size);
			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
				memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

				vtx_dst += cmd_list->VtxBuffer.Size;
				idx_dst += cmd_list->IdxBuffer.Size;
			}
			vertexBuffer->unmap();
			indexBuffer->unmap();
		}

		ui_cb->reset();
		ui_cb->begin();

		if (ui_waitEvents.size() > 0)
			ui_cb->waitEvents(ui_waitEvents.size(), ui_waitEvents.data());

		VkClearValue clear_value = { bkColor.r, bkColor.g, bkColor.b };
		ui_cb->beginRenderPass(need_clear ? renderPass_window_clear : renderPass_window, 
			window_framebuffers[window_imageIndex].get(), need_clear ? &clear_value : nullptr);

		ui_cb->setViewportAndScissor(window_cx, window_cy);

		ui_cb->bindVertexBuffer(vertexBuffer);
		ui_cb->bindIndexBuffer(indexBuffer);

		ui_cb->bindPipeline(plainPipeline_2d);
		ui_cb->bindDescriptorSet();

		ui_cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec4), &glm::vec4(2.f / io.DisplaySize.x, 2.f / io.DisplaySize.y, -1.f, -1.f));

		int vtx_offset = 0;
		int idx_offset = 0;
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmd_list, pcmd);
					pcmd->TextureId;
				}
				else
				{
					ui_cb->setScissor(ImMax((int32_t)(pcmd->ClipRect.x), 0),
						ImMax((int32_t)(pcmd->ClipRect.y), 0),
						ImMax((uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x), 0),
						ImMax((uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y + 1), 0)); // TODO: + 1??????
					ui_cb->drawIndex(pcmd->ElemCount, idx_offset, vtx_offset, 1, (int)pcmd->TextureId);
				}
				idx_offset += pcmd->ElemCount;
			}
			vtx_offset += cmd_list->VtxBuffer.Size;
		}

		ui_cb->endRenderPass();

		if (ui_waitEvents.size())
		{
			for (auto &e : ui_waitEvents)
				ui_cb->resetEvent(e);
		}

		ui_cb->end();
	}

	static void _SetClipboardCallback(void *user_data, const char *s)
	{
		setClipBoard(s);
	}

	static const char *_GetClipboardCallback(void *user_data)
	{
		return getClipBoard();
	}

	static Image *fontImage;
	void initUi()
	{
		ImGuiIO& io = ImGui::GetIO();
		{
			//io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msmincho.ttc", 16, nullptr, io.Fonts->GetGlyphRangesJapanese());
			//static const ImWchar icons_ranges[] = { 
			//	ICON_MIN_FA, 
			//	ICON_MAX_FA, 
			//	0 
			//};
			//ImFontConfig icons_config; 
			//icons_config.MergeMode = true; 
			//icons_config.PixelSnapH = true;
			//io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/fontawesome-webfont.ttf", 16.0f, &icons_config, icons_ranges);
			unsigned char* pixels; int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
			fontImage = new Image(width, height, VK_FORMAT_R8G8B8A8_UNORM, 
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, false);
			fontImage->fillData(0, pixels, width * height * 4);
			io.Fonts->TexID = (void*)0; // image index

			plainPipeline_2d->descriptorSet->setImage(0, 0, fontImage, colorSampler);
		}

		ui_cb = new CommandBuffer;
		ui_cb->begin();
		ui_cb->end();
		
		io.KeyMap[ImGuiKey_Tab] = VK_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
		io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
		io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
		io.KeyMap[ImGuiKey_Home] = VK_HOME;
		io.KeyMap[ImGuiKey_End] = VK_END;
		io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
		io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
		io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
		io.KeyMap[ImGuiKey_A] = 'A';
		io.KeyMap[ImGuiKey_C] = 'C';
		io.KeyMap[ImGuiKey_V] = 'V';
		io.KeyMap[ImGuiKey_X] = 'X';
		io.KeyMap[ImGuiKey_Y] = 'Y';
		io.KeyMap[ImGuiKey_Z] = 'Z';
		io.RenderDrawListsFn = _gui_renderer;
		io.SetClipboardTextFn = _SetClipboardCallback;
		io.GetClipboardTextFn = _GetClipboardCallback;
	}

	void ui_onKeyDown(int k)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[k] = true;

		io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
		io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
		io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
		io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
	}

	void ui_onKeyUp(int k)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[k] = false;

		io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
		io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
		io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
		io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
	}

	void ui_onChar(int c)
	{
		if (c == VK_TAB)
			return;

		ImGuiIO& io = ImGui::GetIO();
		if (c > 0 && c < 0x10000)
			io.AddInputCharacter((unsigned short)c);
	}

	void beginUi(bool _need_clear)
	{
		static int last_time = 0;
		if (last_time == 0) last_time = nowTime;

		need_clear = _need_clear;

		uiAcceptedMouse = false;
		uiAcceptedKey = false;

		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize = ImVec2((float)window_cx, (float)window_cy);
		io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

		io.DeltaTime = (float)((nowTime - last_time) / 1000.f);
		last_time = nowTime;

		io.MousePos = ImVec2((float)mouseX, (float)mouseY);

		io.MouseDown[0] = mouseLeft.pressing;
		io.MouseDown[1] = mouseRight.pressing;
		io.MouseDown[2] = mouseMiddle.pressing;

		io.MouseWheel = mouseScroll / 120;

		ImGui::NewFrame();
	}

	void endUi()
	{
		ImGui::Render();

		uiAcceptedMouse = ImGui::IsMouseHoveringAnyWindow();
		uiAcceptedKey = ImGui::IsAnyItemActive();
	}

	static std::vector<Image*> _images;

	void addUiImage(Image *image)
	{
		_images.push_back(image);

		for (int index = 0; index < _images.size(); index++)
		{
			_images[index]->index = index + 1;
			plainPipeline_2d->descriptorSet->setImage(0, _images[index]->index, _images[index], colorSampler);
		}
	}

	void removeUiImage(Image *image)
	{
		for (auto it = _images.begin(); it != _images.end(); it++)
		{
			if (*it == image)
			{
				_images.erase(it);
				break;
			}
		}

		for (int index = 0; index < _images.size(); index++)
		{
			_images[index]->index = index + 1;
			plainPipeline_2d->descriptorSet->setImage(0, _images[index]->index, _images[index], colorSampler);
		}
	}
}
