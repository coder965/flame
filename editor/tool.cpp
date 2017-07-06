#include "tool.h"

Tool::Tool(tke::Framebuffer *_fb)
	:fb(_fb)
{
	cb = new tke::CommandBuffer(tke::commandPool);
}

Tool::~Tool() {}

bool Tool::leftDown(int x, int y) { return false; }

Tool *currentTool = nullptr;