#pragma once

#include <vector>
#include <memory>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"
#include "../../IconFontCppHeaders/IconsFontAwesome.h"

#include "../refl.h"
#include "../math/math.h"

namespace ImGui
{
	extern bool main_menu_alive;
	extern bool last_frame_main_menu_alive;

	bool BeginMenu_keepalive(const char* label, bool enabled = true);

	bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);
}

namespace tke
{
	struct Image;

	IMPL() bool uiAcceptedMouse;
	IMPL() bool uiAcceptedKey;

	IMPL(glm::vec3(0.69f,0.76f,0.79f)) glm::vec3 bkColor;

	void initUi();
	void ui_onKeyDown(int);
	void ui_onKeyUp(int);
	void ui_onChar(int);
	void beginUi(bool _need_clear);
	void endUi();
	int get_ui_image_index(std::shared_ptr<Image> i);
}
