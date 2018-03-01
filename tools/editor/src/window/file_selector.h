#pragma once

#include <vector>
#include <functional>

#include <flame/utils/filesystem.h>
#include <flame/engine/system.h>
#include <flame/ui/ui.h>

enum FileSelectorIo
{
	FileSelectorOpen,
	FileSelectorSave
};

enum FileSelectorCreateFlag
{
	FileSelectorCreateFlagNull,
	FileSelectorNoFiles = 1 << 0,
	FileSelectorNoRightArea = 1 << 1,
	FileSelectorTreeMode = 1 << 2
};

struct FileSelector : flame::ui::Window
{
	bool io_mode;
	bool enable_file;
	bool enable_right_area = false;
	bool tree_mode;

	ImGui::Splitter splitter;

	struct ItemData
	{
		std::string value;
		std::string name;
		std::string filename;
	};

	struct DirItem;
	struct FileItem;

	struct DirItem : ItemData
	{
		std::vector<std::unique_ptr<DirItem>> dir_list;
		std::vector<std::unique_ptr<FileItem>> file_list;
	};

	struct FileItem : ItemData
	{
		flame::FileType file_type = flame::FileTypeUnknown;

		virtual ~FileItem() {}
	};

	char filename[260];
	DirItem curr_dir;
	std::string default_dir;
	std::list<std::string> curr_dir_hierarchy;
	int select_index = -1;
	DirItem *select_dir = nullptr;
	bool need_refresh = true;

	std::function<bool(std::string)> callback;

	std::unique_ptr<flame::FileWatcherHandler> file_watcher;

	FileSelector(const std::string &_title, FileSelectorIo io, const std::string &_default_dir = "", 
		unsigned int window_flags = flame::ui::WindowCreateFlagNull, unsigned int flags = FileSelectorCreateFlagNull);
	void set_current_path(const std::string &s);
	void refresh();
	virtual void on_show() override;
	virtual bool on_refresh();
	virtual FileItem *on_new_file_item();
	virtual void on_add_file_item(FileItem *i);
	virtual void on_dir_item_selected(DirItem *i);
	virtual void on_file_item_selected(FileItem *i, bool doubleClicked);
	virtual void on_top_area_show();
	virtual void on_bottom_area_show();
	virtual void on_right_area_show();
};

struct DirSelectorDialog : FileSelector
{
	DirSelectorDialog();
	static void open(const std::string &default_dir, const std::function<bool(std::string)> &_callback);
};
