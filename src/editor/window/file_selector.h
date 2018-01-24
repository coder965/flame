#pragma once

#include <vector>
#include <functional>

#include "../../file_utils.h"
#include "../../ui/ui.h"

struct FileSelector : tke::ui::Window
{
	bool enable_file;
	bool enable_right_region = false;
	float left_region_width = 300.f;
	float right_region_width;
	bool save_mode;
	bool tree_mode;

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
		tke::FileType file_type = tke::FileTypeUnknown;

		virtual ~FileItem() {}
	};

	int driver_index = 0;
	char filename[260];
	DirItem curr_dir;
	int select_index = -1;
	DirItem *select_dir = nullptr;
	bool need_refresh = true;

	std::function<bool(std::string)> callback;
	std::string user_define_extra_path;

	// mode: 0 - for open, 1 - for save
	FileSelector(const std::string &_title, bool _modal, bool _enable_file, bool _enable_right_region, bool _save_mode, int _cx = 0, int _cy = 0, bool _tree_mode = false);
	void set_current_path(const std::string &s);
	void refresh();
	virtual void on_show() override;
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
