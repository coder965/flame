#pragma once

#include <vector>
#include <memory>

#include "../../graphics/image.h"

#include "file_selector.h"

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
	virtual void on_top_area_show() override;
	virtual void on_bottom_area_show() override;
	virtual void on_right_area_show() override;
};

extern ResourceExplorer *resourceExplorer;