#include "node.h"
#include "scene.h"
#include "light.h"
#include "sky.h"

namespace tke
{
	Sky::Sky(SkyType _type)
		:type(_type)
	{
	}

	SkyAtmosphereScattering::SkyAtmosphereScattering(Scene *_scene)
		:Sky(SkyType::atmosphere_scattering), scene(_scene)
	{
		node = new Node(NodeTypeNode);
		sun_light = new LightComponent(LightTypeParallax);
		node->add_component(sun_light);
		scene->add_child(node);
	}

	SkyAtmosphereScattering::~SkyAtmosphereScattering()
	{
		scene->remove_child(node);
	}

	SkyPanorama::SkyPanorama()
		:Sky(SkyType::panorama)
	{
	}
}
