#include "gui.h"
#include "window.h"
#include "render.h"
#include "core.h"

namespace tke
{
	GuiWindow *guiCurrentWindow;

	void DialogT::begin()
	{
		if (state == State::eNeedOpen)
		{
			state = State::eOpened;
			ImGui::OpenPopup(name.c_str());
		}
	}

	void DialogT::end()
	{
		if (state == State::eOpened)
		{
			state = State::eClosed;
			ImGui::CloseCurrentPopup();
		}
	}

	YesNoDialog::YesNoDialog()
	{
		name = "YesNoDialog";
	}

	void YesNoDialog::start(const std::string &text, void(*callback)(bool))
	{
		m_text = text;
		m_callback = callback;
		state = State::eNeedOpen;
	}

	void YesNoDialog::show()
	{
		begin();
		if (state != State::eOpened)
			return;

		if (ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			guiCurrentWindow->m_uiAcceptedMouse = true;
			guiCurrentWindow->m_uiAcceptedKey = true;

			ImGui::Text(m_text.c_str());
			if (ImGui::Button("Yes", ImVec2(120, 0)))
			{
				m_callback(true);
				end();
			}
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(120, 0)))
			{
				m_callback(false);
				end();
			}
			ImGui::EndPopup();
		}
	}

	MessageDialog::MessageDialog()
	{
		name = "MessageDialog";
	}

	void MessageDialog::add(const std::string &text)
	{
		texts.push_back(text);
	}

	void MessageDialog::show()
	{
		if (texts.size() > 0)
			state = State::eNeedOpen;

		begin();
		if (state != State::eOpened)
			return;

		if (ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			guiCurrentWindow->m_uiAcceptedMouse = true;
			guiCurrentWindow->m_uiAcceptedKey = true;

			auto ok = false;
			if (ImGui::Button("Ok##0", ImVec2(120, 0)))
				ok = true;
			ImGui::TextUnformatted(texts[0].c_str());
			if (ImGui::Button("Ok##1", ImVec2(120, 0)))
				ok = true;
			if (ok)
			{
				texts.erase(texts.begin());
				end();
			}
			ImGui::EndPopup();
		}
	}

	InputDialog::InputDialog()
	{
		name = "InputDialog";
	}

	void InputDialog::start(void(*callback)(const std::string &))
	{
		m_callback = callback;
		m_buf[0] = 0;

		state = State::eNeedOpen;
	}

	void InputDialog::show()
	{
		begin();
		if (state != State::eOpened)
			return;

		if (ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			guiCurrentWindow->m_uiAcceptedMouse = true;
			guiCurrentWindow->m_uiAcceptedKey = true;

			ImGui::InputText("Input:", m_buf, MAX_PATH);
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				m_callback(m_buf);
				end();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
				end();
			ImGui::EndPopup();
		}
	}

	void FileDialogT::search()
	{
		m_dirs.clear();
		m_files.clear();
		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile((m_path + "*.*").c_str(), &fd);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			return;
		}
		for (;;)
		{
			if (fd.cFileName[0] != '.')
			{
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					m_dirs.push_back(fd.cFileName);
				else
					m_files.push_back(fd.cFileName);
			}
			if (!FindNextFile(hFind, &fd))
				break;
		}
		FindClose(hFind);
	}

	DirectoryDialog::DirectoryDialog()
	{
		name = "DirectoryDialog";
	}

	void DirectoryDialog::search()
	{
		m_dirs.clear();
		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile((m_path + "*.*").c_str(), &fd);
		if (INVALID_HANDLE_VALUE == hFind)
			return;
		for (;;)
		{
			if (fd.cFileName[0] != '.')
			{
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					m_dirs.push_back(fd.cFileName);
			}
			if (!FindNextFile(hFind, &fd))
				break;
		}
		FindClose(hFind);
	}

	void DirectoryDialog::start(void(*callback)(const std::string &))
	{
		m_path.clear();
		search();
		m_callback = callback;

		state = State::eNeedOpen;
	}

	void DirectoryDialog::show()
	{
		begin();
		if (state != State::eOpened)
			return;

		bool changeDir = false;
		if (ImGui::BeginPopupModal("DirectoryDialog", nullptr, 0))
		{
			guiCurrentWindow->m_uiAcceptedMouse = true;
			guiCurrentWindow->m_uiAcceptedKey = true;

			ImGui::Text("Path:%s*.*", m_path.c_str());
			ImGui::SameLine();
			if (ImGui::Button("Up"))
			{
				m_path += "../";
				search();
			}
			if (ImGui::Button("Home"))
			{
				m_path.clear();
				search();
			}
			for (auto d : m_dirs)
			{
				if (ImGui::Selectable(d.c_str(), false, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick))
				{
					if (ImGui::IsMouseDoubleClicked(0))
					{
						m_path += d + "/";
						changeDir = true;
					}
					m_currentPath = d;
				}
			}
			ImGui::Text("Name:", m_currentPath);
			if (ImGui::Button("Ok", ImVec2(120, 0)))
			{
				m_callback(m_path + m_currentPath);
				end();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
				end();

			ImGui::EndPopup();
		}
		if (state != State::eClosed && changeDir)
			search();
	}

	OpenFileDialog::OpenFileDialog()
	{
		name = "OpenFileDialog";
	}

	void OpenFileDialog::start(void(*callback)(const std::string &))
	{
		m_path.clear();
		search();
		m_callback = callback;

		state = State::eNeedOpen;
	}

	void OpenFileDialog::show()
	{
		begin();
		if (state != State::eOpened)
			return;

		bool changeDir = false;
		if (ImGui::BeginPopupModal("OpenFileDialog", NULL, 0))
		{
			guiCurrentWindow->m_uiAcceptedMouse = true;
			guiCurrentWindow->m_uiAcceptedKey = true;

			ImGui::Text("Path:%s*.*", m_path.c_str());
			ImGui::SameLine();
			if (ImGui::Button("Up"))
			{
				m_path += "../";
				search();
			}
			if (ImGui::Button("Home"))
			{
				m_path.clear();
				search();
			}
			for (auto d : m_dirs)
			{
				if (ImGui::Selectable(("+" + d).c_str(), false, ImGuiSelectableFlags_DontClosePopups))
				{
					m_path += d + "/";
					changeDir = true;
				}
			}
			for (auto f : m_files)
			{
				if (ImGui::Selectable(f.c_str()))
				{
					m_callback(m_path + f);
					end();
				}
			}
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
				end();

			ImGui::EndPopup();
		}
		if (state != State::eClosed && changeDir)
			search();
	}

	SaveFileDialog::SaveFileDialog()
	{
		name = "SaveFileDialog";
	}

	void SaveFileDialog::start(void(*callback)(const std::string &))
	{
		m_path.clear();
		search();
		m_callback = callback;

		state = State::eNeedOpen;
	}

	void SaveFileDialog::show()
	{
		begin();
		if (state != State::eOpened)
			return;

		bool changeDir = false;
		if (ImGui::BeginPopupModal("SaveFileDialog", nullptr, 0))
		{
			guiCurrentWindow->m_uiAcceptedMouse = true;
			guiCurrentWindow->m_uiAcceptedKey = true;

			ImGui::Text("Path:%s*.*", m_path.c_str());
			ImGui::SameLine();
			if (ImGui::Button("Up"))
			{
				m_path += "../";
				search();
			}
			if (ImGui::Button("Home"))
			{
				m_path.clear();
				search();
			}
			for (auto d : m_dirs)
			{
				if (ImGui::Selectable(("+" + d).c_str(), false, ImGuiSelectableFlags_DontClosePopups))
				{
					m_path += d + "/";
					changeDir = true;
				}
			}
			for (auto f : m_files)
			{
				if (ImGui::Selectable(f.c_str(), false, ImGuiSelectableFlags_DontClosePopups))
					strcpy(filename, f.c_str());
			}
			ImGui::InputText("Name", filename, 50);
			if (ImGui::Button("Ok", ImVec2(120, 0)))
			{
				m_callback(m_path + filename);
				end();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
				end();

			ImGui::EndPopup();
		}
		if (state != State::eClosed && changeDir)
			search();
	}


	void Dialogs::show()
	{
		yesNoDialog.show();
		messageDialog.show();
		inputDialog.show();
		directoryDialog.show();
		openFileDialog.show();
		saveFileDialog.show();
	}

	static CRITICAL_SECTION cs;

	static Image g_FontImage;

	static Pipeline g_Pipeline;

	static void _SetClipboardCallback(void *user_data, const char *s)
	{
		setClipBoard(s);
	}

	static const char *_GetClipboardCallback(void *user_data)
	{
		return getClipBoard();
	}

	static void _guiRenderer(ImDrawData* draw_data)
	{
		ImGuiIO& io = ImGui::GetIO();
		if ((int)(io.DisplaySize.x * io.DisplayFramebufferScale.x) == 0 || (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y) == 0)
			return;
		draw_data->ScaleClipRects(io.DisplayFramebufferScale);

		vkResetCommandBuffer(guiCurrentWindow->m_uiCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = guiCurrentWindow->m_uiRenderPass;
		inheritanceInfo.subpass = guiCurrentWindow->m_uiSubpassIndex;
		inheritanceInfo.framebuffer = guiCurrentWindow->m_uiFramebuffer;

		vk::beginCommandBuffer(guiCurrentWindow->m_uiCommandBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, &inheritanceInfo);

		// Create the Vertex Buffer:
		static VertexBuffer	g_VertexBuffer;
		size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
		if (!g_VertexBuffer.m_buffer || g_VertexBuffer.m_size < vertex_size)
		{
			g_VertexBuffer.destory();
			g_VertexBuffer.create(vertex_size);
		}

		// Create the Index Buffer:
		static IndexBuffer g_IndexBuffer;
		size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
		if (!g_IndexBuffer.m_buffer || g_IndexBuffer.m_size < index_size)
		{
			g_IndexBuffer.destory();
			g_IndexBuffer.create(index_size);
		}

		static StagingBuffer g_StagingBuffer;
		auto totalSize = vertex_size + index_size;
		if (g_StagingBuffer.m_size < totalSize)
		{
			g_StagingBuffer.destory();
			g_StagingBuffer.create(totalSize);
		}

		// Upload Vertex and index Data:
		{
			auto map = vk::mapMemory(g_StagingBuffer.m_memory, 0, totalSize);
			auto vtx_dst = (ImDrawVert*)map;
			auto idx_dst = (ImDrawIdx*)((char*)map + vertex_size);
			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
				memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
				vtx_dst += cmd_list->VtxBuffer.Size;
				idx_dst += cmd_list->IdxBuffer.Size;
			}
			vk::unmapMemory(g_StagingBuffer.m_memory);

			vk::copyBuffer(g_StagingBuffer.m_buffer, g_VertexBuffer.m_buffer, vertex_size, 0, 0);
			vk::copyBuffer(g_StagingBuffer.m_buffer, g_IndexBuffer.m_buffer, index_size, vertex_size, 0);
		}

		// Bind Vertex And Index Buffer:
		{
			VkBuffer vertex_buffers[1] = { g_VertexBuffer.m_buffer };
			VkDeviceSize vertex_offset[1] = { 0 };
			vkCmdBindVertexBuffers(guiCurrentWindow->m_uiCommandBuffer, 0, 1, vertex_buffers, vertex_offset);
			vkCmdBindIndexBuffer(guiCurrentWindow->m_uiCommandBuffer, g_IndexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT16);
		}

		// Setup scale and translation:
		{
			auto v = glm::vec4(2.f / io.DisplaySize.x, 2.f / io.DisplaySize.y, -1.f, -1.f);
			vkCmdPushConstants(guiCurrentWindow->m_uiCommandBuffer, g_Pipeline.m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec4), &v);
		}

		// Bind pipeline and descriptor sets:
		{
			vkCmdBindPipeline(guiCurrentWindow->m_uiCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, guiCurrentWindow->m_uiPipeline);
			vkCmdBindDescriptorSets(guiCurrentWindow->m_uiCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_Pipeline.m_pipelineLayout, 0, 1, &g_Pipeline.m_descriptorSet, 0, NULL);
		}

		// Render the command lists:

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
					VkRect2D scissor;
					scissor.offset.x = (int32_t)(pcmd->ClipRect.x);
					scissor.offset.y = (int32_t)(pcmd->ClipRect.y);
					scissor.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
					scissor.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y + 1); // TODO: + 1??????
					vkCmdSetScissor(guiCurrentWindow->m_uiCommandBuffer, 0, 1, &scissor);
					vkCmdDrawIndexed(guiCurrentWindow->m_uiCommandBuffer, pcmd->ElemCount, 1, idx_offset, vtx_offset, (int)pcmd->TextureId);
				}
				idx_offset += pcmd->ElemCount;
			}
			vtx_offset += cmd_list->VtxBuffer.Size;
		}

		vkEndCommandBuffer(guiCurrentWindow->m_uiCommandBuffer);
	}

	GuiWindow::GuiWindow(int cx, int cy, const char *title, unsigned int windowStyle, unsigned int windowStyleEx, bool hasFrame)
		: Window(cx, cy, title, windowStyle, windowStyleEx, hasFrame)
	{
		ready = false;
		m_uiCommandBuffer = vk::allocateSecondaryCommandBuffer();
	}

	void GuiWindow::keyDownEvent(int wParam)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[wParam] = true;

		io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
		io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
		io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
		io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
	}

	void GuiWindow::keyUpEvent(int wParam)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[wParam] = false;

		io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
		io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
		io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
		io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
	}

	void GuiWindow::charEvent(int wParam)
	{
		if (wParam == VK_TAB)
			return;
		ImGuiIO& io = ImGui::GetIO();
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
	}

	void GuiWindow::initUi(VkRenderPass uiRenderPass, uint32_t uiSubpassIndex)
	{
		assert(!ready);
		m_uiRenderPass = uiRenderPass;
		m_uiSubpassIndex = uiSubpassIndex;
		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = m_uiRenderPass;
		inheritanceInfo.subpass = m_uiSubpassIndex;
		vk::beginCommandBuffer(m_uiCommandBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, &inheritanceInfo);
		vkEndCommandBuffer(m_uiCommandBuffer);
		{
			guiCurrentWindow = this;
			m_uiPipeline = g_Pipeline.getPipeline(m_cx, m_cy, m_uiRenderPass, m_uiSubpassIndex);
			m_uiDialogs = new Dialogs;

			static bool first = true;

			ImGuiContext *lastContext = nullptr;
			if (!first)
			{
				first = false;
				auto context = ImGui::CreateContext();
				lastContext = ImGui::GetCurrentContext();
				ImGui::SetCurrentContext(context);
			}

			ImGuiIO& io = ImGui::GetIO();
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
			io.RenderDrawListsFn = _guiRenderer;
			io.SetClipboardTextFn = _SetClipboardCallback;
			io.GetClipboardTextFn = _GetClipboardCallback;

			m_uiContext = ImGui::GetCurrentContext();
			if (!lastContext) ImGui::SetCurrentContext(lastContext);
		}
		ready = true;
	}

	void GuiWindow::lockUi()
	{
		EnterCriticalSection(&cs);

		guiCurrentWindow = this;

		m_uiAcceptedMouse = false;
		m_uiAcceptedKey = false;

		ImGui::SetCurrentContext((ImGuiContext*)m_uiContext);

		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize = ImVec2((float)m_cx, (float)m_cy);
		io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

		static double g_Time = 0.0;
		double current_time = 0.0;
		io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
		g_Time = current_time;

		if (focus)
			io.MousePos = ImVec2((float)mouseX, (float)mouseY);
		else
			io.MousePos = ImVec2(-1, -1);

		io.MouseDown[0] = leftPressing;
		io.MouseDown[1] = rightPressing;
		io.MouseDown[2] = middlePressing;

		io.MouseWheel = mouseScroll / 120;

		ImGui::NewFrame();
	}

	void GuiWindow::unlockUi()
	{
		ImGui::Render();

		m_uiAcceptedMouse = ImGui::IsMouseHoveringAnyWindow();
		m_uiAcceptedKey = ImGui::IsAnyItemActive();

		LeaveCriticalSection(&cs);
	}

	std::vector<Image*> _icons;
	void guiPushIcon(Image *image)
	{
		_icons.push_back(image);
	}

	void guiSetupIcons(VkSampler sampler)
	{
		static int texture_position = -1;
		if (texture_position == -1) texture_position = g_Pipeline.descriptorPosition("sTexture");

		std::vector<VkWriteDescriptorSet> writes;
		writes.push_back(vk::writeDescriptorSet(g_Pipeline.m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture_position, g_FontImage.getInfo(sampler), 0));
		auto imageID = 1;
		for (auto image : _icons)
		{
			writes.push_back(vk::writeDescriptorSet(g_Pipeline.m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture_position, image->getInfo(sampler), imageID));
			imageID++;
		}
		vk::updataDescriptorSet(writes.size(), writes.data());
	}

	VkVertexInputBindingDescription binding_desc[1] = {};
	VkVertexInputAttributeDescription attribute_desc[3] = {};
	VkPipelineVertexInputStateCreateInfo vertex_info = {};

	void initGui()
	{
		static bool first = true;
		if (!first) return;
		first = false;

		InitializeCriticalSection(&cs);

		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		g_FontImage.create(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, pixels, width * height * 4);
		io.Fonts->TexID = (void*)0;

		binding_desc[0].stride = sizeof(ImDrawVert);
		binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		attribute_desc[0].location = 0;
		attribute_desc[0].binding = binding_desc[0].binding;
		attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_desc[0].offset = (size_t)(&((ImDrawVert*)0)->pos);
		attribute_desc[1].location = 1;
		attribute_desc[1].binding = binding_desc[0].binding;
		attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_desc[1].offset = (size_t)(&((ImDrawVert*)0)->uv);
		attribute_desc[2].location = 2;
		attribute_desc[2].binding = binding_desc[0].binding;
		attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
		attribute_desc[2].offset = (size_t)(&((ImDrawVert*)0)->col);

		vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_info.vertexBindingDescriptionCount = 1;
		vertex_info.pVertexBindingDescriptions = binding_desc;
		vertex_info.vertexAttributeDescriptionCount = 3;
		vertex_info.pVertexAttributeDescriptions = attribute_desc;

		g_Pipeline.m_dynamics.push_back(VK_DYNAMIC_STATE_SCISSOR);
		g_Pipeline.m_pVertexInputState = &vertex_info;
		g_Pipeline.setFilename("../pipeline/ui/ui.xml");
		g_Pipeline.loadXML();
		g_Pipeline.getLayout();
		g_Pipeline.reallocateDescriptorSet();
	}
}
