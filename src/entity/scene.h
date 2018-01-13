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
	struct CollisionGroup;

	struct Scene : Node
	{
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

		std::vector<CollisionGroup*> pCollisionGroups;

		physx::PxScene *pxScene = nullptr;
		physx::PxControllerManager *pxControllerManager = nullptr;

		virtual void on_update() override;

		Scene();
		~Scene();
		void setSkyType(SkyType skyType);
		void set_pano_sky_image(std::shared_ptr<Image> i);
		int getCollisionGroupID(int ID, unsigned int mask);
		void setSunDir(const glm::vec2 &);
		void setAmbientColor(const glm::vec3 &);
		void setFogColor(const glm::vec3 &);
		void loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename);
		void save(const std::string &filename);
	};

	std::shared_ptr<Scene> getScene(const std::string &filename);
}
