#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "PhysicsData.h"
#include "BoundingBox.h"

enum class FluidParticleShaderTypes {
	PlainShader,
	SpecimenShader,
	VelocityShader,
	SurfaceTensionShader,
};


struct SceneData
{

	std::shared_ptr<PhysicsData> PhysicsData;

	BoundingBox Viewport;

	bool ShowForceArrow = false;
	bool ShowBoundaryParticles = false;
	bool ShowParticleDomain = true;
	bool MatchBallSizeToSpacing = false;
	bool FlipForceArrow = false;
	bool InGridFormation;

	int ArrowLengthMax = 40;
	int ArrowWidth = 2;
	int SpecimenIndex = 0;

	unsigned int FramebufferTextureId{};

	float VMax = 100;

	FluidParticleShaderTypes FluidParticleShader = FluidParticleShaderTypes::VelocityShader;
	glm::vec3 PlainParticleColor{ 0.05, 0.20, 0.55 };
	glm::vec3 SurfaceParticleColor{ 1.00, 0.55, 0.20 };
	glm::vec3 SpecimenParticleColor{ 1.00, 0.55, 0.20 };
	glm::vec3 SpecimenNeighborColor{ 0.92 };
	bool HighlightSpecimenNeighbors = true;
	bool ShowKernelRadius = true;


	SceneData();
};


