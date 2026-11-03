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
	std::vector<std::pair<uint32_t, uint32_t>> m_SpatialLookup; //query with particle index. Stores a list fo (Cell key, Particle Index) pairs
	std::vector<uint32_t> m_StartIndices;  //query with cell key. Stores a list of (indicies) at which cells start appearing in spatialLookup

	void AdvanceParticles(double timeStep);
	void UpdateGridSearch();
	void UpdatePredictedPositions(double timeStep);
	void UpdateParticleDensities();
	void UpdateParticlePressures();
	void UpdateParticlePressureGradients();
	void UpdateActualPositions(double timeStep);
	void DetectBoundaryCollision(Particle& particle);
	void ForEachNeighbour(const glm::vec3& position, std::function<void(const Particle&)> callback, const std::vector<std::pair<uint32_t, uint32_t>>& spatialLookup, const std::vector<uint32_t>& startIndices);

};

