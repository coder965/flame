#pragma once

#include <memory>

struct B;

struct A
{
	std::unique_ptr<B> p;

	A();
	~A();
};
