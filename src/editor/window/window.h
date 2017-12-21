#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../../utils.h"

struct IWindow
{
	bool opened = true;
	bool _need_focus = false;

	IWindow();
	void show();
	virtual void do_show() = 0;
	virtual void save(tke::AttributeTreeNode *) {}
	virtual ~IWindow() {}
};

extern std::vector<std::unique_ptr<IWindow>> windows;
