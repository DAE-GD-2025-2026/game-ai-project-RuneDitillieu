#include "NavGraphPathfinding.h"

#include "AStar.h"
#include "PathSmoothing.h"
#include "VectorTypes.h"
#include "Shared/Graph/NavGraph/NavGraph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

using namespace GameAI;

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos,
	NavGraph* const pNavGraph, std::vector<FVector2D>& debugNodePositions, std::vector<NavLine>& debugPortals) 
{
	//Create the path to return
	std::vector<FVector2D> finalPath{};

	//Get the start and endTriangle
	FVector2D start{};
	const TriPolygon::Triangle* startTriangle{ pNavGraph->GetNavPolygon()->GetClosestTriangleToPosition(startPos, start) };
	FVector2D end{};
	const TriPolygon::Triangle* endTriangle{ pNavGraph->GetNavPolygon()->GetClosestTriangleToPosition(endPos, end) };

	if (startTriangle == nullptr || endTriangle == nullptr)
	{
		return finalPath;
	}
	else if (startTriangle == endTriangle)
	{
		finalPath.emplace_back(start);
		return finalPath;
	}
	
	//We have valid start/end triangles and they are not the same
	//=> Start looking for a path
	//Copy the graph
	
	std::shared_ptr<NavGraph> pGraph{ pNavGraph->Clone() };

	//Create Extra node for the Start Node (Agent's position)

	int startNodeId{ pGraph->AddNode(std::make_unique<NavGraphNode>(startPos, -1)) };
	for (auto& edge : startTriangle->GetEdges())
	{
		std::optional<int> edgeId{ pNavGraph->GetNavPolygon()->FindEdgeIndex(edge) };
		int nodeId{ pGraph->GetNodeIdFromEdgeIndex(edgeId.value()) };
		if (nodeId > -1)
		{
			pGraph->AddConnection(startNodeId, nodeId);
		}
		
	}
	
	//Create extra node for the endNode

	int endNodeId{ pGraph->AddNode(std::make_unique<NavGraphNode>(endPos, -1)) };
	for (auto& edge : endTriangle->GetEdges())
	{
		std::optional<int> edgeId{pNavGraph->GetNavPolygon()->FindEdgeIndex(edge).value() };
		int nodeId{ pGraph->GetNodeIdFromEdgeIndex(edgeId.value()) };
		if (nodeId > -1)
		{
			pGraph->AddConnection(nodeId, endNodeId);
		}
	}
	
	//Run A star on new graph

	AStar astar{ pGraph.get(), HeuristicFunctions::Chebyshev };
	for (auto* node : astar.FindPath(pGraph->GetNode(startNodeId).get(), pGraph->GetNode(endNodeId).get()))
	{
		finalPath.emplace_back(node->GetPosition());
		debugNodePositions.emplace_back(node->GetPosition());
	}
	
	//Debug Visualisation

	// Extra: Run optimiser on new graph (First check if everything works without SSFA!)
	// debugPortals = SSFA::FindPortals(nodes, *pNavGraph->GetNavPolygon());
	// finalPath = SSFA::OptimizePortals(debugPortals, *pNavGraph->GetNavPolygon());
	
	return finalPath;
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph)
{
	std::vector<FVector2D> debugNodePositions{};
	std::vector<NavLine> debugPortals{};

	return FindPath(startPos, endPos, pNavGraph, debugNodePositions, debugPortals);
}