namespace dialog_scene
{
	bool opened;

	void show()
	{
		if (!opened)
			return;

		ImGui::Begin("Scene", &opened, ImGuiWindowFlags_MenuBar);

		static tke::AnimationTemplate *selectAnimTemp = nullptr;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New"))
				{
				}
				if (ImGui::MenuItem("Open"))
				{
					//pMainWindow->m_pUiDialogs->openFileDialog.start();
				}
				if (ImGui::MenuItem("Save"))
				{
					//pMainWindow->m_pUiDialogs->saveFileDialog.start();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (ImGui::CollapsingHeader("Ambient"))
		{
			bool changed = false;

			if (ImGui::ColorEdit3("Color", &tke::scene->ambient.color[0])) changed = true;
			if (ImGui::TreeNode("Screen Space Ambient Occlusion"))
			{
				if (ImGui::DragFloat("Radius", &tke::scene->ambient.ssaoRadius, 0.1f, 0.f, 10000.f)) changed = true;
				if (ImGui::DragFloat("Bias", &tke::scene->ambient.ssaoBias, 0.01f, 0.f, 100.f)) changed = true;
				if (ImGui::DragFloat("Intensity", &tke::scene->ambient.ssaoIntensity, 0.1f, 0.f, 10000000.f)) changed = true;
				ImGui::TreePop();
			}

			if (changed) tke::scene->ambient.set();
		}

		if (ImGui::CollapsingHeader("Sky"))
		{
			const char *skyTypeName[] = {
				"Atmosphere Scattering", 
				"Panorama"
			};
			if (ImGui::Combo("Type", (int*)&tke::scene->skyType, skyTypeName, 2))
			{

			}
			if (ImGui::TreeNode("Atmosphere Scattering"))
			{
				bool changed = false;

				if (ImGui::DragFloat("SunDirX", &tke::scene->atmosphere.sunDir.x, 1.f)) changed = true;
				if (ImGui::DragFloat("SunDirY", &tke::scene->atmosphere.sunDir.y, 1.f)) changed = true;
				if (ImGui::DragFloat("eSun", &tke::scene->atmosphere.eSun, 0.1f)) changed = true;
				if (ImGui::DragFloat("Inner Radius", &tke::scene->atmosphere.innerRadius, 0.1f)) changed = true;
				if (ImGui::DragFloat("Outer Radius", &tke::scene->atmosphere.outerRadius, 0.1f)) changed = true;
				if (ImGui::DragFloat("Camera Height", &tke::scene->atmosphere.cameraHeight, 0.1f)) changed = true;
				if (ImGui::DragFloat("Km", &tke::scene->atmosphere.km, 0.0001f)) changed = true;
				if (ImGui::DragFloat("Kr", &tke::scene->atmosphere.kr, 0.0001f)) changed = true;

				if (changed) tke::scene->atmosphere.set();

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Panorama"))
			{
				//ImGui::Text("Sky Map:");
				//if (tke::scene->skyImage)
				//{
				//	ImGui::SameLine();
				//	ImGui::Text("%s", tke::scene->skyImage->m_fileName);
				//}
				//ImGui::SameLine();
				//if (ImGui::Button("...##SelectSkyMap"))
				//{
				//	//enumNodeForTexturePopup.start(&tke3_scene->skyTexture);
				//}

				//ImGui::Text("Radiance Map:");
				//if (tke::scene->radianceImage)
				//{
				//	ImGui::SameLine();
				//	ImGui::Text("%s", tke::scene->radianceImage->m_fileName);
				//}
				//ImGui::SameLine();
				//if (ImGui::Button("...##SelectRadianceMap"))
				//{
				//	//enumNodeForTexturePopup.start(&tke3_scene->radianceTexture);
				//}

				//ImGui::Text("Irradiance Map:");
				//if (tke::scene->irradianceImage)
				//{
				//	ImGui::SameLine();
				//	ImGui::Text("%s", tke::scene->irradianceImage->m_fileName);
				//}
				//ImGui::SameLine();
				//if (ImGui::Button("...##SelectIrradianceMap"))
				//{
				//	//enumNodeForTexturePopup.start(&tke3_scene->irradianceTexture);
				//}

				////enumNodeForTexturePopup.show();

				//ImGui::TreePop();
			}
		}

		if (ImGui::CollapsingHeader("Fog"))
		{
			ImGui::DragFloat("Thickness", &tke::scene->fogThickness, 0.01f);
		}

		if (ImGui::CollapsingHeader("HDR"))
		{
			bool changed = false;

			if (ImGui::DragFloat("Exposure", &tke::scene->hdr.exposure, 0.01f)) changed = true;
			if (ImGui::DragFloat("White", &tke::scene->hdr.white, 0.1f)) changed = true;

			if (changed) tke::scene->hdr.set();
		}

		if (ImGui::CollapsingHeader("Model"))
		{
			static tke::Model *selectModel = nullptr;
			for (auto m : tke::scene->pModels)
			{
				if (ImGui::Selectable(m->name.c_str(), selectModel == m))
					selectModel = m;
			}

			if (ImGui::Button("Open"))
			{
				pMainWindow->uiDialogs->openFileDialog.start([](const std::string &str) {
					auto pModel = tke::createModel(str);
					if (pModel) tke::scene->pModels.push_back(pModel);
				});
			}
		}

		if (ImGui::CollapsingHeader("Animation Template"))
		{
			for (auto a : tke::scene->pAnimTemps)
			{
				if (ImGui::Selectable(a->name.c_str(), selectAnimTemp == a))
					selectAnimTemp = a;
			}
			if (ImGui::Button("Load"))
			{
				pMainWindow->uiDialogs->openFileDialog.start([](const std::string &str) {
					auto pAnimTemp = tke::createAnimation(str);
					if (pAnimTemp) tke::scene->pAnimTemps.push_back(pAnimTemp);
				});
			}
			ImGui::SameLine();
			if (ImGui::Button("Save As"))
			{
				if (selectAnimTemp)
				{
					pMainWindow->uiDialogs->saveFileDialog.start([](const std::string &str) {
						tke::TKA::save(selectAnimTemp, str);
					});
				}
			}
		}

		if (ImGui::CollapsingHeader("Camera"))
		{
			glm::vec3 coord = tke::scene->camera.getCoord();
			glm::vec3 ang = tke::scene->camera.getEuler();
			auto length = tke::scene->camera.m_length;
			if (ImGui::DragFloat3("Coord", &coord[0], 0.1f))
				tke::scene->camera.setCoord(coord);
			if (ImGui::DragFloat2("Ang", &ang[0], 0.1f))
				tke::scene->camera.setEuler(ang);
			if (ImGui::DragFloat("Length", &length, 0.1f))
				tke::scene->camera.setLength(length);
		}

		if (ImGui::CollapsingHeader("Light"))
		{
			for (int i = 0; i < tke::scene->pLights.size(); i++)
			{
				auto p = tke::scene->pLights[i];
				if (ImGui::Selectable((std::to_string(i) + " " + p->getTypeName()).c_str(), selectType == SelectType::eLight && p == selectLight()))
					select(p);
			}
		}

		if (ImGui::CollapsingHeader("Object"))
		{
			for (int i = 0; i < tke::scene->pObjects.size(); i++)
			{
				auto p = tke::scene->pObjects[i];
				if (ImGui::Selectable((std::to_string(i) + " " + p->pModel->name).c_str(), selectType == SelectType::eObject && p == selectObject()))
					select(p);
			}
		}

		if (ImGui::CollapsingHeader("Terrain"))
		{
			for (int i = 0; i < tke::scene->pTerrains.size(); i++)
			{
				auto p = tke::scene->pTerrains[i];
				if (ImGui::Selectable(std::to_string(i).c_str(), selectType == SelectType::eTerrain && p == selectTerrain()))
					select(p);
			}
		}

		ImGui::End();
	}
}
