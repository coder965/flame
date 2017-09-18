//This file is auto generated by Project Reflect, please do not edit this file.
#include "..\src\core.h"
#include "..\src\entity\camera.h"
#include "..\src\entity\controller.h"
#include "..\src\entity\light.h"
#include "..\src\entity\object.h"
#include "..\src\entity\scene.h"
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
#include "..\src\render\framebuffer.h"
#include "..\src\render\image.h"
#include "..\src\render\pipeline.h"
#include "..\src\render\push_constant.h"
#include "..\src\render\renderpass.h"
#include "..\src\render\sampler.h"
#include "..\src\render\shader.h"
#include "..\src\render\synchronization.h"
#include "..\src\render\vulkan.h"
#include "..\src\resource\resource.h"
#include "..\src\sound\sound.h"
#include "..\src\ui\ui.h"
#include "..\src\utils.h"
#include "..\src\window.h"
namespace tke{
float near_plane = 0.1f;
float far_plane = 1000.f;
float fovy = 60.f;
int lastTime;
int nowTime;
int timeDisp;
std::string enginePath;
int resCx;
int resCy;
float screenAspect;
glm::mat4 matOrtho;
glm::mat4 matOrthoInv;
glm::mat4 matPerspective;
glm::mat4 matPerspectiveInv;
StagingBuffer *stagingBuffer = nullptr;
bool needUpdateVertexBuffer;
bool needUpdateMaterialBuffer;
bool needUpdateTexture;
std::vector<std::pair<std::string,Image*>> debugImages;
std::vector<std::unique_ptr<Image>> textures;
std::vector<std::unique_ptr<Image>> modelTextures;
std::vector<Material*> modelMaterials;
Material *defaultMaterial = nullptr;
std::vector<std::unique_ptr<Animation>> animations;
std::vector<std::unique_ptr<Model>> models;
std::vector<std::unique_ptr<Scene>> scenes;
VertexBuffer *staticVertexBuffer = nullptr;
IndexBuffer *staticIndexBuffer = nullptr;
VertexBuffer *animatedVertexBuffer = nullptr;
IndexBuffer *animatedIndexBuffer = nullptr;
UniformBuffer *constantBuffer = nullptr;
UniformBuffer *materialBuffer = nullptr;
Image *plainDepthImage = nullptr;
Image *pickUpImage = nullptr;
RenderPass *renderPass_image8 = nullptr;
RenderPass *renderPass_image8_clear = nullptr;
RenderPass *renderPass_image16 = nullptr;
RenderPass *renderPass_image16_clear = nullptr;
RenderPass *renderPass_depth_clear = nullptr;
RenderPass *renderPass_depth_clear_image8 = nullptr;
RenderPass *renderPass_depth_clear_image8_clear = nullptr;
RenderPass *renderPass_depth_clear_image32f_clear = nullptr;
Framebuffer *pickUpFb = nullptr;
Pipeline *plainPipeline_2d = nullptr;
Pipeline *plainPipeline_3d = nullptr;
Pipeline *plainPipeline_3d_anim = nullptr;
Pipeline *plainPipeline_3d_normal = nullptr;
Pipeline *plainPipeline_3d_tex = nullptr;
Pipeline *plainPipeline_3d_anim_tex = nullptr;
Pipeline *plainPipeline_3d_wire = nullptr;
Pipeline *plainPipeline_3d_anim_wire = nullptr;
Pipeline *plainPipeline_3d_line = nullptr;
int plain3d_bone_pos = -1;
DescriptorSet *ds_maps = nullptr;
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
tke::ReflectionBank *Descriptor::b = tke::addReflectionBank("Descriptor");
tke::ReflectionBank *BlendAttachment::b = tke::addReflectionBank("BlendAttachment");
tke::ReflectionBank *DynamicState::b = tke::addReflectionBank("DynamicState");
tke::ReflectionBank *LinkResource::b = tke::addReflectionBank("LinkResource");
tke::ReflectionBank *Pipeline::b = tke::addReflectionBank("Pipeline");
tke::ReflectionBank *PushConstantRange::b = tke::addReflectionBank("PushConstantRange");
struct ReflectInit{ReflectInit(){
tke::EnumType *currentEnumType = nullptr;
tke::ReflectionBank *currentBank = nullptr;
currentEnumType = tke::addReflectEnumType("ObjectPhysicsType");
currentEnumType->items.emplace_back("null", (int)ObjectPhysicsType::null);
currentEnumType->items.emplace_back("static_r", (int)ObjectPhysicsType::static_r);
currentEnumType->items.emplace_back("dynamic", (int)ObjectPhysicsType::dynamic);
currentEnumType->items.emplace_back("controller", (int)ObjectPhysicsType::controller);
currentEnumType = tke::addReflectEnumType("DescriptorType");
currentEnumType->items.emplace_back("uniform_buffer", (int)DescriptorType::uniform_buffer);
currentEnumType->items.emplace_back("image_n_sampler", (int)DescriptorType::image_n_sampler);
currentEnumType = tke::addReflectEnumType("PrimitiveTopology");
currentEnumType->items.emplace_back("triangle_list", (int)PrimitiveTopology::triangle_list);
currentEnumType->items.emplace_back("line_list", (int)PrimitiveTopology::line_list);
currentEnumType->items.emplace_back("line_strip", (int)PrimitiveTopology::line_strip);
currentEnumType->items.emplace_back("patch_list", (int)PrimitiveTopology::patch_list);
currentEnumType = tke::addReflectEnumType("PolygonMode");
currentEnumType->items.emplace_back("fill", (int)PolygonMode::fill);
currentEnumType->items.emplace_back("line", (int)PolygonMode::line);
currentEnumType = tke::addReflectEnumType("CullMode");
currentEnumType->items.emplace_back("none", (int)CullMode::none);
currentEnumType->items.emplace_back("front", (int)CullMode::front);
currentEnumType->items.emplace_back("back", (int)CullMode::back);
currentEnumType->items.emplace_back("front_and_back", (int)CullMode::front_and_back);
currentEnumType = tke::addReflectEnumType("BlendFactor");
currentEnumType->items.emplace_back("zero", (int)BlendFactor::zero);
currentEnumType->items.emplace_back("one", (int)BlendFactor::one);
currentEnumType->items.emplace_back("src_alpha", (int)BlendFactor::src_alpha);
currentEnumType->items.emplace_back("one_minus_src_alpha", (int)BlendFactor::one_minus_src_alpha);
currentEnumType = tke::addReflectEnumType("VertexInputType");
currentEnumType->items.emplace_back("zero", (int)VertexInputType::zero);
currentEnumType->items.emplace_back("plain2d", (int)VertexInputType::plain2d);
currentEnumType->items.emplace_back("normal", (int)VertexInputType::normal);
currentEnumType->items.emplace_back("animated", (int)VertexInputType::animated);
currentEnumType->items.emplace_back("line", (int)VertexInputType::line);
currentEnumType = tke::addReflectEnumType("DynamicStateType");
currentEnumType->items.emplace_back("viewport", (int)DynamicStateType::viewport);
currentEnumType->items.emplace_back("scissor", (int)DynamicStateType::scissor);
currentEnumType = tke::addReflectEnumType("SamplerType");
currentEnumType->items.emplace_back("none", (int)SamplerType::none);
currentEnumType->items.emplace_back("plain", (int)SamplerType::plain);
currentEnumType->items.emplace_back("plain_unnormalized", (int)SamplerType::plain_unnormalized);
currentEnumType->items.emplace_back("color", (int)SamplerType::color);
currentEnumType->items.emplace_back("color_border", (int)SamplerType::color_border);
currentEnumType = tke::addReflectEnumType("StageType");
currentEnumType->items.emplace_back("vert", (int)StageType::vert);
currentEnumType->items.emplace_back("tesc", (int)StageType::tesc);
currentEnumType->items.emplace_back("tese", (int)StageType::tese);
currentEnumType->items.emplace_back("geom", (int)StageType::geom);
currentEnumType->items.emplace_back("frag", (int)StageType::frag);
currentBank = Controller::b;
currentBank->addV<float>("ang_offset", offsetof(Controller, ang_offset));
currentBank->addV<float>("speed", offsetof(Controller, speed));
currentBank->addV<float>("turn_speed", offsetof(Controller, turn_speed));
currentBank = Object::b;
currentBank->parents.emplace_back(Controller::b, TK_STRUCT_OFFSET(Object, Controller));
currentBank->addV<std::string>("model_filename", offsetof(Object, model_filename));
currentBank->addV<std::string>("name", offsetof(Object, name));
currentBank->addE("ObjectPhysicsType", "physics_type", offsetof(Object, physics_type));
currentBank = Scene::b;
currentBank->addV<std::string>("name", offsetof(Scene, name));
currentBank = Terrain::b;
currentBank->addV<bool>("use_physx", offsetof(Terrain, use_physx));
currentBank->addV<std::string>("blend_map_filename", offsetof(Terrain, blend_map_filename));
currentBank->addV<std::string>("height_map_filename", offsetof(Terrain, height_map_filename));
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
currentBank->parents.emplace_back(Transformer::b, TK_STRUCT_OFFSET(Water, Transformer));
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
currentBank = Descriptor::b;
currentBank->addE("DescriptorType", "type", offsetof(Descriptor, type));
currentBank->addV<int>("binding", offsetof(Descriptor, binding));
currentBank->addV<int>("count", offsetof(Descriptor, count));
currentBank->addV<std::string>("name", offsetof(Descriptor, name));
currentBank = BlendAttachment::b;
currentBank->addV<bool>("enable", offsetof(BlendAttachment, enable));
currentBank->addE("BlendFactor", "src_color", offsetof(BlendAttachment, src_color));
currentBank->addE("BlendFactor", "dst_color", offsetof(BlendAttachment, dst_color));
currentBank->addE("BlendFactor", "src_alpha", offsetof(BlendAttachment, src_alpha));
currentBank->addE("BlendFactor", "dst_alpha", offsetof(BlendAttachment, dst_alpha));
currentBank = DynamicState::b;
currentBank->addE("DynamicStateType", "type", offsetof(DynamicState, type));
currentBank = LinkResource::b;
currentBank->addV<int>("binding", offsetof(LinkResource, binding));
currentBank->addV<int>("array_element", offsetof(LinkResource, array_element));
currentBank->addV<std::string>("descriptor_name", offsetof(LinkResource, descriptor_name));
currentBank->addV<std::string>("resource_name", offsetof(LinkResource, resource_name));
currentBank->addE("SamplerType", "sampler", offsetof(LinkResource, sampler));
currentBank = Pipeline::b;
currentBank->addV<std::string>("name", offsetof(Pipeline, name));
currentBank->addV<int>("cx", offsetof(Pipeline, cx));
currentBank->addV<int>("cy", offsetof(Pipeline, cy));
currentBank->addE("VertexInputType", "vertex_input_type", offsetof(Pipeline, vertex_input_type));
currentBank->addV<int>("patch_control_points", offsetof(Pipeline, patch_control_points));
currentBank->addV<bool>("depth_test", offsetof(Pipeline, depth_test));
currentBank->addV<bool>("depth_write", offsetof(Pipeline, depth_write));
currentBank->addV<bool>("depth_clamp", offsetof(Pipeline, depth_clamp));
currentBank->addE("PrimitiveTopology", "primitive_topology", offsetof(Pipeline, primitive_topology));
currentBank->addE("PolygonMode", "polygon_mode", offsetof(Pipeline, polygon_mode));
currentBank->addE("CullMode", "cull_mode", offsetof(Pipeline, cull_mode));
currentBank = PushConstantRange::b;
currentBank->addV<int>("offset", offsetof(PushConstantRange, offset));
currentBank->addV<int>("size", offsetof(PushConstantRange, size));
}};static ReflectInit _init;
}