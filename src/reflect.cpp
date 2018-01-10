//This file is auto generated by Project Reflect, please do not edit this file.
#include "..\src\application.h"
#include "..\src\editor\select.h"
#include "..\src\editor\tool\tool.h"
#include "..\src\editor\tool\transformer_tool.h"
#include "..\src\editor\window\file_selector.h"
#include "..\src\editor\window\hierarchy.h"
#include "..\src\editor\window\image_editor.h"
#include "..\src\editor\window\inspector.h"
#include "..\src\editor\window\model_editor.h"
#include "..\src\editor\window\resource_explorer.h"
#include "..\src\editor\window\scene_editor.h"
#include "..\src\editor\window\show_material.h"
#include "..\src\editor\window\terrain_editor.h"
#include "..\src\editor\window\window.h"
#include "..\src\entity\camera.h"
#include "..\src\entity\component.h"
#include "..\src\entity\controller.h"
#include "..\src\entity\light.h"
#include "..\src\entity\node.h"
#include "..\src\entity\object.h"
#include "..\src\entity\scene.h"
#include "..\src\entity\sky.h"
#include "..\src\entity\terrain.h"
#include "..\src\entity\water.h"
#include "..\src\error.h"
#include "..\src\event.h"
#include "..\src\file_utils.h"
#include "..\src\global.h"
#include "..\src\graphics\buffer.h"
#include "..\src\graphics\command_buffer.h"
#include "..\src\graphics\descriptor.h"
#include "..\src\graphics\display_layer.h"
#include "..\src\graphics\framebuffer.h"
#include "..\src\graphics\graphics.h"
#include "..\src\graphics\image.h"
#include "..\src\graphics\material.h"
#include "..\src\graphics\pipeline.h"
#include "..\src\graphics\renderer.h"
#include "..\src\graphics\renderpass.h"
#include "..\src\graphics\sampler.h"
#include "..\src\graphics\shader.h"
#include "..\src\graphics\synchronization.h"
#include "..\src\hash.h"
#include "..\src\image_data.h"
#include "..\src\input.h"
#include "..\src\language.h"
#include "..\src\math\math.h"
#include "..\src\model\animation.h"
#include "..\src\model\model.h"
#include "..\src\physics\physics.h"
#include "..\src\pick_up\pick_up.h"
#include "..\src\refl.h"
#include "..\src\resource\resource.h"
#include "..\src\sound\sound.h"
#include "..\src\string_utils.h"
#include "..\src\system.h"
#include "..\src\time.h"
#include "..\src\type.h"
#include "..\src\ui\ui.h"
#include "..\src\_object.h"
namespace tke{
//define:
UniformBuffer *constantBuffer = nullptr;
void *hWnd;
int window_cx;
int window_cy;
VkSurfaceKHR window_surface;
VkSwapchainKHR swapchain;
Image *window_images[2];
std::shared_ptr<Framebuffer> window_framebuffers[2];
VkSemaphore window_imageAvailable;
uint32_t window_imageIndex;
VkFence frameDone;
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
std::uint32_t window_style;
Camera *main_camera = nullptr;
tke::ReflectionBank *Controller::b = tke::addReflectionBank("Controller");
tke::ReflectionBank *Node::b = tke::addReflectionBank("Node");
tke::ReflectionBank *Object::b = tke::addReflectionBank("Object");
tke::ReflectionBank *Scene::b = tke::addReflectionBank("Scene");
VkPipelineVertexInputStateCreateInfo terrianVertexInputState;
tke::ReflectionBank *Terrain::b = tke::addReflectionBank("Terrain");
tke::ReflectionBank *Water::b = tke::addReflectionBank("Water");
bool only_2d = false;
float near_plane = 0.1f;
float far_plane = 1000.f;
float fovy = 60.f;
int nowTime;
std::string engine_path;
int res_cx;
int res_cy;
float res_aspect;
uint32_t FPS;
StagingBuffer *defalut_staging_buffer = nullptr;
std::shared_ptr<Image> default_color_image;
std::shared_ptr<Image> default_normal_image;
std::shared_ptr<Image> default_blend_image;
std::weak_ptr<Material> materials[MaxMaterialCount];
std::shared_ptr<Material> defaultMaterial;
UniformBuffer *materialBuffer = nullptr;
std::weak_ptr<Image> materialImages[MaxMaterialImageCount];
DescriptorSet *ds_material = nullptr;
VkPipelineVertexInputStateCreateInfo zeroVertexInputState;
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
VkSampler plainSampler;
VkSampler plainUnnormalizedSampler;
VkSampler colorSampler;
VkSampler colorBorderSampler;
VkSampler colorWrapSampler;
tke::ReflectionBank *PushConstantRange::b = tke::addReflectionBank("PushConstantRange");
std::vector<std::weak_ptr<Shader>> loaded_shaders;
int mouseX;
int mouseY;
int mousePrevX;
int mousePrevY;
int mouseDispX;
int mouseDispY;
int mouseScroll;
KeyState mouseLeft;
KeyState mouseMiddle;
KeyState mouseRight;
KeyState keyStates[256];
tke::ReflectionBank *Model::b = tke::addReflectionBank("Model");
VkPipelineVertexInputStateCreateInfo vertexStatInputState;
VkPipelineVertexInputStateCreateInfo vertexAnimInputState;
std::unique_ptr<VertexBuffer> vertexStatBuffer;
std::unique_ptr<VertexBuffer> vertexAnimBuffer;
std::unique_ptr<IndexBuffer> indexBuffer;
std::shared_ptr<Model> triangleModel;
std::shared_ptr<Model> cubeModel;
std::shared_ptr<Model> sphereModel;
std::shared_ptr<Model> cylinderModel;
std::shared_ptr<Model> coneModel;
std::shared_ptr<Model> arrowModel;
std::shared_ptr<Model> torusModel;
std::shared_ptr<Model> hamerModel;
Image *pick_up_image = nullptr;
Image *pick_up_depth_image = nullptr;
std::shared_ptr<Framebuffer> pick_up_fb;
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
currentBank = Node::b;
currentBank->addV<glm::vec3>("coord", offsetof(Node, coord));
currentBank->addV<glm::vec3>("euler", offsetof(Node, euler));
currentBank->addV<glm::vec3>("scale", offsetof(Node, scale));
currentBank = Object::b;
currentBank->parents.emplace_back(Node::b, TK_DERIVE_OFFSET(Object, Node));
currentBank->parents.emplace_back(Controller::b, TK_DERIVE_OFFSET(Object, Controller));
currentBank->addV<std::string>("model_filename", offsetof(Object, model_filename));
currentBank->addV<std::string>("name", offsetof(Object, name));
currentBank->addV<std::uint32_t>("physics_type", offsetof(Object, physics_type));
currentBank = Scene::b;
currentBank->parents.emplace_back(Node::b, TK_DERIVE_OFFSET(Scene, Node));
currentBank = Terrain::b;
currentBank->parents.emplace_back(Node::b, TK_DERIVE_OFFSET(Terrain, Node));
currentBank->addV<bool>("use_physx", offsetof(Terrain, use_physx));
currentBank->addV<int>("block_cx", offsetof(Terrain, block_cx));
currentBank->addV<int>("block_cy", offsetof(Terrain, block_cy));
currentBank->addV<float>("block_size", offsetof(Terrain, block_size));
currentBank->addV<float>("height", offsetof(Terrain, height));
currentBank->addV<float>("displacement_height", offsetof(Terrain, displacement_height));
currentBank->addV<float>("tessellation_factor", offsetof(Terrain, tessellation_factor));
currentBank->addV<float>("tiling_scale", offsetof(Terrain, tiling_scale));
currentBank = Water::b;
currentBank->parents.emplace_back(Node::b, TK_DERIVE_OFFSET(Water, Node));
currentBank = PushConstantRange::b;
currentBank->addV<int>("offset", offsetof(PushConstantRange, offset));
currentBank->addV<int>("size", offsetof(PushConstantRange, size));
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
}};static ReflectInit _init;
}