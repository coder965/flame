#pragma once

#include <memory>

#include "../object.h"

namespace tke
{
	struct Image;
	struct Framebuffer;

	struct DisplayLayer : Object
	{
		bool enable_depth;
		std::shared_ptr<Image> image;
		std::unique_ptr<Image> depth_image;
		std::shared_ptr<Framebuffer> framebuffer;

		virtual bool on_message(Object *sender, Message msg) override;

		DisplayLayer(bool _enable_depth = false);

	private:
		void create();
	};
}
