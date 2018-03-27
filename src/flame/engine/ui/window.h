#pragma once

#include <flame/engine/ui/ui.h>
#include <flame/engine/ui/layout.h>

namespace flame
{
	namespace ui
	{
		struct Layout;

		enum WindowTag
		{
			WindowTagNull,
			WindowTagUndock,
			WindowTagClose
		};

		enum WindowCreateFlag
		{
			WindowCreateFlagNull,
			WindowHasMenu = 1 << 0,
			WindowNoSavedSettings = 1 << 1,
			WindowModal = 1 << 2,
			WindowBanDock = 1 << 3
		};

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
			void show();
		};

		const std::list<std::unique_ptr<Window>> &get_windows();
		void show_windows();
	}
}
