#include "scene.h"
#include "sky.h"

namespace tke
{
	Sky::Sky(SkyType _type)
		:Node(NodeTypeSky), type(_type)
	{
	}

	SkyAtmosphereScattering::SkyAtmosphereScattering(Scene *_scene)
		:Sky(SkyType::atmosphere_scattering), scene(_scene)
	{
		sun_light = new Light(LightType::parallax);
		scene->addLight(sun_light);
	}

	SkyAtmosphereScattering::~SkyAtmosphereScattering()
	{
		scene->removeLight(sun_light);
	}

	SkyPanorama::SkyPanorama()
		:Sky(SkyType::panorama)
	{
	}
}
