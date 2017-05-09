namespace dialog_model_editor
{
	bool opened;

	bool showRgSelect = true;
	bool showController = false;
	bool showBounding = false;
	bool showEyePosition = false;
	bool showWeaponPosition = false;

	tke::Model *selectingModel;

	int rgIndex = 0;

	tke::Rigidbody *selectingRigidbody;

	tke::RenderGroupTemplate *prg = nullptr;

	void show()
	{
		if (!opened)
			return;

		ImGui::Begin("Model Editor", &opened, ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save Model"))
				{
					if (selectingModel)
					{
						pMainWindow->m_uiDialogs->yesNoDialog.start("Need Copy Texture?", [](bool opt) {
							tke::TKM::save(selectingModel, selectingModel->filepath + selectingModel->name + ".tkm", opt);
						});
					}
					else
					{
						tke::report("You need select a model.");
					}
				}
				if (ImGui::MenuItem("Save Model As"))
				{
					if (selectingModel)
					{
						static std::string filename;
						pMainWindow->m_uiDialogs->saveFileDialog.start([](const std::string &str) {
							filename = str;
							pMainWindow->m_uiDialogs->yesNoDialog.start("Need Copy Texture?", [](bool opt) {
								tke::TKM::save(selectingModel, filename, opt);
							});
						});
					}
					else
					{
						tke::report("You need select a model.");
					}
				}
				if (ImGui::MenuItem("Reload Data"))
				{
					if (selectingModel)
					{
						pMainWindow->m_uiDialogs->openFileDialog.start([](const std::string &str) {
							selectingModel->loadDat(str);
						});
					}
					else
					{
						//tke3_notiBoard.add("You need select a model.");
					}
				}
				if (ImGui::MenuItem("Save Data"))
				{
					if (selectingModel)
					{
						pMainWindow->m_uiDialogs->saveFileDialog.start([](const std::string &str) {
							selectingModel->saveDat(str);
						});
					}
					else
					{
						//tke3_notiBoard.add("You need select a model.");
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		static int modelIndex = 0;
		ImGui::Combo("Model", &modelIndex, [](void *, int index, const char **out_text) {
			if (index == 0)
				*out_text = "None";
			else
				*out_text = tke::scene->pModels[index - 1]->name.c_str();
			return true;
		}, nullptr, tke::scene->pModels.size() + 1);
		if (modelIndex == 0)
			selectingModel = nullptr;
		else
			selectingModel = tke::scene->pModels[modelIndex - 1];

		if (selectingModel)
		{
			if (ImGui::CollapsingHeader("Render Group"))
			{
				ImGui::BeginChild("##rgList", ImVec2(200, 200));
				ImGui::BeginGroup();

				if (selectingModel->renderGroups.size() > 0)
				{
					ImGui::ListBox("", &rgIndex, [](void *data, int index, const char **out_text) {
						static std::string text;
						if (index == 0)
						{
							*out_text = "-1";
							return true;
						}
						text = std::to_string(index - 1);
						*out_text = text.c_str();
						return true;
					}, nullptr, selectingModel->renderGroups.size() + 1);
				}
				else
				{
					rgIndex = 0;
				}

				ImGui::Checkbox("select line", &showRgSelect);

				ImGui::EndGroup();
				ImGui::EndChild();
				if (rgIndex != 0)
				{
					prg = &selectingModel->renderGroups[rgIndex - 1];

					ImGui::SameLine();

					ImGui::BeginGroup();
					
					auto type = (int)prg->type;
					if (ImGui::Combo("Type", &type, [](void *, int index, const char **out_text) {
						*out_text = tke::RenderGroupTemplate::getTypeName((tke::RenderGroupTemplate::Type)index);
						return true;
					}, nullptr, (int)tke::RenderGroupTemplate::Type::eLast))
					{
						prg->type = (tke::RenderGroupTemplate::Type)type;
					}

					ImGui::Text("Indice Base:%d", prg->indiceBase);
					ImGui::Text("Indice Count:%d", prg->indiceCount);

					if (ImGui::Checkbox("Visible", &prg->visible))
					{
					}

					auto pmt = &selectingModel->renderGroups[rgIndex - 1].material;
					ImGui::Text("Material:%s", pmt->name);
					ImGui::SameLine();
					if (ImGui::Button("...##SelectMaterial"))
					{
						ImGui::OpenPopup("Select Material");

					}

					if (ImGui::BeginPopupModal("Select Material", nullptr, 0))
					{
						tke::guiCurrentWindow->m_uiAcceptedMouse = true;
						tke::guiCurrentWindow->m_uiAcceptedKey = true;

						if (ImGui::Button("Cancel", ImVec2(120, 0))) 
						{ 
							ImGui::CloseCurrentPopup(); 
						}
						ImGui::EndPopup();
					}

					ImGui::EndGroup();
				}
			}

			if (ImGui::CollapsingHeader("Rigid Body"))
			{
				for (int i = 0; i < selectingModel->rigidbodies.size(); i++)
				{
					auto p = selectingModel->rigidbodies[i];
					if (ImGui::Selectable((std::to_string(i) + " " + p->name).c_str(), p == selectingRigidbody))
					{
						selectingRigidbody = p;
						tke::select(selectingRigidbody);
					}
				}

				if (ImGui::Button("Add Static"))
				{
					auto pRigidbody = new tke::Rigidbody;
					pRigidbody->mode = tke::Rigidbody::Mode::eStatic;
					selectingModel->addRigidbody(pRigidbody);
				}
				ImGui::SameLine();
				if (ImGui::Button("Add Dynamic"))
				{
					auto pRigidbody = new tke::Rigidbody;
					pRigidbody->mode = tke::Rigidbody::Mode::eDynamic;
					selectingModel->addRigidbody(pRigidbody);
				}
				ImGui::SameLine();
				if (ImGui::Button("Add Dynamic Lock Location"))
				{
					auto pRigidbody = new tke::Rigidbody;
					pRigidbody->mode = tke::Rigidbody::Mode::eDynamicLockLocation;
					selectingModel->addRigidbody(pRigidbody);
				}
				ImGui::SameLine();
				if (ImGui::Button("Delete"))
				{
					if (selectingRigidbody)
					{
						selectingRigidbody = selectingModel->deleteRigidbody(selectingRigidbody);
						if (tke::selectType == tke::SelectType::eRigidbody)
							tke::select(selectingRigidbody);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Up"))
				{
					if (selectingRigidbody)
					{
						for (int i = 0; i < selectingModel->rigidbodies.size(); i++)
						{
							if (selectingModel->rigidbodies[i] == selectingRigidbody && i > 0)
								std::swap(selectingModel->rigidbodies[i - 1], selectingModel->rigidbodies[i]);
						}
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Down"))
				{
					if (selectingRigidbody)
					{
						for (int i = 0; i < selectingModel->rigidbodies.size(); i++)
						{
							if (selectingModel->rigidbodies[i] == selectingRigidbody && i < selectingModel->rigidbodies.size() - 1)
								std::swap(selectingModel->rigidbodies[i + 1], selectingModel->rigidbodies[i]);
						}
					}
				}

				if (selectingRigidbody)
				{
					auto coord = selectingRigidbody->getCoord();
					if (ImGui::DragFloat3("Coord", &coord[0], 0.1f))
						tke::moveTransformer(tke::SelectType::eRigidbody, selectingRigidbody, coord);
					auto euler = selectingRigidbody->getEuler();
					if (ImGui::DragFloat3("Rotate", &euler[0], 0.1f))
						tke::setTransformerEuler(tke::SelectType::eRigidbody, selectingRigidbody, euler);

					int boneID = selectingRigidbody->boneID + 1;
					if (ImGui::Combo("Bone ID", &boneID, [](void *data, int index, const char **out_text) {
						static std::string text;
						text = std::to_string(index - 1);
						*out_text = text.c_str();
						return true;
					}, nullptr, selectingModel->bones.size() + 1))
						selectingRigidbody->boneID = boneID - 1;

					ImGui::DragInt("Group Number", &selectingRigidbody->originCollisionGroupID);
					ImGui::DragInt("Flag", &selectingRigidbody->originCollisionFreeFlag);

					if (ImGui::TreeNode("Shape"))
					{
						for (int i = 0; i < selectingRigidbody->shapes.size(); i++)
						{
							auto p = selectingRigidbody->shapes[i];
							if (ImGui::Selectable((std::to_string(i) + " " + p->getTypeName()).c_str(), tke::selectType == tke::SelectType::eShape && p == tke::selectShape()))
								tke::select(p);
						}
						if (ImGui::Button("Add"))
						{
							auto p = new tke::Shape;
							p->type = tke::Shape::Type::eBox;
							selectingRigidbody->addShape(p);
							tke::select(p);
						}
						ImGui::SameLine();
						if (ImGui::Button("Duplicate"))
						{
							if (tke::selectType == tke::SelectType::eShape)
							{
								auto p = new tke::Shape(*tke::selectShape());
								selectingRigidbody->addShape(p);
								tke::select(p);
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Delete"))
						{
							if (tke::selectType == tke::SelectType::eShape)
							{
								auto p = selectingRigidbody->deleteShape(tke::selectShape());
								tke::select(p);
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Up"))
						{
							if (tke::selectType == tke::SelectType::eShape)
							{
								for (int i = 0; i < selectingRigidbody->shapes.size(); i++)
								{
									if (selectingRigidbody->shapes[i] == tke::selectShape() && i > 0)
										std::swap(selectingRigidbody->shapes[i - 1], selectingRigidbody->shapes[i]);
								}
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Down"))
						{
							if (tke::selectType == tke::SelectType::eShape)
							{
								for (int i = 0; i < selectingRigidbody->shapes.size(); i++)
								{
									if (selectingRigidbody->shapes[i] == tke::selectShape() && i < selectingRigidbody->shapes.size() - 1)
										std::swap(selectingRigidbody->shapes[i - 1], selectingRigidbody->shapes[i]);
								}
							}
						}

						if (tke::selectType == tke::SelectType::eShape)
						{
							ImGui::Combo("Shape", (int*)&tke::selectShape()->type, [](void *data, int index, const char **out_text) {
								*out_text = tke::Shape::getTypeName(tke::Shape::Type(index));
								return true;
							}, nullptr, (int)tke::Shape::Type::eLast);
							auto coord = tke::selectShape()->getCoord();
							if (ImGui::DragFloat3("Coord", &coord[0], 0.1f))
								tke::moveTransformer(tke::SelectType::eShape, tke::selectShape(), coord);
							auto euler = tke::selectShape()->getEuler();
							if (ImGui::DragFloat3("Rotate", &euler[0], 0.1f))
								tke::setTransformerEuler(tke::SelectType::eShape, tke::selectShape(), euler);
							auto scale = tke::selectShape()->getScale();
							if (ImGui::DragFloat3("Scale", &scale[0], 0.1f))
								tke::scaleTransformer(tke::SelectType::eShape, tke::selectShape(), scale);
						}

						ImGui::TreePop();
					}

				}
			}

			if (ImGui::CollapsingHeader("Joint"))
			{
				for (int i = 0; i < selectingModel->joints.size(); i++)
				{
					auto p = selectingModel->joints[i];
					if (ImGui::Selectable(std::to_string(i).c_str(), p == tke::selectJoint()))
						tke::select(p);
				}

				if (tke::selectType == tke::SelectType::eJoint)
				{
					auto coord = tke::selectJoint()->getCoord();
					if (ImGui::DragFloat3("Coord", &coord[0], 0.1f))
						tke::moveTransformer(tke::SelectType::eJoint, tke::selectJoint(), coord);
					auto euler = tke::selectJoint()->getEuler();
					if (ImGui::DragFloat3("Rotate", &euler[0], 0.1f))
						tke::setTransformerEuler(tke::SelectType::eJoint, tke::selectJoint(), euler);

					auto funGetRigid = [](void *pModel, int index, const char **out_text) {
						if (index == 0)
						{
							*out_text = "none";
							return true;
						}
						*out_text = ((tke::Model*)pModel)->rigidbodies[index - 1]->name.c_str();
						return true;
					};
					int rigidID;

					rigidID = tke::selectJoint()->rigid0ID + 1;
					if (ImGui::Combo("Rigid 0", &rigidID, funGetRigid, selectingModel, selectingModel->joints.size() + 1))
						tke::selectJoint()->rigid0ID = rigidID - 1;
					rigidID = tke::selectJoint()->rigid1ID + 1;
					if (ImGui::Combo("Rigid 0", &rigidID, funGetRigid, selectingModel, selectingModel->joints.size() + 1))
						tke::selectJoint()->rigid1ID = rigidID - 1;
				}
			}

			if (ImGui::CollapsingHeader("Misc"))
			{
				ImGui::Checkbox("show controller", &showController);
				ImGui::DragFloat3("Controller Position", &selectingModel->controllerPosition[0], 0.1f);
				ImGui::DragFloat("Controller Height", &selectingModel->controllerHeight, 0.1f);
				ImGui::DragFloat("Controller Radius", &selectingModel->controllerRadius, 0.1f);
				ImGui::Checkbox("show bounding", &showBounding);
				ImGui::DragFloat3("Bounding Position", &selectingModel->boundingPosition[0], 0.1f);
				ImGui::DragFloat("Bounding Size", &selectingModel->boundingSize, 0.1f);
				ImGui::Checkbox("show eye", &showEyePosition);
				ImGui::DragFloat3("Eye Position", &selectingModel->eyePosition[0], 0.1f);
				ImGui::Checkbox("show weapon", &showWeaponPosition);
				ImGui::DragFloat3("Weapon Position", &selectingModel->mainWeaponPosition[0], 0.1f);
			}

			if (selectingModel->animated)
			{
				if (ImGui::CollapsingHeader("Animation"))
				{
					static tke::Animation *selectAnimation = nullptr;
					for (int i = 0; i < selectingModel->animations.size(); i++)
					{
						auto a = selectingModel->animations[i];
						if (ImGui::Selectable((std::to_string(i) + " " + a->pTemplate->name).c_str(), selectAnimation == a))
							selectAnimation = a;
					}
					if (ImGui::Button("Add"))
						ImGui::OpenPopup("SelectAnimDialog");
					ImGui::SameLine();
					if (ImGui::Button("Delete"))
					{
						if (selectAnimation)
						{
							if (selectingModel->animationStand == selectAnimation)
								selectingModel->animationStand = nullptr;
							if (selectingModel->animationForward == selectAnimation)
								selectingModel->animationForward = nullptr;
							if (selectingModel->animationLeft == selectAnimation)
								selectingModel->animationLeft = nullptr;
							if (selectingModel->animationRight == selectAnimation)
								selectingModel->animationRight = nullptr;
							if (selectingModel->animationBackward == selectAnimation)
								selectingModel->animationBackward = nullptr;
							if (selectingModel->animationJump == selectAnimation)
								selectingModel->animationJump = nullptr;

							for (auto it = selectingModel->animations.begin(); it != selectingModel->animations.end(); it++)
							{
								if (*it == selectAnimation)
								{
									selectingModel->animations.erase(it);
									break;
								}
							}
						}
					}

					if (ImGui::BeginPopupModal("SelectAnimDialog", nullptr, 0))
					{
						for (auto a : tke::scene->pAnimTemps)
						{
							if (ImGui::Selectable(a->name.c_str()))
								selectingModel->bindAnimation(a);
						}

						if (ImGui::Button("Cancel", ImVec2(120, 0)))
							ImGui::CloseCurrentPopup();

						ImGui::EndPopup();
					}

					int ID;
					int animCount = selectingModel->animations.size() + 1;

					ID = funGetAnimID(selectingModel, selectingModel->animationStand);
					if (ImGui::Combo("Stand", &ID, funGetAnimName, selectingModel, animCount))
						selectingModel->animationStand = funGetAnim(selectingModel, ID);
					ID = funGetAnimID(selectingModel, selectingModel->animationForward);
					if (ImGui::Combo("Forward", &ID, funGetAnimName, selectingModel, animCount))
						selectingModel->animationForward = funGetAnim(selectingModel, ID);
					ID = funGetAnimID(selectingModel, selectingModel->animationLeft);
					if (ImGui::Combo("Left", &ID, funGetAnimName, selectingModel, animCount))
						selectingModel->animationLeft = funGetAnim(selectingModel, ID);
					ID = funGetAnimID(selectingModel, selectingModel->animationRight);
					if (ImGui::Combo("Right", &ID, funGetAnimName, selectingModel, animCount))
						selectingModel->animationRight = funGetAnim(selectingModel, ID);
					ID = funGetAnimID(selectingModel, selectingModel->animationBackward);
					if (ImGui::Combo("Backward", &ID, funGetAnimName, selectingModel, animCount))
						selectingModel->animationBackward = funGetAnim(selectingModel, ID);
					ID = funGetAnimID(selectingModel, selectingModel->animationJump);
					if (ImGui::Combo("Jump", &ID, funGetAnimName, selectingModel, animCount))
						selectingModel->animationJump = funGetAnim(selectingModel, ID);
				}
			}
		}

		ImGui::End();
	}
}
