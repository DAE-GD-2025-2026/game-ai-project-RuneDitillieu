#pragma once
#include "BehaviorTree/BlackboardComponent.h"

namespace GameAI::FSM
{
	class State
	{
	public:
		virtual ~State() = default;
		virtual void OnEnter(UBlackboardComponent* pBlackboard){}
		virtual void Tick(UBlackboardComponent* pBlackboard) {}
	};
}