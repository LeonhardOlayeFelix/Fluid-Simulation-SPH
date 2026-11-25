#include <cstdint>
#include <algorithm>
#include <utility>

#include "PhysicsSolver.h"
#include "../Data/FluidParticle.h"
PhysicsSolver::PhysicsSolver(std::shared_ptr<PhysicsData> physicsState) : m_PhysicsState(physicsState)
{
}

void PhysicsSolver::Update(double timeStep)
{
	if (!m_PhysicsState->FluidParameters.SimulationRunning || m_PhysicsState->FluidParticles.size() == 0) return;

	UpdatePredictedProperties(timeStep);

	UpdateSpatialHashGrids();

	UpdateParticleDensities();

	UpdateParticlePressures();

	UpdateParticlePressureForceDensities();

	UpdateParticleViscosityForceDensities();

	UpdateParticleSurfaceTensionForceDensities();

	UpdateActualProperties(timeStep);

	m_PhysicsState->BoundaryParameters.Stale = false;

}

void PhysicsSolver::UpdatePredictedProperties(double timeStep)
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;

	float dt = (float)timeStep;

	for (FluidParticle& particle : particles)
	{
		particle.PredictedVelocity = particle.Velocity + dt * getResultantPredictionForce();
		particle.PredictedPosition = particle.Position + dt * particle.PredictedVelocity;
	}
}

void PhysicsSolver::UpdateSpatialHashGrids()
{
	if (m_PhysicsState->BoundaryParameters.Stale)
	{
		updateBoundaryParticleSpatialHashGrid();
		updateGammaCorrectionCoefficients();
	}

	updateFluidParticleSpatialHashGrid();
	updateFluidNeighborLists();
	updateBoundaryNeighborLists();
}

void PhysicsSolver::UpdateParticleDensities()
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float h = fluid.SmoothingRadius;
	float h2 = h * h;
	float h8 = h2 * h2 * h2 * h2;
	float coeff = 4.0f / (PI * h8);
	float particleMass = fluid.ParticleMass;

	for (int i = 0; i < particles.size(); i++) {

		FluidParticle& pi = particles[i];
		float fluidTerm = 0;

		ForEachFluidNeighbour(i, [&](const FluidParticle& pj) {

				glm::vec3 d = pi.PredictedPosition - pj.PredictedPosition;
				float r2 = glm::dot(d, d);

				fluidTerm += poly6Kernel(r2, h2, coeff);

			});

		fluidTerm *= particleMass;

		float boundaryTerm = 0;

		if (fluid.InfluencedByBoundaryParticles) 
		{
			ForEachBoundaryNeighbour(i, [&](const BoundaryParticle& pj) {

				glm::vec3 d = pi.PredictedPosition - pj.Position;
				float r2 = glm::dot(d, d);

				boundaryTerm += poly6Kernel(r2, h2, coeff);
			});
		}

		boundaryTerm *= particleMass;

		pi.Density = fluidTerm + fluid.Gamma1 * boundaryTerm;

	}
}

void PhysicsSolver::UpdateParticlePressures()
{
	const FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float stiffness = fluid.Stiffness;
	float restDensity = fluid.RestDensity;

	for (int i = 0; i < m_PhysicsState->FluidParticles.size(); i++) {

		FluidParticle& source = m_PhysicsState->FluidParticles[i];

		if (!fluid.InfluencedByPressure) 
		{
			source.Pressure = 0;
			continue;
		}

		float pressure = stiffness * (source.Density - restDensity);
		source.Pressure = fluid.ClampPressure ? std::max(0.0f, pressure) : pressure;
	}
}

void PhysicsSolver::UpdateParticlePressureForceDensities()
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	float h2 = h * h;
	float h5 = h2 * h2 * h;
	float coeff = -30.0f / (PI * h5);
	float particleMass = m_PhysicsState->FluidParameters.ParticleMass;

	for (int i = 0; i < particles.size(); i++) {

		FluidParticle& pi = particles[i];

		glm::vec3 fluidTerm{};

		ForEachFluidNeighbour(i, [&](const FluidParticle& pj) {

			glm::vec3 r = pi.PredictedPosition - pj.PredictedPosition;
			float r2 = glm::dot(r, r);

			float symmetrisedPressure = pi.Pressure / (pi.Density * pi.Density) + pj.Pressure / (pj.Density * pj.Density);

			fluidTerm += symmetrisedPressure * spikyKernelGradient(r, r2, h, h2, coeff); 

			});

		fluidTerm *= particleMass;

		glm::vec3 boundaryTerm{};

		if (fluid.InfluencedByBoundaryParticles)
		{

			ForEachBoundaryNeighbour(i, [&](const BoundaryParticle& pj) {

				glm::vec3 r = pi.PredictedPosition - pj.Position;
				float r2 = glm::dot(r, r);

				boundaryTerm += spikyKernelGradient(r, r2, h, h2, coeff);

				});
		}

		boundaryTerm *= pi.Pressure * 2 * particleMass / (pi.Density * pi.Density);

		pi.PressureForceDensity = pi.Density * (-fluidTerm - fluid.Gamma2 * boundaryTerm);

	}
}

void PhysicsSolver::UpdateParticleViscosityForceDensities()
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float h = fluid.SmoothingRadius;
	float h2 = h * h;
	float h5 = h2 * h2 * h;
	float coeff = 40 / (PI * h5);
	float particleMass = fluid.ParticleMass;

	for (int i = 0; i < particles.size(); i++) {

		FluidParticle& pi = particles[i];

		if (!fluid.InfluencedByViscousForce) 
		{
			pi.ViscousForceDensity = glm::vec3(0);
			continue;
		}

		glm::vec3 summation{};

		ForEachFluidNeighbour(i, [&](const FluidParticle& pj) {

			glm::vec3 r = pi.PredictedPosition - pj.PredictedPosition;
			float r2 = glm::dot(r, r);

			glm::vec3 symmetrisedVelocity = (pj.Velocity - pi.Velocity);

			summation += symmetrisedVelocity / pj.Density * viscosityLaplacianKernel2D(r2, h, h2, coeff);

			});

		pi.ViscousForceDensity = fluid.Viscosity * particleMass * summation;

	}

}

void PhysicsSolver::UpdateParticleSurfaceTensionForceDensities()
{


	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;
	const FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float h = fluid.SmoothingRadius;
	float h2 = h * h;
	float h8 = h2 * h2 * h2 * h2;
	float gradientCoeff = -24.0f / (PI * h8);
	float laplacianCoeff = -48.0f / (PI * h8);

	

	for (int i = 0; i < particles.size(); i++) {

		FluidParticle& pi = particles[i];

		if (!fluid.InfluencedBySurfaceTension) {
			pi.SurfaceTensionForceDensity = glm::vec3(0);
			pi.ColorFieldGradient = glm::vec3(0);
			continue;
		}

		glm::vec3 gradientSummation{};
		float laplacianSummation = 0;

		ForEachFluidNeighbour(i, [&](const FluidParticle& pj) {

			glm::vec3 r = pi.PredictedPosition - pj.PredictedPosition;
			float r2 = glm::dot(r, r);

			gradientSummation += (1 / pj.Density) * poly6KernelGradient(r, r2, h2, gradientCoeff);
			laplacianSummation += (1 / pj.Density) * poly6KernelLaplacian(r2, h2, laplacianCoeff);

			});

		glm::vec3 colorFieldGradient = fluid.ParticleMass * gradientSummation;
		float colorFieldLaplacian = fluid.ParticleMass * laplacianSummation;

		glm::vec3 n = colorFieldGradient;

		float l = fluid.SurfaceTensionThreshold;

		if (glm::dot(n, n) > l * l) 
		{
			float kappa = -(colorFieldLaplacian) / glm::length(n);
			pi.SurfaceTensionForceDensity = fluid.TensionCoefficient * kappa * n;
			pi.ColorFieldGradient = n;
		}
		else 
		{
			pi.SurfaceTensionForceDensity = glm::vec3(0);
			pi.ColorFieldGradient = glm::vec3(0);
		}
	}
}

void PhysicsSolver::UpdateActualProperties(double timeStep)
{
	float dt = (float)timeStep;
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;

	for (FluidParticle& particle : particles)
	{
		glm::vec3 f_i = particle.RecalculateResultantForceDensity(); // pressure + viscous only
		particle.Acceleration = f_i / particle.Density;
		particle.Velocity = particle.PredictedVelocity + dt * particle.Acceleration;
		particle.Position += dt * particle.Velocity;

		DetectBoundaryCollision(particle);
	}
}

void PhysicsSolver::DetectBoundaryCollision(FluidParticle& particle)
{
	const BoundingBox& bounds = m_PhysicsState->BoundaryParameters.ParticleDomain;

	if (particle.Position.y - FluidParticleRadius <= bounds.ymin())
	{
		particle.Position.y = FluidParticleRadius + bounds.ymin();
		particle.Velocity.y = -particle.Velocity.y * CollisionDampingFactor;
	}
	else if (particle.Position.y + FluidParticleRadius >= bounds.ymax())
	{
		particle.Position.y = bounds.ymax() - FluidParticleRadius;
		particle.Velocity.y = -particle.Velocity.y * CollisionDampingFactor;
	}

	if (particle.Position.x - FluidParticleRadius <= bounds.xmin())
	{
		particle.Position.x = FluidParticleRadius + bounds.xmin();
		particle.Velocity.x = -particle.Velocity.x * CollisionDampingFactor;
	}
	else if (particle.Position.x + FluidParticleRadius >= bounds.xmax())
	{
		particle.Position.x = bounds.xmax() - FluidParticleRadius;
		particle.Velocity.x = -particle.Velocity.x * CollisionDampingFactor;
	}
}

glm::vec3 PhysicsSolver::getResultantPredictionForce()
{
	const FluidParameters& fluid = m_PhysicsState->FluidParameters;

	return fluid.InfluencedByGravity ? glm::vec3(0, Gravity, 0) : glm::vec3(0);
}

void PhysicsSolver::updateBoundaryParticleSpatialHashGrid()
{
	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	std::vector<BoundaryParticle>& particles = m_PhysicsState->BoundaryParticles;

	m_BoundaryCellEntries.resize(particles.size());
	m_BoundaryCellStarts.assign(particles.size(), UINT32_MAX);

	for (int i = 0; i < m_BoundaryCellEntries.size(); i++) {
		BoundaryParticle& particle = particles[i];

		int cellX = std::floor(particle.Position.x / h);
		int cellY = std::floor(particle.Position.y / h);

		uint32_t key = hashCell(cellX, cellY, (uint32_t)particles.size());

		m_BoundaryCellEntries[i].first = key;
		m_BoundaryCellEntries[i].second = i;

	}

	std::sort(m_BoundaryCellEntries.begin(), m_BoundaryCellEntries.end());

	uint32_t lastSeenNum = -1;

	for (int i = 0; i < m_BoundaryCellEntries.size(); i++) {
		if (m_BoundaryCellEntries[i].first != lastSeenNum) {
			m_BoundaryCellStarts[m_BoundaryCellEntries[i].first] = i;
			lastSeenNum = m_BoundaryCellEntries[i].first;
		}
	}
}

void PhysicsSolver::updateFluidParticleSpatialHashGrid()
{
	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;

	m_FluidCellEntries.resize(particles.size());
	m_FluidCellStarts.assign(particles.size(), UINT32_MAX);

	for (int i = 0; i < m_FluidCellEntries.size(); i++) {
		FluidParticle& particle = particles[i];

		int cellX = std::floor(particle.PredictedPosition.x / h);
		int cellY = std::floor(particle.PredictedPosition.y / h);

		uint32_t key = hashCell(cellX, cellY, (uint32_t)particles.size());

		m_FluidCellEntries[i].first = key;
		m_FluidCellEntries[i].second = i;

	}

	std::sort(m_FluidCellEntries.begin(), m_FluidCellEntries.end());

	uint32_t lastSeenNum = -1;

	for (int i = 0; i < m_FluidCellEntries.size(); i++) {
		if (m_FluidCellEntries[i].first != lastSeenNum) {
			m_FluidCellStarts[m_FluidCellEntries[i].first] = i;
			lastSeenNum = m_FluidCellEntries[i].first;
		}
	}
}

void PhysicsSolver::updateFluidNeighborLists()
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;
	if (particles.empty()) return;
	float h = m_PhysicsState->FluidParameters.SmoothingRadius;

	m_FluidNeighborIndices.clear();
	m_FluidNeighborOffsets.clear();
	m_FluidNeighborIndices.reserve(32 * particles.size());
	m_FluidNeighborOffsets.resize(particles.size() + 1);

	for (int i = 0; i < particles.size(); i++) {

		const FluidParticle& particle = particles[i];

		m_FluidNeighborOffsets[i] = m_FluidNeighborIndices.size();

		int cellX = std::floor(particle.PredictedPosition.x / h);
		int cellY = std::floor(particle.PredictedPosition.y / h);

		uint32_t searchedKeys[9];
		int searchedKeyCount = 0;

		for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
			for (int colOffset = -1; colOffset <= 1; colOffset++) {
				int cellToSearchX = cellX + rowOffset;
				int cellToSearchY = cellY + colOffset;

				uint32_t cellKey = hashCell(cellToSearchX, cellToSearchY, (uint32_t)particles.size());

				bool alreadySearched = false;
				for (int k = 0; k < searchedKeyCount; k++) {
					if (searchedKeys[k] == cellKey) {
						alreadySearched = true;
						break;
					}
				}
				if (alreadySearched) continue;

				searchedKeys[searchedKeyCount++] = cellKey;

				uint32_t startParticleIndex = m_FluidCellStarts[cellKey];

				for (uint32_t lookUpIndex = startParticleIndex; lookUpIndex < m_FluidCellEntries.size(); lookUpIndex++) {

					if (m_FluidCellEntries[lookUpIndex].first != cellKey) break;

					uint32_t neighborIndex = m_FluidCellEntries[lookUpIndex].second;

					const FluidParticle& neighbor = particles[neighborIndex];

					glm::vec3 r = particle.PredictedPosition - neighbor.PredictedPosition;
					float r2 = glm::dot(r, r);

					if (r2 < h * h)
						m_FluidNeighborIndices.push_back(neighborIndex);

				}

			}
		}

	}

	m_FluidNeighborOffsets[particles.size()] = m_FluidNeighborIndices.size();
}

void PhysicsSolver::updateBoundaryNeighborLists()
{
	std::vector<FluidParticle>& fluidParticles = m_PhysicsState->FluidParticles;
	std::vector<BoundaryParticle>& boundaryParticles = m_PhysicsState->BoundaryParticles;

	if (fluidParticles.empty()) return;
	float h = m_PhysicsState->FluidParameters.SmoothingRadius;

	m_BoundaryNeighborIndices.clear();
	m_BoundaryNeighborOffsets.clear();
	m_BoundaryNeighborIndices.reserve(32 * boundaryParticles.size());
	m_BoundaryNeighborOffsets.resize(fluidParticles.size() + 1);

	for (int i = 0; i < fluidParticles.size(); i++) {

		const FluidParticle& particle = fluidParticles[i];

		m_BoundaryNeighborOffsets[i] = m_BoundaryNeighborIndices.size();

		if (boundaryParticles.empty()) continue;

		int cellX = std::floor(particle.PredictedPosition.x / h);
		int cellY = std::floor(particle.PredictedPosition.y / h);

		uint32_t searchedKeys[9];
		int searchedKeyCount = 0;

		for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
			for (int colOffset = -1; colOffset <= 1; colOffset++) {
				int cellToSearchX = cellX + rowOffset;
				int cellToSearchY = cellY + colOffset;

				uint32_t cellKey = hashCell(cellToSearchX, cellToSearchY, (uint32_t)boundaryParticles.size());

				bool alreadySearched = false;
				for (int k = 0; k < searchedKeyCount; k++) {
					if (searchedKeys[k] == cellKey) {
						alreadySearched = true;
						break;
					}
				}
				if (alreadySearched) continue;

				searchedKeys[searchedKeyCount++] = cellKey;

				uint32_t startParticleIndex = m_BoundaryCellStarts[cellKey];

				for (uint32_t lookUpIndex = startParticleIndex; lookUpIndex < m_BoundaryCellEntries.size(); lookUpIndex++) {

					if (m_BoundaryCellEntries[lookUpIndex].first != cellKey) break;

					uint32_t neighborIndex = m_BoundaryCellEntries[lookUpIndex].second;

					const BoundaryParticle& neighbor = boundaryParticles[neighborIndex];

					glm::vec3 r = particle.PredictedPosition - neighbor.Position;
					float r2 = glm::dot(r, r);

					if (r2 < h * h)
						m_BoundaryNeighborIndices.push_back(neighborIndex);

				}

			}
		}

	}

	m_BoundaryNeighborOffsets[fluidParticles.size()] = m_BoundaryNeighborIndices.size();
}


void PhysicsSolver::updateGammaCorrectionCoefficients()
{
	GammaTemplate templ = generateGammaCorrectionTemplate();

	updateGamma1(templ);
	updateGamma2(templ);

}

uint32_t PhysicsSolver::hashCell(int x, int y, uint32_t n) {
	constexpr uint32_t p1 = 73856093u;
	constexpr uint32_t p2 = 19349663u;

	uint32_t key = ((uint32_t)x * p1) ^ ((uint32_t)y * p2);

	return key % n;
}

GammaTemplate PhysicsSolver::generateGammaCorrectionTemplate()
{
	//setup template, then compute coefficients.

	GammaTemplate templ;

	const FluidParameters& fluid = m_PhysicsState->FluidParameters;
	float d = fluid.ParticleSpacing;

	int K = std::ceil(fluid.SmoothingRadius / fluid.ParticleSpacing) + 1;

	templ.BoundaryParticles.push_back(BoundaryParticle({ 0, 0, 0 }));

	//push the boundary particles first
	for (int k = 1; k <= K; k++) {
		templ.BoundaryParticles.push_back(BoundaryParticle({ k * d, 0, 0 }));
		templ.BoundaryParticles.push_back(BoundaryParticle({ -k * d, 0, 0 }));
	}

	//push the fluid particles next. test particle is 1 above wall, so use K+1
	for (int row = 1; row <= K + 1; row++) {
		templ.FluidParticles.push_back(FluidParticle({ 0, row * d, 0 }));

		for (int k = 1; k <= K; k++) {
			templ.FluidParticles.push_back(FluidParticle({ k * d, row * d, 0 }));
			templ.FluidParticles.push_back(FluidParticle({ -k * d, row * d, 0 }));
		}
	}

	return templ;
}

void PhysicsSolver::updateGamma1(const GammaTemplate& templ)
{
	const FluidParticle& testParticle = templ.FluidParticles[0];
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float d = fluid.ParticleSpacing;
	float h = fluid.SmoothingRadius;
	float h2 = h * h;
	float h8 = h2 * h2 * h2 * h2;
	float coeff = 4.0f / (PI * h8);
	float particleMass = fluid.ParticleMass;

	float c1 = 1 / (d * d);

	float c2 = 0;

	for (const FluidParticle& fluidParticle : templ.FluidParticles) {

		glm::vec3 r = testParticle.Position - fluidParticle.Position;
		float r2 = glm::dot(r, r);

		c2 += poly6Kernel(r2, h2, coeff);

	}

	float c3 = 0;

	for (const BoundaryParticle& boundaryParticle : templ.BoundaryParticles) {

		glm::vec3 r = testParticle.Position - boundaryParticle.Position;

		float r2 = glm::dot(r, r);

		c3 += poly6Kernel(r2, h2, coeff);
	}

	fluid.Gamma1 = (c1 - c2) / c3;
}

void PhysicsSolver::updateGamma2(const GammaTemplate& templ)
{
	const FluidParticle& testParticle = templ.FluidParticles[0];
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float h = fluid.SmoothingRadius;
	float h2 = h * h;
	float h5 = h2 * h2 * h;
	float h8 = h2 * h2 * h2 * h2;
	float coeff = -30.0f / (PI * h5);

	glm::vec3 c1 = {};

	for (const FluidParticle& fluidParticle : templ.FluidParticles) {

		glm::vec3 r = testParticle.Position - fluidParticle.Position;
		float r2 = glm::dot(r, r);

		c1 += spikyKernelGradient(r, r2, h, h2, coeff);

	}

	glm::vec3 c2{};

	for (const BoundaryParticle& boundaryParticle : templ.BoundaryParticles) {

		glm::vec3 r = testParticle.Position - boundaryParticle.Position;

		float r2 = glm::dot(r, r);

		c2 += spikyKernelGradient(r, r2, h, h2, coeff);
	}

	float numerator = glm::dot(c1, c2);
	float denominator = glm::dot(c2, c2);

	fluid.Gamma2 = -numerator / denominator;

}

float PhysicsSolver::poly6Kernel(float r2, float h2, float coeff)
{
	if (r2 >= h2) return 0.0f;

	float diff = h2 - r2;

	return coeff * diff * diff * diff;
}

glm::vec3 PhysicsSolver::poly6KernelGradient(glm::vec3 r, float r2, float h2, float coeff)
{
	if (r2 >= h2) return glm::vec3(0);

	float diff = h2 - r2;

	return coeff * diff * diff * r;
}

float PhysicsSolver::poly6KernelLaplacian(float r2, float h2, float coeff)
{
	if (r2 >= h2) return 0.0f;

	float diff = h2 - r2;

	return coeff * diff * (h2 - 3.0f * r2);
}

glm::vec3 PhysicsSolver::spikyKernelGradient(glm::vec3 r, float r2, float h, float h2, float coeff)
{
	if (r2 == 0.0f || r2 >= h2) return glm::vec3(0);

	float dst = std::sqrt(r2);

	float diff = h - dst;

	return coeff * diff * diff * (r / dst);
}

float PhysicsSolver::viscosityLaplacianKernel2D(float r2, float h, float h2, float coeff)
{
	if (r2 == 0.0f || r2 >= h2) return 0.0;

	float dst = std::sqrt(r2);
	float diff = h - dst;

	return coeff * diff;
}


