//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include <flame/common/system.h>
#include <flame/common/image.h>
#include <flame/engine/core/core.h>
#include <flame/engine/core/application.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/ui/ui.h>

struct App : flame::Application
{
	int pid;
	HANDLE hProcess;
	bool do_refresh;
	std::vector<std::pair<std::string, int>> flame_instances;

	App() :
		Application(1280, 720, flame::WindowStyleFrame | flame::WindowStyleResizable, "spv"),
		pid(0),
		hProcess(0),
		do_refresh(true)
	{
	}

	virtual void on_render() override
	{
		ImGui::BeginOverlapWindow("main");
		if (pid)
		{
			ImGui::Text("Now Attaching: %d", pid);
			ImGui::SameLine();
			if (ImGui::Button("Detach"))
			{
				CloseHandle(hProcess);
				pid = 0;
			}
		}
		else
			ImGui::TextUnformatted("Not Attaching");
		if (ImGui::Button(ICON_FA_REFRESH))
			do_refresh = true;
		if (do_refresh)
		{
			flame_instances.clear();
			auto hwnd = FindWindow(nullptr, "Flame Engine Editor");
			if (hwnd)
			{
				DWORD _pid;
				GetWindowThreadProcessId(hwnd, &_pid);
				flame_instances.push_back({ "Flame Engine Editor", _pid });
			}
			do_refresh = false;
		}
		auto do_attach = [&](int _pid) {
			if (pid)
				CloseHandle(hProcess);
			pid = _pid;
			hProcess = OpenProcess(PROCESS_VM_READ, false, pid);
			assert(hProcess);
		};
		for (auto &i : flame_instances)
		{
			if(ImGui::Button(("Attach To [" +  i.first + "] " + std::to_string(i.second)).c_str()))
				do_attach(i.second);
		}
		if (ImGui::Button("Attach To Pid.."))
			ImGui::OpenPopup("Attach To");
		if (ImGui::BeginPopupModal("Attach To"))
		{
			static char buf[30];
			ImGui::InputText("pid", buf, sizeof(buf));
			if (ImGui::Button("OK"))
			{
				do_attach(std::stoi(buf));
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		if (pid)
		{
			ImGui::Separator();
			if (ImGui::Button("View Image"))
				ImGui::OpenPopup("Enter Address Of Image");
			if (ImGui::BeginPopupModal("Enter Address Of Image"))
			{
				static char buf[30];
				ImGui::InputText("##address", buf, sizeof(buf));
				if (ImGui::Button("OK"))
				{
					void *address = (void*)std::stoll(buf, 0, 16);
					if (address)
					{
						auto img_u = flame::read_process_memory(hProcess, address, sizeof(flame::Image));
						auto img = (flame::Image*)img_u.get();
						auto t = std::make_shared<flame::Texture>(img->cx, img->cy, flame::get_texture_format(img->bpp, img->channel, false), VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, false);
						auto img_data = flame::read_process_memory(hProcess, img->data, img->cy * img->pitch);
						t->fill_data(0, img_data.get());
						new flame::ui::ImageViewer("1", t);
					}
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
		}
		ImGui::EndOverlapWindow();
	}
};

int main(int argc, char **args)
{
	flame::init("../", 1280, 720, 1, false, true);
	new App;
	flame::run();

	return 0;
}
