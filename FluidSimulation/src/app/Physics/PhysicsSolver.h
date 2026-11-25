#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <functional>
#include <utility>

#include "../Data/FluidParticle.h"
#include "../Data/ParameterData.h"
#include "../Data/PhysicsData.h"

struct GammaTemplate {
	std::vector<FluidParticle> FluidParticles;
	std::vector<BoundaryParticle> BoundaryParticles;

};

class PhysicsSolver {
public:

	inline static float MetersGravity = -9.81;
	inline static float PixelsPerMeter = 102;
	inline static float Gravity = MetersGravity * PixelsPerMeter;
	inline static float CollisionDampingFactor = 0.6;
	inline static float FluidParticleRadius = 3;
	inline static float BoundaryParticleRadius = 3;
	inline static constexpr float PI = 3.141592;

	PhysicsSolver(std::shared_ptr<PhysicsData> physicsState);

	void Update(double timeStep);

private:
	std::shared_ptr<PhysicsData> m_PhysicsState;
	std::vector<std::pair<uint32_t, uint32_t>> m_FluidCellEntries;      // (cell key, fluid index) pairs, sorted by key
	std::vector<uint32_t> m_FluidCellStarts;                            // by cell key: first entry in m_FluidCellEntries, or UINT32_MAX
	std::vector<std::pair<uint32_t, uint32_t>> m_BoundaryCellEntries;   // (cell key, boundary index) pairs, sorted by key
	std::vector<uint32_t> m_BoundaryCellStarts;                         // by cell key: first entry in m_BoundaryCellEntries, or UINT32_MAX

	std::vector<uint32_t> m_FluidNeighborIndices;                       // fluid neighbours of every fluid particle, one after another
	std::vector<uint32_t> m_BoundaryNeighborIndices;                    // boundary neighbours of every fluid particle, one after another
	std::vector<uint32_t> m_FluidNeighborOffsets;						// size N+1: particle i's range is [offsets[i], offsets[i+1])
	std::vector<uint32_t> m_BoundaryNeighborOffsets;                    // size N+1: particle i's range is [offsets[i], offsets[i+1])


	void UpdatePredictedProperties(double timeStep);
	void UpdateSpatialHashGrids();
	void UpdateParticleDensities();
	void UpdateParticlePressures();
	void UpdateParticlePressureForceDensities();
	void UpdateParticleViscosityForceDensities();
	void UpdateParticleSurfaceTensionForceDensities();
	void UpdateActualProperties(double timeStep);
	void DetectBoundaryCollision(FluidParticle& particle);

	glm::vec3 getResultantPredictionForce();
	void updateBoundaryParticleSpatialHashGrid();
	void updateFluidParticleSpatialHashGrid();
	void updateFluidNeighborLists();
	void updateBoundaryNeighborLists();
	void updateGammaCorrectionCoefficients();
	uint32_t hashCell(int x, int y, uint32_t n);
	GammaTemplate generateGammaCorrectionTemplate();
	void updateGamma1(const GammaTemplate& templ);
	void updateGamma2(const GammaTemplate& templ);
	static float poly6Kernel(float r2, float h2, float coeff);
	static glm::vec3 poly6KernelGradient(glm::vec3 r, float r2, float h2, float coeff);
	static float poly6KernelLaplacian(float r2, float h2, float coeff);
	static glm::vec3 spikyKernelGradient(glm::vec3 r, float r2, float h, float h2, float coeff);
	static float viscosityLaplacianKernel2D(float r2, float h, float h2, float coeff);


	template <typename Callback>
	void ForEachFluidNeighbour(uint32_t particleIndex, Callback callback) {

		std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;

		uint32_t offset = m_FluidNeighborOffsets[particleIndex];

		uint32_t numNeighbors = m_FluidNeighborOffsets[particleIndex + 1] - offset;

		for (uint32_t i = offset; i < offset + numNeighbors; i++) {

			uint32_t neighborIndex = m_FluidNeighborIndices[i];

			const FluidParticle& pj = particles[neighborIndex];

			callback(pj);

		}
	}

	template <typename Callback>
	void ForEachBoundaryNeighbour(uint32_t particleIndex, Callback callback) {

		std::vector<BoundaryParticle>& particles = m_PhysicsState->BoundaryParticles;

		uint32_t offset = m_BoundaryNeighborOffsets[particleIndex];

		uint32_t numNeighbors = m_BoundaryNeighborOffsets[particleIndex + 1] - offset;

		for (uint32_t i = offset; i < offset + numNeighbors; i++) {

			uint32_t neighborIndex = m_BoundaryNeighborIndices[i];

			const BoundaryParticle& pj = particles[neighborIndex];

			callback(pj);

		}
	}

};

