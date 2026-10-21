#include "PhysicsSolver.h"
#include "Particle.h"

void PhysicsSolver::AdvanceParticles(std::vector<Particle>& particles, double timeStep, BoundaryParameters& boundaryParameters, FluidParameters& fluidParameters)
{
	UpdateParticleDensities(particles, fluidParameters.SmoothingRadius);

	if (fluidParameters.SimulationRunning)
		for (Particle& particle : particles) AdvanceParticle(particle, timeStep, boundaryParameters);
}

void PhysicsSolver::AdvanceParticle(Particle& particle, double timeStep, BoundaryParameters& boundaryParameters)
{
	particle.Velocity += (float) timeStep * glm::vec3(0, Gravity, 0);
	particle.Position += (float) timeStep * particle.Velocity;

	DetectBoundaryCollision(particle, boundaryParameters);
}

void PhysicsSolver::UpdateParticleDensities(std::vector<Particle>& particles, float h)
{
	float h2 = h * h;
	float h8 = h2 * h2 * h2 * h2;
	float poly6Coeff = 4.0f / (PI * h8);

	for (int i = 0; i < particles.size(); i++) {
		Particle& source = particles[i];
		source.Density = 0;
		for (int j = 0; j < particles.size(); j++) {
			const Particle& neighbour = particles[j];

			glm::vec3 d = source.Position - neighbour.Position;
			float r2 = glm::dot(d, d);

			source.Density += neighbour.Mass * poly6Kernel(r2, h2, poly6Coeff);
		}
	}
}

float PhysicsSolver::poly6Kernel(float r2, float h2, float coeff)
{
	if (r2 >= h2) return 0.0f;
	float diff = h2 - r2;
	return coeff * diff * diff * diff;
}

void PhysicsSolver::DetectBoundaryCollision(Particle& particle, BoundaryParameters& boundaryParameters)
{
	if (particle.Position.y - BallRadius <= 0.0f)
	{
		particle.Position.y = BallRadius;
		particle.Velocity.y = -particle.Velocity.y * CollisionDampingFactor;
	}
	else if (particle.Position.y + BallRadius >= boundaryParameters.ViewportSize.y) {
		particle.Position.y = boundaryParameters.ViewportSize.y - BallRadius;
		particle.Velocity.y = -particle.Velocity.y * CollisionDampingFactor;
	}

	if (particle.Position.x - BallRadius <= 0.0f)
	{
		particle.Position.x = BallRadius;
		particle.Velocity.x = -particle.Velocity.x * CollisionDampingFactor;
	}
	else if (particle.Position.x + BallRadius >= boundaryParameters.ViewportSize.x) {
		particle.Position.x = boundaryParameters.ViewportSize.x - BallRadius;
		particle.Velocity.x = -particle.Velocity.x * CollisionDampingFactor;
	}
}
