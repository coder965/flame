#include <algorithm>
#include <sstream>
#include <filesystem>
#include <iostream>

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

	std::vector<Animation*> animations;

	AnimationBinding::~AnimationBinding()
	{
		for (auto t : pTracks)
			delete t;
	}

	std::string shapeTypeName(ShapeType t)
	{
		char *names[] = {
			"Box",
			"Sphere",
			"Capsule",
			"Plane",
			"Convex Mesh",
			"Triangle Mesh",
			"Height Field"
		};
		return names[(int)t];
	}

	Shape::Shape()
	{
	}

	Shape::Shape(ShapeType _type)
		:type(_type)
	{
	}

	float Shape::getVolume() const
	{
		auto size = getScale();
		switch (type)
		{
		case ShapeTypeBox:
			return size.x * size.y * size.z * 8.f;
		case ShapeTypeSphere:
			return 4.f * size.x * size.x * size.x * M_PI / 3.f;
		case ShapeTypeCapsule:
			return 4.f * size.x * size.x * size.x * M_PI / 3.f + M_PI * size.x * size.x * size.y;
		}
		return 0.f;
	}

	Rigidbody::Rigidbody()
	{
	}

	Rigidbody::Rigidbody(RigidbodyType _type)
		:type(_type)
	{
	}

	Rigidbody::~Rigidbody()
	{
		for (auto s : shapes)
			delete s;
	}

	void Rigidbody::addShape(Shape *pShape)
	{
		static auto magicNumber = 0;
		pShape->id = magicNumber++;
		shapes.push_back(pShape);
	}

	Shape *Rigidbody::deleteShape(Shape *pShape)
	{
		for (auto it = shapes.begin(); it != shapes.end(); it++)
		{
			if (*it == pShape)
			{
				if (it > shapes.begin())
					pShape = *(it - 1);
				else
					pShape = nullptr;
				shapes.erase(it);
				break;
			}
		}
		return pShape;
	}

	void Model::loadDat(const std::string &filename)
	{
		std::ifstream file(filename);
		if (!file.good()) return;
		int count;
		file >> count;
		for (auto p : rigidbodies) delete p;
		rigidbodies.clear();
		for (int i = 0; i < count; i++)
		{
			auto rb = new Rigidbody;
			file.read((char*)rb, sizeof(Rigidbody));

			rigidbodies.push_back(rb);
		}
		file >> eyePosition;
	}

	void Model::saveDat(const std::string &filename)
	{
		std::ofstream file(filename);
		if (!file.good()) return;
		file << rigidbodies.size();
		for (int i = 0; i < rigidbodies.size(); i++)
			file.write((char*)&rigidbodies[i], sizeof(Rigidbody));

		file << eyePosition;
	}

	AnimationBinding *Model::bindAnimation(Animation *pAnimationTemplate)
	{
		auto pAnimation = new AnimationBinding;
		pAnimation->pTemplate = pAnimationTemplate;
		for (auto &motion : pAnimationTemplate->motions)
		{
			pAnimation->frameTotal = glm::max(pAnimation->frameTotal, motion.frame);

			int boneID = -1;
			for (int iBone = 0; iBone < bones.size(); iBone++)
			{
				if (motion.name.compare(bones[iBone].name) == 0)
				{
					boneID = iBone;
					break;
				}
			}
			if (boneID == -1) continue;

			BoneMotionTrack *pTrack = nullptr;
			for (auto t : pAnimation->pTracks)
			{
				if (t->boneID == boneID)
				{
					pTrack = t;
					break;
				}
			}
			if (!pTrack)
			{
				pTrack = new BoneMotionTrack;
				pTrack->boneID = boneID;
				pTrack->pMotions.push_back(&motion);
				pAnimation->pTracks.push_back(pTrack);
			}
			else
			{
				bool inserted = false;
				for (int i = 0; i < pTrack->pMotions.size(); i++)
				{
					if (pTrack->pMotions[i]->frame > motion.frame)
					{
						pTrack->pMotions.insert(pTrack->pMotions.begin() + i, &motion);
						inserted = true;
						break;
					}
				}
				if (!inserted) pTrack->pMotions.push_back(&motion);
			}
		}
		animations.push_back(pAnimation);
		return pAnimation;
	}

	Image *Model::getImage(const char *name)
	{
		for (auto pImage : pImages)
		{
			if (pImage->filename.compare(name) == 0)
				return pImage;
		}
		return nullptr;
	}

	void Model::addRigidbody(Rigidbody *pRigidbody)
	{
		static auto magicNumber = 0;
		pRigidbody->id = magicNumber++;
		rigidbodies.push_back(pRigidbody);
	}

	Rigidbody *Model::deleteRigidbody(Rigidbody *pRigidBody)
	{
		for (auto it = rigidbodies.begin(); it != rigidbodies.end(); it++)
		{
			if (*it == pRigidBody)
			{
				if (it > rigidbodies.begin())
					pRigidBody = *(it - 1);
				else
					pRigidBody = nullptr;
				rigidbodies.erase(it);
				break;
			}
		}
		return pRigidBody;
	}

	void Model::addJoint(Joint *pJoint)
	{
		static auto magicNumber = 0;
		pJoint->id = magicNumber++;
		joints.push_back(pJoint);
	}

	std::vector<Model*> models;
	static void _add_model(Model *m)
	{
		for (auto src : m->pImages)
		{
			bool found = false;
			for (auto i : modelTextures)
			{
				if (i == src)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				src->index = modelTextures.size();
				modelTextures.push_back(src);
			}
		}
		for (auto mt : m->materials)
		{
			MaterialShaderStruct stru;
			stru.albedoAlphaCompress = mt->albedoR + (mt->albedoG << 8) + (mt->albedoB << 16) + (mt->alpha << 24);
			stru.specRoughnessCompress = mt->spec + (mt->roughness << 8);

			auto albedoAlphaMapIndex = mt->albedoAlphaMap ? mt->albedoAlphaMap->index + 1 : 0;
			auto normalHeightMapIndex = mt->normalHeightMap ? mt->normalHeightMap->index + 1 : 0;
			auto specRoughnessMapIndex = mt->specRoughnessMap ? mt->specRoughnessMap->index + 1 : 0;

			int sameIndex = -1;
			int materialIndex = 0;
			for (auto &mt : modelMaterials)
			{
				auto storeAlbedoAlphaMapIndex = mt.mapIndex & 0xff;
				auto storeNormalHeightMapIndex = (mt.mapIndex >> 8) & 0xff;
				auto storeSpecRoughnessMapIndex = (mt.mapIndex >> 16) & 0xff;

				bool theSameAlbedoAlpha = false;
				bool theSameNormalHeight = false;
				bool theSameSpecRoughness = false;

				if (albedoAlphaMapIndex != 0 && albedoAlphaMapIndex == storeAlbedoAlphaMapIndex)
					theSameAlbedoAlpha = true;
				else if (albedoAlphaMapIndex == 0 && storeAlbedoAlphaMapIndex == 0 &&
					stru.albedoAlphaCompress == mt.albedoAlphaCompress)
					theSameAlbedoAlpha = true;
				if (normalHeightMapIndex == storeNormalHeightMapIndex)
					theSameNormalHeight = true;
				if (specRoughnessMapIndex != 0 && specRoughnessMapIndex == storeSpecRoughnessMapIndex)
					theSameSpecRoughness = true;
				else if (specRoughnessMapIndex == 0 && storeSpecRoughnessMapIndex == 0 &&
					stru.specRoughnessCompress == mt.specRoughnessCompress)
					theSameSpecRoughness = true;

				if (theSameAlbedoAlpha && theSameNormalHeight && theSameSpecRoughness)
				{
					sameIndex = materialIndex;
					break;
				}

				materialIndex++;
			}
			if (sameIndex == -1)
			{
				mt->sceneIndex = modelMaterials.size();
				stru.mapIndex = albedoAlphaMapIndex + (normalHeightMapIndex << 8) + (specRoughnessMapIndex << 16);
				modelMaterials.push_back(stru);
			}
			else
			{
				mt->sceneIndex = sameIndex;
			}
		}
		models.push_back(m);
		needUpdateVertexBuffer = true;
		needUpdateMaterialBuffer = true;
		needUpdateTexture = true;
	}

	void clearModel()
	{
		for (auto m : models)
			delete m;
		models.clear();
	}

	AnimationComponent::AnimationComponent(Model *_model)
	{
		model = _model;
		boneData = new BoneData[model->bones.size()];
		boneMatrix = new glm::mat4[model->bones.size()];
		boneMatrixBuffer = new UniformBuffer(sizeof(glm::mat4) * model->bones.size());
		boneMatrixBuffer->update(boneMatrix, *stagingBuffer, sizeof(glm::mat4) * model->bones.size());
	}

	AnimationComponent::~AnimationComponent()
	{
		delete[]boneData;
		delete[]boneMatrix;
		delete boneMatrixBuffer;
	}

	void AnimationComponent::refreshBone()
	{
		assert(model);

		for (int i = 0; i < model->bones.size(); i++)
		{
			boneMatrix[i] = glm::translate(model->bones[i].relateCoord + boneData[i].coord) * glm::mat4(boneData[i].rotation);
			if (model->bones[i].parent != -1) boneMatrix[i] = boneMatrix[model->bones[i].parent] * boneMatrix[i];
		}
	}

	void AnimationComponent::refreshBone(int i)
	{
		assert(model && i < model->bones.size());

		boneMatrix[i] = glm::translate(model->bones[i].relateCoord + boneData[i].coord) * glm::mat4(boneData[i].rotation);
		if (model->bones[i].parent != -1) boneMatrix[i] = boneMatrix[model->bones[i].parent] * boneMatrix[i];

		for (auto child : model->bones[i].children)
			refreshBone(child);
	}

	void AnimationComponent::setAnimation(AnimationBinding *animation)
	{
		if (!animation && currentAnimation)
		{
			for (int i = 0; i < model->bones.size(); i++)
			{
				boneData[i].coord = glm::vec3();
				boneData[i].rotation = glm::mat3();
				boneMatrix[i] = glm::mat4();
			}
			boneMatrixBuffer->update(boneMatrix, *stagingBuffer, sizeof(glm::mat4) * model->bones.size());
		}
		currentAnimation = animation;
		currentFrame = 0;
		currentTime = nowTime;
	}

	void AnimationComponent::update()
	{
		const float dist = 1000.f / 60.f;

		if (currentAnimation)
		{
			for (int i = 0; i < model->bones.size(); i++)
			{
				boneMatrix[i] = glm::mat4();
				boneData[i].rotation = glm::mat3();
				boneData[i].coord = glm::vec3();
			}

			for (auto pTrack : currentAnimation->pTracks)
			{
				auto pBoneData = &boneData[pTrack->boneID];
				auto it = std::upper_bound(pTrack->pMotions.rbegin(), pTrack->pMotions.rend(), currentFrame, [](int frame, BoneMotion *bm) {
					return frame > bm->frame;
				});

				if (it == pTrack->pMotions.rend()) continue;

				auto pLeftMotion = *it;
				auto pRightMotion = (it == pTrack->pMotions.rbegin() ? pTrack->pMotions[0] : *(it - 1));

				auto beta = 0.f;
				if (pLeftMotion != pRightMotion) beta = (currentFrame - pLeftMotion->frame) / (pRightMotion->frame - pLeftMotion->frame);

				tke::quaternionToMatrix(glm::normalize((1.f - beta) * pLeftMotion->quaternion + beta * pRightMotion->quaternion), pBoneData->rotation);
				pBoneData->coord = pLeftMotion->coord + (pRightMotion->coord - pLeftMotion->coord) * beta;
			}

			currentFrame += (nowTime - currentTime) / dist;
			if (currentFrame >= currentAnimation->frameTotal)
				currentFrame = currentAnimation->frameTotal - 1.f;
			currentTime = nowTime;
		}

		refreshBone();

		if (processedIK)
		{
			//	for (int i = 0; i < pModel->iks.size(); i++)
			//	{
			//		auto &ik = pModel->iks[i];
			//		auto t = glm::vec3(boneMatrix[ik.targetID][3]);
			//		//t.z *= -1.f;
			//		for (int times = 0; times < ik.iterations; times++)
			//		{
			//			for (int index = 0; index < ik.chain.size(); index++)
			//			{
			//				//index = iChain;
			//				auto e = glm::vec3(boneMatrix[ik.effectorID][3]);
			//				//e.z *= -1.f;
			//				if (glm::length(t - e) < 0.0001f)
			//				{
			//					goto nextIk;
			//				}

			//				auto boneID = ik.chain[index];

			//				auto p = glm::vec3(boneMatrix[boneID][3]);
			//				//p.z *= -1.f;

			//				auto pe = glm::normalize(e - p);
			//				auto pt = glm::normalize(t - p);
			//				auto theDot = glm::dot(pe, pt);
			//				theDot = glm::clamp(theDot, 0.f, 1.f);
			//				auto theAcos = glm::acos(theDot);
			//				auto ang = glm::degrees(theAcos);
			//				if (glm::abs(ang) > 0.5f)
			//				{
			//					auto n = glm::normalize(glm::cross(pe, pt));
			//					if (glm::abs(n.z) < 1.f)
			//					{
			//						n.z = 0;
			//						n = glm::normalize(n);
			//					}
			//					boneData[boneID].rotation = glm::mat3(glm::rotate(ang, n)) * boneData[boneID].rotation;
			//					//refreshBone(ik.effectorID, boneData, outMat);
			//					pModel->refreshBone(boneID, boneData, boneMatrix);
			//					p = glm::vec3(boneMatrix[boneID][3]);
			//					e = glm::vec3(boneMatrix[ik.effectorID][3]);
			//					pe = glm::normalize(e - p);
			//					auto dot = glm::dot(pe, pt);
			//					int cut = 1;
			//				}
			//				//break;
			//			}
			//		}
			//	nextIk:
			//		//break;
			//		continue;
			//	}
			//}
		}

		for (int i = 0; i < model->bones.size(); i++)
			boneMatrix[i] *= glm::translate(-model->bones[i].rootCoord);

		boneMatrixBuffer->update(boneMatrix, *stagingBuffer, sizeof(glm::mat4) * model->bones.size());
	}

	void addTriangleVertex(Model *m, glm::mat3 rotation, glm::vec3 center)
	{
		int baseVertex = m->positions.size();

		m->positions.insert(m->positions.end(), { center + rotation * glm::vec3(0.f, 0.f, 0.f), center + rotation * glm::vec3(0.f, 1.f, 0.f), center + rotation * glm::vec3(1.f, 0.f, 0.f) });
		m->normals.insert(m->normals.end(), 3, glm::vec3(0.f, 1.f, 0.f));
		for (int i = 0; i < 3; i++) m->indices.push_back(baseVertex + i);
	}

	void addCubeVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float length)
	{
		int baseVertex = m->positions.size();

		glm::vec3 a = center + rotation * (glm::vec3(0.5f, -0.5f, 0.5f) * length);
		glm::vec3 b = center + rotation * (glm::vec3(0.5f, -0.5f, -0.5f) * length);
		glm::vec3 c = center + rotation * (glm::vec3(0.5f, 0.5f, -0.5f) * length);
		glm::vec3 d = center + rotation * (glm::vec3(0.5f, 0.5f, 0.5f) * length);
		glm::vec3 e = center + rotation * (glm::vec3(-0.5f, -0.5f, 0.5f) * length);
		glm::vec3 f = center + rotation * (glm::vec3(-0.5f, -0.5f, -0.5f) * length);
		glm::vec3 g = center + rotation * (glm::vec3(-0.5f, 0.5f, -0.5f) * length);
		glm::vec3 h = center + rotation * (glm::vec3(-0.5f, 0.5f, 0.5f) * length);

		m->positions.insert(m->positions.end(), {
			a, b, c, d,
			e, f, g, h,
			c, d, g, h,
			a, b, e, f,
			a, d, e, h,
			b, c, f, g
		});

		m->normals.insert(m->normals.end(), 4, rotation * glm::vec3(1.f, 0.f, 0.f));
		m->normals.insert(m->normals.end(), 4, rotation * glm::vec3(-1.f, 0.f, 0.f));
		m->normals.insert(m->normals.end(), 4, rotation * glm::vec3(0.f, 1.f, 0.f));
		m->normals.insert(m->normals.end(), 4, rotation * glm::vec3(0.f, -1.f, 0.f));
		m->normals.insert(m->normals.end(), 4, rotation * glm::vec3(0.f, 0.f, 1.f));
		m->normals.insert(m->normals.end(), 4, rotation * glm::vec3(0.f, 0.f, -1.f));

		std::vector<int> list = {
			3, 0, 1, 3, 1, 2,
			6, 5, 4, 6, 4, 7,
			11, 9, 8, 11, 8, 10,
			12, 14, 15, 12, 15, 13,
			19, 18, 16, 19, 16, 17,
			21, 20, 22, 21, 22, 23
		};

		for (auto &i : list) i += baseVertex;

		m->indices.insert(m->indices.end(), list.begin(), list.end());
	}

	void addSphereVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float radius, int horiSubdiv, int vertSubdiv)
	{
		std::vector<std::vector<int>> indexs;
		indexs.resize(horiSubdiv + 1);

		for (int level = 1; level < horiSubdiv; level++)
		{
			for (int i = 0; i < vertSubdiv; i++)
			{
				auto radian = glm::radians(level * 180.f / horiSubdiv - 90.f);
				auto ringRadius = cos(radian) * radius;
				auto height = sin(radian) * radius;
				auto ang = glm::radians(i * 360.f / vertSubdiv);
				auto index = m->positions.size();
				indexs[level].push_back(index);
				glm::vec3 v = rotation * glm::vec3(cos(ang) * ringRadius, height, sin(ang) * ringRadius);
				m->positions.push_back(center + v);
				m->normals.push_back(glm::normalize(v));
			}
		}

		{
			auto index = m->positions.size();
			indexs[0].push_back(index);
			glm::vec3 v = rotation * glm::vec3(0.f, -radius, 0.f);
			m->positions.push_back(center + v);
			m->normals.push_back(glm::normalize(v));
		}

		{
			auto index = m->positions.size();
			indexs[horiSubdiv].push_back(index);
			glm::vec3 v = rotation * glm::vec3(0.f, radius, 0.f);
			m->positions.push_back(center + v);
			m->normals.push_back(glm::normalize(v));
		}

		for (int level = 0; level < horiSubdiv; level++)
		{
			if (level == 0)
			{
				for (int i = 0; i < vertSubdiv; i++)
				{
					auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

					m->indices.push_back(indexs[0][0]);
					m->indices.push_back(indexs[1][i]);
					m->indices.push_back(indexs[1][ii]);
				}
			}
			else if (level == horiSubdiv - 1)
			{
				for (int i = 0; i < vertSubdiv; i++)
				{
					auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

					m->indices.push_back(indexs[horiSubdiv - 1][i]);
					m->indices.push_back(indexs[horiSubdiv][0]);
					m->indices.push_back(indexs[horiSubdiv - 1][ii]);
				}
			}
			else
			{
				for (int i = 0; i < vertSubdiv; i++)
				{
					auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

					m->indices.push_back(indexs[level][i]);
					m->indices.push_back(indexs[level + 1][i]);
					m->indices.push_back(indexs[level][ii]);

					m->indices.push_back(indexs[level][ii]);
					m->indices.push_back(indexs[level + 1][i]);
					m->indices.push_back(indexs[level + 1][ii]);
				}
			}
		}
	}

	void addCylinderVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float halfHeight, float radius, int subdiv)
	{
		std::vector<std::vector<int>> indexs;
		indexs.resize(4);

		// top cap
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = m->positions.size();
			indexs[0].push_back(index);
			m->positions.push_back(rotation * glm::vec3(cos(ang) * radius, halfHeight, sin(ang) * radius) + center);
			m->normals.push_back(rotation * glm::vec3(0.f, 1.f, 0.f));
		}

		// bottom cap
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = m->positions.size();
			indexs[1].push_back(index);
			m->positions.push_back(rotation * glm::vec3(cos(ang) * radius, -halfHeight, sin(ang) * radius) + center);
			m->normals.push_back(rotation * glm::vec3(0.f, -1.f, 0.f));
		}

		// top
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = m->positions.size();
			indexs[2].push_back(index);
			m->positions.push_back(rotation * glm::vec3(cos(ang) * radius, halfHeight, sin(ang) * radius) + center);
			m->normals.push_back(rotation * glm::vec3(cos(ang), 0.f, sin(ang)));
		}

		// bottom
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = m->positions.size();
			indexs[3].push_back(index);
			m->positions.push_back(rotation * glm::vec3(cos(ang) * radius, -halfHeight, sin(ang) * radius) + center);
			m->normals.push_back(rotation * glm::vec3(cos(ang), 0.f, sin(ang)));
		}

		// top cap
		for (int i = 1; i < subdiv - 1; i++)
		{
			m->indices.push_back(indexs[0][i]);
			m->indices.push_back(indexs[0][0]);
			m->indices.push_back(indexs[0][i + 1]);
		}

		// bottom cap
		for (int i = 1; i < subdiv - 1; i++)
		{
			m->indices.push_back(indexs[1][i + 1]);
			m->indices.push_back(indexs[1][0]);
			m->indices.push_back(indexs[1][i]);
		}

		// middle
		for (int i = 0; i < subdiv; i++)
		{
			auto ii = i + 1; if (ii == subdiv) ii = 0;

			m->indices.push_back(indexs[2][i]);
			m->indices.push_back(indexs[2][ii]);
			m->indices.push_back(indexs[3][i]);

			m->indices.push_back(indexs[2][ii]);
			m->indices.push_back(indexs[3][ii]);
			m->indices.push_back(indexs[3][i]);
		}
	}

	void addConeVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float height, float radius, int subdiv)
	{
		std::vector<std::vector<int>> indexs;
		indexs.resize(3);

		// top
		{
			auto index = m->positions.size();
			indexs[0].push_back(index);
			m->positions.push_back(rotation * glm::vec3(0.f, height, 0.f) + center);
			m->normals.push_back(rotation * glm::vec3(0.f, 1.f, 0.f));
		}

		// cap
		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = m->positions.size();
			indexs[1].push_back(index);
			m->positions.push_back(rotation * glm::vec3(cos(ang) * radius, 0.f, sin(ang) * radius) + center);
			m->normals.push_back(rotation * glm::vec3(0.f, -1.f, 0.f));
		}

		for (int i = 0; i < subdiv; i++)
		{
			auto ang = glm::radians(i * 360.f / subdiv);
			auto index = m->positions.size();
			indexs[2].push_back(index);
			m->positions.push_back(rotation * glm::vec3(cos(ang) * radius, 0.f, sin(ang) * radius) + center);
			m->normals.push_back(rotation * glm::vec3(cos(ang), 0.f, sin(ang)));
		}

		for (int i = 0; i < subdiv; i++)
		{
			auto ii = i + 1; if (ii == subdiv) ii = 0;

			m->indices.push_back(indexs[2][i]);
			m->indices.push_back(indexs[0][0]);
			m->indices.push_back(indexs[2][ii]);
		}

		for (int i = 1; i < subdiv - 1; i++)
		{
			m->indices.push_back(indexs[1][i + 1]);
			m->indices.push_back(indexs[1][0]);
			m->indices.push_back(indexs[1][i]);
		}
	}

	void addTorusVertex(Model *m, glm::mat3 rotation, glm::vec3 center, float radius, float sectionRadius, int axisSubdiv, int heightSubdiv)
	{
		std::vector<std::vector<int>> indexs;
		indexs.resize(axisSubdiv);

		for (int i = 0; i < axisSubdiv; i++)
		{
			float ang = i * 360.f / axisSubdiv;
			glm::mat3 R = glm::mat3(glm::rotate(-ang, glm::vec3(0.f, 1.f, 0.f)));
			for (int j = 0; j < heightSubdiv; j++)
			{
				auto secang = glm::radians(j * 360.f / heightSubdiv);
				auto index = m->positions.size();
				indexs[i].push_back(index);
				m->positions.push_back(rotation * (center + R * (glm::vec3(cos(secang) * sectionRadius + radius, sin(secang) * sectionRadius, 0.f))));
				m->normals.push_back(rotation * R * glm::vec3(cos(secang), sin(secang), 0.f));
			}
		}

		for (int i = 0; i < axisSubdiv; i++)
		{
			auto ii = i + 1; if (ii == axisSubdiv) ii = 0;

			for (int j = 0; j < heightSubdiv; j++)
			{
				auto jj = j + 1; if (jj == heightSubdiv) jj = 0;

				m->indices.push_back(indexs[i][j]);
				m->indices.push_back(indexs[i][jj]);
				m->indices.push_back(indexs[ii][j]);

				m->indices.push_back(indexs[i][jj]);
				m->indices.push_back(indexs[ii][jj]);
				m->indices.push_back(indexs[ii][j]);
			}
		}
	}

	Model *triangleModel = nullptr;
	Model *cubeModel = nullptr;
	Model *sphereModel = nullptr;
	Model *cylinderModel = nullptr;
	Model *coneModel = nullptr;
	Model *arrowModel = nullptr;
	Model *torusModel = nullptr;
	Model *hamerModel = nullptr;

	static void _model_after_process(Model *m)
	{
		m->maxCoord = m->positions[0];
		m->minCoord = m->positions[0];
		for (int i = 1; i < m->positions.size(); i++)
		{
			m->maxCoord = glm::max(m->maxCoord, m->positions[i]);
			m->minCoord = glm::min(m->minCoord, m->positions[i]);
		}

		if (m->tangents.size() == 0 && m->uvs.size() > 0)
		{
			m->tangents.resize(m->positions.size());

			for (int i = 0; i < m->indices.size(); i += 3)
			{
				int id[3] = {
					m->indices[i],
					m->indices[i + 1],
					m->indices[i + 2]
				};

				auto u0 = m->uvs[id[1]].s - m->uvs[id[0]].s;
				auto v0 = m->uvs[id[1]].t - m->uvs[id[0]].t;

				auto u1 = m->uvs[id[2]].s - m->uvs[id[0]].s;
				auto v1 = m->uvs[id[2]].t - m->uvs[id[0]].t;

				auto e0 = m->positions[id[1]] - m->positions[id[0]];
				auto e1 = m->positions[id[2]] - m->positions[id[0]];

				auto d = u0 * v1 - u1 * v0;
				if (d == 0.f) continue;

				auto tangent = glm::vec3(v1 * e0.x - v0 * e1.x, v1 * e0.y - v0 * e1.y, v1 * e0.z - v0 * e1.z);
				if (glm::length(tangent) > 0.f)
				{
					tangent = glm::normalize(tangent);
					m->tangents[id[0]] = tangent;
					m->tangents[id[1]] = tangent;
					m->tangents[id[2]] = tangent;
				}
				else
				{
					m->tangents[id[0]] = glm::vec3();
					m->tangents[id[1]] = glm::vec3();
					m->tangents[id[2]] = glm::vec3();
				}
			}
		}

		for (int i = 0; i < m->bones.size(); i++)
		{
			m->bones[i].relateCoord = m->bones[i].rootCoord;
			int parentID = m->bones[i].parent;
			if (parentID != -1)
			{
				m->bones[i].relateCoord -= m->bones[parentID].rootCoord;
				m->bones[parentID].children.push_back(i);
			}
		}
	}

	void initGeneralModels()
	{
		{
			triangleModel = new Model;
			triangleModel->name = "triangle";

			addTriangleVertex(triangleModel, glm::mat3(), glm::vec3());

			auto mt = new Material;
			mt->indiceCount = triangleModel->indices.size();
			triangleModel->materials.push_back(mt);

			_model_after_process(triangleModel);

			_add_model(triangleModel);

			globalResource.setModel(triangleModel, "Triangle.Model");
		}

		{
			cubeModel = new Model;
			cubeModel->name = "cube";

			addCubeVertex(cubeModel, glm::mat3(), glm::vec3(), 1.f);

			auto mt = new Material;
			mt->indiceCount = cubeModel->indices.size();
			cubeModel->materials.push_back(mt);

			auto pRigidbody = new Rigidbody(RigidbodyTypeDynamic);
			cubeModel->addRigidbody(pRigidbody);
			auto pShape = new Shape(ShapeTypeBox);
			pRigidbody->addShape(pShape);
			pShape->setScale(glm::vec3(0.5f));

			_model_after_process(cubeModel);

			_add_model(cubeModel);

			globalResource.setModel(cubeModel, "Cube.Model");
		}

		{
			sphereModel = new Model;
			sphereModel->name = "sphere";

			addSphereVertex(sphereModel, glm::mat3(), glm::vec3(), 0.5f, 32, 32);

			auto mt0 = new Material;
			mt0->indiceCount = sphereModel->indices.size() / 2;
			sphereModel->materials.push_back(mt0);
			auto mt1 = new Material;
			mt1->indiceBase = sphereModel->indices.size() / 2;
			mt1->indiceCount = sphereModel->indices.size() / 2;
			sphereModel->materials.push_back(mt1);

			auto pRigidbody = new Rigidbody(RigidbodyTypeDynamic);
			sphereModel->addRigidbody(pRigidbody);
			auto pShape = new Shape(ShapeTypeSphere);
			pRigidbody->addShape(pShape);
			pShape->setScale(glm::vec3(0.5f));

			_model_after_process(sphereModel);

			_add_model(sphereModel);

			globalResource.setModel(sphereModel, "Sphere.Model");
		}

		{
			cylinderModel = new Model;
			cylinderModel->name = "cylinder";

			addCylinderVertex(cylinderModel, glm::mat3(), glm::vec3(), 0.5f, 0.5f, 32);

			auto mt = new Material;
			mt->indiceCount = cylinderModel->indices.size();
			cylinderModel->materials.push_back(mt);

			auto pRigidbody = new Rigidbody(RigidbodyTypeDynamic);
			cylinderModel->addRigidbody(pRigidbody);
			auto pShape = new Shape(ShapeTypeCapsule);
			pRigidbody->addShape(pShape);
			pShape->setScale(glm::vec3(0.5f));

			_model_after_process(cylinderModel);

			_add_model(cylinderModel);

			globalResource.setModel(cylinderModel, "Cylinder.Model");
		}

		{
			coneModel = new Model;
			coneModel->name = "cone";

			addConeVertex(coneModel, glm::mat3(), glm::vec3(), 0.5f, 0.5f, 32);

			auto mt = new Material;
			mt->indiceCount = coneModel->indices.size();
			coneModel->materials.push_back(mt);

			_model_after_process(coneModel);

			_add_model(coneModel);

			globalResource.setModel(coneModel, "Cone.Model");
		}

		{
			arrowModel = new Model;
			arrowModel->name = "arrow";

			glm::mat3 matR = glm::mat3(glm::rotate(-90.f, glm::vec3(0.f, 0.f, 1.f)));

			addCylinderVertex(arrowModel, matR, glm::vec3(0.4f, 0.f, 0.f), 0.4f, 0.01f, 32);
			addConeVertex(arrowModel, matR, glm::vec3(0.8f, 0.f, 0.f), 0.2f, 0.05f, 32);

			auto mt = new Material;
			mt->indiceCount = arrowModel->indices.size();
			arrowModel->materials.push_back(mt);

			_model_after_process(arrowModel);

			_add_model(arrowModel);

			globalResource.setModel(arrowModel, "Arrow.Model");
		}

		{
			torusModel = new Model;
			torusModel->name = "torus";

			glm::mat3 matR = glm::mat3(glm::rotate(-90.f, glm::vec3(0.f, 0.f, 1.f)));

			addTorusVertex(torusModel, matR, glm::vec3(), 1.f, 0.01f, 32, 32);

			auto mt = new Material;
			mt->indiceCount = torusModel->indices.size();
			torusModel->materials.push_back(mt);

			_model_after_process(torusModel);

			_add_model(torusModel);

			globalResource.setModel(torusModel, "Torus.Model");
		}

		{
			hamerModel = new Model;
			hamerModel->name = "hammer";

			glm::mat3 matR = glm::mat3(glm::rotate(-90.f, glm::vec3(0.f, 0.f, 1.f)));

			addCylinderVertex(hamerModel, matR, glm::vec3(0.45f, 0.f, 0.f), 0.45f, 0.01f, 32);
			int ic0 = hamerModel->indices.size();
			addCubeVertex(hamerModel, matR, glm::vec3(0.9f, 0.f, 0.f), 0.1f);
			int ic1 = hamerModel->indices.size();

			auto mt0 = new Material;
			mt0->indiceCount = ic0;
			hamerModel->materials.push_back(mt0);
			auto mt1 = new Material;
			mt1->indiceBase = ic0;
			mt1->indiceCount = ic1 - ic0;
			hamerModel->materials.push_back(mt1);

			_model_after_process(hamerModel);

			_add_model(hamerModel);

			globalResource.setModel(hamerModel, "Hamer.Model");
		}
	}

	namespace OBJ
	{
		struct obj_ctype : std::ctype<char>
		{
			mask my_table[table_size];
			obj_ctype(size_t refs = 0)
				: std::ctype<char>(my_table, false, refs)
			{
				std::copy_n(classic_table(), table_size, my_table);
				my_table['/'] = (mask)space;
			}
		};
		std::stringstream obj_line_ss;
		struct Init
		{
			Init()
			{
				std::locale x(std::locale::classic(), new obj_ctype);
				obj_line_ss.imbue(x);
			}
		};
		static Init _init;

		void load(Model *m, std::ifstream &file)
		{
			int currentIndex = 0;
			Material *pmt = nullptr;

			std::vector<glm::vec3> rawPositions;
			std::vector<glm::vec2> rawTexcoords;
			std::vector<glm::vec3> rawNormals;
			std::vector<glm::ivec3> rawIndexs;

			while (!file.eof())
			{
				std::string line;
				std::getline(file, line);

				std::stringstream ss(line);
				std::string token;
				ss >> token;
				if (token == "v")
				{
					glm::vec3 v;
					ss >> v.x;
					ss >> v.y;
					ss >> v.z;
					rawPositions.push_back(v);
				}
				else if (token == "vn")
				{
					glm::vec3 n;
					ss >> n.x;
					ss >> n.y;
					ss >> n.z;
					rawNormals.push_back(n);
				}
				else if (token == "vt")
				{
					glm::vec2 t;
					ss >> t.x;
					ss >> t.y;
					rawTexcoords.push_back(t);
				}
				else if (token == "f")
				{
					for (int i = 0; i < 3; i++)
					{
						std::string token;
						ss >> token;
						obj_line_ss.clear();
						obj_line_ss << token;
						glm::ivec3 ids;
						for (int j = 0; j < 3; j++)
						{
							obj_line_ss >> ids[j];
							ids[j]--;
						}

						int index = -1;
						for (int i = 0; i < rawIndexs.size(); i++)
						{
							if (ids == rawIndexs[i])
							{
								index = i;
								break;
							}
						}

						if (index == -1)
						{
							index = m->positions.size();

							if (ids[0] < rawPositions.size()) m->positions.push_back(rawPositions[ids[0]]);
							else m->positions.push_back(glm::vec3(0.f));
							if (ids[1] != -1 && ids[1] < rawTexcoords.size()) m->uvs.push_back(rawTexcoords[ids[1]]);
							else m->uvs.push_back(glm::vec2(0.f));
							if (ids[2] != -1 && ids[2] < rawNormals.size()) m->normals.push_back(rawNormals[ids[2]]);
							else m->normals.push_back(glm::vec3(0.f));

							rawIndexs.push_back(ids);

						}
						m->indices.push_back(index);
						currentIndex++;
						pmt->indiceCount++;
					}
				}
				else if (token == "usemtl")
				{
					std::string name;
					ss >> name;
					for (auto mt : m->materials)
					{
						if (name == mt->name)
						{
							pmt = mt;
							pmt->indiceBase = currentIndex;
							break;
						}
					}
				}
				else if (token == "mtllib")
				{
					std::string mtlName;
					ss >> mtlName;

					if (mtlName != "")
					{
						std::ifstream file(m->filepath + "/" + mtlName);

						while (!file.eof())
						{
							std::string line;
							std::getline(file, line);

							std::stringstream ss(line);
							std::string token;
							ss >> token;

							if (token == "newmtl")
							{
								pmt = new Material;

								std::string mtlName;
								ss >> mtlName;
								pmt->name = mtlName;

								m->materials.push_back(pmt);
							}
							else if (token == "tk_spec")
							{
								float spec;
								ss >> spec;
								pmt->spec = 255.f * spec;
							}
							else if (token == "tk_roughness")
							{
								float roughness;
								ss >> roughness;
								pmt->roughness = 255.f * roughness;
							}
							else if (token == "map_Kd")
							{
								std::string filename;
								ss >> filename;
								auto pImage = m->getImage(filename.c_str());
								if (!pImage)
								{
									pImage = createImage(m->filepath + "/" + filename, true, true);
									if (pImage) m->pImages.push_back(pImage);
								}
								pmt->albedoAlphaMap = pImage;
							}
							else if (token == "map_bump")
							{
								std::string filename;
								ss >> filename;
								auto pImage = m->getImage(filename.c_str());
								if (!pImage)
								{
									pImage = createImage(m->filepath + "/" + filename, false, false);
									if (pImage) m->pImages.push_back(pImage);
								}
								pmt->normalHeightMap = pImage;
							}
						}
					}
				}
			}

			_model_after_process(m);
		}
	}

	namespace PMD
	{
#pragma pack(1)
		struct Header
		{
			char pmdStr[3];
			float version;
			char name[20];
			char comment[256];
		};

		struct VertexData
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
			unsigned short boneID0;
			unsigned short boneID1;
			BYTE weight;
			BYTE enableEdges;
		};

		struct MaterialData
		{
			glm::vec4 diffuse;
			float specPower;
			glm::vec3 specColor;
			glm::vec3 materialShadow;
			BYTE toonIndex;
			BYTE edgeFlag;
			int indiceCount;
			char mapName[20];
		};

		struct BoneData
		{
			char name[20];
			short parent;
			short boneID;
			char type;
			short ik;
			glm::vec3 coord;
		};

		struct IkData
		{
			short target;
			short effector;
			char chainLength;
			unsigned short iterations;
			float weight;
		};

		struct MorphHeadData
		{
			char name[20];
			unsigned int size;
			char type;
		};

		struct MorphData
		{
			unsigned int index;
			glm::vec3 offset;
		};

		struct RigidData
		{
			char name[20];
			short bone;
			char collisionGroupNumber;
			unsigned short collisionGroupMask;
			char type;
			glm::vec3 size;
			glm::vec3 location;
			glm::vec3 rotation;
			float mass;
			float velocityAttenuation;
			float rotationAttenuation;
			float bounce;
			float friction;
			char mode;
		};

		struct JointData
		{
			char name[20];
			int rigid0;
			int rigid1;
			glm::vec3 coord;
			glm::vec3 rotation;
			glm::vec3 maxCoord;
			glm::vec3 minCoord;
			glm::vec3 maxRotation;
			glm::vec3 minRotation;
			glm::vec3 springConstant;
			glm::vec3 springRotationConstant;
		};
#pragma pack()

		void load(Model *m, std::ifstream &file)
		{
			static_assert(sizeof(Header) == 283, "");
			static_assert(sizeof(VertexData) == 38, "");
			static_assert(sizeof(MaterialData) == 70, "");
			static_assert(sizeof(BoneData) == 39, "");
			static_assert(sizeof(MorphHeadData) == 25, "");
			static_assert(sizeof(MorphData) == 16, "");
			static_assert(sizeof(RigidData) == 83, "");
			static_assert(sizeof(JointData) == 124, "");

			m->animated = true;

			Header header;
			file.read((char*)&header, sizeof(Header));
			m->name = japaneseToChinese(header.name);
			m->comment = japaneseToChinese(header.comment);

			int vertexCount;
			file >> vertexCount;
			m->positions.resize(vertexCount);
			m->normals.resize(vertexCount);
			m->uvs.resize(vertexCount);
			m->boneWeights.resize(vertexCount);
			m->boneIDs.resize(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				VertexData data;
				file.read((char*)&data, sizeof(VertexData));
				m->positions[i] = data.position;
				m->positions[i].z *= -1.f;
				m->normals[i] = data.normal;
				m->normals[i].z *= -1.f;
				m->uvs[i] = data.uv;
				m->uvs[i].y = 1.f - m->uvs[i].y;
				float fWeight = data.weight / 100.f;
				m->boneWeights[i].x = fWeight;
				m->boneWeights[i].y = 1.f - fWeight;
				m->boneIDs[i].x = data.boneID0 + 0.5f;
				m->boneIDs[i].y = data.boneID1 + 0.5f;
			}

			int indiceCount;
			file >> indiceCount;
			m->indices.resize(indiceCount);
			for (int i = 0; i < indiceCount; i += 3)
			{
				unsigned short indice;
				file >> indice;
				m->indices[i + 0] = indice;
				file >> indice;
				m->indices[i + 2] = indice;
				file >> indice;
				m->indices[i + 1] = indice;
			}

			int materialCount;
			file >> materialCount;
			int currentIndiceVertex = 0;
			for (int i = 0; i < materialCount; i++)
			{
				MaterialData data;
				file.read((char*)&data, sizeof(MaterialData));

				auto pmt = new Material;
				pmt->name = std::to_string(i);

				pmt->albedoR = data.diffuse.r * 255;
				pmt->albedoG = data.diffuse.g * 255;
				pmt->albedoB = data.diffuse.b * 255;
				pmt->alpha = data.diffuse.a * 255;
				pmt->indiceBase = currentIndiceVertex;
				pmt->indiceCount = data.indiceCount;

				auto pImage = m->getImage(data.mapName);
				if (!pImage)
				{
					pImage = createImage(m->filepath + "/" + data.mapName, true, true);
					if (pImage) m->pImages.push_back(pImage);
				}
				pmt->albedoAlphaMap = pImage;


				currentIndiceVertex += data.indiceCount;

				m->materials.push_back(pmt);
			}

			unsigned short boneCount;
			file >> boneCount;
			m->bones.resize(boneCount);
			for (int i = 0; i < boneCount; i++)
			{
				BoneData data;
				file.read((char*)&data, sizeof(BoneData));

				m->bones[i].name = japaneseToChinese(data.name);
				m->bones[i].parent = data.parent;
				m->bones[i].type = data.type;
				m->bones[i].rootCoord = data.coord;
				m->bones[i].rootCoord.z *= -1.f;
			}

			unsigned short ikCount;
			file >> ikCount;
			m->iks.resize(ikCount);
			for (int i = 0; i < ikCount; i++)
			{
				IkData data;
				file.read((char*)&data, sizeof(IkData));

				m->iks[i].targetID = data.target;
				m->iks[i].effectorID = data.effector;
				m->iks[i].iterations = data.iterations;
				m->iks[i].weight = data.weight;
				m->iks[i].chain.resize(data.chainLength);
				for (int j = 0; j < data.chainLength; j++)
				{
					short boneID;
					file >> boneID;
					m->iks[i].chain[j] = boneID;
				}
			}

			unsigned short morphsCount;
			file >> morphsCount;
			for (int i = 0; i < morphsCount; i++)
			{
				MorphHeadData data;
				file.read((char*)&data, sizeof(MorphHeadData));

				for (int j = 0; j < data.size; j++)
				{
					MorphData data;
					file.read((char*)&data, sizeof(MorphData));
				}
			}

			char dispMorphsListLength;
			file >> dispMorphsListLength;
			for (int i = 0; i < dispMorphsListLength; i++)
			{
				unsigned short id;
				file >> id;
			}
			char dispBoneListLength;
			file >> dispBoneListLength;
			for (int i = 0; i < dispBoneListLength; i++)
			{
				char name[50];
				file.read(name, 50);
			}

			unsigned int dispBoneCount;
			file >> dispBoneCount;
			for (int i = 0; i < dispBoneCount; i++)
			{
				unsigned short boneIndex;
				char index;
				file >> boneIndex;
				file >> index;
			}

			char endFlag;
			file >> endFlag;
			if (endFlag)
			{
				char englishName[20];
				char englishComment[256];
				file.read(englishName, 20);
				file.read(englishComment, 256);
				for (int i = 0; i < boneCount; i++)
				{
					char name[20];
					file.read(name, 20);
				}
				for (int i = 1; i < morphsCount; i++)
				{
					char name[20];
					file.read(name, 20);
				}
				for (int i = 0; i < dispBoneListLength; i++)
				{
					char name[50];
					file.read(name, 50);
				}
			}

			for (int i = 0; i < 10; i++)
			{
				char toonTextureName[100];
				file.read(toonTextureName, 100);
			}

			unsigned int rigidCount;
			file >> rigidCount;
			for (int i = 0; i < rigidCount; i++)
			{
				RigidData data;
				file.read((char*)&data, sizeof(RigidData));

				auto p = new Rigidbody;
				p->name = japaneseToChinese(data.name);
				p->boneID = data.bone;
				p->originCollisionGroupID = data.collisionGroupNumber;
				p->originCollisionFreeFlag = data.collisionGroupMask;
				data.location.z *= -1.f;
				p->setCoord(data.location);
				data.rotation = glm::degrees(data.rotation);
				glm::mat3 rotationMat;
				eulerYxzToMatrix(glm::vec3(-data.rotation.y, -data.rotation.x, data.rotation.z), rotationMat);
				glm::vec4 rotationQuat;
				matrixToQuaternion(rotationMat, rotationQuat);
				p->setQuat(rotationQuat);
				p->type = (RigidbodyType)data.mode;
				m->addRigidbody(p);
				auto q = new Shape;
				p->addShape(q);
				switch (data.type)
				{
				case 0: q->type = ShapeTypeSphere; break;
				case 1: q->type = ShapeTypeBox; break;
				case 2: q->type = ShapeTypeCapsule; break;
				}
				switch (q->type)
				{
				case ShapeTypeSphere:
					data.size.y = data.size.z = data.size.x;
					break;
				case ShapeTypeCapsule:
					data.size.y *= 0.5f;
					data.size.z = data.size.x;
					break;
				}
				q->setScale(data.size);
				auto v = q->getVolume();
				if (v != 0.f) p->density = data.mass / v;
			}

			unsigned int jointCount;
			file >> jointCount;
			for (int i = 0; i < jointCount; i++)
			{
				JointData data;
				file.read((char*)&data, sizeof(JointData));

				auto p = new Joint;
				p->name = japaneseToChinese(data.name);
				p->rigid0ID = data.rigid0;
				p->rigid1ID = data.rigid1;
				p->maxCoord = data.maxCoord;
				p->minCoord = data.minCoord;
				p->maxRotation = data.maxRotation;
				p->minRotation = data.minRotation;
				p->springConstant = data.springConstant;
				p->sprintRotationConstant = data.springRotationConstant;

				data.coord.z *= -1.f;
				p->setCoord(data.coord);
				glm::mat3 rotationMat;
				eulerYxzToMatrix(glm::vec3(-data.rotation.y, -data.rotation.x, data.rotation.z), rotationMat);
				glm::vec4 rotationQuat;
				matrixToQuaternion(rotationMat, rotationQuat);
				p->setQuat(rotationQuat);
				m->addJoint(p);
			}

			_model_after_process(m);
		}
	}

	namespace VMD
	{
#pragma pack(1)
		struct Header
		{
			char str[30];
			char modelName[20];
		};

		struct BoneMotionData
		{
			char name[15];
			int frame;
			glm::vec3 coord;
			glm::vec4 quaternion;
			char bezier[64];
		};
#pragma pack()

		void load(Animation *a, std::ifstream &file)
		{
			static_assert(sizeof(Header) == 50, "");
			static_assert(sizeof(BoneMotionData) == 111, "");

			Header header;
			file.read((char*)&header, sizeof(Header));
			a->name = japaneseToChinese(header.modelName);
			a->comment = japaneseToChinese(header.str);

			int count;
			file >> count;
			a->motions.resize(count);
			for (int i = 0; i < count; i++)
			{
				BoneMotionData data;
				file.read((char*)&data, sizeof(BoneMotionData));
				a->motions[i].name = japaneseToChinese(data.name);
				a->motions[i].frame = data.frame;
				a->motions[i].coord = glm::vec3(data.coord);
				a->motions[i].quaternion = glm::vec4(data.quaternion);
				memcpy(a->motions[i].bezier, data.bezier, 64);

				a->motions[i].coord.z *= -1.f;
				a->motions[i].quaternion.z *= -1.f;
				a->motions[i].quaternion.w *= -1.f;
			}
		}
	}

	namespace TKM
	{
		static AnimationBinding *_loadAnimation(std::ifstream &file, Model *pModel)
		{
			std::string animName;
			file >> animName;
			for (auto a : animations)
			{
				if (a->name == animName == 0)
					return pModel->bindAnimation(a);
			}
			return nullptr;
		}

		void load(Model *m, std::ifstream &file)
		{
			int textureCount = 0;
			file >> textureCount;
			for (int i = 0; i < textureCount; i++)
			{
				std::string filename;
				file >> filename;
				bool sRGB;
				file >> sRGB;
				auto pImage = createImage(m->filepath + "/" + filename, sRGB, false);
				if (pImage) m->pImages.push_back(pImage);
			}

			file >> m->animated;

			int vertexCount;
			int indiceCount;

			file >> vertexCount;
			file >> indiceCount;
			if (vertexCount > 0)
			{
				m->positions.resize(vertexCount);
				m->uvs.resize(vertexCount);
				m->normals.resize(vertexCount);
				m->tangents.resize(vertexCount);
				file.read((char*)m->positions.data(), vertexCount * sizeof(glm::vec3));
				file.read((char*)m->uvs.data(), vertexCount * sizeof(glm::vec2));
				file.read((char*)m->normals.data(), vertexCount * sizeof(glm::vec3));
				file.read((char*)m->tangents.data(), vertexCount * sizeof(glm::vec3));
				if (m->animated)
				{
					m->boneWeights.resize(vertexCount);
					m->boneIDs.resize(vertexCount);
					file.read((char*)m->boneWeights.data(), vertexCount * sizeof(glm::vec4));
					file.read((char*)m->boneIDs.data(), vertexCount * sizeof(glm::ivec4));
				}
			}
			if (indiceCount > 0)
			{
				m->indices.reserve(indiceCount);
				file.read((char*)m->indices.data(), indiceCount * sizeof(int));
			}

			int materialCount;
			file >> materialCount;
			for (int i = 0; i < materialCount; i++)
			{
				auto pmt = new Material;

				file >> pmt->indiceBase;
				file >> pmt->indiceCount;

				file >> pmt->visible;

				file >> pmt->name;
				file >> pmt->albedoR;
				file >> pmt->albedoG;
				file >> pmt->albedoB;
				file >> pmt->alpha;
				file >> pmt->spec;
				file >> pmt->roughness;
				std::string name;
				file >> name;
				pmt->albedoAlphaMap = m->getImage(name.c_str());
				file >> name;
				pmt->normalHeightMap = m->getImage(name.c_str());
				file >> name;
				pmt->specRoughnessMap = m->getImage(name.c_str());

				m->materials.push_back(pmt);
			}

			int boneCount;
			file >> boneCount;
			for (int i = 0; i < boneCount; i++)
			{
				Bone bone;

				char name[20];
				file.read(name, 20);
				bone.name = name;

				file >> bone.type;
				file >> bone.parent;
				file >> bone.rootCoord;

				m->bones.push_back(bone);
			}

			int ikCount;
			file >> ikCount;
			m->iks.resize(boneCount);
			for (int i = 0; i < ikCount; i++)
			{
				file >> m->iks[i].targetID;
				file >> m->iks[i].effectorID;
				file >> m->iks[i].iterations;
				file >> m->iks[i].weight;

				int chainLength;
				file >> chainLength;
				m->iks[i].chain.resize(chainLength);
				file.read((char*)m->iks[i].chain.data(), sizeof(int) * chainLength);
			}

			if (m->animated)
			{
				m->animationStand = _loadAnimation(file, m);
				m->animationForward = _loadAnimation(file, m);
				m->animationLeft = _loadAnimation(file, m);
				m->animationRight = _loadAnimation(file, m);
				m->animationBackward = _loadAnimation(file, m);
				m->animationJump = _loadAnimation(file, m);
			}

			int rigidbodyCount;
			file >> rigidbodyCount;
			for (int i = 0; i < rigidbodyCount; i++)
			{
				auto p = new Rigidbody;
				int type;
				file >> type;
				p->type = (RigidbodyType)type;
				file >> p->name;
				file >> p->originCollisionGroupID;
				file >> p->originCollisionFreeFlag;
				file >> p->boneID;
				glm::vec3 coord;
				file >> coord;
				p->setCoord(coord);
				glm::vec3 euler;
				file >> euler;
				p->setEuler(euler);
				file >> p->density;
				file >> p->velocityAttenuation;
				file >> p->rotationAttenuation;
				file >> p->bounce;
				file >> p->friction;
				m->addRigidbody(p);

				int shapeCount;
				file >> shapeCount;
				for (int j = 0; j < shapeCount; j++)
				{
					auto q = new Shape;
					p->addShape(q);
					glm::vec3 coord;
					file >> coord;
					q->setCoord(coord);
					glm::vec3 euler;
					file >> euler;
					q->setEuler(euler);
					glm::vec3 scale;
					file >> scale;
					q->setScale(scale);
					int type;
					file >> type;
					q->type = (ShapeType)type;
				}
			}

			int jointCount = 0;
			file >> jointCount;
			for (int i = 0; i < jointCount; i++)
			{
				auto j = new Joint;
				glm::vec3 coord;
				file >> coord;
				j->setCoord(coord);
				glm::vec3 euler;
				file >> euler;
				j->setEuler(euler);
				file >> j->rigid0ID;
				file >> j->rigid1ID;
				file >> j->maxCoord;
				file >> j->minCoord;
				file >> j->maxRotation;
				file >> j->minRotation;
				file >> j->springConstant;
				file >> j->sprintRotationConstant;
				m->addJoint(j);
			}

			file >> m->boundingPosition;
			file >> m->boundingSize;

			file >> m->eyePosition;

			_model_after_process(m);
		}

		static void _saveAnimation(std::ofstream &file, Model *pModel, AnimationBinding *pAnim)
		{
			std::string animName;
			if (pAnim)
				animName = pAnim->pTemplate->name;
			file << animName;
		}

		void save(Model *m, const std::string &filename, bool copyTexture)
		{
			std::experimental::filesystem::path p(filename);

			std::string dstFilepath = p.parent_path().string();
			if (dstFilepath == "")
				dstFilepath = ".";

			std::ofstream file(filename);

			file << m->pImages.size();
			for (auto pImage : m->pImages)
			{
				file << pImage->filename;
				file << pImage->sRGB;
				if (copyTexture)
				{
					std::string srcFilename = m->filepath + "/" + pImage->filename;
					std::string dstFilename = dstFilepath + "/" + pImage->filename;
					CopyFile(srcFilename.c_str(), dstFilename.c_str(), false);
				}
			}

			file << m->animated;;

			int vertexCount = m->positions.size();
			int indiceCount = m->indices.size();

			file << vertexCount;
			file << indiceCount;
			if (vertexCount > 0)
			{
				file.write((char*)m->positions.data(), vertexCount * sizeof glm::vec3);
				file.write((char*)m->uvs.data(), vertexCount * sizeof glm::vec2);
				file.write((char*)m->normals.data(), vertexCount * sizeof glm::vec3);
				file.write((char*)m->tangents.data(), vertexCount * sizeof glm::vec3);
				if (m->animated)
				{
					file.write((char*)m->boneWeights.data(), vertexCount * sizeof glm::vec4);
					file.write((char*)m->boneIDs.data(), vertexCount * sizeof glm::ivec4);
				}
			}
			if (indiceCount > 0)
			{
				file.write((char*)m->indices.data(), vertexCount * sizeof(int));
			}

			file << m->materials.size();
			for (auto mt : m->materials)
			{
				file << mt->indiceBase;
				file << mt->indiceCount;

				file << mt->visible;

				file << mt->name;

				file << mt->albedoR;
				file << mt->albedoG;
				file << mt->albedoB;
				file << mt->alpha;
				file << mt->spec;
				file << mt->roughness;
				if (mt->albedoAlphaMap) file << mt->albedoAlphaMap->filename;
				else file << 0;
				if (mt->normalHeightMap) file << mt->normalHeightMap->filename;
				else file << 0;
				if (mt->specRoughnessMap) file << mt->specRoughnessMap->filename;
				else file << 0;
			}

			file << m->bones.size();
			for (auto &bone : m->bones)
			{
				file << bone.name;
				file << bone.type;
				file << bone.parent;
				file << bone.rootCoord;
			}

			file << m->iks.size();
			for (auto &ik : m->iks)
			{
				file << ik.targetID;
				file << ik.effectorID;
				file << ik.iterations;
				file << ik.weight;

				file << ik.chain.size();
				file.write((char*)ik.chain.data(), sizeof(int) * ik.chain.size());
			}

			if (m->animated)
			{
				_saveAnimation(file, m, m->animationStand);
				_saveAnimation(file, m, m->animationForward);
				_saveAnimation(file, m, m->animationLeft);
				_saveAnimation(file, m, m->animationRight);
				_saveAnimation(file, m, m->animationBackward);
				_saveAnimation(file, m, m->animationJump);
			}


			file << m->rigidbodies.size();
			for (auto rb : m->rigidbodies)
			{
				int mode = (int)rb->type;
				file << mode;
				file << rb->name;
				file << rb->originCollisionGroupID;
				file << rb->originCollisionFreeFlag;
				file << rb->boneID;
				file << rb->getCoord();
				file << rb->getEuler();
				file << rb->density;
				file << rb->velocityAttenuation;
				file << rb->rotationAttenuation;
				file << rb->bounce;
				file << rb->friction;

				file << rb->shapes.size();
				for (auto s : rb->shapes)
				{
					file << s->getCoord();
					file << s->getEuler();
					file << s->getScale();
					int type = (int)s->type;
					file << type;
				}
			}

			file << m->joints.size();
			for (auto j : m->joints)
			{
				file << j->getCoord();
				file << j->getEuler();
				file << j->rigid0ID;
				file << j->rigid1ID;
				file << j->maxCoord;
				file << j->minCoord;
				file << j->maxRotation;
				file << j->minRotation;
				file << j->springConstant;
				file << j->sprintRotationConstant;
			}

			file << m->boundingPosition;
			file << m->boundingSize;

			file << m->eyePosition;
		}
	}

	namespace TKA
	{
		void load(Animation *a, std::ifstream &file)
		{
			int count;
			file >> count;
			a->motions.resize(count);
			for (int i = 0; i < count; i++)
			{
				int nameSize;
				file >> nameSize;
				a->motions[i].name.resize(nameSize);
				file.read((char*)a->motions[i].name.data(), nameSize);
				file >> a->motions[i].frame;
				file.read((char*)&a->motions[i].coord, sizeof(glm::vec3));
				file.read((char*)&a->motions[i].quaternion, sizeof(glm::vec4));
				file.read(a->motions[i].bezier, 64);
			}
		}

		void save(Animation *a, const std::string &filename)
		{
			std::ofstream file(filename, std::ios::binary);

			file << a->motions.size();
			for (auto &motion : a->motions)
			{
				file << motion.name.size();
				file.write(motion.name.c_str(), motion.name.size());
				file << motion.frame;
				file.write((char*)&motion.coord, sizeof(glm::vec3));
				file.write((char*)&motion.quaternion, sizeof(glm::vec4));
				file.write(motion.bezier, 64);
			}
		}
	}

	Model *createModel(const std::string &filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.good())
		{
			std::cout << "Model File Lost:" << filename;
			return nullptr;
		}

		std::experimental::filesystem::path p(filename);
		auto ext = p.extension().string();
		void(*load_func)(Model *, std::ifstream &) = nullptr;
		if (ext == ".obj")
		{
			// obj need file open in text mode
			file.close();
			file.open(filename);
			load_func = &OBJ::load;
		}
		else if (ext == ".pmd")
			load_func = &PMD::load;
		else if (ext == ".tkm")
			load_func = &TKM::load;
		else
		{
			std::cout << "Model Format Not Support:" << ext;
			return nullptr;
		}

		auto m = new Model;
		m->filename = filename;
		m->filepath = p.parent_path().string();
		if (m->filepath == "")
			m->filepath = ".";
		load_func(m, file);

		_add_model(m);

		return m;
	}

	Animation *createAnimation(const std::string &filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.good())
		{
			std::cout << "Animation File Lost:" << filename;
			return nullptr;
		}

		std::experimental::filesystem::path p(filename);
		auto ext = p.extension().string();
		void(*load_func)(Animation *, std::ifstream &) = nullptr;
		if (ext == ".vmd")
			load_func = &VMD::load;
		else if (ext == ".t3a")
			load_func = &TKA::load;
		else
		{
			std::cout << "Animation Format Not Support:%s" << ext;
			return nullptr;
		}
		auto a = new Animation;
		a->filename = p.filename().string();
		a->filepath = p.parent_path().string();
		if (a->filepath == "")
			a->filepath = ".";
		load_func(a, file);

		animations.push_back(a);

		return a;
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
