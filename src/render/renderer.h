#pragma once

#include <memory>

#include "vulkan.h"
#include "command_buffer.h"

namespace tke
{
	struct Renderer
	{
		VkEvent renderFinished;

		Renderer();
		virtual void render() = 0;

	};

	struct PlainRenderer : Renderer
	{
		PlainRenderer();
		virtual void render() override;
	};

	struct WireframeRenderer : Renderer
	{
		std::unique_ptr<CommandBuffer> cb;

		WireframeRenderer();
		virtual void render() override;
	};

	struct DeferredRenderer : Renderer
	{
		DeferredRenderer();
		virtual void render() override;
	};
}
