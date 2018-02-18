#pragma once

#include <flame/ui/ui.h>

struct TextEditor : tke::ui::Window
{
	TextEditor();
	~TextEditor();
	virtual void on_show() override;
};

extern TextEditor *text_editor;
