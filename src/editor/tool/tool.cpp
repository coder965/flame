#include "tool.h"

Tool::Tool(tke::Framebuffer *_fb)
	:fb(_fb)
{
}

Tool::~Tool() {}

bool Tool::leftDown(int x, int y) { return false; }
void Tool::mouseMove(int xDisp, int yDisp) {}

Tool *currentTool = nullptr;