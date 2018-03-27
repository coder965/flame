#pragma once

#include <flame/engine/ui/window.h>

namespace flame
{
	namespace ui
	{
		struct ImageViewer : Window
		{
			std::shared_ptr<Texture> texture;
			std::shared_ptr<Buffer> staging_buffer;

			ImageViewer(const std::string &_title, std::shared_ptr<Texture> _texture);
			~ImageViewer();
			virtual void on_show() override;
			virtual void on_menu_bar() {};
			virtual void on_top_area() {};
			virtual void on_mouse_overing_image(ImVec2 image_pos) {}
		};
	}
}
