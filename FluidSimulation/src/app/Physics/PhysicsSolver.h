#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "Particle.h"
#include "Parameters.h"
#include "PhysicsState.h"

class PhysicsSolver {
public:
	inline static float MetersGravity = -9.81;
	inline static float PixelsPerMeter = 102;
	inline static float Gravity = MetersGravity * PixelsPerMeter;
	inline static float CollisionDampingFactor = 0.6;
	inline static float BallRadius = 5;
	inline static constexpr float PI = 3.141592;

	PhysicsSolver(std::shared_ptr<PhysicsState> physicsState);

	void Update(double timeStep);

	static float poly6Kernel(float r2, float h2, float coeff);

private:
	std::shared_ptr<PhysicsState> m_PhysicsState;

	void AdvanceParticles(double timeStep);
	void UpdateParticleDensities(float smoothingRadius);
	void DetectBoundaryCollision(Particle& particle);
};

