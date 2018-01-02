#include "a.h"

A::A()
{
	p = new B;
}

A::~A()
{
	delete p;
}