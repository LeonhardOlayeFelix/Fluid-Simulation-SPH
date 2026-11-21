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
	if (!m_PhysicsState->FluidParameters.SimulationRunning || m_PhysicsState->FluidParticles.size() == 0) return;

	UpdateNeighborGridSearch();

	UpdateParticleDensities();

	UpdateParticleNonPressureForceDensities();

	UpdatePredictedVelocities(timeStep);

	UpdatePredictedDensities(timeStep);

	UpdateParticlePressures(timeStep);

	UpdateParticlePressureForceDensities();

	IntegrateParticles(timeStep);

	m_PhysicsState->BoundaryParameters.Stale = false;
}

void PhysicsSolver::UpdateNeighborGridSearch()
{
	if (m_PhysicsState->BoundaryParameters.Stale)
	{
		updateBoundaryParticleNeighborSearchGrid();
		UpdateGammaCorrectionCoefficients();
	}

	updateFluidParticleNeighborGridSearch();
}

void PhysicsSolver::UpdateGammaCorrectionCoefficients()
{
	GammaTemplate templ = generateGammaCorrectionTemplate();

	updateGamma1(templ);
	updateGamma2(templ);
}

void PhysicsSolver::UpdateParticleDensities()
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float h = fluid.SmoothingRadius;
	float h2 = h * h;
	float coeff = 40 / (7 * PI * h2);
	float particleMass = fluid.ParticleMass;

	for (FluidParticle& pi : particles) {

		float fluidTerm = 0;

		ForEachFluidNeighbour(pi.Position, [&](const FluidParticle& pj) {

			glm::vec3 d = pi.Position - pj.Position;

			float r = glm::length(d);

			fluidTerm += cubicSplineKernel(r, h, coeff);

			});

		fluidTerm *= particleMass;

		float boundaryTerm = 0;

		if (!fluid.IgnoreBoundaryParticles)
		{
			ForEachBoundaryNeighbour(pi.Position, [&](const BoundaryParticle& pj) {

				glm::vec3 d = pi.Position - pj.Position;
				float r = glm::length(d);

				boundaryTerm += cubicSplineKernel(r, h, coeff);
				});
		}

		boundaryTerm *= particleMass;

		pi.Density = fluidTerm + fluid.Gamma1 * boundaryTerm;

	}
}

void PhysicsSolver::UpdateParticleNonPressureForceDensities()
{
	UpdateParticleViscosityForceDensities();
	UpdateParticleGravityForceDensities();
	UpdateParticleSurfaceTensionForceDensities();
}

void PhysicsSolver::UpdateParticleViscosityForceDensities()
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float h = fluid.SmoothingRadius;
	float h2 = h * h;
	float h3 = h2 * h;
	float coeff = 240 / (7 * PI * h3);
	float particleMass = fluid.ParticleMass;
	const float dimFactor = 2.0f * (2 + 2);

	for (FluidParticle& pi : particles) {

		glm::vec3 laplacian{};

		ForEachFluidNeighbour(pi.Position, [&](const FluidParticle& pj) {

			glm::vec3 x_ij = pi.Position - pj.Position;
			float rlen = glm::length(x_ij);

			glm::vec3 v_ij = pi.Velocity - pj.Velocity;

			float scalar = glm::dot(v_ij, x_ij) / (rlen * rlen + 0.01f * h2);

			laplacian += (particleMass / pj.Density) * scalar * cubicSplineKernelGradient(x_ij, rlen, h, coeff);

			});

		laplacian *= dimFactor;

		pi.ViscousForceDensity = fluid.Viscosity * laplacian;
	}
}
void PhysicsSolver::UpdateParticleGravityForceDensities()
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	for (FluidParticle& pi : particles) {
		pi.GravityForceDensity = glm::vec3(0, Gravity, 0) * pi.Density;
	}
}

void PhysicsSolver::UpdateParticleSurfaceTensionForceDensities()
{

}

void PhysicsSolver::UpdatePredictedVelocities(double timeStep)
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;

	float dt = (float)timeStep;

	for (FluidParticle& particle : particles)
	{
		particle.RecalculateNonPressureResultantForceDensity();

		particle.PredictedVelocity = particle.Velocity + dt * particle.GetNonPressureAcceleration();
	}
}

void PhysicsSolver::UpdatePredictedDensities(double timeStep)
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;

	float dt = (float)timeStep;

	UpdatePredictedVelocityDivergence();

	for (FluidParticle& particle : particles)
	{
		particle.PredictedDensity = particle.Density - dt * particle.Density * particle.PredictedVelocityDivergence;
	}
}

void PhysicsSolver::UpdatePredictedVelocityDivergence()
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float h = fluid.SmoothingRadius;
	float h3 = h * h * h;
	float coeff = 240 / (7 * PI * h3);

	for (FluidParticle& pi : particles) {

		float summation = 0;

		ForEachFluidNeighbour(pi.Position, [&](const FluidParticle& pj) {

			glm::vec3 r = pi.Position - pj.Position;
			float rlen = glm::length(r);

			summation += glm::dot((pi.PredictedVelocity - pj.PredictedVelocity), cubicSplineKernelGradient(r, rlen, h, coeff));

			});

		pi.PredictedVelocityDivergence = -fluid.ParticleMass / pi.Density * summation;

	}
}

void PhysicsSolver::UpdateParticlePressures(double timestep)
{
	const FluidParameters& fluid = m_PhysicsState->FluidParameters;
	
	if (fluid.PressureSolver == PressureSolverMethod::SESPH) 
	{
		updateParticlePressuresSESPH();
	}
	else if (fluid.PressureSolver == PressureSolverMethod::IISPH) 
	{
		updateParticlePressuresIISPH(timestep);
	}
}

void PhysicsSolver::UpdateParticlePressureForceDensities()
{
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float h = fluid.SmoothingRadius;
	float h3 = h * h * h;
	float coeff = 240 / (7 * PI * h3);

	float particleMass = m_PhysicsState->FluidParameters.ParticleMass;

	for (FluidParticle& pi : particles) {

		glm::vec3 fluidTerm{};

		ForEachFluidNeighbour(pi.Position, [&](const FluidParticle& pj) {

			glm::vec3 r = pi.Position - pj.Position;
			float rlen = glm::length(r);

			float symmetrisedPressure = pi.Pressure / (pi.Density * pi.Density) + pj.Pressure / (pj.Density * pj.Density);

			fluidTerm += symmetrisedPressure * cubicSplineKernelGradient(r, rlen, h, coeff);

			});

		fluidTerm *= particleMass;

		glm::vec3 boundaryTerm{};

		if (!fluid.IgnoreBoundaryParticles)
		{

			ForEachBoundaryNeighbour(pi.Position, [&](const BoundaryParticle& pj) {

				glm::vec3 r = pi.Position - pj.Position;
				float rlen = glm::length(r);

				boundaryTerm += cubicSplineKernelGradient(r, rlen, h, coeff);

				});
		}

		boundaryTerm *= pi.Pressure * 2 * particleMass / (pi.Density * pi.Density);

		pi.PressureForceDensity = pi.Density * (-fluidTerm - fluid.Gamma2 * boundaryTerm);

	}
}

void PhysicsSolver::IntegrateParticles(double timeStep)
{
	float dt = (float)timeStep;
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;

	for (FluidParticle& particle : particles)
	{
		particle.Velocity = particle.PredictedVelocity + dt * particle.GetPressureAcceleration();
		particle.Position = particle.Position + dt * particle.Velocity;

		DetectBoundaryCollision(particle);
	}
}

void PhysicsSolver::DetectBoundaryCollision(FluidParticle& particle)
{
	const BoundingBox& bounds = m_PhysicsState->BoundaryParameters.ParticleDomain;

	if (particle.Position.y <= bounds.ymin())
	{
		particle.Position.y = bounds.ymin();
		particle.Velocity.y = -particle.Velocity.y * CollisionDampingFactor;
	}
	else if (particle.Position.y >= bounds.ymax())
	{
		particle.Position.y = bounds.ymax();
		particle.Velocity.y = -particle.Velocity.y * CollisionDampingFactor;
	}

	if (particle.Position.x <= bounds.xmin())
	{
		particle.Position.x = bounds.xmin();
		particle.Velocity.x = -particle.Velocity.x * CollisionDampingFactor;
	}
	else if (particle.Position.x >= bounds.xmax())
	{
		particle.Position.x = bounds.xmax();
		particle.Velocity.x = -particle.Velocity.x * CollisionDampingFactor;
	}
}

//float PhysicsSolver::poly6Kernel(float r2, float h2, float coeff)
//{
//	if (r2 >= h2) return 0.0f;
//
//	float diff = h2 - r2;
//
//	return coeff * diff * diff * diff;
//}
//
//
//glm::vec3 PhysicsSolver::poly6KernelGradient(glm::vec3 r, float r2, float h2, float coeff)
//{
//	if (r2 >= h2) return glm::vec3(0);
//
//	float diff = h2 - r2;
//
//	return coeff * diff * diff * r;
//}
//
//glm::vec3 PhysicsSolver::spikyKernelGradient(glm::vec3 r, float r2, float h, float h2, float coeff)
//{
//	if (r2 == 0.0f || r2 >= h2) return glm::vec3(0);
//
//	float dst = std::sqrt(r2);
//
//	float diff = h - dst;
//
//	return coeff * diff * diff * (r / dst);
//}
//
float PhysicsSolver::viscosityLaplacianKernel2D(float r2, float h, float h2, float coeff)
{
	if (r2 == 0.0f || r2 >= h2) return 0.0;

	float dst = std::sqrt(r2);
	float diff = h - dst;

	return coeff * diff;
}


float PhysicsSolver::cubicSplineKernel(float r, float h, float coeff)
{
	float q = r / h;
	if (q >= 1.0f) return 0.0f;

	if (q <= 0.5f)
		return coeff * (6.0f * (q * q * q - q * q) + 1.0f);

	float t = 1.0f - q;
	return coeff * 2.0f * t * t * t;
}

// r = x_i - x_j, rLen = length(r)
glm::vec3 PhysicsSolver::cubicSplineKernelGradient(glm::vec3 r, float rLen, float h, float coeff)
{
	float q = rLen / h;
	if (q >= 1.0f || rLen < 1e-6f) return glm::vec3(0);

	glm::vec3 dir = r / rLen;

	if (q <= 0.5f)
		return coeff * (3.0f * q * q - 2.0f * q) * dir;

	float t = 1.0f - q;
	return -coeff * t * t * dir;
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

void PhysicsSolver::updateBoundaryParticleNeighborSearchGrid()
{
	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	std::vector<BoundaryParticle>& particles = m_PhysicsState->BoundaryParticles;

	m_BoundarySpatialLookup.resize(particles.size());
	m_BoundaryStartIndices.assign(particles.size(), UINT32_MAX);

	for (int i = 0; i < m_BoundarySpatialLookup.size(); i++) {
		BoundaryParticle& particle = particles[i];

		int cellX = std::floor(particle.Position.x / h);
		int cellY = std::floor(particle.Position.y / h);

		uint32_t key = hashCell(cellX, cellY, (uint32_t)particles.size());

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

void PhysicsSolver::updateFluidParticleNeighborGridSearch()
{
	float h = m_PhysicsState->FluidParameters.SmoothingRadius;
	std::vector<FluidParticle>& particles = m_PhysicsState->FluidParticles;

	m_FluidSpatialLookup.resize(particles.size());
	m_FluidStartIndices.assign(particles.size(), UINT32_MAX);

	for (int i = 0; i < m_FluidSpatialLookup.size(); i++) {
		FluidParticle& particle = particles[i];

		int cellX = std::floor(particle.Position.x / h);
		int cellY = std::floor(particle.Position.y / h);

		uint32_t key = hashCell(cellX, cellY, (uint32_t)particles.size());

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



void PhysicsSolver::updateGamma1(const GammaTemplate& templ)
{
	const FluidParticle& testParticle = templ.FluidParticles[0];
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float d = fluid.ParticleSpacing;
	float h = fluid.SmoothingRadius;
	float h2 = h * h;
	float coeff = 40 / (7 * PI * h2);
	float particleMass = fluid.ParticleMass;

	float c1 = 1 / (d * d);

	float c2 = 0;

	for (const FluidParticle& fluidParticle : templ.FluidParticles) {

		glm::vec3 d = testParticle.Position - fluidParticle.Position;
		float r = glm::length(d);

		c2 += cubicSplineKernel(r, h, coeff);

	}

	float c3 = 0;

	for (const BoundaryParticle& boundaryParticle : templ.BoundaryParticles) {

		glm::vec3 d = testParticle.Position - boundaryParticle.Position;
		float r = glm::length(d);

		c3 += cubicSplineKernel(r, h, coeff);
	}

	fluid.Gamma1 = (c1 - c2) / c3;
}

void PhysicsSolver::updateGamma2(const GammaTemplate& templ)
{
	const FluidParticle& testParticle = templ.FluidParticles[0];
	FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float h = fluid.SmoothingRadius;
	float h3 = h * h * h;
	float coeff = 240 / (7 * PI * h3);

	glm::vec3 c1 = {};

	for (const FluidParticle& fluidParticle : templ.FluidParticles) {

		glm::vec3 r = testParticle.Position - fluidParticle.Position;

		float rlen = glm::length(r);

		c1 += cubicSplineKernelGradient(r, rlen, h, coeff);

	}

	glm::vec3 c2{};

	for (const BoundaryParticle& boundaryParticle : templ.BoundaryParticles) {

		glm::vec3 r = testParticle.Position - boundaryParticle.Position;

		float rlen = glm::length(r);

		c2 += cubicSplineKernelGradient(r, rlen, h, coeff);
	}

	float numerator = glm::dot(c1, c2);
	float denominator = glm::dot(c2, c2);

	fluid.Gamma2 = -numerator / denominator;

}

void PhysicsSolver::updateParticlePressuresSESPH()
{
	const FluidParameters& fluid = m_PhysicsState->FluidParameters;

	float stiffness = fluid.Stiffness;
	float restDensity = fluid.RestDensity;

	for (int i = 0; i < m_PhysicsState->FluidParticles.size(); i++) {
		FluidParticle& source = m_PhysicsState->FluidParticles[i];
		source.Pressure = std::max(0.0f, stiffness * (source.Density - restDensity));
	}
}

void PhysicsSolver::updateParticlePressuresIISPH(double timeStep)
{
	const FluidParameters& fluid = m_PhysicsState->FluidParameters;
	std::vector<FluidParticle>& fluidParticles = m_PhysicsState->FluidParticles;

	float h = fluid.SmoothingRadius;
	float h3 = h * h * h;
	float coeff = 240 / (7 * PI * h3);

	float dt = (float)timeStep;

	for (FluidParticle& pi : fluidParticles) {
		pi.Pressure = 0;
		pi.AP_i = 0;
		pi.s_i = fluid.RestDensity - pi.PredictedDensity;
	}

	for (int i = 0; i <= 10; i++) {


		for (FluidParticle& pi : fluidParticles) {

			glm::vec3 summation1{};
			glm::vec3 summation2{};

			ForEachFluidNeighbour(pi.Position, [&](const FluidParticle& pj) {

				glm::vec3 r = pi.Position - pj.Position;
				float rlen = glm::length(r);

				float symmetrisedPressure = pi.Pressure / (pi.Density * pi.Density) + pj.Pressure / (pj.Density * pj.Density);

				summation1 += symmetrisedPressure * cubicSplineKernelGradient(r, rlen, h, coeff);
				summation2 += (1 / (pi.Density * pi.Density)) * cubicSplineKernelGradient(r, rlen, h, coeff);

				});

			pi.a_ip = -fluid.ParticleMass * summation1;
			pi.d_ii = -dt * dt * fluid.ParticleMass * summation2;
		}

		for (FluidParticle& pi : fluidParticles) {

			float summation1 = 0;
			float summation2 = 0;

			ForEachFluidNeighbour(pi.Position, [&](const FluidParticle& pj) {

				glm::vec3 r = pi.Position - pj.Position;
				float rlen = glm::length(r);

				summation1 += glm::dot((pi.a_ip - pj.a_ip), cubicSplineKernelGradient(r, rlen, h, coeff));

				glm::vec3 d_ji = dt * dt * fluid.ParticleMass / (pi.Density * pi.Density) * cubicSplineKernelGradient(r, rlen, h, coeff);

				summation2 += glm::dot((pi.d_ii - d_ji), cubicSplineKernelGradient(r, rlen, h, coeff));

				});

			pi.AP_i = timeStep * timeStep * fluid.ParticleMass * summation1;
			pi.a_ii = fluid.ParticleMass * summation2;
		}

		for (FluidParticle& pi : fluidParticles)
		{
			if (glm::length(pi.a_ii) < 1e-20f)
				pi.Pressure = 0;
			else
				pi.Pressure = std::max(0.0f, pi.Pressure + (fluid.IISPHRelaxationCoefficient / pi.a_ii) * (pi.s_i - pi.AP_i));
		}
	}
}
