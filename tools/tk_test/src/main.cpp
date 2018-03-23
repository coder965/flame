#include <vector>
#include <string>

static const char *sdf_text_chars = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static int sdf_text_char_count = strlen(sdf_text_chars);
static int sdf_text_size = 32;

int main(int argc, char **args)
{
	std::string cl(sdf_text_chars);
	cl += " ";
	cl += std::to_string(sdf_text_size);
	//exec("sdf_generator", cl.c_str());

	return 0;
}
