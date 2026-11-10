#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Particle {
public:
	glm::vec3 Position{};
	glm::vec3 PredictedPosition{};
	glm::vec3 Velocity{};
	glm::vec3 Acceleration{};
	glm::vec3 PressureForceDensity{};
	glm::vec3 ViscousForceDensity{};
	glm::vec3 ResultantForceDensity{};
	float Density{ 0 };
	float Pressure{ 0 };

	float Mass{ 1000 };

	Particle() = default;

	Particle(glm::vec2 position, glm::vec2 velocity)
	{
		Position = glm::vec3(position, 0);
		Velocity = glm::vec3(velocity, 0);
	}

	glm::vec3 RecalculateResultantForceDensity()
	{
		ResultantForceDensity = PressureForceDensity + ViscousForceDensity;
		return ResultantForceDensity;

	}
};