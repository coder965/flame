#pragma once

#include <vector>
#include <memory>

#include <flame/graphics/texture.h>
#include <flame/ui/ui.h>

struct ResourceExplorer : flame::ui::FileSelector
{
	ResourceExplorer();
	virtual ~ResourceExplorer() override;
	virtual void on_right_area_show() override;
};

extern ResourceExplorer *resourceExplorer;