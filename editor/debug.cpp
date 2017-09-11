#include "../src/ui/ui.h"
#include "../src/core.h"

#include "debug.h"

void DebugWidget::show()
{
	ImGui::BeginDock("Debug##widget", &opened);

	ImGui::Image((ImTextureID)tke::debugImages[0].second->index, ImVec2(tke::debugImages[0].second->levels[0].cx, tke::debugImages[0].second->levels[0].cx));

	ImGui::EndDock();
}

DebugWidget *debugWidget = nullptr;