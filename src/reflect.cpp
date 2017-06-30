#include "utils.h"
#include "render.h"
#include <string>
namespace tke{
tke::ReflectionBank *PushConstantRange::b = tke::addReflectionBank("PushConstantRange");
tke::ReflectionBank *BlendAttachment::b = tke::addReflectionBank("BlendAttachment");
tke::ReflectionBank *Descriptor::b = tke::addReflectionBank("Descriptor");
tke::ReflectionBank *UniformBufferInfo::b = tke::addReflectionBank("UniformBufferInfo");
tke::ReflectionBank *ImageInfo::b = tke::addReflectionBank("ImageInfo");
tke::ReflectionBank *DynamicState::b = tke::addReflectionBank("DynamicState");
tke::ReflectionBank *PipelineInfo::b = tke::addReflectionBank("PipelineInfo");
tke::ReflectionBank *ShaderMacro::b = tke::addReflectionBank("ShaderMacro");
tke::ReflectionBank *Stage::b = tke::addReflectionBank("Stage");
tke::ReflectionBank *LinkResource::b = tke::addReflectionBank("LinkResource");
tke::ReflectionBank *Pipeline::b = tke::addReflectionBank("Pipeline");
tke::ReflectionBank *Drawcall::b = tke::addReflectionBank("Drawcall");
tke::ReflectionBank *DrawAction::b = tke::addReflectionBank("DrawAction");
tke::ReflectionBank *Attachment::b = tke::addReflectionBank("Attachment");
tke::ReflectionBank *Dependency::b = tke::addReflectionBank("Dependency");
tke::ReflectionBank *RenderPass::b = tke::addReflectionBank("RenderPass");
tke::ReflectionBank *Renderer::b = tke::addReflectionBank("Renderer");
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
currentEnum = tke::addReflectEnum("RenderPassType");
currentEnum->items.emplace_back("draw_action", (int)RenderPassType::draw_action);
currentEnum->items.emplace_back("call_secondary_cmd", (int)RenderPassType::call_secondary_cmd);
currentEnum = tke::addReflectEnum("DrawActionType");
currentEnum->items.emplace_back("draw_action", (int)DrawActionType::draw_action);
currentEnum->items.emplace_back("call_fuction", (int)DrawActionType::call_fuction);
currentEnum = tke::addReflectEnum("DrawcallType");
currentEnum->items.emplace_back("vertex", (int)DrawcallType::vertex);
currentEnum->items.emplace_back("index", (int)DrawcallType::index);
currentEnum->items.emplace_back("indirect_vertex", (int)DrawcallType::indirect_vertex);
currentEnum->items.emplace_back("indirect_index", (int)DrawcallType::indirect_index);
currentEnum->items.emplace_back("push_constant", (int)DrawcallType::push_constant);
currentEnum = tke::addReflectEnum("PushConstantType");
currentEnum->items.emplace_back("int_t", (int)PushConstantType::int_t);
currentEnum->items.emplace_back("float_t", (int)PushConstantType::float_t);
currentEnum->items.emplace_back("vec2_t", (int)PushConstantType::vec2_t);
currentEnum->items.emplace_back("vec3_t", (int)PushConstantType::vec3_t);
currentEnum->items.emplace_back("vec4_t", (int)PushConstantType::vec4_t);
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
currentEnum->items.emplace_back("storage_buffer", (int)DescriptorType::storage_buffer);
currentEnum->items.emplace_back("storage_image", (int)DescriptorType::storage_image);
currentEnum->items.emplace_back("image_n_sampler", (int)DescriptorType::image_n_sampler);
currentEnum->items.emplace_back("input_attachment", (int)DescriptorType::input_attachment);
currentBank = Descriptor::b;
currentBank->addE("DescriptorType", "type", offsetof(Descriptor, type));
currentBank->addV<int>("binding", offsetof(Descriptor, binding));
currentBank->addV<int>("count", offsetof(Descriptor, count));
currentBank->addV<std::string>("name", offsetof(Descriptor, name));
currentBank = UniformBufferInfo::b;
currentBank->addV<std::string>("name", offsetof(UniformBufferInfo, name));
currentBank->addV<int>("size", offsetof(UniformBufferInfo, size));
currentBank = ImageInfo::b;
currentBank->addV<std::string>("name", offsetof(ImageInfo, name));
currentBank->addV<std::string>("file_name", offsetof(ImageInfo, file_name));
currentBank->addV<bool>("sRGB", offsetof(ImageInfo, sRGB));
currentBank->addV<int>("cx", offsetof(ImageInfo, cx));
currentBank->addV<int>("cy", offsetof(ImageInfo, cy));
currentBank->addE("Format", "format", offsetof(ImageInfo, format));
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
currentBank = PipelineInfo::b;
currentBank->addV<std::string>("file_name", offsetof(PipelineInfo, file_name));
currentBank = ShaderMacro::b;
currentBank->addV<std::string>("pipeline_name", offsetof(ShaderMacro, pipeline_name));
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
currentBank = Drawcall::b;
currentBank->addV<std::string>("name", offsetof(Drawcall, name));
currentBank->addV<int>("instance_count", offsetof(Drawcall, instance_count));
currentBank->addV<int>("first_instance", offsetof(Drawcall, first_instance));
currentBank->addE("DrawcallType", "type", offsetof(Drawcall, type));
currentBank->addV<int>("vertex_count", offsetof(Drawcall, vertex_count));
currentBank->addV<int>("first_vertex", offsetof(Drawcall, first_vertex));
currentBank->addV<int>("index_count", offsetof(Drawcall, index_count));
currentBank->addV<int>("first_index", offsetof(Drawcall, first_index));
currentBank->addV<int>("vertex_offset", offsetof(Drawcall, vertex_offset));
currentBank->addV<std::string>("model_name", offsetof(Drawcall, model_name));
currentBank->addV<std::string>("indirect_vertex_buffer_name", offsetof(Drawcall, indirect_vertex_buffer_name));
currentBank->addV<std::string>("indirect_index_buffer_name", offsetof(Drawcall, indirect_index_buffer_name));
currentBank->addV<int>("first_indirect", offsetof(Drawcall, first_indirect));
currentBank->addV<int>("indirect_count", offsetof(Drawcall, indirect_count));
currentBank->addV<std::string>("indirect_count_name", offsetof(Drawcall, indirect_count_name));
currentBank->addV<StageType>("push_constant_stage", offsetof(Drawcall, push_constant_stage));
currentBank->addV<int>("push_constant_offset", offsetof(Drawcall, push_constant_offset));
currentBank->addE("PushConstantType", "push_constant_type", offsetof(Drawcall, push_constant_type));
currentBank = DrawAction::b;
currentBank->addV<std::string>("name", offsetof(DrawAction, name));
currentBank->addE("DrawActionType", "type", offsetof(DrawAction, type));
currentBank->addV<std::string>("vertex_buffer_name", offsetof(DrawAction, vertex_buffer_name));
currentBank->addV<std::string>("index_buffer_name", offsetof(DrawAction, index_buffer_name));
currentBank->addV<std::string>("pipeline_name", offsetof(DrawAction, pipeline_name));
currentBank->addV<std::string>("render_func_name", offsetof(DrawAction, render_func_name));
currentBank->addV<int>("cx", offsetof(DrawAction, cx));
currentBank->addV<int>("cy", offsetof(DrawAction, cy));
currentBank = Attachment::b;
currentBank->addV<std::string>("name", offsetof(Attachment, name));
currentBank->addV<std::string>("image_name", offsetof(Attachment, image_name));
currentBank->addV<int>("level", offsetof(Attachment, level));
currentBank->addV<int>("layer", offsetof(Attachment, layer));
currentBank->addE("AspectFlags", "aspect", offsetof(Attachment, aspect));
currentBank->addV<bool>("clear", offsetof(Attachment, clear));
currentBank->addV<float>("clear_r", offsetof(Attachment, clear_r));
currentBank->addV<float>("clear_g", offsetof(Attachment, clear_g));
currentBank->addV<float>("clear_b", offsetof(Attachment, clear_b));
currentBank->addV<float>("clear_a", offsetof(Attachment, clear_a));
currentBank->addV<float>("clear_depth", offsetof(Attachment, clear_depth));
currentBank->addV<int>("clear_stencil", offsetof(Attachment, clear_stencil));
currentBank = Dependency::b;
currentBank->addV<std::string>("pass_name", offsetof(Dependency, pass_name));
currentBank = RenderPass::b;
currentBank->addE("RenderPassType", "type", offsetof(RenderPass, type));
currentBank->addV<std::string>("name", offsetof(RenderPass, name));
currentBank->addV<std::string>("secondary_cmd_name", offsetof(RenderPass, secondary_cmd_name));
currentBank = Renderer::b;
currentBank->addV<std::string>("name", offsetof(Renderer, name));
currentBank->addV<int>("cx", offsetof(Renderer, cx));
currentBank->addV<int>("cy", offsetof(Renderer, cy));
currentBank->addV<std::string>("vertex_buffer_name", offsetof(Renderer, vertex_buffer_name));
currentBank->addV<std::string>("index_buffer_name", offsetof(Renderer, index_buffer_name));
}};static ReflectInit init;
}