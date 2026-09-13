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

	static Primitive Circle(unsigned int n) {

		std::vector<float> res = { 0.0f, 0.0f, 0.0f };
		
		constexpr double PI = 3.141593;

		double factor = (2 * PI) / n;

		for (int k = 0; k < n; k++) {
			res.insert(res.end(), {(float)cos(factor * k), (float)sin(factor * k), 0});
		}

		std::vector<unsigned int> indices = {};

		for (unsigned int i = 1; i < n; i++) {
			indices.insert(indices.end(), {0, i, i + 1});
		}

		indices.insert(indices.end(), { 0, n, 1 });

		return { res, indices };
	}

};

