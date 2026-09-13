#pragma once

#include <vector>
#include <math.h>

struct Primitive {
	std::vector<float> positions;
	std::vector<unsigned int> indicies;
};

class Primitives
{
public:
	Primitives() = delete;

	static Primitive Circle(unsigned int n);
	static Primitive NDCQuad();

};

