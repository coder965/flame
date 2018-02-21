#define NOMINMAX
#include <Windows.h>
#include <thread>

#include <flame/global.h>
#include <flame/engine/system.h>
#include <flame/utils/file.h>

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

	std::unique_ptr<FileWatcherHandler> add_file_watcher(const std::string &filepath)
	{
		auto w = new FileWatcher;
		w->handle = FindFirstChangeNotification(filepath.c_str(), true,
			FILE_NOTIFY_CHANGE_FILE_NAME |
			FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_LAST_WRITE);
		assert(w->handle != INVALID_HANDLE_VALUE);

		std::thread t([&]() {
			auto ww = w;
			while (true)
			{
				if (ww->expired)
				{
					FindCloseChangeNotification(ww->handle);
					delete ww;
					break;
				}

				auto r = WaitForSingleObject(ww->handle, 2000);
				if (r != WAIT_TIMEOUT)
					ww->dirty = true;
				FindNextChangeNotification(ww->handle);
			}
		});

		t.detach();

		auto h = std::make_unique<FileWatcherHandler>();
		h->ptr = w;
		return h;
	}

	FileWatcher::FileWatcher() :
		dirty(false),
		expired(false)
	{
	}

	FileWatcherHandler::~FileWatcherHandler()
	{
		ptr->expired = true;
	}
}
