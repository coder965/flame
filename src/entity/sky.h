#pragma once

namespace tke
{
	struct Scene;
	struct Node;
	struct LightComponent;
	struct Image;

	enum class SkyType
	{
		null,
		atmosphere_scattering = 1 << 0,
		panorama = 1 << 1
	};

	struct Sky
	{
		SkyType type;

		Sky(SkyType _type);
		virtual ~Sky() {}
	};

	struct SkyAtmosphereScattering : Sky
	{
		float atmosphereSunE = 20.f;
		float atmosphereInnerRadius = 10.f; // The inner (planetary) radius
		float atmosphereOuterRadius = 10.25f; // The outer (atmosphere) radius
		float atmosphereCameraHeight = 10.0002f;
		float atmosphereKm = 0.0025f;
		float atmosphereKr = 0.001f;
		glm::vec3 sun_color = glm::vec3(1.f);
		Scene *scene;
		Node *node;
		LightComponent *sun_light;
		float sun_power = 1.f;

		SkyAtmosphereScattering(Scene *_scene);
		virtual ~SkyAtmosphereScattering();
	};

	struct SkyPanorama : Sky
	{
		std::shared_ptr<Image> panoImage;

		SkyPanorama();
	};
}
