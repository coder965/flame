#define NOMINMAX
#include <Windows.h>

#include "input.h"

namespace tke
{
	std::string get_clipBoard()
	{
		OpenClipboard(NULL);
		auto hMemory = ::GetClipboardData(CF_TEXT);
		std::string str((char*)GlobalLock(hMemory));
		GlobalUnlock(hMemory);
		CloseClipboard();
		return str;
	}

	void set_clipBoard(const std::string &s)
	{
		auto hGlobalMemory = GlobalAlloc(GHND, s.size() + 1);
		strcpy((char*)GlobalLock(hGlobalMemory), s.c_str());
		GlobalUnlock(hGlobalMemory);
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hGlobalMemory);
		CloseClipboard();
	}
}
