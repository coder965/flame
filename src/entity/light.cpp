#include "light.h"

namespace tke
{
	Light::Light(LightType _type, bool _shadow)
		:Node(NodeTypeLight), type(_type), shadow(_shadow)
	{
	}

	void Light::setColor(const glm::vec3 &v)
	{
		color = v;
		transform_dirty = true;
	}

	std::string getLightTypeName(LightType _type)
	{
		char *typeNames[] = {
			"parallax light",
			"point light",
			"spot light"
		};
		return typeNames[(int)_type];
	}

}
