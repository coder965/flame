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

	fb_one = tke::getFramebuffer(image, tke::plainRenderPass_image8);
	fb_scene = scene->createFramebuffer(image);

	tke::ShaderMacro macro;
	macro.pipeline_name = "Deferred.Pipeline";
	macro.stage = tke::StageType::frag;
	macro.value = "USE_PBR";
	renderer->resource.shaderMacros.push_back(macro);

	renderer->setup();

	tke::setMasterRenderer(renderer);

	cb = new tke::CommandBuffer(tke::commandPool);
	renderFinished = tke::createEvent();

	transformerTool = new TransformerTool(fb_one);

	auto obj = new tke::Object(model);
	scene->addObject(obj);

	selectedItem.select(obj);

	scene->camera.setMode(tke::CameraModeTargeting);
	scene->camera.setCoord(0.f, 5.f, 0.f);
}

MonitorWidget::~MonitorWidget()
{
	tke::destroyEvent(renderFinished);

	delete renderer;
	delete scene;
	tke::removeGuiImage(image);
	delete image;
	tke::releaseFramebuffer(fb_one);
	tke::releaseFramebuffer(fb_scene);
	delete cb;
}

void MonitorWidget::show()
{
	scene->show(cb, fb_scene, renderFinished);

	ImGui::BeginDock("Monitor", &opened);
	if (ImGui::IsWindowFocused())
	{
		lastWindowType = LastWindowTypeMonitor;
		lastMonitor = this;
	}

	ImGui::ImageButton(ImTextureID(image->index), ImVec2(tke::resCx, tke::resCy), ImVec2(0, 0), ImVec2(1, 1), 0);
	if (ImGui::IsItemHovered())
	{
		if (mainWindow->mouseDispX != 0 || mainWindow->mouseDispY != 0)
		{
			auto distX = (float)mainWindow->mouseDispX / (float)tke::resCx;
			auto distY = (float)mainWindow->mouseDispY / (float)tke::resCy;
			if (mainWindow->leftPressing)
			{
				if (GetAsyncKeyState(VK_MENU) & 0x8000)
					scene->camera.rotateByCursor(distX, distY);
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

	transformerTool->show(tke::matPerspective, renderFinished);
}

std::vector<MonitorWidget*> monitors;