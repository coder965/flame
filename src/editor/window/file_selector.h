#pragma once

#include <vector>
#include <functional>

#include "../../file_utils.h"
#include "window.h"

struct FileSelector : IWindow
{
	std::string title;
	bool modal;

	bool enable_file;
	int mode; // 0 - open, 1 - save

	struct DirItem
	{
		std::string value;
		std::string name;
		std::string filename;
	};

	struct FileItem
	{
		std::string value;
		std::string name;
		std::string filename;

		int file_size = 0;
		tke::FileType file_type = tke::FileTypeUnknown;

		virtual ~FileItem() {}
	};

	int driver_index = 0;
	std::experimental::filesystem::path current_path;
	char filename[260];
	std::vector<std::unique_ptr<DirItem>> dir_list;
	std::vector<std::unique_ptr<FileItem>> file_list;
	int list_index = -1;
	bool need_refresh = true;

	std::function<bool(std::string)> callback;
	std::string user_define_extra_path;

	// mode: 0 - open, 1 - save
	FileSelector(const std::string &_title, bool _modal, bool _enable_file, int _mode, int _cx = 0, int _cy = 0);
	void set_current_path(const std::string &s);
	void refresh();
	virtual void do_show() override;
	virtual int on_left_area_width();
	virtual bool on_refresh();
	virtual bool on_parent_path();
	virtual FileItem *on_new_file_item();
	virtual void on_add_file_item(FileItem *i);
	virtual void on_refresh_user_define_dir();
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
