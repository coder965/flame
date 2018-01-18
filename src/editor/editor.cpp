#include "../ui/ui.h"
#include "../input.h"
#include "../global.h"
#include "../engine.h"
#include "window/file_selector.h"
#include "window/resource_explorer.h"
#include "window/hierarchy.h"
#include "window/inspector.h"
#include "window/scene_editor.h"
#include "window/image_editor.h"

struct NewImageDialog : FileSelector
{
	bool first = true;
	int cx = 512;
	int cy = 512;

	NewImageDialog() :
		FileSelector("New Image", true, true, true, 1, 800, 600)
	{
		callback = [this](std::string s) {
			if (std::experimental::filesystem::exists(s))
				return false;
			tke::newImageFile(s, cx, cy, 32);
			return true;
		};
	}

	virtual void on_right_area_show() override
	{
		ImGui::PushItemWidth(200);
		ImGui::DragInt("cx", &cx);
		ImGui::DragInt("cy", &cy);
		const char *typeNames[] = {
			"color R8G8B8A8"
		};
		static int type = 0;
		ImGui::Combo("type", &type, typeNames, TK_ARRAYSIZE(typeNames));
		ImGui::PopItemWidth();
	}
};

int main(int argc, char** argv)
{
	tke::init(true, "../", 1920, 1080, 1280, 720, "TK Engine Editor", tke::WindowStyleFrame | tke::WindowStyleResize, false);

	ShowWindow((HWND)tke::hWnd, SW_SHOWMAXIMIZED);

	{
		tke::XMLDoc at("data", "ui.xml");
		if (at.good)
		{
			for (auto &n : at.children)
			{
				if (n->name == "resource_explorer")
					resourceExplorer = new ResourceExplorer;
				else if (n->name == "hierarchy_window")
					hierarchy_window = new HierarchyWindow;
				else if (n->name == "inspector_window")
					inspector_window = new InspectorWindow;
			}
		}
	}

	tke::onRender = []() {
		tke::begin_frame(true);

		bool show_device_Props = false;
		if (ImGui::last_frame_main_menu_alive || tke::mouseY <= ImGui::GetFrameHeight())
		{
			ImGui::BeginMainMenuBar();
			if (ImGui::BeginMenu_keepalive("File"))
			{
				if (ImGui::BeginMenu("New"))
				{
					if (ImGui::MenuItem("Scene"))
						;
					if (ImGui::MenuItem("Image"))
						new NewImageDialog;

					ImGui::EndMenu();
				}

				if (scene_editor)
					scene_editor->on_file_menu();

				ImGui::EndMenu();
			}
			if (scene_editor)
				scene_editor->on_menu_bar();
			if (ImGui::BeginMenu_keepalive("View"))
			{
				static bool fullscreen = false;
				if (ImGui::MenuItem("Fullscreen", "", &fullscreen))
				{
					if (fullscreen)
					{
						tke::window_style |= tke::WindowStyleFullscreen;
						tke::window_style &= (~tke::WindowStyleFrame);
					}
					else
					{
						tke::window_style |= tke::WindowStyleFrame;
						tke::window_style &= (~tke::WindowStyleFullscreen);
						tke::window_cx = tke::res_cx;
						tke::window_cy = tke::res_cy;
					}
					auto wndProp = tke::getWin32WndProp();
					SetWindowLong((HWND)tke::hWnd, GWL_STYLE, wndProp.second);
					SetWindowPos((HWND)tke::hWnd, HWND_TOP, 0, 0, wndProp.first.x, wndProp.first.y, SWP_NOZORDER);
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Resource Explorer"))
				{
					if (!resourceExplorer)
						resourceExplorer = new ResourceExplorer;
					resourceExplorer->_need_focus = true;
				}
				if (ImGui::MenuItem("Hierarchy"))
				{
					if (!hierarchy_window)
						hierarchy_window = new HierarchyWindow;
					hierarchy_window->_need_focus = true;
				}
				if (ImGui::MenuItem("Inspector"))
				{
					if (!inspector_window)
						inspector_window = new InspectorWindow;
					inspector_window->_need_focus = true;
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu_keepalive("Help"))
			{
				if (ImGui::MenuItem("Device Properties"))
					show_device_Props = true;

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (show_device_Props)
		{
			ImGui::OpenPopup("Device Properties");
			ImGui::SetNextWindowSize(ImVec2(400, 300));
		}
		if (ImGui::BeginPopupModal("Device Properties"))
		{
			ImGui::BeginChild("##items", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 1), true);

			static char filter[260];
			ImGui::InputText("filter", filter, 260);

			auto fText = [](const char* fmt, ...) {
				va_list args;
				va_start(args, fmt);
				ImGui::TextVFilted(fmt, filter, args);
				va_end(args);
			};

			if (ImGui::TreeNode("Physical Device Properties"))
			{
				auto &p = tke::vk_physical_device_properties;
				fText("api version: %d", p.apiVersion);
				fText("driver version: %d", p.driverVersion);
				fText("vendor ID: %d", p.vendorID);
				fText("device ID: %d", p.deviceID);
				fText("device type: %s", tke::vk_device_type_names[p.deviceType]);
				fText("device name: %s", p.deviceName);

				if (ImGui::TreeNode("Limits"))
				{
					auto &l = p.limits;
					fText("max image dimension 1D: %d", l.maxImageDimension1D);
					fText("max image dimension 2D: %d", l.maxImageDimension2D);
					fText("max image dimension 3D: %d", l.maxImageDimension3D);
					fText("max image dimension cube: %d", l.maxImageDimensionCube);
					fText("max image array layers: %d", l.maxImageArrayLayers);
					fText("max texel buffer range: %d", l.maxTexelBufferElements);
					fText("max uniform buffer range: %d", l.maxUniformBufferRange);
					fText("max storage buffer range: %d", l.maxStorageBufferRange);
					fText("max push constants size: %d", l.maxPushConstantsSize);
					fText("max memory allocation count: %d", l.maxMemoryAllocationCount);
					fText("max sampler allocation count: %d", l.maxSamplerAllocationCount);
					fText("buffer image granularity: %lld", l.bufferImageGranularity);
					fText("sparse address space size: %lld", l.sparseAddressSpaceSize);
					fText("max bound descriptor sets: %d", l.maxBoundDescriptorSets);
					fText("max per stage descriptor samplers: %d", l.maxPerStageDescriptorSamplers);
					fText("max per stage descriptor uniform buffers: %d", l.maxPerStageDescriptorUniformBuffers);
					fText("max per stage descriptor storage buffers: %d", l.maxPerStageDescriptorStorageBuffers);
					fText("max per stage descriptor sampled images: %d", l.maxPerStageDescriptorSampledImages);
					fText("max per stage descriptor storage images: %d", l.maxPerStageDescriptorStorageImages);
					fText("max per stage descriptor input attachments: %d", l.maxPerStageDescriptorInputAttachments);
					fText("max per stage resources: %d", l.maxPerStageResources);
					fText("max descriptor set samplers: %d", l.maxDescriptorSetSamplers);
					fText("max descriptor set uniform buffers: %d", l.maxDescriptorSetUniformBuffers);
					fText("max descriptor set uniform buffers dynamic: %d", l.maxDescriptorSetUniformBuffersDynamic);
					fText("max descriptor set storage buffers: %d", l.maxDescriptorSetStorageBuffers);
					fText("max descriptor set storage buffers dynamic: %d", l.maxDescriptorSetStorageBuffersDynamic);
					fText("max descriptor set sampled images: %d", l.maxDescriptorSetSampledImages);
					fText("max descriptor set storage images: %d", l.maxDescriptorSetStorageImages);
					fText("max descriptor set input attachments: %d", l.maxDescriptorSetInputAttachments);
					fText("max vertex input attributes: %d", l.maxVertexInputAttributes);
					fText("max vertex input bindings: %d", l.maxVertexInputBindings);
					fText("max vertex input attribute offset: %d", l.maxVertexInputAttributeOffset);
					fText("max vertex input binding stride: %d", l.maxVertexInputBindingStride);
					fText("max vertex output components: %d", l.maxVertexOutputComponents);
					fText("max tesellation generation level: %d", l.maxTessellationGenerationLevel);
					fText("max tesellation patch size: %d", l.maxTessellationPatchSize);
					fText("max tesellation control per vertex input components: %d", l.maxTessellationControlPerVertexInputComponents);
					fText("max tesellation control per vertex output components: %d", l.maxTessellationControlPerVertexOutputComponents);
					fText("max tesellation control per patch output components: %d", l.maxTessellationControlPerPatchOutputComponents);
					fText("max tesellation control per total output components: %d", l.maxTessellationControlTotalOutputComponents);
					fText("max tesellation evaluation input components: %d", l.maxTessellationEvaluationInputComponents);
					fText("max tesellation evaluation output components: %d", l.maxTessellationEvaluationOutputComponents);
					fText("max geometry shader invocations: %d", l.maxGeometryShaderInvocations);
					fText("max geometry input components: %d", l.maxGeometryInputComponents);
					fText("max geometry output components: %d", l.maxGeometryOutputComponents);
					fText("max geometry output vertices: %d", l.maxGeometryOutputVertices);
					fText("max geometry total output components: %d", l.maxGeometryTotalOutputComponents);
					fText("max fragment input components: %d", l.maxFragmentInputComponents);
					fText("max fragment output attachments: %d", l.maxFragmentOutputAttachments);
					fText("max fragment dual src attachments: %d", l.maxFragmentDualSrcAttachments);
					fText("max fragment combine output resources: %d", l.maxFragmentCombinedOutputResources);
					fText("max compute shader memory size: %d", l.maxComputeSharedMemorySize);
					fText("max compute work group count: %d, %d, %d", l.maxComputeWorkGroupCount[0], l.maxComputeWorkGroupCount[1], l.maxComputeWorkGroupCount[2]);
					fText("max compute work group invocations: %d", l.maxComputeWorkGroupInvocations);
					fText("max compute work group size: %d, %d, %d", l.maxComputeWorkGroupSize[0], l.maxComputeWorkGroupSize[1], l.maxComputeWorkGroupSize[2]);
					fText("sub pixel precision bits: %d", l.subPixelPrecisionBits);
					fText("sub texel precision bits: %d", l.subTexelPrecisionBits);
					fText("mipmap precision bits: %d", l.mipmapPrecisionBits);
					fText("max draw indexed index value: %d", l.maxDrawIndexedIndexValue);
					fText("max draw indirect count: %d", l.maxDrawIndirectCount);
					fText("max sampler lod bias: %f", l.maxSamplerLodBias);
					fText("max sampler anisotropy: %f", l.maxSamplerAnisotropy);
					fText("max viewports: %d", l.maxViewports);
					fText("max viewport dimensions: %d, %d", l.maxViewportDimensions[0], l.maxViewportDimensions[1]);
					fText("viewport bounds range: %f, %f", l.viewportBoundsRange[0], l.viewportBoundsRange[1]);
					fText("viewport sub pixel bits: %d", l.viewportSubPixelBits);
					fText("min memory map alignment: %lld", l.minMemoryMapAlignment);
					fText("min texel buffer offset alignment: %lld", l.minTexelBufferOffsetAlignment);
					fText("min uniform buffer offset alignment: %lld", l.minUniformBufferOffsetAlignment);
					fText("min storage buffer offset alignment: %lld", l.minStorageBufferOffsetAlignment);
					fText("min texel offset: %d", l.minTexelOffset);
					fText("max texel offset: %d", l.maxTexelOffset);
					fText("min texel gather offset: %d", l.minTexelGatherOffset);
					fText("max texel gather offset: %d", l.maxTexelGatherOffset);
					fText("min interpolation offset: %f", l.minInterpolationOffset);
					fText("max interpolation offset: %f", l.maxInterpolationOffset);
					fText("sun pixel interpolation offset bits: %d", l.subPixelInterpolationOffsetBits);
					fText("max framebuffer width: %d", l.maxFramebufferWidth);
					fText("max framebuffer height: %d", l.maxFramebufferHeight);
					fText("max framebuffer layers: %d", l.maxFramebufferLayers);
					fText("framebuffer color sample counts: %d", l.framebufferColorSampleCounts);
					fText("framebuffer depth sample counts: %d", l.framebufferDepthSampleCounts);
					fText("framebuffer stencil sample counts: %d", l.framebufferStencilSampleCounts);
					fText("framebuffer no attachments sample counts: %d", l.framebufferNoAttachmentsSampleCounts);
					fText("max color attachments: %d", l.maxColorAttachments);
					fText("sampled image color sample counts: %d", l.sampledImageColorSampleCounts);
					fText("sampled image integer sample counts: %d", l.sampledImageIntegerSampleCounts);
					fText("sampled image depth sample counts: %d", l.sampledImageDepthSampleCounts);
					fText("sampled image stencil sample counts: %d", l.sampledImageStencilSampleCounts);
					fText("storage image sample counts: %d", l.storageImageSampleCounts);
					fText("max sample mask words: %d", l.maxSampleMaskWords);
					fText("timestamp compute and graphics: %s", l.timestampComputeAndGraphics ? "true" : "false");
					fText("timestamp period: %f", l.timestampPeriod);
					fText("max clip distances: %d", l.maxClipDistances);
					fText("max cull distances: %d", l.maxCullDistances);
					fText("max combined clip and cull distances: %d", l.maxCombinedClipAndCullDistances);
					fText("discrete queue priorities: %d", l.discreteQueuePriorities);
					fText("point size range: %f, %f", l.pointSizeRange[0], l.pointSizeRange[1]);
					fText("line width range: %f, %f", l.lineWidthRange[0], l.lineWidthRange[1]);
					fText("point size granularity: %f", l.pointSizeGranularity);
					fText("line width granularity: %f", l.lineWidthGranularity);
					fText("strict lines: %s", l.strictLines ? "true" : "false");
					fText("standard sample locations: %s", l.standardSampleLocations ? "true" : "false");
					fText("optimal buffer copy offset alignment: %lld", l.optimalBufferCopyOffsetAlignment);
					fText("optimal buffer copy row pitch alignment: %lld", l.optimalBufferCopyRowPitchAlignment);
					fText("non coherent atom size: %lld", l.nonCoherentAtomSize);

					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Sparse Properties"))
				{
					auto &s = p.sparseProperties;
					fText("residency standard 2D block shape: %s", s.residencyStandard2DBlockShape ? "true" : "false");
					fText("residency standard 2D multisample block shape: %s", s.residencyStandard2DMultisampleBlockShape ? "true" : "false");
					fText("residency standard 3D block shape: %s", s.residencyStandard3DBlockShape ? "true" : "false");
					fText("residency aligned mip size: %s", s.residencyAlignedMipSize ? "true" : "false");
					fText("residency non resident strict: %s", s.residencyNonResidentStrict ? "true" : "false");

					ImGui::TreePop();
				}

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Physical Device Features"))
			{
				auto &f = tke::vk_physical_device_features;
				fText("robust buffer access: %s", f.robustBufferAccess ? "true" : "false");
				fText("full draw index uint32: %s", f.fullDrawIndexUint32 ? "true" : "false");
				fText("image cube array: %s", f.imageCubeArray ? "true" : "false");
				fText("independent blend: %s", f.independentBlend ? "true" : "false");
				fText("geometry shader: %s", f.geometryShader ? "true" : "false");
				fText("tessellation shader: %s", f.tessellationShader ? "true" : "false");
				fText("sample rate shading: %s", f.sampleRateShading ? "true" : "false");
				fText("dua src blend: %s", f.dualSrcBlend ? "true" : "false");
				fText("logic op: %s", f.logicOp ? "true" : "false");
				fText("multi draw indirect: %s", f.multiDrawIndirect ? "true" : "false");
				fText("draw indirect first instance: %s", f.drawIndirectFirstInstance ? "true" : "false");
				fText("depth clamp: %s", f.depthClamp ? "true" : "false");
				fText("depth bias clamp: %s", f.depthBiasClamp ? "true" : "false");
				fText("fill mode non solid: %s", f.fillModeNonSolid ? "true" : "false");
				fText("depth bounds: %s", f.depthBounds ? "true" : "false");
				fText("wide lines: %s", f.wideLines ? "true" : "false");
				fText("large points: %s", f.largePoints ? "true" : "false");
				fText("alpha to one: %s", f.alphaToOne ? "true" : "false");
				fText("multi viewport: %s", f.multiViewport ? "true" : "false");
				fText("sampler anisotropy: %s", f.samplerAnisotropy ? "true" : "false");
				fText("texture compression ETC2: %s", f.textureCompressionETC2 ? "true" : "false");
				fText("texture compression ASTC_LDR: %s", f.textureCompressionASTC_LDR ? "true" : "false");
				fText("texture compression BC: %s", f.textureCompressionBC ? "true" : "false");
				fText("occlusion query precise: %s", f.occlusionQueryPrecise ? "true" : "false");
				fText("pipeline statistics query: %s", f.pipelineStatisticsQuery ? "true" : "false");
				fText("vertex pipeline stores and atomics: %s", f.vertexPipelineStoresAndAtomics ? "true" : "false");
				fText("fragment stores and atomics: %s", f.fragmentStoresAndAtomics ? "true" : "false");
				fText("shader tessellation and geometry point size: %s", f.shaderTessellationAndGeometryPointSize ? "true" : "false");
				fText("shader image gather extended: %s", f.shaderImageGatherExtended ? "true" : "false");
				fText("shader storage image extended formats: %s", f.shaderStorageImageExtendedFormats ? "true" : "false");

				ImGui::TreePop();
			}
			ImGui::EndChild();

			if (ImGui::Button("Ok"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		if (scene_editor)
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(tke::window_cx, tke::window_cy));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
			if (ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing | 
				ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings))
				scene_editor->do_show();
			ImGui::End();
			ImGui::PopStyleVar(2);
		}

		{
			std::vector<IWindow*> _w;
			for (auto &w : windows)
				_w.push_back(w.get());
			for (auto &w : _w)
				w->show();
		}

		ImGui::SetNextWindowPos(ImVec2(0, tke::window_cy - ImGui::GetFrameHeightWithSpacing()));
		ImGui::Begin("status", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("FPS:%d", tke::FPS);
		ImGui::End();

		tke::end_frame();

		for (auto it = windows.begin(); it != windows.end(); )
		{
			if (!(*it)->opened)
				it = windows.erase(it);
			else
				it++;
		}
	};
	
	tke::onDestroy = []() {
		tke::XMLDoc at("data");
		if (resourceExplorer)
			at.newNode("resource_explorer");
		if (hierarchy_window)
			at.newNode("hierarchy_window");
		if (inspector_window)
			at.newNode("inspector_window");
		if (SelectObject)
			at.newNode("select");
		at.save("ui.xml");
	};

	tke::run();

	return 0;
}
