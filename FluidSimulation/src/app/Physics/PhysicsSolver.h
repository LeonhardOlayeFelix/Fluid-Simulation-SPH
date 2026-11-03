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
	void ForEachNeighbour(const glm::vec3& position, std::function<void(const Particle&)> callback);
	uint32_t HashCell(int x, int y, uint32_t n);

	template <typename Callback>
	void ForEachNeighbour2(const glm::vec3& position, Callback callback) {

		//function which searches all the neighbors of a particle using a spatialLookup and StartIndices array.

		std::vector<Particle>& particles = m_PhysicsState->Particles;
		float h = m_PhysicsState->FluidParameters.SmoothingRadius;

		int cellX = std::floor(position.x / h);
		int cellY = std::floor(position.y / h);

		for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
			for (int colOffset = -1; colOffset <= 1; colOffset++) {
				int cellToSearchX = cellX + rowOffset;
				int cellToSearchY = cellY + colOffset;

				uint32_t cellKey = HashCell(cellToSearchX, cellToSearchY, (uint32_t)particles.size());

				uint32_t startParticleIndex = m_StartIndices[cellKey];

				for (uint32_t i = startParticleIndex; i < m_SpatialLookup.size(); i++) {

					if (m_SpatialLookup[i].first != cellKey) break;

					const Particle& pj = particles[m_SpatialLookup[i].second];

					callback(pj);

				}

			}
		}

	}

};

