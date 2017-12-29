#pragma once

#include <string>

#include "refl.h"

namespace tke
{
	struct KeyState
	{
		bool justDown = false;
		bool justUp = false;
		bool pressing = false;
	};

	IMPL() int mouseX;
	IMPL() int mouseY;
	IMPL() int mousePrevX;
	IMPL() int mousePrevY;
	IMPL() int mouseDispX;
	IMPL() int mouseDispY;
	IMPL() int mouseScroll;

	IMPL() KeyState mouseLeft;
	IMPL() KeyState mouseMiddle;
	IMPL() KeyState mouseRight;

	IMPL() KeyState keyStates[256];

	std::string get_clipBoard();
	void set_clipBoard(const std::string &);
}
