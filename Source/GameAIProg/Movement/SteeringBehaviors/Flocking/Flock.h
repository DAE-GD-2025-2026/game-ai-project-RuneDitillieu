#pragma once

// Toggle this define to enable/disable spatial partitioning
#define GAMEAI_USE_SPACE_PARTITIONING

#include "FlockingSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"
#include "Movement/SteeringBehaviors/SteeringHelpers.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include <memory>
#include "imgui.h"
#include "Shared/WorldTrimVolume.h"
#include "../SpacePartitioning/SpacePartitioning.h"

class CellSpace;
class Flock final
{
public:
	Flock(
	UWorld* pWorld,
	AWorldTrimVolume* pTrimWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize = 10, 
	float WorldSize = 100.f, 
	ASteeringAgent* const pAgentToEvade = nullptr, 
	bool bTrimWorld = false);

	~Flock();

	void Tick(float DeltaTime);
	void RenderDebug();
	void ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize);
	
	void RegisterNeighbors(ASteeringAgent* const Agent);
	int GetNrOfNeighbors() const { if (UseSpacePartitioning) return pPartitionedSpace->GetNrOfNeighbors(); return NrOfNeighbors; }
	const TArray<ASteeringAgent*>& GetNeighbors() const { if (UseSpacePartitioning) return pPartitionedSpace->GetNeighbors(); return Neighbors; }

	FVector2D GetAverageNeighborPos() const;
	FVector2D GetAverageNeighborVelocity() const;

	void SetTarget_Seek(FSteeringParams const & Target);

private:
	// For debug rendering purposes
	UWorld* pWorld{ nullptr };
	
	AWorldTrimVolume* pTrimWorld{ nullptr };
	float WorldSize;
	bool ShouldTrimWorld;
	
	int FlockSize{0};
	TArray<ASteeringAgent*> Agents{};

	std::unique_ptr<CellSpace> pPartitionedSpace{};
	int NrOfCellsX{ 10 };
	TArray<FVector2D> OldPositions{};

	TArray<ASteeringAgent*> Neighbors{};

	bool UseSpacePartitioning{ false };
	float NeighborhoodRadius{ 300.f };
	int NrOfNeighbors{0};

	ASteeringAgent* pAgentToEvade{ nullptr };
	
	//Steering Behaviors
	std::unique_ptr<Separation> pSeparationBehavior{ std::make_unique<Separation>(this) };
	std::unique_ptr<Cohesion> pCohesionBehavior{ std::make_unique<Cohesion>(this) };
	std::unique_ptr<VelocityMatch> pVelMatchBehavior{ std::make_unique<VelocityMatch>(this) };
	std::unique_ptr<Seek> pSeekBehavior{ std::make_unique<Seek>() };
	std::unique_ptr<Wander> pWanderBehavior{ std::make_unique<Wander>() };
	std::unique_ptr<Evade> pEvadeBehavior{ std::make_unique<Evade>() };
	
	std::unique_ptr<BlendedSteering> pBlendedSteering{};
	
	std::unique_ptr<PrioritySteering> pPrioritySteering{};

	// UI and rendering
	bool DebugRenderSteering{false};
	bool DebugRenderNeighborhood{true};
	bool DebugRenderPartitions{true};

	void RenderNeighborhood();
	void RenderSteering(const ASteeringAgent& /*, const SteeringOutput& Steering*/) const;
};
