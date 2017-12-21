#pragma once

#include "../utils.h"
#include "../math/math.h"
#include "camera.h"
#include "sky.h"
#include "light.h"
#include "object.h"
#include "terrain.h"
#include "water.h"
#include "../physics/physics.h"
#include "../render/buffer.h"
#include "../render/image.h"
#include "../render/framebuffer.h"
#include "../render/pipeline.h"
#include "../render/command_buffer.h"
#include "../resource/resource.h"

namespace tke
{
	REFLECTABLE struct Scene
	{
		REFL_BANK;

		std::mutex mtx;

		std::string filename;
		std::string filepath;

		SkyType skyType = SkyType::null;
		float atmosphereSunE = 20.f;
		float atmosphereInnerRadius = 10.f; // The inner (planetary) radius
		float atmosphereOuterRadius = 10.25f; // The outer (atmosphere) radius
		float atmosphereCameraHeight = 10.0002f;
		float atmosphereKm = 0.0025f;
		float atmosphereKr = 0.001f;
		bool enable_sun_light = true;
		glm::vec2 sun_light_dir = glm::vec2(0.f);
		glm::mat3 sun_light_axis = glm::mat3(1.f);
		float sun_light_power = 1.f;
		bool enable_sun_light_shadow = true;

		float hdrExposure = 0.01f;
		float hdrWhite = 1.f;

		glm::vec3 ambientColor = glm::vec3(0.5f);
		glm::vec3 fogColor = glm::vec3(0.5f);

		float ssaoRadius = 10.f;
		float ssaoBias = 0.01f;
		float ssaoIntensity = 100000.f;

		float fogThickness = 0.01f;

		Camera camera;

		std::vector<std::unique_ptr<Light>> lights;

		std::vector<std::unique_ptr<Object>> objects;

		std::unique_ptr<Terrain> terrain;

		std::vector<std::unique_ptr<Water>> waters;

		bool needUpdateSky = true;
		bool needUpdateSunLight = true;
		bool needUpdateAmbientBuffer = true;
		bool needUpdateIndirectBuffer = true;
		bool needUpdateLightCount = true;

		std::vector<CollisionGroup*> pCollisionGroups;

		physx::PxScene *pxScene = nullptr;
		physx::PxControllerManager *pxControllerManager = nullptr;

		Scene();
		~Scene();
		void setSkyType(SkyType _skyType);
		void addLight(Light *l);
		Light *removeLight(Light *l);
		void addObject(Object *o);
		Object *removeObject(Object *o);
		int getCollisionGroupID(int ID, unsigned int mask);
		void addTerrain(Terrain *t);
		void removeTerrain();
		void addWater(Water *w);
		Water *removeWater(Water *w);
		void reset();
		void clear();
		void setSunDir(const glm::vec2 &);
		void setAmbientColor(const glm::vec3 &);
		void setFogColor(const glm::vec3 &);
		void update();
		void loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename);
		void save(const std::string &filename);
	};

	std::shared_ptr<Scene> getScene(const std::string &filename);
}
