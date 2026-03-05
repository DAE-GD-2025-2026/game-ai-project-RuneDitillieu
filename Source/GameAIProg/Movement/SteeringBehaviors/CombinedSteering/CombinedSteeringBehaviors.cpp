
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"
#include "Shared/DebugHelpers.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	bool DoDebugRendering{ Agent.GetDebugRenderingEnabled() };
	Agent.SetDebugRenderingEnabled(false);	// so that only the vectors of the combined steering get drawn
	
	SteeringOutput BlendedSteering{};
	for (const WeightedBehavior Wb : WeightedBehaviors)
	{
		if (Wb.Weight >= 0.01f)
		{
			SteeringOutput Steering{ Wb.pBehavior->CalculateSteering(DeltaT, Agent) };
			BlendedSteering.LinearVelocity += Steering.LinearVelocity * Wb.Weight;
		}
	}
	
	BlendedSteering.LinearVelocity.Normalize();
	
	Agent.SetDebugRenderingEnabled(DoDebugRendering);
	
	if (Agent.GetDebugRenderingEnabled())
	{
		Debug::DrawSteeringVectors(Agent, BlendedSteering);
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
	SteeringOutput Steering{};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If none of the behavior return a valid output, last behavior is returned
	return Steering;
}