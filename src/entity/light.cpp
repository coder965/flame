#include "../global.h"
#include "../file_utils.h"
#include "light.h"

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
		dst->add_attribute(new XMLAttribute("type", get_light_type_name(type)));
		dst->add_attribute(new XMLAttribute("color", color));
		dst->add_attribute(new XMLAttribute("range", range));
		dst->add_attribute(new XMLAttribute("enable_shadow", enable_shadow));
	}

	void LightComponent::unserialize(XMLNode *src)
	{
		auto type_name = src->first_attribute("type")->get_string(); // required
		if (type_name == "parallax")
			type = LightTypeParallax;
		else if (type_name == "point")
			type = LightTypePoint;
		else if (type_name == "spot")
			type = LightTypeSpot;
		else
			assert(0); // require a vaild type name
		src->get_attribute_float3("color", color);
		src->get_attribute_float("range", range);
		src->get_attribute_bool("enable_shadow", enable_shadow);
	}

	LightComponent::LightComponent() :
		Component(ComponentTypeLight), 
		type(LightTypePoint),
		color(0.5f),
		range(0.5f),
		enable_shadow(false),
		light_index(-1),
		shadow_index(-1),
		attribute_dirty_frame(0)
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
		attribute_dirty_frame = total_frame_count;
	}

	void LightComponent::set_color(const glm::vec3 &v)
	{
		color = v;
		attribute_dirty_frame = total_frame_count;
	}

	void LightComponent::set_ranget(float v)
	{
		range = v;
		attribute_dirty_frame = total_frame_count;
	}

	void LightComponent::set_enable_shadow(bool v)
	{
		if (enable_shadow == v)
			return;

		enable_shadow = v;
		attribute_dirty_frame = total_frame_count;
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

	long long LightComponent::get_attribute_dirty_frame() const
	{
		return attribute_dirty_frame;
	}
}
