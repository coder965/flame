#pragma once

#include "../../../src/render/image.h"

#include "window.h"

struct ImageEditorClass : WindowClass
{
	virtual std::string getName() override;
	virtual Window *load(tke::AttributeTreeNode *n) override;
};

extern ImageEditorClass imageEditorClass;

struct ImageEditor : Window
{
	std::shared_ptr<tke::Image> image;

	ImageEditor(std::shared_ptr<tke::Image> _image);
	~ImageEditor();
	virtual void show() override;
};

extern ImageEditor *imageEditor;