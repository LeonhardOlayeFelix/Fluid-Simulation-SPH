#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "Particle.h"
#include "Parameters.h"
#include "PhysicsState.h"
#include <functional>

class PhysicsSolver {
public:
	inline static float MetersGravity = -9.81;
	inline static float PixelsPerMeter = 102;
	inline static float Gravity = MetersGravity * PixelsPerMeter;
	inline static float CollisionDampingFactor = 0.6;
	inline static float BallRadius = 3;
	inline static constexpr float PI = 3.141592;

	PhysicsSolver(std::shared_ptr<PhysicsState> physicsState);

	void Update(double timeStep);

	static float poly6Kernel(float r2, float h2, float coeff);
	static glm::vec3 poly6KernelGradient(glm::vec3 r, float r2, float h2, float coeff);
	static glm::vec3 spikyKernelGradient(glm::vec3 r, float r2, float h, float h2, float coeff);

private:
	std::shared_ptr<PhysicsState> m_PhysicsState;

	void AdvanceParticles(double timeStep);
	void UpdateParticleDensities(const std::vector<std::pair<uint32_t, uint32_t>>& spatialLookup, const std::vector<uint32_t>& startIndices);
	void static UpdateParticleDensitySummand(Particle& pi, const Particle& pj, const std::vector<float>& constants);
	void UpdateParticlePressures(const std::vector<std::pair<uint32_t, uint32_t>>& spatialLookup, const std::vector<uint32_t>& startIndices);
	void UpdateParticlePressureGradients(const std::vector<std::pair<uint32_t, uint32_t>>& spatialLookup, const std::vector<uint32_t>& startIndices);
	void DetectBoundaryCollision(Particle& particle);
	float CalculateSymmetrisedPressure(const Particle& source, const Particle& target);
	void ForEachNeighbour(const glm::vec3& position, std::function<void(const Particle&)> callback, const std::vector<std::pair<uint32_t, uint32_t>>& spatialLookup, const std::vector<uint32_t>& startIndices);

};

