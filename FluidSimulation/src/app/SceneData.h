#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "Physics/PhysicsState.h"

struct SceneData
{
	unsigned int FramebufferTextureId{};

	std::shared_ptr<PhysicsState> PhysicsState;

	SceneData();
};


