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
	void BeginOverlapWindow(const char *title);
	void EndOverlapWindow();
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

		enum FileSelectorIo
		{
			FileSelectorOpen,
			FileSelectorSave
		};

		enum FileSelectorCreateFlag
		{
			FileSelectorCreateFlagNull,
			FileSelectorNoFiles = 1 << 0,
			FileSelectorNoRightArea = 1 << 1,
			FileSelectorTreeMode = 1 << 2
		};

		struct FileSelector : Window
		{
			bool io_mode;
			bool enable_file;
			bool enable_right_area = false;
			bool tree_mode;

			ImGui::Splitter splitter;

			struct ItemData
			{
				std::string value;
				std::string name;
				std::string filename;
			};

			struct DirItem;
			struct FileItem;

			struct DirItem : ItemData
			{
				std::vector<std::unique_ptr<DirItem>> dir_list;
				std::vector<std::unique_ptr<FileItem>> file_list;
			};

			struct FileItem : ItemData
			{
				FileType file_type = FileTypeUnknown;

				virtual ~FileItem() {}
			};

			char filename[260];
			DirItem curr_dir;
			std::string default_dir;
			std::list<std::string> curr_dir_hierarchy;
			int select_index = -1;
			DirItem *select_dir = nullptr;
			bool need_refresh = true;

			std::function<bool(std::string)> callback;

			std::unique_ptr<FileWatcherHandler> file_watcher;

			FileSelector(const std::string &_title, FileSelectorIo io, const std::string &_default_dir = "",
				unsigned int window_flags = WindowCreateFlagNull, unsigned int flags = FileSelectorCreateFlagNull);
			void set_current_path(const std::string &s);
			void refresh();
			virtual void on_show() override;
			virtual bool on_refresh();
			virtual FileItem *on_new_file_item();
			virtual void on_add_file_item(FileItem *i);
			virtual void on_dir_item_selected(DirItem *i);
			virtual void on_file_item_selected(FileItem *i, bool doubleClicked);
			virtual void on_top_area_show();
			virtual void on_bottom_area_show();
			virtual void on_right_area_show();
		};

		struct DirSelectorDialog : FileSelector
		{
			DirSelectorDialog();
			static void open(const std::string &default_dir, const std::function<bool(std::string)> &_callback);
		};

		struct ImageViewer : Window
		{
			std::shared_ptr<Texture> texture;
			std::shared_ptr<Buffer> staging_buffer;

			ImageViewer(const std::string &_title, std::shared_ptr<Texture> _texture);
			virtual void on_show() override;
			virtual void on_menu_bar() {};
			virtual void on_top_area() {};
			virtual void on_mouse_overing_image(ImVec2 image_pos) {}
		};

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
		void draw_text(const std::string &text, int x, int y, int size);
		void save_layout();
	}
}
