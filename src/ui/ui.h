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

	struct Splitter
	{
		bool vertically;
		float size[2];
		float min_size[2];
		float draw_offset;

		Splitter(bool _vertically, float _min_size1 = 50.f, float _min_size2 = 50.f);
		void set_size_greedily();
		void set_general_draw_offset();
		void set_vertically(bool _vertically);
		bool do_split();
	};

	void TextVFilted(const char* fmt, const char* filter, va_list args);
	ImTextureID ImageID(std::shared_ptr<tke::Image> i);
	void Image_f(const std::string &filename, const ImVec2& size, const ImVec4& border_col = ImVec4(0, 0, 0, 0));
	bool ImageButton_f(const std::string &filename, const ImVec2& size, bool active = false);
	bool IconButton(const char *label, float font_scale = 1.f);
	bool Checkbox_2in1(const char *label, bool *v);
	bool BeginMainMenuBar_l();
	bool BeginToolBar();
	void EndToolBar();
	bool BeginStatusBar();
	void EndStatusBar();
}

namespace tke
{
	namespace ui
	{
		extern bool accepted_mouse;
		extern bool accepted_key;

		enum WindowTag
		{
			WindowTagNull,
			WindowTagUndock,
			WindowTagClose
		};

		struct Tab
		{
			std::string title;
			bool enable_dock;
			bool enable_close_tab;

			int tab_x;
			int tab_width;

			WindowTag _tag;

			Tab();
		};

		struct Layout;

		enum WindowCreateFlag
		{
			WindowCreateFlagNull,
			WindowHasMenu = 1 << 0,
			WindowNoSavedSettings = 1 << 1,
			WindowModal = 1 << 2,
			WindowBanDock = 1 << 3
		};

		enum DockDirection
		{
			DockCenter = 1 << 0,
			DockLeft = 1 << 1,
			DockRight = 1 << 2,
			DockTop = 1 << 3,
			DockBottom = 1 << 4,
			DockAll = DockCenter | DockLeft | DockRight | DockTop | DockBottom
		};

		const char *get_dock_dir_name(DockDirection dir);

		struct Window : Tab
		{
			bool first;
			int first_cx;
			int first_cy;
			bool _need_focus;

			bool enable_menu;
			bool enable_saved_settings;
			bool modal;
			bool opened;

			Layout *layout;
			int idx;

			Window(const std::string &_title, unsigned int flags = WindowCreateFlagNull);
			virtual ~Window() {}
			virtual void on_show() = 0;
			virtual void save(XMLNode *) {}
			void dock(Window *w = nullptr, DockDirection dir = DockCenter);
			void undock();
			void show();
		};

		struct Tabbar
		{
			std::list<Tab*> tabs;
			Tab *curr_tab;
			Tab *dragging_tab;
			int dragging_tab_offset;
			bool _dragging_tab_out_of_bar;

			Tabbar();
			bool is_empty() const;
			void clear();
			void add_tab(Tab *t);
			void remove_tab(Tab *t);
			void show();
		};

		const std::list<std::unique_ptr<Window>> &get_windows();

		float get_layout_padding(bool horizontal);

		enum LayoutType
		{
			LayoutCenter,
			LayoutHorizontal,
			LayoutVertical
		};

		struct Layout
		{
			Layout *parent;
			int idx;

			LayoutType type;

			float width;
			float height;
			float size_radio;
			ImGui::Splitter splitter;

			std::unique_ptr<Layout> children[2];
			Tabbar tabbar[2];

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
