#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct BoundaryParameters {
	glm::vec2 ViewportSize{ };
};

struct FluidParameters {
	bool SimulationRunning;
	float RestDensity = 6.0;
	float FluidStiffness = 300000;
	float Viscosity = 3000;
	float SmoothingRadius{ 25 };
	int ParticleCount = 3000;
};