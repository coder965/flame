#include "select.h"
#include "editor.h"
#include "monitor.h"

MonitorWidget::MonitorWidget(const std::string _renderer_filename, tke::Model *_model)
	:renderer_filename(_renderer_filename), model(_model)
{
	renderer = new tke::Renderer;
	renderer->loadXML(renderer_filename);
	tke::setMasterRenderer(renderer);

	scene = new tke::Scene;

	image = new tke::Image(tke::resCx, tke::resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	renderer->resource.setImage(image, "Window.Image");
	tke::addGuiImage(image);
	tke::ShaderMacro macro;
	macro.pipelineName = "Deferred.Pipeline";
	macro.stageType = tke::StageType::frag;
	macro.value = "#define USE_PBR\n";
	renderer->resource.shaderMacros.push_back(macro);

	renderer->setup();

	tke::setMasterRenderer(renderer);
	scene->setRenderer(renderer);

	tke::needRedraw = true;
	tke::needUpdateTexture = true;
	scene->needUpdateSky = true;

	renderFinished = tke::createEvent();
	mainWindow->addEvent(renderFinished);

	cmd = tke::commandPool.allocate();

	auto obj = new tke::Object(model);
	scene->addObject(obj);

	selectedItem.select(obj);

	scene->camera.setMode(tke::CameraModeTargeting);
	scene->camera.setCoord(0.f, 5.f, 0.f);
}

MonitorWidget::~MonitorWidget()
{
	mainWindow->removeEvent(renderFinished);
	tke::destroyEvent(renderFinished);
}

void MonitorWidget::makeCmd()
{
	vkResetCommandBuffer(cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	tke::beginCommandBuffer(cmd);
	renderer->execute(cmd);
	vkCmdSetEvent(cmd, renderFinished, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	vkEndCommandBuffer(cmd);
}

void MonitorWidget::show()
{
	scene->update();

	if (tke::needRedraw)
	{
		makeCmd();
		tke::needRedraw = false;
	}

	ImGui::BeginDock("Monitor", &opened);
	ImGui::ImageButton(ImTextureID(image->index), ImVec2(tke::resCx, tke::resCy), ImVec2(0, 0), ImVec2(1, 1), 0);

	if (ImGui::IsItemHovered())
	{
		float distX = mainWindow->mouseX - mainWindow->mousePrevX;
		float distY = mainWindow->mouseY - mainWindow->mousePrevY;

		if (distX != 0 || distY != 0)
		{
			distX /= tke::resCx;
			distY /= tke::resCy;
			if (mainWindow->leftPressing)
			{
				if (GetAsyncKeyState(VK_MENU) & 0x8000)
				{
					scene->camera.rotateByCursor(distX, distY);
				}
			}
			else if (mainWindow->middlePressing)
			{
				if (GetAsyncKeyState(VK_MENU) & 0x8000)
					scene->camera.moveByCursor(distX, distY);
			}
			else if (mainWindow->rightPressing)
			{
				if (GetAsyncKeyState(VK_MENU) & 0x8000)
					scene->camera.scroll(distX);
			}
		}
	}
	ImGui::EndDock();

	if (!opened)
	{
		for (auto it = monitors.begin(); it != monitors.end(); it++)
		{
			if (*it == this)
			{
				monitors.erase(it);
				break;
			}
		}
		delete this;
	}
}

std::vector<MonitorWidget*> monitors;