#include <flame/engine/ui/imageviewer.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/command_buffer.h>
#include <flame/engine/ui/fileselector.h>

namespace flame
{
	namespace ui
	{
		ImageViewer::ImageViewer(const std::string &_title, std::shared_ptr<Texture> _texture) :
			Window(_title, WindowHasMenu | WindowNoSavedSettings),
			texture(_texture)
		{
			first_cx = 800;
			first_cy = 600;

			increase_texture_ref(texture.get());

			staging_buffer = std::make_unique<Buffer>(BufferTypeStaging, texture->total_size);

			{
				VkBufferImageCopy r = {};
				r.imageExtent.width = texture->get_cx();
				r.imageExtent.height = texture->get_cy();
				r.imageExtent.depth = 1;
				r.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				r.imageSubresource.layerCount = 1;

				auto cb = begin_once_command_buffer();
				texture->transition_layout(cb, texture->layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				vkCmdCopyImageToBuffer(cb->v, texture->v, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging_buffer->v, 1, &r);
				texture->transition_layout(cb, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->layout);
				end_once_command_buffer(cb);
			}
		}

		ImageViewer::~ImageViewer()
		{
			decrease_texture_ref(texture.get());
		}

		void ImageViewer::on_show()
		{
			ImGui::BeginMenuBar();
			if (ImGui::BeginMenu("File"))
			{
				auto do_save = false;
				auto need_filename_popup = false;
				if (ImGui::MenuItem("Save"))
				{
					do_save = true;
					if (texture->filename == "")
						need_filename_popup = true;
				}
				if (ImGui::MenuItem("Save As"))
				{
					do_save = true;
					need_filename_popup = true;
				}
				if (do_save)
				{
					auto fun_save = [&](const std::string &filename) {
						staging_buffer->map();
						save_image(texture->get_cx(), texture->get_cy(), texture->channel, texture->bpp, (unsigned char*)staging_buffer->mapped, filename);
						staging_buffer->unmap();
						return true;
					};
					if (need_filename_popup)
					{
						auto dialog = new FileSelector("Save Image", FileSelectorSave, "", WindowModal | WindowNoSavedSettings);
						dialog->first_cx = 800;
						dialog->first_cy = 600;
						dialog->callback = fun_save;
					}
					else
						fun_save(texture->filename);
				}
				ImGui::EndMenu();
			}
			on_menu_bar();
			ImGui::EndMenuBar();

			on_top_area();

			auto image_pos = ImGui::GetCursorScreenPos();
			auto image_size = ImVec2(texture->get_cx(), texture->get_cy());
			ImGui::InvisibleButton("canvas", image_size);
			auto draw_list = ImGui::GetWindowDrawList();
			draw_list->AddImage(texture.get(), image_pos, image_pos + image_size);
			if (ImGui::IsItemHovered())
				on_mouse_overing_image(image_pos);
		}
	}
}
