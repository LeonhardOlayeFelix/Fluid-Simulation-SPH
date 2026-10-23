#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Particle.h"
#include "Parameters.h"

struct PhysicsState
{
	std::vector<Particle> Particles;

	FluidParameters FluidParameters;

	BoundaryParameters BoundaryParameters;

	PhysicsState() = default;

    void RandomiseParticleData(int count);
    void GridParticleData(int numRows, int numCols);
    

private:
	
    int randomInt(int min, int max);
	int randomIntNormal(int min, int max);
};
