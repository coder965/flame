#include "..\src\gui.h"
#include "..\src\core.h"

#include "debug.h"

void DebugWidget::show()
{
	ImGui::BeginDock("Debug##widget", &opened);

	ImGui::Image((ImTextureID)tke::debugImages[0].second->index, ImVec2(tke::debugImages[0].second->cx, tke::debugImages[0].second->cx));

	ImGui::EndDock();
}

DebugWidget *debugWidget = nullptr;