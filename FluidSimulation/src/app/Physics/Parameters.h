#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "BoundingBox.h"

struct BoundaryParameters {
	BoundingBox ParticleDomain;
	BoundingBox Viewport;
	bool Stale = false;
};

struct FluidParameters {
	bool SimulationRunning;
	bool IgnoreBoundaryParticles = false;

	int FluidParticleCount = 1500;

	float RestDensity = 6.0;
	float FluidStiffness = 1'300'000.0f;
	float Viscosity = 10000;
	float ParticleSpacing = 10;
	float SmoothingRadius{ 25 };
	float ParticleMass = RestDensity * ParticleSpacing * ParticleSpacing;
	float Gamma1 = 0;
	float Gamma2 = 0;
};