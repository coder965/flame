#include "scene_private.h"
#include "device_private.h"
#include "rigid_private.h"

namespace flame
{
	namespace physics
	{
		void Scene::add_rigid(Rigid *r)
		{
			_priv->v->addActor(*r->_priv->v);
		}

		void Scene::remove_rigid(Rigid *r)
		{
			_priv->v->removeActor(*r->_priv->v);
		}
		
		void Scene::update(float disp)
		{
			_priv->v->simulate(disp);
			_priv->v->fetchResults(true);
		}

		Scene *create_scene(Device *d, float gravity, int thread_count)
		{
			auto s = new Scene;
			
			s->_priv = new ScenePrivate;
			physx::PxSceneDesc desc(d->_priv->inst->getTolerancesScale());
			desc.gravity = physx::PxVec3(0.0f, gravity, 0.0f);
			desc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(thread_count);
			desc.filterShader = physx::PxDefaultSimulationFilterShader;
			s->_priv->v = d->_priv->inst->createScene(desc);

			return s;
		}

		void destroy_scene(Scene *s)
		{
			s->_priv->v->release();

			delete s->_priv;
			delete s;
		}
	}
}

