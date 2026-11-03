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

uint32_t hashCell(int x, int y, uint32_t n) {
	constexpr uint32_t p1 = 73856093u;
	constexpr uint32_t p2 = 19349663u;

	uint32_t key = ((uint32_t)x * p1) ^ ((uint32_t)y * p2);

	return key % n;
}

std::vector<std::pair<int, int>> getOwnAndNeighboringCells(int x, int y) {
	return { {x, y}, { x - 1, y }, {x + 1, y}, {x, y - 1}, {x, y + 1}, {x - 1, y - 1}, {x + 1, y + 1}, {x + 1, y - 1}, {x - 1, y + 1} };
}

void PhysicsSolver::AdvanceParticles(double timeStep)
{
	if (!m_PhysicsState->FluidParameters.SimulationRunning)
		return;

	std::vector<Particle>& particles = m_PhysicsState->Particles;

	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	float dt = (float)timeStep;

	for (Particle& particle : particles)
	{
		particle.PredictedPosition = particle.Position + dt * particle.Velocity;
	}

	std::vector<std::pair<uint32_t, uint32_t>> spatialLookup(particles.size()); //query with particle index. Stores a list fo (Cell key, Particle Index) pairs

	for (int i = 0; i < spatialLookup.size(); i++) {
		Particle& particle = particles[i];

		int cellX = std::floor(particle.Position.x / h);
		int cellY = std::floor(particle.Position.y / h);

		uint32_t key = hashCell(cellX, cellY, (uint32_t)particles.size());

		spatialLookup[i].first = key;
		spatialLookup[i].second = i;
		
	}

	std::sort(spatialLookup.begin(), spatialLookup.end());

	std::vector<uint32_t> startIndices(particles.size(), UINT32_MAX); //query with cell key. Stores a list of (indicies) at which cells start appearing in spatialLookup

	uint32_t lastSeenNum = -1;

	for (int i = 0; i < spatialLookup.size(); i++) {
		if (spatialLookup[i].first != lastSeenNum) {
			startIndices[spatialLookup[i].first] = i;
			lastSeenNum = spatialLookup[i].first;
		}
	}


	UpdateParticleDensities(spatialLookup, startIndices);
	UpdateParticlePressures(spatialLookup, startIndices);
	UpdateParticlePressureGradients(spatialLookup, startIndices);

	for (Particle& particle : particles)
	{
		particle.RecalculateForceDensity();
		particle.Acceleration = particle.ForceDensity / particle.Density;
		particle.Velocity += dt * particle.Acceleration;
		particle.Position += dt * particle.Velocity;

		DetectBoundaryCollision(particle);
	}
}

//note to self - this could be changed to be a template function to better more performant.
void PhysicsSolver::ForEachNeighbour(const glm::vec3& position, std::function<void(const Particle&)> callback, const std::vector<std::pair<uint32_t, uint32_t>>& spatialLookup, const std::vector<uint32_t>& startIndices) {

	//function which searches all the neighbors of a particle using a spatialLookup and StartIndices array.

	std::vector<Particle>& particles = m_PhysicsState->Particles;
	float h = m_PhysicsState->FluidParameters.SmoothingRadius;

	int cellX = std::floor(position.x / h);
	int cellY = std::floor(position.y / h);

	for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
		for (int colOffset = -1; colOffset <= 1; colOffset++) {
			int cellToSearchX = cellX + rowOffset;
			int cellToSearchY = cellY + colOffset;

			uint32_t cellKey = hashCell(cellToSearchX, cellToSearchY, (uint32_t)particles.size());

			uint32_t startParticleIndex = startIndices[cellKey];

			for (uint32_t i = startParticleIndex; i < spatialLookup.size(); i++) {

				if (spatialLookup[i].first != cellKey) break;

				const Particle& pj = particles[spatialLookup[i].second];

				callback(pj);

			}

		}
	}

}

void PhysicsSolver::UpdateParticleDensities(const std::vector<std::pair<uint32_t, uint32_t>>& spatialLookup, const std::vector<uint32_t>& startIndices)
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

			}, spatialLookup, startIndices);

	}
}

void PhysicsSolver::UpdateParticlePressures(const std::vector<std::pair<uint32_t, uint32_t>>& spatialLookup, const std::vector<uint32_t>& startIndices)
{
	float stiffness = m_PhysicsState->FluidParameters.FluidStiffness;
	float restDensity = m_PhysicsState->FluidParameters.RestDensity;

	for (int i = 0; i < m_PhysicsState->Particles.size(); i++) {
		Particle& source = m_PhysicsState->Particles[i];
		source.Pressure = stiffness * (source.Density - restDensity);
	}
}

void PhysicsSolver::UpdateParticlePressureGradients(const std::vector<std::pair<uint32_t, uint32_t>>& spatialLookup, const std::vector<uint32_t>& startIndices)
{
	std::vector<Particle>& particles = m_PhysicsState->Particles;

	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	float h2 = h * h;
	float h5 = h2 * h2 * h;
	float h8 = h2 * h2 * h2 * h2;
	float coeff = -30.0f / (PI * h5);


	for (int i = 0; i < particles.size(); i++) {
		Particle& source = particles[i];
		source.PressureGradient = glm::vec3(0);

		for (int j = 0; j < particles.size(); j++) {

			const Particle& neighbour = m_PhysicsState->Particles[j];

			glm::vec3 r = source.PredictedPosition - neighbour.PredictedPosition;
			float r2 = glm::dot(r, r);

			float sharedPressure = CalculateSymmetrisedPressure(source, neighbour);

			source.PressureGradient += neighbour.Mass * sharedPressure / neighbour.Density * spikyKernelGradient(r, r2, h, h2, coeff);
		}
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

float PhysicsSolver::CalculateSymmetrisedPressure(const Particle& source, const Particle& neighbour)
{
	return (source.Pressure + neighbour.Pressure) / 2;
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
