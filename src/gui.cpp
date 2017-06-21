#include <process.h>

#include "gui.h"
#include "core.h"

namespace tke
{
	static thread_local Window *current_window;
	static bool need_clear = false;

	bool uiAcceptedMouse = false;
	bool uiAcceptedKey = false;

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
			uiAcceptedMouse = true;
			uiAcceptedKey = true;

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
			uiAcceptedMouse = true;
			uiAcceptedKey = true;

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
			uiAcceptedMouse = true;
			uiAcceptedKey = true;

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
			uiAcceptedMouse = true;
			uiAcceptedKey = true;

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
			uiAcceptedMouse = true;
			uiAcceptedKey = true;

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
			uiAcceptedMouse = true;
			uiAcceptedKey = true;

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

	YesNoDialog yesNoDialog;
	MessageDialog messageDialog;
	InputDialog inputDialog;
	DirectoryDialog directoryDialog;
	OpenFileDialog openFileDialog;
	SaveFileDialog saveFileDialog;

	void showDialogs()
	{
		yesNoDialog.show();
		messageDialog.show();
		inputDialog.show();
		directoryDialog.show();
		openFileDialog.show();
		saveFileDialog.show();
	}

	static Pipeline *pipeline = nullptr;
	static int texture_position = -1;

	static void _gui_renderer(ImDrawData* draw_data)
	{
		ImGuiIO& io = ImGui::GetIO();
		if ((int)(io.DisplaySize.x * io.DisplayFramebufferScale.x) == 0 || (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y) == 0)
			return;
		draw_data->ScaleClipRects(io.DisplayFramebufferScale);

		static VertexBuffer	*vertexBuffer = nullptr;
		size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
		if (!vertexBuffer || vertexBuffer->m_size < vertex_size)
		{
			if (vertexBuffer) delete vertexBuffer;
			vertexBuffer = new VertexBuffer(vertex_size);
		}

		static IndexBuffer *indexBuffer = nullptr;
		size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
		if (!indexBuffer || indexBuffer->m_size < index_size)
		{
			if (indexBuffer) delete indexBuffer;
			indexBuffer = new IndexBuffer(index_size);
		}

		static StagingBuffer *stagingBuffer = nullptr;
		auto totalSize = vertex_size + index_size;
		if (!stagingBuffer || stagingBuffer->m_size < totalSize)
		{
			if (stagingBuffer) delete stagingBuffer;
			stagingBuffer = new StagingBuffer(totalSize);
		}

		{
			auto map = stagingBuffer->map(0, totalSize);
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
			stagingBuffer->unmap();

			commandPool.cmdCopyBuffer(stagingBuffer->m_buffer, vertexBuffer->m_buffer, vertex_size, 0, 0);
			commandPool.cmdCopyBuffer(stagingBuffer->m_buffer, indexBuffer->m_buffer, index_size, vertex_size, 0);
		}

		auto cmd = current_window->ui->cmd;

		vkResetCommandBuffer(cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		beginCommandBuffer(cmd);

		if (current_window->events.size() > 0)
			vkCmdWaitEvents(cmd, current_window->events.size(), current_window->events.data(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, nullptr, 0, nullptr, 0, nullptr);

		VkClearValue clear_value = { current_window->ui->bkColor.r, current_window->ui->bkColor.g, current_window->ui->bkColor.b };
		vkCmdBeginRenderPass(cmd, &renderPassBeginInfo(need_clear ? plainRenderPass_clear : plainRenderPass, 
			current_window->framebuffers[current_window->imageIndex]->v, current_window->cx, current_window->cy, need_clear ? 1 : 0, need_clear ? &clear_value : nullptr), VK_SUBPASS_CONTENTS_INLINE);

		cmdSetViewportAndScissor(cmd, current_window->cx, current_window->cy);

		VkDeviceSize vertex_offset[1] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer->m_buffer, vertex_offset);
		vkCmdBindIndexBuffer(cmd, indexBuffer->m_buffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdPushConstants(cmd, pipeline->m_pipelineLayout->v, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec4), &glm::vec4(2.f / io.DisplaySize.x, 2.f / io.DisplaySize.y, -1.f, -1.f));

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->m_pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->m_pipelineLayout->v, 0, 1, &pipeline->m_descriptorSet, 0, NULL);

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
					scissor.offset.x = ImMax((int32_t)(pcmd->ClipRect.x), 0);
					scissor.offset.y = ImMax((int32_t)(pcmd->ClipRect.y), 0);
					scissor.extent.width = ImMax((uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x), 0);
					scissor.extent.height = ImMax((uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y + 1), 0); // TODO: + 1??????
					vkCmdSetScissor(cmd, 0, 1, &scissor);
					vkCmdDrawIndexed(cmd, pcmd->ElemCount, 1, idx_offset, vtx_offset, (int)pcmd->TextureId);
				}
				idx_offset += pcmd->ElemCount;
			}
			vtx_offset += cmd_list->VtxBuffer.Size;
		}

		vkCmdEndRenderPass(cmd);

		for (auto &e : current_window->events)
			vkCmdResetEvent(cmd, e, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		vkEndCommandBuffer(cmd);
	}

	static void _SetClipboardCallback(void *user_data, const char *s)
	{
		setClipBoard(s);
	}

	static const char *_GetClipboardCallback(void *user_data)
	{
		return getClipBoard();
	}

	GuiComponent::GuiComponent(Window *_window)
		:window(_window)
	{
		static bool first = true;
		if (first)
		{
			first = false;

			static VkVertexInputBindingDescription binding_desc[] = {
				{ 0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX }
			};
			static VkVertexInputAttributeDescription attribute_desc[3] = {
				{ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos) },
				{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv) },
				{ 2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col) }
			};
			static VkPipelineVertexInputStateCreateInfo vertex_info = {
				VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				nullptr,
				0,
				ARRAYSIZE(binding_desc),
				binding_desc,
				ARRAYSIZE(attribute_desc),
				attribute_desc
			};

			pipeline = new Pipeline;
			pipeline->loadXML(enginePath + "pipeline/ui/ui.xml");
			pipeline->m_pVertexInputState = &vertex_info;
			pipeline->m_dynamics.push_back(VK_DYNAMIC_STATE_SCISSOR);
			pipeline->setup(plainRenderPass, 0);

			context = ImGui::GetCurrentContext();

			ImGuiIO& io = ImGui::GetIO();
			unsigned char* pixels;
			int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
			auto fontImage = new Image(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, pixels, width * height * 4);
			io.Fonts->TexID = (void*)0; // image index

			if (texture_position == -1) texture_position = pipeline->descriptorPosition("sTexture");

			descriptorPool.addWrite(pipeline->m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture_position, fontImage->getInfo(colorSampler));

			descriptorPool.update();

		}
		else
		{
			context = ImGui::CreateContext();
		}

		cmd = commandPool.allocate();
		beginCommandBuffer(cmd);
		vkEndCommandBuffer(cmd);

		ImGui::SetCurrentContext(context);

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
		io.RenderDrawListsFn = _gui_renderer;
		io.SetClipboardTextFn = _SetClipboardCallback;
		io.GetClipboardTextFn = _GetClipboardCallback; 

		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.30f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.80f, 0.80f, 0.80f, 0.40f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.90f, 0.90f, 0.45f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.54f, 0.54f, 0.54f, 0.83f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.40f, 0.20f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.63f, 0.63f, 0.63f, 0.87f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.40f, 0.40f, 0.40f, 0.80f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.25f, 0.25f, 0.25f, 0.60f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.40f, 0.30f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 0.40f);
		style.Colors[ImGuiCol_ComboBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.99f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.40f, 0.40f, 0.40f, 0.60f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.40f, 0.40f, 0.40f, 0.45f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.45f, 0.45f, 0.80f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.53f, 0.53f, 0.53f, 0.80f);
		style.Colors[ImGuiCol_Column] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.60f, 0.60f, 0.60f, 0.60f);
		style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}

	void GuiComponent::onKeyDown(int k)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[k] = true;

		io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
		io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
		io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
		io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
	}

	void GuiComponent::onKeyUp(int k)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[k] = false;

		io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
		io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
		io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
		io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
	}

	void GuiComponent::onChar(int c)
	{
		if (c == VK_TAB)
			return;
		ImGuiIO& io = ImGui::GetIO();
		if (c > 0 && c < 0x10000)
			io.AddInputCharacter((unsigned short)c);
	}

	void GuiComponent::begin(bool _need_clear)
	{
		static int last_time = 0;
		if (last_time == 0) last_time = nowTime;

		current_window = window;
		need_clear = _need_clear;

		uiAcceptedMouse = false;
		uiAcceptedKey = false;

		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize = ImVec2((float)window->cx, (float)window->cy);
		io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

		io.DeltaTime = (float)((nowTime - last_time) / 1000.f);
		last_time = nowTime;

		io.MousePos = ImVec2((float)window->mouseX, (float)window->mouseY);

		io.MouseDown[0] = window->leftPressing;
		io.MouseDown[1] = window->rightPressing;
		io.MouseDown[2] = window->middlePressing;

		io.MouseWheel = window->mouseScroll / 120;

		ImGui::NewFrame();
	}

	void GuiComponent::end()
	{
		ImGui::Render();

		uiAcceptedMouse = ImGui::IsMouseHoveringAnyWindow();
		uiAcceptedKey = ImGui::IsAnyItemActive();
	}

	static std::vector<Image*> _images;

	static void _update_descriptor_set()
	{
		for (int index = 0; index < _images.size(); index++)
		{
			_images[index]->index = index + 1;
			descriptorPool.addWrite(pipeline->m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture_position, _images[index]->getInfo(colorSampler), _images[index]->index);
		}

		descriptorPool.update();
	}

	void addGuiImage(Image *image)
	{
		_images.push_back(image);

		_update_descriptor_set();
	}

	void removeGuiImage(Image *image)
	{
		for (auto it = _images.begin(); it != _images.end(); it++)
		{
			if (*it == image)
			{
				_images.erase(it);
				break;
			}
		}

		_update_descriptor_set();
	}
}

namespace ImGui
{

	struct DockContext
	{
		enum Slot_
		{
			Slot_Left,
			Slot_Right,
			Slot_Top,
			Slot_Bottom,
			Slot_Tab,

			Slot_Float,
			Slot_None
		};

		enum EndAction_
		{
			EndAction_None,
			EndAction_Panel,
			EndAction_End,
			EndAction_EndChild
		};

		enum Status_
		{
			Status_Docked,
			Status_Float,
			Status_Dragged
		};

		struct Dock
		{
			std::string label;
			ImU32 id = 0;
			Dock* next_tab = nullptr;
			Dock* prev_tab = nullptr;
			Dock* children[2] = {};
			Dock* parent = nullptr;
			bool active = true;
			ImVec2 pos = ImVec2(0, 0);
			ImVec2 size = ImVec2(-1, -1);
			Status_ status = Status_Float;
			int last_frame;
			int invalid_frames;
			char location[16] = {};
			bool opened = false;
			bool first = true;

			ImVec2 getMinSize() const
			{
				if (!children[0]) return ImVec2(16, 16 + GetTextLineHeightWithSpacing());

				ImVec2 s0 = children[0]->getMinSize();
				ImVec2 s1 = children[1]->getMinSize();
				return isHorizontal() ? ImVec2(s0.x + s1.x, ImMax(s0.y, s1.y))
					: ImVec2(ImMax(s0.x, s1.x), s0.y + s1.y);
			}

			bool isHorizontal() const
			{
				return children[0]->pos.x < children[1]->pos.x;
			}

			void setParent(Dock* dock)
			{
				parent = dock;
				for (Dock* tmp = prev_tab; tmp; tmp = tmp->prev_tab) tmp->parent = dock;
				for (Dock* tmp = next_tab; tmp; tmp = tmp->next_tab) tmp->parent = dock;
			}

			Dock& getSibling()
			{
				IM_ASSERT(parent);
				if (parent->children[0] == &getFirstTab()) return *parent->children[1];
				return *parent->children[0];
			}

			Dock& getFirstTab()
			{
				Dock* tmp = this;
				while (tmp->prev_tab) tmp = tmp->prev_tab;
				return *tmp;
			}

			void setActive()
			{
				active = true;
				for (Dock* tmp = prev_tab; tmp; tmp = tmp->prev_tab) tmp->active = false;
				for (Dock* tmp = next_tab; tmp; tmp = tmp->next_tab) tmp->active = false;
			}


			bool isContainer() const { return children[0] != nullptr; }

			void setChildrenPosSize(const ImVec2& _pos, const ImVec2& _size)
			{
				ImVec2 s = children[0]->size;
				if (isHorizontal())
				{
					s.y = _size.y;
					s.x = (float)int(
						_size.x * children[0]->size.x / (children[0]->size.x + children[1]->size.x));
					if (s.x < children[0]->getMinSize().x)
					{
						s.x = children[0]->getMinSize().x;
					}
					else if (_size.x - s.x < children[1]->getMinSize().x)
					{
						s.x = _size.x - children[1]->getMinSize().x;
					}
					children[0]->setPosSize(_pos, s);

					s.x = _size.x - children[0]->size.x;
					ImVec2 p = _pos;
					p.x += children[0]->size.x;
					children[1]->setPosSize(p, s);
				}
				else
				{
					s.x = _size.x;
					s.y = (float)int(
						_size.y * children[0]->size.y / (children[0]->size.y + children[1]->size.y));
					if (s.y < children[0]->getMinSize().y)
					{
						s.y = children[0]->getMinSize().y;
					}
					else if (_size.y - s.y < children[1]->getMinSize().y)
					{
						s.y = _size.y - children[1]->getMinSize().y;
					}
					children[0]->setPosSize(_pos, s);

					s.y = _size.y - children[0]->size.y;
					ImVec2 p = _pos;
					p.y += children[0]->size.y;
					children[1]->setPosSize(p, s);
				}
			}

			void setPosSize(const ImVec2& _pos, const ImVec2& _size)
			{
				size = _size;
				pos = _pos;
				for (Dock* tmp = prev_tab; tmp; tmp = tmp->prev_tab)
				{
					tmp->size = _size;
					tmp->pos = _pos;
				}
				for (Dock* tmp = next_tab; tmp; tmp = tmp->next_tab)
				{
					tmp->size = _size;
					tmp->pos = _pos;
				}

				if (!isContainer()) return;
				setChildrenPosSize(_pos, _size);
			}
		};

		ImVector<Dock*> m_docks;
		ImVec2 m_drag_offset;
		Dock* m_current = nullptr;
		int m_last_frame = 0;
		EndAction_ m_end_action;

		Dock& getDock(const char* label, bool opened, const ImVec2& default_size)
		{
			ImU32 id = ImHash(label, 0);
			for (int i = 0; i < m_docks.size(); ++i)
			{
				if (m_docks[i]->id == id)
					return *m_docks[i];
			}

			Dock* new_dock = new Dock;
			new(new_dock) Dock();
			m_docks.push_back(new_dock);
			new_dock->label = label;
			assert(new_dock->label != "");
			new_dock->id = id;
			new_dock->setActive();
			new_dock->status = Status_Float;
			new_dock->pos = ImVec2(0, 0);
			new_dock->size.x = default_size.x < 0 ? GetIO().DisplaySize.x : default_size.x;
			new_dock->size.y = default_size.y < 0 ? GetIO().DisplaySize.y : default_size.y;
			new_dock->opened = opened;
			new_dock->first = true;
			new_dock->last_frame = 0;
			new_dock->invalid_frames = 0;
			new_dock->location[0] = 0;
			return *new_dock;
		}

		void putInBackground()
		{
			ImGuiWindow* win = GetCurrentWindow();
			ImGuiContext& g = *GImGui;
			if (g.Windows[0] == win) return;

			for (int i = 0; i < g.Windows.Size; i++)
			{
				if (g.Windows[i] == win)
				{
					for (int j = i - 1; j >= 0; --j)
					{
						g.Windows[j + 1] = g.Windows[j];
					}
					g.Windows[0] = win;
					break;
				}
			}
		}

		void splits()
		{
			if (GetFrameCount() == m_last_frame) return;
			m_last_frame = GetFrameCount();

			putInBackground();

			ImU32 color = GetColorU32(ImGuiCol_Button);
			ImU32 color_hovered = GetColorU32(ImGuiCol_ButtonHovered);
			ImDrawList* draw_list = GetWindowDrawList();
			ImGuiIO& io = GetIO();
			for (int i = 0; i < m_docks.size(); ++i)
			{
				Dock& dock = *m_docks[i];
				if (!dock.isContainer()) continue;

				PushID(i);
				if (!IsMouseDown(0)) dock.status = Status_Docked;

				ImVec2 size = dock.children[0]->size;
				ImVec2 dsize(0, 0);
				SetCursorScreenPos(dock.children[1]->pos);
				ImVec2 min_size0 = dock.children[0]->getMinSize();
				ImVec2 min_size1 = dock.children[1]->getMinSize();
				if (dock.isHorizontal())
				{
					InvisibleButton("split", ImVec2(3, dock.size.y));
					if (dock.status == Status_Dragged) dsize.x = io.MouseDelta.x;
					dsize.x = -ImMin(-dsize.x, dock.children[0]->size.x - min_size0.x);
					dsize.x = ImMin(dsize.x, dock.children[1]->size.x - min_size1.x);
				}
				else
				{
					InvisibleButton("split", ImVec2(dock.size.x, 3));
					if (dock.status == Status_Dragged) dsize.y = io.MouseDelta.y;
					dsize.y = -ImMin(-dsize.y, dock.children[0]->size.y - min_size0.y);
					dsize.y = ImMin(dsize.y, dock.children[1]->size.y - min_size1.y);
				}
				ImVec2 new_size0 = dock.children[0]->size + dsize;
				ImVec2 new_size1 = dock.children[1]->size - dsize;
				ImVec2 new_pos1 = dock.children[1]->pos + dsize;
				dock.children[0]->setPosSize(dock.children[0]->pos, new_size0);
				dock.children[1]->setPosSize(new_pos1, new_size1);

				if (IsItemHovered() && IsMouseClicked(0))
					dock.status = Status_Dragged;

				draw_list->AddRectFilled(
					GetItemRectMin(), GetItemRectMax(), IsItemHovered() ? color_hovered : color);
				PopID();
			}
		}

		void beginPanel()
		{
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_ShowBorders |
				ImGuiWindowFlags_NoBringToFrontOnFocus;
			Dock* root = getRootDock();
			if (root)
			{
				SetNextWindowPos(root->pos);
				SetNextWindowSize(root->size);
			}
			else
			{
				SetNextWindowPos(ImVec2(0, 0));
				SetNextWindowSize(GetIO().DisplaySize);
			}
			PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
			Begin("###DockPanel", nullptr, flags);
			splits();
		}

		void endPanel()
		{
			End();
			PopStyleVar();
		}

		Dock* getDockAt(const ImVec2& pos) const
		{
			for (int i = 0; i < m_docks.size(); ++i)
			{
				Dock& dock = *m_docks[i];
				if (dock.isContainer()) continue;
				if (dock.status != Status_Docked) continue;
				if (IsMouseHoveringRect(dock.pos, dock.pos + dock.size, false))
					return &dock;
			}

			return nullptr;
		}

		static ImRect getDockedRect(const ImRect& rect, Slot_ dock_slot)
		{
			ImVec2 half_size = rect.GetSize() * 0.5f;
			switch (dock_slot)
			{
			default: return rect;
			case Slot_Top: return ImRect(rect.Min, rect.Min + ImVec2(rect.Max.x, half_size.y));
			case Slot_Right: return ImRect(rect.Min + ImVec2(half_size.x, 0), rect.Max);
			case Slot_Bottom: return ImRect(rect.Min + ImVec2(0, half_size.y), rect.Max);
			case Slot_Left: return ImRect(rect.Min, rect.Min + ImVec2(half_size.x, rect.Max.y));
			}
		}

		static ImRect getSlotRect(ImRect parent_rect, Slot_ dock_slot)
		{
			ImVec2 size = parent_rect.Max - parent_rect.Min;
			ImVec2 center = parent_rect.Min + size * 0.5f;
			switch (dock_slot)
			{
			default: return ImRect(center - ImVec2(20, 20), center + ImVec2(20, 20));
			case Slot_Top: return ImRect(center + ImVec2(-20, -50), center + ImVec2(20, -30));
			case Slot_Right: return ImRect(center + ImVec2(30, -20), center + ImVec2(50, 20));
			case Slot_Bottom: return ImRect(center + ImVec2(-20, +30), center + ImVec2(20, 50));
			case Slot_Left: return ImRect(center + ImVec2(-50, -20), center + ImVec2(-30, 20));
			}
		}

		static ImRect getSlotRectOnBorder(ImRect parent_rect, Slot_ dock_slot)
		{
			ImVec2 size = parent_rect.Max - parent_rect.Min;
			ImVec2 center = parent_rect.Min + size * 0.5f;
			switch (dock_slot)
			{
			case Slot_Top:
				return ImRect(ImVec2(center.x - 20, parent_rect.Min.y + 10),
					ImVec2(center.x + 20, parent_rect.Min.y + 30));
			case Slot_Left:
				return ImRect(ImVec2(parent_rect.Min.x + 10, center.y - 20),
					ImVec2(parent_rect.Min.x + 30, center.y + 20));
			case Slot_Bottom:
				return ImRect(ImVec2(center.x - 20, parent_rect.Max.y - 30),
					ImVec2(center.x + 20, parent_rect.Max.y - 10));
			case Slot_Right:
				return ImRect(ImVec2(parent_rect.Max.x - 30, center.y - 20),
					ImVec2(parent_rect.Max.x - 10, center.y + 20));
			default: assert(false);
			}
			IM_ASSERT(false);
			return ImRect();
		}

		Dock* getRootDock()
		{
			for (int i = 0; i < m_docks.size(); ++i)
			{
				if (!m_docks[i]->parent &&
					(m_docks[i]->status == Status_Docked || m_docks[i]->children[0]))
				{
					return m_docks[i];
				}
			}
			return nullptr;
		}

		bool dockSlots(Dock& dock, Dock* dest_dock, const ImRect& rect, bool on_border)
		{
			ImDrawList* canvas = GetWindowDrawList();
			ImU32 color = GetColorU32(ImGuiCol_Button);
			ImU32 color_hovered = GetColorU32(ImGuiCol_ButtonHovered);
			ImVec2 mouse_pos = GetIO().MousePos;
			for (int i = 0; i < (on_border ? 4 : 5); ++i)
			{
				ImRect r =
					on_border ? getSlotRectOnBorder(rect, (Slot_)i) : getSlotRect(rect, (Slot_)i);
				bool hovered = r.Contains(mouse_pos);
				canvas->AddRectFilled(r.Min, r.Max, hovered ? color_hovered : color);
				if (!hovered) continue;

				if (!IsMouseDown(0))
				{
					doDock(dock, dest_dock ? dest_dock : getRootDock(), (Slot_)i);
					return true;
				}
				ImRect docked_rect = getDockedRect(rect, (Slot_)i);
				canvas->AddRectFilled(docked_rect.Min, docked_rect.Max, GetColorU32(ImGuiCol_Button));
			}
			return false;
		}

		void handleDrag(Dock& dock)
		{
			Dock* dest_dock = getDockAt(GetIO().MousePos);

			Begin("##Overlay",
				NULL,
				ImVec2(0, 0),
				0.f,
				ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_AlwaysAutoResize);
			ImDrawList* canvas = ImGui::GetWindowDrawList();

			canvas->PushClipRectFullScreen();

			ImU32 docked_color = GetColorU32(ImGuiCol_FrameBg);
			docked_color = (docked_color & 0x00ffFFFF) | 0x80000000;
			dock.pos = GetIO().MousePos - m_drag_offset;
			if (dest_dock)
			{
				if (dockSlots(dock,
					dest_dock,
					ImRect(dest_dock->pos, dest_dock->pos + dest_dock->size),
					false))
				{
					canvas->PopClipRect();
					End();
					return;
				}
			}
			if (dockSlots(dock, nullptr, ImRect(ImVec2(0, 0), ImGui::GetIO().DisplaySize), true))
			{
				canvas->PopClipRect();
				End();
				return;
			}
			canvas->AddRectFilled(dock.pos, dock.pos + dock.size, docked_color);
			canvas->PopClipRect();

			if (!IsMouseDown(0))
			{
				dock.status = Status_Float;
				dock.location[0] = 0;
				dock.setActive();
			}

			End();
		}

		void fillLocation(Dock& dock)
		{
			if (dock.status == Status_Float) return;
			char* c = dock.location;
			Dock* tmp = &dock;
			while (tmp->parent)
			{
				*c = getLocationCode(tmp);
				tmp = tmp->parent;
				++c;
			}
			*c = 0;
		}

		void doUndock(Dock& dock)
		{
			if (dock.prev_tab)
				dock.prev_tab->setActive();
			else if (dock.next_tab)
				dock.next_tab->setActive();
			else
				dock.active = false;
			Dock* container = dock.parent;

			if (container)
			{
				Dock& sibling = dock.getSibling();
				if (container->children[0] == &dock)
				{
					container->children[0] = dock.next_tab;
				}
				else if (container->children[1] == &dock)
				{
					container->children[1] = dock.next_tab;
				}

				bool remove_container = !container->children[0] || !container->children[1];
				if (remove_container)
				{
					if (container->parent)
					{
						Dock*& child = container->parent->children[0] == container
							? container->parent->children[0]
							: container->parent->children[1];
						child = &sibling;
						child->setPosSize(container->pos, container->size);
						child->setParent(container->parent);
					}
					else
					{
						if (container->children[0])
						{
							container->children[0]->setParent(nullptr);
							container->children[0]->setPosSize(container->pos, container->size);
						}
						if (container->children[1])
						{
							container->children[1]->setParent(nullptr);
							container->children[1]->setPosSize(container->pos, container->size);
						}
					}
					for (int i = 0; i < m_docks.size(); ++i)
					{
						if (m_docks[i] == container)
						{
							m_docks.erase(m_docks.begin() + i);
							break;
						}
					}
					container->~Dock();
					delete(container);
				}
			}
			if (dock.prev_tab) dock.prev_tab->next_tab = dock.next_tab;
			if (dock.next_tab) dock.next_tab->prev_tab = dock.prev_tab;
			dock.parent = nullptr;
			dock.prev_tab = dock.next_tab = nullptr;
		}

		void drawTabbarListButton(Dock& dock)
		{
			if (!dock.next_tab) return;

			ImDrawList* draw_list = GetWindowDrawList();
			if (InvisibleButton("list", ImVec2(16, 16)))
				OpenPopup("tab_list_popup");
			if (BeginPopup("tab_list_popup"))
			{
				Dock* tmp = &dock;
				while (tmp)
				{
					bool dummy = false;
					if (Selectable(tmp->label.c_str(), &dummy))
						tmp->setActive();
					tmp = tmp->next_tab;
				}
				EndPopup();
			}

			bool hovered = IsItemHovered();
			ImVec2 min = GetItemRectMin();
			ImVec2 max = GetItemRectMax();
			ImVec2 center = (min + max) * 0.5f;
			ImU32 text_color = GetColorU32(ImGuiCol_Text);
			ImU32 color_active = GetColorU32(ImGuiCol_FrameBgActive);
			draw_list->AddRectFilled(ImVec2(center.x - 4, min.y + 3),
				ImVec2(center.x + 4, min.y + 5),
				hovered ? color_active : text_color);
			draw_list->AddTriangleFilled(ImVec2(center.x - 4, min.y + 7),
				ImVec2(center.x + 4, min.y + 7),
				ImVec2(center.x, min.y + 12),
				hovered ? color_active : text_color);
		}

		bool tabbar(Dock& dock, bool close_button)
		{
			float tabbar_height = 2 * GetTextLineHeightWithSpacing();
			ImVec2 size(dock.size.x, tabbar_height);
			bool tab_closed = false;

			SetCursorScreenPos(dock.pos);
			char tmp[20];
			ImFormatString(tmp, IM_ARRAYSIZE(tmp), "tabs%d", (int)dock.id);
			if (BeginChild(tmp, size, true))
			{
				Dock* dock_tab = &dock;

				ImDrawList* draw_list = GetWindowDrawList();
				ImU32 color = GetColorU32(ImGuiCol_FrameBg);
				ImU32 color_active = GetColorU32(ImGuiCol_FrameBgActive);
				ImU32 color_hovered = GetColorU32(ImGuiCol_FrameBgHovered);
				ImU32 text_color = GetColorU32(ImGuiCol_Text);
				float line_height = GetTextLineHeightWithSpacing();
				float tab_base;

				drawTabbarListButton(dock);

				while (dock_tab)
				{
					SameLine(0, 15);

					const char* text_end = FindRenderedTextEnd(dock_tab->label.c_str());
					ImVec2 size(CalcTextSize(dock_tab->label.c_str(), text_end).x, line_height);
					if (InvisibleButton(dock_tab->label.c_str(), size))
						dock_tab->setActive();

					if (IsItemActive() && IsMouseDragging())
					{
						m_drag_offset = GetMousePos() - dock_tab->pos;
						doUndock(*dock_tab);
						dock_tab->status = Status_Dragged;
					}

					bool hovered = IsItemHovered();
					ImVec2 pos = GetItemRectMin();
					if (dock_tab->active && close_button)
					{
						size.x += 16 + GetStyle().ItemSpacing.x;
						SameLine();
						tab_closed = InvisibleButton("close", ImVec2(16, 16));
						ImVec2 center = (GetItemRectMin() + GetItemRectMax()) * 0.5f;
						draw_list->AddLine(
							center + ImVec2(-3.5f, -3.5f), center + ImVec2(3.5f, 3.5f), text_color);
						draw_list->AddLine(
							center + ImVec2(3.5f, -3.5f), center + ImVec2(-3.5f, 3.5f), text_color);
					}
					tab_base = pos.y;
					draw_list->PathClear();
					draw_list->PathLineTo(pos + ImVec2(-15, size.y));
					draw_list->PathBezierCurveTo(
						pos + ImVec2(-10, size.y), pos + ImVec2(-5, 0), pos + ImVec2(0, 0), 10);
					draw_list->PathLineTo(pos + ImVec2(size.x, 0));
					draw_list->PathBezierCurveTo(pos + ImVec2(size.x + 5, 0),
						pos + ImVec2(size.x + 10, size.y),
						pos + ImVec2(size.x + 15, size.y),
						10);
					draw_list->PathFill(
						hovered ? color_hovered : (dock_tab->active ? color_active : color));
					draw_list->AddText(pos, text_color, dock_tab->label.c_str(), text_end);

					dock_tab = dock_tab->next_tab;
				}
				ImVec2 cp(dock.pos.x, tab_base + line_height);
				draw_list->AddLine(cp, cp + ImVec2(dock.size.x, 0), color);
			}
			EndChild();
			return tab_closed;
		}

		static void setDockPosSize(Dock& dest, Dock& dock, Slot_ dock_slot, Dock& container)
		{
			IM_ASSERT(!dock.prev_tab && !dock.next_tab && !dock.children[0] && !dock.children[1]);

			dest.pos = container.pos;
			dest.size = container.size;
			dock.pos = container.pos;
			dock.size = container.size;

			switch (dock_slot)
			{
			case Slot_Bottom:
				dest.size.y *= 0.5f;
				dock.size.y *= 0.5f;
				dock.pos.y += dest.size.y;
				break;
			case Slot_Right:
				dest.size.x *= 0.5f;
				dock.size.x *= 0.5f;
				dock.pos.x += dest.size.x;
				break;
			case Slot_Left:
				dest.size.x *= 0.5f;
				dock.size.x *= 0.5f;
				dest.pos.x += dock.size.x;
				break;
			case Slot_Top:
				dest.size.y *= 0.5f;
				dock.size.y *= 0.5f;
				dest.pos.y += dock.size.y;
				break;
			default: IM_ASSERT(false); break;
			}
			dest.setPosSize(dest.pos, dest.size);

			if (container.children[1]->pos.x < container.children[0]->pos.x ||
				container.children[1]->pos.y < container.children[0]->pos.y)
			{
				Dock* tmp = container.children[0];
				container.children[0] = container.children[1];
				container.children[1] = tmp;
			}
		}

		void doDock(Dock& dock, Dock* dest, Slot_ dock_slot)
		{
			IM_ASSERT(!dock.parent);
			if (!dest)
			{
				dock.status = Status_Docked;
				dock.setPosSize(ImVec2(0, GetTextLineHeightWithSpacing()), GetIO().DisplaySize);
			}
			else if (dock_slot == Slot_Tab)
			{
				Dock* tmp = dest;
				while (tmp->next_tab)
				{
					tmp = tmp->next_tab;
				}

				tmp->next_tab = &dock;
				dock.prev_tab = tmp;
				dock.size = tmp->size;
				dock.pos = tmp->pos;
				dock.parent = dest->parent;
				dock.status = Status_Docked;
			}
			else if (dock_slot == Slot_None)
			{
				dock.status = Status_Float;
			}
			else
			{
				Dock* container = new Dock;
				new(container) Dock();
				m_docks.push_back(container);
				container->children[0] = &dest->getFirstTab();
				container->children[1] = &dock;
				container->next_tab = nullptr;
				container->prev_tab = nullptr;
				container->parent = dest->parent;
				container->size = dest->size;
				container->pos = dest->pos;
				container->status = Status_Docked;
				container->label = ImStrdup("");

				if (!dest->parent)
				{
				}
				else if (&dest->getFirstTab() == dest->parent->children[0])
				{
					dest->parent->children[0] = container;
				}
				else
				{
					dest->parent->children[1] = container;
				}

				dest->setParent(container);
				dock.parent = container;
				dock.status = Status_Docked;

				setDockPosSize(*dest, dock, dock_slot, *container);
			}
			dock.setActive();
		}

		void rootDock(const ImVec2& pos, const ImVec2& size)
		{
			Dock* root = getRootDock();
			if (!root) return;

			ImVec2 min_size = root->getMinSize();
			ImVec2 requested_size = size;
			root->setPosSize(pos, ImMax(min_size, requested_size));
		}

		void setDockActive()
		{
			IM_ASSERT(m_current);
			if (m_current) m_current->setActive();
		}

		static Slot_ getSlotFromLocationCode(char code)
		{
			switch (code)
			{
			case '1': return Slot_Left;
			case '2': return Slot_Top;
			case '3': return Slot_Bottom;
			default: return Slot_Right;
			}
		}

		static char getLocationCode(Dock* dock)
		{
			if (!dock) return '0';

			if (dock->parent->isHorizontal())
			{
				if (dock->pos.x < dock->parent->children[0]->pos.x) return '1';
				if (dock->pos.x < dock->parent->children[1]->pos.x) return '1';
				return '0';
			}
			else
			{
				if (dock->pos.y < dock->parent->children[0]->pos.y) return '2';
				if (dock->pos.y < dock->parent->children[1]->pos.y) return '2';
				return '3';
			}
		}

		void tryDockToStoredLocation(Dock& dock)
		{
			if (dock.status == Status_Docked) return;
			if (dock.location[0] == 0) return;

			Dock* tmp = getRootDock();
			if (!tmp) return;

			Dock* prev = nullptr;
			char* c = dock.location + strlen(dock.location) - 1;
			while (c >= dock.location && tmp)
			{
				prev = tmp;
				tmp = *c == getLocationCode(tmp->children[0]) ? tmp->children[0] : tmp->children[1];
				if (tmp) --c;
			}
			if (tmp && tmp->children[0]) tmp = tmp->parent;
			doDock(dock, tmp ? tmp : prev, tmp && !tmp->children[0] ? Slot_Tab : getSlotFromLocationCode(*c));
		}

		bool begin(const char* label, bool* opened, ImGuiWindowFlags extra_flags, const ImVec2& default_size)
		{
			Dock& dock = getDock(label, !opened || *opened, default_size);
			if (!dock.opened && (!opened || *opened)) tryDockToStoredLocation(dock);
			dock.last_frame = GetFrameCount();
			dock.label = label;

			m_end_action = EndAction_None;

			if (dock.first && opened) *opened = dock.opened;
			dock.first = false;
			if (opened && !*opened)
			{
				if (dock.status != Status_Float)
				{
					fillLocation(dock);
					doUndock(dock);
					dock.status = Status_Float;
				}
				dock.opened = false;
				return false;
			}
			dock.opened = true;

			m_end_action = EndAction_Panel;
			beginPanel();

			m_current = &dock;
			if (dock.status == Status_Dragged) handleDrag(dock);

			bool is_float = dock.status == Status_Float;

			if (is_float)
			{
				SetNextWindowPos(dock.pos);
				SetNextWindowSize(dock.size);
				bool ret = Begin(label,
					opened,
					dock.size,
					-1.0f,
					ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_ShowBorders | extra_flags);
				m_end_action = EndAction_End;
				dock.pos = GetWindowPos();
				dock.size = GetWindowSize();

				ImGuiContext& g = *GImGui;

				if (g.ActiveId == GetCurrentWindow()->MoveId && g.IO.MouseDown[0])
				{
					m_drag_offset = GetMousePos() - dock.pos;
					doUndock(dock);
					dock.status = Status_Dragged;
				}
				return ret;
			}

			if (!dock.active && dock.status != Status_Dragged) return false;

			m_end_action = EndAction_EndChild;

			PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
			PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));
			float tabbar_height = GetTextLineHeightWithSpacing();
			if (tabbar(dock.getFirstTab(), opened != nullptr))
			{
				fillLocation(dock);
				*opened = false;
			}
			ImVec2 pos = dock.pos;
			ImVec2 size = dock.size;
			pos.y += tabbar_height + GetStyle().WindowPadding.y;
			size.y -= tabbar_height + GetStyle().WindowPadding.y;

			SetCursorScreenPos(pos);
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
				extra_flags;
			char tmp[256];
			strcpy(tmp, label);
			strcat(tmp, "_docked"); // to avoid https://github.com/ocornut/imgui/issues/713
			bool ret = BeginChild(tmp, size, true, flags);
			PopStyleColor();
			PopStyleColor();
			return ret;
		}

		void end()
		{
			if (m_end_action == EndAction_End)
			{
				End();
			}
			else if (m_end_action == EndAction_EndChild)
			{
				PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
				PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));
				EndChild();
				PopStyleColor();
				PopStyleColor();
			}
			m_current = nullptr;
			if (m_end_action > EndAction_None) endPanel();
		}

		int getDockIndex(Dock* dock)
		{
			if (!dock) return -1;

			for (int i = 0; i < m_docks.size(); ++i)
			{
				if (dock == m_docks[i]) return i;
			}

			IM_ASSERT(false);
			return -1;
		}

		Dock* getDockByIndex(int idx) { return idx < 0 ? nullptr : m_docks[idx]; }
	};

	static DockContext g_dock;

	void ShutdownDock()
	{
		for (int i = 0; i < g_dock.m_docks.size(); ++i)
		{
			delete(g_dock.m_docks[i]);
		}
		g_dock.m_docks.clear();
	}

	void RootDock(const ImVec2& pos, const ImVec2& size)
	{
		g_dock.rootDock(pos, size);
	}

	void SetDockActive()
	{
		g_dock.setDockActive();
	}

	bool BeginDock(const char* label, bool* opened, ImGuiWindowFlags extra_flags, const ImVec2& default_size)
	{
		return g_dock.begin(label, opened, extra_flags, default_size);
	}

	void EndDock()
	{
		g_dock.end();
	}
}