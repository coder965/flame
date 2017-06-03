#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <regex>

#include "../src/utils.h"

int main(int argc, char** argv)
{
	auto at = tke::createAttributeTreeFromXML("stages", "stages.xml");

	return 0;
}

