#pragma once

#include <vector>

#include "../TK_Engine/src/render/image.h"
#include "window.h"

struct ResourceExplorerClass : WindowClass
{
	virtual std::string getName() override;
	virtual Window *load(tke::AttributeTreeNode *n) override;
};

extern ResourceExplorerClass resourceExplorerClass;

void load_resource();

struct ResourceExplorerDirListItem
{
	std::string value;
	std::string name;
};

struct ResourceExplorerFileListItem
{
	std::string value;
	std::string name;

	int file_size;

	enum FileType
	{
		FileTypeFile,
		FileTypeText,
		FileTypeImage,
		FileTypeModel,
		FileTypeScene
	};
	FileType file_type = FileTypeFile;

	std::shared_ptr<tke::Image> image;

	~ResourceExplorerFileListItem();
};

struct ResourceExplorer : Window
{
	std::experimental::filesystem::path path;
	std::vector<std::unique_ptr<ResourceExplorerDirListItem>> dir_list;
	std::vector<std::unique_ptr<ResourceExplorerFileListItem>> file_list;
	int list_index = -1;
	bool need_refresh = true;

	ResourceExplorer();
	virtual ~ResourceExplorer() override;
	void refresh();
	virtual void show() override;
};

extern ResourceExplorer *resourceExplorer;