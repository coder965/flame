#include <string>

#include <functional>
#include <memory>

#include "../src/math/math.h"
#include "../src/ui/ui.h"
#include "../src/core.h"

int main(int argc, char** argv)
{
	tke::init(false, "../", 800, 600, 1280, 720, "test", tke::WindowStyleFrame | tke::WindowStyleResize, true);

	tke::onRender = []() {
		tke::beginFrame(true);

		static bool first = true;
		if (first)
		{
			ImGui::OpenPopup("p0");
		}

		if (ImGui::BeginPopupModal("p0"))
		{
			if (ImGui::Button("ok1"))
				ImGui::CloseCurrentPopup();

			if (first)
			ImGui::OpenPopup("p1");

			if (ImGui::BeginPopupModal("p1"))
			{
				if (ImGui::Button("ok2"))
					ImGui::CloseCurrentPopup();

				ImGui::EndPopup();
			}

			ImGui::EndPopup();
		}
		first = false;

		tke::endFrame();
	};

	tke::run();

	return 0;
}
