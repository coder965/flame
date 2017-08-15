#include <algorithm>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <regex>
#include <memory>

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
		switch (which)
		{
		case AxisX:
		{
			auto m = glm::mat3(glm::rotate(angle, axis[0]));
			axis[1] = glm::normalize(m * axis[1]);
			axis[2] = glm::normalize(m * axis[2]);
		}
			break;
		case AxisY:
		{
			auto m = glm::mat3(glm::rotate(angle, axis[1]));
			axis[0] = glm::normalize(m * axis[0]);
			axis[2] = glm::normalize(m * axis[2]);
		}
			break;
		case AxisZ:
		{
			auto m = glm::mat3(glm::rotate(angle, axis[2]));
			axis[1] = glm::normalize(m * axis[1]);
			axis[0] = glm::normalize(m * axis[0]);
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

	bool Controller::setState(State _s, bool enable)
	{
		if (_s == State::stand)
		{
			if (state != State::stand)
			{
				state = State::stand;
				return true;
			}
			return false;
		}

		if (enable)
		{
			if (!((int)state & (int)_s))
			{
				state = State((int)state | (int)_s);
				return true;
			}
			return false;
		}
		else
		{
			if ((int)state & (int)_s)
			{
				state = State((int)state & ~(int)_s);
				return true;
			}
			return false;
		}
	}

	void Controller::reset()
	{
		state = State::stand;
		lastTime = nowTime;
	}

	bool Controller::move(float inEulerX, glm::vec3 &outCoord, glm::vec3 &outEuler)
	{
		outCoord = glm::vec3();
		outEuler = glm::vec3();

		float dist = (nowTime - lastTime) / 1000.f;
		lastTime = nowTime;

		if (state == State::stand)
			return false;

		inEulerX = glm::radians(inEulerX + ang_offset);

		bool changed = false;

		if (speed > 0.f)
		{
			if (((int)state & (int)State::forward) && !((int)state & (int)State::backward))
			{
				outCoord.x -= sin(inEulerX) * speed * dist;
				outCoord.z -= cos(inEulerX) * speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::backward) && !((int)state & (int)State::forward))
			{
				outCoord.x += sin(inEulerX) * speed * dist;
				outCoord.z += cos(inEulerX) * speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::left) && !((int)state & (int)State::right))
			{
				outCoord.x -= cos(inEulerX) * speed * dist;
				outCoord.z += sin(inEulerX) * speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::right) && !((int)state & (int)State::left))
			{
				outCoord.x += cos(inEulerX) * speed * dist;
				outCoord.z -= sin(inEulerX) * speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::up) && !((int)state & (int)State::down))
			{
				outCoord.y += speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::down) && !((int)state & (int)State::up))
			{
				outCoord.y -= speed * dist;
				changed = true;
			}
		}

		if (turn_speed > 0.f)
		{
			if (((int)state & (int)State::turn_left) && !((int)state & (int)State::turn_right))
			{
				outEuler.x += turn_speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::turn_right) && !((int)state & (int)State::turn_left))
			{
				outEuler.x -= turn_speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::turn_up) && !((int)state & (int)State::turn_down))
			{
				outEuler.z += turn_speed * dist;
				changed = true;
			}
			if (((int)state & (int)State::turn_down) && !((int)state & (int)State::turn_up))
			{
				outEuler.z -= turn_speed * dist;
				changed = true;
			}
		}

		return changed;
	}

	Camera::Camera()
	{
		ang_offset = 90.f;
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
		if (mode == CameraMode::targeting)
		{
			if (object)
			{
				target = object->getCoord() + object->model->eye_position * object->getScale();
				object = nullptr;
			}

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

		for (auto i = 0; i < 6; i++) 
			frustumPlanes[i] = glm::normalize(frustumPlanes[i]);
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
		if (mode == CameraMode::targeting)
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
		case CameraMode::free:
			addCoord(coord);
			break;
		case CameraMode::targeting:
			setTarget(target + coord);
			break;
		}
		addEuler(euler);
	}

	Light::Light(LightType _type, bool _shadow)
		:type(_type), shadow(_shadow)
	{
	}

	void Light::setColor(const glm::vec3 &v)
	{
		color = v;
		changed = true;
	}

	std::string getLightTypeName(LightType _type)
	{
		char *typeNames[] = {
			"parallax light",
			"point light",
			"spot light"
		};
		return typeNames[(int)_type];
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
		case ShapeType::box:
			return size.x * size.y * size.z * 8.f;
		case ShapeType::sphere:
			return 4.f * size.x * size.x * size.x * M_PI / 3.f;
		case ShapeType::capsule:
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

	void Rigidbody::addShape(Shape *s)
	{
		static auto magicNumber = 0;
		s->id = magicNumber++;
		shapes.push_back(std::move(std::unique_ptr<Shape>(s)));
	}

	Shape *Rigidbody::removeShape(Shape *s)
	{
		for (auto it = shapes.begin(); it != shapes.end(); it++)
		{
			if ((*it).get() == s)
			{
				if (it > shapes.begin())
					s = (*(it - 1)).get();
				else
					s = nullptr;
				shapes.erase(it);
				break;
			}
		}
		return s;
	}

	void Model::loadData(bool needRigidbody)
	{
		AttributeTree at("data", filename + ".xml");
		if (!at.good)
			return;

		if (needRigidbody)
		{
			for (auto r : rigidbodies)
				delete r;
			rigidbodies.clear();
		}

		at.obtainFromAttributes(this, b);

		for (auto &c : at.children)
		{
			if (c->name == "rigid_body")
			{
				auto r = new Rigidbody;
				rigidbodies.push_back(r);
			}
		}
	}

	void Model::saveData(bool needRigidbody)
	{
		AttributeTree at("data");

		at.addAttributes(this, b);

		if (needRigidbody)
		{
			for (auto r : rigidbodies)
			{
				auto n = new AttributeTreeNode("rigid_body");
				at.add(n);
			}
		}

		at.saveXML(filename + ".xml");
	}

	AnimationBinding *Model::bindAnimation(Animation *a)
	{
		for (auto b : animationBindings)
		{
			if (b->animation == a)
				return b;
		}

		auto binding = new AnimationBinding;
		binding->animation = a;
		for (auto &m : a->motions)
		{
			binding->frameTotal = glm::max(binding->frameTotal, m->frame);

			int boneID = -1;
			for (int iBone = 0; iBone < bones.size(); iBone++)
			{
				if (m->name == bones[iBone].name)
				{
					boneID = iBone;
					break;
				}
			}
			if (boneID == -1) continue;

			BoneMotionTrack *t = nullptr;
			for (auto &_t : binding->tracks)
			{
				if (_t->boneID == boneID)
				{
					t = _t.get();
					break;
				}
			}
			if (!t)
			{
				auto ut = std::make_unique<BoneMotionTrack>();
				t = ut.get();
				t->boneID = boneID;
				t->motions.push_back(m.get());
				binding->tracks.push_back(std::move(ut));
			}
			else
			{
				std::vector<BoneMotion*>::iterator it;
				for (it = t->motions.begin(); it != t->motions.end(); it++)
				{
					if ((*it)->frame > m->frame)
						break;
				}
				t->motions.insert(it, m.get());
			}
		}
		animationBindings.push_back(binding);
		return binding;
	}

	void Model::setStandAnimation(AnimationBinding *b)
	{
		standAnimation = b;
		stand_animation_filename = b ? b->animation->filename : "";
	}

	void Model::setForwardAnimation(AnimationBinding *b)
	{
		forwardAnimation = b;
		forward_animation_filename = b ? b->animation->filename : "";
	}

	void Model::setBackwardAnimation(AnimationBinding *b)
	{
		backwardAnimation = b;
		backward_animation_filename = b ? b->animation->filename : "";
	}

	void Model::setLeftAnimation(AnimationBinding *b)
	{
		leftAnimation = b;
		left_animation_filename = b ? b->animation->filename : "";
	}

	void Model::setRightAnimation(AnimationBinding *b)
	{
		rightAnimation = b;
		right_animation_filename = b ? b->animation->filename : "";
	}

	void Model::setJumpAnimation(AnimationBinding *b)
	{
		jumpAnimation = b;
		jump_animation_filename = b ? b->animation->filename : "";
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

	static void _add_model(Model *m)
	{
		models.push_back(std::move(std::unique_ptr<Model>(m)));
		needUpdateVertexBuffer = true;
		needUpdateMaterialBuffer = true;
		needUpdateTexture = true;
	}

	AnimationComponent::AnimationComponent(Model *_model)
	{
		model = _model;
		boneData = new BoneData[model->bones.size()];
		boneMatrix = new glm::mat4[model->bones.size()];
		for (int i = 0; i < model->bones.size(); i++)
			boneMatrix[i] = glm::mat4(1.f);
		boneMatrixBuffer = new UniformBuffer(sizeof(glm::mat4) * model->bones.size());
		boneMatrixBuffer->update(boneMatrix, stagingBuffer, sizeof(glm::mat4) * model->bones.size());
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
			if (model->bones[i].parents != -1) boneMatrix[i] = boneMatrix[model->bones[i].parents] * boneMatrix[i];
		}
	}

	void AnimationComponent::refreshBone(int i)
	{
		assert(model && i < model->bones.size());

		boneMatrix[i] = glm::translate(model->bones[i].relateCoord + boneData[i].coord) * glm::mat4(boneData[i].rotation);
		if (model->bones[i].parents != -1) boneMatrix[i] = boneMatrix[model->bones[i].parents] * boneMatrix[i];

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
			boneMatrixBuffer->update(boneMatrix, stagingBuffer, sizeof(glm::mat4) * model->bones.size());
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

			for (auto &t : currentAnimation->tracks)
			{
				auto pBoneData = &boneData[t->boneID];
				auto it = std::upper_bound(t->motions.rbegin(), t->motions.rend(), currentFrame, [](int frame, BoneMotion *bm) {
					return frame > bm->frame;
				});

				if (it == t->motions.rend()) continue;

				auto pLeftMotion = *it;
				auto pRightMotion = (it == t->motions.rbegin() ? t->motions[0] : *(it - 1));

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

		boneMatrixBuffer->update(boneMatrix, stagingBuffer, sizeof(glm::mat4) * model->bones.size());
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
			int parentID = m->bones[i].parents;
			if (parentID != -1)
			{
				m->bones[i].relateCoord -= m->bones[parentID].rootCoord;
				m->bones[parentID].children.push_back(i);
			}
		}

		Animation *a;
		a = getAnimation(m->stand_animation_filename);
		if (a) m->standAnimation = m->bindAnimation(a);
		a = getAnimation(m->forward_animation_filename);
		if (a) m->forwardAnimation = m->bindAnimation(a);
		a = getAnimation(m->backward_animation_filename);
		if (a) m->backwardAnimation = m->bindAnimation(a);
		a = getAnimation(m->left_animation_filename);
		if (a) m->leftAnimation = m->bindAnimation(a);
		a = getAnimation(m->right_animation_filename);
		if (a) m->rightAnimation = m->bindAnimation(a);
		a = getAnimation(m->jump_animation_filename);
		if (a) m->jumpAnimation = m->bindAnimation(a);
	}

	void initGeneralModels()
	{
		{
			triangleModel = new Model;
			triangleModel->name = "triangle";
			triangleModel->filename = "[triangle]";

			addTriangleVertex(triangleModel, glm::mat3(), glm::vec3());

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = triangleModel->indices.size();
			triangleModel->geometries.push_back(std::move(g));

			_model_after_process(triangleModel);

			_add_model(triangleModel);
		}

		{
			cubeModel = new Model;
			cubeModel->name = "cube";
			cubeModel->filename = "[cube]";

			addCubeVertex(cubeModel, glm::mat3(), glm::vec3(), 1.f);

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = cubeModel->indices.size();
			cubeModel->geometries.push_back(std::move(g));

			auto pRigidbody = new Rigidbody(RigidbodyType::dynamic);
			cubeModel->addRigidbody(pRigidbody);
			auto s = new Shape(ShapeType::box);
			s->setScale(glm::vec3(0.5f));
			pRigidbody->addShape(s);

			_model_after_process(cubeModel);

			_add_model(cubeModel);
		}

		{
			sphereModel = new Model;
			sphereModel->name = "sphere";
			sphereModel->filename = "[sphere]";

			addSphereVertex(sphereModel, glm::mat3(), glm::vec3(), 0.5f, 32, 32);

			auto g0 = std::make_unique<Geometry>();
			g0->material = defaultMaterial;
			g0->indiceCount = sphereModel->indices.size() / 2;
			auto g1 = std::make_unique<Geometry>();
			g1->material = defaultMaterial;
			g1->indiceBase = g0->indiceCount;
			g1->indiceCount = g0->indiceCount;
			sphereModel->geometries.push_back(std::move(g0));
			sphereModel->geometries.push_back(std::move(g1));

			auto pRigidbody = new Rigidbody(RigidbodyType::dynamic);
			sphereModel->addRigidbody(pRigidbody);
			auto s = new Shape(ShapeType::sphere);
			s->setScale(glm::vec3(0.5f));
			pRigidbody->addShape(s);

			_model_after_process(sphereModel);

			_add_model(sphereModel);
		}

		{
			cylinderModel = new Model;
			cylinderModel->name = "cylinder";
			cylinderModel->filename = "[cylinder]";

			addCylinderVertex(cylinderModel, glm::mat3(), glm::vec3(), 0.5f, 0.5f, 32);

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = cylinderModel->indices.size();
			cylinderModel->geometries.push_back(std::move(g));

			auto pRigidbody = new Rigidbody(RigidbodyType::dynamic);
			cylinderModel->addRigidbody(pRigidbody);
			auto s = new Shape(ShapeType::capsule);
			s->setScale(glm::vec3(0.5f));
			pRigidbody->addShape(s);

			_model_after_process(cylinderModel);

			_add_model(cylinderModel);
		}

		{
			coneModel = new Model;
			coneModel->name = "cone";
			coneModel->filename = "[cone]";

			addConeVertex(coneModel, glm::mat3(), glm::vec3(), 0.5f, 0.5f, 32);

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = coneModel->indices.size();
			coneModel->geometries.push_back(std::move(g));

			_model_after_process(coneModel);

			_add_model(coneModel);
		}

		{
			arrowModel = new Model;
			arrowModel->name = "arrow";
			arrowModel->filename = "[arrow]";

			glm::mat3 matR = glm::mat3(glm::rotate(-90.f, glm::vec3(0.f, 0.f, 1.f)));

			addCylinderVertex(arrowModel, matR, glm::vec3(0.4f, 0.f, 0.f), 0.4f, 0.01f, 32);
			addConeVertex(arrowModel, matR, glm::vec3(0.8f, 0.f, 0.f), 0.2f, 0.05f, 32);

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = arrowModel->indices.size();
			arrowModel->geometries.push_back(std::move(g));

			_model_after_process(arrowModel);

			_add_model(arrowModel);
		}

		{
			torusModel = new Model;
			torusModel->name = "torus";
			torusModel->filename = "[torus]";

			glm::mat3 matR = glm::mat3(glm::rotate(-90.f, glm::vec3(0.f, 0.f, 1.f)));

			addTorusVertex(torusModel, matR, glm::vec3(), 1.f, 0.01f, 32, 32);

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = torusModel->indices.size();
			torusModel->geometries.push_back(std::move(g));

			_model_after_process(torusModel);

			_add_model(torusModel);
		}

		{
			hamerModel = new Model;
			hamerModel->name = "hammer";
			hamerModel->filename = "[hammer]";

			glm::mat3 matR = glm::mat3(glm::rotate(-90.f, glm::vec3(0.f, 0.f, 1.f)));

			addCylinderVertex(hamerModel, matR, glm::vec3(0.45f, 0.f, 0.f), 0.45f, 0.01f, 32);
			int ic0 = hamerModel->indices.size();
			addCubeVertex(hamerModel, matR, glm::vec3(0.9f, 0.f, 0.f), 0.1f);
			int ic1 = hamerModel->indices.size();

			auto g0 = std::make_unique<Geometry>();
			g0->material = defaultMaterial;
			g0->indiceCount = ic0;
			auto g1 = std::make_unique<Geometry>();
			g1->material = defaultMaterial;
			g1->indiceBase = ic0;
			g1->indiceCount = ic1 - ic0;
			hamerModel->geometries.push_back(std::move(g0));
			hamerModel->geometries.push_back(std::move(g1));

			_model_after_process(hamerModel);

			_add_model(hamerModel);
		}
	}

	namespace OBJ
	{
		void load(Model *m, const std::string &filename)
		{
			std::ifstream file(filename);

			int currentIndex = 0;

			std::vector<std::pair<std::string, Material*>> materials;

			Geometry *currentGeometry = nullptr;

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

						std::regex pattern(R"(([0-9]+)?/([0-9]+)?/([0-9]+)?)");
						std::smatch match;
						std::regex_search(token, match, pattern);

						glm::ivec3 ids;
						for (int j = 0; j < 3; j++)
							ids[j] = match[j + 1].matched ? std::stoi(match[j + 1].str()) - 1 : -1;

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
							m->positions.push_back(               rawPositions[ids[0]]                 ); 
							m->uvs.push_back(      ids[1] != -1 ? rawTexcoords[ids[1]] : glm::vec2(0.f));
							m->normals.push_back(  ids[2] != -1 ? rawNormals[ids[2]]   : glm::vec3(0.f));
							rawIndexs.push_back(ids);

						}
						m->indices.push_back(index);
						currentIndex++;
						currentGeometry->indiceCount++;
					}
				}
				else if (token == "usemtl")
				{
					std::string name;
					ss >> name;
					for (auto &_m : materials)
					{
						if (name == _m.first)
						{
							auto g = std::make_unique<Geometry>();
							currentGeometry = g.get();
							currentGeometry->material = _m.second;
							currentGeometry->indiceBase = currentIndex;
							m->geometries.push_back(std::move(g));
							break;
						}
					}
				}
				else if (token == "mtllib")
				{
					std::string libName;
					ss >> libName;

					if (libName != "")
					{
						std::ifstream file(m->filepath + "/" + libName);

						std::string mtlName;
						unsigned char spec, roughness;
						Image *albedoAlphaMap = nullptr;
						Image *normalHeightMap = nullptr;

						while (!file.eof())
						{
							std::string line;
							std::getline(file, line);

							std::stringstream ss(line);
							std::string token;
							ss >> token;

							if (token == "newmtl")
							{
								ss >> mtlName;
							}
							else if (token == "tk_spec")
							{
								ss >> spec;
							}
							else if (token == "tk_roughness")
							{
								ss >> roughness;
							}
							else if (token == "map_Kd")
							{
								std::string filename;
								ss >> filename;
								albedoAlphaMap = addModelTexture(m->filepath + "/" + filename, true);
							}
							else if (token == "map_bump")
							{
								std::string filename;
								ss >> filename;
								normalHeightMap = addModelTexture(m->filepath + "/" + filename);
							}
						}

						auto _m = addModelMaterial(255, 255, 255, 255, spec, roughness, albedoAlphaMap, normalHeightMap, nullptr);
						materials.emplace_back(mtlName, _m);
					}
				}
			}

			m->loadData(true);

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
			short parents;
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

		void load(Model *m, const std::string &filename)
		{
			std::ifstream file(filename, std::ios::binary);

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

				auto g = std::make_unique<Geometry>();
				g->material = addModelMaterial(data.diffuse.r * 255, data.diffuse.g * 255, data.diffuse.b * 255, data.diffuse.a * 255,
					0, 255, addModelTexture(m->filepath + "/" + data.mapName, true), nullptr, nullptr);
				g->indiceBase = currentIndiceVertex;
				g->indiceCount = data.indiceCount;

				currentIndiceVertex += data.indiceCount;

				m->geometries.push_back(std::move(g));
			}

			unsigned short boneCount;
			file >> boneCount;
			m->bones.resize(boneCount);
			for (int i = 0; i < boneCount; i++)
			{
				BoneData data;
				file.read((char*)&data, sizeof(BoneData));

				m->bones[i].name = japaneseToChinese(data.name);
				m->bones[i].parents = data.parents;
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
				//m->addRigidbody(p); // TODO : FIX
				auto q = new Shape;
				switch (data.type)
				{
				case 0: q->type = ShapeType::sphere; break;
				case 1: q->type = ShapeType::box; break;
				case 2: q->type = ShapeType::capsule; break;
				}
				switch (q->type)
				{
				case ShapeType::sphere:
					data.size.y = data.size.z = data.size.x;
					break;
				case ShapeType::capsule:
					data.size.y *= 0.5f;
					data.size.z = data.size.x;
					break;
				}
				q->setScale(data.size);
				auto v = q->getVolume();
				if (v != 0.f) p->density = data.mass / v;
				p->addShape(q);
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

			m->loadData(false);

			_model_after_process(m);
		}
	}

	namespace COLLADA
	{
		std::string getId(const std::string &str)
		{
			assert(str.size() > 0);
			if (str[0] == '#')
				return std::string(str.c_str() + 1);
		}

		struct Source
		{
			std::string id;
			float *float_array;

			~Source()
			{
				delete[]float_array;
			}
			glm::vec2 &v2(int index)
			{
				return *(glm::vec2*)&float_array[index * 2];
			}
			glm::vec3 &v3(int index)
			{
				return *(glm::vec3*)&float_array[index * 3];
			}
		};

		struct VertexInfo
		{
			int position_source_index;
		};

		void load(Model *m, const std::string &filename)
		{
			AttributeTree at("COLLADA", filename);
			AttributeTreeNode *n;
			n = at.firstNode("library_geometries"); assert(n);
			n = n->firstNode("geometry"); assert(n);
			n = n->firstNode("mesh"); assert(n);
			std::vector<std::unique_ptr<Source>> sources;
			VertexInfo vertex_info;
			for (auto &c : n->children)
			{
				if (c->name == "source")
				{
					AttributeTreeNode *n;
					Attribute *a;
					auto s = std::make_unique<Source>();
					a = c->firstAttribute("id"); assert(a);
					s->id = a->value;
					n = c->firstNode("float_array"); assert(n);
					a = n->firstAttribute("count"); assert(a);
					auto count = std::stoi(a->value);
					s->float_array = new float[count];
					auto str = n->value;
					std::regex pattern(R"(([0-9e\.\+\-]+))");
					std::smatch match;
					int id = 0;
					while (std::regex_search(str, match, pattern) && id < count)
					{
						s->float_array[id] = std::stof(match[1].str());
						id++;
						str = match.suffix();
					}
					sources.push_back(std::move(s));
				}
				else if (c->name == "vertices")
				{
					for (auto &cc : c->children)
					{
						if (cc->name == "input")
						{
							auto a = cc->firstAttribute("semantic"); assert(a);
							if (a->value == "POSITION")
							{
								a = cc->firstAttribute("source"); assert(a);
								auto id = getId(a->value);
								for (int i = 0; i < sources.size(); i++)
								{
									if (sources[i]->id == id)
									{
										vertex_info.position_source_index = i;
										break;
									}
								}
							}
						}
					}
				}
				else if (c->name == "polylist")
				{
					int position_source_index = -1;
					int position_offset = -1;
					int uv_source_index = -1;
					int uv_offset = -1;
					int normal_source_index = -1;
					int normal_offset = -1;
					int element_count_per_vertex = 0;
					std::vector<int> vcount;
					for (auto &cc : c->children)
					{
						if (cc->name == "input")
						{
							Attribute *a;
							a = cc->firstAttribute("source"); assert(a);
							auto id = getId(a->value);
							int source_index = -1;
							for (int i = 0; i < sources.size(); i++)
							{
								if (sources[i]->id == id)
								{
									source_index = i;
									break;
								}
							}
							a = cc->firstAttribute("offset"); assert(a);
							auto offset = std::stoi(a->value);
							a = cc->firstAttribute("semantic"); assert(a);
							if (a->value == "VERTEX")
							{
								position_source_index = vertex_info.position_source_index;
								position_offset = offset;
							}
							else if (a->value == "NORMAL")
							{
								normal_source_index = source_index;
								normal_offset = offset;
							}
							element_count_per_vertex++;
						}
						else if (cc->name == "vcount")
						{
							auto str = cc->value;
							std::regex pattern(R"([0-9]+)");
							std::smatch match;
							while (std::regex_search(str, match, pattern))
							{
								auto count = std::stoi(match[0].str());
								assert(count == 3);
								vcount.push_back(count);
								str = match.suffix();
							}
						}
						else if (cc->name == "p")
						{
							auto str = cc->value;
							std::smatch match;
							assert(element_count_per_vertex > 0);
							assert(element_count_per_vertex <= 3);
							switch (element_count_per_vertex)
							{
							case 1:
							{
								std::regex pattern(R"([0-9]+)");
								auto indice_count = vcount.size() * 3;
								while (std::regex_search(str, match, pattern) && indice_count > 0)
								{
									auto index = std::stoi(match[0].str());
									m->positions.push_back(sources[position_source_index]->v3(index));
									m->uvs.push_back(glm::vec2(0.f));
									m->normals.push_back(glm::vec3(0.f));
									m->indices.push_back(index);
									indice_count--;
									str = match.suffix();
								}
							}
								break;
							case 2:
							{
								std::vector<glm::ivec2> ids;
								std::regex pattern(R"(([0-9]+)\s+([0-9]+))");
								auto indice_count = vcount.size() * 3;
								while (std::regex_search(str, match, pattern) && indice_count > 0)
								{
									glm::ivec2 id;
									id[0] = std::stoi(match[1].str());
									id[1] = std::stoi(match[2].str());
									auto index = -1;
									for (int i = 0; i < ids.size(); i++)
									{
										if (id == ids[i])
										{
											index = i;
											break;
										}
									}
									if (index == -1)
									{
										index = m->positions.size();
										m->positions.push_back(sources[position_source_index]->v3(id[position_offset]));
										m->uvs.push_back(uv_source_index == -1 ? glm::vec2(0.f) : sources[uv_source_index]->v2(id[uv_offset]));
										m->normals.push_back(normal_source_index == -1 ? glm::vec3(0.f) : sources[normal_source_index]->v3(id[normal_offset]));
										ids.push_back(id);

									}
									m->indices.push_back(index);
									indice_count--;
									str = match.suffix();
								}
							}
								break;
							case 3:
							{
								std::vector<glm::ivec3> ids;
								std::regex pattern(R"(([0-9]+)\s+([0-9]+)\s+([0-9]+))");
								auto indice_count = vcount.size() * 3;
								while (std::regex_search(str, match, pattern) && indice_count > 0)
								{
									glm::ivec3 id;
									id[0] = std::stoi(match[1].str());
									id[1] = std::stoi(match[2].str());
									id[2] = std::stoi(match[3].str());
									auto index = -1;
									for (int i = 0; i < ids.size(); i++)
									{
										if (id == ids[i])
										{
											index = i;
											break;
										}
									}
									if (index == -1)
									{
										index = m->positions.size();
										m->positions.push_back(sources[position_source_index]->v3(id[position_offset]));
										m->uvs.push_back(uv_source_index == -1 ? glm::vec2(0.f) : sources[uv_source_index]->v2(id[uv_offset]));
										m->normals.push_back(normal_source_index == -1 ? glm::vec3(0.f) : sources[normal_source_index]->v3(id[normal_offset]));
										ids.push_back(id);

									}
									m->indices.push_back(index);
									indice_count--;
									str = match.suffix();
								}
							}
								break;
							}
						}
					}
				}
			}

			auto g = std::make_unique<Geometry>();
			g->material = defaultMaterial;
			g->indiceCount = m->indices.size();
			m->geometries.push_back(std::move(g));

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

		void load(Animation *a, const std::string &filename)
		{
			std::ifstream file(filename, std::ios::binary);

			static_assert(sizeof(Header) == 50, "");
			static_assert(sizeof(BoneMotionData) == 111, "");

			Header header;
			file.read((char*)&header, sizeof(Header));
			a->name = japaneseToChinese(header.modelName);
			a->comment = japaneseToChinese(header.str);

			int count;
			file >> count;
			for (int i = 0; i < count; i++)
			{
				BoneMotionData data;
				file.read((char*)&data, sizeof(BoneMotionData));
				auto m = std::make_unique<BoneMotion>();
				m->name = japaneseToChinese(data.name);
				m->frame = data.frame;
				m->coord = glm::vec3(data.coord);
				m->quaternion = glm::vec4(data.quaternion);
				memcpy(m->bezier, data.bezier, 64);
				m->coord.z *= -1.f;
				m->quaternion.z *= -1.f;
				m->quaternion.w *= -1.f;
				a->motions.push_back(std::move(m));
			}
		}
	}

	namespace TKM
	{
		void load(Model *m, const std::string &filename)
		{
			std::ifstream file(filename, std::ios::binary);

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

			int geometryCount;
			file >> geometryCount;
			for (int i = 0; i < geometryCount; i++)
			{
				unsigned char albedoR, albedoG, albedoB, alpha, spec, roughness;
				file >> albedoR;
				file >> albedoG;
				file >> albedoB;
				file >> alpha;
				file >> spec;
				file >> roughness;
				std::string albedoAlphaMapName;
				std::string normalHeightMapName;
				std::string specRoughnessMapName;
				file >> albedoAlphaMapName;
				file >> normalHeightMapName;
				file >> specRoughnessMapName;

				auto g = std::make_unique<Geometry>();
				g->material = addModelMaterial(albedoR, albedoG, albedoB, alpha, spec, roughness, 
					addModelTexture(m->filepath + "/" + albedoAlphaMapName, true),
					addModelTexture(m->filepath + "/" + normalHeightMapName, true),
					addModelTexture(m->filepath + "/" + specRoughnessMapName, true));
				file >> g->indiceBase;
				file >> g->indiceCount;
				file >> g->visible;

				m->geometries.push_back(std::move(g));
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
				file >> bone.parents;
				file & bone.rootCoord;

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
				file >> m->stand_animation_filename;
				file >> m->forward_animation_filename;
				file >> m->backward_animation_filename;
				file >> m->left_animation_filename;
				file >> m->right_animation_filename;
				file >> m->jump_animation_filename;
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
				file & coord;
				p->setCoord(coord);
				glm::vec3 euler;
				file & euler;
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
					file & coord;
					q->setCoord(coord);
					glm::vec3 euler;
					file & euler;
					q->setEuler(euler);
					glm::vec3 scale;
					file & scale;
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
				file & coord;
				j->setCoord(coord);
				glm::vec3 euler;
				file & euler;
				j->setEuler(euler);
				file >> j->rigid0ID;
				file >> j->rigid1ID;
				file & j->maxCoord;
				file & j->minCoord;
				file & j->maxRotation;
				file & j->minRotation;
				file & j->springConstant;
				file & j->sprintRotationConstant;
				m->addJoint(j);
			}

			file & m->bounding_position;
			file & m->bounding_size;

			file & m->controller_height;
			file & m->controller_radius;

			file & m->eye_position;

			_model_after_process(m);
		}

		void save(Model *m, const std::string &filename, bool copyTexture)
		{
			std::experimental::filesystem::path path(filename);

			std::string dstFilepath = path.parent_path().string();
			if (dstFilepath == "")
				dstFilepath = ".";

			std::ofstream file(filename);

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

			file << m->geometries.size();
			for (auto &g : m->geometries)
			{
				file << g->material->albedoR;
				file << g->material->albedoG;
				file << g->material->albedoB;
				file << g->material->alpha;
				file << g->material->spec;
				file << g->material->roughness;
				file << g->material->albedoAlphaMap ? g->material->albedoAlphaMap->filename : 0;
				file << g->material->normalHeightMap ? g->material->normalHeightMap->filename : 0;
				file << g->material->specRoughnessMap ? g->material->specRoughnessMap->filename : 0;

				file << g->indiceBase;
				file << g->indiceCount;
				file << g->visible;
				if (copyTexture)
				{
					if (g->material->albedoAlphaMap)
					{
						std::string dst = dstFilepath + "/" + g->material->albedoAlphaMap->filename;
						CopyFile(g->material->albedoAlphaMap->full_filename.c_str(), dst.c_str(), false);
					}
					if (g->material->normalHeightMap)
					{
						std::string dst = dstFilepath + "/" + g->material->normalHeightMap->filename;
						CopyFile(g->material->normalHeightMap->full_filename.c_str(), dst.c_str(), false);
					}
					if (g->material->specRoughnessMap)
					{
						std::string dst = dstFilepath + "/" + g->material->specRoughnessMap->filename;
						CopyFile(g->material->specRoughnessMap->full_filename.c_str(), dst.c_str(), false);
					}
				}
			}

			file << m->bones.size();
			for (auto &bone : m->bones)
			{
				file << bone.name;
				file << bone.type;
				file << bone.parents;
				file & bone.rootCoord;
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
				file << m->stand_animation_filename;
				file << m->forward_animation_filename;
				file << m->backward_animation_filename;
				file << m->left_animation_filename;
				file << m->right_animation_filename;
				file << m->jump_animation_filename;
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
				file & rb->getCoord();
				file & rb->getEuler();
				file << rb->density;
				file << rb->velocityAttenuation;
				file << rb->rotationAttenuation;
				file << rb->bounce;
				file << rb->friction;

				file << rb->shapes.size();
				for (auto &s : rb->shapes)
				{
					file & s->getCoord();
					file & s->getEuler();
					file & s->getScale();
					int type = (int)s->type;
					file << type;
				}
			}

			file << m->joints.size();
			for (auto j : m->joints)
			{
				file & j->getCoord();
				file & j->getEuler();
				file << j->rigid0ID;
				file << j->rigid1ID;
				file & j->maxCoord;
				file & j->minCoord;
				file & j->maxRotation;
				file & j->minRotation;
				file & j->springConstant;
				file & j->sprintRotationConstant;
			}

			file & m->bounding_position;
			file & m->bounding_size;

			file & m->controller_height;
			file & m->controller_radius;

			file & m->eye_position;
		}
	}

	namespace TKA
	{
		void load(Animation *a, const std::string &filename)
		{
			std::ifstream file(filename, std::ios::binary);

			int count;
			file >> count;
			for (int i = 0; i < count; i++)
			{
				auto m = std::make_unique<BoneMotion>();
				file > m->name;
				file >> m->frame;
				file & m->coord;
				file & m->quaternion;
				file.read(m->bezier, 64);
			}
		}

		void save(Animation *a, const std::string &filename)
		{
			std::ofstream file(filename, std::ios::binary);

			file << a->motions.size();
			for (auto &m : a->motions)
			{
				file < m->name;
				file << m->frame;
				file & m->coord;
				file & m->quaternion;
				file.write(m->bezier, 64);
			}
		}
	}

	Model *createModel(const std::string &_filename)
	{
		auto filename = std::experimental::filesystem::path(_filename).string();
		if (!std::experimental::filesystem::exists(filename))
		{
			std::cout << "Model File Lost:" << filename;
			return nullptr;
		}

		std::experimental::filesystem::path path(filename);
		auto ext = path.extension().string();
		void(*load_func)(Model *, const std::string &) = nullptr;
		if (ext == ".obj")
			load_func = &OBJ::load;
		else if (ext == ".pmd")
			load_func = &PMD::load;
		else if (ext == ".dae")
			load_func = &COLLADA::load;
		else if (ext == ".tkm")
			load_func = &TKM::load;
		else
		{
			std::cout << "Model Format Not Support:" << ext;
			return nullptr;
		}

		auto m = new Model;
		m->filename = filename;
		m->filepath = path.parent_path().string();
		if (m->filepath == "")
			m->filepath = ".";
		load_func(m, filename);

		_add_model(m);

		return m;
	}

	Animation *createAnimation(const std::string &_filename)
	{
		auto filename = std::experimental::filesystem::path(_filename).string();
		if (!std::experimental::filesystem::exists(filename))
		{
			std::cout << "Animation File Lost:" << filename;
			return nullptr;
		}

		std::experimental::filesystem::path path(filename);
		auto ext = path.extension().string();
		void(*load_func)(Animation *, const std::string &) = nullptr;
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
		a->filename = path.filename().string();
		a->filepath = path.parent_path().string();
		if (a->filepath == "")
			a->filepath = ".";
		load_func(a, filename);

		animations.push_back(std::move(std::unique_ptr<Animation>(a)));

		return a;
	}

	RigidBodyData::~RigidBodyData()
	{
		actor->release();
	}

	Object::Object() {}

	Object::Object(Model *_model, ObjectPhysicsType _physicsType)
		:model(_model), physics_type(_physicsType)
	{
		model_filename = model->filename;
		if (model->animated)
			animationComponent = std::make_unique<AnimationComponent>(model);
	}

	Object::~Object()
	{
		if (pxController)
			pxController->release();
	}

	void Object::setState(Controller::State _s, bool enable)
	{
		if (Controller::setState(_s, enable))
		{
			if (animationComponent)
			{
				if (state == Controller::State::stand)
					animationComponent->setAnimation(model->standAnimation);
				else if (state == Controller::State::forward)
					animationComponent->setAnimation(model->forwardAnimation);
			}
		}
	}

	Terrain::Terrain(bool _use_physx, Image *_heightMap, Image *_blendMap, 
		Image *_colorMap0, Image *_colorMap1, Image *_colorMap2, Image *_colorMap3,
		Image *_normalMap0, Image *_normalMap1, Image *_normalMap2, Image *_normalMap3)
		:use_physx(_use_physx), blendMap(_blendMap), heightMap(_heightMap)
	{
		colorMaps[0] = _colorMap0;
		colorMaps[1] = _colorMap1;
		colorMaps[2] = _colorMap2;
		colorMaps[3] = _colorMap3;
		normalMaps[0] = _normalMap0;
		normalMaps[1] = _normalMap1;
		normalMaps[2] = _normalMap2;
		normalMaps[3] = _normalMap3;
		if (heightMap)
			height_map_filename = heightMap->filename;
		if (_colorMap0)
			color_map0_filename = _colorMap0->filename;
		if (_colorMap1)
			color_map1_filename = _colorMap1->filename;
		if (_colorMap2)
			color_map2_filename = _colorMap2->filename;
		if (_colorMap3)
			color_map3_filename = _colorMap2->filename;
		if (_normalMap0)
			normal_map0_filename = _normalMap0->filename;
		if (_normalMap1)
			normal_map1_filename = _normalMap1->filename;
		if (_normalMap2)
			normal_map2_filename = _normalMap2->filename;
		if (_normalMap3)
			normal_map3_filename = _normalMap3->filename;
	}

	static const float gravity = 9.81f;

	static Image *envrImageDownsample[3] = {};

	static RenderPass *sceneRenderPass = nullptr;

	Pipeline *scatteringPipeline = nullptr;

	Pipeline *downsamplePipeline = nullptr;

	Pipeline *convolvePipeline = nullptr;

	Pipeline *mrtPipeline;

	Pipeline *mrtAnimPipeline;
	static int mrt_bone_position = -1;

	Pipeline *terrainPipeline = nullptr;
	static int terr_heightMap_position = -1;
	static int terr_blendMap_position = -1;
	static int terr_colorMap_position = -1;
	static int terr_normalMap_position = -1;

	Pipeline *waterPipeline = nullptr;

	Pipeline *deferredPipeline = nullptr;
	static int defe_envr_position = -1;
	static int defe_shad_position = -1;

	Pipeline *esmPipeline = nullptr;

	Pipeline *esmAnimPipeline = nullptr;

	Pipeline *composePipeline = nullptr;

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

	static void _setSunLight_attribute(Scene *s)
	{
		s->sunLight->setEuler(glm::vec3(s->sunDir.x, 0.f, s->sunDir.y));
	}

	Scene::Scene()
		:resource(&globalResource)
	{
		physx::PxSceneDesc pxSceneDesc(pxPhysics->getTolerancesScale());
		pxSceneDesc.gravity = physx::PxVec3(0.0f, -gravity, 0.0f);
		pxSceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		pxSceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		pxScene = pxPhysics->createScene(pxSceneDesc);
		pxControllerManager = PxCreateControllerManager(*pxScene);

		envrImage = std::make_unique<Image>(TKE_ENVR_SIZE_CX, TKE_ENVR_SIZE_CY, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 4);
		mainImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		depthImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		albedoAlphaImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		normalHeightImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		specRoughnessImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		esmImage = std::make_unique<Image>(TKE_SHADOWMAP_CX, TKE_SHADOWMAP_CX, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, TKE_MAX_SHADOW_COUNT * 6);
		debugImages.emplace_back("Esm Image", esmImage.get());
		esmDepthImage = std::make_unique<Image>(TKE_SHADOWMAP_CX, TKE_SHADOWMAP_CX, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		resource.setImage(envrImage.get(), "Envr.Image");
		resource.setImage(mainImage.get(), "Main.Image");
		resource.setImage(depthImage.get(), "Depth.Image");
		resource.setImage(albedoAlphaImage.get(), "AlbedoAlpha.Image");
		resource.setImage(normalHeightImage.get(), "NormalHeight.Image");
		resource.setImage(specRoughnessImage.get(), "SpecRoughness.Image");

		matrixBuffer = std::make_unique<UniformBuffer>(sizeof MatrixBufferShaderStruct);
		staticObjectMatrixBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * TKE_MAX_STATIC_OBJECT_COUNT);
		animatedObjectMatrixBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * TKE_MAX_ANIMATED_OBJECT_COUNT);
		terrainBuffer = std::make_unique<UniformBuffer>(sizeof TerrainShaderStruct);
		waterBuffer = std::make_unique<UniformBuffer>(sizeof(WaterShaderStruct) * TKE_MAX_WATER_COUNT);
		lightBuffer = std::make_unique<UniformBuffer>(sizeof(LightBufferShaderStruct));
		ambientBuffer = std::make_unique<UniformBuffer>(sizeof AmbientBufferShaderStruct);
		shadowBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * TKE_MAX_SHADOW_COUNT);
		staticObjectIndirectBuffer = std::make_unique<IndirectIndexBuffer>(sizeof(VkDrawIndexedIndirectCommand) * TKE_MAX_INDIRECT_COUNT);
		animatedObjectIndirectBuffer = std::make_unique<IndirectIndexBuffer>(sizeof(VkDrawIndexedIndirectCommand) * TKE_MAX_INDIRECT_COUNT);

		resource.setBuffer(matrixBuffer.get(), "Matrix.UniformBuffer");
		resource.setBuffer(staticObjectMatrixBuffer.get(), "StaticObjectMatrix.UniformBuffer");
		resource.setBuffer(animatedObjectMatrixBuffer.get(), "AnimatedObjectMatrix.UniformBuffer");
		resource.setBuffer(terrainBuffer.get(), "Terrain.UniformBuffer");
		resource.setBuffer(waterBuffer.get(), "Water.UniformBuffer");
		resource.setBuffer(lightBuffer.get(), "Light.UniformBuffer");
		resource.setBuffer(ambientBuffer.get(), "Ambient.UniformBuffer");
		resource.setBuffer(shadowBuffer.get(), "Shadow.UniformBuffer");
		resource.setBuffer(staticObjectIndirectBuffer.get(), "Scene.Static.IndirectBuffer");
		resource.setBuffer(animatedObjectIndirectBuffer.get(), "Scene.Animated.IndirectBuffer");

		ds_mrt = std::make_unique<DescriptorSet>(descriptorPool, mrtPipeline);
		mrtPipeline->linkDescriptors(ds_mrt.get(), &resource);
		ds_mrtAnim = std::make_unique<DescriptorSet>(descriptorPool, mrtAnimPipeline);
		mrtAnimPipeline->linkDescriptors(ds_mrtAnim.get(), &resource);
		ds_mrtAnim_bone = std::make_unique<DescriptorSet>(descriptorPool, mrtAnimPipeline, 2);
		ds_terrain = std::make_unique<DescriptorSet>(descriptorPool, terrainPipeline);
		terrainPipeline->linkDescriptors(ds_terrain.get(), &resource);
		ds_water = std::make_unique<DescriptorSet>(descriptorPool, waterPipeline);
		waterPipeline->linkDescriptors(ds_water.get(), &resource);
		ds_esm = std::make_unique<DescriptorSet>(descriptorPool, esmPipeline);
		esmPipeline->linkDescriptors(ds_esm.get(), &resource);
		ds_esmAnim = std::make_unique<DescriptorSet>(descriptorPool, esmAnimPipeline);
		esmAnimPipeline->linkDescriptors(ds_esmAnim.get(), &resource);
		ds_defe = std::make_unique<DescriptorSet>(descriptorPool, deferredPipeline);
		deferredPipeline->linkDescriptors(ds_defe.get(), &resource);
		ds_comp = std::make_unique<DescriptorSet>(descriptorPool, composePipeline);
		composePipeline->linkDescriptors(ds_comp.get(), &resource);

		cb_shadow = std::make_unique<CommandBuffer>(commandPool);
		cb_mrt = std::make_unique<CommandBuffer>(commandPool);
		cb_deferred = std::make_unique<CommandBuffer>(commandPool);

		for (int i = 0; i < TKE_MAX_SHADOW_COUNT * 6; i++)
		{
			VkImageView views[] = {
				esmImage->getView(0, 0, 1, i),
				esmDepthImage->getView()
			};
			fb_esm[i] = std::move(std::unique_ptr<Framebuffer>(getFramebuffer(TKE_SHADOWMAP_CX, TKE_SHADOWMAP_CY, renderPass_depth_clear_image32f_clear, TK_ARRAYSIZE(views), views)));
		}

		shadowRenderFinished = createEvent();
		mrtRenderFinished = createEvent();

		sunLight = new Light(LightType::parallax);
		sunLight->shadow = true;
		_setSunLight_attribute(this);
		addLight(sunLight);
	}

	Scene::~Scene()
	{
		pxControllerManager->release();
		pxScene->release();

		destroyEvent(shadowRenderFinished);
		destroyEvent(mrtRenderFinished);
	}

	void Scene::addLight(Light *l) // when a light is added to scene, the owner is the scene, light cannot be deleted elsewhere
	{
		mtx.lock();
		lights.push_back(std::move(std::unique_ptr<Light>(l)));
		needUpdateLightCount = true;
		mtx.unlock();
	}

	Light *Scene::removeLight(Light *l)
	{
		mtx.lock();
		for (auto it = lights.begin(); it != lights.end(); it++)
		{
			if (it->get() == l)
			{
				for (auto itt = it + 1; itt != lights.end(); itt++)
				{
					(*itt)->sceneIndex--;
					(*itt)->changed = true;
				}
				delete l;
				it = lights.erase(it);
				l = it == lights.end() ? nullptr : it->get();
				break;
			}
		}
		needUpdateLightCount = true;
		mtx.unlock();
		return l;
	}

	static int _objectMagicIndex = 0;

	void Scene::addObject(Object *o) // when a object is added to scene, the owner is the scene, object cannot be deleted elsewhere
									 // and, if object has physics componet, it can be only moved by physics
	{
		auto m = o->model;
		if (!m)
		{
			delete o;
			return;
		}

		if (o->name == "")
		{
			o->name = std::to_string(_objectMagicIndex);
			_objectMagicIndex++;
		}

		mtx.lock();

		// since object can move to somewhere first, we create physics component here
		if (((int)o->physics_type & (int)ObjectPhysicsType::static_r) || ((int)o->physics_type & (int)ObjectPhysicsType::dynamic))
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
					auto actor = (((int)o->physics_type & (int)ObjectPhysicsType::dynamic) && (r->type == RigidbodyType::dynamic || r->type == RigidbodyType::dynamic_but_location)) ?
						createDynamicRigidActor(rigTrans, false, r->density) : createStaticRigidActor(rigTrans);

					for (auto &s : r->shapes)
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
						case ShapeType::box:
							actor->createShape(physx::PxBoxGeometry(scale[0], scale[1], scale[2]), *pxDefaultMaterial, trans);
							break;
						case ShapeType::sphere:
							actor->createShape(physx::PxSphereGeometry(scale[0]), *pxDefaultMaterial, trans);
							break;
						case ShapeType::capsule:
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

		if ((int)o->physics_type & (int)ObjectPhysicsType::controller)
		{
			auto c = m->controller_position * o->getScale() + o->getCoord();
			physx::PxCapsuleControllerDesc capsuleDesc;
			capsuleDesc.radius = (m->controller_radius * o->getScale().x) / 0.8f;
			capsuleDesc.height = (m->controller_height * o->getScale().y) / 0.8f;
			capsuleDesc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
			capsuleDesc.material = pxDefaultMaterial;
			capsuleDesc.position.x = c.x;
			capsuleDesc.position.y = c.y;
			capsuleDesc.position.z = c.z;
			capsuleDesc.stepOffset = capsuleDesc.radius;

			o->pxController = pxControllerManager->createController(capsuleDesc);
		}

		objects.push_back(std::move(std::unique_ptr<Object>(o)));

		needUpdateIndirectBuffer = true;
		mtx.unlock();
	}

	Object *Scene::removeObject(Object *o)
	{
		mtx.lock();
		for (auto it = objects.begin(); it != objects.end(); it++)
		{
			if (it->get() == o)
			{
				for (auto itt = it + 1; itt != objects.end(); itt++)
				{
					(*itt)->sceneIndex--;
					(*itt)->changed = true;
				}
				for (auto &r : o->rigidbodyDatas)
				{
					if (r.actor)
						r.actor->release();
				}
				if (o->pxController)
					o->pxController->release();
				delete o;
				it = objects.erase(it);
				o = it == objects.end() ? nullptr : it->get();
				break;
			}
		}
		needUpdateIndirectBuffer = true;
		mtx.unlock();
		return o;
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

	void Scene::addTerrain(Terrain *t) // when a terrain is added to scene, the owner is the scene, terrain cannot be deleted elsewhere
	{
		mtx.lock();

		if (t->use_physx && t->heightMap)
		{
			auto m = t->heightMap;

			auto numVerts = m->cx * m->cy;

			auto samples = new physx::PxHeightFieldSample[numVerts];
			memset(samples, 0, numVerts * sizeof(physx::PxHeightFieldSample));

			for (int y = 0; y < m->cy; y++)
			{
				for (int x = 0; x < m->cx; x++)
					samples[y + x * m->cx].height = m->getR(x - 0.5, y - 0.5);
			}

			physx::PxHeightFieldDesc hfDesc;
			hfDesc.format = physx::PxHeightFieldFormat::eS16_TM;
			hfDesc.nbRows = m->cx;
			hfDesc.nbColumns = m->cy;
			hfDesc.samples.data = samples;
			hfDesc.samples.stride = sizeof(physx::PxHeightFieldSample);

			physx::PxHeightFieldGeometry hfGeom(pxPhysics->createHeightField(hfDesc), physx::PxMeshGeometryFlags(), t->height / 255.f, t->block_size * t->block_cx / m->cx, t->block_size * t->block_cx / m->cy);
			t->actor = pxPhysics->createRigidStatic(physx::PxTransform(physx::PxIdentity));
			t->actor->createShape(hfGeom, *pxDefaultMaterial);

			pxScene->addActor(*t->actor);

			delete[]samples;
		}

		terrain = std::unique_ptr<Terrain>(t);

		mtx.unlock();
	}

	void Scene::removeTerrain()
	{
		mtx.lock();

		if (terrain->actor)
			terrain->actor->release();

		terrain.reset();

		mtx.unlock();
	}

	void Scene::addWater(Water *w)
	{
		mtx.lock();
		waters.push_back(std::move(std::unique_ptr<Water>(w)));
		mtx.unlock();
	}

	Water *Scene::removeWater(Water *w)
	{
		mtx.lock();
		for (auto it = waters.begin(); it != waters.end(); it++)
		{
			if (it->get() == w)
			{
				for (auto itt = it + 1; itt != waters.end(); itt++)
				{
					//(*itt)->sceneIndex--;
					(*itt)->changed = true;
				}
				delete w;
				it = waters.erase(it);
				w = it == waters.end() ? nullptr : it->get();
				break;
			}
		}
		mtx.unlock();
		return w;
	}

	void Scene::clear()
	{
		mtx.lock();

		sunLight = nullptr;
		lights.clear();
		objects.clear();
		terrain.reset();

		mtx.unlock();
	}

	void Scene::setSunDir(const glm::vec2 &v)
	{
		sunDir = v;
		needUpdateSky = true;
	}

	void Scene::setAmbientColor(const glm::vec3 &v)
	{
		ambientColor = v;
		needUpdateAmbientBuffer = true;
	}

	void Scene::setFogColor(const glm::vec3 &v)
	{
		fogColor = v;
		needUpdateAmbientBuffer = true;
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

	void Scene::show(Framebuffer *fb, VkEvent signalEvent)
	{
		// update animation and bones
		for (auto &o : objects)
		{
			if (o->animationComponent)
				o->animationComponent->update();
		}

		// update physics (controller should move first, then simulate, and then get the result coord)
		auto dist = (timeDisp) / 1000.f;
		if (dist > 0.f)
		{
			for (auto &o : objects) // set controller coord
			{
				if ((int)o->physics_type & (int)ObjectPhysicsType::controller)
				{
					glm::vec3 e, c;
					o->move(o->getEuler().x, c, e);
					o->addEuler(e);

					physx::PxVec3 disp(c.x, -gravity * o->floatingTime * o->floatingTime, c.z);
					o->floatingTime += dist;

					if (o->pxController->move(disp, 0.f, dist, nullptr) & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
						o->floatingTime = 0.f;
				}
			}
			pxScene->simulate(dist);
			//pxScene->simulate(1.f / 60.f);
			pxScene->fetchResults(true);
			for (auto &o : objects)
			{
				if ((int)o->physics_type & (int)ObjectPhysicsType::dynamic)
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

				if ((int)o->physics_type & (int)ObjectPhysicsType::controller)
				{
					auto p = o->pxController->getPosition();
					auto c = glm::vec3(p.x, p.y, p.z) - o->model->controller_position * o->getScale();
					o->setCoord(c);
				}
			}
		}

		camera.move();
		if (camera.changed || camera.object)
			camera.lookAtTarget();
		if (camera.changed)
			camera.updateFrustum();
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
			matrixBuffer->update(&stru, stagingBuffer);
		}
		if (needUpdateSky)
		{
			needUpdateAmbientBuffer = true;

			switch (skyType)
			{
			case SkyType::atmosphere_scattering:
				{ // update Atmospheric Scattering
					_setSunLight_attribute(this);

					{
						auto cb = commandPool->begineOnce();
						auto fb = getFramebuffer(envrImage.get(), renderPass_image16);

						cb->beginRenderPass(renderPass_image16, fb);
						cb->bindPipeline(scatteringPipeline);
						auto dir = sunLight->getAxis()[2];
						cb->pushConstant(tke::StageType::frag, 0, sizeof(dir), &dir);
						cb->draw(3);
						cb->endRenderPass();

						commandPool->endOnce(cb);
						releaseFramebuffer(fb);
					}

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
									auto fb = getFramebuffer(envrImageDownsample[i], renderPass_image16);

									cb->beginRenderPass(renderPass_image16, fb);
									cb->bindPipeline(downsamplePipeline);
									cb->setViewportAndScissor(TKE_ENVR_SIZE_CX >> (i + 1), TKE_ENVR_SIZE_CY >> (i + 1));
									auto size = glm::vec2(TKE_ENVR_SIZE_CX >> (i + 1), TKE_ENVR_SIZE_CY >> (i + 1));
									cb->pushConstant(StageType::frag, 0, sizeof glm::vec2, &size);
									downsamplePipeline->descriptorSet->setImage(down_source_position, 0, i == 0 ? envrImage.get() : envrImageDownsample[i - 1], plainSampler);
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
									auto fb = getFramebuffer(envrImage.get(), renderPass_image16, i);

									cb->beginRenderPass(renderPass_image16, fb);
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
						}
					}
				}
				break;
			case SkyType::panorama:
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
				break;
			}

			needUpdateSky = false;
		}
		if (needUpdateAmbientBuffer)
		{
			AmbientBufferShaderStruct stru;
			stru.color = ambientColor;
			stru.envr_max_mipmap = envrImage->level - 1;
			stru.fogcolor = glm::vec4(fogColor, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
			ambientBuffer->update(&stru, stagingBuffer);

			needUpdateAmbientBuffer = false;
		}
		if (objects.size() > 0)
		{
			int updateCount = 0;
			std::vector<VkBufferCopy> staticUpdateRanges;
			std::vector<VkBufferCopy> animatedUpdateRanges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(glm::mat4) * objects.size());
			int staticObjectIndex = 0;
			int animatedObjectIndex = 0;

			for (auto &o : objects)
			{
				if (!o->model->animated)
				{
					if (o->changed)
					{
						auto srcOffset = sizeof(glm::mat4) * updateCount;
						memcpy(map + srcOffset, &o->getMat(), sizeof(glm::mat4));
						VkBufferCopy range = {};
						range.srcOffset = srcOffset;
						range.dstOffset = sizeof(glm::mat4) * staticObjectIndex;
						range.size = sizeof(glm::mat4);
						staticUpdateRanges.push_back(range);

						updateCount++;
					}
					o->sceneIndex = staticObjectIndex;
					staticObjectIndex++;
				}
				else
				{
					if (o->changed)
					{
						auto srcOffset = sizeof(glm::mat4) * updateCount;
						memcpy(map + srcOffset, &o->getMat(), sizeof(glm::mat4));
						VkBufferCopy range = {};
						range.srcOffset = srcOffset;
						range.dstOffset = sizeof(glm::mat4) * animatedObjectIndex;
						range.size = sizeof(glm::mat4);
						animatedUpdateRanges.push_back(range);

						updateCount++;
					}
					o->sceneIndex = animatedObjectIndex;
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
				TerrainShaderStruct stru;
				stru.coord = terrain->getCoord();
				stru.blockCx = terrain->block_cx;
				stru.blockSize = terrain->block_size;
				stru.height = terrain->height;
				stru.tessellationFactor = terrain->tessellation_factor;
				stru.textureUvFactor = terrain->texture_uv_factor;
				stru.mapDimension = terrain->heightMap->cx;

				terrainBuffer->update(&stru, stagingBuffer);

				if (terr_heightMap_position != -1 && terrain->heightMap)
					ds_terrain->setImage(terr_heightMap_position, 0, terrain->heightMap, colorBorderSampler);
				if (terr_blendMap_position != -1 && terrain->blendMap)
					ds_terrain->setImage(terr_blendMap_position, 0, terrain->blendMap, colorBorderSampler);
				if (terr_colorMap_position != -1)
				{
					for (int i = 0; i < 4; i++)
					{
						if (terrain->colorMaps[i])
							ds_terrain->setImage(terr_colorMap_position, i, terrain->colorMaps[i], colorWrapSampler);
					}
				}
				if (terr_normalMap_position != -1)
				{
					for (int i = 0; i < 4; i++)
					{
						if (terrain->normalMaps[i])
							ds_terrain->setImage(terr_normalMap_position, i, terrain->normalMaps[i], colorWrapSampler);
					}
				}
			}
		}
		if (waters.size() > 0)
		{
			int updateCount = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(WaterShaderStruct) * waters.size());

			for (auto &w : waters)
			{
				if (w->changed)
				{
					auto offset = sizeof(WaterShaderStruct) * updateCount;
					WaterShaderStruct stru;
					stru.coord = w->getCoord();
					stru.blockCx = w->blockCx;
					stru.blockSize = w->blockSize;
					stru.height = w->height;
					stru.tessellationFactor = w->tessellationFactor;
					stru.textureUvFactor = w->textureUvFactor;
					stru.mapDimension = 1024;
					memcpy(map + offset, &stru, sizeof(WaterShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = offset;
					range.dstOffset = offset;
					range.size = sizeof(WaterShaderStruct);
					ranges.push_back(range);

					updateCount++;
				}
			}
			stagingBuffer->unmap();
			if (ranges.size() > 0) commandPool->copyBuffer(stagingBuffer->v, waterBuffer->v, ranges.size(), ranges.data());
		}
		static std::vector<Object*> staticObjects;
		static std::vector<Object*> animatedObjects;
		if (needUpdateIndirectBuffer)
		{
			staticObjects.clear();
			animatedObjects.clear();

			if (objects.size() > 0)
			{
				std::vector<VkDrawIndexedIndirectCommand> staticCommands;
				std::vector<VkDrawIndexedIndirectCommand> animatedCommands;

				int staticIndex = 0;
				int animatedIndex = 0;

				for (auto &o : objects)
				{
					auto m = o->model;

					if (!m->animated)
					{
						for (auto &g : m->geometries)
						{
							VkDrawIndexedIndirectCommand command = {};
							command.instanceCount = 1;
							command.indexCount = g->indiceCount;
							command.vertexOffset = m->vertexBase;
							command.firstIndex = m->indiceBase + g->indiceBase;
							command.firstInstance = (staticIndex << 8) + g->material->sceneIndex;

							staticCommands.push_back(command);
						}

						staticObjects.push_back(o.get());
						staticIndex++;
					}
					else
					{
						for (auto &g : m->geometries)
						{
							VkDrawIndexedIndirectCommand command = {};
							command.instanceCount = 1;
							command.indexCount = g->indiceCount;
							command.vertexOffset = m->vertexBase;
							command.firstIndex = m->indiceBase + g->indiceBase;
							command.firstInstance = (animatedIndex << 8) + g->material->sceneIndex;

							animatedCommands.push_back(command);
						}

						if (mrt_bone_position != -1)
							ds_mrtAnim_bone->setBuffer(mrt_bone_position, animatedIndex, o->animationComponent->boneMatrixBuffer);

						animatedObjects.push_back(o.get());
						animatedIndex++;
					}
				}

				staticIndirectCount = staticCommands.size();
				animatedIndirectCount = animatedCommands.size();

				if (staticCommands.size() > 0) staticObjectIndirectBuffer->update(staticCommands.data(), stagingBuffer, sizeof(VkDrawIndexedIndirectCommand) * staticCommands.size());
				if (animatedCommands.size() > 0) animatedObjectIndirectBuffer->update(animatedCommands.data(), stagingBuffer, sizeof(VkDrawIndexedIndirectCommand) * animatedCommands.size());
			}
			needUpdateIndirectBuffer = false;
		}
		if (needUpdateLightCount)
		{ // light count in light attribute
			auto count = lights.size();
			lightBuffer->update(&count, stagingBuffer, 4);
			needUpdateLightCount = false;
		}
		std::vector<Light*> shadowLights;
		if (lights.size() > 0)
		{ // shadow
			auto shadowIndex = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(glm::mat4) * lights.size());

			for (auto &l : lights)
			{
				if (!l->shadow)
				{
					l->sceneShadowIndex = -1;
					continue;
				}

				l->sceneShadowIndex = shadowIndex;
				shadowLights.push_back(l.get());

				if (l->type == LightType::parallax)
				{
					if (l->changed || camera.changed)
					{
						glm::vec3 p[8];
						auto cameraCoord = camera.coord;
						for (int i = 0; i < 8; i++) p[i] = camera.frustumPoints[i] - cameraCoord;
						auto lighAxis = l->getAxis();
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
						//auto shadowMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, TKE_NEAR, halfDepth + halfDepth) * glm::lookAt(center + halfDepth * lighAxis[2], center, lighAxis[1]);
						auto shadowMatrix = glm::mat4( 1.f, 0.f, 0.f,  0.f,
													   0.f, 1.f, 0.f,  0.f,
													   0.f, 0.f, 0.5f, 0.f,
													   0.f, 0.f, 0.5f, 1.f) *
							glm::ortho(-1.f, 1.f, -1.f, 1.f, TKE_NEAR, TKE_FAR) * glm::lookAt(camera.target + glm::vec3(0, 0, 100), camera.target, glm::vec3(0, 1, 0));

						auto srcOffset = sizeof(glm::mat4) * ranges.size();
						memcpy(map + srcOffset, &shadowMatrix, sizeof(glm::mat4));
						VkBufferCopy range = {};
						range.srcOffset = srcOffset;
						range.dstOffset = sizeof(glm::mat4) * shadowIndex;
						range.size = sizeof(glm::mat4);
						ranges.push_back(range);

						if (defe_shad_position != -1)
							ds_defe->setImage(defe_shad_position, shadowIndex, esmImage.get(), colorSampler, 0, 0, 1, shadowIndex, 1);
					}
					shadowIndex += 6;
				}
				else if (l->type == LightType::point)
				{
					if (l->changed)
					{
						glm::mat4 shadowMatrix[6];

						auto coord = l->getCoord();
						auto proj = glm::perspective(90.f, 1.f, TKE_NEAR, TKE_FAR);
						shadowMatrix[0] = proj * glm::lookAt(coord, coord + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
						shadowMatrix[1] = proj * glm::lookAt(coord, coord + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
						shadowMatrix[2] = proj * glm::lookAt(coord, coord + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
						shadowMatrix[3] = proj * glm::lookAt(coord, coord + glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
						shadowMatrix[4] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
						shadowMatrix[5] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
					}
					shadowIndex += 6;
				}
			}
			stagingBuffer->unmap();
			if (ranges.size() > 0) commandPool->copyBuffer(stagingBuffer->v, shadowBuffer->v, ranges.size(), ranges.data());
		}
		if (lights.size() > 0)
		{ // light attribute
			int lightIndex = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(LightShaderStruct) * lights.size());
			for (auto &l : lights)
			{
				if (l->changed)
				{
					LightShaderStruct stru;
					if (l->type == LightType::parallax)
						stru.coord = glm::vec4(l->getAxis()[2], 0.f);
					else
						stru.coord = glm::vec4(l->getCoord(), l->type);
					stru.color = glm::vec4(l->color, l->sceneShadowIndex);
					stru.spotData = glm::vec4(-l->getAxis()[2], l->range);
					auto srcOffset = sizeof(LightShaderStruct) * ranges.size();
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

		camera.changed = false;
		for (auto &l : lights)
			l->changed = false;
		for (auto &o : objects)
			o->changed = false;
		if (terrain)
			terrain->changed = false;

		// shadow
		cb_shadow->reset();
		cb_shadow->begin();

		for (int i = 0; i < shadowLights.size(); i++)
		{
			auto l = shadowLights[i];

			VkClearValue clearValues[] = {
				{ 1.f, 0 },
				{ 1.f, 1.f, 1.f, 1.f }
			};
			cb_shadow->beginRenderPass(renderPass_depth_clear_image32f_clear, fb_esm[i].get(), clearValues);
			// static
			if (staticObjects.size() > 0)
			{
				cb_shadow->bindVertexBuffer(staticVertexBuffer);
				cb_shadow->bindIndexBuffer(staticIndexBuffer);
				cb_shadow->bindPipeline(esmPipeline);
				VkDescriptorSet sets[] = {
					ds_esm->v,
					ds_maps->v
				};
				cb_shadow->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
				for (int oId = 0; oId < staticObjects.size(); oId++)
				{
					auto o = staticObjects[oId];
					auto m = o->model;
					for (int gId = 0; gId < m->geometries.size(); gId++)
						cb_shadow->drawModel(m, gId, 1, (i << 28) + (oId << 8) + gId);
				}
			}
			// animated
			if (animatedObjects.size() > 0)
			{
				cb_shadow->bindVertexBuffer(animatedVertexBuffer);
				cb_shadow->bindIndexBuffer(animatedIndexBuffer);
				cb_shadow->bindPipeline(esmAnimPipeline);
				VkDescriptorSet sets[] = {
					ds_esmAnim->v,
					ds_maps->v,
					ds_mrtAnim_bone->v
				};
				cb_shadow->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
				for (int oId = 0; oId < animatedObjects.size(); oId++)
				{
					auto o = animatedObjects[oId];
					auto m = o->model;
					for (int gId = 0; gId < m->geometries.size(); gId++)
						cb_shadow->drawModel(m, gId, 1, (i << 28) + (oId << 8) + gId);
				}
			}
			cb_shadow->endRenderPass();
		}

		cb_shadow->end();

		cb_deferred->reset();
		cb_deferred->begin();

		cb_deferred->beginRenderPass(sceneRenderPass, fb);

		// mrt
			// static
		if (staticIndirectCount > 0)
		{
			cb_deferred->bindVertexBuffer(staticVertexBuffer);
			cb_deferred->bindIndexBuffer(staticIndexBuffer);
			cb_deferred->bindPipeline(mrtPipeline);
			VkDescriptorSet sets[] = {
				ds_mrt->v,
				ds_maps->v
			};
			cb_deferred->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
			cb_deferred->drawIndirectIndex(staticObjectIndirectBuffer.get(), staticIndirectCount);
		}
			// animated
		if (animatedIndirectCount)
		{
			cb_deferred->bindVertexBuffer(animatedVertexBuffer);
			cb_deferred->bindIndexBuffer(animatedIndexBuffer);
			cb_deferred->bindPipeline(mrtAnimPipeline);
			VkDescriptorSet sets[] = {
				ds_mrtAnim->v,
				ds_maps->v,
				ds_mrtAnim_bone->v
			};
			cb_deferred->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
			cb_deferred->drawIndirectIndex(animatedObjectIndirectBuffer.get(), animatedIndirectCount);
		}
			// terrain
		if (terrain)
		{
			cb_deferred->bindPipeline(terrainPipeline);
			cb_deferred->bindDescriptorSet(&ds_terrain->v);
			cb_deferred->draw(4, 0, terrain->block_cx * terrain->block_cx);
		}
			// water
		if (waters.size() > 0)
		{
			int index = 0;
			for (auto &w : waters)
			{
				cb_deferred->bindPipeline(waterPipeline);
				cb_deferred->bindDescriptorSet(&ds_water->v);
				cb_deferred->draw(4, 0, w->blockCx * w->blockCx);
			}
		}

		//cb->imageBarrier(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
		//	VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 
		//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
		//	esmImage.get(), 0, 1, 0, TKE_MAX_SHADOW_COUNT * 8);

		// deferred
		cb_deferred->nextSubpass();
		cb_deferred->bindPipeline(deferredPipeline);
		cb_deferred->bindDescriptorSet(&ds_defe->v);
		cb_deferred->draw(3);

		// compose
		cb_deferred->nextSubpass();
		cb_deferred->bindPipeline(composePipeline);
		cb_deferred->bindDescriptorSet(&ds_comp->v);
		cb_deferred->draw(3);

		cb_deferred->endRenderPass();

		cb_deferred->setEvent(signalEvent);
		cb_deferred->end();
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

	void Scene::load(const std::string &_filename)
	{
		filename = std::experimental::filesystem::path(_filename).string();

		tke::AttributeTree at("scene", filename);
		at.obtainFromAttributes(this, b);
		for (auto &c : at.children)
		{
			if (c->name == "object")
			{
				auto o = new Object;
				c->obtainFromAttributes(o, o->b);
				o->model = getModel(o->model_filename);
				if (o->model && o->model->animated)
					o->animationComponent = std::make_unique<AnimationComponent>(o->model);
				o->needUpdateAxis = true;
				o->needUpdateQuat = true;
				o->needUpdateMat = true;
				o->changed = true;
				addObject(o);
			}
			else if (c->name == "light")
			{
				;
			}
			else if (c->name == "terrain")
			{
				auto t = new Terrain;
				c->obtainFromAttributes(t, t->b);
				t->heightMap = getTexture(t->height_map_filename);
				t->blendMap = getTexture(t->blend_map_filename);
				t->colorMaps[0] = getTexture(t->color_map0_filename);
				t->colorMaps[1] = getTexture(t->color_map1_filename);
				t->colorMaps[2] = getTexture(t->color_map2_filename);
				t->colorMaps[3] = getTexture(t->color_map3_filename);
				t->normalMaps[0] = getTexture(t->normal_map0_filename);
				t->normalMaps[1] = getTexture(t->normal_map1_filename);
				t->normalMaps[2] = getTexture(t->normal_map2_filename);
				t->normalMaps[3] = getTexture(t->normal_map3_filename);
				t->needUpdateAxis = true;
				t->needUpdateQuat = true;
				t->needUpdateMat = true;
				t->changed = true;
				addTerrain(t);
			}
		}
	}

	void Scene::save(const std::string &filename)
	{
		tke::AttributeTree at("scene");
		at.addAttributes(this, b);
		for (auto &o : objects)
		{
			auto n = new AttributeTreeNode("object");
			o->getCoord();
			o->getEuler();
			o->getScale();
			n->addAttributes(o.get(), o->b);
			at.add(n);
		}
		if (terrain)
		{
			auto n = new AttributeTreeNode("terrain");
			terrain->getCoord();
			terrain->getEuler();
			terrain->getScale();
			n->addAttributes(terrain.get(), terrain->b);
			at.add(n);
		}
		at.saveXML(filename);
	}

	void initScene()
	{
		for (int i = 0; i < 3; i++)
			envrImageDownsample[i] = new Image(TKE_ENVR_SIZE_CX >> (i + 1), TKE_ENVR_SIZE_CY >> (i + 1), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		VkAttachmentDescription atts[] = {
			colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_DONT_CARE), // main
			depthAttachmentDesc(VK_FORMAT_D16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),				 // depth
			colorAttachmentDesc(VK_FORMAT_R16G16B16A16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),		 // albedo alpha
			colorAttachmentDesc(VK_FORMAT_R16G16B16A16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),		 // normal height
			colorAttachmentDesc(VK_FORMAT_R16G16B16A16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),		 // spec roughness
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
			subpassDesc(ARRAYSIZE(mrt_col_ref), mrt_col_ref, &dep_ref), // mrt
			subpassDesc(1, &main_col_ref),                              // deferred
			subpassDesc(1, &dst_col_ref)                                // compose
		};

		VkSubpassDependency dependencies[] = {
			subpassDependency(0, 1),
			subpassDependency(1, 2)
		};

		sceneRenderPass = new RenderPass(ARRAYSIZE(atts), atts, ARRAYSIZE(subpasses), subpasses, ARRAYSIZE(dependencies), dependencies);

		scatteringPipeline = new Pipeline;
		scatteringPipeline->loadXML(enginePath + "pipeline/sky/scattering.xml");
		scatteringPipeline->setup(renderPass_image16, 0, false);

		downsamplePipeline = new Pipeline;
		downsamplePipeline->loadXML(enginePath + "pipeline/sky/downsample.xml");
		downsamplePipeline->setup(renderPass_image16, 0, true);

		convolvePipeline = new Pipeline;
		convolvePipeline->loadXML(enginePath + "pipeline/sky/convolve.xml");
		convolvePipeline->setup(renderPass_image16, 0, true);

		mrtPipeline = new Pipeline;
		mrtPipeline->loadXML(enginePath + "pipeline/deferred/mrt.xml");
		mrtPipeline->setup(sceneRenderPass, 0, false);

		mrtAnimPipeline = new Pipeline;
		mrtAnimPipeline->loadXML(enginePath + "pipeline/deferred/mrt_anim.xml");
		mrtAnimPipeline->setup(sceneRenderPass, 0, false);
		mrt_bone_position = mrtAnimPipeline->descriptorPosition("BONE");

		terrainPipeline = new Pipeline;
		terrainPipeline->loadXML(enginePath + "pipeline/deferred/terrain.xml");
		terrainPipeline->setup(sceneRenderPass, 0, false);
		terr_heightMap_position = terrainPipeline->descriptorPosition("heightMap");
		terr_blendMap_position = terrainPipeline->descriptorPosition("blendMap");
		terr_colorMap_position = terrainPipeline->descriptorPosition("colorMaps");
		terr_normalMap_position = terrainPipeline->descriptorPosition("normalMaps");

		waterPipeline = new Pipeline;
		waterPipeline->loadXML(enginePath + "pipeline/deferred/water.xml");
		waterPipeline->setup(sceneRenderPass, 0, false);

		//proceduralTerrainPipeline = new Pipeline;
		//proceduralTerrainPipeline->loadXML(enginePath + "pipeline/deferred/procedural_terrain.xml");
		//proceduralTerrainPipeline->setup(sceneRenderPass, 1);

		esmPipeline = new Pipeline;
		esmPipeline->loadXML(enginePath + "pipeline/esm/esm.xml");
		esmPipeline->setup(renderPass_depth_clear_image8_clear, 0, false);

		esmAnimPipeline = new Pipeline;
		esmAnimPipeline->loadXML(enginePath + "pipeline/esm/esm_anim.xml");
		esmAnimPipeline->setup(renderPass_depth_clear_image8_clear, 0, false);

		deferredPipeline = new Pipeline;
		deferredPipeline->loadXML(enginePath + "pipeline/deferred/deferred.xml");
		deferredPipeline->setup(sceneRenderPass, 1, false);
		defe_envr_position = deferredPipeline->descriptorPosition("envrSampler");
		defe_shad_position = deferredPipeline->descriptorPosition("shadowSampler");

		composePipeline = new Pipeline;
		composePipeline->loadXML(enginePath + "pipeline/compose/compose.xml");
		composePipeline->setup(sceneRenderPass, 2, false);
	}
}
