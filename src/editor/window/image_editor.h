#pragma once

#include "../../graphics/image.h"

#include "window.h"

struct ImageEditor : IWindow
{
	std::shared_ptr<tke::Image> image;

	ImageEditor(const std::string &filename);
	virtual void do_show() override;
};
