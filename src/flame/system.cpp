#define NOMINMAX
#include <Windows.h>
#include <assert.h>
#include <thread>
#include <list>
#include <map>

#include <flame/global.h>
#include <flame/filesystem.h>
#include <flame/string.h>
#include <flame/system.h>

namespace flame
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

	std::string get_app_path()
	{
		static std::string path;
		if (path == "")
		{
			char buf[260];
			GetModuleFileName(nullptr, buf, 260);
			path = std::filesystem::path(buf).parent_path().string();
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

	std::string exec_and_get_output(const std::string &filename, const std::string &command_line)
	{
		HANDLE g_hChildStd_OUT_Rd = NULL;
		HANDLE g_hChildStd_OUT_Wr = NULL;

		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		assert(CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0));

		assert(SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0));

		char cl_buf[260];
		cl_buf[0] = 0;
		assert(command_line.size() < sizeof(cl_buf));
		if (filename.empty())
			strcpy(cl_buf, filename.c_str());
		strcat(cl_buf, command_line.c_str());

		STARTUPINFO start_info = {};
		start_info.cb = sizeof(STARTUPINFO);
		start_info.hStdError = g_hChildStd_OUT_Wr;
		start_info.hStdOutput = g_hChildStd_OUT_Wr;
		start_info.dwFlags |= STARTF_USESTDHANDLES;
		PROCESS_INFORMATION proc_info = {};
		if (!CreateProcess(filename.empty() ? nullptr : filename.c_str(), cl_buf, NULL, NULL, TRUE, 0, NULL, NULL, &start_info, &proc_info))
		{
			auto e = GetLastError();
			assert(0);
		}

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

	FileWatcher *add_file_watcher(FileWatcherMode mode, const std::string &filepath, const std::function<void(const std::vector<FileChangeInfo> &infos)> &callback)
	{
		auto w = new FileWatcher;
		w->dirty = false;
		w->hEventExpired = CreateEvent(NULL, false, false, NULL);

		std::thread new_thread([=]() {
			auto dir_handle = CreateFileA(filepath.c_str(), GENERIC_READ | GENERIC_WRITE |
				FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS,
				NULL);
			assert(dir_handle != INVALID_HANDLE_VALUE);

			BYTE notify_buf[1024];

			OVERLAPPED overlapped = {};
			auto hEvent = CreateEvent(NULL, false, false, NULL);

			auto flags = FILE_NOTIFY_CHANGE_FILE_NAME |
				FILE_NOTIFY_CHANGE_DIR_NAME |
				FILE_NOTIFY_CHANGE_CREATION |
				FILE_NOTIFY_CHANGE_LAST_WRITE;

			while (true)
			{
				ZeroMemory(&overlapped, sizeof(OVERLAPPED));
				overlapped.hEvent = hEvent;

				assert(ReadDirectoryChangesW(dir_handle, notify_buf, sizeof(notify_buf), true, flags,
					NULL, &overlapped, NULL));

				HANDLE events[] = {
					overlapped.hEvent,
					w->hEventExpired
				};
				
				if (WaitForMultipleObjects(2, events, false, INFINITE) - WAIT_OBJECT_0 == 1)
				{
					CloseHandle(dir_handle);
					delete w;
					break;
				}

				DWORD ret_bytes;
				assert(GetOverlappedResult(dir_handle, &overlapped, &ret_bytes, false) == 1);

				w->dirty = true;
				if (callback)
				{
					std::vector<FileChangeInfo> infos;
					auto base = 0;
					auto p = (FILE_NOTIFY_INFORMATION*)notify_buf;
					while (true)
					{
						std::string filename;
						auto str_size = WideCharToMultiByte(CP_ACP, 0, p->FileName, p->FileNameLength / sizeof(wchar_t), NULL, 0, NULL, NULL);
						filename.resize(str_size);
						WideCharToMultiByte(CP_ACP, 0, p->FileName, p->FileNameLength / sizeof(wchar_t), (char*)filename.data(), str_size, NULL, NULL);
						if (filename.find('~') != std::string::npos)
						{
							FileChangeType type;
							switch (p->Action)
							{
								case 0x1:
									type = FileChangeAdded;
									break;
								case 0x2:
									type = FileChangeRemoved;
									break;
								case 0x3:
									type = FileChangeModified;
									break;
								case 0x4:
									type = FileChangeRename;
									break;
								case 0x5:
									type = FileChangeRename;
									break;
							}
							infos.push_back({type, filename});
						}

						if (p->NextEntryOffset <= 0)
							break;
						base += p->NextEntryOffset;
						p = (FILE_NOTIFY_INFORMATION*)(notify_buf + base);
					}
					callback(infos);
				}
			}
		});
		new_thread.detach();

		return w;
	}

	void remove_file_watcher(FileWatcher *w)
	{
		SetEvent(w->hEventExpired);
		delete w;
	}

	void read_process_memory(void *process, void *address, int size, void *dst)
	{
		SIZE_T ret_byte;
		assert(ReadProcessMemory(process, address, dst, size, &ret_byte));
	}

	static HHOOK global_key_hook = 0;
	static std::map<int, std::list<std::function<void()>>> global_key_listeners;

	LRESULT CALLBACK global_key_callback(int nCode, WPARAM wParam, LPARAM lParam)
	{
		auto kbhook = (KBDLLHOOKSTRUCT*)lParam;

		auto it = global_key_listeners.find(kbhook->vkCode);
		if (it != global_key_listeners.end())
		{
			for (auto &e : it->second)
				e();
		}

		return CallNextHookEx(global_key_hook, nCode, wParam, lParam);
	}

	void *add_global_key_listener(int key, const std::function<void()> &callback)
	{
		void *ret;

		auto it = global_key_listeners.find(key);
		if (it == global_key_listeners.end())
			it = global_key_listeners.emplace(key, std::list<std::function<void()>>()).first;
		it->second.emplace_back(callback);
		ret = &it->second.back();

		if (global_key_hook == 0)
			global_key_hook = SetWindowsHookEx(WH_KEYBOARD_LL, global_key_callback, (HINSTANCE)get_hinst(), 0);

		return ret;
	}

	void remove_global_key_listener(int key, void *p)
	{
		auto it = global_key_listeners.find(key);
		if (it == global_key_listeners.end())
			return;

		for (auto _it = it->second.begin(); _it != it->second.end(); _it++)
		{
			if (&(*_it) == p)
			{
				it->second.erase(_it);
				break;
			}
		}

		if (it->second.empty())
			global_key_listeners.erase(it);

		if (global_key_listeners.empty())
		{
			if (global_key_hook)
			{
				UnhookWindowsHookEx(global_key_hook);
				global_key_hook = 0;
			}
		}
	}
}
