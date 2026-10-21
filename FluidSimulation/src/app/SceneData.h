#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Physics/PhysicsSolver.h"
#include "Physics/Particle.h"
#include "Physics/BoundaryData.h"

struct FluidData{
	//Fluid Parameters
	bool SimulationRunning;
	float SmoothingRadius{ 20 };


	//Objects
	std::vector<Particle> Particles;


	void SetParticles(std::vector<Particle>& particles);
};

struct SceneData
{
	FluidData FluidData;
	BoundaryData BoundaryData;


	unsigned int FramebufferTextureId{};
};

