#include "FSM.h"

void GameAI::FSM::FSM::Tick(float DeltaTime)
{
	if (!m_CanTick) return;
	
	// update state if needed
	for (auto transition : m_Transitions)
	{
		if (transition->GetFromStateType() == typeid(m_ActiveState))
		{
			m_ActiveState = m_States[transition->Tick(DeltaTime)].get();
			break;
		}
	}
	
	// state behavior
	m_ActiveState->Tick(DeltaTime);
}

void GameAI::FSM::FSM::AddState(std::unique_ptr<State>&& state)
{
	if (std::find(m_States.begin(), m_States.end(), [](auto s){ return typeid(s) == typeid(state); } ) == m_States.end())
	{
		if (m_ActiveState == nullptr)
		{
			m_ActiveState = state.get();
		}
		m_States[typeid(state)] = std::move(state);
	}
}

void GameAI::FSM::FSM::AddTransition(std::unique_ptr<Transition>&& transition)
{
	if (std::find(m_Transitions.begin(), m_Transitions.end(), [](auto t){ return typeid(t) == typeid(transition); } ) == m_Transitions.end())
	{
		m_Transitions.push_back(transition);
	}
}
