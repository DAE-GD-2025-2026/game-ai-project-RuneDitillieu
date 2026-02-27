#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/SpacePartitioning/SpacePartitioning.h"
#include "Shared/ImGuiHelpers.h"
#include "Shared/Level_Base.h"

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
#ifdef GAMEAI_USE_SPACE_PARTITIONING
	pPartitionedSpace = std::make_unique<CellSpace>(CellSpace(pWorld, WorldSize * 2, WorldSize * 2, 14, 14, FlockSize));
#endif
	
	std::vector<BlendedSteering::WeightedBehavior> WeightedBehaviors;
	WeightedBehaviors.reserve(5);
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pSeparationBehavior.get(), 0.35f));
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pCohesionBehavior.get(), 0.05f));
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pVelMatchBehavior.get(), 0.1f));
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pSeekBehavior.get(), 0.4f));
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pWanderBehavior.get(), 0.1f));
	
	pBlendedSteering = std::make_unique<BlendedSteering>(WeightedBehaviors);
	pPrioritySteering = std::make_unique<PrioritySteering>(PrioritySteering({pEvadeBehavior.get(), pBlendedSteering.get()}));
	
	Agents.SetNum(FlockSize);
#ifndef GAMEAI_USE_SPACE_PARTITIONING
	Neighbors.SetNum(FlockSize);
#endif
	
	
	for (int idx{ 0 }; idx < FlockSize; ++idx)
	{
		ASteeringAgent* Agent{ nullptr };
		bool AgentSuccessfullySpawned{ false };
		
		const int SpawnRadius{ FlockSize * 12 };
		while (!AgentSuccessfullySpawned)
		{
			FVector2D RandPos{ double((rand() % SpawnRadius) - (SpawnRadius / 2)), double((rand() % SpawnRadius) - (SpawnRadius / 2)) };
			if (Agent = pWorld->SpawnActor<ASteeringAgent>(AgentClass, FVector{RandPos.X, RandPos.Y,90}, FRotator::ZeroRotator))
				AgentSuccessfullySpawned = true;
		}
		
		Agent->SetActorTickEnabled(false);
		Agent->SetSteeringBehavior(pPrioritySteering.get());
		Agent->SetDebugRenderingEnabled(false);
		Agents[idx] = Agent;
		
#ifdef GAMEAI_USE_SPACE_PARTITIONING
		//pPartitionedSpace->AddAgent(*Agents[idx]);
#endif
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
	
	for (ASteeringAgent* Agent : Agents )
	{
#ifndef GAMEAI_USE_SPACE_PARTITIONING
		RegisterNeighbors(Agent);
#endif
		
 		//Agent->Tick(DeltaTime);
	}
	
	//RenderNeighborhood();
}

void Flock::RenderDebug()
{
	if (DebugRenderSteering)
	{
		for (ASteeringAgent* Agent : Agents)
		{
			RenderSteering(*Agent);
		}
	}
	
	pPartitionedSpace->RenderCells();
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
#ifndef GAMEAI_USE_SPACE_PARTITIONING
		RegisterNeighbors(Agents[0]);
#endif
		
		size_t index{ 0 };
	
		// draw red circle around first agent
		DrawDebugCircle(pWorld, FVector(Agents[0]->GetPosition().X, Agents[0]->GetPosition().Y, 1), 50, 16, 
				FColor(0, 255, 0, 255), false, -1, 0, 0,
			FVector(0, 1, 0), FVector(1, 0, 0), false);
	
		// draw neighbourhood radius
		DrawDebugCircle(pWorld, FVector(Agents[0]->GetPosition().X, Agents[0]->GetPosition().Y, 1), NeighborhoodRadius, 16, 
				FColor(0, 0, 0, 255), false, -1, 0, 0,
			FVector(0, 1, 0), FVector(1, 0, 0), false);
	
		// draw yellow circle around registered neighbors
		for (ASteeringAgent* neighbor : GetNeighbors())
		{
			DrawDebugCircle(pWorld, FVector(neighbor->GetPosition().X, neighbor->GetPosition().Y, 1), 50, 16, 
				FColor(255, 255, 0, 255), false, -1, 0, 0,
			FVector(0, 1, 0), FVector(1, 0, 0), false);
		
			++index;
			if (index >= NrOfNeighbors)
				break;
		}
	}
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	NrOfNeighbors = 0;
	
	for (ASteeringAgent* const PossibleNeighbor : Agents)
	{
		if (PossibleNeighbor != pAgent)
		{
			FVector2D VecToAgent = PossibleNeighbor->GetPosition() - pAgent->GetPosition();
			if (VecToAgent.Length() <= NeighborhoodRadius)
			{
				Neighbors[NrOfNeighbors] = PossibleNeighbor;
				++NrOfNeighbors;
			}
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	FVector2D avgPosition{};

	size_t index{ 0 };
	for (ASteeringAgent* Neighbor : GetNeighbors())
	{
		if (Neighbor)
		{
			if (index == 0)
				avgPosition = Neighbor->GetPosition();
			else
			{
				avgPosition += (Neighbor->GetPosition() - avgPosition) / 2.f;
			}
		
			++index;
			if (index > NrOfNeighbors)
				break;
		}
	}
	
	return avgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	FVector2D avgVelocity = FVector2D::ZeroVector;

	size_t index{ 0 };
	for (ASteeringAgent* neighbor : GetNeighbors())
	{
		avgVelocity += neighbor->GetLinearVelocity();
		
		++index;
		if (index >= NrOfNeighbors)
			break;
	}
	
	if (index > 0)
		avgVelocity.Normalize();
	
	return avgVelocity;
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
