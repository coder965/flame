#pragma once

#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/ui/ui.h>

struct ImageEditor : tke::ui::Window
{
	std::shared_ptr<tke::Texture> texture;
	std::shared_ptr<tke::Buffer> staging_buffer;

	ImageEditor(std::shared_ptr<tke::Texture> _texture);
	virtual void on_show() override;
};
