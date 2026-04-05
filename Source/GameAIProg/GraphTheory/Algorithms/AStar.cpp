#include "AStar.h"
#include "String/FormatStringSan.h"

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}

std::vector<Node*>AStar::FindPath(Node* const pStartNode, Node* const pGoalNode)
{
	std::vector<Node*> path{};
	
	std::vector<NodeRecord> openList{ }; // nodes to be checked
	std::vector<NodeRecord> closedList{ }; // nodes already checked
	NodeRecord currentNodeRecord{ }; // to be evaluated node
	
	currentNodeRecord.pNode = pStartNode;
	openList.push_back(currentNodeRecord);
	
	// start of the algorithm
	while (!openList.empty())
	{
		// record with smallest f-cost
		currentNodeRecord = *std::min_element(openList.begin(), openList.end(), 
			[](NodeRecord first, NodeRecord smallest){ return first.estimatedTotalCost < smallest.estimatedTotalCost; });
		
		// if curRecord is the destNode, exit loop
		if (currentNodeRecord.pNode == pGoalNode)
			break;
		else
		{
			std::vector<Connection*> connections{ pGraph->FindConnectionsFrom(currentNodeRecord.pNode->GetId()) };
			// for every connection
			for (Connection* conn : connections)
			{
				bool skipToNextNeighbor{ false };
				
				// calc G-cost to connected node
				Node* pNextNode{ pGraph->GetNode(conn->GetToId()).get() };
				float GCost{ currentNodeRecord.costSoFar + GetHeuristicCost(currentNodeRecord.pNode, pNextNode)};
				
				// check if Node is in closed list
				for (NodeRecord NodeRec : closedList)
				{
					if (NodeRec.pNode == pNextNode)
					{
						// if the already recorded G-cost is smaller than the current G-cost
						if (NodeRec.costSoFar <= GCost)
						{
							skipToNextNeighbor = true;
						}
						else
						{
							closedList.erase(std::remove(closedList.begin(), closedList.end(), NodeRec));
							break;
						}
					}
				}
				
				for (NodeRecord NodeRec : openList)
				{
					if (NodeRec.pNode == pNextNode)
					{
						// if the already recorded G-cost is smaller than the current G-cost
						if (NodeRec.costSoFar <= GCost)
						{
							skipToNextNeighbor = true;
						}
						else
						{
							openList.erase(std::remove(openList.begin(), openList.end(), NodeRec));
							break;
						}
					}
				}
				
				if (!skipToNextNeighbor)
				{
					openList.emplace_back(NodeRecord{ pNextNode, conn, 
						GCost, GCost + GetHeuristicCost(pNextNode, pGoalNode) });
				}
			}
			
			auto it{ std::remove_if(openList.begin(), openList.end(), 
				[currentNodeRecord](NodeRecord nr){return nr.pNode == currentNodeRecord.pNode;})};
			openList.erase(it, openList.end());
			closedList.push_back(currentNodeRecord);
		}
	}

	while (currentNodeRecord.pNode != pStartNode)
	{
		path.push_back(currentNodeRecord.pNode);
		for (NodeRecord NodeRec : closedList)
		{
			if (NodeRec.pNode == pGraph->GetNode(currentNodeRecord.pConnection->GetFromId()).get())
			{
				currentNodeRecord = NodeRec;
				break;
			}
		}
	}
	path.push_back(pStartNode);
	std::reverse(path.begin(), path.end());
	
	return path;
}

float AStar::GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const
{
	FVector2D toDestination = pGraph->GetNode(pEndNode->GetId())->GetPosition() - pGraph->GetNode(pStartNode->GetId())->GetPosition();
	return HeuristicFunction(abs(toDestination.X), abs(toDestination.Y));
}