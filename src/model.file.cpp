#include <experimental/filesystem>
#include <sstream>
#include <memory>

#include "core.h"
#include "scene.h"
#include "model.file.h"
#include "image.file.h"

namespace tke
{
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
			reportMinorProgress(0);

			int currentIndex = 0;
			int currentRenderGroupID = -1;
			Material *pmt = nullptr;

			std::vector<glm::vec3> rawPositions;
			std::vector<glm::vec2> rawTexcoords;
			std::vector<glm::vec3> rawNormals;
			std::vector<glm::ivec3> rawIndexs;

			reportMinorProgress(10);

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
							obj_line_ss >> ids[j];

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

						}
						m->indices.push_back(index);
						currentIndex++;
						m->renderGroups[currentRenderGroupID].indiceCount++;
					}
				}
				else if (token == "usemtl")
				{
					std::string name;
					ss >> name;
					for (int i = 0; i < m->renderGroups.size(); i++)
					{
						if (name.compare(m->renderGroups[i].material.name) == 0)
						{
							currentRenderGroupID = i;
							m->renderGroups[i].indiceBase = currentIndex;
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
								m->renderGroups.emplace_back();

								std::string mtlName;
								ss >> mtlName;
								pmt = &m->renderGroups.back().material;
								pmt->name = mtlName;
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

			reportMinorProgress(50);

			m->createTangent();

			reportMinorProgress(100);
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

			reportMinorProgress(0);

			m->animated = true;

			Header header;
			file.read((char*)&header, sizeof(Header));

			int vertexCount;
			file >> vertexCount;
			m->positions.resize(vertexCount);
			m->normals.resize(vertexCount);
			m->uvs.resize(vertexCount);
			m->clusterCounts.resize(vertexCount);
			m->boneIDs.resize(vertexCount);
			m->clusterWeights.resize(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				VertexData data;
				file.read((char*)&data, sizeof(VertexData));
				m->positions[i] = data.position;
				m->positions[i].z *= -1.f;
				m->normals[i] = data.normal;
				m->normals[i].z *= -1.f;
				m->uvs[i] = data.uv;
				m->uvs[i].y *= -1.f;
				m->boneIDs[i].x = data.boneID0;
				m->boneIDs[i].y = data.boneID1;
				if (data.weight < 100) m->clusterCounts[i] = 2.f;
				else m->clusterCounts[i] = 1.f;
				float fWeight = data.weight / 100.f;
				m->clusterWeights[i].x = fWeight;
				m->clusterWeights[i].y = 1.f - fWeight;
			}
			m->createTangent();

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

			int renderGroupCount;
			file >> renderGroupCount;
			m->renderGroups.resize(renderGroupCount);
			int currentIndiceVertex = 0;
			for (int i = 0; i < renderGroupCount; i++)
			{
				MaterialData data;
				file.read((char*)&data, sizeof(MaterialData));

				auto prg = &m->renderGroups[i];
				auto pmt = &prg->material;
				pmt->name = std::to_string(i);

				pmt->albedoR = data.diffuse.r * 255;
				pmt->albedoG = data.diffuse.g * 255;
				pmt->albedoB = data.diffuse.b * 255;
				pmt->alpha = data.diffuse.a * 255;
				prg->indiceBase = currentIndiceVertex;
				prg->indiceCount = data.indiceCount;

				auto pImage = m->getImage(data.mapName);
				if (!pImage)
				{
					pImage = createImage(m->filepath + "/" + data.mapName, true, true);
					if (pImage) m->pImages.push_back(pImage);
				}
				pmt->albedoAlphaMap = pImage;


				currentIndiceVertex += data.indiceCount;
			}

			unsigned short boneCount;
			file >> boneCount;
			m->bones.resize(boneCount);
			for (int i = 0; i < boneCount; i++)
			{
				BoneData data;
				file.read((char*)&data, sizeof(BoneData));

				m->bones[i].parent = data.parent;
				m->bones[i].type = data.type;
				m->bones[i].rootCoord = data.coord;
				m->bones[i].rootCoord.z *= -1.f;
			}

			m->arrangeBone();

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
				p->name = data.name;
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
				p->mode = (Rigidbody::Mode)data.mode;
				m->addRigidbody(p);
				auto q = new Shape;
				p->addShape(q);
				switch (data.type)
				{
				case 0: q->type = Shape::Type::eSphere; break;
				case 1: q->type = Shape::Type::eBox; break;
				case 2: q->type = Shape::Type::eCapsule; break;
				}
				switch (q->type)
				{
				case Shape::Type::eSphere:
					data.size.y = data.size.z = data.size.x;
					break;
				case Shape::Type::eCapsule:
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

			reportMinorProgress(100);
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

		void load(AnimationTemplate *a, std::ifstream &file)
		{
			static_assert(sizeof(Header) == 50, "");
			static_assert(sizeof(BoneMotionData) == 111, "");

			Header header;
			file.read((char*)&header, sizeof(Header));

			int count;
			file >> count;
			a->motions.resize(count);
			for (int i = 0; i < count; i++)
			{
				BoneMotionData data;
				file.read((char*)&data, sizeof(BoneMotionData));
				a->motions[i].name = data.name;
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
		static Animation *_loadAnimation(std::ifstream &file, Model *pModel)
		{
			std::string animName;
			file >> animName;
			for (auto anim : scene->pAnimTemps)
			{
				if (anim->name == animName == 0)
					return pModel->bindAnimation(anim);
			}
			return nullptr;
		}

		void load(Model *m, std::ifstream &file)
		{
			reportMinorProgress(0);

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

			reportMinorProgress(30);

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
					m->clusterCounts.resize(vertexCount);
					m->boneIDs.resize(vertexCount);
					m->clusterWeights.resize(vertexCount);
					file.read((char*)m->clusterCounts.data(), vertexCount * sizeof(float));
					file.read((char*)m->boneIDs.data(), vertexCount * sizeof(glm::vec4));
					file.read((char*)m->clusterWeights.data(), vertexCount * sizeof(glm::vec4));
				}
			}
			if (indiceCount > 0)
			{
				m->indices.reserve(indiceCount);
				file.read((char*)m->indices.data(), indiceCount * sizeof(int));
			}

			reportMinorProgress(50);

			int renderGroupCount;
			file >> renderGroupCount;
			for (int i = 0; i < renderGroupCount; i++)
			{
				RenderGroupTemplate rg;

				file.read((char*)rg.type, sizeof(RenderGroupTemplate::Type));

				file >> rg.indiceBase;
				file >> rg.indiceCount;

				file >> rg.visible;

				file >> rg.material.name;
				file >> rg.material.albedoR;
				file >> rg.material.albedoG;
				file >> rg.material.albedoB;
				file >> rg.material.alpha;
				file >> rg.material.spec;
				file >> rg.material.roughness;
				std::string name;
				file >> name;
				rg.material.albedoAlphaMap = m->getImage(name.c_str());
				file >> name;
				rg.material.normalHeightMap = m->getImage(name.c_str());
				file >> name;
				rg.material.specRoughnessMap = m->getImage(name.c_str());

				m->renderGroups.push_back(rg);
			}

			reportMinorProgress(80);

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

			m->arrangeBone();

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
				int mode;
				file >> mode;
				p->mode = (Rigidbody::Mode)mode;
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
					q->type = (Shape::Type)type;
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

			file >> m->controllerPosition;
			file >> m->controllerHeight;
			file >> m->controllerRadius;

			file >> m->boundingPosition;
			file >> m->boundingSize;

			file >> m->eyePosition;
			file >> m->mainWeaponPosition;

			reportMinorProgress(100);
		}

		static void _saveAnimation(std::ofstream &file, Model *pModel, Animation *pAnim)
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

			std::ofstream file(filename);

			file << m->pImages.size();
			for (auto pImage : m->pImages)
			{
				file << pImage->filename;
				file << pImage->m_sRGB;
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
					file.write((char*)m->clusterCounts.data(), vertexCount * sizeof(float));
					file.write((char*)m->boneIDs.data(), vertexCount * sizeof glm::vec4);
					file.write((char*)m->clusterWeights.data(), vertexCount * sizeof glm::vec4);
				}
			}
			if (indiceCount > 0)
			{
				file.write((char*)m->indices.data(), vertexCount * sizeof(int));
			}

			file << m->renderGroups.size();
			for (auto &rg : m->renderGroups)
			{
				int type = (int)rg.type;
				file << type;

				file << rg.indiceBase;
				file << rg.indiceCount;

				file << rg.visible;

				file << rg.material.name;

				file << rg.material.albedoR;
				file << rg.material.albedoG;
				file << rg.material.albedoB;
				file << rg.material.alpha;
				file << rg.material.spec;
				file << rg.material.roughness;
				if (rg.material.albedoAlphaMap) file << rg.material.albedoAlphaMap->filename;
				else file << 0;
				if (rg.material.normalHeightMap) file << rg.material.normalHeightMap->filename;
				else file << 0;
				if (rg.material.specRoughnessMap) file << rg.material.specRoughnessMap->filename;
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
				int mode = (int)rb->mode;
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

			file << m->controllerPosition;
			file << m->controllerHeight;
			file << m->controllerRadius;

			file << m->boundingPosition;
			file << m->boundingSize;

			file << m->eyePosition;
			file << m->mainWeaponPosition;
		}
	}

	namespace TKA
	{
		void load(AnimationTemplate *a, std::ifstream &file)
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

		void save(AnimationTemplate *a, const std::string &filename)
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
			report("Model File Lost:" + filename);
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
			report("Model Format Not Support:" + ext);
			return nullptr;
		}

		auto pModel = new Model;
		pModel->name = p.filename().string();
		pModel->filepath = p.parent_path().string();
		load_func(pModel, file);

		return pModel;
	}

	AnimationTemplate *createAnimation(const std::string &filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.good())
		{
			report("Animation File Lost:" + filename);
			return nullptr;
		}

		std::experimental::filesystem::path p(filename);
		auto ext = p.extension().string();
		void(*load_func)(AnimationTemplate *, std::ifstream &) = nullptr;
		if (ext == ".vmd")
			load_func = &VMD::load;
		else if (ext == ".t3a")
			load_func = &TKA::load;
		else
		{
			report("Animation Format Not Support:%s"+ ext);
			return nullptr;
		}
		auto pAnimation = new AnimationTemplate;
		pAnimation->name = p.filename().string();
		pAnimation->filepath = p.parent_path().string();
		load_func(pAnimation, file);

		return pAnimation;
	}
}