#include "..\src\core.h"

#include "editor.h"
#include "attribute.h"

void AttributeWidget::show()
{
	ImGui::BeginDock("Attribute", &opened);
	ImGui::EndDock();

	if (!opened)
	{
		attributeWidget = nullptr;
		delete this;
	}
}

AttributeWidget *attributeWidget = nullptr;