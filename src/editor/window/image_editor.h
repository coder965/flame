#pragma once

#include "../../render/image.h"

#include "window.h"

struct ImageEditor : IWindow
{
	std::shared_ptr<tke::Image> image;

	ImageEditor(std::shared_ptr<tke::Image> _image);
	~ImageEditor();
	virtual void do_show() override;
};
