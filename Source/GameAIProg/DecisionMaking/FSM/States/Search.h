#pragma once
#include "FSMState.h"
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "BehaviorTree/BlackboardComponent.h"

namespace GameAI::FSM
{
	class Search : public State
	{
	public:
		virtual void OnEnter(UBlackboardComponent* pBlackboard)
		{
			hasReachedLastPos = false;
			FVector targetPos{ pBlackboard->GetValueAsVector(FName("LastSpottedThiefPosition")) };
			pSeekBehavior->SetTarget(FTargetData(FVector2D(targetPos.X, targetPos.Y)));
		}
		
		virtual void Tick(UBlackboardComponent* pBlackboard) override
		{
			ASteeringAgent* steeringAgent = Cast<ASteeringAgent>(pBlackboard->GetValueAsObject("SelfActor"));
			
			if (!hasReachedLastPos)
			{
				steeringAgent->SetSteeringBehavior(pSeekBehavior.get());
			}
			else
			{
				steeringAgent->SetSteeringBehavior(pWanderBehavior.get());
			}
			
			FVector targetPos{ pBlackboard->GetValueAsVector(FName("LastSpottedThiefPosition")) };
			if ((targetPos - pBlackboard->GetValueAsVector(FName("GuardPosition"))).Length() <= 0.05f)
			{
				hasReachedLastPos = true;
			}
		}
		
	private:
		std::unique_ptr<Seek> pSeekBehavior{ std::make_unique<Seek>() };
		std::unique_ptr<Wander> pWanderBehavior{ std::make_unique<Wander>() };
		bool hasReachedLastPos{ false };
	};
}
