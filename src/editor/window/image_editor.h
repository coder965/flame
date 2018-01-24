#pragma once

#include "../../graphics/image.h"

#include "../../ui/ui.h"

struct ImageEditor : tke::ui::Window
{
	std::shared_ptr<tke::Image> image;

	ImageEditor(const std::string &filename);
	virtual void on_show() override;
};
