#pragma once

#include "node.h"

namespace tke
{
	enum class LightType
	{
		parallax = 1 << 0,
		point = 1 << 1,
		spot = 1 << 2
	};

	std::string getLightTypeName(LightType _type);

	struct Light : Node
	{
		LightType type;
		glm::vec3 color = glm::vec3(0.5f);
		float range = 0.5f;

		bool shadow;

		int sceneShadowIndex = -1;

		Light(LightType _type, bool _shadow = false);
		void setColor(const glm::vec3 &);
		virtual void on_message(_Object*, Message) override {}
	};
}
