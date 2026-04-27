#pragma once
#include "FSMState.h"
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "BehaviorTree/BlackboardComponent.h"

namespace GameAI::FSM
{
	class Search : public State
	{
	public:
		
		virtual void Tick(TObjectPtr<UBlackboardComponent>& blackboardRef) override
		{
			
		}
	};
}
