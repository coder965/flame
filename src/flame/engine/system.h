#pragma once

#include <string>
#include <mutex>

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
		void *handle;
		bool expired;

		FileWatcher();
	};

	struct FileWatcherHandler
	{
		FileWatcher *ptr;

		~FileWatcherHandler();
	};

	std::unique_ptr<FileWatcherHandler> add_file_watcher(const std::string &filepath);

	std::string create_process_and_get_output(const std::string &filename);
}
