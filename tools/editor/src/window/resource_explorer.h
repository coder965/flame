#pragma once

#include <vector>
#include <memory>

#include <flame/graphics/texture.h>

#include "file_selector.h"

struct ResourceExplorer : FileSelector
{
	ResourceExplorer();
	virtual ~ResourceExplorer() override;
	virtual void on_right_area_show() override;
};

extern ResourceExplorer *resourceExplorer;