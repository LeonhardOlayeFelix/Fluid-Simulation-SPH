#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct BoundaryParameters {
	glm::vec2 ViewportSize{ };
};

struct FluidParameters {
	bool SimulationRunning;
	float RestDensity = 0.1;
	float FluidStiffness = 10000;
	float SmoothingRadius{ 45 };
};