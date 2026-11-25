#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "BoundingBox.h"

struct BoundaryParameters {
	BoundingBox ParticleDomain{glm::vec2(0, 0), glm::vec2(300, 300)};
	bool Stale = false;
};

struct FluidParameters {
	bool SimulationRunning = false;
	bool InfluencedByBoundaryParticles = true;
	bool InfluencedByGravity = true;
	bool InfluencedBySurfaceTension = true;
	bool InfluencedByPressure = true;
	bool InfluencedByViscousForce = true;

	bool ClampPressure = true;

	int FluidParticleCount = 1500;

	float RestDensity = 6.0;
	float ParticleSpacing = 10;
	float SmoothingRadius{ 25 };
	float ParticleMass = RestDensity * ParticleSpacing * ParticleSpacing;
	float SurfaceTensionThreshold = 0.75 / SmoothingRadius;

	float Stiffness = 1'300'000.0f;
	float Viscosity = 10000;
	float Gamma1 = 0;
	float Gamma2 = 0;
	float TensionCoefficient = 4000000;
};