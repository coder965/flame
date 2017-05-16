namespace dialog_objectAttribute
{
	int magicNum = 0;

	struct Window
	{
		int wID;
		char name[50];
		bool opened = true;
		bool first = true;

		tke::Object *pObject;

		void show()
		{
			if (pObject->dying)
			{
				opened = false;
				pObject->release();
				return;
			}

			if (first)
			{
				ImGui::SetNextWindowPos(ImVec2(500, 300));
				ImGui::SetNextWindowSize(ImVec2(600, 300));
				first = false;
			}
			ImGui::Begin(name, &opened, ImGuiWindowFlags_NoSavedSettings);

			auto pModel = pObject->pModel;

			ImGui::Text("id:%d", pObject->m_id);
			ImGui::Text("Model:%s", pModel->name);

			auto coord = pObject->getCoord();
			if (ImGui::DragFloat3("Coord", &coord[0], 0.1f))
				moveTransformer(pObject, coord);
			auto euler = pObject->getEuler();
			if (ImGui::DragFloat3("Rotate", &euler[0], 0.1f))
				setTransformerEuler(pObject, euler);
			auto scale = pObject->getScale();
			if (ImGui::DragFloat3("Scale", &scale[0], 0.1f))
				scaleTransformer(pObject, scale);

			if (pModel->animated)
			{
				auto ID = funGetAnimID(pModel, pObject->animationSolver->pAnimation);
				if (ImGui::Combo("Current Animation", &ID, funGetAnimName, pModel, pModel->animations.size() + 1))
				{
					pObject->animationSolver->pAnimation = funGetAnim(pModel, ID);
				}
			}

			ImGui::Checkbox("Phyx", &pObject->phyx);
			if (ImGui::TreeNode("Speed"))
			{
				ImGui::DragFloat("Front", &pObject->frontSpeed, 0.1f);
				ImGui::DragFloat("Back", &pObject->backSpeed, 0.1f);
				ImGui::DragFloat("Up", &pObject->upSpeed, 0.1f);
				ImGui::DragFloat("Left", &pObject->leftSpeed, 0.1f);
				ImGui::DragFloat("Right", &pObject->rightSpeed, 0.1f);
				ImGui::DragFloat("Down", &pObject->downSpeed, 0.1f);
				ImGui::DragFloat("Turn Left", &pObject->turnSpeed[0], 0.1f);
				ImGui::DragFloat("Turn Right", &pObject->turnSpeed[1], 0.1f);

				ImGui::TreePop();
			}
			ImGui::Combo("Move Type", (int*)&pObject->moveType, "Normal\0By animation (Lock Y)\0\0");

			ImGui::End();
		}
	};

	std::vector<Window*> windows;

	void addWindow(tke::Object *pObject)
	{
		auto p = new Window;
		p->wID = magicNum++;
		wsprintf(p->name, "%s##%d", "Object Attribute", p->wID);
		p->pObject = pObject;
		pObject->getRefrence();
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
