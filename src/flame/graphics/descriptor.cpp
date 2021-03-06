//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "descriptor_private.h"
#include "device_private.h"
#include "pipeline_private.h"
#include "buffer_private.h"
#include "texture_private.h"
#include "sampler_private.h"

namespace flame
{
	namespace graphics
	{
#if defined(FLAME_GRAPHICS_VULKAN)
		void Descriptorsetlayout::clear_bindings()
		{
			_priv->bindings.clear();
		}

		void Descriptorsetlayout::add_binding(ShaderResourceType type, int binding, int count, ShaderType shader_type)
		{
			for (auto &b : _priv->bindings)
			{
				if (b.binding == binding)
				{
					assert(b.type == type);
					assert(b.count == count);
					b.shader_stage |= shader_type;
					return;
				}
			}

			DescriptorsetlayoutBinding b;
			b.type = type;
			b.binding = binding;
			b.count = count;
			b.shader_stage = shader_type;
			_priv->bindings.push_back(b);
		}

		void Descriptorsetlayout::build()
		{
			std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
			vk_bindings.resize(_priv->bindings.size());
			for (auto i = 0; i < vk_bindings.size(); i++)
			{
				vk_bindings[i].binding = _priv->bindings[i].binding;
				vk_bindings[i].descriptorType = Z(_priv->bindings[i].type);
				vk_bindings[i].descriptorCount = _priv->bindings[i].count;
				vk_bindings[i].stageFlags = Z(ShaderType(_priv->bindings[i].shader_stage));
				vk_bindings[i].pImmutableSamplers = nullptr;
			}

			VkDescriptorSetLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.bindingCount = vk_bindings.size();
			info.pBindings = vk_bindings.data();

			vk_chk_res(vkCreateDescriptorSetLayout(_priv->d->_priv->device,
				&info, nullptr, &_priv->v));
		}

		void Descriptorsetlayout::release()
		{
			if (_priv->v)
			{
				vkDestroyDescriptorSetLayout(_priv->d->_priv->device, _priv->v, nullptr);
				_priv->v = 0;
			}
		}

		Descriptorsetlayout *create_descriptorsetlayout(Device *d)
		{
			auto l = new Descriptorsetlayout;

			l->_priv = new DescriptorsetlayoutPrivate;
			l->_priv->d = d;
			l->_priv->v = 0;

			return l;
		}

		void destroy_descriptorsetlayout(Device *d, Descriptorsetlayout *l)
		{
			assert(d == l->_priv->d);

			l->release();
			
			delete l->_priv;
			delete l;
		}

		void Descriptorset::set_uniformbuffer(int binding, int index, Buffer *b, int offset, int range)
		{
			VkDescriptorBufferInfo i;
			i.buffer = b->_priv->v;
			i.offset = offset;
			i.range = range == 0 ? b->size : range;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = _priv->v;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.descriptorCount = 1;
			write.pBufferInfo = &i;
			write.pImageInfo = nullptr;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(_priv->d->_priv->device, 1, &write, 0, nullptr);
		}

		void Descriptorset::set_storagebuffer(int binding, int index, Buffer *b, int offset, int range)
		{
			VkDescriptorBufferInfo i;
			i.buffer = b->_priv->v;
			i.offset = offset;
			i.range = range == 0 ? b->size : range;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = _priv->v;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write.descriptorCount = 1;
			write.pBufferInfo = &i;
			write.pImageInfo = nullptr;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(_priv->d->_priv->device, 1, &write, 0, nullptr);
		}

		void Descriptorset::set_texture(int binding, int index, Textureview *v, Sampler *sampler)
		{
			VkDescriptorImageInfo i;
			i.imageView = v->_priv->v;
			i.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			i.sampler = sampler->_priv->v;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = _priv->v;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;
			write.pBufferInfo = nullptr;
			write.pImageInfo = &i;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(_priv->d->_priv->device, 1, &write, 0, nullptr);
		}

		void Descriptorset::set_storagetexture(int binding, int index, Textureview *v)
		{
			VkDescriptorImageInfo i;
			i.imageView = v->_priv->v;
			i.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			i.sampler = 0;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = _priv->v;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			write.descriptorCount = 1;
			write.pBufferInfo = nullptr;
			write.pImageInfo = &i;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(_priv->d->_priv->device, 1, &write, 0, nullptr);
		}

		Descriptorset *Descriptorpool::create_descriptorset(Descriptorsetlayout *l)
		{
			auto d = new Descriptorset;

			d->_priv = new DescriptorsetPrivate;
			d->_priv->d = _priv->d;

			VkDescriptorSetAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.descriptorPool = _priv->v;
			info.descriptorSetCount = 1;
			info.pSetLayouts = &l->_priv->v;

			vk_chk_res(vkAllocateDescriptorSets(_priv->d->_priv->device, &info, &d->_priv->v));

			return d;
		}

		Descriptorset *Descriptorpool::create_descriptorset(Pipeline *p, int index)
		{
			return create_descriptorset(p->_priv->descriptorsetlayouts[index]);
		}

		void Descriptorpool::destroy_descriptorset(Descriptorset *s)
		{
			assert(_priv->d == s->_priv->d);

			vk_chk_res(vkFreeDescriptorSets(_priv->d->_priv->device, _priv->v, 1, &s->_priv->v));
		}

		Descriptorpool *create_descriptorpool(Device *d)
		{
			auto p = new Descriptorpool;

			p->_priv = new DescriptorpoolPrivate;
			p->_priv->d = d;

			VkDescriptorPoolSize descriptorPoolSizes[] = {
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 128},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 32},
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048},
				{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 32},
			};

			VkDescriptorPoolCreateInfo descriptorPoolInfo;
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			descriptorPoolInfo.pNext = nullptr;
			descriptorPoolInfo.poolSizeCount = ARRAYSIZE(descriptorPoolSizes);
			descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
			descriptorPoolInfo.maxSets = 64;
			vk_chk_res(vkCreateDescriptorPool(d->_priv->device, &descriptorPoolInfo, nullptr, &p->_priv->v));

			return p;
		}

		void destroy_descriptorpool(Device *d, Descriptorpool *p)
		{
			assert(d == p->_priv->d);

			vkDestroyDescriptorPool(d->_priv->device, p->_priv->v, nullptr);
		}
#endif
	}
}
