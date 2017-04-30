namespace dialog_terrainAttribute
{
	int magicNum = 0;

	struct Window
	{
		int wID;
		char name[50];
		bool opened = true;
		bool first = true;

		tke::Terrain *pTerrain;

		void show()
		{
			if (pTerrain->dying)
			{
				opened = false;
				pTerrain->release();
				return;
			}

			if (first)
			{
				ImGui::SetNextWindowPos(ImVec2(500, 300));
				ImGui::SetNextWindowSize(ImVec2(600, 300));
				first = false;
			}
			ImGui::Begin(name, &opened, ImGuiWindowFlags_NoSavedSettings);

			ImGui::Text("id:%d", pTerrain->m_id);

			auto coord = pTerrain->getCoord();
			if (ImGui::DragFloat3("Coord", &coord[0], 0.1f))
				tke::moveTransformer(pTerrain, coord);
			auto euler = pTerrain->getEuler();
			if (ImGui::DragFloat3("Rotate", &euler[0], 0.1f))
				tke::setTransformerEuler(pTerrain, euler);
			auto scale = pTerrain->getScale();
			if (ImGui::DragFloat3("Scale", &scale[0], 0.1f))
				tke::scaleTransformer(pTerrain, scale);

			ImGui::DragFloat("Ext", &pTerrain->ext, 1.f, 0.f, 500.f);
			ImGui::DragFloat("Height", &pTerrain->height, 1.f, 0.f, 500.f);

			ImGui::Text("Height Map:");
			ImGui::SameLine();
			if (pTerrain->heightMap) ImGui::Text("%s", pTerrain->heightMap->filename.c_str());
			ImGui::SameLine();
			if (ImGui::Button("...##SelectHeightMap"))
			{
				//enumNodeForTexturePopup.start(&pTerrain->heightMap);
			}

			ImGui::Text("Color Map:");
			ImGui::SameLine();
			if (pTerrain->colorMap) ImGui::Text("%s", pTerrain->colorMap->filename.c_str());
			ImGui::SameLine();
			if (ImGui::Button("...##SelectColorMap"))
			{
				//enumNodeForTexturePopup.start(&pTerrain->colorMap);
			}

			//enumNodeForTexturePopup.show();

			ImGui::DragFloat("Spec", &pTerrain->spec, 0.01f, 0.f, 1.f);
			ImGui::DragFloat("Roughness", &pTerrain->roughness, 0.01f, 0.f, 1.f);

			ImGui::End();
		}
	};

	std::vector<Window*> windows;

	void addWindow(tke::Terrain *pTerrain)
	{
		auto p = new Window;
		p->wID = magicNum++;
		wsprintf(p->name, "%s##%d", "Terrain Attribute", p->wID);
		p->pTerrain = pTerrain;
		pTerrain->getRefrence();
		windows.push_back(p);
	}

	bool isOpened()
	{
		return windows.size() > 0;
	}

	void show()
	{
		for (auto it = windows.begin(); it != windows.end(); )
		{
			if (!(*it)->opened)
			{
				delete *it;
				it = windows.erase(it);
			}
			else
			{
				it++;
			}
		}

		for (auto &w : windows)
		{
			w->show();
		}
	}
}
