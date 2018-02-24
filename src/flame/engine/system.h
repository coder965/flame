#pragma once

#include <string>
#include <mutex>
#include <vector>

namespace tke
{
	void *get_hinst();
	int get_screen_cx();
	int get_screen_cy();
	std::string get_exe_path();
	void exec(const std::string &filename, const std::string &parameters);

	std::string get_clipBoard();
	void set_clipBoard(const std::string &);

	struct FileWatcher
	{
		bool dirty;
		void *hEventExpired;

		FileWatcher();
	};

	struct FileWatcherHandler
	{
		FileWatcher *ptr;

		~FileWatcherHandler();
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

	std::unique_ptr<FileWatcherHandler> add_file_watcher(FileWatcherMode mode, const std::string &filepath, std::function<void(const std::vector<FileChangeInfo> infos)> callback = nullptr);

	std::string create_process_and_get_output(const std::string &filename, const std::string &command_line);
}
