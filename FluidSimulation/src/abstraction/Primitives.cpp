#include "Primitives.h"

Primitive Primitives::Circle(unsigned int n)
{
	std::vector<float> vertices = { 0.0f, 0.0f, 0.0f };

	constexpr double PI = 3.141593;

	double factor = (2 * PI) / n;

	for (int k = 0; k < n; k++) {
		vertices.insert(vertices.end(), { (float)cos(factor * k), (float)sin(factor * k), 0 });
	}

	std::vector<unsigned int> indices = {};

	for (unsigned int i = 1; i < n; i++) {
		indices.insert(indices.end(), { 0, i, i + 1 });
	}

	indices.insert(indices.end(), { 0, n, 1 });

	return { vertices, indices };
}

Primitive Primitives::NDCQuad()
{

	std::vector<float> vertices = {
		-1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,   1.0f, 1.0f
	};

	std::vector<unsigned int> indices = {
	0, 1, 2,
	0, 2, 3 
	};

	return { vertices, indices };
}
