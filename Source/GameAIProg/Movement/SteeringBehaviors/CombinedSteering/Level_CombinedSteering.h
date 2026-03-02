// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>
#include "CoreMinimal.h"
#include "CombinedSteeringBehaviors.h"
#include "GameAIProg/Shared/Level_Base.h"
#include "GameAIProg/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"
#include "Level_CombinedSteering.generated.h"

UCLASS()
class GAMEAIPROG_API ALevel_CombinedSteering : public ALevel_Base
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevel_CombinedSteering();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

private:
	//Datamembers
	bool UseMouseTarget = false;
	bool CanDebugRender = false;

	//Behaviors
	std::unique_ptr<Seek> pSeekBehavior{ std::make_unique<Seek>() };
	std::unique_ptr<Wander> pWanderBehavior{ std::make_unique<Wander>() };
	std::unique_ptr<Evade> pEvadeBehavior{ std::make_unique<Evade>() };
	std::unique_ptr<BlendedSteering> pBlendedSteering{ nullptr };
	std::unique_ptr<PrioritySteering> pPrioritySteering{ nullptr };
	ASteeringAgent* pSteeringAgentBlended{ nullptr };
	ASteeringAgent* pSteeringAgentPriority{ nullptr };
};
