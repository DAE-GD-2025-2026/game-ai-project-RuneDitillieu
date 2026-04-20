#pragma once
#include <memory>
#include <typeinfo>
#include <vector>

#include "States/FSMState.h"
#include "FSMTransition.h"

namespace GameAI::FSM
{
	class FSM
	{
	public:
		void Tick(float DeltaTime);
		void AddState(std::unique_ptr<State>&& state, bool isStartState = false, bool isStopState = false);
		void AddTransition(std::unique_ptr<Transition>&& transition);
		void Start();
		void Stop();
		
	private:
		std::vector<std::unique_ptr<State>> m_States{};
		std::vector<std::unique_ptr<Transition>> m_Transitions{};
		
		State* m_ActiveState{ nullptr };
		State* m_StartState{ nullptr };
		State* m_StopState{ nullptr };
		
		bool m_CanTick{ false };
	};
}
