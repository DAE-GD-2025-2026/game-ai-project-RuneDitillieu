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

void DrawTopDownDebugCircle(const ASteeringAgent& Agent, FVector&& Center, float Radius, FColor&& Color)
{
	DrawDebugCircle(Agent.GetWorld(), Center, Radius, 16, Color, 
		false, -1, 0, 0,
		FVector(0, 1, 0), FVector(1, 0, 0), false);
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
SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	if (!OriginalSpeedIsSet)
	{
		OriginalMaxLinearSpeed = Agent.GetMaxLinearSpeed();
		OriginalSpeedIsSet = true;
	}
	
	SteeringOutput Steering{};
	Steering.LinearVelocity = Target.Position - Agent.GetPosition();
	const float DistToTarget{ float(Steering.LinearVelocity.Length()) };
	Steering.LinearVelocity.Normalize();
	
	constexpr float SlowRadius{ 500.f };
	constexpr float TargetRadius{ SlowRadius / 5.f };
	
	if (DistToTarget > SlowRadius) // full speed
	{
		Agent.SetMaxLinearSpeed(OriginalMaxLinearSpeed);
	}
	else if (DistToTarget < SlowRadius && DistToTarget > TargetRadius) // slow down
	{
		Agent.SetMaxLinearSpeed(Agent.GetMaxLinearSpeed() - Agent.GetMaxLinearSpeed() / (SlowRadius - TargetRadius) * 7);
	}
	else if (DistToTarget < TargetRadius) // stop
	{
		Agent.SetMaxLinearSpeed(1);
	}
	
	// draw debug
	DrawDebugVectors(Agent, Steering);
	DrawTopDownDebugCircle(Agent, FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), SlowRadius, FColor( 0, 0, 255, 255));
	DrawTopDownDebugCircle(Agent, FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), TargetRadius, FColor( 255, 180, 0, 255));
	DrawTopDownDebugCircle(Agent, FVector(Target.Position.X, Target.Position.Y, 1), 10, FColor(255, 0, 0, 255));
	
	return Steering;
}