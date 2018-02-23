#define NOMINMAX
#include <Windows.h>
#include <thread>

#include <flame/global.h>
#include <flame/engine/system.h>
#include <flame/utils/filesystem.h>

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
			std::filesystem::path _p(buf);
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

	std::unique_ptr<FileWatcherHandler> add_file_watcher(const std::string &filepath, std::function<void()> callback)
	{
		auto w = new FileWatcher;
		w->handle = FindFirstChangeNotification(filepath.c_str(), true,
			FILE_NOTIFY_CHANGE_FILE_NAME |
			FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_LAST_WRITE);
		assert(w->handle != INVALID_HANDLE_VALUE);
		w->callback = callback;

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
				{
					ww->dirty = true;
					if (ww->callback)
						ww->callback();
				}
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

	std::string create_process_and_get_output(const std::string &filename, const std::string &command_line)
	{
		HANDLE g_hChildStd_OUT_Rd = NULL;
		HANDLE g_hChildStd_OUT_Wr = NULL; 

		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL; 

		assert(CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0));

		assert(SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0));

		STARTUPINFO start_info = {}; 
		start_info.cb = sizeof(STARTUPINFO);
		start_info.hStdError = g_hChildStd_OUT_Wr;
		start_info.hStdOutput = g_hChildStd_OUT_Wr;
		start_info.dwFlags |= STARTF_USESTDHANDLES;
		PROCESS_INFORMATION proc_info = {};
		auto success = CreateProcess(filename.c_str(), (char*)command_line.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &start_info, &proc_info);

		WaitForSingleObject(proc_info.hProcess, INFINITE);

		CloseHandle(proc_info.hProcess);
		CloseHandle(proc_info.hThread);

		DWORD size;
		PeekNamedPipe(g_hChildStd_OUT_Rd, NULL, NULL, NULL, &size, NULL);
		std::string str;
		str.resize(size);
		PeekNamedPipe(g_hChildStd_OUT_Rd, (void*)str.data(), size, NULL, NULL, NULL);

		return str;
	}
}
