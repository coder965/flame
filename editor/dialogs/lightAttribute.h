namespace dialog_lightAttribute
{
	int magicNum = 0;

	struct Window
	{
		int wID;
		char name[50];
		bool opened = true;
		bool first = true;

		tke::Light *pLight;

		void show()
		{
			if (pLight->dying)
			{
				opened = false;
				pLight->release();
				return;
			}

			if (first)
			{
				ImGui::SetNextWindowPos(ImVec2(500, 300));
				ImGui::SetNextWindowSize(ImVec2(600, 300));
				first = false;
			}
			ImGui::Begin(name, &opened, ImGuiWindowFlags_NoSavedSettings);

			ImGui::Text("id:%d", pLight->m_id);

			auto coord = pLight->getCoord();
			if (ImGui::DragFloat3("Coord", &coord[0], 0.1f))
				moveTransformer(pLight, coord);
			auto euler = pLight->getEuler();
			if (ImGui::DragFloat3("Rotate", &euler[0], 0.1f))
				setTransformerEuler(pLight, euler);
			ImGui::ColorEdit3("Color", &pLight->color[0]);
			if (pLight->type == tke::Light::Type::ePoint)
				ImGui::DragFloat3("Decay Factor", &pLight->decayFactor[0], 0.1f, 0.f, 10000.f);

			ImGui::End();
		}
	};

	std::vector<Window*> windows;

	void addWindow(tke::Light *pLight)
	{
		auto p = new Window;
		p->wID = magicNum++;
		wsprintf(p->name, "%s##%d", "Light Attribute", p->wID);
		p->pLight = pLight;
		pLight->getRefrence();
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
			w->show();
	}
}
