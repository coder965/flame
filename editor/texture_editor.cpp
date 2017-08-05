#include "..\src\gui.h"

#include "texture_editor.h"

void TextureEditor::show()
{
	ImGui::BeginDock("Texture Editor", &opened);

	if (ImGui::Button("New"))
	{

	}
	ImGui::SameLine();
	if (ImGui::Button("Load"))
	{

	}
	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{

	}

	if (image)
	{
		if (ImGui::Button("Remove"))
		{
			tke::removeGuiImage(image);
			delete image;
		}
	}
	else
	{
		ImGui::Text("[No Image]");
	}

	ImGui::EndDock();
}

TextureEditor *textureEditor = nullptr;