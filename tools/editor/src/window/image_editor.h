#pragma once

#include "../../graphics/buffer.h"
#include "../../graphics/texture.h"

#include "../../ui/ui.h"

struct ImageEditor : tke::ui::Window
{
	std::shared_ptr<tke::Texture> texture;
	std::shared_ptr<tke::StagingBuffer> staging_buffer;

	ImageEditor(const std::string &filename);
	virtual void on_show() override;
};
