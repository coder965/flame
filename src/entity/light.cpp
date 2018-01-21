#include "light.h"
#include "../file_utils.h"

namespace tke
{
	std::string get_light_type_name(LightType type)
	{
		char *names[] = {
			"parallax",
			"point",
			"spot"
		};
		return names[type];
	}

	void LightComponent::serialize(XMLNode *dst)
	{
		dst->new_attribute("type", get_light_type_name(type));
		dst->new_attribute("color", &color);
		dst->new_attribute("range", &range);
		dst->new_attribute("enable_shadow", &enable_shadow);
	}

	bool LightComponent::unserialize(XMLNode *src)
	{

	}

	LightComponent::LightComponent(LightType _type) :
		Component(ComponentTypeLight), 
		type(_type),
		color(0.5f),
		range(0.5f),
		enable_shadow(false),
		light_index(-1),
		shadow_index(-1)
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

	bool LightComponent::is_enable_shadow() const
	{
		return enable_shadow;
	}

	int LightComponent::get_light_index() const
	{
		return light_index;
	}

	int LightComponent::get_shadow_index() const
	{
		return shadow_index;
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

	void LightComponent::set_enable_shadow(bool v)
	{
		if (enable_shadow == v)
			return;

		enable_shadow = v;
		attribute_dirty = true;
		broadcast(this, MessageToggleShaodw);
	}

	void LightComponent::set_light_index(int v)
	{
		light_index = v;
	}

	void LightComponent::set_shadow_index(int v)
	{
		shadow_index = v;
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
