#pragma once

#include <memory>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_tabs.h>
#include <IconsFontAwesome.h>

#include <flame/utils/filesystem.h>
#include <flame/math/math.h>

namespace flame
{
	struct Texture;
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
	ImTextureID ImageID(std::shared_ptr<flame::Texture> i);
	void Image_f(const std::string &filename, const ImVec2& size, const ImVec4& border_col = ImVec4(0, 0, 0, 0));
	bool ImageButton_f(const std::string &filename, const ImVec2& size, bool active = false);
	bool IconButton(const char *label, float font_scale = 1.f);
	bool Checkbox_2in1(const char *label, bool *v);
	bool BeginToolBar();
	void EndToolBar();
	bool BeginStatusBar();
	void EndStatusBar();
}

namespace flame
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

		struct Window
		{
			std::string title;
			bool first;
			int first_cx;
			int first_cy;
			bool _need_focus;
			WindowTag _tag;

			bool enable_menu;
			bool enable_saved_settings;
			bool enable_dock;
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
			std::list<Window*> windows[2];
			Window *curr_tab[2];

			Layout();
			bool is_empty(int idx) const;
			bool is_empty() const;
			void set_size();
			void set_layout(int idx, Layout *l);
			void set_layout(int idx, std::unique_ptr<Layout> &&l);
			void add_window(int idx, Window *w);
			void remove_window(int idx, Window *w);
			void clear_window(int idx);
			void show_window(int idx);
			void show();
		};

		extern Layout *main_layout;

		glm::vec4 get_bg_color();
		void set_bg_color(const glm::vec4 &v);

		void init();
		void begin();
		void end();
		void save_layout();
	}
}
