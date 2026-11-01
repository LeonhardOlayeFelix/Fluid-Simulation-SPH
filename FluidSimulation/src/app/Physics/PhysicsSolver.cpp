#include "PhysicsSolver.h"
#include "Particle.h"

PhysicsSolver::PhysicsSolver(std::shared_ptr<PhysicsState> physicsState) : m_PhysicsState(physicsState)
{
}

void PhysicsSolver::Update(double timeStep)
{
	AdvanceParticles(timeStep);
}

void PhysicsSolver::AdvanceParticles(double timeStep)
{
	UpdateParticleDensities(m_PhysicsState->FluidParameters.SmoothingRadius);
	UpdateParticleDensityGradients(m_PhysicsState->FluidParameters.SmoothingRadius);
	UpdateParticlePressures();
	UpdateParticlePressureGradients(m_PhysicsState->FluidParameters.SmoothingRadius);

	if (m_PhysicsState->FluidParameters.SimulationRunning)
	{
		for (Particle& particle : m_PhysicsState->Particles)
		{
			particle.RecalculateForceDensity();
			particle.Acceleration = particle.ForceDensity / particle.Density;
			particle.Velocity += (float)timeStep * particle.Acceleration;
			particle.Position += (float)timeStep * particle.Velocity;

			DetectBoundaryCollision(particle);
		}
	}
}

void PhysicsSolver::UpdateParticleDensities(float h)
{
	float h2 = h * h;
	float h4 = h2 * h2;
	float h8 = h2 * h2 * h2 * h2;
	float coeff = 4.0f / (PI * h8);
	float coeff2 = 6.0f / (PI * h4);

	for (int i = 0; i < m_PhysicsState->Particles.size(); i++)
	{
		Particle& source = m_PhysicsState->Particles[i];
		source.Density = 0;
		for (int j = 0; j < m_PhysicsState->Particles.size(); j++)
		{
			const Particle& neighbour = m_PhysicsState->Particles[j];

			glm::vec3 d = source.Position - neighbour.Position;
			float r2 = glm::dot(d, d);

			source.Density += neighbour.Mass * poly6Kernel(r2, h2, coeff);
		}
	}
}
void PhysicsSolver::UpdateParticleDensityGradients(float h)
{
	float h2 = h * h;
	float h4 = h2 * h2;
	float h8 = h2 * h2 * h2 * h2;
	float coeff = -24.0f / (PI * h8);
	float coeff2 = -12.0f / (h4 * PI);

	for (int i = 0; i < m_PhysicsState->Particles.size(); i++)
	{
		Particle& source = m_PhysicsState->Particles[i];
		source.DensityGradient = glm::vec3(0);
		for (int j = 0; j < m_PhysicsState->Particles.size(); j++)
		{
			const Particle& neighbour = m_PhysicsState->Particles[j];

			glm::vec3 r = source.Position - neighbour.Position;
			float r2 = glm::dot(r, r);

			source.DensityGradient += neighbour.Mass * poly6KernelGradient(r, r2, h2, coeff);
		}
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


void PhysicsSolver::UpdateParticlePressureGradients(float h)
{
	std::vector<Particle>& particles = m_PhysicsState->Particles;

	float h2 = h * h;
	float h5 = h2 * h2 * h;
	float h8 = h2 * h2 * h2 * h2;
	float coeff = -30.0f / (PI * h5);


	for (int i = 0; i < particles.size(); i++) {
		Particle& source = particles[i];
		source.PressureGradient = glm::vec3(0);

		for (int j = 0; j < particles.size(); j++) {

			const Particle& neighbour = m_PhysicsState->Particles[j];

			glm::vec3 r = source.Position - neighbour.Position;
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
float PhysicsSolver::kernel1(float r2, float h, float h2, float coeff)
{
	if (r2 >= h2) return 0.0f;

	float dst = h - std::sqrt(r2);

	return coeff * dst * dst;
}
glm::vec3 PhysicsSolver::kernel1Gradient(glm::vec3 r, float r2, float h, float h2, float coeff)
{
	if (r2 == 0.0f || r2 >= h2) return glm::vec3(0);

	float dst = std::sqrt(r2);

	return coeff * (h - dst) * (r / dst);
}

glm::vec3 PhysicsSolver::spikyKernelGradient(glm::vec3 r, float r2, float h, float h2, float coeff)
{
	if (r2 == 0.0f || r2 >= h2) return glm::vec3(0);

	float dst = std::sqrt(r2);
	float diff = h - dst;

	return coeff * diff * diff * (r / dst);
}
