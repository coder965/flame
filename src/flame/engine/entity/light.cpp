#include <flame/global.h>
#include <flame/serialize_math.h>
#include <flame/filesystem.h>
#include <flame/engine/core/core.h>
#include <flame/engine/entity/light.h>

namespace flame
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
		dst->attributes.emplace_back(new XMLAttribute("type", get_light_type_name(type)));
		dst->attributes.emplace_back(new XMLAttribute("color", to_str(color)));
		dst->attributes.emplace_back(new XMLAttribute("range", to_str(range)));
		dst->attributes.emplace_back(new XMLAttribute("enable_shadow", to_str(enable_shadow)));
	}

	void LightComponent::unserialize(XMLNode *src)
	{
		for (auto &a : src->attributes)
		{
			if (a->name == "type")
			{
				if (a->value == "parallax")
					type = LightTypeParallax;
				else if (a->value == "point")
					type = LightTypePoint;
				else if (a->value == "spot")
					type = LightTypeSpot;
				else
					assert(0); // require a vaild type name
			}
			else if (a->name == "color")
				color = to_float3(a->value);
			else if (a->name == "range")
				range = to_float(a->value);
			else if (a->name == "enable_shadow")
				enable_shadow = to_bool(a->value);
		}
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
