#include "../../src/ui/ui.h"
#include "../../src/core.h"

#include "editor.h"
#include "window/dir_selector.h"
#include "window/scene_editor.h"
#include "window/image_editor.h"

std::experimental::filesystem::path project_path = "d:\\TK_Engine\\editor";

int main(int argc, char** argv)
{
	tke::init(true, "../", 800, 600, 1280, 720, "TK Engine Editor", tke::WindowStyleFrame | tke::WindowStyleResize, false);

	ShowWindow(tke::hWnd, SW_SHOWMAXIMIZED);

	initWindow();

	{
		tke::AttributeTree at("data", "ui.xml");
		if (at.good)
		{
			for (auto &n : at.children)
			{
				if (n->name == "window")
				{
					Window *w = nullptr;
					for (auto c : windowClasses)
					{
						if (w) break;
						auto a = n->firstAttribute("type");
						if (a && a->value == c->getName())
							w = c->load(n.get());
					}
					if (w)
						windows.push_back(std::move(std::unique_ptr<Window>(w)));
				}
			}
		}
	}

	tke::onRender = []() {
		tke::beginFrame(true);

		bool openNewImagePopop = false;

		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("New"))
			{
				if (ImGui::MenuItem("Project"))
					;
				if (ImGui::MenuItem("Image"))
					openNewImagePopop = true;
				if (ImGui::MenuItem("Terrain"))
					;

				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Open Project"))
			{
				DirSelector::open("c:\\", [](std::string path) {
					project_path = path;
					if (resourceExplorer)
					{
						resourceExplorer->path = project_path;
						resourceExplorer->refresh();
					}
				});
			}
			if (ImGui::MenuItem("Save Project"))
				;
			if (ImGui::MenuItem("Close Project"))
				;

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Resource Explorer", nullptr, resourceExplorer != nullptr))
			{
				if (!resourceExplorer)
				{
					resourceExplorer = new ResourceExplorer;
					windows.push_back(std::move(std::unique_ptr<Window>(resourceExplorer)));
				}
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		if (openNewImagePopop)
			ImGui::OpenPopup("New Image");
		if (ImGui::BeginPopupModal("New Image", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
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
				//image = new tke::Image(cx, cy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
				//auto cb = tke::begineOnceCommandBuffer();
				//VkClearColorValue clearValue = {0.f, 0.f, 0.f, 1.f};
				//VkImageSubresourceRange range;
				//range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				//range.baseMipLevel = 0;
				//range.levelCount = 1;
				//range.baseArrayLayer = 0;
				//range.layerCount = 1;
				//vkCmdClearColorImage(cb->v, image->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &range);
				//tke::endOnceCommandBuffer(cb);
				//tke::addUiImage(image);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		for (auto &w : windows)
			w->show();

		ImGui::SetNextWindowPos(ImVec2(0, tke::window_cy - ImGui::GetItemsLineHeightWithSpacing()));
		ImGui::Begin("status", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("FPS:%d", tke::FPS);
		ImGui::End();

		tke::endFrame();

		for (auto it = windows.begin(); it != windows.end(); )
		{
			if (!(*it)->opened)
				it = windows.erase(it);
			else
				it++;
		}
	};

	tke::onDestroy = []() {
		{
			tke::AttributeTree at("data");
			for (auto &w : windows)
			{
				if (!w->pClass)
					continue;
				auto n = new tke::AttributeTreeNode("window");
				n->addAttribute("type", w->pClass->getName());
				w->save(n);
				at.add(n);
			}
			if (SelectObject)
			{
				auto n = new tke::AttributeTreeNode("select");
				at.add(n);
			}
			at.saveXML("ui.xml");
		}
	};

	tke::run();

	return 0;
}
