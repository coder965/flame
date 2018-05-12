#define NOMINMAX
#include <Windows.h>
#include <assert.h>
#include <thread>
#include <list>
#include <map>

#include <flame/global.h>
#include <flame/filesystem.h>
#include <flame/system.h>

namespace flame
{
	void *get_hinst()
	{
		return GetModuleHandle(nullptr);
	}

	int get_screen_cx()
	{
		return GetSystemMetrics(SM_CXSCREEN);
	}

	int get_screen_cy()
	{
		return GetSystemMetrics(SM_CYSCREEN);
	}

	void get_app_path(MediumString *out)
	{
		GetModuleFileName(nullptr, out->data, sizeof(out->data));
		auto path = std::filesystem::path(out->data).parent_path().string();
		strncpy(out->data, path.data(), sizeof(out->data));
	}

	void exec(const char *filename, const char *parameters, LongString *output)
	{
		if (!output)
		{
			SHELLEXECUTEINFOA info = {};
			info.cbSize = sizeof(SHELLEXECUTEINFOA);
			info.fMask = SEE_MASK_NOCLOSEPROCESS;
			info.lpVerb = "open";
			info.lpFile = filename;
			info.lpParameters = parameters;
			ShellExecuteExA(&info);
			WaitForSingleObject(info.hProcess, INFINITE);

			return;
		}

		HANDLE hChildStd_OUT_Rd = NULL;
		HANDLE hChildStd_OUT_Wr = NULL;

		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		assert(CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0));

		assert(SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0));

		char cl_buf[1024 * 8];
		{
			auto tail = cl_buf;
			if (filename[0] == 0)
				tail = strncpy(cl_buf, filename, sizeof(cl_buf));
			strncpy(tail, parameters, sizeof(cl_buf) - (tail - cl_buf));
		}
		cl_buf[sizeof(cl_buf) - 1] = 0;

		STARTUPINFO start_info = {};
		start_info.cb = sizeof(STARTUPINFO);
		start_info.hStdError = hChildStd_OUT_Wr;
		start_info.hStdOutput = hChildStd_OUT_Wr;
		start_info.dwFlags |= STARTF_USESTDHANDLES;
		PROCESS_INFORMATION proc_info = {};
		if (!CreateProcess(filename[0] == 0 ? nullptr : filename, cl_buf, NULL, NULL, TRUE, 0, NULL, NULL, &start_info, &proc_info))
		{
			auto e = GetLastError();
			assert(0);
		}

		WaitForSingleObject(proc_info.hProcess, INFINITE);

		CloseHandle(proc_info.hProcess);
		CloseHandle(proc_info.hThread);

		DWORD size;
		PeekNamedPipe(hChildStd_OUT_Rd, NULL, NULL, NULL, &size, NULL);
		if (size > sizeof(output->data))
			size = sizeof(output->data);
		PeekNamedPipe(hChildStd_OUT_Rd, (void*)output->data, size, NULL, NULL, NULL);
		output->data[size] = 0;
	}

	void get_clipboard(LongString *out)
	{
		OpenClipboard(NULL);
		auto hMemory = GetClipboardData(CF_TEXT);
		strcpy(out->data, (char*)GlobalLock(hMemory));
		GlobalUnlock(hMemory);
		CloseClipboard();
	}

	void set_clipboard(const char *s)
	{
		auto hGlobalMemory = GlobalAlloc(GHND, strlen(s) + 1);
		strcpy((char*)GlobalLock(hGlobalMemory), s);
		GlobalUnlock(hGlobalMemory);
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hGlobalMemory);
		CloseClipboard();
	}

	struct FileWatcher
	{
		void *hEventExpired;
	};

	FileWatcher *add_file_watcher(FileWatcherMode mode, const char *filepath, 
		void(*callback)(FileChangeType type, const char *filename, void *user_data), void *user_data)
	{
		auto w = new FileWatcher;
		w->hEventExpired = CreateEvent(NULL, false, false, NULL);

		std::thread new_thread([=]() {
			auto dir_handle = CreateFileA(filepath, GENERIC_READ | GENERIC_WRITE |
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

				auto base = 0;
				auto p = (FILE_NOTIFY_INFORMATION*)notify_buf;
				while (true)
				{
					MediumString filename;
					auto str_size = WideCharToMultiByte(CP_ACP, 0, p->FileName, p->FileNameLength / sizeof(wchar_t), NULL, 0, NULL, NULL);
					if (str_size > sizeof(filename.data))
						str_size = sizeof(filename.data);
					WideCharToMultiByte(CP_ACP, 0, p->FileName, p->FileNameLength / sizeof(wchar_t), (char*)filename.data, str_size, NULL, NULL);
					if (*filename.find('~') != 0)
					{
						FileChangeType type;
						switch (p->Action)
						{
							case 0x1:
								type = FileAdded;
								break;
							case 0x2:
								type = FileRemoved;
								break;
							case 0x3:
								type = FileModified;
								break;
							case 0x4:
								type = FileRenamed;
								break;
							case 0x5:
								type = FileRenamed;
								break;
						}
						callback(type, filename.data, user_data);
					}

					if (p->NextEntryOffset <= 0)
						break;
					base += p->NextEntryOffset;
					p = (FILE_NOTIFY_INFORMATION*)(notify_buf + base);
				}
			}
		});
		new_thread.detach();

		return w;
	}

	void remove_file_watcher(FileWatcher *w)
	{
		SetEvent(w->hEventExpired);
	}

	void read_process_memory(void *process, void *address, int size, void *dst)
	{
		SIZE_T ret_byte;
		assert(ReadProcessMemory(process, address, dst, size, &ret_byte));
	}

	struct Listener
	{
		void(*callback)(void *);
		void *user_data;
	};

	static HHOOK global_key_hook = 0;
	static std::map<int, std::list<Listener>> global_key_listeners;

	LRESULT CALLBACK global_key_callback(int nCode, WPARAM wParam, LPARAM lParam)
	{
		auto kbhook = (KBDLLHOOKSTRUCT*)lParam;

		auto it = global_key_listeners.find(kbhook->vkCode);
		if (it != global_key_listeners.end())
		{
			for (auto &e : it->second)
				e.callback(e.user_data);
		}

		return CallNextHookEx(global_key_hook, nCode, wParam, lParam);
	}

	void *add_global_key_listener(int key, void(*callback)(void *user_data), void *user_data)
	{
		void *ret;

		auto it = global_key_listeners.find(key);
		if (it == global_key_listeners.end())
			it = global_key_listeners.emplace(key, std::list<Listener>()).first;
		it->second.push_back({callback, user_data});
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
