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
	bool ClampPressure = true;

	int FluidParticleCount = 1500;

	float RestDensity = 6.0;
	float ParticleSpacing = 10;
	float SmoothingRadius{ 25 };
	float ParticleMass = RestDensity * ParticleSpacing * ParticleSpacing;

	float Stiffness = 1'300'000.0f;
	float Viscosity = 10000;
	float Gamma1 = 0;
	float Gamma2 = 0;
	float TensionCoefficient = 1;
};