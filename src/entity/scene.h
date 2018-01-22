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

	class Scene : public Node
	{
	private:
		std::string filename;

		std::unique_ptr<Sky> sky;
		bool enable_sun_light;

		float hdr_exposure;
		float hdr_white;

		glm::vec3 ambient_color ;
		glm::vec3 fog_color;

		float ssao_radius;
		float ssao_bias;
		float ssao_intensity;

		float fog_thickness;

		//std::vector<CollisionGroup*> pCollisionGroups;

		//physx::PxScene *pxScene = nullptr;
		//physx::PxControllerManager *pxControllerManager = nullptr;
	protected:
		virtual void on_update() override;
	public:
		Scene();
		~Scene();

		std::string get_filename() const;
		SkyType get_sky_type() const;
		Sky *get_sky() const;
		glm::vec3 get_ambient_color() const;
		glm::vec3 get_fog_color() const;

		void set_filename(const std::string &_filename);
		void set_sky_type(SkyType skyType);
		void set_pano_sky_image(std::shared_ptr<Image> i);
		//int getCollisionGroupID(int ID, unsigned int mask);
		void setSunDir(const glm::vec2 &);
		void set_ambient_color(const glm::vec3 &);
		void set_fog_color(const glm::vec3 &);
		void loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename);
	};

	Scene *create_scene(const std::string &filename);
	void save_scene(Scene *src);
}
