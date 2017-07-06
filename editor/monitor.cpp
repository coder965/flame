#include "select.h"
#include "editor.h"
#include "monitor.h"

MonitorWidget::MonitorWidget(tke::Model *_model)
	:model(_model)
{
	scene = new tke::Scene;

	image = new tke::Image(tke::resCx, tke::resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	tke::addGuiImage(image);

	VkImageView views[] = {
		image->getView(),
		tke::depthImage->getView()
	};
	fb_tool = tke::getFramebuffer(image->width, image->height, tke::plainRenderPass_depth_clear_image8, ARRAYSIZE(views), views);
	fb_scene = scene->createFramebuffer(image);

	tke::ShaderMacro macro;
	macro.pipeline_name = "Deferred.Pipeline";
	macro.stage = tke::StageType::frag;
	macro.value = "USE_PBR";
	tke::globalResource.shaderMacros.push_back(macro);

	cb = new tke::CommandBuffer(tke::commandPool);
	renderFinished = tke::createEvent();

	transformerTool = new TransformerTool(fb_tool);

	auto obj = new tke::Object(model);
	scene->addObject(obj);

	selectedItem.select(obj);

	scene->camera.setMode(tke::CameraModeTargeting);
	scene->camera.setCoord(0.f, 5.f, 0.f);
}

MonitorWidget::~MonitorWidget()
{
	tke::destroyEvent(renderFinished);

	delete scene;
	tke::removeGuiImage(image);
	delete image;
	tke::releaseFramebuffer(fb_tool);
	tke::releaseFramebuffer(fb_scene);
	delete cb;
}

void MonitorWidget::show()
{
	ImGui::BeginDock("Monitor", &opened);
	if (ImGui::IsWindowFocused())
	{
		lastWindowType = LastWindowTypeMonitor;
		lastMonitor = this;
	}

	ImGui::ImageButton(ImTextureID(image->index), ImVec2(tke::resCx, tke::resCy), ImVec2(0, 0), ImVec2(1, 1), 0);
	ImVec2 image_pos = ImGui::GetItemRectMin();
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
		if (mainWindow->leftJustDown && !(GetAsyncKeyState(VK_MENU) & 0x8000))
		{
			auto x = mainWindow->mouseX - image_pos.x;
			auto y = mainWindow->mouseY - image_pos.y;
			if (!transformerTool->leftDown(x, y))
			{

			}
		}
	}

	if (ImGui::Button("S"))
		transformerTool->mode = TransformerTool::ModeNull;
	ImGui::SameLine();
	if (ImGui::Button("M"))
		transformerTool->mode = TransformerTool::ModeMove;
	ImGui::SameLine();
	if (ImGui::Button("R"))
		transformerTool->mode = TransformerTool::ModeRotate;
	ImGui::SameLine();
	if (ImGui::Button("C"))
		transformerTool->mode = TransformerTool::ModeScale;

	ImGui::EndDock();

	scene->show(cb, fb_scene, renderFinished);

	if (selectedItem)
	{
		switch (selectedItem.type)
		{
		case ItemTypeObject:
			transformerTool->transformer = selectedItem.toObject();
			break;
		}
		transformerTool->show(&scene->camera, renderFinished);
	}
}

std::vector<MonitorWidget*> monitors;