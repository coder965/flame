#pragma once

#include <vector>

#include "../TK_Engine/src/utils.h"
#include "../TK_Engine/src/render/image.h"
#include "file_selector.h"

struct ResourceExplorerClass : WindowClass
{
	virtual std::string getName() override;
	virtual Window *load(tke::AttributeTreeNode *n) override;
};

extern ResourceExplorerClass resourceExplorerClass;

struct ResourceExplorerFileItem : FileSelector::FileItem
{
	std::shared_ptr<tke::Image> image;

	~ResourceExplorerFileItem();
};

struct ResourceExplorer : FileSelector
{
	ResourceExplorer();
	virtual ~ResourceExplorer() override;
	virtual int on_left_area_width() override;
	virtual FileItem *on_new_file_item() override;
	virtual void on_file_item_selected(FileItem *i, bool doubleClicked) override;
	virtual void on_top_area_begin() override;
	virtual void on_bottom_area_begin() override;
	virtual void on_right_area_begin() override;
};

extern ResourceExplorer *resourceExplorer;