#include "PhysicsSolver.h"
#include "Particle.h"
#include <cstdint>
#include <algorithm>
#include <utility>

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
	UpdateParticlePressureForceDensities();
	UpdateParticleViscosityForceDensities();
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
		particle.Velocity = particle.Velocity + dt * glm::vec3(0, Gravity, 0);
		particle.PredictedPosition = particle.Position + dt * particle.Velocity;
	}

}

void PhysicsSolver::UpdateActualPositions(double timeStep)
{
	float dt = (float)timeStep;
	std::vector<Particle>& particles = m_PhysicsState->Particles;

	for (Particle& particle : particles)
	{
		glm::vec3 f_i = particle.RecalculateResultantForceDensity();
		particle.Acceleration = f_i / particle.Density;
		particle.Velocity += dt * particle.Acceleration;
		particle.Position += dt * particle.Velocity;

		DetectBoundaryCollision(particle);
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

		ForEachNeighbour(pi.PredictedPosition, [&](const Particle& pj) {

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
		source.Pressure = std::max(0.0f, stiffness * (source.Density - restDensity));
		//source.Pressure = stiffness * (source.Density - restDensity);
	}
}

void PhysicsSolver::UpdateParticlePressureForceDensities()
{
	std::vector<Particle>& particles = m_PhysicsState->Particles;

	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	float h2 = h * h;
	float h5 = h2 * h2 * h;
	float h8 = h2 * h2 * h2 * h2;
	float coeff = -30.0f / (PI * h5);

	for (Particle& pi : particles) {

		pi.PressureForceDensity = glm::vec3(0);

		ForEachNeighbour(pi.PredictedPosition, [&](const Particle& pj) {

			glm::vec3 r = pi.PredictedPosition - pj.PredictedPosition;
			float r2 = glm::dot(r, r);

			float symmetrisedPressure = (pi.Pressure + pj.Pressure) / 2;

			pi.PressureForceDensity += pj.Mass * symmetrisedPressure / pj.Density * spikyKernelGradient(r, r2, h, h2, coeff);

			});

		pi.PressureForceDensity = -pi.PressureForceDensity;

	}
}

void PhysicsSolver::UpdateParticleViscosityForceDensities()
{
	std::vector<Particle>& particles = m_PhysicsState->Particles;

	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	float h2 = h * h;
	float h5 = h2 * h2 * h;
	float coeff = 40 / (PI * h5);


	for (Particle& pi : particles) {

		pi.ViscousForceDensity = glm::vec3(0);

		ForEachNeighbour(pi.PredictedPosition, [&](const Particle& pj) {

			glm::vec3 r = pi.PredictedPosition - pj.PredictedPosition;
			float r2 = glm::dot(r, r);

			glm::vec3 symmetrisedVelocity = (pj.Velocity - pi.Velocity);

			pi.ViscousForceDensity += pj.Mass * symmetrisedVelocity / pj.Density * viscosityLaplacianKernel2D(r2, h, h2, coeff);

			});

		pi.ViscousForceDensity *= m_PhysicsState->FluidParameters.Viscosity;

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

float PhysicsSolver::viscosityLaplacianKernel2D(float r2, float h, float h2, float coeff)
{
	if (r2 == 0.0f || r2 >= h2) return 0.0;

	float dst = std::sqrt(r2);
	float diff = h - dst;

	return coeff * diff;
}


