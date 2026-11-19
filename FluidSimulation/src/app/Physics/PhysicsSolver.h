#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "FluidParticle.h"
#include "Parameters.h"
#include "PhysicsState.h"
#include <functional>
#include <utility>

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
	static float viscosityLaplacianKernel2D(float r2, float h, float h2, float coeff);
	static glm::vec3 poly6KernelGradient(glm::vec3 r, float r2, float h2, float coeff);
	static glm::vec3 spikyKernelGradient(glm::vec3 r, float r2, float h, float h2, float coeff);

private:
	std::shared_ptr<PhysicsState> m_PhysicsState;
	std::vector<std::pair<uint32_t, uint32_t>> m_FluidSpatialLookup; //query with particle index. Stores a list fo (Cell key, Particle Index) pairs
	std::vector<uint32_t> m_FluidStartIndices;  //query with cell key. Stores a list of (indicies) at which cells start appearing in spatialLookup
	std::vector<std::pair<uint32_t, uint32_t>> m_BoundarySpatialLookup; //query with particle index. Stores a list fo (Cell key, Particle Index) pairs
	std::vector<uint32_t> m_BoundaryStartIndices;  //query with cell key. Stores a list of (indicies) at which cells start appearing in spatialLookup

	void AdvanceParticles(double timeStep);
	void UpdateBoundaryParticleGridSearch();
	void UpdateFluidParticleGridSearch();
	void UpdateGammaCorrectionCoefficients();
	GammaTemplate GenerateGammaCorrectionTemplate();
	void UpdateGamma1(const GammaTemplate& templ);
	void UpdateGamma2(const GammaTemplate& templ);
	void UpdatePredictedPositions(double timeStep);
	void UpdateParticleDensities();
	void UpdateParticlePressures();
	void UpdateParticlePressureForceDensities();
	void UpdateParticleViscosityForceDensities();
	void UpdateActualPositions(double timeStep);
	void DetectBoundaryCollision(FluidParticle& particle);
	uint32_t HashCell(int x, int y, uint32_t n);

	template <typename Callback>
	void ForEachFluidNeighbour(const glm::vec3& position, Callback callback) {

		//function which searches all the neighbors of a particle using a spatialLookup and StartIndices array.

		std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;
		if (particles.empty()) return;
		float h = m_PhysicsState->FluidParameters.SmoothingRadius;

		int cellX = std::floor(position.x / h);
		int cellY = std::floor(position.y / h);

		uint32_t searchedKeys[9];
		int searchedKeyCount = 0;

		for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
			for (int colOffset = -1; colOffset <= 1; colOffset++) {
				int cellToSearchX = cellX + rowOffset;
				int cellToSearchY = cellY + colOffset;

				uint32_t cellKey = HashCell(cellToSearchX, cellToSearchY, (uint32_t)particles.size());

				bool alreadySearched = false;
				for (int k = 0; k < searchedKeyCount; k++) {
					if (searchedKeys[k] == cellKey) {
						alreadySearched = true;
						break;
					}
				}
				if (alreadySearched) continue;

				searchedKeys[searchedKeyCount++] = cellKey;

				uint32_t startParticleIndex = m_FluidStartIndices[cellKey];

				for (uint32_t i = startParticleIndex; i < m_FluidSpatialLookup.size(); i++) {

					if (m_FluidSpatialLookup[i].first != cellKey) break;

					const FluidParticle& pj = particles[m_FluidSpatialLookup[i].second];

					callback(pj);

				}

			}
		}

	}

	template <typename Callback>
	void ForEachBoundaryNeighbour(const glm::vec3& position, Callback callback) {

		//function which searches all the boundary neighbours of a position using the boundary spatialLookup and StartIndices arrays.

		std::vector<BoundaryParticle>& particles = m_PhysicsState->BoundaryParticles;
		if (particles.empty()) return;

		float h = m_PhysicsState->FluidParameters.SmoothingRadius;

		int cellX = std::floor(position.x / h);
		int cellY = std::floor(position.y / h);

		uint32_t searchedKeys[9];
		int searchedKeyCount = 0;

		for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
			for (int colOffset = -1; colOffset <= 1; colOffset++) {
				int cellToSearchX = cellX + rowOffset;
				int cellToSearchY = cellY + colOffset;

				uint32_t cellKey = HashCell(cellToSearchX, cellToSearchY, (uint32_t)particles.size());

				bool alreadySearched = false;
				for (int k = 0; k < searchedKeyCount; k++) {
					if (searchedKeys[k] == cellKey) {
						alreadySearched = true;
						break;
					}
				}
				if (alreadySearched) continue;

				searchedKeys[searchedKeyCount++] = cellKey;

				uint32_t startParticleIndex = m_BoundaryStartIndices[cellKey];

				for (uint32_t i = startParticleIndex; i < m_BoundarySpatialLookup.size(); i++) {

					if (m_BoundarySpatialLookup[i].first != cellKey) break;

					const BoundaryParticle& pb = particles[m_BoundarySpatialLookup[i].second];

					callback(pb);

				}

			}
		}

	}

};

struct GammaTemplate {
	std::vector<FluidParticle> FluidParticles;
	std::vector<BoundaryParticle> BoundaryParticles;

};