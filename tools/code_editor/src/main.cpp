#include <sstream>
#include <string>
#include <vector>
#include <fstream>

#include <flame/common/system.h>
#include <flame/common/image.h>
#include <flame/engine/core/core.h>
#include <flame/engine/core/application.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/ui/ui.h>

struct App : flame::Application
{
	char text_buf[1024 * 1024];

	App() :
		Application(1280, 720, flame::WindowStyleFrame | flame::WindowStyleResizable, "code editor")
	{
		text_buf[0] = 0;
	}

	virtual void on_render() override
	{
		ImGui::BeginOverlapWindow("main");
		if (ImGui::Button("Eliminate Unactive Blocks"))
		{
			std::ofstream o_file("temp.c");
			o_file.write(text_buf, strlen(text_buf));
			o_file.close();

			std::stringstream sstream(flame::create_process_and_get_output("", "gcc temp.c -E -o temp.perprocess"));
			text_buf[0] = 0;
			auto p = text_buf;
			while (!sstream.eof())
			{
				std::string line;
				std::getline(sstream, line);

				p = strcat(p, line.c_str());
				p = strcat(p, "\n");
			}
		}
		ImGui::BeginChild("##text");
		ImGui::InputTextMultiline("##text", text_buf, sizeof(text_buf), ImVec2(ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin()), ImGuiInputTextFlags_AllowTabInput);
		ImGui::EndChild();
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
