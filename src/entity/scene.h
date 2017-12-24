#pragma once

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

		std::unique_ptr<Sky> sky;
		bool enable_sun_light = true;

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
		bool needUpdateAmbientBuffer = true;
		bool needUpdateIndirectBuffer = true;
		bool needUpdateLightCount = true;

		std::vector<CollisionGroup*> pCollisionGroups;

		physx::PxScene *pxScene = nullptr;
		physx::PxControllerManager *pxControllerManager = nullptr;

		Scene();
		~Scene();
		void setSkyType(SkyType skyType);
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
