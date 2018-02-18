#pragma once

#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/ui/ui.h>

struct ImageEditor : tke::ui::Window
{
	std::shared_ptr<tke::Texture> texture;
	std::shared_ptr<tke::Buffer> staging_buffer;

	ImageEditor(const std::string &filename);
	virtual void on_show() override;
};
