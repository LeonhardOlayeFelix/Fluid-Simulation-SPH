#include "PhysicsLayer.h"
#include "../Physics/PhysicsSolver.h"

PhysicsLayer::PhysicsLayer(std::shared_ptr<PhysicsData> initialState, double fixedTimeStep)
	: m_PhysicsState(initialState), m_FixedTimeStep(fixedTimeStep), m_TimeAccumulator(0.0)
{
	if (!m_PhysicsState)
		m_PhysicsState = std::make_shared<PhysicsData>();

	m_Solver = std::make_shared<PhysicsSolver>(m_PhysicsState);
}

PhysicsLayer::~PhysicsLayer()
{
}

void PhysicsLayer::OnUpdate(double deltaTime)
{
	m_TimeAccumulator += deltaTime;

	while (m_TimeAccumulator >= m_FixedTimeStep)
	{
		m_Solver->Update(m_FixedTimeStep);
		m_TimeAccumulator -= m_FixedTimeStep;
	}
}

std::shared_ptr<PhysicsData> PhysicsLayer::GetPhysicsState() const
{
	return m_PhysicsState;
}

void PhysicsLayer::SetFixedTimeStep(double timeStep)
{
	m_FixedTimeStep = timeStep;
}
