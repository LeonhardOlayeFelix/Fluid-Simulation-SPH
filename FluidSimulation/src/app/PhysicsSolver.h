#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "BoundaryData.h"
#include "Particle.h"


class PhysicsSolver {
public:

	static constexpr float MetersGravity = -9.81;
	static constexpr float PixelsPerMeter = 50;
	static constexpr float Gravity = MetersGravity * PixelsPerMeter;
	static constexpr float CollisionDampingFactor = 0.8;

	static void AdvanceParticles(std::vector<Particle>& particles, double timeStep, BoundaryData& boundaryConfig);
	static void AdvanceParticle(Particle& particle, double timeStep, BoundaryData& boundaryConfig);
	static void DetectBoundaryCollision(Particle& particle, BoundaryData& boundaryConfig);
};

