#pragma once
#include <memory>
#include <typeinfo>
#include <vector>

#include "States/FSMState.h"
#include "FSMTransition.h"
#include "BehaviorTree/BlackboardComponent.h"

namespace GameAI::FSM
{
	class FSM
	{
	public:
		FSM(UBlackboardComponent* pBlackboard);
		void Tick(float DeltaTime);
		void AddState(std::unique_ptr<State>&& state, bool isStartState = false, bool isStopState = false);
		void AddTransition(std::unique_ptr<Transition>&& transition);
		void Start();
		void Stop();
		void SetBlackboard(UBlackboardComponent* pBlackboard) { if (m_pBlackboard == nullptr) m_pBlackboard = pBlackboard; }
		
	private:
		std::vector<std::unique_ptr<State>> m_States{};
		std::vector<std::unique_ptr<Transition>> m_Transitions{};
		
		State* m_ActiveState{ nullptr };
		State* m_StartState{ nullptr };
		State* m_StopState{ nullptr };
		
		bool m_CanTick{ false };
		
		UBlackboardComponent* m_pBlackboard;
	};
}
