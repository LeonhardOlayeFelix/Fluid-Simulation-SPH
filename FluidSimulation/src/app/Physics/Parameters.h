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

enum class PressureSolverMethod {
	SESPH,
	IISPH
};

struct FluidParameters {

	PressureSolverMethod PressureSolver = PressureSolverMethod::SESPH;

	bool SimulationRunning;
	bool IgnoreBoundaryParticles = false;

	int FluidParticleCount = 1'000;

	float RestDensity = 6;
	float Stiffness = 1'300'000;
	float Viscosity = 30'000;
	float ParticleSpacing = 10;
	float SmoothingRadius = 25;
	float ParticleMass = RestDensity * ParticleSpacing * ParticleSpacing;
	float Gamma1 = 0;
	float Gamma2 = 0;
	float IISPHRelaxationCoefficient = 0.5;
};