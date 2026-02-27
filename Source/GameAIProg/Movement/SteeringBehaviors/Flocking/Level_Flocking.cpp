// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_Flocking.h"


// Sets default values
ALevel_Flocking::ALevel_Flocking()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_Flocking::BeginPlay()
{
	Super::BeginPlay();

	TrimWorld->SetTrimWorldSize(3000.f);
	TrimWorld->bShouldTrimWorld = true;

	pAgentToEvade = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{0, 0,90}, FRotator::ZeroRotator);
	pAgentToEvade->SetSteeringBehavior(pWanderBehavior.get());
	
	pFlock = TUniquePtr<Flock>(
		new Flock(
			GetWorld(),
			SteeringAgentClass,
			FlockSize,
			TrimWorld->GetTrimWorldSize(),
			pAgentToEvade,
			true)
			);
}

// Called every frame
void ALevel_Flocking::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TrimWorld->SetTrimWorldSize(1400.f);
	
	pFlock->ImGuiRender(WindowPos, WindowSize);
	pFlock->Tick(DeltaTime);
	pFlock->RenderDebug();
	if (bUseMouseTarget)
		pFlock->SetTarget_Seek(MouseTarget);
	
	if (pAgentToEvade->GetDebugRenderingEnabled())
	{
		DrawDebugCircle(GetWorld(), FVector(pAgentToEvade->GetPosition().X, pAgentToEvade->GetPosition().Y, 1), 50, 16, 
			FColor(255, 0, 0, 255), false, -1, 0, 0,
		FVector(0, 1, 0), FVector(1, 0, 0), false);
	}
}

