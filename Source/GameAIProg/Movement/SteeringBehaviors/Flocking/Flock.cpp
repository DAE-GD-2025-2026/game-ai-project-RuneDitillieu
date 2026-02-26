#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, FlockSize{ FlockSize }
	, pAgentToEvade{pAgentToEvade}
{
	std::vector<BlendedSteering::WeightedBehavior> WeightedBehaviors;
	WeightedBehaviors.reserve(5);
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pSeparationBehavior.get(), 0.35f));
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pCohesionBehavior.get(), 0.05f));
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pVelMatchBehavior.get(), 0.1f));
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pSeekBehavior.get(), 0.4f));
	WeightedBehaviors.push_back(BlendedSteering::WeightedBehavior(pWanderBehavior.get(), 0.1f));
	
	pBlendedSteering = std::make_unique<BlendedSteering>(WeightedBehaviors);
	
	Agents.SetNum(FlockSize);
	Neighbors.SetNum(FlockSize);
	
	for (int idx{ 0 }; idx < FlockSize; ++idx)
	{
		ASteeringAgent* Agent{ nullptr };
		bool AgentSuccessfullySpawned{ false };
		
		constexpr int SpawnRadius{ 1000 };
		while (!AgentSuccessfullySpawned)
		{
			FVector2D RandPos{ double((rand() % SpawnRadius) - (SpawnRadius / 2)), double((rand() % SpawnRadius) - (SpawnRadius / 2)) };
			if (Agent = pWorld->SpawnActor<ASteeringAgent>(AgentClass, FVector{RandPos.X, RandPos.Y,90}, FRotator::ZeroRotator))
				AgentSuccessfullySpawned = true;
		}
		
		Agent->SetActorTickEnabled(false);
		Agent->SetSteeringBehavior(pBlendedSteering.get());
		Agent->SetDebugRenderingEnabled(false);
		Agents[idx] = Agent;
	}
}

Flock::~Flock()
{
 // TODO: Cleanup any additional data
}

void Flock::Tick(float DeltaTime)
{
 // TODO: trim the agent to the world
	
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
		RegisterNeighbors(Agent);
 		Agent->Tick(DeltaTime);
	}
	
	RenderNeighborhood();
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
	for (ASteeringAgent* agent : Agents)
	{
		
	}
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

  // TODO: implement ImGUI checkboxes for debug rendering here

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

  // TODO: implement ImGUI sliders for steering behavior weights here
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
	RegisterNeighbors(Agents[0]);
	size_t index{ 0 };
	
	// draw red circle around first agent
	DrawDebugCircle(pWorld, FVector(Agents[0]->GetPosition().X, Agents[0]->GetPosition().Y, 1), 50, 16, 
			FColor(255, 0, 0, 255), false, -1, 0, 0,
		FVector(0, 1, 0), FVector(1, 0, 0), false);
	
	// draw neighbourhood radius
	DrawDebugCircle(pWorld, FVector(Agents[0]->GetPosition().X, Agents[0]->GetPosition().Y, 1), NeighborhoodRadius, 16, 
			FColor(0, 0, 0, 255), false, -1, 0, 0,
		FVector(0, 1, 0), FVector(1, 0, 0), false);
	
	// draw yellow circle around registered neighbors
	for (ASteeringAgent* neighbor : Neighbors)
	{
		DrawDebugCircle(pWorld, FVector(neighbor->GetPosition().X, neighbor->GetPosition().Y, 1), 50, 16, 
			FColor(255, 255, 0, 255), false, -1, 0, 0,
		FVector(0, 1, 0), FVector(1, 0, 0), false);
		
		++index;
		if (index >= NrOfNeighbors)
			break;
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
	for (ASteeringAgent* Neighbor : Neighbors)
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
	for (ASteeringAgent* neighbor : Neighbors)
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

