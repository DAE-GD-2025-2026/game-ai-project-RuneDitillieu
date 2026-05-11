#include "FSM.h"
#include "States/Chase.h"
#include "States/Patrol.h"
#include "States/Search.h"

GameAI::FSM::FSM::FSM(UBlackboardComponent* pBlackboard)
	: m_pBlackboard(pBlackboard)
{}

void GameAI::FSM::FSM::Tick(float DeltaTime)
{
	if (!m_CanTick) return;
	
	// update state if needed
	for (auto& transition : m_Transitions)
	{
		if (transition->GetFromState() == m_ActiveState)
		{
			if (transition->Evaluate())
			{
				m_ActiveState = transition->GetToState();
				m_ActiveState->OnEnter(m_pBlackboard);
				break;
			}
		}
	}
	
	// state behavior
	m_ActiveState->Tick(m_pBlackboard);
}

void GameAI::FSM::FSM::AddState(std::unique_ptr<State>&& state, bool isStartState, bool isStopState)
{
	if (std::find_if(m_States.begin(), m_States.end(), [&state](std::unique_ptr<State>& s) { return s.get() == state.get(); }) == m_States.end())
	{
		if (isStartState)
		{
			m_StartState = state.get();
		}
		if (isStopState)
		{
			m_StopState = state.get();
		}
		
		m_States.emplace_back(std::move(state));
	}
}

void GameAI::FSM::FSM::AddTransition(std::unique_ptr<Transition>&& transition)
{
	// if transitions hasn't been added yet
	if (std::find_if(m_Transitions.begin(), m_Transitions.end(), [&transition](std::unique_ptr<Transition>& t){ return t.get() == transition.get(); } ) == m_Transitions.end())
	{
		// if from state and to state have been added
		//if (std::find_if(m_States.begin(), m_States.end(), [&transition](std::unique_ptr<State>& s) { return s.get() == transition->GetFromState(); }) != m_States.end()
		//	&& std::find_if(m_States.begin(), m_States.end(), [&transition](std::unique_ptr<State>& s) { return s.get() == transition->GetToState(); }) != m_States.end())
		{
			m_Transitions.emplace_back(std::move(transition));
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
