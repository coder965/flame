#include "entity.h"
#include "core.h"

namespace tke
{
	Transformer::Transformer() {}

	Transformer::Transformer(glm::mat3 &rotation, glm::vec3 coord)
	{
		axis = rotation;
		coord = coord;

		needUpdateQuat = true;
		needUpdateEuler = true;
		needUpdateMat = true;
	}

	void Transformer::updateAxis()
	{
		if (!needUpdateQuat)
			quaternionToMatrix(quat, axis);// update by quat
		else
			eulerYzxToMatrix(euler, axis);// update by euler
		needUpdateAxis = false;
	}

	void Transformer::updateEuler()
	{
		if (needUpdateQuat) updateQuat();
		// updata by quat
		float heading, attitude, bank;

		auto sqw = quat.w * quat.w;
		auto sqx = quat.x * quat.x;
		auto sqy = quat.y * quat.y;
		auto sqz = quat.z * quat.z;

		auto unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
		auto test = quat.x * quat.y + quat.z * quat.w;
		if (test > 0.499f * unit)
		{ // singularity at north pole
			heading = 2.f * atan2(quat.x, quat.w);
			attitude = M_PI / 2.f;
			bank = 0;
			return;
		}
		if (test < -0.499f * unit)
		{ // singularity at south pole
			heading = -2.f * atan2(quat.x, quat.w);
			attitude = -M_PI / 2.f;
			bank = 0;
			return;
		}

		heading = atan2(2.f * quat.y * quat.w - 2.f * quat.x * quat.z, sqx - sqy - sqz + sqw);
		attitude = asin(2.f * test / unit);
		bank = atan2(2.f * quat.x * quat.w - 2.f * quat.y * quat.z, -sqx + sqy - sqz + sqw);

		euler.x = glm::degrees(heading);
		euler.y = glm::degrees(attitude);
		euler.z = glm::degrees(bank);
		needUpdateEuler = false;
	}

	void Transformer::updateQuat()
	{
		if (needUpdateAxis) updateAxis();
		// update by axis
		matrixToQuaternion(axis, quat);
		needUpdateQuat = false;
	}

	void Transformer::updateMat()
	{
		if (needUpdateAxis) updateAxis();
		mat = glm::translate(coord * worldScale) * glm::mat4(axis) * glm::scale(scale * worldScale);
		matInv = glm::inverse(mat);
		needUpdateMat = false;
	}

	glm::vec3 Transformer::getCoord() const
	{
		return coord;
	}

	glm::mat3 Transformer::getAxis()
	{
		if (needUpdateAxis) updateAxis();
		return axis;
	}

	glm::vec3 Transformer::getScale() const
	{
		return scale;
	}

	glm::vec3 Transformer::getWorldScale() const
	{
		return worldScale;
	}

	glm::vec3 Transformer::getEuler()
	{
		if (needUpdateEuler) updateEuler(); // Y -> Z -> X
		return euler;
	}

	glm::vec4 Transformer::getQuat()
	{
		if (needUpdateQuat)
			updateQuat();
		return quat;
	}

	glm::mat4 Transformer::getMat()
	{
		if (needUpdateMat) updateMat();
		return mat;
	}

	glm::mat4 Transformer::getMatInv()
	{
		if (needUpdateMat) updateMat();
		return matInv;
	}

	void Transformer::setCoord(const glm::vec3 &_coord)
	{
		coord = _coord;
		needUpdateMat = true;
		changed = true;
	}

	void Transformer::addCoord(const glm::vec3 &_coord)
	{
		setCoord(coord + _coord);
	}

	void Transformer::setEuler(const glm::vec3 &_euler)
	{
		euler = glm::mod(_euler, 360.f);

		needUpdateAxis = true;
		needUpdateEuler = false;
		needUpdateQuat = true;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::addEuler(const glm::vec3 &_euler)
	{
		setEuler(getEuler() + _euler);
	}

	void Transformer::setQuat(const glm::vec4 &_quat)
	{
		quat = _quat;
		needUpdateAxis = true;
		needUpdateEuler = true;
		needUpdateQuat = false;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::axisRotate(Axis which, float angle)
	{
		using namespace glm;
		switch (which)
		{
		case Axis::eX:
		{
			auto m = mat3(rotate(angle, axis[0]));
			axis[1] = normalize(m * axis[1]);
			axis[2] = normalize(m * axis[2]);
		}
		break;
		case Axis::eY:
		{
			auto m = mat3(rotate(angle, axis[1]));
			axis[0] = normalize(m * axis[0]);
			axis[2] = normalize(m * axis[2]);
		}
		break;
		case Axis::eZ:
		{
			auto m = mat3(rotate(angle, axis[2]));
			axis[1] = normalize(m * axis[1]);
			axis[0] = normalize(m * axis[0]);
		}
		break;
		}

		needUpdateAxis = false;
		needUpdateEuler = true;
		needUpdateQuat = true;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::leftRotate(const glm::mat3 &left)
	{
		axis = left * axis;

		needUpdateAxis = false;
		needUpdateEuler = true;
		needUpdateQuat = true;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::rightRotate(const glm::mat3 &right)
	{
		axis = axis * right;

		needUpdateAxis = false;
		needUpdateEuler = true;
		needUpdateQuat = true;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::setScale(const glm::vec3 &_scale)
	{
		scale = _scale;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::addScale(const glm::vec3 &_scale)
	{
		setScale(scale + _scale);
	}

	void Transformer::setWorldScale(const glm::vec3 &_scale)
	{
		worldScale = _scale;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::scaleRelate(Transformer *t)
	{
		coord *= t->scale;
		coord *= t->scale;
		axis = t->axis * axis;
		coord = t->axis * coord;
		coord += t->coord;
		needUpdateMat = true;

		changed = true;
	}

	void Transformer::relate(Transformer *t)
	{
		coord -= t->coord;
		axis *= glm::transpose(t->axis);
		needUpdateMat = true;

		changed = true;
	}

	void Controller::reset()
	{
		front = back = left = right = up = down = turnLeft = turnRight = false;
		lastTime = nowTime;
	}

	bool Controller::move(float inEulerX, glm::vec3 &outCoord, glm::vec3 &outEuler)
	{
		float dist = (nowTime - lastTime) / 1000.f;
		lastTime = nowTime;

		outCoord = glm::vec3();
		outEuler = glm::vec3();

		inEulerX = glm::radians(inEulerX + baseForwardAng);

		if (front && speed > 0.f)
		{
			outCoord.x -= sin(inEulerX) * speed * dist;
			outCoord.z -= cos(inEulerX) * speed * dist;
		}
		if (back && speed > 0.f)
		{
			outCoord.x += sin(inEulerX) * speed * dist;
			outCoord.z += cos(inEulerX) * speed * dist;
		}
		if (left && speed > 0.f)
		{
			outCoord.x -= cos(inEulerX) * speed * dist;
			outCoord.z += sin(inEulerX) * speed * dist;
		}
		if (right && speed > 0.f)
		{
			outCoord.x += cos(inEulerX) * speed * dist;
			outCoord.z -= sin(inEulerX) * speed * dist;
		}
		if (up)
			outCoord.y += speed * dist;
		if (down)
			outCoord.y -= speed * dist;

		if (turnLeft)
			outEuler.x = turnSpeed * dist;
		if (turnRight)
			outEuler.x = -turnSpeed * dist;
		if (turnUp)
			outEuler.z = turnSpeed * dist;
		if (turnDown)
			outEuler.z = -turnSpeed * dist;

		return (outCoord.x != 0.f || outCoord.y != 0.f || outCoord.z != 0.f) || (outEuler.x != 0.f) || (outEuler.y != 0.f) || (outEuler.z != 0.f);
	}

	bool Controller::keyDown(int key)
	{
		switch (key)
		{
		case 'W':
			front = true;
			return true;
		case 'S':
			back = true;
			return true;
		case 'A':
			left = true;
			return true;
		case 'D':
			right = true;
			return true;
		}
		return false;
	}

	bool Controller::keyUp(int key)
	{
		switch (key)
		{
		case 'W':
			front = false;
			return true;
		case 'S':
			back = false;
			return true;
		case 'A':
			left = false;
			return true;
		case 'D':
			right = false;
			return true;
		}
		return false;
	}

	Camera::Camera()
	{
		baseForwardAng = 90.f;
	}

	void Camera::setMode(CameraMode _mode)
	{
		mode = _mode;
		needUpdateMat = true;
		changed = true;
	}

	void Camera::setLength(float _length)
	{
		length = _length;
		needUpdateMat = true;
		changed = true;
	}

	void Camera::setTarget(const glm::vec3 &_target)
	{
		target = _target;
		needUpdateMat = true;
		changed = true;
	}

	void Camera::lookAtTarget()
	{
		if (mode == CameraModeTargeting)
		{
			if (needUpdateAxis) updateAxis();
			coord = target + axis[2] * length;
			needUpdateMat = true;
			changed = true;
		}
	}


	void Camera::updateFrustum()
	{
		auto tanHfFovy = glm::tan(glm::radians(TKE_FOVY * 0.5f));

		auto _y1 = TKE_NEAR * tanHfFovy;
		auto _z1 = _y1 * aspect;
		auto _y2 = TKE_FAR * tanHfFovy;
		auto _z2 = _y2 * aspect;
		frustumPoints[0] = -_z1 * axis[2] + _y1 * axis[1] + TKE_NEAR * axis[0] + coord;
		frustumPoints[1] = _z1 * axis[2] + _y1 * axis[1] + TKE_NEAR * axis[0] + coord;
		frustumPoints[2] = _z1 * axis[2] + -_y1 * axis[1] + TKE_NEAR * axis[0] + coord;
		frustumPoints[3] = -_z1 * axis[2] + -_y1 * axis[1] + TKE_NEAR * axis[0] + coord;
		frustumPoints[4] = -_z2 * axis[2] + _y2 * axis[1] + TKE_FAR * axis[0] + coord;
		frustumPoints[5] = _z2 * axis[2] + _y2 * axis[1] + TKE_FAR * axis[0] + coord;
		frustumPoints[6] = _z2 * axis[2] + -_y2 * axis[1] + TKE_FAR * axis[0] + coord;
		frustumPoints[7] = -_z2 * axis[2] + -_y2 * axis[1] + TKE_FAR * axis[0] + coord;
		for (int i = 0; i < 4; i++)
		{
			auto y = frustumPoints[i + 4].y;
			if (y < 0.f)
			{
				auto py = frustumPoints[i + 4].y - frustumPoints[i].y;
				if (py != 0.f)
				{
					frustumPoints[i + 4].x -= y * ((frustumPoints[i + 4].x - frustumPoints[i].x) / py);
					frustumPoints[i + 4].z -= y * ((frustumPoints[i + 4].z - frustumPoints[i].z) / py);
					frustumPoints[i + 4].y = 0.f;
				}
			}
		}

		auto matrix = matPerspective * mat;

		frustumPlanes[0].x = matrix[0].w + matrix[0].x;
		frustumPlanes[0].y = matrix[1].w + matrix[1].x;
		frustumPlanes[0].z = matrix[2].w + matrix[2].x;
		frustumPlanes[0].w = matrix[3].w + matrix[3].x;

		frustumPlanes[1].x = matrix[0].w - matrix[0].x;
		frustumPlanes[1].y = matrix[1].w - matrix[1].x;
		frustumPlanes[1].z = matrix[2].w - matrix[2].x;
		frustumPlanes[1].w = matrix[3].w - matrix[3].x;

		frustumPlanes[2].x = matrix[0].w - matrix[0].y;
		frustumPlanes[2].y = matrix[1].w - matrix[1].y;
		frustumPlanes[2].z = matrix[2].w - matrix[2].y;
		frustumPlanes[2].w = matrix[3].w - matrix[3].y;

		frustumPlanes[3].x = matrix[0].w + matrix[0].y;
		frustumPlanes[3].y = matrix[1].w + matrix[1].y;
		frustumPlanes[3].z = matrix[2].w + matrix[2].y;
		frustumPlanes[3].w = matrix[3].w + matrix[3].y;

		frustumPlanes[4].x = matrix[0].w + matrix[0].z;
		frustumPlanes[4].y = matrix[1].w + matrix[1].z;
		frustumPlanes[4].z = matrix[2].w + matrix[2].z;
		frustumPlanes[4].w = matrix[3].w + matrix[3].z;

		frustumPlanes[5].x = matrix[0].w - matrix[0].z;
		frustumPlanes[5].y = matrix[1].w - matrix[1].z;
		frustumPlanes[5].z = matrix[2].w - matrix[2].z;
		frustumPlanes[5].w = matrix[3].w - matrix[3].z;

		for (auto i = 0; i < 6; i++) frustumPlanes[i] = glm::normalize(frustumPlanes[i]);
	}

	void Camera::reset()
	{
		Controller::reset();
		coord = glm::vec3(0.f);
		length = 1.0f;
		needUpdateMat = true;
		changed = true;
	}

	void Camera::rotateByCursor(float x, float y)
	{
		addEuler(glm::vec3(-x * 180.f, 0.f, -y * 180.f));
	}

	void Camera::moveByCursor(float x, float y)
	{
		auto l = length / TKE_NEAR;
		auto cy = tan(glm::radians(TKE_FOVY / 2.f)) * TKE_NEAR * 2.f;
		target += (-x * cy * aspect * l) * axis[0] + (y * cy * l) * axis[1];
		lookAtTarget();
	}

	void Camera::scroll(float value)
	{
		if (mode == CameraModeTargeting)
		{
			if (value < 0.f)
				length = (length + 0.1) * 1.1f;
			else
				length = (length / 1.1f) - 0.1f;
			if (length < 1.f)
			{
				length = 1.f;
				coord += glm::normalize(target - coord) * 0.5f;
			}
			needUpdateMat = true;
			changed = true;
		}
	}

	void Camera::move()
	{
		glm::vec3 coord;
		glm::vec3 euler;
		if (!Controller::move(getEuler().x, coord, euler))
			return;
		switch (mode)
		{
		case CameraModeFree:
			addCoord(coord);
			break;
		case CameraModeTargeting:
			setTarget(target + coord);
			break;
		}
		addEuler(euler);
	}

	Light::Light(LightType _type)
		:type(_type)
	{}

	std::string getLightTypeName(LightType _type)
	{
		char *typeNames[] = {
			"parallax light",
			"point light",
			"spot light"
		};
		return typeNames[_type];
	}

	Object::Object(Model *_model, ObjectPhysicsType _physicsType)
		:model(_model), physicsType(_physicsType)
	{
		if (model->animated)
			animationComponent = new AnimationComponent(model);
	}

	Object::~Object()
	{
		delete animationComponent;
	}

	Terrain::Terrain(TerrainType _type)
		:type(_type)
	{}

	static const float gravity = 9.81f;

	UniformBuffer *matrixBuffer = nullptr;
	UniformBuffer *staticObjectMatrixBuffer = nullptr;
	UniformBuffer *animatedObjectMatrixBuffer = nullptr;
	IndirectIndexBuffer *staticObjectIndirectBuffer = nullptr;
	IndirectIndexBuffer *animatedObjectIndirectBuffer = nullptr;
	UniformBuffer *heightMapTerrainBuffer = nullptr;
	UniformBuffer *proceduralTerrainBuffer = nullptr;
	UniformBuffer *lightBuffer = nullptr;
	UniformBuffer *ambientBuffer = nullptr;

	Image *envrImage = nullptr;
	Image *envrImageDownsample[3] = {};

	Image *mainImage = nullptr;
	Image *albedoAlphaImage = nullptr;
	Image *normalHeightImage = nullptr;
	Image *specRoughnessImage = nullptr;

	RenderPass *sceneRenderPass = nullptr;

	Pipeline *scatteringPipeline = nullptr;

	Pipeline *downsamplePipeline = nullptr;

	Pipeline *convolvePipeline = nullptr;

	Pipeline *panoramaPipeline = nullptr;

	Pipeline *mrtPipeline;

	Pipeline *mrtAnimPipeline;
	static int mrt_bone_position = -1;

	Pipeline *heightMapTerrainPipeline = nullptr;
	static int heightMapTerr_heightMap_position = -1;
	static int heightMapTerr_colorMap_position = -1;

	Pipeline *proceduralTerrainPipeline = nullptr;

	Pipeline *deferredPipeline = nullptr;
	static int defe_envr_position = -1;

	Pipeline *composePipeline = nullptr;

	Scene::Scene()
	{
		physx::PxSceneDesc pxSceneDesc(pxPhysics->getTolerancesScale());
		pxSceneDesc.gravity = physx::PxVec3(0.0f, -gravity, 0.0f);
		pxSceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		pxSceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		pxScene = pxPhysics->createScene(pxSceneDesc);
		pxControllerManager = PxCreateControllerManager(*pxScene);
	}

	Scene::~Scene()
	{
		for (auto pLight : lights)
			delete pLight;

		for (auto pObject : objects)
			delete pObject;

		delete terrain;
	}

	void Scene::loadSky(const char *skyMapFilename, int radianceMapCount, const char *radianceMapFilenames[], const char *irradianceMapFilename)
	{
		//WIN32_FIND_DATA fd;
		//HANDLE hFind;

		//hFind = FindFirstFile(sprintf("%s\\pano.*", dir), &fd);
		//if (hFind != INVALID_HANDLE_VALUE)
		//{
		//	auto pImage = createImage(sprintf("%s\\%s", dir, fd.cFileName), true);
		//	if (pImage)
		//	{
		//		delete skyImage;
		//		skyImage = pImage;
		//	}
		//	FindClose(hFind);
		//}

		//std::vector<std::string> mipmapNames;
		//for (int i = 0; i < 100; i++)
		//{
		//	hFind = FindFirstFile(sprintf("%s\\rad%d.*", dir, i), &fd);
		//	if (hFind == INVALID_HANDLE_VALUE)
		//		break;
		//	FindClose(hFind);
		//	mipmapNames.push_back(sprintf("%s\\%s", dir, fd.cFileName));
		//}
		//auto pImage = createImage(mipmapNames, true);
		//if (pImage)
		//{
		//	strcpy(pImage->m_fileName, dir);
		//	delete radianceImage;
		//	radianceImage = pImage;
		//}

		//hFind = FindFirstFile(sprintf("%s\\irr.*", dir), &fd);
		//if (hFind != INVALID_HANDLE_VALUE)
		//{
		//	auto pImage = createImage(sprintf("%s\\%s", dir, fd.cFileName), true);
		//	if (pImage)
		//	{
		//		delete irradianceImage;
		//		irradianceImage = pImage;
		//	}
		//	FindClose(hFind);
		//}

		//strcpy(skyName, dir);

		needUpdateSky = true;
	}

	void Scene::addLight(Light *pLight) // when a light is added to scene, the owner is the scene, light cannot be deleted elsewhere
	{
		mtx.lock();
		lights.push_back(pLight);
		lightCountChanged = true;
		mtx.unlock();
	}

	Light *Scene::removeLight(Light *pLight)
	{
		mtx.lock();
		for (auto it = lights.begin(); it != lights.end(); it++)
		{
			if (*it == pLight)
			{
				for (auto itt = it + 1; itt != lights.end(); itt++)
				{
					(*itt)->sceneIndex--;
					if ((*itt)->shadow && pLight->shadow)
					{
						if (pLight->type == LightTypeParallax)
							(*itt)->sceneShadowIndex--;
						else if (pLight->type == LightTypePoint)
							(*itt)->sceneShadowIndex -= 6;
					}
					(*itt)->changed = true;
				}
				delete pLight;
				it = lights.erase(it);
				if (it == lights.end())
					pLight = nullptr;
				else
					pLight = *it;
				break;
			}
		}
		lightCountChanged = true;
		mtx.unlock();
		return pLight;
	}

	void Scene::addObject(Object *o) // when a object is added to scene, the owner is the scene, object cannot be deleted elsewhere
									 // and, if object has physics componet, it can be only moved by physics
	{
		auto m = o->model;

		mtx.lock();

		// since object can move to somewhere first, we create physics component here
		if (o->physicsType & ObjectPhysicsTypeStatic || o->physicsType & ObjectPhysicsTypeDynamic)
		{
			if (m->rigidbodies.size() > 0)
			{
				auto objScale = o->getScale();
				auto objCoord = o->getCoord();
				auto objAxis = o->getAxis();
				physx::PxTransform objTrans(objCoord.x, objCoord.y, objCoord.z, physx::PxQuat(physx::PxMat33(
					physx::PxVec3(objAxis[0][0], objAxis[0][1], objAxis[0][2]),
					physx::PxVec3(objAxis[1][0], objAxis[1][1], objAxis[1][2]),
					physx::PxVec3(objAxis[2][0], objAxis[2][1], objAxis[2][2]))));

				for (auto r : m->rigidbodies)
				{
					RigidBodyData rigidbodyData;
					rigidbodyData.rigidbody = r;

					auto rigidCoord = r->getCoord();
					if (r->boneID != -1) rigidCoord += m->bones[r->boneID].rootCoord;
					rigidCoord *= objScale;
					auto rigidAxis = r->getAxis();

					rigidbodyData.rotation = objAxis * rigidAxis;
					rigidbodyData.coord = objCoord + objAxis * rigidCoord;
					physx::PxTransform rigTrans(rigidCoord.x, rigidCoord.y, rigidCoord.z, physx::PxQuat(physx::PxMat33(
						physx::PxVec3(rigidAxis[0][0], rigidAxis[0][1], rigidAxis[0][2]),
						physx::PxVec3(rigidAxis[1][0], rigidAxis[1][1], rigidAxis[1][2]),
						physx::PxVec3(rigidAxis[2][0], rigidAxis[2][1], rigidAxis[2][2]))));
					rigTrans = objTrans * rigTrans;
					auto actor = ((o->physicsType & ObjectPhysicsTypeDynamic) && (r->type == RigidbodyTypeDynamic || r->type == RigidbodyTypeDynamicButLocation)) ?
						createDynamicRigidActor(rigTrans, false, r->density) : createStaticRigidActor(rigTrans);

					for (auto s : r->shapes)
					{
						glm::vec3 coord = s->getCoord() * objScale;
						glm::mat3 axis = s->getAxis();
						glm::vec3 scale = s->getScale() * objScale;
						physx::PxTransform trans(coord.x, coord.y, coord.z, physx::PxQuat(physx::PxMat33(
							physx::PxVec3(axis[0][0], axis[0][1], axis[0][2]),
							physx::PxVec3(axis[1][0], axis[1][1], axis[1][2]),
							physx::PxVec3(axis[2][0], axis[2][1], axis[2][2]))));
						switch (s->type)
						{
						case ShapeTypeBox:
							actor->createShape(physx::PxBoxGeometry(scale[0], scale[1], scale[2]), *pxDefaultMaterial, trans);
							break;
						case ShapeTypeSphere:
							actor->createShape(physx::PxSphereGeometry(scale[0]), *pxDefaultMaterial, trans);
							break;
						case ShapeTypeCapsule:
							actor->createShape(physx::PxCapsuleGeometry(scale[0], scale[1]), *pxDefaultMaterial, trans * physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
							break;
						}
					}

					rigidbodyData.actor = actor;

					o->rigidbodyDatas.push_back(rigidbodyData);

					pxScene->addActor(*actor);
				}
			}

			// create joints

			//		int jID = 0;
			//		for (auto j : pModel->joints)
			//		{
			//			if (j->rigid0ID == j->rigid1ID)
			//			{
			//				jID++;
			//				continue;
			//			}

			//			auto pR0 = pModel->rigidbodies[j->rigid0ID];
			//			auto pR1 = pModel->rigidbodies[j->rigid1ID];
			//			auto coord0 = (pR0->getCoord() + pModel->bones[pR0->boneID].rootCoord);
			//			coord0 = j->getCoord() - coord0;
			//			//coord0 = - coord0;
			//			auto coord1 = (pR1->getCoord() + pModel->bones[pR1->boneID].rootCoord);
			//			coord1 = j->getCoord() - coord1;
			//			//coord1 =  - coord1;
			//			auto axis = j->getAxis();
			//			auto axis0 = glm::transpose(pR0->getAxis());
			//			auto axis1 = glm::transpose(pR1->getAxis());
			//			auto t = PxTransform(PxQuat(PxMat33(
			//				PxVec3(axis[0][0], axis[0][1], axis[0][2]),
			//				PxVec3(axis[1][0], axis[1][1], axis[1][2]),
			//				PxVec3(axis[2][0], axis[2][1], axis[2][2]))));
			//			auto t0 = PxTransform(PxQuat(PxMat33(
			//				PxVec3(axis0[0][0], axis0[0][1], axis0[0][2]),
			//				PxVec3(axis0[1][0], axis0[1][1], axis0[1][2]),
			//				PxVec3(axis0[2][0], axis0[2][1], axis0[2][2]))));
			//			auto t1 = PxTransform(PxQuat(PxMat33(
			//				PxVec3(axis1[0][0], axis1[0][1], axis1[0][2]),
			//				PxVec3(axis1[1][0], axis1[1][1], axis1[1][2]),
			//				PxVec3(axis1[2][0], axis1[2][1], axis1[2][2]))));
			//			//auto j = PxSphericalJointCreate(*physics, (PxRigidActor*)pR0->phyActor, t * PxTransform(PxVec3(coord0.x, coord0.y, coord0.z)),
			//			//	(PxRigidActor*)pR1->phyActor, t * PxTransform(PxVec3(coord1.x, coord1.y, coord1.z)));
			//			auto p = PxD6JointCreate(*pxPhysics, (PxRigidActor*)pR0->phyActor, t0 * PxTransform(PxVec3(coord0.x, coord0.y, coord0.z)) * t,
			//				(PxRigidActor*)pR1->phyActor, t1 * PxTransform(PxVec3(coord1.x, coord1.y, coord1.z)) * t);
			//			p->setConstraintFlag(PxConstraintFlag::Enum::eCOLLISION_ENABLED, true);
			//			p->setSwingLimit(PxJointLimitCone(PxPi / 4, PxPi / 4));
			//			p->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
			//			p->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
			//			//auto p = PxSphericalJointCreate(*physics, (PxRigidActor*)pR0->phyActor, PxTransform(PxVec3(coord0.x, coord0.y, coord0.z), PxQuat(PxMat33(
			//			//	PxVec3(axis1[0][0], axis1[0][1], axis1[0][2]),
			//			//	PxVec3(axis1[1][0], axis1[1][1], axis1[1][2]),
			//			//	PxVec3(axis1[2][0], axis1[2][1], axis1[2][2])))),
			//			//	(PxRigidActor*)pR1->phyActor, PxTransform(PxVec3(coord1.x, coord1.y, coord1.z), PxQuat(PxMat33(
			//			//		PxVec3(axis1[0][0], axis1[0][1], axis1[0][2]),
			//			//		PxVec3(axis1[1][0], axis1[1][1], axis1[1][2]),
			//			//		PxVec3(axis1[2][0], axis1[2][1], axis1[2][2])))));

			//			//break;
			//			//if (jID == 0)
			//			//p->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
			//			jID++;
			//		}
		}

		if (o->physicsType & ObjectPhysicsTypeController)
		{
			auto centerPos = ((m->maxCoord + m->minCoord) * 0.5f) * o->getScale() + o->getCoord();
			physx::PxCapsuleControllerDesc capsuleDesc;
			capsuleDesc.height = (m->maxCoord.y - m->minCoord.y) * o->getScale().y;
			capsuleDesc.radius = glm::max((m->maxCoord.x - m->minCoord.x) * o->getScale().x, (m->maxCoord.z - m->minCoord.z) * o->getScale().z) * 0.5f;
			capsuleDesc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
			capsuleDesc.material = pxDefaultMaterial;
			capsuleDesc.position.x = centerPos.x;
			capsuleDesc.position.y = centerPos.y;
			capsuleDesc.position.z = centerPos.z;
			capsuleDesc.stepOffset = capsuleDesc.radius;

			o->pxController = pxControllerManager->createController(capsuleDesc);
		}

		objects.push_back(o);

		needUpdateIndirectBuffer = true;
		mtx.unlock();
	}

	Object *Scene::removeObject(Object *pObject)
	{
		mtx.lock();
		for (auto it = objects.begin(); it != objects.end(); it++)
		{
			if (*it == pObject)
			{
				for (auto itt = it + 1; itt != objects.end(); itt++)
				{
					(*itt)->sceneIndex--;
					(*itt)->changed = true;
				}
				delete pObject;
				if (it > objects.begin())
					pObject = *(it - 1);
				else
					pObject = nullptr;
				objects.erase(it);
				break;
			}
		}
		needUpdateIndirectBuffer = true;
		mtx.unlock();
		return pObject;
	}

	int Scene::getCollisionGroupID(int ID, unsigned int mask)
	{
		if (mask == 0)
		{
			return -1;
		}
		auto count = pCollisionGroups.size();
		for (int i = 0; i < count; i++)
		{
			if (pCollisionGroups[i]->originalID == ID && pCollisionGroups[i]->originalmask == mask)
			{
				return i;
			}
		}
		auto c = new CollisionGroup;
		c->originalID = ID;
		c->originalmask = mask;
		pCollisionGroups.push_back(c);
		return count;
	}

	void Scene::addTerrain(Terrain *pTerrain) // when a terrain is added to scene, the owner is the scene, terrain cannot be deleted elsewhere
	{
		mtx.lock();

		terrain = pTerrain;

		mtx.unlock();
	}

	void Scene::removeTerrain()
	{
		mtx.lock();

		delete terrain;
		terrain = nullptr;

		mtx.unlock();
	}

	void Scene::clear()
	{
		mtx.lock();

		pSunLight = nullptr;

		for (auto pLight : lights)
			delete pLight;
		lights.clear();

		for (auto pObject : objects)
			delete pObject;
		objects.clear();

		delete terrain;
		terrain = nullptr;

		mtx.unlock();
	}

	Framebuffer *Scene::createFramebuffer(Image *dst)
	{
		VkImageView views[] = {
			mainImage->getView(),
			depthImage->getView(),
			albedoAlphaImage->getView(),
			normalHeightImage->getView(),
			specRoughnessImage->getView(),
			dst->getView(),
		};
		return getFramebuffer(resCx, resCy, sceneRenderPass, ARRAYSIZE(views), views);
	}

	struct MatrixBufferShaderStruct
	{
		glm::mat4 proj;
		glm::mat4 projInv;
		glm::mat4 view;
		glm::mat4 viewInv;
		glm::mat4 projView;
		glm::mat4 projViewRotate;
		glm::vec4 frustumPlanes[6];
		glm::vec2 viewportDim;
	};

	void Scene::show(CommandBuffer *cb, Framebuffer *fb, VkEvent signalEvent)
	{
		// update animation and bones
		for (auto object : objects)
		{
			if (object->animationComponent)
				object->animationComponent->update();
		}

		// update physics (controller should move first, then simulate, and then get the result coord)
		auto dist = (timeDisp) / 1000.f;
		if (dist > 0.f)
		{
			for (auto object : objects) // set controller coord
			{
				if (object->physicsType & ObjectPhysicsTypeController)
				{
					glm::vec3 e, c;
					object->move(object->getEuler().x, c, e);
					object->addEuler(e);

					physx::PxVec3 disp(c.x, -gravity * object->floatingTime * object->floatingTime, c.z);
					object->floatingTime += dist;

					if (object->pxController->move(disp, 0.f, dist, nullptr) & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
						object->floatingTime = 0.f;
				}
			}
			pxScene->simulate(dist);
			pxScene->fetchResults(true);
			for (auto o : objects)
			{
				if (o->physicsType & ObjectPhysicsTypeDynamic)
				{
					auto pModel = o->model;

					auto objScale = o->getScale();
					auto objCoord = o->getCoord();
					auto objAxis = o->getAxis();
					physx::PxTransform objTrans(objCoord.x, objCoord.y, objCoord.z, physx::PxQuat(physx::PxMat33(
						physx::PxVec3(objAxis[0][0], objAxis[0][1], objAxis[0][2]),
						physx::PxVec3(objAxis[1][0], objAxis[1][1], objAxis[1][2]),
						physx::PxVec3(objAxis[2][0], objAxis[2][1], objAxis[2][2]))));

					for (auto &data : o->rigidbodyDatas)
					{
						if (data.rigidbody->boneID == -1)
						{
							auto trans = data.actor->getGlobalPose();
							auto coord = glm::vec3(trans.p.x, trans.p.y, trans.p.z);
							auto quat = glm::vec4(trans.q.x, trans.q.y, trans.q.z, trans.q.w);
							o->setCoord(coord);
							o->setQuat(quat);
							glm::mat3 axis;
							quaternionToMatrix(quat, axis);
							data.coord = coord;
							data.rotation = axis;
						}
						//else
						//{
						//	auto solver = pObject->animationSolver;
						//	if (r->mode == Rigidbody::Mode::eStatic)
						//	{
						//		auto pBone = &pModel->bones[r->boneID];
						//		auto coord = objAxis * (glm::vec3(solver->boneMatrix[r->boneID][3]) + glm::mat3(solver->boneMatrix[r->boneID]) * r->getCoord()) * objScale + objCoord;
						//		auto axis = objAxis * glm::mat3(solver->boneMatrix[r->boneID]) * r->getAxis();
						//		PxTransform trans(coord.x, coord.y, coord.z, PxQuat(PxMat33(
						//			PxVec3(axis[0][0], axis[0][1], axis[0][2]),
						//			PxVec3(axis[1][0], axis[1][1], axis[1][2]),
						//			PxVec3(axis[2][0], axis[2][1], axis[2][2]))));
						//		((PxRigidDynamic*)body)->setKinematicTarget(trans);
						//		pObject->rigidDatas[id].coord = coord;
						//		pObject->rigidDatas[id].rotation = axis;
						//	}
						//	else
						//	{
						//		auto objAxisT = glm::transpose(objAxis);
						//		auto rigidAxis = r->getAxis();
						//		auto rigidAxisT = glm::transpose(rigidAxis);
						//		auto trans = body->getGlobalPose();
						//		auto coord = glm::vec3(trans.p.x, trans.p.y, trans.p.z);
						//		glm::mat3 axis;
						//		Math::quaternionToMatrix(glm::vec4(trans.q.x, trans.q.y, trans.q.z, trans.q.w), axis);
						//		pObject->rigidDatas[id].coord = coord;
						//		pObject->rigidDatas[id].rotation = axis;
						//		auto boneAxis = objAxisT * axis * rigidAxisT;
						//		glm::vec3 boneCoord;
						//		if (r->mode != Rigidbody::Mode::eDynamicLockLocation)
						//			boneCoord = (objAxisT * (coord - objCoord) - boneAxis * (r->getCoord() * objScale)) / objScale;
						//		else
						//			boneCoord = glm::vec3(solver->boneMatrix[r->boneID][3]);
						//		solver->boneMatrix[r->boneID] = Math::makeMatrix(boneAxis, boneCoord);
						//	}
						//}
					}
				}

				if (o->physicsType & ObjectPhysicsTypeController)
				{
					auto p = o->pxController->getPosition();
					auto c = glm::vec3(p.x, p.y, p.z) - (o->model->maxCoord + o->model->minCoord) * 0.5f * o->getScale();
					o->setCoord(c);
				}
			}
		}

		camera.move();
		if (camera.changed)
		{
			camera.lookAtTarget();
			camera.updateFrustum();
			// update procedural terrain
			{

				auto pos = camera.getCoord();
				auto seed = glm::vec2(pos.x - 500.f, pos.z - 500.f);
				seed /= 1000.f;
				proceduralTerrainBuffer->update(&seed, *stagingBuffer);
			}
		}
		{ // always update the matrix buffer
			MatrixBufferShaderStruct stru;
			stru.proj = matPerspective;
			stru.projInv = matPerspective;
			stru.view = camera.getMatInv();
			stru.viewInv = camera.getMat();
			stru.projView = stru.proj * stru.view;
			stru.projViewRotate = stru.proj * glm::mat4(glm::mat3(stru.view));
			memcpy(stru.frustumPlanes, camera.frustumPlanes, sizeof(glm::vec4) * 6);
			stru.viewportDim = glm::vec2(resCx, resCy);
			matrixBuffer->update(&stru, *stagingBuffer);
		}
		if (needUpdateSky)
		{
			AmbientBufferShaderStruct stru;
			stru.v = glm::vec4(ambientColor, 0);
			stru.fogcolor = glm::vec4(0.f, 0.f, 0.f, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
			ambientBuffer->update(&stru, *stagingBuffer);

			if (skyType == SkyTypeNull)
			{
			}
			else if (skyType == SkyTypePanorama)
			{
				// TODO : FIX SKY FROM FILE
				//if (skyImage)
				//{
				//	//writes.push_back(vk->writeDescriptorSet(engine->panoramaPipeline.m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, skyImage->getInfo(engine->colorSampler), 0));
				//	//writes.push_back(vk->writeDescriptorSet(engine->deferredPipeline.m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7, radianceImage->getInfo(engine->colorSampler), 0));

				//	AmbientBufferShaderStruct stru;
				//	stru.v = glm::vec4(1.f, 1.f, 1.f, skyImage->level - 1);
				//	stru.fogcolor = glm::vec4(0.f, 0.f, 1.f, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
				//	ambientBuffer->update(&stru, *stagingBuffer);
				//}
			}
			else if (skyType == SkyTypeAtmosphereScattering)
			{
				if (panoramaPipeline)
				{ // update Atmospheric Scattering
					{
						float fScaleDepth = 0.25;// The scale depth (i.e. the altitude at which the atmosphere's average density is found)

						auto mrX = glm::mat3(glm::rotate(atmosphereSunDir.x, glm::vec3(0.f, 1.f, 0.f)));
						auto v3LightPosition = glm::normalize(glm::mat3(glm::rotate(atmosphereSunDir.y, mrX * glm::vec3(0.f, 0.f, 1.f))) * mrX * glm::vec3(1.f, 0.f, 0.f));// The direction vector to the light source

						if (pSunLight)
							pSunLight->setEuler(glm::vec3(atmosphereSunDir, 0.f));

						float fScale = 1.f / (atmosphereOuterRadius - atmosphereInnerRadius);	// 1 / (fOuterRadius - fInnerRadius)
						float fScaleOverScaleDepth = fScale / fScaleDepth;	// fScale / fScaleDepth
					}

					{
						auto cb = commandPool->begineOnce();
						auto fb = getFramebuffer(envrImage, plainRenderPass_image16);

						cb->beginRenderPass(plainRenderPass_image16, fb);
						cb->bindPipeline(scatteringPipeline);
						cb->draw(3);
						cb->endRenderPass();

						commandPool->endOnce(cb);
						releaseFramebuffer(fb);
					}

					if (deferredPipeline)
					{ // update IBL
						if (defe_envr_position != -1)
						{
							static int down_source_position = -1;
							if (down_source_position == -1) down_source_position = downsamplePipeline->descriptorPosition("source");
							if (down_source_position != -1)
							{
								for (int i = 0; i < 3; i++)
								{
									auto cb = commandPool->begineOnce();
									auto fb = getFramebuffer(envrImageDownsample[i], plainRenderPass_image16);

									cb->beginRenderPass(plainRenderPass_image16, fb);
									cb->bindPipeline(downsamplePipeline);
									cb->setViewportAndScissor(TKE_ENVR_SIZE_CX >> (i + 1), TKE_ENVR_SIZE_CY >> (i + 1));
									auto size = glm::vec2(TKE_ENVR_SIZE_CX >> (i + 1), TKE_ENVR_SIZE_CY >> (i + 1));
									cb->pushConstant(StageType::frag, 0, sizeof glm::vec2, &size);
									downsamplePipeline->descriptorSet->setImage(down_source_position, 0, i == 0 ? envrImage : envrImageDownsample[i - 1], plainSampler);
									cb->bindDescriptorSet();
									cb->draw(3);
									cb->endRenderPass();

									commandPool->endOnce(cb);
									releaseFramebuffer(fb);
								}
							}

							static int con_source_position = -1;
							if (con_source_position == -1) con_source_position = convolvePipeline->descriptorPosition("source");
							if (con_source_position != -1)
							{
								for (int i = 1; i < envrImage->level; i++)
								{
									auto cb = commandPool->begineOnce();
									auto fb = getFramebuffer(envrImage, plainRenderPass_image16, i);

									cb->beginRenderPass(plainRenderPass_image16, fb);
									cb->bindPipeline(convolvePipeline);
									auto data = 1.f + 1024.f - 1024.f * (i / 3.f);
									cb->pushConstant(StageType::frag, 0, sizeof(float), &data);
									cb->setViewportAndScissor(TKE_ENVR_SIZE_CX >> i, TKE_ENVR_SIZE_CY >> i);
									convolvePipeline->descriptorSet->setImage(con_source_position, 0, envrImageDownsample[i - 1], plainSampler);
									cb->bindDescriptorSet();
									cb->draw(3);
									cb->endRenderPass();

									commandPool->endOnce(cb);
									releaseFramebuffer(fb);
								}
							}

							deferredPipeline->descriptorSet->setImage(defe_envr_position, 0, envrImage, colorSampler, 0, 0, envrImage->level);

							AmbientBufferShaderStruct stru;
							stru.v = glm::vec4(1.f, 1.f, 1.f, 3);
							stru.fogcolor = glm::vec4(0.f, 0.f, 1.f, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
							ambientBuffer->update(&stru, *stagingBuffer);
						}
					}
				}
			}

			needUpdateSky = false;
		}
		if (objects.size() > 0)
		{
			int updateCount = 0;
			std::vector<VkBufferCopy> staticUpdateRanges;
			std::vector<VkBufferCopy> animatedUpdateRanges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(glm::mat4) * objects.size());
			int staticObjectIndex = 0;
			int animatedObjectIndex = 0;

			for (auto pObject : objects)
			{
				if (!pObject->model->animated)
				{
					if (pObject->changed)
					{
						auto srcOffset = sizeof(glm::mat4) * updateCount;
						memcpy(map + srcOffset, &pObject->getMat(), sizeof(glm::mat4));
						VkBufferCopy range = {};
						range.srcOffset = srcOffset;
						range.dstOffset = sizeof(glm::mat4) * staticObjectIndex;
						range.size = sizeof(glm::mat4);
						staticUpdateRanges.push_back(range);

						updateCount++;
					}
					pObject->sceneIndex = staticObjectIndex;
					staticObjectIndex++;
				}
				else
				{
					if (pObject->changed)
					{
						auto srcOffset = sizeof(glm::mat4) * updateCount;
						memcpy(map + srcOffset, &pObject->getMat(), sizeof(glm::mat4));
						VkBufferCopy range = {};
						range.srcOffset = srcOffset;
						range.dstOffset = sizeof(glm::mat4) * animatedObjectIndex;
						range.size = sizeof(glm::mat4);
						animatedUpdateRanges.push_back(range);

						updateCount++;
					}
					pObject->sceneIndex = animatedObjectIndex;
					animatedObjectIndex++;
				}

			}
			stagingBuffer->unmap();
			if (staticUpdateRanges.size() > 0) commandPool->copyBuffer(stagingBuffer->v, staticObjectMatrixBuffer->v, staticUpdateRanges.size(), staticUpdateRanges.data());
			if (animatedUpdateRanges.size() > 0) commandPool->copyBuffer(stagingBuffer->v, animatedObjectMatrixBuffer->v, animatedUpdateRanges.size(), animatedUpdateRanges.data());
		}
		if (terrain)
		{
			if (terrain->changed)
			{
				HeightMapTerrainShaderStruct stru;
				stru.ext = terrain->ext;
				stru.height = terrain->height;
				stru.tessFactor = terrain->tessFactor;
				stru.mapDim = terrain->heightMap ? terrain->heightMap->width : 1024;

				heightMapTerrainBuffer->update(&stru, *stagingBuffer);

				if (heightMapTerr_heightMap_position != -1 && terrain->heightMap)
					heightMapTerrainPipeline->descriptorSet->setImage(heightMapTerr_heightMap_position, 0, terrain->heightMap, colorBorderSampler);
				if (heightMapTerr_colorMap_position != -1 && terrain->colorMap)
					heightMapTerrainPipeline->descriptorSet->setImage(heightMapTerr_colorMap_position, 0, terrain->colorMap, colorBorderSampler);
			}
		}
		if (needUpdateIndirectBuffer)
		{
			if (objects.size() > 0)
			{
				std::vector<VkDrawIndexedIndirectCommand> staticCommands;
				std::vector<VkDrawIndexedIndirectCommand> animatedCommands;

				int staticIndex = 0;
				int animatedIndex = 0;

				for (auto pObject : objects)
				{
					auto pModel = pObject->model;

					if (!pModel->animated)
					{
						for (auto mt : pModel->materials)
						{
							VkDrawIndexedIndirectCommand command = {};
							command.instanceCount = 1;
							command.indexCount = mt->indiceCount;
							command.vertexOffset = pModel->vertexBase;
							command.firstIndex = pModel->indiceBase + mt->indiceBase;
							command.firstInstance = (staticIndex << 16) + mt->sceneIndex;

							staticCommands.push_back(command);
						}

						staticIndex++;
					}
					else
					{
						for (auto mt : pModel->materials)
						{
							VkDrawIndexedIndirectCommand command = {};
							command.instanceCount = 1;
							command.indexCount = mt->indiceCount;
							command.vertexOffset = pModel->vertexBase;
							command.firstIndex = pModel->indiceBase + mt->indiceBase;
							command.firstInstance = (animatedIndex << 16) + mt->sceneIndex;

							animatedCommands.push_back(command);
						}

						if (mrt_bone_position != -1)
							mrtAnimPipeline->descriptorSet->setBuffer(mrt_bone_position, animatedIndex, pObject->animationComponent->boneMatrixBuffer);

						animatedIndex++;
					}
				}

				staticIndirectCount = staticCommands.size();
				animatedIndirectCount = animatedCommands.size();

				if (staticCommands.size() > 0) staticObjectIndirectBuffer->update(staticCommands.data(), *stagingBuffer, sizeof(VkDrawIndexedIndirectCommand) * staticCommands.size());
				if (animatedCommands.size() > 0) animatedObjectIndirectBuffer->update(animatedCommands.data(), *stagingBuffer, sizeof(VkDrawIndexedIndirectCommand) * animatedCommands.size());
			}
			needUpdateIndirectBuffer = false;
		}
		if (lights.size() > 0)
		{ // light attribute
			int lightIndex = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(LightShaderStruct) * lights.size());
			for (auto pLight : lights)
			{
				if (pLight->changed)
				{
					auto srcOffset = sizeof(LightShaderStruct) * ranges.size();
					LightShaderStruct stru;
					if (pLight->type == LightTypeParallax)
						stru.coord = glm::vec4(pLight->getAxis()[2], 0.f);
					else
						stru.coord = glm::vec4(pLight->getCoord(), pLight->type);
					stru.color = glm::vec4(pLight->color, 1.f);
					stru.spotData = glm::vec4(-pLight->getAxis()[2], pLight->range);
					memcpy(map + srcOffset, &stru, sizeof(LightShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = 16 + sizeof(LightShaderStruct) * lightIndex;
					range.size = sizeof(LightShaderStruct);
					ranges.push_back(range);
				}
				lightIndex++;
			}
			stagingBuffer->unmap();
			if (ranges.size() > 0) commandPool->copyBuffer(stagingBuffer->v, lightBuffer->v, ranges.size(), ranges.data());
		}
		if (lights.size() > 0)
		{ // shadow
			shadowCount = 0;

			for (auto pLight : lights)
			{
				if (pLight->shadow)
				{
					pLight->sceneShadowIndex = shadowCount;
					if (pLight->type == LightTypeParallax)
					{
						if (pLight->changed || camera.changed)
						{
							glm::vec3 p[8];
							auto cameraCoord = camera.coord;
							for (int i = 0; i < 8; i++) p[i] = camera.frustumPoints[i] - cameraCoord;
							auto lighAxis = pLight->getAxis();
							auto axisT = glm::transpose(lighAxis);
							auto vMax = axisT * p[0], vMin = vMax;
							for (int i = 1; i < 8; i++)
							{
								auto tp = axisT * p[i];
								vMax = glm::max(tp, vMax);
								vMin = glm::min(tp, vMin);
							}

							auto halfWidth = (vMax.z - vMin.z) * 0.5f;
							auto halfHeight = (vMax.y - vMin.y) * 0.5f;
							auto halfDepth = glm::max(vMax.x - vMin.x, TKE_NEAR) * 0.5f;

							auto center = lighAxis * ((vMax + vMin) * 0.5f) + cameraCoord;

							auto shadowMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, TKE_NEAR, halfDepth + halfDepth) * glm::lookAt(center - halfDepth * lighAxis[0], center, lighAxis[1]);
						}
						shadowCount++;
					}
					else if (pLight->type == LightTypePoint)
					{
						if (pLight->changed)
						{
							glm::mat4 shadowMatrix[6];

							auto coord = pLight->getCoord();
							auto proj = glm::perspective(90.f, 1.f, TKE_NEAR, TKE_FAR);
							shadowMatrix[0] = proj * glm::lookAt(coord, coord + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
							shadowMatrix[1] = proj * glm::lookAt(coord, coord + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
							shadowMatrix[2] = proj * glm::lookAt(coord, coord + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
							shadowMatrix[3] = proj * glm::lookAt(coord, coord + glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
							shadowMatrix[4] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
							shadowMatrix[5] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
						}
						shadowCount += 6;
					}
				}
			}
		}
		if (lightCountChanged)
		{ // light count in light attribute
			auto count = lights.size();
			lightBuffer->update(&count, *stagingBuffer, 4);
			lightCountChanged = false;
		}

		camera.changed = false;

		for (auto pLight : lights)
			pLight->changed = false;
		for (auto pObject : objects)
			pObject->changed = false;
		if (terrain)
			terrain->changed = false;

		cb->reset();
		cb->begin();

		cb->beginRenderPass(sceneRenderPass, fb);

		// sky
		cb->bindVertexBuffer(staticVertexBuffer);
		cb->bindIndexBuffer(staticIndexBuffer);
		cb->bindPipeline(panoramaPipeline);
		cb->bindDescriptorSet();
		cb->drawIndex(sphereModel->indices.size(), sphereModel->indiceBase, sphereModel->vertexBase);

		// mrt
		cb->nextSubpass();
			// static
		cb->bindVertexBuffer(staticVertexBuffer);
		cb->bindIndexBuffer(staticIndexBuffer);
		cb->bindPipeline(mrtPipeline);
		cb->bindDescriptorSet();
		cb->drawIndirectIndex(staticObjectIndirectBuffer, staticIndirectCount);
			// animated
		cb->bindVertexBuffer(animatedVertexBuffer);
		cb->bindIndexBuffer(animatedIndexBuffer);
		cb->bindPipeline(mrtAnimPipeline);
		cb->bindDescriptorSet();
		cb->drawIndirectIndex(animatedObjectIndirectBuffer, animatedIndirectCount);
			// terrain
		if (terrain)
		{
			switch (terrain->type)
			{
			case TerrainTypeHeightMap:
				cb->bindPipeline(heightMapTerrainPipeline);
				cb->bindDescriptorSet();
				cb->draw(4, 0, TKE_PATCH_SIZE * TKE_PATCH_SIZE);
				break;
			}
		}

		// deferred
		cb->nextSubpass();
		cb->bindPipeline(deferredPipeline);
		cb->bindDescriptorSet();
		cb->draw(3);

		// compose
		cb->nextSubpass();
		cb->bindPipeline(composePipeline);
		cb->bindDescriptorSet();
		cb->draw(3);

		cb->endRenderPass();

		cb->setEvent(signalEvent);
		cb->end();
	}

	void Scene::load(const std::string &_filename)
	{
		filename = _filename;

		tke::AttributeTree at("scene", filename);
		auto a = at.firstAttribute("name");
		name = a->value;
		for (auto c : at.children)
		{
			if (c->name == "object")
			{
				auto a = c->firstAttribute("model");
				tke::Model *m = nullptr;
				for (auto _m : models)
				{
					if (_m->filename == a->value)
					{
						m = _m;
						break;
					}
				}
				if (m)
				{
					auto object = new tke::Object(m);
					for (auto a : c->attributes)
					{
						if (a->name == "coord_x")
							object->setCoordX(std::stof(a->value));
						if (a->name == "coord_y")
							object->setCoordY(std::stof(a->value));
						if (a->name == "coord_z")
							object->setCoordZ(std::stof(a->value));
						if (a->name == "euler_x")
							object->setEulerX(std::stof(a->value));
						if (a->name == "euler_y")
							object->setEulerY(std::stof(a->value));
						if (a->name == "euler_z")
							object->setEulerZ(std::stof(a->value));
						if (a->name == "scale_x")
							object->setScaleX(std::stof(a->value));
						if (a->name == "scale_y")
							object->setScaleY(std::stof(a->value));
						if (a->name == "scale_z")
							object->setScaleZ(std::stof(a->value));
					}
					addObject(object);
				}
			}
			else if (c->name == "light")
			{
				;
			}
		}
	}

	void Scene::save(const std::string &filename)
	{
		tke::AttributeTree at("scene");
		at.attributes.push_back(new tke::Attribute("name", &name));
		for (auto object : objects)
		{
			auto n = new AttributeTreeNode("object");
			n->attributes.push_back(new tke::Attribute("model", &object->model->filename));
			static glm::vec3 coord, euler, scale;
			coord = object->getCoord();
			euler = object->getEuler();
			scale = object->getScale();
			n->attributes.push_back(new tke::Attribute("coord_x", &coord.x));
			n->attributes.push_back(new tke::Attribute("coord_y", &coord.y));
			n->attributes.push_back(new tke::Attribute("coord_z", &coord.z));
			n->attributes.push_back(new tke::Attribute("euler_x", &euler.x));
			n->attributes.push_back(new tke::Attribute("euler_y", &euler.y));
			n->attributes.push_back(new tke::Attribute("euler_z", &euler.z));
			n->attributes.push_back(new tke::Attribute("scale_x", &scale.x));
			n->attributes.push_back(new tke::Attribute("scale_y", &scale.y));
			n->attributes.push_back(new tke::Attribute("scale_z", &scale.z));
			at.children.push_back(n);
		}
		at.saveXML(filename);
	}

	//	Pipeline proceduralTerrainPipeline;

	//MasterRenderer::MasterRenderer(int _cx, int _cy, Window *pWindow, VertexBuffer *vertexBuffer, IndexBuffer *indexBuffer, IndexedIndirectBuffer *indirectBuffer)
	//{
	//	// TODO : FIX PROCEDURAL TERRAIN
	//	//auto mrtProceduralTerrainAction = mrtPass->addAction(&proceduralTerrainPipeline);
	//	//mrtProceduralTerrainAction->addDrawcall(4, 0, 100 * 100, 0);

	//}

	void initScene()
	{
		matrixBuffer = new UniformBuffer(sizeof MatrixBufferShaderStruct);
		staticObjectMatrixBuffer = new UniformBuffer(sizeof(glm::mat4) * TKE_MAX_STATIC_OBJECT_COUNT);
		animatedObjectMatrixBuffer = new UniformBuffer(sizeof(glm::mat4) * TKE_MAX_ANIMATED_OBJECT_COUNT);
		heightMapTerrainBuffer = new UniformBuffer(sizeof(HeightMapTerrainShaderStruct) * 8);
		proceduralTerrainBuffer = new UniformBuffer(sizeof(glm::vec2));
		lightBuffer = new UniformBuffer(sizeof(LightBufferShaderStruct));
		ambientBuffer = new UniformBuffer(sizeof AmbientBufferShaderStruct);

		staticObjectIndirectBuffer = new IndirectIndexBuffer(sizeof(VkDrawIndexedIndirectCommand) * TKE_MAX_INDIRECT_COUNT);
		animatedObjectIndirectBuffer = new IndirectIndexBuffer(sizeof(VkDrawIndexedIndirectCommand) * TKE_MAX_INDIRECT_COUNT);

		envrImage = new Image(TKE_ENVR_SIZE_CX, TKE_ENVR_SIZE_CY, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 4);
		for (int i = 0; i < 3; i++)
			envrImageDownsample[i] = new Image(TKE_ENVR_SIZE_CX >> (i + 1), TKE_ENVR_SIZE_CY >> (i + 1), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		globalResource.setImage(envrImage, "Envr.Image");

		mainImage = new Image(resCx, resCy, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		albedoAlphaImage = new Image(resCx, resCy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		normalHeightImage = new Image(resCx, resCy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		specRoughnessImage = new Image(resCx, resCy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		globalResource.setImage(mainImage, "Main.Image");
		globalResource.setImage(albedoAlphaImage, "AlbedoAlpha.Image");
		globalResource.setImage(normalHeightImage, "NormalHeight.Image");
		globalResource.setImage(specRoughnessImage, "SpecRoughness.Image");

		globalResource.setBuffer(matrixBuffer, "Matrix.UniformBuffer");
		globalResource.setBuffer(staticObjectMatrixBuffer, "StaticObjectMatrix.UniformBuffer");
		globalResource.setBuffer(animatedObjectMatrixBuffer, "AnimatedObjectMatrix.UniformBuffer");
		globalResource.setBuffer(heightMapTerrainBuffer, "HeightMapTerrain.UniformBuffer");
		globalResource.setBuffer(proceduralTerrainBuffer, "ProceduralTerrain.UniformBuffer");
		globalResource.setBuffer(lightBuffer, "Light.UniformBuffer");
		globalResource.setBuffer(ambientBuffer, "Ambient.UniformBuffer");

		globalResource.setBuffer(staticObjectIndirectBuffer, "Scene.Static.IndirectBuffer");
		globalResource.setBuffer(animatedObjectIndirectBuffer, "Scene.Animated.IndirectBuffer");

		VkAttachmentDescription atts[] = {
			colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_DONT_CARE), // main
			depthAttachmentDesc(VK_FORMAT_D32_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR),				 // depth
			colorAttachmentDesc(VK_FORMAT_R16G16B16A16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),			 // albedo alpha
			colorAttachmentDesc(VK_FORMAT_R16G16B16A16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),			 // normal height
			colorAttachmentDesc(VK_FORMAT_R16G16B16A16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),			 // spec roughness
			colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_DONT_CARE)		 // dst
		};
		VkAttachmentReference main_col_ref = { 0, VK_IMAGE_LAYOUT_GENERAL };
		VkAttachmentReference mrt_col_ref[] = {
			{ 2, VK_IMAGE_LAYOUT_GENERAL },
			{ 3, VK_IMAGE_LAYOUT_GENERAL },
			{ 4, VK_IMAGE_LAYOUT_GENERAL }
		};
		VkAttachmentReference dep_ref = { 1, VK_IMAGE_LAYOUT_GENERAL };
		VkAttachmentReference dst_col_ref = { 5, VK_IMAGE_LAYOUT_GENERAL };
		VkSubpassDescription subpasses[] = {
			subpassDesc(1, &main_col_ref),                              // sky
			subpassDesc(ARRAYSIZE(mrt_col_ref), mrt_col_ref, &dep_ref), // mrt
			subpassDesc(1, &main_col_ref),                              // deferred
			subpassDesc(1, &dst_col_ref)                                // compose
		};

		VkSubpassDependency dependencies[] = {
			subpassDependency(0, 2),
			subpassDependency(1, 2),
			subpassDependency(2, 3)
		};

		sceneRenderPass = new RenderPass(ARRAYSIZE(atts), atts, ARRAYSIZE(subpasses), subpasses, ARRAYSIZE(dependencies), dependencies);

		scatteringPipeline = new Pipeline;
		scatteringPipeline->loadXML(enginePath + "pipeline/sky/scattering.xml");
		scatteringPipeline->setup(plainRenderPass_image16, 0);
		globalResource.setPipeline(scatteringPipeline);

		downsamplePipeline = new Pipeline;
		downsamplePipeline->loadXML(enginePath + "pipeline/sky/downsample.xml");
		downsamplePipeline->setup(plainRenderPass_image16, 0);
		globalResource.setPipeline(downsamplePipeline);

		convolvePipeline = new Pipeline;
		convolvePipeline->loadXML(enginePath + "pipeline/sky/convolve.xml");
		convolvePipeline->setup(plainRenderPass_image16, 0);
		globalResource.setPipeline(convolvePipeline);

		panoramaPipeline = new Pipeline;
		panoramaPipeline->loadXML(enginePath + "pipeline/sky/panorama.xml");
		panoramaPipeline->setup(sceneRenderPass, 0);
		globalResource.setPipeline(panoramaPipeline);

		mrtPipeline = new Pipeline;
		mrtPipeline->loadXML(enginePath + "pipeline/deferred/mrt.xml");
		mrtPipeline->setup(sceneRenderPass, 1);
		//globalResource.setPipeline(mrtPipeline);

		mrtAnimPipeline = new Pipeline;
		mrtAnimPipeline->loadXML(enginePath + "pipeline/deferred/mrt_anim.xml");
		mrtAnimPipeline->setup(sceneRenderPass, 1);
		//globalResource.setPipeline(mrtAnimPipeline);
		mrt_bone_position = mrtAnimPipeline->descriptorPosition("BONE");

		heightMapTerrainPipeline = new Pipeline;
		heightMapTerrainPipeline->loadXML(enginePath + "pipeline/deferred/height_map_terrain.xml");
		heightMapTerrainPipeline->setup(sceneRenderPass, 1);
		heightMapTerr_heightMap_position = heightMapTerrainPipeline->descriptorPosition("heightMap");
		heightMapTerr_colorMap_position = heightMapTerrainPipeline->descriptorPosition("colorMap");

		//proceduralTerrainPipeline = new Pipeline;
		//proceduralTerrainPipeline->loadXML(enginePath + "pipeline/deferred/procedural_terrain.xml");
		//proceduralTerrainPipeline->setup(sceneRenderPass, 1);

		deferredPipeline = new Pipeline;
		deferredPipeline->loadXML(enginePath + "pipeline/deferred/deferred.xml");
		deferredPipeline->setup(sceneRenderPass, 2);
		//globalResource.setPipeline(deferredPipeline);
		defe_envr_position = deferredPipeline->descriptorPosition("envrSampler");

		composePipeline = new Pipeline;
		composePipeline->loadXML(enginePath + "pipeline/compose/compose.xml");
		composePipeline->setup(sceneRenderPass, 3);
		//globalResource.setPipeline(composePipeline);
	}
}