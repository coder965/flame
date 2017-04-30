namespace dialog_image
{
#if (TKE_GRAPHICS_API == TKE_OPENGL)
	int width_max = 4096;
	int height_max = 4096;

	tkeFramebuffer fbo;

	enum class ImageType
	{
		null,
		perlinGray,
		perlinColor,
		debug
	};

	const char *nameOfType[] = {
		"NULL",
		"Perlin Gray",
		"Perlin Color",
		"Debug"
	};

	const char *nameOfFormat[] = {
		"float",
		"int",
		"uint"
	};

	unsigned int hashOfTexture[] = {
		CHASH("tex"),
		CHASH("texArray"),
		CHASH("texCube"),
		CHASH("itex"),
		CHASH("itexArray"),
		CHASH("itexCube"),
		CHASH("utex"),
		CHASH("utexArray"),
		CHASH("utexCube")
	};

	const char *nameOfCubeFace[] = {
		"x+", "x-",
		"y+", "y-",
		"z+", "z-"
	};

	glm::vec4 channelMaskColor[] = {
		glm::vec4(1.f),
		glm::vec4(1.f, 0.f, 0.f, 1.f),
		glm::vec4(0.f, 1.f, 0.f, 1.f),
		glm::vec4(0.f, 0.f, 1.f, 1.f)
	};

	int magicNum = 0;

	struct Window
	{
		int wID;
		char name[50];
		bool opened = true;
		bool first = true;

		tkeTexture tex;

		ImageType type = ImageType::null;
		int debugIndex = -1;
		int width = 800, height = 600;
		int currentFormat = 0;
		int currentLevel = 0;
		int currentDepth = 0;
		int currentCubeFace = 0;
		int channel = 0;
		float radius = 5.f;
		bool depthMode = false;

		bool autoRefresh = false;

		int frameWidth = 800, frameHeight = 600;

		Window()
		{
			tex.create(tkeTexture::Type::e2D, width_max, height_max, tkeTexture::Format::eRGBA8, 1, 0,
				tkeTexture::Filter::eLinear, tkeTexture::Filter::eLinear);
		}
		void show()
		{
			if (first)
			{
				ImGui::SetNextWindowPos(ImVec2(500, 300));
				ImGui::SetNextWindowSize(ImVec2(600, 300));
				first = false;
			}
			ImGui::Begin(name, &opened, ImGuiWindowFlags_NoSavedSettings);

			tkeTexture *pTexture = nullptr;
			ImGui::Combo("Type", (int*)&type, nameOfType, 4);
			if (type == ImageType::perlinGray || type == ImageType::perlinColor)
			{
				ImGui::DragInt("Image Width", &width, 1.f, 0, width_max);
				ImGui::DragInt("Image Height", &height, 1.f, 0, height_max);
				ImGui::DragFloat("Radius", &radius, 0.1f, 0.f, 10000.f);
			}
			else if (type == ImageType::debug)
			{
				if (tke_textureDebugList.size() == 0)
				{
					debugIndex = -1;
				}
				else
				{
					ImGui::Combo("Texture", &debugIndex, [](void *, int index, const char **out_text) {
						*out_text = tke_textureDebugList[index].name;
						return true;
					}, nullptr, tke_textureDebugList.size());
					if (debugIndex != -1)
					{
						pTexture = tke_textureDebugList[debugIndex].pTexture;
						ImGui::Combo("Current Format", &currentFormat, (const char**)nameOfFormat, 3);
						if (currentLevel >= pTexture->level)
						{
							currentLevel = 0;
						}
						ImGui::Combo("Current Level", &currentLevel, tke3_getNumString, nullptr, pTexture->level);
						width = pTexture->getWidth(currentLevel);
						height = pTexture->getHeight(currentLevel);
						if (pTexture->type == tkeTexture::Type::e2DArray)
						{
							if (currentDepth >= pTexture->depth) currentDepth = 0;
							ImGui::Combo("Current Depth", &currentDepth, tke3_getNumString, nullptr, pTexture->depth);
						}
						else if (pTexture->type == tkeTexture::Type::eCube)
						{
							ImGui::Combo("Current Cube Face", &currentCubeFace, (const char**)nameOfCubeFace, 6);
						}
						ImGui::Combo("Channel", &channel, "RGB\0R\0G\0B\0");
						if (ImGui::CollapsingHeader("Attribute"))
						{
							ImGui::Text("Type:%s", pTexture->typeName());
							ImGui::Text("ID:%d", pTexture->texture);
							ImGui::Text("Size:%d x %d", width, height);
							ImGui::Text("Format:%s", pTexture->formatName());
							ImGui::Text("Level:%d", pTexture->level);
							if (pTexture->type == tkeTexture::Type::e2DArray)
								ImGui::Text("Depth:%d", pTexture->depth);
							ImGui::Text("Filter:%s(mag) %s(min)", pTexture->filterNameMin(), pTexture->filterNameMag());
							ImGui::Text("Wrap:%s(s) %s(t)", pTexture->wrapNameS(), pTexture->wrapNameT());
						}
						ImGui::Checkbox("Depth Mode", &depthMode);
					}
				}
			}
			ImGui::Checkbox("Auto Refresh", &autoRefresh);
			bool refresh = autoRefresh;
			if (!autoRefresh)
			{
				ImGui::SameLine();
				if (ImGui::Button("Refresh"))
				{
					refresh = true;
				}
			}

			if (refresh)
			{
				fbo.present();
				fbo.attachTex(GL_COLOR_ATTACHMENT0, &tex);
				glViewport(0, 0, width, height);
				glClear(GL_COLOR_BUFFER_BIT);
				switch (type)
				{
				case ImageType::null:
					break;
				case ImageType::perlinGray: case ImageType::perlinColor:
					if (GET_PROGRAM("3D editor", "image_perlin"))
					{
						USE_PROGRAM();
						SET_UNIFORM("mode", type == ImageType::perlinGray ? 0 : 1);
						SET_UNIFORM("rad", radius);
						SET_UNIFORM("time", (float)tke_scriptTime);
						tke_vaoRect.draw();
					}
					break;
				case ImageType::debug:
					if (debugIndex != -1)
					{
						if (GET_PROGRAM("3D editor", "debug"))
						{
							USE_PROGRAM();
							tke_currentProgram->texture(hashOfTexture[currentFormat * 3 + (int)pTexture->type], pTexture);
							SET_UNIFORM("type", (int)pTexture->type);
							SET_UNIFORM("format", currentFormat);
							SET_UNIFORM("level", currentLevel);
							SET_UNIFORM("depth", currentDepth);
							SET_UNIFORM("cubeFace", currentCubeFace);
							SET_UNIFORM("depthMode", depthMode ? 1 : 0);
							SET_UNIFORM("color", channelMaskColor[channel]);
							tke_vaoRect.draw();
						}
					}
					break;
				}
				glBindFramebuffer(GL_FRAMEBUFFER, NULL);
				glViewport(0, 0, tke_resolutionCx, tke_resolutionCy);
			}

			ImGui::DragInt("Frame Width", &frameWidth, 1.f, 0, width_max);
			ImGui::DragInt("Frame Height", &frameHeight, 1.f, 0, height_max);
			ImGui::BeginChild("Image", ImVec2(frameWidth, frameHeight), false, ImGuiWindowFlags_HorizontalScrollbar);
			ImVec2 screen_pos = ImGui::GetCursorScreenPos();
			ImGui::Image((ImTextureID)tex.texture, ImVec2(width, height), ImVec2(0.f, (float)height / height_max), ImVec2((float)width / width_max, 0.f),
				ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
			auto xPos = int(ImGui::GetMousePos().x - screen_pos.x);
			auto yPos = int(ImGui::GetMousePos().y - screen_pos.y);
			if (ImGui::IsItemHovered())
			{
				unsigned int uiValue = 0;
				glm::vec3 fValue;
				if (debugIndex != -1)
				{
					if (currentFormat == 0)
					{
						if (depthMode)
							glGetTextureSubImage(pTexture->texture, 0, xPos, height - yPos - 1, 0, 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, 4, &fValue);
						else
							glGetTextureSubImage(pTexture->texture, 0, xPos, height - yPos - 1, 0, 1, 1, 1, GL_RGB, GL_FLOAT, 4 * 3, &fValue);
					}
					else if (currentFormat == 2)
					{
						glGetTextureSubImage(pTexture->texture, 0, xPos, height - yPos - 1, 0, 1, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, 4, &uiValue);
					}
					auto err = glGetError();
				}
				ImGui::BeginTooltip();
				if (currentFormat == 0)
				{
					if (depthMode)
						ImGui::Text("(%d, %d) %f", xPos, yPos, fValue.x);
					else
						ImGui::Text("(%d, %d) %f %f %f", xPos, yPos, fValue.x, fValue.y, fValue.z);
				}
				else if (currentFormat == 2)
					ImGui::Text("(%d, %d) %u", xPos, yPos, uiValue);
				ImGui::EndTooltip();
			}
			ImGui::EndChild();


			if (ImGui::Button("Export"))
			{
				tkeImgui_saveFileDialog.start();
			}
			char file[MAX_PATH];
			if (tkeImgui_saveFileDialog.show(file))
			{
				tkwin_ext_cat(file, ".bmp");
				auto temp = new unsigned char[width * height * 4];
				GLint lastFramebuffer;
				glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFramebuffer);
				fbo.present();
				fbo.attachTex(GL_COLOR_ATTACHMENT0, &tex);
				glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, temp);
				glBindFramebuffer(GL_FRAMEBUFFER, lastFramebuffer);
				auto pitch = tkImage_getPitch(width * 3);
				auto data = new unsigned char[pitch * height];
				for (int h = 0; h < height; h++)
					for (int w = 0; w < width; w++)
					{
						auto v0 = pitch * h + w * 3;
						auto v1 = (width * h + w) * 4;
						data[v0 + 0] = temp[v1 + 0];
						data[v0 + 1] = temp[v1 + 1];
						data[v0 + 2] = temp[v1 + 2];
					}
				delete[]temp;
				tkwin_saveBitmap24(file, width, height, data);
				delete[]data;
			}

			ImGui::End();
		}
	};

	std::vector<Window*> windows;

	void addWindow()
	{
		auto p = new Window;
		p->wID = magicNum++;
		wsprintf(p->name, "%s##%d", "Image", p->wID);
		windows.push_back(p);
	}

	bool isOpened()
	{
		return windows.size() > 0;
	}

	void show()
	{
		struct Init
		{
			Init()
			{
				fbo.create();
			}
		};
		static Init init;

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

		char name[50];
		for (auto &w : windows)
		{
			w->show();
		}
	}
#endif
}
