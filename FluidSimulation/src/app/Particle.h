#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Particle {
	glm::vec3 Position;
	glm::vec3 Velocity;
	float Radius{ 20 };

	float Mass;

	Particle(glm::vec2 position, glm::vec2 velocity, float mass)
	{
		Position = glm::vec3(position, 0);
		Velocity = glm::vec3(velocity, 0);
		Mass = mass;
	}
};