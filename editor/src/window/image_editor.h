#pragma once

#include "../../../src/render/image.h"

#include "window.h"

struct ImageEditorClass : IWindowClass
{
	virtual std::string getName() override;
	virtual IWindow *load(tke::AttributeTreeNode *n) override;
};

extern ImageEditorClass imageEditorClass;

struct ImageEditor : IWindow
{
	std::shared_ptr<tke::Image> image;

	ImageEditor(std::shared_ptr<tke::Image> _image);
	~ImageEditor();
	virtual void do_show() override;
};
