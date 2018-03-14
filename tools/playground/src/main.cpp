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
	int n;
	std::vector<int> data;
	std::string fmt;

	App() :
		Application(1280, 720, flame::WindowStyleFrame | flame::WindowStyleResizable, "playground")
	{
		n = 3;
		srand(time(0));
		create();
	}

	void create()
	{
		data.resize(n * n);
		for (auto i = 0; i < data.size(); i++)
			data[i] = i;
		std::random_shuffle(data.begin(), data.end());
		fmt = "%" + std::to_string(flame::digit_num(data.size())) + "d ";
	}

	virtual void on_render() override
	{
		ImGui::BeginOverlapWindow("main");
		if (ImGui::DragInt("n", &n, 1, 1, 100))
			create();
		for (auto j = 0; j < n; j++)
		{
			for (auto i = 0; i < n; i++)
			{
				ImGui::Text(fmt.c_str(), data[j * n + i]);
				if (i != n - 1)
					ImGui::SameLine();
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
