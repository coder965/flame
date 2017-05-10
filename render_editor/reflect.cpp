#include "../src/core/render.abstract.h"
 #include "../src/core/utils.h"
 #include "../render_editor/render.h"
 #define PipelineAbstract PipelineAbstract<StageAbstract>
 #define DrawActionAbstract DrawActionAbstract<Drawcall>
 #define RenderPassAbstract RenderPassAbstract<Attachment, Dependency, DrawAction>
 #define RendererAbstract RendererAbstract<RenderPass>
#include <string>
namespace tke{
tke::ReflectionBank *BlendAttachment::b = tke::addReflectionBank("BlendAttachment");
tke::ReflectionBank *Descriptor::b = tke::addReflectionBank("Descriptor");
tke::ReflectionBank *PushConstantRange::b = tke::addReflectionBank("PushConstantRange");
tke::ReflectionBank *StageAbstract::b = tke::addReflectionBank("StageAbstract");
tke::ReflectionBank *LinkResource::b = tke::addReflectionBank("LinkResource");
tke::ReflectionBank *PipelineAbstract::b = tke::addReflectionBank("PipelineAbstract");
tke::ReflectionBank *DrawcallAbstract::b = tke::addReflectionBank("DrawcallAbstract");
tke::ReflectionBank *DrawActionAbstract::b = tke::addReflectionBank("DrawActionAbstract");
tke::ReflectionBank *AttachmentAbstract::b = tke::addReflectionBank("AttachmentAbstract");
tke::ReflectionBank *DependencyAbstract::b = tke::addReflectionBank("DependencyAbstract");
tke::ReflectionBank *RenderPassAbstract::b = tke::addReflectionBank("RenderPassAbstract");
tke::ReflectionBank *RendererAbstract::b = tke::addReflectionBank("RendererAbstract");
struct ReflectInit{ReflectInit(){
tke::Enum *currentEnum = nullptr;
tke::ReflectionBank *currentBank = nullptr;
currentEnum = tke::addReflectEnum("StageFlags");
currentEnum->items.emplace_back("vert", (int)StageFlags::vert);
currentEnum->items.emplace_back("tesc", (int)StageFlags::tesc);
currentEnum->items.emplace_back("tese", (int)StageFlags::tese);
currentEnum->items.emplace_back("geom", (int)StageFlags::geom);
currentEnum->items.emplace_back("frag", (int)StageFlags::frag);
currentEnum = tke::addReflectEnum("AspectFlags");
currentEnum->items.emplace_back("color", (int)AspectFlags::color);
currentEnum->items.emplace_back("depth", (int)AspectFlags::depth);
currentEnum->items.emplace_back("stencil", (int)AspectFlags::stencil);
currentEnum->items.emplace_back("depth_stencil", (int)AspectFlags::depth_stencil);
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
currentEnum->items.emplace_back("sampler", (int)DescriptorType::sampler);
currentEnum->items.emplace_back("input_attachment", (int)DescriptorType::input_attachment);
currentBank = Descriptor::b;
currentBank->addV<std::string>("name", offsetof(Descriptor, name));
currentBank->addE("DescriptorType", "type", offsetof(Descriptor, type));
currentBank->addV<int>("binding", offsetof(Descriptor, binding));
currentBank->addV<int>("count", offsetof(Descriptor, count));
currentBank = PushConstantRange::b;
currentBank->addV<int>("offset", offsetof(PushConstantRange, offset));
currentBank->addV<int>("size", offsetof(PushConstantRange, size));
currentBank = StageAbstract::b;
currentBank->addV<std::string>("filename", offsetof(StageAbstract, filename));
currentEnum = tke::addReflectEnum("SamplerType");
currentEnum->items.emplace_back("none", (int)SamplerType::none);
currentEnum->items.emplace_back("plain", (int)SamplerType::plain);
currentEnum->items.emplace_back("plain_unnormalized", (int)SamplerType::plain_unnormalized);
currentEnum->items.emplace_back("color", (int)SamplerType::color);
currentEnum->items.emplace_back("color_border", (int)SamplerType::color_border);
currentBank = LinkResource::b;
currentBank->addE("DescriptorType", "type", offsetof(LinkResource, type));
currentBank->addV<int>("binding", offsetof(LinkResource, binding));
currentBank->addV<int>("array_element", offsetof(LinkResource, array_element));
currentBank->addV<std::string>("name", offsetof(LinkResource, name));
currentBank->addE("SamplerType", "sampler", offsetof(LinkResource, sampler));
currentBank = PipelineAbstract::b;
currentBank->addV<std::string>("name", offsetof(PipelineAbstract, name));
currentBank->addV<int>("cx", offsetof(PipelineAbstract, cx));
currentBank->addV<int>("cy", offsetof(PipelineAbstract, cy));
currentBank->addV<int>("patch_control_points", offsetof(PipelineAbstract, patch_control_points));
currentBank->addV<bool>("depth_test", offsetof(PipelineAbstract, depth_test));
currentBank->addV<bool>("depth_write", offsetof(PipelineAbstract, depth_write));
currentBank->addV<bool>("depth_clamp", offsetof(PipelineAbstract, depth_clamp));
currentBank->addE("PrimitiveTopology", "primitive_topology", offsetof(PipelineAbstract, primitive_topology));
currentBank->addE("PolygonMode", "polygon_mode", offsetof(PipelineAbstract, polygon_mode));
currentBank->addE("CullMode", "cull_mode", offsetof(PipelineAbstract, cull_mode));
currentBank = DrawcallAbstract::b;
currentBank->addV<std::string>("name", offsetof(DrawcallAbstract, name));
currentBank->addV<int>("instance_count", offsetof(DrawcallAbstract, instance_count));
currentBank->addV<int>("first_instance", offsetof(DrawcallAbstract, first_instance));
currentBank->addE("DrawcallType", "type", offsetof(DrawcallAbstract, type));
currentBank->addV<int>("vertex_count", offsetof(DrawcallAbstract, vertex_count));
currentBank->addV<int>("first_vertex", offsetof(DrawcallAbstract, first_vertex));
currentBank->addV<int>("index_count", offsetof(DrawcallAbstract, index_count));
currentBank->addV<int>("first_index", offsetof(DrawcallAbstract, first_index));
currentBank->addV<int>("vertex_offset", offsetof(DrawcallAbstract, vertex_offset));
currentBank->addV<std::string>("indirect_vertex_buffer_name", offsetof(DrawcallAbstract, indirect_vertex_buffer_name));
currentBank->addV<std::string>("indirect_index_buffer_name", offsetof(DrawcallAbstract, indirect_index_buffer_name));
currentBank->addV<int>("first_indirect", offsetof(DrawcallAbstract, first_indirect));
currentBank->addV<int>("indirect_count", offsetof(DrawcallAbstract, indirect_count));
currentBank->addV<StageFlags>("push_constant_stage", offsetof(DrawcallAbstract, push_constant_stage));
currentBank->addV<int>("push_constant_offset", offsetof(DrawcallAbstract, push_constant_offset));
currentBank->addE("PushConstantType", "push_constant_type", offsetof(DrawcallAbstract, push_constant_type));
currentBank = DrawActionAbstract::b;
currentBank->addV<std::string>("name", offsetof(DrawActionAbstract, name));
currentBank->addE("DrawActionType", "type", offsetof(DrawActionAbstract, type));
currentBank->addV<std::string>("vertex_buffer_name", offsetof(DrawActionAbstract, vertex_buffer_name));
currentBank->addV<std::string>("index_buffer_name", offsetof(DrawActionAbstract, index_buffer_name));
currentBank->addV<std::string>("pipeline_name", offsetof(DrawActionAbstract, pipeline_name));
currentBank->addV<std::string>("renderable_name", offsetof(DrawActionAbstract, renderable_name));
currentBank->addV<int>("cx", offsetof(DrawActionAbstract, cx));
currentBank->addV<int>("cy", offsetof(DrawActionAbstract, cy));
currentBank = AttachmentAbstract::b;
currentBank->addV<std::string>("name", offsetof(AttachmentAbstract, name));
currentBank->addV<std::string>("image_name", offsetof(AttachmentAbstract, image_name));
currentBank->addV<int>("level", offsetof(AttachmentAbstract, level));
currentBank->addV<int>("layer", offsetof(AttachmentAbstract, layer));
currentBank->addE("AspectFlags", "aspect", offsetof(AttachmentAbstract, aspect));
currentBank->addV<bool>("clear", offsetof(AttachmentAbstract, clear));
currentBank->addV<float>("clear_r", offsetof(AttachmentAbstract, clear_r));
currentBank->addV<float>("clear_g", offsetof(AttachmentAbstract, clear_g));
currentBank->addV<float>("clear_b", offsetof(AttachmentAbstract, clear_b));
currentBank->addV<float>("clear_a", offsetof(AttachmentAbstract, clear_a));
currentBank->addV<float>("clear_depth", offsetof(AttachmentAbstract, clear_depth));
currentBank->addV<int>("clear_stencil", offsetof(AttachmentAbstract, clear_stencil));
currentBank = DependencyAbstract::b;
currentBank->addV<std::string>("pass_name", offsetof(DependencyAbstract, pass_name));
currentBank = RenderPassAbstract::b;
currentBank->addE("RenderPassType", "type", offsetof(RenderPassAbstract, type));
currentBank->addV<std::string>("name", offsetof(RenderPassAbstract, name));
currentBank->addV<std::string>("secondary_cmd_name", offsetof(RenderPassAbstract, secondary_cmd_name));
currentBank = RendererAbstract::b;
currentBank->addV<std::string>("name", offsetof(RendererAbstract, name));
currentBank->addV<int>("cx", offsetof(RendererAbstract, cx));
currentBank->addV<int>("cy", offsetof(RendererAbstract, cy));
}};static ReflectInit init;
}