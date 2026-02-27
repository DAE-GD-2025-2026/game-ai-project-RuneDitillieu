#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	Target.Position = pFlock->GetAverageNeighborPos();
	
	return Seek::CalculateSteering(deltaT, pAgent);
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};
	
	size_t index{ 0 };
	for (ASteeringAgent* neighbor : pFlock->GetNeighbors())
	{
		FVector2D neighborPos{neighbor->GetPosition() };
		FVector2D agentPos{ pAgent.GetPosition() };
		const float DistToNeighbor{ float((neighborPos - agentPos).Length()) };
		const float Impact{ 1.f / DistToNeighbor };
		
		Target.Position = neighbor->GetPosition();
		
		Steering.LinearVelocity += Flee::CalculateSteering(deltaT, pAgent).LinearVelocity * Impact;
		
		++index;
		if (index >= pFlock->GetNrOfNeighbors())
			break;
	}
	
	if (index > 0)
		Steering.LinearVelocity.Normalize();

	return Steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};
	Steering.LinearVelocity = pFlock->GetAverageNeighborVelocity();
	Steering.LinearVelocity.Normalize();
	
	return Steering;
}