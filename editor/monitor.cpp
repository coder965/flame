#include "editor.h"
#include "monitor.h"

MonitorWidget::MonitorWidget()
{
	renderer = new tke::Renderer;
	renderer->loadXML(renderer_filename);
	scene = new tke::Scene;

	image = new tke::Image(800, 600, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
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
	tke::beginCommandBuffer(cmd);
	renderer->execute(cmd);
	vkCmdSetEvent(cmd, renderFinished, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	vkEndCommandBuffer(cmd);
}

MonitorWidget::~MonitorWidget()
{
	mainWindow->removeEvent(renderFinished);
	tke::destroyEvent(renderFinished);
}

void MonitorWidget::show()
{
	ImGui::Begin("Monitor", &opened);

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