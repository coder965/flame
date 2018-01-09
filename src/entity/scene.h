#pragma once

#include <mutex>

#include "../math/math.h"
#include "node.h"
#include "camera.h"
#include "light.h"
#include "sky.h"

namespace physx
{
	struct PxScene;
	struct PxControllerManager;
}

namespace tke
{
	struct Light;
	struct Object;
	struct Terrain;
	struct Water;
	struct CollisionGroup;

	REFLECTABLE struct Scene : Node
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

		std::vector<std::shared_ptr<Light>> lights;
		std::vector<std::shared_ptr<Object>> objects;
		std::vector<std::shared_ptr<Terrain>> terrains;
		std::vector<std::shared_ptr<Water>> waters;

		bool needUpdateSky = true;
		bool needUpdateAmbientBuffer = true;
		bool light_count_dirty = true;
		bool object_count_dirty = true;
		bool terrain_count_dirty = true;
		bool water_count_dirty = true;

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
		Terrain *removeTerrain(Terrain *t);
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
