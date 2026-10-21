#include "PhysicsSolver.h"
#include "Particle.h"

void PhysicsSolver::AdvanceParticles(std::vector<Particle>& particles, double timeStep, BoundaryData& boundaryConfig)
{
	//UpdateParticleDensities(particles, parameters);

	for (Particle& particle : particles) AdvanceParticle(particle, timeStep, boundaryConfig);
}

void PhysicsSolver::AdvanceParticle(Particle& particle, double timeStep, BoundaryData& boundaryConfig)
{
	particle.Velocity += (float) timeStep * glm::vec3(0, Gravity, 0);
	particle.Position += (float) timeStep * particle.Velocity;

	DetectBoundaryCollision(particle, boundaryConfig);
}

void PhysicsSolver::UpdateParticleDensities(std::vector<Particle>& particles)
{
	for (int i = 0; i < particles.size(); i++) {
		Particle& source = particles[i];
		source.Density = 0;
		for (int j = 0; j < particles.size(); j++) {
			const Particle& neighbour = particles[j];
			float r = glm::length(source.Position - neighbour.Position);
			float smoothingFactor = DensityKernel2d(r, 20);
			float volumeFactor = neighbour.Mass;
			source.Density += smoothingFactor * volumeFactor;

		}
	}
}

float PhysicsSolver::DensityKernel2d(float r, float h)
{
	return 0.0f;
}

void PhysicsSolver::DetectBoundaryCollision(Particle& particle, BoundaryData& boundaryConfig)
{
	if (particle.Position.y - BallRadius <= 0.0f)
	{
		particle.Position.y = BallRadius;
		particle.Velocity.y = -particle.Velocity.y * CollisionDampingFactor;
	}
	else if (particle.Position.y + BallRadius >= boundaryConfig.ViewportSize.y) {
		particle.Position.y = boundaryConfig.ViewportSize.y - BallRadius;
		particle.Velocity.y = -particle.Velocity.y * CollisionDampingFactor;
	}

	if (particle.Position.x - BallRadius <= 0.0f)
	{
		particle.Position.x = BallRadius;
		particle.Velocity.x = -particle.Velocity.x * CollisionDampingFactor;
	}
	else if (particle.Position.x + BallRadius >= boundaryConfig.ViewportSize.x) {
		particle.Position.x = boundaryConfig.ViewportSize.x - BallRadius;
		particle.Velocity.x = -particle.Velocity.x * CollisionDampingFactor;
	}
}
