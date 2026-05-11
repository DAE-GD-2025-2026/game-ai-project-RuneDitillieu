#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) { return SteeringOutput(); };

	virtual void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};


// SEEK
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() override = default;

	//steering
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// FLEE
class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() override = default;
	
	//steering
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// ARRIVE
class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() override = default;
	
	//steering
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
	void SetTargetRadius(float Radius) { m_TargetRadius = Radius; }
	
protected:
	float m_OriginalMaxLinearSpeed{};
	bool m_OriginalSpeedIsSet{ false };
	float m_TargetRadius{ 100.f };
};

// FACE
class Face : public  ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() override = default;
	
	//steering
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// PURSUIT
class Pursuit : public  ISteeringBehavior
{
public:
	Pursuit() = default;
	virtual ~Pursuit() override = default;
	
	//steering
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// EVADE
class Evade : public  ISteeringBehavior
{
public:
	Evade() = default;
	virtual ~Evade() override = default;
	
	//steering
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
	void SetUseEvadeRadius(bool useEvadeRadius) { m_UseEvadeRadius = useEvadeRadius; }
	
private:
	bool m_UseEvadeRadius{ false };
	float m_EvadeRadius{ 300.f };
};

// WANDER
class Wander : public  Seek
{
public:
	Wander() = default;
	virtual ~Wander() override = default;
	
	//steering
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
	
	void SetWanderOffset(float Offset) { m_OffsetDistance = Offset; };
	void SetWanderRadius(float Radius) { m_Radius = Radius; };
	void SetMaxAngleChange(float Rad) { m_MaxAngleChange = Rad; };
	
protected:	
	float m_OffsetDistance{ 160.f };	// distance to circle center in degrees
	float m_Radius{ 100.f };			// radius of the circle
	int m_MaxAngleChange{ 15 };			// max allowed difference between calculated angles
	float m_WanderAngle{ 0.f };			// last calculated angle
};