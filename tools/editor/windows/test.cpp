#include <flame/engine/ui/window.h>
#include <flame/engine/ui/window_exports.h>

namespace flame
{
	namespace ui
	{
		struct TestWindow : Window
		{
			TestWindow() : 
				Window("Hello")
			{

			}

			virtual void on_show() override
			{

			}
		};
	}
}

extern "C"
{
	flame::ui::Window *flame_create_ui_window()
	{
		return new flame::ui::TestWindow;
	}

	void flame_destroy_ui_window(flame::ui::Window *w)
	{
		delete w;
	}
}
