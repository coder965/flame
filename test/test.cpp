#include <vector>

#include "1.h"

#include "../src/core/utils.h"

int main(int argc, char** argv)
{
	tke::VariableReflection v("123", (int*)0);

	tke::ReflectionBank b;

	b.addV<int>("fuck", 0);

	return 0;
}

