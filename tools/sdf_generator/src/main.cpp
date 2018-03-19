#include <msdfgen.h>
#include <msdfgen-ext.h>

float clamp(float v, float _min, float _max)
{
	if (v < _min)
		return _min;
	if (v > _max)
		return _max;
	return v;
}

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
	auto cx = size * count;
	auto pitch = cx * 3;
	if (pitch % 4 != 0)
		pitch += 4 - pitch % 4;
	auto cy = size;
	auto channel = 3;
	auto img_size = cx * pitch;
	auto data = new unsigned char[img_size];
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
					data[j * pitch + (i_c * size + i) * 3 + 0] = clamp(msdf(i, j).r * 255.f, 0.f, 255.f);
					data[j * pitch + (i_c * size + i) * 3 + 1] = clamp(msdf(i, j).g * 255.f, 0.f, 255.f);
					data[j * pitch + (i_c * size + i) * 3 + 2] = clamp(msdf(i, j).b * 255.f, 0.f, 255.f);
				}
			}
		}
	}
	msdfgen::destroyFont(ttf_data);
	msdfgen::deinitializeFreetype(ft_library);

	auto f = fopen("sdf.rimg", "wb");
	fwrite(&cx, 4, 1, f);
	fwrite(&cy, 4, 1, f);
	fwrite(&channel, 4, 1, f);
	fwrite(data, img_size, 1, f);
	fclose(f);
	delete[]data;

	return 0;
}
