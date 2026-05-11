// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_FSM.h"

#include "FSMComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DecisionMaking/GameAIController.h"
#include "States/Chase.h"
#include "States/Patrol.h"
#include "States/Search.h"


// Sets default values
ALevel_FSM::ALevel_FSM()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_FSM::BeginPlay()
{
	Super::BeginPlay();
	
	Agent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, 
	FVector{0,0,90}, FRotator::ZeroRotator);
	Agent->SetDebugRenderingEnabled(false);
	Agent->SetSteeringBehavior(pSeekBehavior.get());
	
	AgentGuard = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, 
	FVector{100,0,90}, FRotator::ZeroRotator);
	AgentGuard->SetDebugRenderingEnabled(false);
	
	// TODO
	if (AGameAIController* AIController = Cast<AGameAIController>(AgentGuard->GetController()))
	{
		
		if (UFSMComponent* FSM = Cast<UFSMComponent>(AIController->GetBrainComponent()))
		{
			
			UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, 
				AIController->GetPerceptionComponent()->GetDominantSense(), 
				Agent);
			
			std::unique_ptr<GameAI::FSM::Search> search{ std::make_unique<GameAI::FSM::Search>() };
			std::unique_ptr<GameAI::FSM::Patrol> patrol{ std::make_unique<GameAI::FSM::Patrol>() };
			std::unique_ptr<GameAI::FSM::Chase> chase{ std::make_unique<GameAI::FSM::Chase>() };
			
			std::function<bool()> IsSearchingTooLong([&AIController, maxSearchTime{ 5.f }]() -> bool
			{
				float deltaTime{ AIController->GetBlackboardComponent()->GetValueAsFloat(FName("DeltaTime")) };
				float timeSearching{ AIController->GetBlackboardComponent()->GetValueAsFloat(FName("TimeSearching")) };
				if (timeSearching + deltaTime >= maxSearchTime)
				{
					AIController->GetBlackboardComponent()->SetValueAsFloat(FName("TimeSearching"), 0.f);
					return true;
				}
				else
				{
					AIController->GetBlackboardComponent()->SetValueAsFloat(FName("TimeSearching"), timeSearching + deltaTime);
					return false;
				}
			});
			
			std::function<bool()> IsTargetVisible([AIController, detectionRadius{ 10.f }]() -> bool
			{
				if ((AIController->GetBlackboardComponent()->GetValueAsVector(FName("ThiefPosition")) 
					- AIController->GetNavAgentLocation()).Length() < detectionRadius)
				{
					TArray<AActor*> perceivedActors{};
					AIController->GetPerceptionComponent()->GetCurrentlyPerceivedActors(nullptr, perceivedActors);
					
					if (perceivedActors.Num() > 0)
					{
						return true;
					}
				}
				return false;
			});
			
			std::function<bool()> IsTargetNotVisible([IsTargetVisible]() -> bool
			{
				return !IsTargetVisible;
			});
			
			FSM->AddTransition(search.get(), patrol.get(), IsSearchingTooLong);
			FSM->AddTransition(patrol.get(), chase.get(), IsTargetVisible);
			FSM->AddTransition(search.get(), chase.get(), IsTargetVisible);
			FSM->AddTransition(chase.get(), search.get(), IsTargetNotVisible);
			FSM->AddState(std::move(search));
			FSM->AddState(std::move(patrol), true, true);
			FSM->AddState(std::move(chase));
			
			AIController->GetBlackboardComponent()->SetValueAsFloat(FName("TimeSearching"), 0.f);
			AIController->GetBlackboardComponent()->SetValueAsFloat(FName("DeltaTime"), 0.f);
			
			std::vector<FVector2D> path{FVector2D(100, -100), FVector2D(100, 0)};
			//TODO: figure out how to put TArray on blackboard
			
			AIController->RunFiniteStateMachine();
		}
	} 
}

// Called every frame
void ALevel_FSM::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	pSeekBehavior->SetTarget(MouseTarget);
	
	if (AGameAIController* AIController = Cast<AGameAIController>(AgentGuard->GetController()))
	{
		AIController->GetBlackboardComponent()->SetValueAsFloat(FName("DeltaTime"), DeltaTime);
		AIController->GetBlackboardComponent()->SetValueAsVector(FName("ThiefPosition"), FVector(Agent->GetPosition().X, Agent->GetPosition().Y, 0));
		AIController->GetBlackboardComponent()->SetValueAsVector(FName("LastSpottedThiefPosition"), FVector(Agent->GetPosition().X, Agent->GetPosition().Y, 0));
		AIController->GetBlackboardComponent()->SetValueAsVector(FName("GuardPosition"), FVector(AgentGuard->GetPosition().X, AgentGuard->GetPosition().Y, 0));
	}
}

