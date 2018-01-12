#pragma once

#include "../math/math.h"
#include "component.h"

namespace tke
{
	enum LightType
	{
		LightTypeParallax,
		LightTypePoint,
		LightTypeSpot
	};

	std::string get_light_type_name(LightType type);

	class LightComponent : Component
	{
	private:
		LightType type;
		glm::vec3 color;
		float range;

		bool shadow;

		bool attribute_dirty;
	public:
		LightComponent(LightType _type);

		LightType get_type() const;
		glm::vec3 get_color() const;
		float get_range() const;

		void set_type(LightType v);
		void set_color(const glm::vec3 &v);
		void set_ranget(float v);

		bool is_attribute_dirty();
		void clear_attribute_dirty();
	};
}
