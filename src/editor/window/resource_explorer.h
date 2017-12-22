#pragma once

#include <vector>

#include "../../utils.h"
#include "../../render/image.h"
#include "file_selector.h"

extern const char *basic_model_names[8];

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
	virtual void on_refresh_user_define_dir() override;
	virtual void on_file_item_selected(FileItem *i, bool doubleClicked) override;
	virtual void on_top_area_show() override;
	virtual void on_bottom_area_show() override;
	virtual void on_right_area_show() override;
};

extern ResourceExplorer *resourceExplorer;