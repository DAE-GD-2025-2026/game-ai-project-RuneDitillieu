
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput BlendedSteering = {};
	for (const WeightedBehavior Wb : WeightedBehaviors)
	{
		if (Wb.Weight >= 0.01f)
		{
			SteeringOutput Steering{Wb.pBehavior->CalculateSteering(DeltaT, Agent)};
			BlendedSteering.LinearVelocity += Steering.LinearVelocity * Wb.Weight;
			BlendedSteering.AngularVelocity += Steering.AngularVelocity * Wb.Weight;
		}
	}
	BlendedSteering.LinearVelocity.Normalize();
	BlendedSteering.AngularVelocity /= abs(BlendedSteering.AngularVelocity);
	
	if (Agent.GetDebugRenderingEnabled())
	{
		// Velocity
		const FVector2D EndPoint2{Agent.GetPosition() + Agent.GetLinearVelocity() * 0.5f};
		DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), 
			FVector(EndPoint2.X, EndPoint2.Y, 1), FColor(255, 0, 255, 255)); 
	
		// Desired Direction
		const FVector2D EndPoint1{ Agent.GetPosition() + (BlendedSteering.LinearVelocity * 300)};
		DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), 
			FVector(EndPoint1.X, EndPoint1.Y, 1), FColor(0, 255, 0, 255)); 
	
		// Steering
		const FVector2D EndPoint3{ Agent.GetPosition() + (EndPoint1 - EndPoint2)};
		DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), 
			FVector(EndPoint3.X, EndPoint3.Y, 1), FColor(0, 255, 255, 255));
	}
	
	return BlendedSteering;
}

float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
	auto it = find_if(WeightedBehaviors.begin(),
		WeightedBehaviors.end(),
		[SteeringBehavior](const WeightedBehavior& Elem)
		{
			return Elem.pBehavior == SteeringBehavior;
		}
	);

	if(it!= WeightedBehaviors.end())
		return &it->Weight;
	
	return nullptr;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If none of the behavior return a valid output, last behavior is returned
	return Steering;
}