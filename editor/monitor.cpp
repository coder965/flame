#include "select.h"
#include "editor.h"
#include "monitor.h"

MonitorWidget::MonitorWidget(tke::Model *_model)
	:model(_model)
{
	scene = new tke::Scene;

	image = new tke::Image(tke::resCx, tke::resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	tke::addGuiImage(image);

	fb_scene = scene->createFramebuffer(image);
	fb_wireframe = tke::getFramebuffer(image, tke::plainRenderPass_image8);
	VkImageView views[] = {
		image->getView(),
		tke::depthImage->getView()
	};
	fb_tool = tke::getFramebuffer(image->width, image->height, tke::plainRenderPass_depth_clear_image8, ARRAYSIZE(views), views);

	tke::ShaderMacro macro;
	macro.pipeline_name = "Deferred.Pipeline";
	macro.stage = tke::StageType::frag;
	macro.value = "USE_PBR";
	tke::globalResource.shaderMacros.push_back(macro);

	cb = new tke::CommandBuffer(tke::commandPool);
	cb_wireframe = new tke::CommandBuffer(tke::commandPool);
	ds_wireframe = new tke::DescriptorSet(tke::descriptorPool, tke::plainPipeline_3d_anim_wire->descriptorSetLayout);
	scene_renderFinished = tke::createEvent();
	wireframe_renderFinished = tke::createEvent();
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
	delete scene;
	tke::removeGuiImage(image);
	delete image;
	tke::releaseFramebuffer(fb_tool);
	tke::releaseFramebuffer(fb_scene);
	delete cb;
	delete cb_wireframe;
	delete ds_wireframe;
	tke::destroyEvent(scene_renderFinished);
	tke::destroyEvent(wireframe_renderFinished);
	tke::destroyEvent(renderFinished);
}

static tke::Scene *currentScene = nullptr;
void draw_wireframe(tke::CommandBuffer *cb) 
{
	for (int i = 0; i < currentScene->objects.size(); i++)
	{
		auto object = currentScene->objects[i];
		auto model = object->model;
		auto animated = model->animated;
		cb->bindVertexBuffer(animated ? tke::animatedVertexBuffer : tke::staticVertexBuffer);
		cb->bindIndexBuffer(animated ? tke::animatedIndexBuffer : tke::staticIndexBuffer);
		cb->bindPipeline(animated ? tke::plainPipeline_3d_anim_depth : tke::plainPipeline_3d_depth);
		if (animated)
			tke::plainPipeline_3d_anim_depth->descriptorSet->setBuffer(tke::plain3d_bone_pos, 0, object->animationComponent->boneMatrixBuffer);
		cb->bindDescriptorSet();
		struct
		{
			glm::mat4 modelview;
			glm::mat4 proj;
			glm::vec4 color;
		}data;
		data.proj = tke::matPerspective;
		data.modelview = currentScene->camera.getMatInv();
		data.color = glm::vec4(i / 255.f, 0.f, 0.f, 0.f);
		cb->pushConstant(tke::StageType((int)tke::StageType::vert | (int)tke::StageType::frag), 0, sizeof(data), &data);
		cb->drawIndex(model->indices.size(), model->indiceBase, model->vertexBase);
	}
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
				currentScene = scene;
				auto index = tke::pickUp(x, y, draw_wireframe);
				if (index == 0)
					selectedItem.reset();
				else
					selectedItem.select(scene->objects[index - 1]);
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

	scene->show(cb, fb_scene, scene_renderFinished);

	{ // draw wireframe
		static tke::Object *last_obj = nullptr;

		cb_wireframe->reset();
		cb_wireframe->begin();

		cb_wireframe->waitEvents(1, &scene_renderFinished);

		auto obj = selectedItem.toObject();
		if (obj)
		{
			auto model = obj->model;
			auto animated = model->animated;

			if (last_obj != obj && animated)
				ds_wireframe->setBuffer(tke::plain3d_bone_pos, 0, obj->animationComponent->boneMatrixBuffer);

			cb_wireframe->beginRenderPass(tke::plainRenderPass_image8, fb_wireframe);
			cb_wireframe->bindVertexBuffer(animated ? tke::animatedVertexBuffer : tke::staticVertexBuffer);
			cb_wireframe->bindIndexBuffer(animated ? tke::animatedIndexBuffer : tke::staticIndexBuffer);
			cb_wireframe->bindPipeline(animated ? tke::plainPipeline_3d_anim_wire : tke::plainPipeline_3d_wire);
			if (animated)
				cb_wireframe->bindDescriptorSet(ds_wireframe->v);
			else
				cb_wireframe->bindDescriptorSet();
			struct
			{
				glm::mat4 modelview;
				glm::mat4 proj;
				glm::vec4 color;
			}data;
			data.proj = tke::matPerspective;
			data.modelview = scene->camera.getMatInv();
			data.color = glm::vec4(0.f, 1.f, 0.f, 1.f);
			cb_wireframe->pushConstant(tke::StageType((int)tke::StageType::vert | (int)tke::StageType::frag), 0, sizeof(data), &data);
			cb_wireframe->drawIndex(model->indices.size(), model->indiceBase, model->vertexBase);
			cb_wireframe->endRenderPass();
		}
		last_obj = obj;

		cb_wireframe->resetEvent(scene_renderFinished);
		cb_wireframe->setEvent(wireframe_renderFinished);

		cb_wireframe->end();
	}

	if (selectedItem)
	{
		switch (selectedItem.type)
		{
		case ItemTypeObject:
			transformerTool->transformer = selectedItem.toObject();
			break;
		}
		transformerTool->show(&scene->camera, wireframe_renderFinished, renderFinished);
	}
}

MonitorWidget* monitorWidget = nullptr;