#pragma once

struct DebugWidget
{
	bool opened = true;

	void show();
};

extern DebugWidget *debugWidget;