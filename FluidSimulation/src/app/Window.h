#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct WindowSpec {
	int Width;
	int Height;
};

class Window
{
public:
	Window(WindowSpec winSpec);
	void Create();
	void Destroy();
	bool ShouldClose();
	void Update();
	GLFWwindow* Handle();

private:
	WindowSpec m_WinSpec;
	GLFWwindow* m_Handle;


};

