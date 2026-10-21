#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Particle.h"
#include "Parameters.h"


class PhysicsSolver {
public:

	inline static float MetersGravity = -9.81;
	inline static float PixelsPerMeter = 102;
	inline static float Gravity = MetersGravity * PixelsPerMeter;
	inline static float CollisionDampingFactor = 0.6;
	inline static float BallRadius = 20;
	inline static constexpr float PI = 3.141592;

	static void AdvanceParticles(std::vector<Particle>& particles, double timeStep, BoundaryParameters& boundaryParameters, FluidParameters& fluidParameters);
	static void AdvanceParticle(Particle& particle, double timeStep, BoundaryParameters& boundaryConfig);
	static void UpdateParticleDensities(std::vector<Particle>& particles, float smoothingRadius);
	static float poly6Kernel(float r2, float h2, float coeff);
	static void DetectBoundaryCollision(Particle& particle, BoundaryParameters& boundaryConfig);
};

