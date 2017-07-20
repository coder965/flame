#include "utils.h"
#include "render.h"
#include "entity.h"
#include <string>
namespace tke{
tke::ReflectionBank *PushConstantRange::b = tke::addReflectionBank("PushConstantRange");
tke::ReflectionBank *BlendAttachment::b = tke::addReflectionBank("BlendAttachment");
tke::ReflectionBank *Descriptor::b = tke::addReflectionBank("Descriptor");
tke::ReflectionBank *DynamicState::b = tke::addReflectionBank("DynamicState");
tke::ReflectionBank *ShaderMacro::b = tke::addReflectionBank("ShaderMacro");
tke::ReflectionBank *Stage::b = tke::addReflectionBank("Stage");
tke::ReflectionBank *LinkResource::b = tke::addReflectionBank("LinkResource");
tke::ReflectionBank *Pipeline::b = tke::addReflectionBank("Pipeline");
tke::ReflectionBank *Transformer::b = tke::addReflectionBank("Transformer");
tke::ReflectionBank *Object::b = tke::addReflectionBank("Object");
tke::ReflectionBank *Terrain::b = tke::addReflectionBank("Terrain");
tke::ReflectionBank *Scene::b = tke::addReflectionBank("Scene");
struct ReflectInit{ReflectInit(){
tke::Enum *currentEnum = nullptr;
tke::ReflectionBank *currentBank = nullptr;
currentEnum = tke::addReflectEnum("StageType");
currentEnum->items.emplace_back("vert", (int)StageType::vert);
currentEnum->items.emplace_back("tesc", (int)StageType::tesc);
currentEnum->items.emplace_back("tese", (int)StageType::tese);
currentEnum->items.emplace_back("geom", (int)StageType::geom);
currentEnum->items.emplace_back("frag", (int)StageType::frag);
currentEnum = tke::addReflectEnum("Format");
currentEnum->items.emplace_back("R8G8B8A8", (int)Format::R8G8B8A8);
currentEnum->items.emplace_back("R16G16B16A16", (int)Format::R16G16B16A16);
currentEnum->items.emplace_back("R32G32B32A32", (int)Format::R32G32B32A32);
currentEnum = tke::addReflectEnum("AspectFlags");
currentEnum->items.emplace_back("color", (int)AspectFlags::color);
currentEnum->items.emplace_back("depth", (int)AspectFlags::depth);
currentEnum->items.emplace_back("stencil", (int)AspectFlags::stencil);
currentBank = PushConstantRange::b;
currentBank->addV<int>("offset", offsetof(PushConstantRange, offset));
currentBank->addV<int>("size", offsetof(PushConstantRange, size));
currentEnum = tke::addReflectEnum("PrimitiveTopology");
currentEnum->items.emplace_back("triangle_list", (int)PrimitiveTopology::triangle_list);
currentEnum->items.emplace_back("line_list", (int)PrimitiveTopology::line_list);
currentEnum->items.emplace_back("line_strip", (int)PrimitiveTopology::line_strip);
currentEnum->items.emplace_back("patch_list", (int)PrimitiveTopology::patch_list);
currentEnum = tke::addReflectEnum("PolygonMode");
currentEnum->items.emplace_back("fill", (int)PolygonMode::fill);
currentEnum->items.emplace_back("line", (int)PolygonMode::line);
currentEnum = tke::addReflectEnum("CullMode");
currentEnum->items.emplace_back("none", (int)CullMode::none);
currentEnum->items.emplace_back("front", (int)CullMode::front);
currentEnum->items.emplace_back("back", (int)CullMode::back);
currentEnum->items.emplace_back("front_and_back", (int)CullMode::front_and_back);
currentEnum = tke::addReflectEnum("BlendFactor");
currentEnum->items.emplace_back("zero", (int)BlendFactor::zero);
currentEnum->items.emplace_back("one", (int)BlendFactor::one);
currentEnum->items.emplace_back("src_alpha", (int)BlendFactor::src_alpha);
currentEnum->items.emplace_back("one_minus_src_alpha", (int)BlendFactor::one_minus_src_alpha);
currentBank = BlendAttachment::b;
currentBank->addV<bool>("enable", offsetof(BlendAttachment, enable));
currentBank->addE("BlendFactor", "src_color", offsetof(BlendAttachment, src_color));
currentBank->addE("BlendFactor", "dst_color", offsetof(BlendAttachment, dst_color));
currentBank->addE("BlendFactor", "src_alpha", offsetof(BlendAttachment, src_alpha));
currentBank->addE("BlendFactor", "dst_alpha", offsetof(BlendAttachment, dst_alpha));
currentEnum = tke::addReflectEnum("DescriptorType");
currentEnum->items.emplace_back("uniform_buffer", (int)DescriptorType::uniform_buffer);
currentEnum->items.emplace_back("image_n_sampler", (int)DescriptorType::image_n_sampler);
currentBank = Descriptor::b;
currentBank->addE("DescriptorType", "type", offsetof(Descriptor, type));
currentBank->addV<int>("binding", offsetof(Descriptor, binding));
currentBank->addV<int>("count", offsetof(Descriptor, count));
currentBank->addV<std::string>("name", offsetof(Descriptor, name));
currentEnum = tke::addReflectEnum("VertexInputType");
currentEnum->items.emplace_back("zero", (int)VertexInputType::zero);
currentEnum->items.emplace_back("plain2d", (int)VertexInputType::plain2d);
currentEnum->items.emplace_back("normal", (int)VertexInputType::normal);
currentEnum->items.emplace_back("animated", (int)VertexInputType::animated);
currentEnum->items.emplace_back("line", (int)VertexInputType::line);
currentEnum = tke::addReflectEnum("DynamicStateType");
currentEnum->items.emplace_back("viewport", (int)DynamicStateType::viewport);
currentEnum->items.emplace_back("scissor", (int)DynamicStateType::scissor);
currentBank = DynamicState::b;
currentBank->addE("DynamicStateType", "type", offsetof(DynamicState, type));
currentBank = ShaderMacro::b;
currentBank->addE("StageType", "stage", offsetof(ShaderMacro, stage));
currentBank->addV<std::string>("value", offsetof(ShaderMacro, value));
currentBank = Stage::b;
currentBank->addV<std::string>("filename", offsetof(Stage, filename));
currentEnum = tke::addReflectEnum("SamplerType");
currentEnum->items.emplace_back("none", (int)SamplerType::none);
currentEnum->items.emplace_back("plain", (int)SamplerType::plain);
currentEnum->items.emplace_back("plain_unnormalized", (int)SamplerType::plain_unnormalized);
currentEnum->items.emplace_back("color", (int)SamplerType::color);
currentEnum->items.emplace_back("color_border", (int)SamplerType::color_border);
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
currentBank = Transformer::b;
currentBank->addV<glm::vec3>("coord", offsetof(Transformer, coord));
currentBank->addV<glm::vec3>("euler", offsetof(Transformer, euler));
currentBank->addV<glm::vec3>("scale", offsetof(Transformer, scale));
currentEnum = tke::addReflectEnum("ObjectPhysicsType");
currentEnum->items.emplace_back("null", (int)ObjectPhysicsType::null);
currentEnum->items.emplace_back("static_r", (int)ObjectPhysicsType::static_r);
currentEnum->items.emplace_back("dynamic", (int)ObjectPhysicsType::dynamic);
currentEnum->items.emplace_back("controller", (int)ObjectPhysicsType::controller);
currentBank = Object::b;
currentBank->parents.push_back(Transformer::b);
currentBank->addV<std::string>("model_filename", offsetof(Object, model_filename));
currentBank->addE("ObjectPhysicsType", "physics_type", offsetof(Object, physics_type));
currentEnum = tke::addReflectEnum("TerrainType");
currentEnum->items.emplace_back("height_map", (int)TerrainType::height_map);
currentEnum->items.emplace_back("procedural", (int)TerrainType::procedural);
currentBank = Terrain::b;
currentBank->parents.push_back(Transformer::b);
currentBank->addE("TerrainType", "type", offsetof(Terrain, type));
currentBank->addV<bool>("use_physx", offsetof(Terrain, use_physx));
currentBank->addV<std::string>("height_map_filename", offsetof(Terrain, height_map_filename));
currentBank->addV<std::string>("color_map_filename", offsetof(Terrain, color_map_filename));
currentBank = Scene::b;
currentBank->addV<std::string>("name", offsetof(Scene, name));
}};static ReflectInit init;
}