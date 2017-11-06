#include "renderer.h"

namespace tke
{
	Renderer::Renderer(Scene *_scene)
		:scene(_scene)
	{
	}

	WireframeRenderer::WireframeRenderer(Scene *_scene)
		: Renderer(_scene)
	{

	}

	void WireframeRenderer::render()
	{

	}

	DeferredRenderer::DeferredRenderer(Scene *_scene)
		: Renderer(_scene)
	{

	}

	void DeferredRenderer::render()
	{

	}
}
