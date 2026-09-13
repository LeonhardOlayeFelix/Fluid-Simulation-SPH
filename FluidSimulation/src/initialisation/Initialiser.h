#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Initialiser
{
private:
	static GLFWwindow* m_Window;
	static bool m_Initialised;

public:
	Initialiser() = delete;

	static GLFWwindow* initApplication();
	static GLFWwindow* GetWindow();
	static inline bool Initialised() { return m_Initialised; };


private:
	static GLFWwindow* initGLFW();
	static void initGLAD();
	static void initImGui(GLFWwindow* window);
	static void initLogger();
};

