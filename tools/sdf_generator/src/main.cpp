#include <flame/common/filesystem.h>
#include <flame/common/image.h>
#include <msdfgen.h>
#include <msdfgen-ext.h>

int main(int argc, char **args)
{
	if (argc < 3)
		return 0;

	auto chars = args[1];
	auto count = strlen(chars);
	auto size = std::atoi(args[2]);

	auto ft_library = msdfgen::initializeFreetype();
	auto ttf_data = msdfgen::loadFont(ft_library, "c:/windows/fonts/arialbd.ttf");
	auto i_c = 0;
	flame::Image sdf(size * count, size, 3, 24, nullptr, true);
	for (auto i_c = 0; i_c < count; i_c++)
	{
		msdfgen::Shape shape;
		if (msdfgen::loadGlyph(shape, ttf_data, chars[i_c]))
		{
			shape.normalize();
			msdfgen::edgeColoringSimple(shape, 3.0);
			msdfgen::Bitmap<msdfgen::FloatRGB> msdf(size, size);
			msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
			for (auto j = 0; j < size; j++)
			{
				for (auto i = 0; i < size; i++)
				{
					sdf.data[j * sdf.pitch + (i_c * size + i) * 3 + 0] = glm::clamp(msdf(i, j).r * 255.f, 0.f, 255.f);
					sdf.data[j * sdf.pitch + (i_c * size + i) * 3 + 1] = glm::clamp(msdf(i, j).g * 255.f, 0.f, 255.f);
					sdf.data[j * sdf.pitch + (i_c * size + i) * 3 + 2] = glm::clamp(msdf(i, j).b * 255.f, 0.f, 255.f);
				}
			}
		}
	}
	msdfgen::destroyFont(ttf_data);
	msdfgen::deinitializeFreetype(ft_library);

	sdf.save_raw("sdf.rimg");

	return 0;
}
