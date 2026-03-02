#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/SpacePartitioning/SpacePartitioning.h"
#include "Shared/ImGuiHelpers.h"
#include "Shared/Level_Base.h"
#include "Shared/DebugHelpers.h"

Flock::Flock(
	UWorld* pWorld,
	AWorldTrimVolume* pTrimWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, pTrimWorld{pTrimWorld}
	, WorldSize{WorldSize}
	, ShouldTrimWorld{bTrimWorld}
	, FlockSize{FlockSize}
	, pAgentToEvade{pAgentToEvade}
{
	// init Partitioned Space
	constexpr int NumColsRows{ 14 };
	pPartitionedSpace = std::make_unique<CellSpace>(CellSpace(pWorld, (WorldSize + WorldSize / NumColsRows) * 2, (WorldSize + WorldSize / NumColsRows) * 2, NumColsRows, NumColsRows, FlockSize));
	
	// init Steering Behaviors
	std::vector<BlendedSteering::WeightedBehavior> WeightedBehaviors;
	WeightedBehaviors.reserve(5);
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pSeparationBehavior.get(), 0.35f));
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pCohesionBehavior.get(), 0.05f));
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pVelMatchBehavior.get(), 0.1f));
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pSeekBehavior.get(), 0.4f));
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pWanderBehavior.get(), 0.1f));
	
	pBlendedSteering = std::make_unique<BlendedSteering>(WeightedBehaviors);
	pPrioritySteering = std::make_unique<PrioritySteering>(PrioritySteering({pEvadeBehavior.get(), pBlendedSteering.get()}));
	
	// init arrays
	Agents.SetNum(FlockSize);
	OldPositions.SetNum(FlockSize);
	Neighbors.SetNum(FlockSize);
	
	// spawn agents
	for (int Idx{ 0 }; Idx < FlockSize; ++Idx)
	{
		ASteeringAgent* Agent{ nullptr };
		bool AgentSuccessfullySpawned{ false };
		
		const int SpawnRadius{ std::clamp(FlockSize * 12, 200, static_cast<int>(WorldSize)) };
		while (!AgentSuccessfullySpawned)
		{
			const FVector2D RandPos{ static_cast<double>((rand() % SpawnRadius) - (SpawnRadius / 2)), 
				static_cast<double>((rand() % SpawnRadius) - (SpawnRadius / 2)) };
			if (Agent = pWorld->SpawnActor<ASteeringAgent>(AgentClass, FVector{RandPos.X, RandPos.Y,90}, FRotator::ZeroRotator))
				AgentSuccessfullySpawned = true;
		}
		
		Agent->SetActorTickEnabled(false);
		Agent->SetSteeringBehavior(pPrioritySteering.get());
		Agent->SetDebugRenderingEnabled(false);
		
		Agents[Idx] = Agent;
		OldPositions[Idx] = Agent->GetPosition();
		pPartitionedSpace->AddAgent(*Agent);
	}
}

Flock::~Flock()
{
}

void Flock::Tick(float DeltaTime)
{
	if (ShouldTrimWorld)
	{
		pTrimWorld->SetTrimWorldSize(WorldSize);
	}
	
	if (pAgentToEvade)
	{
		FTargetData Target;
		Target.Position = pAgentToEvade->GetPosition();
		Target.Orientation = pAgentToEvade->GetRotation();
		Target.LinearVelocity = pAgentToEvade->GetLinearVelocity();
		Target.AngularVelocity = pAgentToEvade->GetAngularVelocity();
		pEvadeBehavior->SetTarget(Target);
	}
	
	size_t Index{ 0 };
	for (ASteeringAgent* Agent : Agents )
	{
		RegisterNeighbors(Agent);
 		Agent->Tick(DeltaTime);
		if (UseSpacePartitioning)
		{
			pPartitionedSpace->UpdateAgentCell(*Agent, OldPositions[Index]);
			OldPositions[Index] = Agent->GetPosition();
			++Index;
		}
	}
}

void Flock::RenderDebug()
{
	if (DebugRenderSteering)
	{
		for (const ASteeringAgent* Agent : Agents)
		{
			RenderSteering(*Agent);
		}
	}
	
	if (UseSpacePartitioning)
		pPartitionedSpace->RenderCells();
	RenderNeighborhood();
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();
		
		if (ImGui::Checkbox("Trim World", &pTrimWorld->bShouldTrimWorld))
		{
			ShouldTrimWorld = pTrimWorld->bShouldTrimWorld;
		}
		ImGui::Spacing();
		
		ImGui::Checkbox("Use SpacePartitioning", &UseSpacePartitioning);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		
		ImGui::Checkbox("Debug Render Steering", &DebugRenderSteering);
		ImGui::Spacing();
		
		ImGui::Checkbox("Debug Render Neighborhood", &DebugRenderNeighborhood);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();
		
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Separation",
			pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Cohesion",
			pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Vel Match",
			pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
			pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
			pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight = InVal; }, "%.2f");
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
	if (DebugRenderNeighborhood)
	{
		if (UseSpacePartitioning)
		{
			// draw bounding box for checking cells
			Debug::DrawDebugRect(pWorld, Agents[0]->GetPosition(), 
				NeighborhoodRadius, NeighborhoodRadius, FColor(255, 255, 0, 255));
		}
		RegisterNeighbors(Agents[0]);
	
		// draw red circle around first agent
		Debug::DrawTopDownDebugCircle(pWorld, FVector(Agents[0]->GetPosition().X, Agents[0]->GetPosition().Y, 1), 50,
			FColor(0, 255, 0, 255));
	
		// draw neighbourhood radius
		Debug::DrawTopDownDebugCircle(pWorld, FVector(Agents[0]->GetPosition().X, Agents[0]->GetPosition().Y, 1), NeighborhoodRadius,
			FColor(0, 0, 0, 255));
		
		// draw yellow circle around registered neighbors
		size_t Index{ 0 };
		for (const ASteeringAgent* Neighbor : GetNeighbors())
		{
			if (Neighbor != nullptr)
			{
				DrawDebugCircle(pWorld, FVector(Neighbor->GetPosition().X, Neighbor->GetPosition().Y, 1), 50, 16, 
				FColor(255, 255, 0, 255), false, -1, 0, 0,
			FVector(0, 1, 0), FVector(1, 0, 0), false);
			}
		
			++Index;
			if (Index >= GetNrOfNeighbors())
				break;
		}
	}
}

void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	if (UseSpacePartitioning)
	{
		pPartitionedSpace->RegisterNeighbors(*pAgent, NeighborhoodRadius);
	}
	else
	{
		NrOfNeighbors = 0;
		for (ASteeringAgent* PossibleNeighbor : Agents)
		{
			if (PossibleNeighbor != pAgent)
			{
				const FVector2D VecToAgent = PossibleNeighbor->GetPosition() - pAgent->GetPosition();
				if (VecToAgent.Length() <= NeighborhoodRadius)
				{
					Neighbors[NrOfNeighbors] = PossibleNeighbor;
					++NrOfNeighbors;
				}
			}
		}
	}
}

FVector2D Flock::GetAverageNeighborPos() const
{
	FVector2D AvgPosition{};

	size_t Index{ 0 };
	for (const ASteeringAgent* Neighbor : GetNeighbors())
	{
		if (Neighbor)
		{
			if (Index == 0)
				AvgPosition = Neighbor->GetPosition();
			else
			{
				AvgPosition += (Neighbor->GetPosition() - AvgPosition) / 2.f;
			}
		
			++Index;
			if (Index >= GetNrOfNeighbors())
				break;
		}
	}
	
	return AvgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	FVector2D AvgVelocity = FVector2D::ZeroVector;

	size_t Index{ 0 };
	for (const ASteeringAgent* Neighbor : GetNeighbors())
	{
		AvgVelocity += Neighbor->GetLinearVelocity();
		
		++Index;
		if (Index >= GetNrOfNeighbors())
			break;
	}
	
	if (Index > 0)
		AvgVelocity.Normalize();
	
	return AvgVelocity;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	pSeekBehavior->SetTarget(Target);
}

void Flock::RenderSteering(const ASteeringAgent& Agent/*, const SteeringOutput& Steering*/) const
{
	// Velocity
	const FVector2D EndPoint2{Agent.GetPosition() + Agent.GetLinearVelocity() * 0.5f};
	DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), 
		FVector(EndPoint2.X, EndPoint2.Y, 1), FColor(255, 0, 255, 255)); 
	//
	// // Desired Direction
	// const FVector2D EndPoint1{ Agent.GetPosition() + (Steering.LinearVelocity * 300)};
	// DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), 
	// 	FVector(EndPoint1.X, EndPoint1.Y, 1), FColor(0, 255, 0, 255)); 
	//
	// // Steering
	// const FVector2D EndPoint3{ Agent.GetPosition() + (EndPoint1 - EndPoint2)};
	// DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 1), 
	// 	FVector(EndPoint3.X, EndPoint3.Y, 1), FColor(0, 255, 255, 255));
}
