#pragma once
#include "Layer.h"
#include "SceneData.h"
#include "RenderLayer.h"
#include <memory>
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"


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

