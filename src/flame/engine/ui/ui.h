#pragma once

#include <memory>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_tabs.h>
#include <IconsFontAwesome4.h>

#include <flame/common/filesystem.h>
#include <flame/common/math.h>
#include <flame/common/system.h>

namespace flame
{
	struct Buffer;
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

	extern float menubar_height;
	extern float toolbar_height;
	extern float statusbar_height;

	void TextVFilted(const char* fmt, const char* filter, va_list args);
	void Image_s(flame::Texture *t, const ImVec2& size, const ImVec4& border_col = ImVec4(0, 0, 0, 0));
	bool ImageButton_s(flame::Texture *t, const ImVec2& size, bool active = false);
	bool IconButton(const char *label, float font_scale = 1.f);
	bool Checkbox_2in1(const char *label, bool *v);
	bool BeginToolBar();
	void EndToolBar();
	bool BeginStatusBar();
	void EndStatusBar();
	void BeginOverlapWindow(const char *title);
	void EndOverlapWindow();
}

namespace flame
{
	namespace ui
	{
		extern bool accepted_mouse;
		extern bool accepted_key;

		glm::vec4 get_bg_color();
		void set_bg_color(const glm::vec4 &v);

		void init();
		void begin();
		void end();
		void increase_texture_ref(Texture *i);
		void decrease_texture_ref(Texture *i);
		void unregister_texture(Texture *i);
		void draw_text(const std::string &text, int x, int y, int size);
	}
}
