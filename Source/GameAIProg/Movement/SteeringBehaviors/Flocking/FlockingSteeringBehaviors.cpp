#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float DeltaT, ASteeringAgent& pAgent)
{
	Target.Position = pFlock->GetAverageNeighborPos();
	
	return Seek::CalculateSteering(DeltaT, pAgent);
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float DeltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};
	
	size_t Index{ 0 };
	for (const ASteeringAgent* Neighbor : pFlock->GetNeighbors())
	{
		FVector2D NeighborPos{ Neighbor->GetPosition() };
		FVector2D AgentPos{ pAgent.GetPosition() };
		const float DistToNeighbor{ static_cast<float>((NeighborPos - AgentPos).Length()) };
		const float Impact{ 1.f / DistToNeighbor };
		
		Target.Position = Neighbor->GetPosition();
		
		Steering.LinearVelocity += Flee::CalculateSteering(DeltaT, pAgent).LinearVelocity * Impact;
		
		++Index;
		if (Index >= pFlock->GetNrOfNeighbors())
			break;
	}
	
	if (Index > 0)
		Steering.LinearVelocity.Normalize();

	return Steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float DeltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};
	Steering.LinearVelocity = pFlock->GetAverageNeighborVelocity();
	Steering.LinearVelocity.Normalize();
	
	return Steering;
}