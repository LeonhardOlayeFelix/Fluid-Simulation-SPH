#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "BoundingBox.h"

struct BoundaryParameters {
	BoundingBox ParticleDomain;
	BoundingBox Viewport;
};

struct FluidParameters {
	bool SimulationRunning;
	float RestDensity = 6.0;
	float FluidStiffness = 300000;
	float Viscosity = 3000;
	float ParticleSpacing = 10;
	float SmoothingRadius{ 25 };
	float ParticleMass = 1000;
	int ParticleCount = 1500;
};