#include <assert.h>
#include "image.file.h"

#ifdef _WIN64
#include "../../../FreeImage\Dist\x64\FreeImage.h"
#else
#include "../../../FreeImage\Dist\x32\FreeImage.h"
#endif

namespace tke
{
	struct _Data
	{
		FREE_IMAGE_FORMAT fif;
		size_t m_bpp;
		size_t m_channel;
		size_t m_cx;
		size_t m_cy;
		size_t m_pitch;
		size_t m_size;
		unsigned char *m_data;
		void swapChannel(size_t channel0, size_t channel1)
		{
			for (int i = 0; i < m_cy; i++)
			{
				for (int j = 0; j < m_cx; j++)
					std::swap(m_data[i * m_pitch + j * m_channel + channel0], m_data[i * m_pitch + j * m_channel + channel1]);
			}
		}
		~_Data()
		{
			delete[]m_data;
		}
	};

	_Data *createImageData(const std::string &filename)
	{
		auto fif = FIF_UNKNOWN;
		fif = FreeImage_GetFileType(filename.c_str());
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(filename.c_str());
		if (fif == FIF_UNKNOWN) return nullptr;
		auto dib = FreeImage_Load(fif, filename.c_str());
		if (!dib) return nullptr;
		if (fif == FREE_IMAGE_FORMAT::FIF_JPEG || fif == FREE_IMAGE_FORMAT::FIF_TARGA || fif == FREE_IMAGE_FORMAT::FIF_PNG) 
			FreeImage_FlipVertical(dib);
		auto pData = new _Data;
		auto colorType = FreeImage_GetColorType(dib);
		pData->fif = fif;
		switch (colorType)
		{
		case FIC_MINISBLACK: case FIC_MINISWHITE:
			pData->m_channel = 1;
			break;
		case FIC_RGB:
		{
			auto newDib = FreeImage_ConvertTo32Bits(dib);
			FreeImage_Unload(dib);
			dib = newDib;
			pData->m_channel = 4;
		}
			break;
		case FIC_RGBALPHA:
			pData->m_channel = 4;
			break;
		}
		pData->m_cx = FreeImage_GetWidth(dib);
		pData->m_cy = FreeImage_GetHeight(dib);
		pData->m_bpp = FreeImage_GetBPP(dib);
		pData->m_pitch = FreeImage_GetPitch(dib);
		pData->m_size = pData->m_pitch * pData->m_cy;
		pData->m_data = new unsigned char[pData->m_size];
		memcpy(pData->m_data, FreeImage_GetBits(dib), pData->m_size);
		FreeImage_Unload(dib);
		return pData;
	}

	static void _formatImageData(_Data *p)
	{
		if (p->m_channel == 4)
		{
			if (p->fif == FREE_IMAGE_FORMAT::FIF_BMP ||
				p->fif == FREE_IMAGE_FORMAT::FIF_TARGA ||
				p->fif == FREE_IMAGE_FORMAT::FIF_JPEG ||
				p->fif == FREE_IMAGE_FORMAT::FIF_PNG)
				p->swapChannel(0, 2);
		}
	}

	static VkFormat _getFormat(_Data *p, bool sRGB)
	{
		switch (p->m_channel)
		{
		case 1:
			switch (p->m_bpp)
			{
			case 16:
				return VK_FORMAT_R16_UNORM;
			}
			break;
		case 4:
			if (sRGB) return VK_FORMAT_R8G8B8A8_SRGB;
			else  return VK_FORMAT_R8G8B8A8_UNORM;
		}

		return VK_FORMAT_UNDEFINED;
	}

	Image *createImage(const std::string &filename, bool sRGB, bool saveData)
	{
		auto pData = createImageData(filename);
		assert(pData);

		_formatImageData(pData);

		auto vkFormat = _getFormat(pData, sRGB);

		auto pImage = new Image(pData->m_cx, pData->m_cy, vkFormat, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, pData->m_data, pData->m_size);
		pImage->filename = filename;
		pImage->m_sRGB = sRGB;

		delete pData;

		return pImage;
	}
}