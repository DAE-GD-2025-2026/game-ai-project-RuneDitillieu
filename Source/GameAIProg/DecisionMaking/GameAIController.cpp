// Fill out your copyright notice in the Description page of Project Settings.


#include "GameAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "FSM/FSMComponent.h"
#include "Perception/AISenseConfig_Sight.h"


// Sets default values
AGameAIController::AGameAIController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BrainComponent = CreateDefaultSubobject<UFSMComponent>(TEXT("FSMComponent"));
	
	// perception component
	UAISenseConfig_Sight* SenseConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SenseConfig_Sight");
 
	//Set default values
	SenseConfig->SightRadius = 300;
	SenseConfig->LoseSightRadius = 300;
	SenseConfig->PeripheralVisionAngleDegrees = 45.f;
	SenseConfig->DetectionByAffiliation.bDetectEnemies = true;
	SenseConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SenseConfig->DetectionByAffiliation.bDetectFriendlies = true;
 
	//Tell the perception comp to use the config object
	PerceptionComponent->ConfigureSense(*SenseConfig);
	PerceptionComponent->SetDominantSense(SenseConfig->GetSenseImplementation());
}

// Called when the game starts or when spawned
void AGameAIController::BeginPlay()
{
	Super::BeginPlay();
	
	// Create Blackboard if need be
	InitFiniteStateMachine();
	
	
}

// Called every frame
void AGameAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGameAIController::InitFiniteStateMachine()
{
	UFSMComponent* FSMComp = FindComponentByClass<UFSMComponent>();
	if (ensure(FSMComp) && FSMBlackboardAsset)
	{
		UBlackboardComponent* BlackboardComp = Blackboard;
		UseBlackboard(FSMBlackboardAsset, BlackboardComp);
		Blackboard = BlackboardComp;
	}
}

void AGameAIController::RunFiniteStateMachine()
{
	UFSMComponent* FSMComp = FindComponentByClass<UFSMComponent>();
	if (ensure(FSMComp))
	{
		FSMComp->StartLogic();
	}
}



