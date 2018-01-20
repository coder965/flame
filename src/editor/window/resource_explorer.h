#pragma once

#include <vector>
#include <memory>

#include "../../graphics/image.h"

#include "file_selector.h"

struct ResourceExplorer : FileSelector
{
	ResourceExplorer();
	virtual ~ResourceExplorer() override;
	virtual void on_file_item_selected(FileItem *i, bool doubleClicked) override;
	virtual void on_top_area_show() override;
	virtual void on_bottom_area_show() override;
	virtual void on_right_area_show() override;
};

extern ResourceExplorer *resourceExplorer;