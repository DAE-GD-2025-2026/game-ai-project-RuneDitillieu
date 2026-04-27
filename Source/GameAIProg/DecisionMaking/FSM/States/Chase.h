#pragma once
#include "FSMState.h"
#include "BehaviorTree/BlackboardComponent.h"

namespace GameAI::FSM
{
	class Chase : public State
	{
	public:
		virtual void Tick(TObjectPtr<UBlackboardComponent>& blackboardRef) override
		{
			
		}
	};
}