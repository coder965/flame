#include "renderer.h"

namespace tke
{
	Renderer::Renderer()
	{
	}

	WireframeRenderer::WireframeRenderer()
	{
		cb = std::make_unique<CommandBuffer>();
	}

	void WireframeRenderer::render()
	{

	}

	DeferredRenderer::DeferredRenderer()
	{

	}

	void DeferredRenderer::render()
	{

	}
}
