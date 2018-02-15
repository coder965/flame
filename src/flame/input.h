#pragma once

#include <string>

namespace tke
{
	struct KeyState
	{
		bool justDown = false;
		bool justUp = false;
		bool pressing = false;
	};

	extern int mouseX;
	extern int mouseY;
	extern int mousePrevX;
	extern int mousePrevY;
	extern int mouseDispX;
	extern int mouseDispY;
	extern int mouseScroll;

	extern KeyState mouseLeft;
	extern KeyState mouseMiddle;
	extern KeyState mouseRight;

	extern KeyState keyStates[256];

	std::string get_clipBoard();
	void set_clipBoard(const std::string &);
}
