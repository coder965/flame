#include "..\src\gui.h"

#include "select.h"
#include "editor.h"
#include "monitor.h"

MonitorWidget::MonitorWidget(tke::Scene *_scene)
	:scene(_scene)
{
	image = new tke::Image(tke::resCx, tke::resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	tke::addGuiImage(image);

	fb_scene = scene->createFramebuffer(image);
	fb_image = tke::getFramebuffer(image, tke::plainRenderPass_image8);
	VkImageView views[] = {
		image->getView(),
		tke::depthImage->getView()
	};
	fb_tool = tke::getFramebuffer(image->cx, image->cy, tke::plainRenderPass_depth_clear_image8, ARRAYSIZE(views), views);

	cb = new tke::CommandBuffer(tke::commandPool);
	cb_physx = new tke::CommandBuffer(tke::commandPool);
	cb_wireframe = new tke::CommandBuffer(tke::commandPool);
	ds_wireframe = new tke::DescriptorSet(tke::descriptorPool, tke::plainPipeline_3d_anim_wire->descriptorSetLayout);
	scene_renderFinished = tke::createEvent();
	physx_renderFinished = tke::createEvent();
	wireframe_renderFinished = tke::createEvent();
	renderFinished = tke::createEvent();

	transformerTool = new TransformerTool(fb_tool);
}

MonitorWidget::~MonitorWidget()
{
	tke::removeGuiImage(image);
	delete image;
	tke::releaseFramebuffer(fb_scene);
	tke::releaseFramebuffer(fb_image);
	tke::releaseFramebuffer(fb_tool);
	delete cb;
	delete cb_physx;
	delete cb_wireframe;
	delete ds_wireframe;
	delete transformerTool;
	tke::destroyEvent(scene_renderFinished);
	tke::destroyEvent(physx_renderFinished);
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
		data.modelview = currentScene->camera.getMatInv() * object->getMat();
		data.color = glm::vec4((i + 1) / 255.f, 0.f, 0.f, 0.f);
		cb->pushConstant(tke::StageType((int)tke::StageType::vert | (int)tke::StageType::frag), 0, sizeof(data), &data);
		cb->drawIndex(model->indices.size(), model->indiceBase, model->vertexBase);
	}
}

void MonitorWidget::show()
{
	ImGui::BeginDock("Monitor", &opened);
	if (ImGui::IsWindowFocused())
		lastWindowType = LastWindowTypeMonitor;

	if (ImGui::Button("save"))
		scene->save(scene->filename);

	ImGui::ImageButton(ImTextureID(image->index), ImVec2(tke::resCx, tke::resCy), ImVec2(0, 0), ImVec2(1, 1), 0);
	ImVec2 image_pos = ImGui::GetItemRectMin();
	if (ImGui::IsItemHovered())
	{
		if (mainWindow->mouseDispX != 0 || mainWindow->mouseDispY != 0)
		{
			if (tke::atlPressing())
			{
				auto distX = (float)mainWindow->mouseDispX / (float)tke::resCx;
				auto distY = (float)mainWindow->mouseDispY / (float)tke::resCy;
				if (mainWindow->leftPressing)
					scene->camera.rotateByCursor(distX, distY);
				else if (mainWindow->middlePressing)
					scene->camera.moveByCursor(distX, distY);
				else if (mainWindow->rightPressing)
					scene->camera.scroll(distX);
			}
			else
			{
				if (mainWindow->leftPressing)
					transformerTool->mouseMove(mainWindow->mouseDispX, mainWindow->mouseDispY);
			}
		}
		if (mainWindow->leftJustDown)
		{
			if (!tke::atlPressing())
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
	}

	{
		char *names[] = {
			ICON_FA_MOUSE_POINTER, "M", "R", "S"
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

	bool target = false;
	if (ImGui::Button("Target To Selected"))
		target = true;
	ImGui::SameLine();
	static bool follow = false;
	ImGui::Checkbox("Follow", &follow);
	if (target || follow)
	{
		if (selectedItem)
		{
			scene->camera.setTarget(selectedItem.toTransformer()->getCoord());
			scene->camera.lookAtTarget();
		}
	}

	static bool viewPhysx = false;
	if (ImGui::Checkbox("View Physx", &viewPhysx))
	{
		if (viewPhysx)
		{
			scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.f);
			////scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.f);
			scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.f);
			//scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_DYNAMIC, 1.f);
			//pxScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.f);
			////scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.f);
		}
		else
		{
			scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 0.f);
		}
	}

	ImGui::EndDock();

	scene->show(cb, fb_scene, scene_renderFinished);

	{ // view physx
		cb_physx->reset();
		cb_physx->begin();

		cb_physx->waitEvents(1, &scene_renderFinished);

		if (viewPhysx)
		{
			cb_physx->beginRenderPass(tke::plainRenderPass_image8, fb_image);

			auto &rb = scene->pxScene->getRenderBuffer();

			////for (int i = 0; i < rb.getNbPoints(); i++)
			////{
			////	p.color = _intToRGB(rb.getPoints()[i].color);
			////	p.coord = _pxVec3ToVec3(rb.getPoints()[i].pos);
			////	tke3_debugBuffer.points.push_back(p);
			////}

			auto lineCount = rb.getNbLines();
			if (lineCount > 0)
			{
				auto vertexSize = lineCount * 2 * sizeof(tke::LineVertex);
				if (!physxBuffer || physxBuffer->size < vertexSize)
				{
					if (physxBuffer) delete physxBuffer;
					physxBuffer = new tke::OnceVertexBuffer(vertexSize);
				}

				{
					auto map = physxBuffer->map(0, vertexSize);
					auto vtx_dst = (tke::LineVertex*)map;
					for (int i = 0; i < lineCount; i++)
					{
						auto &line = rb.getLines()[i];
						vtx_dst[0].position.x = line.pos0.x;
						vtx_dst[0].position.y = line.pos0.y;
						vtx_dst[0].position.z = line.pos0.z;
						vtx_dst[0].color.r = line.color0 % 256;
						vtx_dst[0].color.g = (line.color0 / 256) % 256;
						vtx_dst[0].color.b = (line.color0 / 65536) % 256;
						vtx_dst[1].position.x = line.pos1.x;
						vtx_dst[1].position.y = line.pos1.y;
						vtx_dst[1].position.z = line.pos1.z;
						vtx_dst[1].color.r = line.color1 % 256;
						vtx_dst[1].color.g = (line.color1 / 256) % 256;
						vtx_dst[1].color.b = (line.color1 / 65536) % 256;
						vtx_dst += 2;
					}
					physxBuffer->unmap();
				}
				cb_physx->bindVertexBuffer(physxBuffer);
				cb_physx->bindPipeline(tke::plainPipeline_3d_line);

				glm::mat4 mvp = tke::matPerspective * scene->camera.getMatInv();
				cb_physx->pushConstant(tke::StageType::vert, 0, sizeof(glm::mat4), &mvp);
				cb_physx->draw(lineCount * 2);
			}

			//glNamedBufferSubData(tke_dynamicVertexVBO, 0, sizeof(GLfloat) * 12 * lineCount, tke_dynamicVertexBuffer);
			//glBindVertexArray(tke3_dynamicVertexVAO);
			//glDrawArrays(GL_LINES, 0, lineCount * 2);

			//auto triangleCount = rb.getNbTriangles();
			//for (int i = 0; i < triangleCount; i++)
			//{
			//	auto &triangle = rb.getTriangles()[i];
			//	tke_dynamicVertexBuffer[i * 18 + 0] = triangle.pos0.x;
			//	tke_dynamicVertexBuffer[i * 18 + 1] = triangle.pos0.y;
			//	tke_dynamicVertexBuffer[i * 18 + 2] = triangle.pos0.z;
			//	tke_dynamicVertexBuffer[i * 18 + 3] = triangle.color0 % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 4] = (triangle.color0 / 256) % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 5] = (triangle.color0 / 65536) % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 6] = triangle.pos1.x;
			//	tke_dynamicVertexBuffer[i * 18 + 7] = triangle.pos1.y;
			//	tke_dynamicVertexBuffer[i * 18 + 8] = triangle.pos1.z;
			//	tke_dynamicVertexBuffer[i * 18 + 9] = triangle.color1 % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 10] = (triangle.color1 / 256) % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 11] = (triangle.color1 / 65536) % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 12] = triangle.pos2.x;
			//	tke_dynamicVertexBuffer[i * 18 + 13] = triangle.pos2.y;
			//	tke_dynamicVertexBuffer[i * 18 + 14] = triangle.pos2.z;
			//	tke_dynamicVertexBuffer[i * 18 + 15] = triangle.color2 % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 16] = (triangle.color2 / 256) % 256;
			//	tke_dynamicVertexBuffer[i * 18 + 17] = (triangle.color2 / 65536) % 256;
			//}
			//glNamedBufferSubData(tke_dynamicVertexVBO, 0, sizeof(GLfloat) * 18 * triangleCount, tke_dynamicVertexBuffer);
			//glBindVertexArray(tke3_dynamicVertexVAO);
			//glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);

			cb_physx->endRenderPass();
		}

		cb_physx->resetEvent(scene_renderFinished);
		cb_physx->setEvent(physx_renderFinished);

		cb_physx->end();
	}

	{ // draw wireframe
		static tke::Object *last_obj = nullptr;

		cb_wireframe->reset();
		cb_wireframe->begin();

		cb_wireframe->waitEvents(1, &physx_renderFinished);

		auto obj = selectedItem.toObject();
		if (obj)
		{
			auto model = obj->model;
			auto animated = model->animated;

			if (last_obj != obj && animated)
				ds_wireframe->setBuffer(tke::plain3d_bone_pos, 0, obj->animationComponent->boneMatrixBuffer);

			cb_wireframe->beginRenderPass(tke::plainRenderPass_image8, fb_image);
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
			data.modelview = scene->camera.getMatInv() * obj->getMat();
			data.color = glm::vec4(0.f, 1.f, 0.f, 1.f);
			cb_wireframe->pushConstant(tke::StageType((int)tke::StageType::vert | (int)tke::StageType::frag), 0, sizeof(data), &data);
			cb_wireframe->drawIndex(model->indices.size(), model->indiceBase, model->vertexBase);
			cb_wireframe->endRenderPass();
		}
		last_obj = obj;

		cb_wireframe->resetEvent(physx_renderFinished);
		cb_wireframe->setEvent(wireframe_renderFinished);

		cb_wireframe->end();
	}

	transformerTool->transformer = selectedItem.toTransformer();
	transformerTool->show(&scene->camera, wireframe_renderFinished, renderFinished);
}

MonitorWidget* monitorWidget = nullptr;