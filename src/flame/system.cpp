#define NOMINMAX
#include <Windows.h>

#include "system.h"
#include "file_utils.h"

namespace tke
{
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
			char buf[260];
			GetModuleFileName(nullptr, buf, 260);
			std::fs::path _p(buf);
			path = _p.parent_path().string();
		}
		return path;
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
