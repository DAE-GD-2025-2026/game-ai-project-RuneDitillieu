#include "FSM.h"
#include "States/Chase.h"
#include "States/Patrol.h"
#include "States/Search.h"

void GameAI::FSM::FSM::Tick(float DeltaTime)
{
	if (!m_CanTick) return;
	
	// update state if needed
	for (auto transition : m_Transitions)
	{
		if (transition->GetFromState() == m_ActiveState)
		{
			if (transition->Evaluate())
			{
				m_ActiveState = transition->GetToState();
				break;
			}
		}
	}
	
	// state behavior
	m_ActiveState->Tick(DeltaTime);
}

void GameAI::FSM::FSM::AddState(std::unique_ptr<State>&& state, bool isStartState, bool isStopState)
{
	if (!m_States[typeid(state)])
	{
		if (isStartState)
		{
			m_StartState = state.get();
		}
		if (isStopState)
		{
			m_StopState = state.get();
		}
		
		m_States[typeid(state)] = std::move(state);
	}
}

void GameAI::FSM::FSM::AddTransition(std::unique_ptr<Transition>&& transition)
{
	// if transitions hasn't been added yet
	if (std::find(m_Transitions.begin(), m_Transitions.end(), [](auto t){ return typeid(t) == typeid(transition); } ) == m_Transitions.end())
	{
		// if from state and to state have been added
		if (m_States[typeid(transition->GetFromState())]
			&& m_States[typeid(transition->GetToState())])
		{
			m_Transitions.push_back(transition);
		}
	}
}

void GameAI::FSM::FSM::Start()
{
	if (m_StartState == nullptr) return;
	
	m_CanTick = true;
	m_ActiveState = m_StartState;
}

void GameAI::FSM::FSM::Stop()
{
	m_CanTick = false;
	m_ActiveState = m_StopState;
}
