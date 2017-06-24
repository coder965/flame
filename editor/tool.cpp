#include "tool.h"

Tool::Tool()
{
	cmd = tke::commandPool.allocate();
}

Tool::~Tool()
{
	tke::commandPool.free(cmd);
}

Tool *currentTool = nullptr;