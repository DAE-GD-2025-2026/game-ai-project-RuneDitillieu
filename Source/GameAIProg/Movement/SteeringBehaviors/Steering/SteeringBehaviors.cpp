#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

void DrawDebugVectors(const ASteeringAgent& Agent, const SteeringOutput& Steering)
{
	// Velocity
	FVector2D EndPoint2{Agent.GetPosition() + Agent.GetLinearVelocity() * 0.5f};
	DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), 
		FVector(EndPoint2.X, EndPoint2.Y, 1), FColor(255, 0, 255, 255)); 
	
	// Desired Direction
	FVector2D EndPoint1{ Agent.GetPosition() + (Steering.LinearVelocity * 300)};
	DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), 
		FVector(EndPoint1.X, EndPoint1.Y, 1), FColor(0, 255, 0, 255)); 
	
	// Steering
	FVector2D EndPoint3{ Agent.GetPosition() + (EndPoint1 - EndPoint2)};
	DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), 
		FVector(EndPoint3.X, EndPoint3.Y, 1), FColor(0, 255, 255, 255));
}

// SEEK
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent & Agent)
{
	SteeringOutput Steering{};
	Steering.LinearVelocity = Target.Position - Agent.GetPosition();
	Steering.LinearVelocity.Normalize();
	
	DrawDebugVectors(Agent, Steering);
	
	return Steering;
}

// FLEE
SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	Steering.LinearVelocity = Target.Position + Agent.GetPosition();
	Steering.LinearVelocity.Normalize();
	
	DrawDebugVectors(Agent, Steering);
	
	return Steering;
}

// ARRIVE
// SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
// {
// 	SteeringOutput Steering{};
// 	Steering.LinearVelocity = Target.Position - Agent.GetPosition();
// 	Steering.LinearVelocity.Normalize();
// 	
// 	DrawDebugVectors(Agent, Steering);
// 	
// 	return Steering;
// }