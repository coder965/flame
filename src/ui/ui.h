#pragma once

#include <memory>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"
#include "../../IconFontCppHeaders/IconsFontAwesome.h"

#include "../file_utils.h"
#include "../math/math.h"

namespace tke
{
	struct Image;
}

namespace ImGui
{
	extern ImVec2 SplitterThickness;

	void TextVFilted(const char* fmt, const char* filter, va_list args);
	bool Splitter(bool split_vertically, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);
	ImTextureID ImageID(std::shared_ptr<tke::Image> i);
	void Image_f(const std::string &filename, const ImVec2& size, const ImVec4& border_col = ImVec4(0, 0, 0, 0));
	bool BeginStatusBar();
	void EndStatusBar();
}

namespace tke
{
	namespace ui
	{
		extern bool accepted_mouse;
		extern bool accepted_key;

		enum DockDirection
		{
			DockCenter,
			DockLeft,
			DockRight,
			DockTop,
			DockBottom
		};


		enum LayoutType
		{
			LayoutNull,
			LayoutCenter,
			LayoutHorizontal,
			LayoutVertical
		};

		struct Layout;

		struct Window
		{
			bool first;
			int first_cx;
			int first_cy;
			bool _need_focus;

			std::string title;
			bool enable_menu;
			bool enable_saved_settings;
			bool modal;
			bool opened;

			Layout *layout;
			int idx;

			Window(const std::string &_title, bool _enable_menu = false, bool _enable_saved_settings = true, bool _modal = false);
			virtual ~Window() {}
			virtual void on_show() = 0;
			virtual void save(XMLNode *) {}
			void add_to_main_dock();
			void add_dock_window(Window *w, DockDirection dir);
			void show();
		};

		float get_layout_padding(bool horizontal);

		struct Layout
		{
			Layout *parent;
			int idx;

			LayoutType type;

			float width;
			float height;
			float size[2];

			std::unique_ptr<Layout> children[2];

			Window *windows[2];

			Layout();
			void set_size();
			void show();
		};

		extern Layout main_layout;

		glm::vec4 get_bg_color();
		void set_bg_color(const glm::vec4 &v);

		void init();
		void begin();
		void end();
	}
}
