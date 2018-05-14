#pragma once

#ifdef _FLAME_SYSTEM_EXPORTS
#define FLAME_SYSTEM_EXPORTS __declspec(dllexport)
#else
#define FLAME_SYSTEM_EXPORTS __declspec(dllimport)
#endif

#include <flame/string.h>

#include <mutex>
#include <vector>
#include <memory>
#include <functional>

namespace flame
{
	FLAME_SYSTEM_EXPORTS void *get_hinst();
	FLAME_SYSTEM_EXPORTS int get_screen_cx();
	FLAME_SYSTEM_EXPORTS int get_screen_cy();
	FLAME_SYSTEM_EXPORTS void get_app_path(MediumString *out);
	FLAME_SYSTEM_EXPORTS void exec(const char *filename, const char *parameters, LongString *output /* could be nullptr */);

	FLAME_SYSTEM_EXPORTS void get_clipboard(LongString *out);
	FLAME_SYSTEM_EXPORTS void set_clipboard(const char *s);

	struct FileWatcher;

	enum FileWatcherMode
	{
		FileWatcherModeAll,
		FileWatcherModeContent
	};

	enum FileChangeType
	{
		FileAdded,
		FileRemoved,
		FileModified,
		FileRenamed
	};

	FLAME_SYSTEM_EXPORTS FileWatcher *add_file_watcher(FileWatcherMode mode, const char *filepath, 
		const std::function<void(FileChangeType type, const char *filename)> &callback);
	FLAME_SYSTEM_EXPORTS void remove_file_watcher(FileWatcher *w);

	FLAME_SYSTEM_EXPORTS void read_process_memory(void *process, void *address, int size, void *dst);

	FLAME_SYSTEM_EXPORTS void *add_global_key_listener(int key, const std::function<void()> &callback);
	FLAME_SYSTEM_EXPORTS void remove_global_key_listener(int key, void *p /* whitch is the return of add_global_key_listener */ );
}
