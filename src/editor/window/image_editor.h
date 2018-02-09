#pragma once

#include "../../graphics/image.h"
#include "../../graphics/buffer.h"

#include "../../ui/ui.h"

struct ImageEditor : tke::ui::Window
{
	std::shared_ptr<tke::Image> image;
	std::shared_ptr<tke::StagingBuffer> staging_buffer;

	ImageEditor(const std::string &filename);
	virtual void on_show() override;
};
