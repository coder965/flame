#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../TK_Engine/src/utils.h"

struct Window;

struct WindowClass
{
	virtual std::string getName() { return ""; };
	virtual Window *load(tke::AttributeTreeNode *) { return nullptr; };
};

extern std::vector<WindowClass*> windowClasses;

struct Window
{
	WindowClass *pClass;
	bool opened = true;
	bool _need_focus = false;

	Window(WindowClass *);
	void show();
	virtual void do_show() = 0;
	virtual void save(tke::AttributeTreeNode *) {}
	virtual ~Window() {}
};

extern std::vector<std::unique_ptr<Window>> windows;

void initWindow();
