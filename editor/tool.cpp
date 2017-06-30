#include "tool.h"

Tool::Tool(tke::Framebuffer *_fb)
	:fb(_fb)
{
	cb = new tke::CommandBuffer(tke::commandPool);
	event = tke::createEvent();
}

Tool::~Tool() {}

Tool *currentTool = nullptr;