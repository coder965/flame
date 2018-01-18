#include "../ui/ui.h"
#include "../input.h"
#include "../global.h"
#include "../engine.h"
#include "window/file_selector.h"
#include "window/resource_explorer.h"
#include "window/hierarchy.h"
#include "window/inspector.h"
#include "window/scene_editor.h"
#include "window/image_editor.h"

struct NewImageDialog : FileSelector
{
	bool first = true;
	int cx = 512;
	int cy = 512;

	NewImageDialog()
		:FileSelector("New Image", true, true, true, 1, 800, 600)
	{
		callback = [this](std::string s) {
			if (std::experimental::filesystem::exists(s))
				return false;
			tke::newImageFile(s, cx, cy, 32);
			return true;
		};
	}

	virtual void on_right_area_show() override
	{
		ImGui::PushItemWidth(200);
		ImGui::DragInt("cx", &cx);
		ImGui::DragInt("cy", &cy);
		const char *typeNames[] = {
			"color R8G8B8A8"
		};
		static int type = 0;
		ImGui::Combo("type", &type, typeNames, TK_ARRAYSIZE(typeNames));
		ImGui::PopItemWidth();
	}
};

int main(int argc, char** argv)
{
	tke::init(true, "../", 1920, 1080, 1280, 720, "TK Engine Editor", tke::WindowStyleFrame | tke::WindowStyleResize, false);

	ShowWindow((HWND)tke::hWnd, SW_SHOWMAXIMIZED);

	{
		tke::XMLDoc at("data", "ui.xml");
		if (at.good)
		{
			for (auto &n : at.children)
			{
				if (n->name == "resource_explorer")
					resourceExplorer = new ResourceExplorer;
				else if (n->name == "hierarchy_window")
					hierarchy_window = new HierarchyWindow;
				else if (n->name == "inspector_window")
					inspector_window = new InspectorWindow;
			}
		}
	}

	tke::onRender = []() {
		tke::begin_frame(true);

		bool show_device_Props = false;
		if (ImGui::last_frame_main_menu_alive || tke::mouseY <= ImGui::GetFrameHeight())
		{
			ImGui::BeginMainMenuBar();
			if (ImGui::BeginMenu_keepalive("File"))
			{
				if (ImGui::BeginMenu("New"))
				{
					if (ImGui::MenuItem("Scene"))
						;
					if (ImGui::MenuItem("Image"))
						new NewImageDialog;

					ImGui::EndMenu();
				}

				if (scene_editor)
					scene_editor->on_file_menu();

				ImGui::EndMenu();
			}
			if (scene_editor)
				scene_editor->on_menu_bar();
			if (ImGui::BeginMenu_keepalive("View"))
			{
				static bool fullscreen = false;
				if (ImGui::MenuItem("Fullscreen", "", &fullscreen))
				{
					if (fullscreen)
					{
						tke::window_style |= tke::WindowStyleFullscreen;
						tke::window_style &= (~tke::WindowStyleFrame);
					}
					else
					{
						tke::window_style |= tke::WindowStyleFrame;
						tke::window_style &= (~tke::WindowStyleFullscreen);
						tke::window_cx = tke::res_cx;
						tke::window_cy = tke::res_cy;
					}
					auto wndProp = tke::getWin32WndProp();
					SetWindowLong((HWND)tke::hWnd, GWL_STYLE, wndProp.second);
					SetWindowPos((HWND)tke::hWnd, HWND_TOP, 0, 0, wndProp.first.x, wndProp.first.y, SWP_NOZORDER);
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Resource Explorer"))
				{
					if (!resourceExplorer)
						resourceExplorer = new ResourceExplorer;
					resourceExplorer->_need_focus = true;
				}
				if (ImGui::MenuItem("Hierarchy"))
				{
					if (!hierarchy_window)
						hierarchy_window = new HierarchyWindow;
					hierarchy_window->_need_focus = true;
				}
				if (ImGui::MenuItem("Inspector"))
				{
					if (!inspector_window)
						inspector_window = new InspectorWindow;
					inspector_window->_need_focus = true;
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu_keepalive("Help"))
			{
				if (ImGui::MenuItem("Device Properties"))
					show_device_Props = true;

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (show_device_Props)
			ImGui::OpenPopup("Device Properties");
		if (ImGui::BeginPopupModal("Device Properties"))
		{
			if (ImGui::TreeNode("Physical Device Properties"))
			{
				auto &p = tke::vk_physical_device_properties;
				ImGui::Text("api version: %d", p.apiVersion);
				ImGui::Text("driver version: %d", p.driverVersion);
				ImGui::Text("vendor ID: %d", p.vendorID);
				ImGui::Text("device ID: %d", p.deviceID);
				ImGui::Text("device type: %s", tke::vk_device_type_names[p.deviceType]);
				ImGui::Text("device name: %s", p.deviceName);

				if (ImGui::TreeNode("Limits"))
				{
					ImGui::BeginChild("##limits", ImVec2(0, 300), true);
					auto &l = p.limits;
					ImGui::Text("max image dimension 1D: %d", l.maxImageDimension1D);
					ImGui::Text("max image dimension 2D: %d", l.maxImageDimension2D);
					ImGui::Text("max image dimension 3D: %d", l.maxImageDimension3D);
					ImGui::Text("max image dimension cube: %d", l.maxImageDimensionCube);
					ImGui::Text("max image array layers: %d", l.maxImageArrayLayers);
					ImGui::Text("max texel buffer range: %d", l.maxTexelBufferElements);
					ImGui::Text("max uniform buffer range: %d", l.maxUniformBufferRange);
					ImGui::Text("max storage buffer range: %d", l.maxStorageBufferRange);
					ImGui::Text("max push constants size: %d", l.maxPushConstantsSize);
					ImGui::Text("max memory allocation count: %d", l.maxMemoryAllocationCount);
					ImGui::Text("max sampler allocation count: %d", l.maxSamplerAllocationCount);
					ImGui::Text("buffer image granularity: %lld", l.bufferImageGranularity);
					ImGui::Text("sparse address space size: %lld", l.sparseAddressSpaceSize);
					ImGui::Text("max bound descriptor sets: %d", l.maxBoundDescriptorSets);
					ImGui::Text("max per stage descriptor samplers: %d", l.maxPerStageDescriptorSamplers);
					ImGui::Text("max per stage descriptor uniform buffers: %d", l.maxPerStageDescriptorUniformBuffers);
					ImGui::Text("max per stage descriptor storage buffers: %d", l.maxPerStageDescriptorStorageBuffers);
					ImGui::Text("max per stage descriptor sampled images: %d", l.maxPerStageDescriptorSampledImages);
					ImGui::Text("max per stage descriptor storage images: %d", l.maxPerStageDescriptorStorageImages);
					ImGui::Text("max per stage descriptor input attachments: %d", l.maxPerStageDescriptorInputAttachments);
					ImGui::Text("max per stage resources: %d", l.maxPerStageResources);
					ImGui::Text("max descriptor set samplers: %d", l.maxDescriptorSetSamplers);
					ImGui::EndChild();

					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Sparse Properties"))
				{
					ImGui::TreePop();
				}

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Physical Device Features"))
			{
				ImGui::TreePop();
			}

			if (ImGui::Button("Ok"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		if (scene_editor)
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(tke::window_cx, tke::window_cy));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
			if (ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing | 
				ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings))
				scene_editor->do_show();
			ImGui::End();
			ImGui::PopStyleVar(2);
		}

		{
			std::vector<IWindow*> _w;
			for (auto &w : windows)
				_w.push_back(w.get());
			for (auto &w : _w)
				w->show();
		}

		ImGui::SetNextWindowPos(ImVec2(0, tke::window_cy - ImGui::GetFrameHeightWithSpacing()));
		ImGui::Begin("status", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("FPS:%d", tke::FPS);
		ImGui::End();

		tke::end_frame();

		for (auto it = windows.begin(); it != windows.end(); )
		{
			if (!(*it)->opened)
				it = windows.erase(it);
			else
				it++;
		}
	};
	
	tke::onDestroy = []() {
		tke::XMLDoc at("data");
		if (resourceExplorer)
			at.newNode("resource_explorer");
		if (hierarchy_window)
			at.newNode("hierarchy_window");
		if (inspector_window)
			at.newNode("inspector_window");
		if (SelectObject)
			at.newNode("select");
		at.save("ui.xml");
	};

	tke::run();

	return 0;
}
