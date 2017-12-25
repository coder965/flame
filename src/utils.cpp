#include <stdio.h>
#include <assert.h>
#include <sstream>
#include <filesystem>
#define NOMINMAX
#include <Windows.h>

#include "utils.h"
#include "file_utils.h"

namespace tke
{
	int lineNumber(const char *str)
	{
		int lineNumber = 0;
		while (*str)
		{
			if (*str == '\n')
				lineNumber++;
			str++;
		}
		return lineNumber;
	}

	const char *getErrorString(Err errNum)
	{
		switch (errNum)
		{
		case NoErr:
			return "No error.";
		case ErrInvalidEnum:
			return "Invalid enum.";
		case ErrInvalidValue:
			return "Invalid value.";
		case ErrInvalidOperation:
			return "Invalid operation.";
		case ErrOutOfMemory:
			return "Out of memory.";
		case ErrContextLost:
			return "Context lost.";
		case ErrResourceLost:
			return "Resource lost.";
		default:
			return "unknow error";
		}
	}

	void *get_hinst()
	{
		static void *hinst = nullptr;
		if (!hinst)
			hinst = GetModuleHandle(nullptr);
		return hinst;
	}

	int get_screen_cx()
	{
		static int cx = 0;
		if (cx == 0)
			cx = GetSystemMetrics(SM_CXSCREEN);
		return cx;
	}

	int get_screen_cy()
	{
		static int cy = 0;
		if (cy == 0)
			cy = GetSystemMetrics(SM_CYSCREEN);
		return cy;
	}

	std::string get_exe_path()
	{
		static std::string path;
		if (path == "")
		{
			char buf[MAX_PATH];
			GetCurrentDirectoryA(MAX_PATH, buf);
			path = buf;
		}
		return path;
	}

	const char *getClipBoard()
	{
		static std::string str;
		OpenClipboard(NULL);
		auto hClipMemory = ::GetClipboardData(CF_TEXT);
		auto dwLength = GlobalSize(hClipMemory);
		auto lpClipMemory = (LPBYTE)GlobalLock(hClipMemory);
		str =  (char*)lpClipMemory;
		GlobalUnlock(hClipMemory);
		CloseClipboard();
		return str.c_str();
	}

	void setClipBoard(const std::string &s)
	{
		auto hGlobalMemory = GlobalAlloc(GHND, s.size() + 1);
		strcpy((char*)GlobalLock(hGlobalMemory), s.c_str());
		GlobalUnlock(hGlobalMemory);
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hGlobalMemory);
		CloseClipboard();
	}

	std::string translate(int srcCP, int dstCP, const std::string &src)
	{
		auto wbuf = new wchar_t[src.size() + 1];
		MultiByteToWideChar(srcCP, 0, src.c_str(), -1, wbuf, src.size() + 1);
		auto buf = new char[src.size() + 1];
		WideCharToMultiByte(dstCP, 0, wbuf, -1, buf, src.size() + 1, NULL, false);
		delete[]wbuf;
		std::string str(buf);
		delete[]buf;
		return str;
	}

	std::string japaneseToChinese(const std::string &src)
	{
		return translate(932, 936, src);
	}

	void exec(const std::string &filename, const std::string &parameters)
	{
		SHELLEXECUTEINFOA info = {};
		info.cbSize = sizeof(SHELLEXECUTEINFOA);
		info.fMask = SEE_MASK_NOCLOSEPROCESS;
		info.lpVerb = "open";
		info.lpFile = filename.c_str();
		info.lpParameters = parameters.c_str();
		ShellExecuteExA(&info);
		WaitForSingleObject(info.hProcess, INFINITE);
	}
}
