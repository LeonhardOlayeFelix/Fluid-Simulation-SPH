#include "PhysicsSolver.h"
#include "Particle.h"
#include <cstdint>
#include <algorithm>

PhysicsSolver::PhysicsSolver(std::shared_ptr<PhysicsState> physicsState) : m_PhysicsState(physicsState)
{
}

void PhysicsSolver::Update(double timeStep)
{
	AdvanceParticles(timeStep);
}

uint32_t PhysicsSolver::HashCell(int x, int y, uint32_t n) {
	constexpr uint32_t p1 = 73856093u;
	constexpr uint32_t p2 = 19349663u;

	uint32_t key = ((uint32_t)x * p1) ^ ((uint32_t)y * p2);

	return key % n;
}

void PhysicsSolver::AdvanceParticles(double timeStep)
{
	if (!m_PhysicsState->FluidParameters.SimulationRunning || m_PhysicsState->Particles.size() == 0) return;

	UpdatePredictedPositions(timeStep);
	UpdateGridSearch();
	UpdateParticleDensities();
	UpdateParticlePressures();
	UpdateParticlePressureGradients();
	UpdateActualPositions(timeStep);
}

void PhysicsSolver::UpdateGridSearch()
{
	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	std::vector<Particle>& particles = m_PhysicsState->Particles;

	m_SpatialLookup.resize(particles.size());
	m_StartIndices.assign(particles.size(), UINT32_MAX);

	//note to self- need to fix case where squares within the same 9 are mapped to the same key, so they are counted twice. Rare, but important.

	for (int i = 0; i < m_SpatialLookup.size(); i++) {
		Particle& particle = particles[i];

		int cellX = std::floor(particle.PredictedPosition.x / h);
		int cellY = std::floor(particle.PredictedPosition.y / h);

		uint32_t key = HashCell(cellX, cellY, (uint32_t)particles.size());

		m_SpatialLookup[i].first = key;
		m_SpatialLookup[i].second = i;

	}

	std::sort(m_SpatialLookup.begin(), m_SpatialLookup.end());


	uint32_t lastSeenNum = -1;

	for (int i = 0; i < m_SpatialLookup.size(); i++) {
		if (m_SpatialLookup[i].first != lastSeenNum) {
			m_StartIndices[m_SpatialLookup[i].first] = i;
			lastSeenNum = m_SpatialLookup[i].first;
		}
	}

}

void PhysicsSolver::UpdatePredictedPositions(double timeStep)
{
	std::vector<Particle>& particles = m_PhysicsState->Particles;

	float dt = (float)timeStep;

	for (Particle& particle : particles)
	{
		particle.PredictedPosition = particle.Position + dt * particle.Velocity;
	}

}

//note to self - this could be changed to be a template function to better more performant.
void PhysicsSolver::ForEachNeighbour(const glm::vec3& position, std::function<void(const Particle&)> callback) {

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

void PhysicsSolver::UpdateParticleDensities()
{
	std::vector<Particle>& particles = m_PhysicsState->Particles;
	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	float h2 = h * h;
	float h8 = h2 * h2 * h2 * h2;
	float coeff = 4.0f / (PI * h8);

	for (Particle& pi : particles) {

		pi.Density = 0;

		ForEachNeighbour2(pi.PredictedPosition, [&](const Particle& pj) {

				glm::vec3 d = pi.PredictedPosition - pj.PredictedPosition;
				float r2 = glm::dot(d, d);

				pi.Density += pj.Mass * poly6Kernel(r2, h2, coeff);

			});

	}
}

void PhysicsSolver::UpdateParticlePressures()
{
	float stiffness = m_PhysicsState->FluidParameters.FluidStiffness;
	float restDensity = m_PhysicsState->FluidParameters.RestDensity;

	for (int i = 0; i < m_PhysicsState->Particles.size(); i++) {
		Particle& source = m_PhysicsState->Particles[i];
		source.Pressure = stiffness * (source.Density - restDensity);
	}
}

void PhysicsSolver::UpdateParticlePressureGradients()
{
	std::vector<Particle>& particles = m_PhysicsState->Particles;

	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	float h2 = h * h;
	float h5 = h2 * h2 * h;
	float h8 = h2 * h2 * h2 * h2;
	float coeff = -30.0f / (PI * h5);

	for (Particle& pi : particles) {

		pi.PressureGradient = glm::vec3(0);

		ForEachNeighbour2(pi.PredictedPosition, [&](const Particle& pj) {

			glm::vec3 r = pi.PredictedPosition - pj.PredictedPosition;
			float r2 = glm::dot(r, r);

			float symmetrisedPressure = (pi.Pressure + pj.Pressure) / 2;

			pi.PressureGradient += pj.Mass * symmetrisedPressure / pj.Density * spikyKernelGradient(r, r2, h, h2, coeff);

			});

	}
}

void PhysicsSolver::UpdateActualPositions(double timeStep)
{
	float dt = (float)timeStep;
	std::vector<Particle>& particles = m_PhysicsState->Particles;

	for (Particle& particle : particles)
	{
		particle.RecalculateForceDensity();
		particle.Acceleration = particle.ForceDensity / particle.Density;
		particle.Velocity += dt * particle.Acceleration;
		particle.Position += dt * particle.Velocity;

		DetectBoundaryCollision(particle);
	}
}

void PhysicsSolver::DetectBoundaryCollision(Particle& particle)
{
	const BoundaryParameters& bounds = m_PhysicsState->BoundaryParameters;

	if (particle.Position.y - BallRadius <= 0.0f)
	{
		particle.Position.y = BallRadius;
		particle.Velocity.y = -particle.Velocity.y * CollisionDampingFactor;
	}
	else if (particle.Position.y + BallRadius >= bounds.ViewportSize.y)
	{
		particle.Position.y = bounds.ViewportSize.y - BallRadius;
		particle.Velocity.y = -particle.Velocity.y * CollisionDampingFactor;
	}

	if (particle.Position.x - BallRadius <= 0.0f)
	{
		particle.Position.x = BallRadius;
		particle.Velocity.x = -particle.Velocity.x * CollisionDampingFactor;
	}
	else if (particle.Position.x + BallRadius >= bounds.ViewportSize.x)
	{
		particle.Position.x = bounds.ViewportSize.x - BallRadius;
		particle.Velocity.x = -particle.Velocity.x * CollisionDampingFactor;
	}
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
glm::vec3 PhysicsSolver::spikyKernelGradient(glm::vec3 r, float r2, float h, float h2, float coeff)
{
	if (r2 == 0.0f || r2 >= h2) return glm::vec3(0);

	float dst = std::sqrt(r2);

	float diff = h - dst;

	return coeff * diff * diff * (r / dst);
}
