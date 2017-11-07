#pragma once

#include "../../src/render/image.h"

struct TextureEditor
{
	tke::Image *image = nullptr;
	bool opened = true;

	void show();
};

extern TextureEditor *textureEditor;