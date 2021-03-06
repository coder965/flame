#pragma once

#include <flame/math.h>
#include <flame/engine/entity/component.h>

namespace flame
{
	enum LightType
	{
		LightTypeParallax,
		LightTypePoint,
		LightTypeSpot
	};

	std::string get_light_type_name(LightType type);

	class LightComponent : public Component
	{
	private:
		LightType type;
		glm::vec3 color;
		float range;
		bool enable_shadow;
		int light_index;
		int shadow_index;

		long long attribute_dirty_frame;
	public:
		virtual void serialize(XMLNode *dst) override;
		virtual void unserialize(XMLNode *src) override;

		LightComponent();

		LightType get_type() const;
		glm::vec3 get_color() const;
		float get_range() const;
		bool is_enable_shadow() const;
		int get_light_index() const;
		int get_shadow_index() const;

		void set_type(LightType v);
		void set_color(const glm::vec3 &v);
		void set_ranget(float v);
		void set_enable_shadow(bool v);
		void set_light_index(int v);
		void set_shadow_index(int v);

		long long get_attribute_dirty_frame() const;
	};
}
