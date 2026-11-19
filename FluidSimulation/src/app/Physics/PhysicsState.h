#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "FluidParticle.h"
#include "Parameters.h"
#include "BoundaryParticle.h"

struct PhysicsState
{
	std::vector<FluidParticle> FluidParticles;
	std::vector<BoundaryParticle> BoundaryParticles;

	FluidParameters FluidParameters;
	BoundaryParameters BoundaryParameters;

	PhysicsState() = default;

    void RandomiseParticleData(int count);
    void GridParticleData(int count, float spacing);
	void RecalculateBoundaryParticles();

private:
	
    int randomInt(int min, int max);
	int randomIntNormal(int min, int max);
};
