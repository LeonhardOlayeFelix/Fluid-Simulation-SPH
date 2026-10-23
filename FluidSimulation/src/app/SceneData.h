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

	int numRows = 10;
	int numCols = 10;
	int count = 150;
	int ArrowLengthMax = 40;
	int ArrowWidth = 2;
	bool showDensityGradient = true;
	bool flipDensityGradient = false;
	bool showKernel = true;



	SceneData();
};


