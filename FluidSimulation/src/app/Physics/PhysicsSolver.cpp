#include "PhysicsSolver.h"
#include "FluidParticle.h"
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
	if (!m_PhysicsState->FluidParameters.SimulationRunning || m_PhysicsState->FluidParticles.size() == 0) return;

	UpdatePredictedPositions(timeStep);

	if (m_PhysicsState->BoundaryParameters.Stale) 
	{
		UpdateBoundaryParticleGridSearch();
		UpdateGammaCorrectionCoefficients();
	}

	UpdateFluidParticleGridSearch();
	UpdateParticleDensities();
	UpdateParticlePressures();
	UpdateParticlePressureForceDensities();
	UpdateParticleViscosityForceDensities();
	UpdateActualPositions(timeStep);

	m_PhysicsState->BoundaryParameters.Stale = false;
}

void PhysicsSolver::UpdateBoundaryParticleGridSearch()
{
	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	std::vector<BoundaryParticle>& particles = m_PhysicsState->BoundaryParticles;

	m_BoundarySpatialLookup.resize(particles.size());
	m_BoundaryStartIndices.assign(particles.size(), UINT32_MAX);

	for (int i = 0; i < m_BoundarySpatialLookup.size(); i++) {
		BoundaryParticle& particle = particles[i];

		int cellX = std::floor(particle.Position.x / h);
		int cellY = std::floor(particle.Position.y / h);

		uint32_t key = HashCell(cellX, cellY, (uint32_t)particles.size());

		m_BoundarySpatialLookup[i].first = key;
		m_BoundarySpatialLookup[i].second = i;

	}

	std::sort(m_BoundarySpatialLookup.begin(), m_BoundarySpatialLookup.end());

	uint32_t lastSeenNum = -1;

	for (int i = 0; i < m_BoundarySpatialLookup.size(); i++) {
		if (m_BoundarySpatialLookup[i].first != lastSeenNum) {
			m_BoundaryStartIndices[m_BoundarySpatialLookup[i].first] = i;
			lastSeenNum = m_BoundarySpatialLookup[i].first;
		}
	}
}

void PhysicsSolver::UpdateFluidParticleGridSearch()
{
	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;

	m_FluidSpatialLookup.resize(particles.size());
	m_FluidStartIndices.assign(particles.size(), UINT32_MAX);

	for (int i = 0; i < m_FluidSpatialLookup.size(); i++) {
		FluidParticle& particle = particles[i];

		int cellX = std::floor(particle.PredictedPosition.x / h);
		int cellY = std::floor(particle.PredictedPosition.y / h);

		uint32_t key = HashCell(cellX, cellY, (uint32_t)particles.size());

		m_FluidSpatialLookup[i].first = key;
		m_FluidSpatialLookup[i].second = i;

	}

	std::sort(m_FluidSpatialLookup.begin(), m_FluidSpatialLookup.end());

	uint32_t lastSeenNum = -1;

	for (int i = 0; i < m_FluidSpatialLookup.size(); i++) {
		if (m_FluidSpatialLookup[i].first != lastSeenNum) {
			m_FluidStartIndices[m_FluidSpatialLookup[i].first] = i;
			lastSeenNum = m_FluidSpatialLookup[i].first;
		}
	}
}

void PhysicsSolver::UpdateGammaCorrectionCoefficients()
{
	GammaTemplate templ = GenerateGammaCorrectionTemplate();

	UpdateGamma1(templ);
	UpdateGamma2(templ);

}

GammaTemplate PhysicsSolver::GenerateGammaCorrectionTemplate()
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

void PhysicsSolver::UpdateGamma1(const GammaTemplate& templ)
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

void PhysicsSolver::UpdateGamma2(const GammaTemplate & templ)
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

	fluid.Gamma2 = - numerator / denominator;

}

void PhysicsSolver::UpdatePredictedPositions(double timeStep)
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;

	float dt = (float)timeStep;

	for (FluidParticle& particle : particles)
	{
		particle.Velocity = particle.Velocity + dt * glm::vec3(0, Gravity, 0);
		particle.PredictedPosition = particle.Position + dt * particle.Velocity;
	}

}

void PhysicsSolver::UpdateActualPositions(double timeStep)
{
	float dt = (float)timeStep;
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;

	for (FluidParticle& particle : particles)
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
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float h = fluid.SmoothingRadius;
	float h2 = h * h;
	float h8 = h2 * h2 * h2 * h2;
	float coeff = 4.0f / (PI * h8);
	float particleMass = fluid.ParticleMass;

	for (FluidParticle& pi : particles) {

		float fluidTerm = 0;

		ForEachFluidNeighbour(pi.PredictedPosition, [&](const FluidParticle& pj) {

				glm::vec3 d = pi.PredictedPosition - pj.PredictedPosition;
				float r2 = glm::dot(d, d);

				fluidTerm += poly6Kernel(r2, h2, coeff);

			});

		fluidTerm *= particleMass;

		float boundaryTerm = 0;

		if (!fluid.IgnoreBoundaryParticles) 
		{
			ForEachBoundaryNeighbour(pi.PredictedPosition, [&](const BoundaryParticle& pj) {

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
	float stiffness = m_PhysicsState->FluidParameters.FluidStiffness;
	float restDensity = m_PhysicsState->FluidParameters.RestDensity;

	for (int i = 0; i < m_PhysicsState->FluidParticles.size(); i++) {
		FluidParticle& source = m_PhysicsState->FluidParticles[i];
		source.Pressure = std::max(0.0f, stiffness * (source.Density - restDensity));
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

	for (FluidParticle& pi : particles) {

		glm::vec3 fluidTerm{};

		ForEachFluidNeighbour(pi.PredictedPosition, [&](const FluidParticle& pj) {

			glm::vec3 r = pi.PredictedPosition - pj.PredictedPosition;
			float r2 = glm::dot(r, r);

			float symmetrisedPressure = pi.Pressure / (pi.Density * pi.Density) + pj.Pressure / (pj.Density * pj.Density);

			fluidTerm += symmetrisedPressure * spikyKernelGradient(r, r2, h, h2, coeff); 

			});

		fluidTerm *= particleMass;

		glm::vec3 boundaryTerm{};

		if (!fluid.IgnoreBoundaryParticles)
		{

			ForEachBoundaryNeighbour(pi.PredictedPosition, [&](const BoundaryParticle& pj) {

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

	for (FluidParticle& pi : particles) {

		glm::vec3 summation{};

		ForEachFluidNeighbour(pi.PredictedPosition, [&](const FluidParticle& pj) {

			glm::vec3 r = pi.PredictedPosition - pj.PredictedPosition;
			float r2 = glm::dot(r, r);

			glm::vec3 symmetrisedVelocity = (pj.Velocity - pi.Velocity);

			summation += symmetrisedVelocity / pj.Density * viscosityLaplacianKernel2D(r2, h, h2, coeff);

			});

		pi.ViscousForceDensity = fluid.Viscosity * particleMass * summation;

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


