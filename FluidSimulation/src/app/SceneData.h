#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Physics/PhysicsSolver.h"
#include "Physics/Particle.h"
#include "Physics/Parameters.h"


struct SceneData
{
	FluidParameters FluidParameters;

	BoundaryParameters BoundaryParameters;

	unsigned int FramebufferTextureId{};

	std::vector<Particle> Particles;

	void SetParticles(std::vector<Particle>& particles);

};

