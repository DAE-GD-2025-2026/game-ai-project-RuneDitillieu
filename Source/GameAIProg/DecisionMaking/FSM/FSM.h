#pragma once
#include <memory>
#include <typeinfo>
#include <vector>
#include <unordered_map>

#include "States/FSMState.h"
#include "Transitions/FSMTransition.h"

namespace GameAI::FSM
{
	class FSM
	{
	public:
		void Tick(float DeltaTime);
		void AddState(std::unique_ptr<State>&& state);
		void AddTransition(std::unique_ptr<Transition>&& transition);
		void CanTick(bool canTick) { m_CanTick = canTick; }
		
	private:
		std::unordered_map<type_info, std::unique_ptr<State>> m_States{};
		std::vector<std::unique_ptr<Transition>> m_Transitions{};
		
		State* m_ActiveState{ nullptr };
		
		bool m_CanTick{ false };
	};
}
