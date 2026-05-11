#pragma once
#include "FSMState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Movement/SteeringBehaviors/PathFollow/PathFollowSteeringBehavior.h"

namespace GameAI::FSM
{
	class Patrol : public State
	{
	public:
		virtual void OnEnter(UBlackboardComponent* pBlackboard)
		{
			m_pPathFollowBehavior->SetPath(path);
			ASteeringAgent* steeringAgent = Cast<ASteeringAgent>(pBlackboard->GetValueAsObject("SelfActor"));
			steeringAgent->SetPosition(path[0]);
			steeringAgent->SetSteeringBehavior(m_pPathFollowBehavior.get());
		}
		
		virtual void Tick(UBlackboardComponent* pBlackboard) override
		{
			if (m_pPathFollowBehavior->HasReachedEnd())
			{
				ASteeringAgent* steeringAgent = Cast<ASteeringAgent>(pBlackboard->GetValueAsObject("SelfActor"));
				steeringAgent->SetSteeringBehavior(m_pPathFollowBehavior.get());
				m_pPathFollowBehavior->SetPath(path);
			}
			
			ASteeringAgent* steeringAgent = Cast<ASteeringAgent>(pBlackboard->GetValueAsObject("SelfActor"));
			auto s = steeringAgent->GetSteeringBehavior();
		}
		
	private:
		std::unique_ptr<PathFollow> m_pPathFollowBehavior{ std::make_unique<PathFollow>() };
		std::vector<FVector2D> path{FVector2D(0, 0), FVector2D(200, -200), FVector2D(200, 0), FVector2D(300, 0)};
	};
}
