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
	extern const float SplitterThickness;

	void TextVFilted(const char* fmt, const char* filter, va_list args);
	bool Splitter(bool split_vertically, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.f, float offset = 0.f);
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
			DockCenter = 1 << 0,
			DockLeft = 1 << 1,
			DockRight = 1 << 2,
			DockTop = 1 << 3,
			DockBottom = 1 << 4,
			DockAll = DockCenter | DockLeft | DockRight | DockTop | DockBottom
		};


		enum LayoutType
		{
			LayoutNull,
			LayoutCenter,
			LayoutHorizontal,
			LayoutVertical
		};

		struct Layout;

		enum WindowTag
		{
			WindowTagNull,
			WindowTagUndock,
			WindowTagClose
		};

		struct Window
		{
			bool first;
			int first_cx;
			int first_cy;
			bool _need_focus;
			WindowTag _tag;

			std::string title;
			bool enable_menu;
			bool enable_saved_settings;
			bool modal;
			bool opened;

			Layout *layout;
			int idx;
			int tab_x;
			int tab_width;

			Window(const std::string &_title, bool _enable_menu = false, bool _enable_saved_settings = true, bool _modal = false);
			virtual ~Window() {}
			virtual void on_show() = 0;
			virtual void save(XMLNode *) {}
			void dock(Window *w = nullptr, DockDirection dir = DockCenter);
			void undock();
			void show();
		};

		const std::list<std::unique_ptr<Window>> &get_windows();

		float get_layout_padding(bool horizontal);

		struct Layout
		{
			Layout *parent;
			int idx;

			LayoutType type;

			float width;
			float height;
			float size_radio;
			float size[2];

			std::unique_ptr<Layout> children[2];

			std::list<Window*> windows[2];
			Window *curr_tab[2];
			Window *dragging_tab[2];
			int dragging_tab_anchor[2];

			Layout();
			bool is_empty(int idx) const;
			bool is_empty() const;
			void set_size();
			void set_layout(int idx, Layout *l);
			void set_layout(int idx, std::unique_ptr<Layout> &&l);
			void add_window(int idx, Window *w);
			void clear_window(int idx);
			void show_window(int idx);
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
