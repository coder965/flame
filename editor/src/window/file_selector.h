#pragma once

#include <vector>
#include <functional>

#include "window.h"

struct FileSelector : Window
{
	bool enable_file;
	int mode; // 0 - open, 1 - save

	struct DirItem
	{
		std::string value;
		std::string name;
	};

	struct FileItem : tke::vdtor
	{
		std::string value;
		std::string name;

		int file_size;
		tke::FileType file_type = tke::FileTypeUnknown;
	};

	int driver_index = 0;
	std::experimental::filesystem::path current_path;
	char filename[260];
	std::vector<std::unique_ptr<DirItem>> dir_list;
	std::vector<std::unique_ptr<FileItem>> file_list;
	int list_index = -1;
	bool need_refresh = true;

	std::function<bool(std::string)> callback;

	FileSelector(WindowClass*_pclass, bool _enable_file, int _mode);
	void set_current_path(const std::string &s);
	void refresh();
	virtual void show() override;
	virtual int on_left_area_width();
	virtual bool on_refresh();
	virtual bool on_parent_path();
	virtual FileItem *on_new_file_item();
	virtual void on_add_file_item(FileItem *i);
	virtual bool on_window_begin() = 0;
	virtual void on_window_end() = 0;
	virtual void on_dir_item_selected(DirItem *i);
	virtual void on_file_item_selected(FileItem *i, bool doubleClicked);
	virtual void on_top_area_begin();
	virtual void on_bottom_area_begin();
	virtual void on_right_area_begin();
};

struct DirSelectorDialog : FileSelector
{
	bool first = true;
	
	DirSelectorDialog();
	virtual bool on_window_begin() override;
	virtual void on_window_end() override;
	static void open(const std::string &default_dir, const std::function<bool(std::string)> &_callback);
};
