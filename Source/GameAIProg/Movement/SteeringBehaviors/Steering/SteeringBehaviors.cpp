#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"
#include "Shared/DebugHelpers.h"


// SEEK
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent & Agent)
{
	SteeringOutput Steering{};
	Steering.LinearVelocity = Target.Position - Agent.GetPosition();
	Steering.LinearVelocity.Normalize();
	
	if (Agent.GetDebugRenderingEnabled())
	{
		Debug::DrawSteeringVectors(Agent, Steering);
		Debug::DrawTarget(Agent.GetWorld(), Target.Position);
	}
	
	return Steering;
}

// FLEE
SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	Steering.LinearVelocity = Agent.GetPosition() - Target.Position;
	Steering.LinearVelocity.Normalize();
	
	if (Agent.GetDebugRenderingEnabled())
	{
		Debug::DrawSteeringVectors(Agent, Steering);
		Debug::DrawTarget(Agent.GetWorld(), Target.Position);
	}
	
	return Steering;
}

// ARRIVE
SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	if (!m_OriginalSpeedIsSet)
	{
		m_OriginalMaxLinearSpeed = Agent.GetMaxLinearSpeed();
		m_OriginalSpeedIsSet = true;
	}
	
	SteeringOutput Steering{};
	Steering.LinearVelocity = Target.Position - Agent.GetPosition();
	const float DistToTarget{ static_cast<float>(Steering.LinearVelocity.Length()) };
	if (DistToTarget > 0.001f)
		Steering.LinearVelocity /= DistToTarget;
	
	const float SlowRadius{ m_TargetRadius * 5.f };
	
	if (DistToTarget > SlowRadius) // full speed
	{
		Agent.SetMaxLinearSpeed(m_OriginalMaxLinearSpeed);
	}
	else if (DistToTarget < SlowRadius && DistToTarget > m_TargetRadius) // slow down
	{
		Agent.SetMaxLinearSpeed(Agent.GetMaxLinearSpeed() - Agent.GetMaxLinearSpeed() / (SlowRadius - m_TargetRadius) * 7);
	}
	else if (DistToTarget < m_TargetRadius) // stop
	{
		Agent.SetMaxLinearSpeed(0);
	}
	
	// draw debug
	if (Agent.GetDebugRenderingEnabled())
	{
		Debug::DrawSteeringVectors(Agent, Steering);
		// slow radius
		Debug::DrawTopDownDebugCircle(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), SlowRadius, 
			FColor( 0, 0, 255, 255));
		// target radius
		Debug::DrawTopDownDebugCircle(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), m_TargetRadius, 
			FColor( 255, 180, 0, 255));
		// target
		Debug::DrawTarget(Agent.GetWorld(), Target.Position);
	}
	
	return Steering;
}

// FACE 
SteeringOutput Face::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	const FVector2D DirToTarget{ Target.Position - Agent.GetPosition() };
	
	const float AgentForwardAngle{ static_cast<float>(Agent.GetActorRotation().Yaw) / 180.f * PI };
	const float AngleToTarget{ static_cast<float>(atan2(DirToTarget.Y, DirToTarget.X))};
	
	float AngleDiff{ AngleToTarget - AgentForwardAngle };
	if (abs(AngleDiff) < 0.01f)
		AngleDiff = 0.f;
	else if (AngleDiff >  PI)
		AngleDiff -= 2 * PI;
	else if (AngleDiff < -PI)
		AngleDiff += 2 * PI;
	
	Steering.AngularVelocity = AngleDiff / abs(AngleDiff);
	
	if (Agent.GetDebugRenderingEnabled())
	{
		Debug::DrawTarget(Agent.GetWorld(), Target.Position);
	}
	
	return Steering;
}

// PURSUIT
SteeringOutput Pursuit::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	const float DistToTarget{ static_cast<float>((Target.Position - Agent.GetPosition()).Length()) };
	const float Time{ DistToTarget / Agent.GetMaxLinearSpeed() };
	const FVector2D PredictedPosition{ Target.Position + Target.LinearVelocity * Time };
	Steering.LinearVelocity = PredictedPosition - Agent.GetPosition();
	Steering.LinearVelocity.Normalize();
	
	if (Agent.GetDebugRenderingEnabled())
	{
		Debug::DrawSteeringVectors(Agent, Steering);
		Debug::DrawTarget(Agent.GetWorld(), Target.Position);
		// predicted position
		Debug::DrawTopDownDebugCircle(Agent.GetWorld(), FVector(PredictedPosition.X, PredictedPosition.Y, 1), 10, 
			FColor(255, 255, 0, 255));
	}
	
	return Steering;
}

// EVADE
SteeringOutput Evade::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	const float DistToTarget{ static_cast<float>((Target.Position - Agent.GetPosition()).Length()) };
	const float Time{ DistToTarget / Agent.GetMaxLinearSpeed() };
	const FVector2D PredictedPosition{ Target.Position + Target.LinearVelocity * Time };
	Steering.LinearVelocity = Agent.GetPosition() - PredictedPosition;
	Steering.LinearVelocity.Normalize();
	
	if (Agent.GetDebugRenderingEnabled())
	{
		Debug::DrawSteeringVectors(Agent, Steering);
		Debug::DrawTarget(Agent.GetWorld(), Target.Position);
		// predicted position
		Debug::DrawTopDownDebugCircle(Agent.GetWorld(), FVector(PredictedPosition.X, PredictedPosition.Y, 1), 10, 
			FColor(255, 255, 0, 255));
		// evade radius
		if (m_UseEvadeRadius)
			Debug::DrawTopDownDebugCircle(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), m_EvadeRadius, 
				FColor(0, 0, 0, 1));
	}
	
	if (m_UseEvadeRadius && DistToTarget > m_EvadeRadius)
	{
		Steering.IsValid = false;
	}
	
	return Steering;
}

// WANDER
SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	const FVector2D CenterWanderCircle{ Agent.GetPosition() + Agent.GetLinearVelocity().GetSafeNormal() * m_OffsetDistance };
	const float AngleRad{ ((rand() % (m_MaxAngleChange * 2)) - m_MaxAngleChange) / 180.f * PI };
	m_WanderAngle += AngleRad;
	
	const FVector2D WanderPos{ CenterWanderCircle.X + cos(m_WanderAngle) * m_Radius, 
						 CenterWanderCircle.Y + sin(m_WanderAngle) * m_Radius };
	Target.Position = WanderPos;
	
	if (Agent.GetDebugRenderingEnabled())
	{
		// wander circle
		Debug::DrawTopDownDebugCircle(Agent.GetWorld(), FVector(CenterWanderCircle.X, CenterWanderCircle.Y, 1), m_Radius, 
			FColor(0, 0, 255, 255));
	}
	
	return Seek::CalculateSteering(DeltaT, Agent);
}
