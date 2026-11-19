#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "Physics/PhysicsState.h"

struct SceneData
{

	std::shared_ptr<PhysicsState> PhysicsState;

	bool ShowForceArrow = false;
	bool ShowKernel = false;
	bool ShowBoundaryParticles = false;
	bool ShowParticleDomain = false;
	bool MatchBallSizeToSpacing = false;
	bool FlipForceArrow = false;

	int ArrowLengthMax = 40;
	int ArrowWidth = 2;

	unsigned int FramebufferTextureId{};

	float VMax = 100;

	SceneData();
};


