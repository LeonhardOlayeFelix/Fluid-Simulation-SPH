#include "PhysicsSystem.h"
#include "PhysicsSolver.h"

PhysicsSystem::PhysicsSystem(std::shared_ptr<PhysicsState> initialState, double fixedTimeStep)
	: m_PhysicsState(initialState), m_FixedTimeStep(fixedTimeStep), m_TimeAccumulator(0.0)
{
	if (!m_PhysicsState)
		m_PhysicsState = std::make_shared<PhysicsState>();

	m_Solver = std::make_shared<PhysicsSolver>(m_PhysicsState);
}

PhysicsSystem::~PhysicsSystem()
{
}

void PhysicsSystem::OnUpdate(double deltaTime)
{
	m_TimeAccumulator += deltaTime;

	while (m_TimeAccumulator >= m_FixedTimeStep)
	{
		m_Solver->Update(m_FixedTimeStep);
		m_TimeAccumulator -= m_FixedTimeStep;
	}
}

std::shared_ptr<PhysicsState> PhysicsSystem::GetPhysicsState() const
{
	return m_PhysicsState;
}

void PhysicsSystem::SetFixedTimeStep(double timeStep)
{
	m_FixedTimeStep = timeStep;
}
