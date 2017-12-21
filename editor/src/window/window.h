#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../TK_Engine/src/utils.h"

struct IWindow;

struct IWindowClass
{
	virtual std::string getName() { return ""; };
	virtual IWindow *load(tke::AttributeTreeNode *) { return nullptr; };
};

extern std::vector<IWindowClass*> windowClasses;

struct IWindow
{
	IWindowClass *pClass;
	bool opened = true;
	bool _need_focus = false;

	IWindow(IWindowClass *);
	void show();
	virtual void do_show() = 0;
	virtual void save(tke::AttributeTreeNode *) {}
	virtual ~IWindow() {}
};

extern std::vector<std::unique_ptr<IWindow>> windows;

void initWindow();
