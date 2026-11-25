#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "FluidParticle.h"
#include "ParameterData.h"
#include "BoundaryParticle.h"

struct PhysicsData
{
	std::vector<FluidParticle> FluidParticles;
	std::vector<BoundaryParticle> BoundaryParticles;

	FluidParameters FluidParameters;
	BoundaryParameters BoundaryParameters;

    void GridParticleData(int count, float spacing);
	void RecalculateBoundaryParticles();

};
