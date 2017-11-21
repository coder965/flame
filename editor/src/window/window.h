#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../TK_Engine/src/utils.h"

struct Window;

struct WindowClass
{
	virtual std::string getName() = 0;
	virtual Window *load(tke::AttributeTreeNode *) = 0;
};

extern std::vector<WindowClass*> windowClasses;

struct Window
{
	WindowClass *pClass;
	bool opened;

	Window(WindowClass *);
	virtual void show() = 0;
	virtual void save(tke::AttributeTreeNode *) {}
};

extern std::vector<std::unique_ptr<Window>> windows;

void initWindow();
