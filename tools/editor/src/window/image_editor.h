#pragma once

#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/ui/ui.h>

struct ImageEditor : flame::ui::Window
{
	std::shared_ptr<flame::Texture> texture;
	std::shared_ptr<flame::Buffer> staging_buffer;

	ImageEditor(std::shared_ptr<flame::Texture> _texture);
	virtual void on_show() override;
};
