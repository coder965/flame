#include "light.h"

namespace tke
{
	std::string get_light_type_name(LightType type)
	{
		char *names[] = {
			"parallax light",
			"point light",
			"spot light"
		};
		return names[type];
	}

	LightComponent::LightComponent(LightType _type) :
		Component(ComponentTypeLight), 
		type(_type),
		color(0.5f),
		range(0.5f),
		shadow(false)
	{
	}

	LightType LightComponent::get_type() const
	{
		return type;
	}

	glm::vec3 LightComponent::get_color() const
	{
		return color;
	}

	float LightComponent::get_range() const
	{
		return range;
	}

	void LightComponent::set_type(LightType v)
	{
		type = v;
		attribute_dirty = true;
	}

	void LightComponent::set_color(const glm::vec3 &v)
	{
		color = v;
		attribute_dirty = true;
	}

	void LightComponent::set_ranget(float v)
	{
		range = v;
		attribute_dirty = true;
	}

	bool LightComponent::is_attribute_dirty()
	{
		return attribute_dirty;
	}

	void LightComponent::clear_attribute_dirty()
	{
		attribute_dirty = false;
	}
}
