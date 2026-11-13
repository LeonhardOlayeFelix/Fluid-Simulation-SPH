#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



enum class CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.2f;
const float ZOOM = 80.0f;
const float NEARPLANE = 0.1f;
const float FARPLANE = 1000.0f;
const glm::vec3 E1 = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 E2 = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 E3 = glm::vec3(0.0f, 0.0f, 1.0f);


class Camera {

	public:
		glm::vec3 Position{};
		glm::vec3 Front{};
		glm::vec3 Up{};
		glm::vec3 Right{};
		glm::vec3 WorldUp{};
		float Yaw{};
		float Pitch{};
		float MovementSpeed{};
		float MouseSensitivity{};
		float Fov{};
		float Near{};
		float Far{};

	public:
		Camera(glm::vec3 position = E3, glm::vec3 up = E2, float yaw = YAW, float pitch = PITCH);
		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
		void ProcessKeyboard(CameraMovement direction, double deltaTime);
		void ProcessMouseMovement(float xoffset, float yoffset);
		void ProcessMouseScroll(float yoffset);
		void UpdateCameraVectors();
		glm::mat4 GetViewMatrix();
		glm::mat4 GetPerspectiveMatrix();

};

