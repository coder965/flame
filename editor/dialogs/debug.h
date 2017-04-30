namespace dialog_debug
{
	bool opened;

	ImGuiTextBuffer log;

	void add_str(char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		log.append(fmt, args);
		va_end(args);
	}

	glm::vec4 input;
	glm::vec4 result;

	void show()
	{
		if (!opened)
		{
			return;
		}

		ImGui::Begin("Debug##editor", &opened);

		if (ImGui::CollapsingHeader("Log"))
		{
			if (ImGui::Button("Clear"))
				log.clear();
			ImGui::BeginChild("");
			ImGui::TextUnformatted(log.begin(), log.end());
			ImGui::EndChild();
		}

		if (ImGui::CollapsingHeader("Calculate"))
		{
			ImGui::InputFloat4("Input", &input[0], 0.1f);
			ImGui::InputFloat4("Result", &result[0], 0.1f);
			if (ImGui::Button("OK"))
			{
			}
		}

		//if (ImGui::CollapsingHeader("Debug Cube"))
		//{
		//	static int index = -1;
		//	if (tke3_debug_cubes.size() > 0)
		//	{
		//		ImGui::ListBox("", &index, tke3_getNumString, nullptr, tke3_debug_cubes.size());
		//	}
		//	if (ImGui::Button("Add"))
		//	{
		//		ImGui::OpenPopup("Debug Cube Attribute");
		//	}
		//	if (ImGui::Button("Add Camera Frustum"))
		//	{
		//		tke3DebugCube p;
		//		p.create(tke3_scene->camera.frustumPoints);
		//		tke3_debug_cubes.push_back(p);
		//	}
		//	if (ImGui::BeginPopupModal("Debug Cube Attribute", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		//	{
		//		static glm::vec3 n0, n1, n2, n3, n4;
		//		static glm::vec3 f0, f1, f2, f3, f4;

		//		ImGui::InputFloat3("N0", &n0[0], 0.1f);
		//		ImGui::InputFloat3("N1", &n1[0], 0.1f);
		//		ImGui::InputFloat3("N2", &n2[0], 0.1f);
		//		ImGui::InputFloat3("N3", &n3[0], 0.1f);
		//		ImGui::InputFloat3("F0", &f0[0], 0.1f);
		//		ImGui::InputFloat3("F1", &f1[0], 0.1f);
		//		ImGui::InputFloat3("F2", &f2[0], 0.1f);
		//		ImGui::InputFloat3("F3", &f3[0], 0.1f);

		//		if (ImGui::Button("OK"))
		//		{
		//			tke3DebugCube p;
		//			p.create(n0, n1, n2, n3,
		//				f0, f1, f2, f3);
		//			tke3_debug_cubes.push_back(p);

		//			ImGui::CloseCurrentPopup();
		//		}
		//		ImGui::SameLine();
		//		if (ImGui::Button("Cancel"))
		//			ImGui::CloseCurrentPopup();
		//		ImGui::EndPopup();
		//	}
		//	ImGui::SameLine();
		//	if (ImGui::Button("Delete"))
		//	{
		//		if (index != -1)
		//		{
		//			tke3DebugCube *p = &tke3_debug_cubes[index];
		//			p->destroy();
		//			tke3_debug_cubes.erase(tke3_debug_cubes.begin() + index);
		//		}
		//	}
		//}

		ImGui::End();
	}
}
