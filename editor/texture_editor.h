#pragma once

#include "..\src\render.h"

struct TextureEditor
{
	tke::Image *image = nullptr;
	bool opened = true;

	void show();
};

extern TextureEditor *textureEditor;