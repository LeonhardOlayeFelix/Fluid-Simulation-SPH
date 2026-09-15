#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

#include "../core/Layer.h"
#include "SceneData.h"
#include "RenderLayer.h"
#include "BoundaryData.h"

class ImGuiLayer : public Layer
{
private:
	std::shared_ptr<SceneData> m_SceneData;
	std::shared_ptr<RenderLayer> m_RenderLayer;

public:
	ImGuiLayer(std::shared_ptr<SceneData> sceneData, std::shared_ptr<RenderLayer> renderLayer);

	virtual ~ImGuiLayer() override;

	virtual void OnUpdate(double ts) override;
	virtual void  OnRender() override;

private:
	void BeginFrame();
	void EndFrame();
	void Destroy();
};

