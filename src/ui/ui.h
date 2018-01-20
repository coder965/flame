#pragma once

#include <vector>
#include <memory>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"
#include "../../IconFontCppHeaders/IconsFontAwesome.h"

#include "../math/math.h"

namespace tke
{
	struct Image;
}

namespace ImGui
{
	extern bool main_menu_alive;
	extern bool last_frame_main_menu_alive;

	bool BeginMenu_keepalive(const char* label, bool enabled = true);
	void TextVFilted(const char* fmt, const char* filter, va_list args);
	bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);
	ImTextureID ImageID(std::shared_ptr<tke::Image> i);
	void Image_f(const std::string &filename, const ImVec2& size, const ImVec4& border_col = ImVec4(0, 0, 0, 0));
}

namespace tke
{
	extern bool uiAcceptedMouse;
	extern bool uiAcceptedKey;

	extern glm::vec3 bkColor;

	void initUi();
	void ui_onKeyDown(int);
	void ui_onKeyUp(int);
	void ui_onChar(int);
	void beginUi(bool _need_clear);
	void endUi();

	namespace ui
	{
		struct Window
		{
			bool dirty = true;
		};
	}
}
