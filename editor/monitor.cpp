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
	cb_wireframe = new tke::CommandBuffer(tke::commandPool);
	renderFinished = tke::createEvent();
	wireframe_renderFinished = tke::createEvent();

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
	tke::destroyEvent(wireframe_renderFinished);
	delete scene;
	tke::removeGuiImage(image);
	delete image;
	tke::releaseFramebuffer(fb_tool);
	tke::releaseFramebuffer(fb_scene);
	delete cb;
	delete cb_wireframe;
}

void MonitorWidget::show()
{
	ImGui::BeginDock("Monitor", &opened);
	if (ImGui::IsWindowFocused())
		lastWindowType = LastWindowTypeMonitor;

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

	{
		char *names[] = {
			"S", "M", "R", "C"
		};
		for (int i = 0; i < 4; i++)
		{
			auto needPopup = false;
			if (transformerTool->mode == TransformerTool::Mode(i))
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.f, 0.6f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.f, 0.8f, 0.8f));
				needPopup = true;
			}
			if (ImGui::Button(names[i])) transformerTool->mode = TransformerTool::Mode(i);
			if (needPopup) ImGui::PopStyleColor(3);
			if (i < 3) ImGui::SameLine();
		}
	}

	ImGui::EndDock();

	scene->show(cb, fb_scene, renderFinished);

	{ // draw wireframe
		//cb->reset();
		//cb->begin();

		//cb->waitEvents(1, &renderFinished);

		//cb->resetEvent(renderFinished);
		//cb->setEvent(wireframe_renderFinished);

		//cb->end();
	}

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

MonitorWidget* monitorWidget = nullptr;