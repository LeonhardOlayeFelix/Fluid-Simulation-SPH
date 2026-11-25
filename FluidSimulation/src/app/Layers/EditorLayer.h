#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

#include "../../core/Layer.h"
#include "../Data/SceneData.h"
#include "RenderLayer.h"

class EditorLayer : public Layer
{
private:
	std::shared_ptr<SceneData> m_SceneData;
	std::shared_ptr<RenderLayer> m_RenderLayer;

public:
	EditorLayer(std::shared_ptr<SceneData> sceneData, std::shared_ptr<RenderLayer> renderLayer);

	virtual ~EditorLayer() override;

	virtual void OnUpdate(double ts) override;
	virtual void  OnRender() override;
	void  DrawViewport();
	void  DrawControlsPanels();
	void  DrawSimulationControlsControlPanel();
	void  DrawVisualisationControlPanel();
	void  DrawInspector();
	void  DrawStats();

private:
	void BeginFrame();
	void EndFrame();
	void Destroy();
};

