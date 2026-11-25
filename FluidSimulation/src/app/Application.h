#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <memory>

#include "Window.h"
#include "Layers/RenderLayer.h"
#include "Layers/EditorLayer.h"
#include "Layers/PhysicsLayer.h"
#include "Data/SceneData.h"

struct ApplicationSpec {
	std::string Title;
	WindowSpec WinSpec;

};


class Application
{
public:
	Application(ApplicationSpec appSpec);

	void Run();
	static Application* Get();
	double GetTime();

	inline std::shared_ptr<Window> GetWindow() { return m_Window; };

private:
	static Application* s_Application;

	ApplicationSpec m_AppSpec;
	std::shared_ptr<Window> m_Window;

	std::shared_ptr<SceneData> m_SceneData;
	std::shared_ptr<PhysicsLayer> m_PhysicsLayer;
	std::shared_ptr<RenderLayer> m_RenderLayer;
	std::shared_ptr<EditorLayer> m_EditorLayer;

	void initLogger();
	void initWindow();
	void initImGui();



};

