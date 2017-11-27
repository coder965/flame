#pragma once

#include <vector>
#include <functional>

#include "window.h"

struct DirSelectorListItem
{
	std::string value;
	std::string name;
};

struct DirSelector : Window
{
	std::experimental::filesystem::path path;
	std::vector<std::unique_ptr<DirSelectorListItem>> dir_list;
	int list_index = -1;
	std::string selected_path;
	std::function<void(std::string)> callback;
	bool need_refresh = true;
	bool first = true;
	
	DirSelector();
	void refresh();
	static void open(const std::string &default_dir, const std::function<void(std::string)> &_callback);
	virtual void show() override;
};
