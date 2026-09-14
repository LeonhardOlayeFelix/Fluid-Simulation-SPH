#pragma once
class Layer
{
public:
	virtual ~Layer() {};

	virtual void OnUpdate(double ts) {};
	virtual void OnRender() {};
};

