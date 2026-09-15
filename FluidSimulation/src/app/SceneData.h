#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "PhysicsSolver.h"
#include "Particle.h"
#include "BoundaryData.h"

struct FluidData{
	std::vector<Particle> Particles = { Particle({250.0, 250.0}, {0, 400}, 5) };
};

struct SceneData
{
	FluidData FluidData;
	BoundaryData BoundaryData;

	unsigned int FramebufferTextureId{};
};

