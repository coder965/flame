#pragma once

#include <string>
#include <mutex>
#include <vector>
#include <memory>

#include <flame/exports.h>

namespace flame
{
	FLAME_EXPORTS void *get_hinst();
	FLAME_EXPORTS int get_screen_cx();
	FLAME_EXPORTS int get_screen_cy();
	FLAME_EXPORTS std::string get_app_path();
	FLAME_EXPORTS void exec(const std::string &filename, const std::string &parameters);
	FLAME_EXPORTS std::string exec_and_get_output(const std::string &filename, const std::string &command_line);

	FLAME_EXPORTS std::string get_clipBoard();
	FLAME_EXPORTS void set_clipBoard(const std::string &);

	struct FileWatcher
	{
		bool dirty;
		void *hEventExpired;
	};

	enum FileWatcherMode
	{
		FileWatcherModeAll,
		FileWatcherModeContent
	};

	enum FileChangeType
	{
		FileChangeAdded,
		FileChangeRemoved,
		FileChangeModified,
		FileChangeRename
	};

	struct FileChangeInfo
	{
		FileChangeType type;
		std::string filename;
	};

	FLAME_EXPORTS FileWatcher *add_file_watcher(FileWatcherMode mode, const std::string &filepath, const std::function<void(const std::vector<FileChangeInfo> &infos)> &callback = nullptr);
	FLAME_EXPORTS void remove_file_watcher(FileWatcher *w);

	FLAME_EXPORTS void read_process_memory(void *process, void *address, int size, void *dst);

	FLAME_EXPORTS void *add_global_key_listener(int key, const std::function<void()> &callback);
	FLAME_EXPORTS void remove_global_key_listener(int key, void *p);
}
