#include <string>

#include "../src/core.h"
#include "../src/window.h"
#include "../src/ui/ui.h"

#include <memory>

struct MyWindow : tke::Window
{
	MyWindow()
		:Window(800, 600, "Count Down", tke::WindowStyleHasFrameNoResize, true)
	{
	}

	virtual ~MyWindow() override
	{
	}

	virtual void renderEvent() override
	{
		struct Item
		{
			std::string title;
			long long remain;

			bool dead = false;
		};

		static std::vector<Item> list;

		beginFrame(true);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("Main", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		static char title[200];
		ImGui::InputText("title", title, 200);
		static int time3[3];
		ImGui::InputInt3("H M S", time3);
		if (ImGui::Button("Add"))
		{
			for (;;)
			{
				if (title[0] != 0)
				{
					Item i;
					i.title = title;
					if (time3[1] >= 60 || time3[2] >= 60)
						break;
					i.remain = (time3[0] * 60 * 60 + time3[1] * 60 + time3[2]) * 1000;
					list.push_back(i);
					std::sort(list.begin(), list.end(), 
						[](Item &a, Item &b) { return a.remain < b.remain; });
					title[0] = 0;
					time3[0] = 0;
					time3[1] = 0;
					time3[2] = 0;
				}
				break;
			}
		}
		ImGui::Separator();
		for (auto it = list.begin(); it != list.end(); )
		{
			if (it->dead == true)
				it = list.erase(it);
			else
				it++;
		}
		int id = 0;
		for (auto &i : list)
		{
			i.remain -= tke::timeDisp;
			if (i.remain <= 0)
				i.remain = 0;
			ImGui::PushID(id);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, i.remain == 0 ? 0.f : 1.f, 0.f, 1.0f));
			ImGui::Text(i.title.c_str());
			ImGui::PopStyleColor();
			auto seconds = i.remain / 1000;
			ImGui::Text("%d:%02d:%02d", seconds / 60 / 60, seconds / 60 % 60, seconds % 60);
			if (ImGui::Button("Del"))
				i.dead = true;
			ImGui::PopID();
			id++;
			ImGui::Separator();
		}
		ImGui::End();

		endFrame();
	}
};


int main(int argc, char** argv)
{
	tke::init(false, "../", 800, 600, true);

	auto window = new MyWindow;
	window->show();
	tke::run();

	return 0;
}
