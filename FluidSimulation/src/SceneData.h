#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct SceneData
{
	float Scale{ 0.1 };
	unsigned int FramebufferTextureId{};
	glm::vec2 ViewportSize{};
};

