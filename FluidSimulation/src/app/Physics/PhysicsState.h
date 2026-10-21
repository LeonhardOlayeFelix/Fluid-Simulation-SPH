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
};
