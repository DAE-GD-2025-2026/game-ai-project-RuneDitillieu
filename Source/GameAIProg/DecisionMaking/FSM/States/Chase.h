#pragma once
#include "FSMState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include <typeinfo>

namespace GameAI::FSM
{
	class Chase : public State
	{
	public:
		virtual void Tick(UBlackboardComponent* pBlackboard) override
		{
			FVector targetPos{ pBlackboard->GetValueAsVector(FName("ThiefPosition")) };
			pSeekBehavior->SetTarget(FTargetData(FVector2D(targetPos.X, targetPos.Y)));

			ASteeringAgent* steeringAgent = Cast<ASteeringAgent>(pBlackboard->GetValueAsObject("SelfActor"));
			steeringAgent->SetSteeringBehavior(pSeekBehavior.get());
		}
		
	private:
		std::unique_ptr<Seek> pSeekBehavior{ std::make_unique<Seek>() };
	};
}
