#pragma once

#include "..\src\image.h"

struct TextureEditor
{
	tke::Image *image = nullptr;
	bool opened = true;

	void show();
};

extern TextureEditor *textureEditor;