#include "BFS.h"

#include <unordered_map>
#include <queue>
#include <unordered_set>

#include "Shared/Graph/Graph.h"

using namespace GameAI;

BFS::BFS(Graph* const pGraph)
	: pGraph(pGraph)
{
}

std::vector<Node*> BFS::FindPath(Node* const pStartNode, Node* const pDestinationNode) const
{
	std::vector<Node*> path{};
	
	std::queue<Node*> openList{};
	std::unordered_set<Node*> closedList{};
	std::unordered_map<Node*, Node*> parent{};
	
	Node* currentNode{ nullptr };
	
	openList.push(pStartNode);
	closedList.emplace(pStartNode);
	
	// while there are nodes to be checked
	while (!openList.empty())
	{
		currentNode = openList.front();
		openList.pop();
		
		// if we reached the goal
		if (currentNode == pDestinationNode)
		{
			// reconstruct path and return it
			while (currentNode != pStartNode)
			{
				path.emplace_back(currentNode);
				currentNode = parent.find(currentNode)->second;
			}
			path.emplace_back(pStartNode);
			std::reverse(path.begin(), path.end());
			return path;
		}
		
		// for every neighbor
		for (Connection* connection : pGraph->FindConnectionsFrom(currentNode->GetId()))
		{
			Node* neighbor = pGraph->GetNode(connection->GetToId()).get();
			
			// if we haven't checked this neighbor yet, add them to the queue
			if (!closedList.contains(neighbor))
			{
				closedList.emplace(neighbor);
				parent[neighbor] = currentNode;
				openList.push(neighbor);
			}
		}
	}
	
	// no path was found
	return path;
}
