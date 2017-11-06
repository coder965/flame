#pragma once

#include "vulkan.h"

namespace tke
{
	struct Scene;

	struct Renderer
	{
		Scene *scene;
		VkEvent renderFinished;

		Renderer(Scene *_scene);
		virtual void render() = 0;

	};

	struct PlainRenderer : Renderer
	{
		PlainRenderer(Scene *_scene);
		virtual void render() override;
	};

	struct WireframeRenderer : Renderer
	{
		WireframeRenderer(Scene *_scene);
		virtual void render() override;
	};

	struct DeferredRenderer : Renderer
	{
		DeferredRenderer(Scene *_scene);
		virtual void render() override;
	};
}
