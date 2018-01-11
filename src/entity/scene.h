#pragma once

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

	struct Scene : Node
	{
		std::string filename;
		std::string filepath;

		std::unique_ptr<Sky> sky;
		bool enable_sun_light = true;

		bool needUpdateSky = true;

		float hdrExposure = 0.01f;
		float hdrWhite = 1.f;

		bool needUpdateAmbientBuffer = true;

		glm::vec3 ambientColor = glm::vec3(0.5f);
		glm::vec3 fogColor = glm::vec3(0.5f);

		float ssaoRadius = 10.f;
		float ssaoBias = 0.01f;
		float ssaoIntensity = 100000.f;

		float fogThickness = 0.01f;

		std::vector<std::unique_ptr<Light>> lights;
		std::vector<std::unique_ptr<Object>> objects;
		std::vector<std::unique_ptr<Terrain>> terrains;

		bool light_count_dirty = true;
		bool object_count_dirty = true;
		bool terrain_count_dirty = true;
		bool water_count_dirty = true;

		std::vector<CollisionGroup*> pCollisionGroups;

		physx::PxScene *pxScene = nullptr;
		physx::PxControllerManager *pxControllerManager = nullptr;

		virtual void on_clear() override;
		virtual void on_update() override;

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
		void setSunDir(const glm::vec2 &);
		void setAmbientColor(const glm::vec3 &);
		void setFogColor(const glm::vec3 &);
		void loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename);
		void save(const std::string &filename);
	};

	std::shared_ptr<Scene> getScene(const std::string &filename);
}
