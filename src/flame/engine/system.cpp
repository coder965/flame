#define NOMINMAX
#include <Windows.h>
#include <thread>

#include <flame/global.h>
#include <flame/utils/filesystem.h>
#include <flame/utils/string.h>
#include <flame/engine/system.h>

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

	std::unique_ptr<FileWatcherHandler> add_file_watcher(FileWatcherMode mode, const std::string &filepath, std::function<void(const std::vector<FileChangeInfo> infos)> callback)
	{
		auto w = new FileWatcher;

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
						if (!string_contain(filename, '~'))
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

		auto h = std::make_unique<FileWatcherHandler>();
		h->ptr = w;
		return h;
	}

	FileWatcher::FileWatcher() :
		dirty(false)
	{
		hEventExpired = CreateEvent(NULL, false, false, NULL);
	}

	FileWatcherHandler::~FileWatcherHandler()
	{
		SetEvent(ptr->hEventExpired);
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
