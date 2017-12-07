//This file is auto generated by Project Reflect, please do not edit this file.
#include "..\src\core.h"
#include "..\src\entity\camera.h"
#include "..\src\entity\controller.h"
#include "..\src\entity\light.h"
#include "..\src\entity\object.h"
#include "..\src\entity\scene.h"
#include "..\src\entity\sky.h"
#include "..\src\entity\terrain.h"
#include "..\src\entity\transformer.h"
#include "..\src\entity\water.h"
#include "..\src\image_data.h"
#include "..\src\math\math.h"
#include "..\src\model\animation.h"
#include "..\src\model\material.h"
#include "..\src\model\model.h"
#include "..\src\physics\physics.h"
#include "..\src\render\buffer.h"
#include "..\src\render\command_buffer.h"
#include "..\src\render\descriptor.h"
#include "..\src\render\display_layer.h"
#include "..\src\render\framebuffer.h"
#include "..\src\render\image.h"
#include "..\src\render\pipeline.h"
#include "..\src\render\push_constant.h"
#include "..\src\render\renderer.h"
#include "..\src\render\renderpass.h"
#include "..\src\render\sampler.h"
#include "..\src\render\shader.h"
#include "..\src\render\synchronization.h"
#include "..\src\render\vulkan.h"
#include "..\src\resource\resource.h"
#include "..\src\sound\sound.h"
#include "..\src\ui\ui.h"
#include "..\src\utils.h"
namespace tke{
//define:
float near_plane = 0.1f;
float far_plane = 1000.f;
float fovy = 60.f;
int nowTime;
std::string enginePath;
int resCx;
int resCy;
float screenAspect;
glm::mat4 matOrtho;
glm::mat4 matOrthoInv;
glm::mat4 matPerspective;
glm::mat4 matPerspectiveInv;
StagingBuffer *stagingBuffer = nullptr;
UniformBuffer *constantBuffer = nullptr;
Image *depthImage = nullptr;
Image *pickUpImage = nullptr;
RenderPass *renderPass_image8 = nullptr;
RenderPass *renderPass_image8C = nullptr;
RenderPass *renderPass_image16 = nullptr;
RenderPass *renderPass_image16C = nullptr;
RenderPass *renderPass_depthC = nullptr;
RenderPass *renderPass_depthC_image8 = nullptr;
RenderPass *renderPass_depthC_image8C = nullptr;
RenderPass *renderPass_depthC_image32fC = nullptr;
RenderPass *renderPass_window = nullptr;
RenderPass *renderPass_windowC = nullptr;
std::shared_ptr<Framebuffer> pickUpFb;
HWND hWnd;
int window_cx;
int window_cy;
KeyState mouseLeft;
KeyState mouseMiddle;
KeyState mouseRight;
int mouseX;
int mouseY;
int mousePrevX;
int mousePrevY;
int mouseDispX;
int mouseDispY;
int mouseScroll;
KeyState keyStates[256];
VkSurfaceKHR window_surface;
VkSwapchainKHR swapchain;
Image *window_images[2];
std::shared_ptr<Framebuffer> window_framebuffers[2];
VkSemaphore window_imageAvailable;
uint32_t window_imageIndex;
VkFence frameDone;
uint32_t FPS;
PF_EVENT1 onKeyDown = nullptr;
PF_EVENT1 onKeyUp = nullptr;
PF_EVENT1 onChar = nullptr;
PF_EVENT2 onMouseLeftDown = nullptr;
PF_EVENT2 onMouseLeftUp = nullptr;
PF_EVENT2 onMouseMiddleDown = nullptr;
PF_EVENT2 onMouseMiddleUp = nullptr;
PF_EVENT2 onMouseRightDown = nullptr;
PF_EVENT2 onMouseRightUp = nullptr;
PF_EVENT2 onMouseMove = nullptr;
PF_EVENT1 onMouseWheel = nullptr;
PF_EVENT0 onRender = nullptr;
PF_EVENT0 onDestroy = nullptr;
std::vector<std::unique_ptr<FrameCommandBufferList>> frameCbLists;
tke::ReflectionBank *Controller::b = tke::addReflectionBank("Controller");
tke::ReflectionBank *Object::b = tke::addReflectionBank("Object");
Pipeline *scatteringPipeline = nullptr;
Pipeline *downsamplePipeline = nullptr;
Pipeline *convolvePipeline = nullptr;
Pipeline *mrtPipeline = nullptr;
Pipeline *mrtAnimPipeline = nullptr;
Pipeline *terrainPipeline = nullptr;
Pipeline *waterPipeline = nullptr;
Pipeline *proceduralTerrainPipeline = nullptr;
Pipeline *deferredPipeline = nullptr;
Pipeline *esmPipeline = nullptr;
Pipeline *esmAnimPipeline = nullptr;
Pipeline *composePipeline = nullptr;
tke::ReflectionBank *Scene::b = tke::addReflectionBank("Scene");
tke::ReflectionBank *Terrain::b = tke::addReflectionBank("Terrain");
tke::ReflectionBank *Transformer::b = tke::addReflectionBank("Transformer");
tke::ReflectionBank *Water::b = tke::addReflectionBank("Water");
tke::ReflectionBank *Model::b = tke::addReflectionBank("Model");
VkPipelineVertexInputStateCreateInfo vertexStatInputState;
VkPipelineVertexInputStateCreateInfo vertexAnimInputState;
VertexBuffer *vertexStatBuffer = nullptr;
VertexBuffer *vertexAnimBuffer = nullptr;
IndexBuffer *indexBuffer = nullptr;
std::weak_ptr<Material> modelMaterials[MaxMaterialCount];
std::shared_ptr<Material> defaultMaterial = nullptr;
UniformBuffer *materialBuffer = nullptr;
std::weak_ptr<Image> modelTextures[MaxTextureCount];
DescriptorSet *ds_textures = nullptr;
std::shared_ptr<Model> triangleModel;
std::shared_ptr<Model> cubeModel;
std::shared_ptr<Model> sphereModel;
std::shared_ptr<Model> cylinderModel;
std::shared_ptr<Model> coneModel;
std::shared_ptr<Model> arrowModel;
std::shared_ptr<Model> torusModel;
std::shared_ptr<Model> hamerModel;
VkPipelineVertexInputStateCreateInfo zeroVertexInputState;
tke::ReflectionBank *PushConstantRange::b = tke::addReflectionBank("PushConstantRange");
std::vector<std::weak_ptr<Shader>> loaded_shaders;
bool uiAcceptedMouse;
bool uiAcceptedKey;
glm::vec3 bkColor = glm::vec3(0.69f,0.76f,0.79f);
struct ReflectInit{ReflectInit(){
tke::EnumType *currentEnumType = nullptr;
tke::ReflectionBank *currentBank = nullptr;
//enum:
currentEnumType = tke::addReflectEnumType("ObjectPhysicsType");
currentEnumType->items.emplace_back("enable", (int)ObjectPhysicsType::enable);
currentEnumType->items.emplace_back("dynamic", (int)ObjectPhysicsType::dynamic);
currentEnumType->items.emplace_back("controller", (int)ObjectPhysicsType::controller);
currentEnumType = tke::addReflectEnumType("SamplerType");
currentEnumType->items.emplace_back("none", (int)SamplerType::none);
currentEnumType->items.emplace_back("plain", (int)SamplerType::plain);
currentEnumType->items.emplace_back("plain_unnormalized", (int)SamplerType::plain_unnormalized);
currentEnumType->items.emplace_back("color", (int)SamplerType::color);
currentEnumType->items.emplace_back("color_border", (int)SamplerType::color_border);
//impl:
currentBank = Controller::b;
currentBank->addV<float>("ang_offset", offsetof(Controller, ang_offset));
currentBank->addV<float>("speed", offsetof(Controller, speed));
currentBank->addV<float>("turn_speed", offsetof(Controller, turn_speed));
currentBank = Object::b;
currentBank->parents.emplace_back(Controller::b, TK_DERIVE_OFFSET(Object, Controller));
currentBank->addV<std::string>("model_filename", offsetof(Object, model_filename));
currentBank->addV<std::string>("name", offsetof(Object, name));
currentBank->addV<std::uint32_t>("physics_type", offsetof(Object, physics_type));
currentBank = Scene::b;
currentBank = Terrain::b;
currentBank->addV<bool>("use_physx", offsetof(Terrain, use_physx));
currentBank->addV<std::string>("height_map_filename", offsetof(Terrain, height_map_filename));
currentBank->addV<std::string>("normal_map_filename", offsetof(Terrain, normal_map_filename));
currentBank->addV<std::string>("blend_map_filename", offsetof(Terrain, blend_map_filename));
currentBank->addV<std::string>("color_map0_filename", offsetof(Terrain, color_map0_filename));
currentBank->addV<std::string>("color_map1_filename", offsetof(Terrain, color_map1_filename));
currentBank->addV<std::string>("color_map2_filename", offsetof(Terrain, color_map2_filename));
currentBank->addV<std::string>("color_map3_filename", offsetof(Terrain, color_map3_filename));
currentBank->addV<std::string>("normal_map0_filename", offsetof(Terrain, normal_map0_filename));
currentBank->addV<std::string>("normal_map1_filename", offsetof(Terrain, normal_map1_filename));
currentBank->addV<std::string>("normal_map2_filename", offsetof(Terrain, normal_map2_filename));
currentBank->addV<std::string>("normal_map3_filename", offsetof(Terrain, normal_map3_filename));
currentBank->addV<int>("block_cx", offsetof(Terrain, block_cx));
currentBank->addV<float>("block_size", offsetof(Terrain, block_size));
currentBank->addV<float>("height", offsetof(Terrain, height));
currentBank->addV<float>("tessellation_factor", offsetof(Terrain, tessellation_factor));
currentBank->addV<float>("texture_uv_factor", offsetof(Terrain, texture_uv_factor));
currentBank = Transformer::b;
currentBank->addV<glm::vec3>("coord", offsetof(Transformer, coord));
currentBank->addV<glm::vec3>("euler", offsetof(Transformer, euler));
currentBank->addV<glm::vec3>("scale", offsetof(Transformer, scale));
currentBank = Water::b;
currentBank->parents.emplace_back(Transformer::b, TK_DERIVE_OFFSET(Water, Transformer));
currentBank = Model::b;
currentBank->addV<std::string>("stand_animation_filename", offsetof(Model, stand_animation_filename));
currentBank->addV<std::string>("forward_animation_filename", offsetof(Model, forward_animation_filename));
currentBank->addV<std::string>("leftward_animation_filename", offsetof(Model, leftward_animation_filename));
currentBank->addV<std::string>("rightward_animation_filename", offsetof(Model, rightward_animation_filename));
currentBank->addV<std::string>("backward_animation_filename", offsetof(Model, backward_animation_filename));
currentBank->addV<std::string>("jump_animation_filename", offsetof(Model, jump_animation_filename));
currentBank->addV<glm::vec3>("controller_position", offsetof(Model, controller_position));
currentBank->addV<float>("controller_height", offsetof(Model, controller_height));
currentBank->addV<float>("controller_radius", offsetof(Model, controller_radius));
currentBank->addV<glm::vec3>("eye_position", offsetof(Model, eye_position));
currentBank = PushConstantRange::b;
currentBank->addV<int>("offset", offsetof(PushConstantRange, offset));
currentBank->addV<int>("size", offsetof(PushConstantRange, size));
}};static ReflectInit _init;
}