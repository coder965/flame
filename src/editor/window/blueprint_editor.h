#pragma once

#include "../../ui/ui.h"

struct BlueprintEditor : tke::ui::Window
{
	BlueprintEditor();
	~BlueprintEditor();
	virtual void on_show() override;
};

extern BlueprintEditor *blueprint_editor;
