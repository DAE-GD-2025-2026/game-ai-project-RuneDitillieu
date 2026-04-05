#pragma once
#include "Movement/SteeringBehaviors/SteeringAgent.h"

namespace Debug
{
	inline void DrawTopDownDebugCircle(UWorld* pWorld, const FVector& Center, float Radius, FColor&& Color)
	{
		DrawDebugCircle(pWorld, Center, Radius, 16, Color, 
			false, -1, 0, 0,
			FVector(0, 1, 0), FVector(1, 0, 0), false);
	}
	
	/// Draw a small red circle at the target location
	inline void DrawTarget(UWorld* pWorld, const FVector2D& Target)
	{
		DrawTopDownDebugCircle(pWorld, FVector(Target.X, Target.Y, 1), 10, FColor(255, 0, 0, 255));
	}
	
	inline void DrawDebugRect(UWorld* pWorld, const FVector2D& TopLeft, const FVector2D& TopRight, 
		const FVector2D& BottomLeft, const FVector2D& BottomRight, const FColor& Color)
	{
		DrawDebugLine(pWorld, FVector(BottomLeft.X, BottomLeft.Y, 1.f), 
			FVector(BottomRight.X, BottomRight.Y, 1.f), Color);
		DrawDebugLine(pWorld, FVector(TopLeft.X, TopLeft.Y, 1.f), 
			FVector(TopRight.X, TopRight.Y, 1.f), Color);
		DrawDebugLine(pWorld, FVector(BottomLeft.X, BottomLeft.Y, 1.f), 
			FVector(TopLeft.X, TopLeft.Y, 1.f), Color);
		DrawDebugLine(pWorld, FVector(TopRight.X, TopRight.Y, 1.f), 
			FVector(BottomRight.X, BottomRight.Y, 1.f), Color);
	}
	
	inline void DrawDebugRect(UWorld* pWorld, const FVector2D& Center, float HalfWidth, float HalfHeight, const FColor& Color)
	{
		FVector2D TopLeft(Center.X - HalfWidth, Center.Y - HalfHeight);
		FVector2D TopRight(Center.X + HalfWidth, Center.Y - HalfHeight);
		FVector2D BottomLeft(Center.X - HalfWidth, Center.Y + HalfHeight);
		FVector2D BottomRight(Center.X + HalfWidth, Center.Y + HalfHeight);
		
		DrawDebugRect(pWorld, TopLeft, TopRight, BottomLeft, BottomRight, Color);
	}
	
	/// Draw 3 debug vectors: Velocity in magenta, Desired Direction in green, and Steering in cyan
	inline void DrawSteeringVectors(const ASteeringAgent& Agent, const SteeringOutput& Steering)
	{
		// Velocity
		const FVector2D EndPoint2{Agent.GetPosition() + Agent.GetLinearVelocity() * 0.5f};
		DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), 
			FVector(EndPoint2.X, EndPoint2.Y, 1), FColor(255, 0, 255, 255)); 
	
		// Desired Direction
		const FVector2D EndPoint1{ Agent.GetPosition() + (Steering.LinearVelocity * 300)};
		DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), 
			FVector(EndPoint1.X, EndPoint1.Y, 1), FColor(0, 255, 0, 255)); 
	
		// Steering
		const FVector2D EndPoint3{ Agent.GetPosition() + (EndPoint1 - EndPoint2)};
		DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), 
			FVector(EndPoint3.X, EndPoint3.Y, 1), FColor(0, 255, 255, 255));
	}
}
