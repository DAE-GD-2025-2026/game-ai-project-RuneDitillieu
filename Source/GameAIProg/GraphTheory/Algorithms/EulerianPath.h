#pragma once
#include <stack>
#include "Shared/Graph/Graph.h"

namespace GameAI
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	class EulerianPath final
	{
	public:
		EulerianPath(Graph* const pGraph);

		Eulerianity IsEulerian() const;
		std::vector<Node*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(const std::vector<Node*>& pNodes, std::vector<bool>& visited, int startIndex) const;
		bool IsConnected() const;

		Graph* m_pGraph;
	};

	inline EulerianPath::EulerianPath(Graph* const pGraph)
		: m_pGraph(pGraph)
	{
	}

	inline Eulerianity EulerianPath::IsEulerian() const
	{
		// If the graph is not connected, there can be no Eulerian Trail
		if (!IsConnected())
		{
			return Eulerianity::notEulerian;
		}
		
		// Count nodes with odd degree 
		int AmountOddNodes{ 0 };
		std::vector<Node*> Nodes{ m_pGraph->GetActiveNodes() };
		for (Node* Node : Nodes)
		{
			if (m_pGraph->FindConnectionsFrom(Node->GetId()).size() % 2 == 1)
			{
				++AmountOddNodes;
			}
		}
		
		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (AmountOddNodes > 2)
			return Eulerianity::notEulerian;
		
		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (unless there are only 2 nodes)
		// An Euler trail can be made, but only starting and ending in these 2 nodes
		if (AmountOddNodes == 2 && Nodes.size() > 2)
		{
			return Eulerianity::semiEulerian;
		}
		
		// A connected graph with no odd nodes is Eulerian
		if (AmountOddNodes == 0)
		{
			return Eulerianity::eulerian;
		}
		
		return Eulerianity::notEulerian;
	}

	inline std::vector<Node*> EulerianPath::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		Graph graphCopy = m_pGraph->Clone();
		std::vector<Node*> Path = {};
		std::vector<Node*> Nodes = graphCopy.GetActiveNodes();
		int currentNodeId{ Graphs::InvalidNodeId };
		
		// Check if there can be an Euler path
		// If this graph is not eulerian, return the empty path
		if (eulerianity == Eulerianity::notEulerian)
			return Path;
		else if (eulerianity == Eulerianity::semiEulerian)
		{
			for (int idx{ 0 }; idx < Nodes.size(); ++idx)
			{
				if (graphCopy.FindConnectionsFrom(Nodes[idx]->GetId()).size() % 2 == 1)
				{
					currentNodeId = Nodes[idx]->GetId();
					break;
				}
			}
		}
		else
		{
			currentNodeId = Nodes[0]->GetId();
		}
		
		// Start algorithm loop
		std::stack<int> nodeStack;
		std::vector<Connection*> Connections{ graphCopy.FindConnectionsFrom(currentNodeId) };

		do
		{
			// if node has neighbors
			if (Connections.size() > 0)
			{
				nodeStack.push(currentNodeId);
				int neighborId = Connections[0]->GetToId();
				graphCopy.RemoveConnection(currentNodeId, neighborId);
				currentNodeId = neighborId;
				Connections = graphCopy.FindConnectionsFrom(currentNodeId);
			}
			// if no more neighbors
			else
			{
				nodeStack.push(currentNodeId);
				while (nodeStack.size() > 0 && Connections.size() == 0)
				{
					Path.push_back(m_pGraph->GetNode(currentNodeId).get());
					nodeStack.pop();
					
					if (nodeStack.size() > 0)
					{
						currentNodeId = nodeStack.top();
						Connections = graphCopy.FindConnectionsFrom(currentNodeId);
					}
				}
			}
		}
		while (nodeStack.size() > 0 || Connections.size() > 0);		// until stack is empty and currentNode has no neighbors
		
		// do
		// {
		// 	if (currentNodeId == -1)
		// 		break;
		// 	
		// 	// get connections of current node
		// 	Connections.clear();
		// 	Connections = graphCopy.FindConnectionsFrom(currentNodeId);
		// 	
		// 	// if there are connections, add cur to stack, remove first connection and put neighbor as cur
		// 	if (Connections.size() > 0)
		// 	{
		// 		nodeStack.push(currentNodeId);
		// 		int NeighborNodeId{ Connections[0]->GetToId() };
		// 		graphCopy.RemoveConnection(currentNodeId, NeighborNodeId);
		// 		currentNodeId = NeighborNodeId;
		// 	}
		// 	// if no connections, pop all nodes without neighbors from the stack and add them to the path
		// 	else
		// 	{
		// 		nodeStack.push(currentNodeId);
		// 		while (nodeStack.size() > 1 && graphCopy.FindConnectionsFrom(nodeStack.top()).size() == 0)
		// 		{
		// 			Path.push_back(m_pGraph->GetNode(nodeStack.top()).get());
		// 			nodeStack.pop();
		// 		}
		// 		if (nodeStack.size() > 0)
		// 			currentNodeId = nodeStack.top();
		// 		else
		// 		 	currentNodeId = -1;
		// 	}
		// } while (nodeStack.size() > 0 || Connections.size() > 0);
		// 	
		// if (currentNodeId != -1)
		// 	Path.push_back(m_pGraph->GetNode(nodeStack.top()).get());

		std::reverse(Path.begin(), Path.end());
		return Path;
	}

	inline void EulerianPath::VisitAllNodesDFS(const std::vector<Node*>& Nodes, std::vector<bool>& visited, int startIndex ) const
	{
		// Mark the visited node
		// Ask the graph for the connections from that node
		// recursively visit any valid connected nodes that were not visited before
		// Tip: use an index-based for-loop to find the correct index
		
		visited[startIndex] = true;
		
		std::vector<Connection*> Connections{ m_pGraph->FindConnectionsFrom(Nodes[startIndex]->GetId()) };
		for (int idx{ 0 }; idx < Connections.size(); ++idx)
		{
			int ConnectedNodeId{ Connections[idx]->GetToId() };
			
			// grab index within vector
			int ConnectedNodeIdx{};
			for (int vecIdx{ 0 }; vecIdx < Nodes.size(); ++vecIdx)
			{
				if (Nodes[vecIdx]->GetId() == ConnectedNodeId)
				{
					ConnectedNodeIdx = vecIdx;
					break;
				}
			}
			
			// if the connected node is not visited yet, recursively visit
			if (visited[ConnectedNodeIdx] == false)
				VisitAllNodesDFS(Nodes, visited, ConnectedNodeId);
		}
	}

	inline bool EulerianPath::IsConnected() const
	{
		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		if (Nodes.size() == 0)
			return false;

		// choose a starting node
		// start a depth-first-search traversal from the node that has at least one connection
		// if a node was never visited, this graph is not connected
		
		std::vector<bool> IsVisited{};
		for (size_t idx{ 0 }; idx < Nodes.size(); ++idx)
		{
			IsVisited.push_back(false);
		}
		
		for (Node* Node : Nodes)	
		{
			// call VisitAllNodesDFS on the first Node with at least 1 connection
			if (m_pGraph->FindConnectionsFrom(Node->GetId()).size() > 0)
			{
				VisitAllNodesDFS(Nodes, IsVisited, 0);
				break;
			}
			// if any Node has no connections, you don't even need to call VisitAllNodes
			else
			{
				return false;
			}
		}
		
		// if any Node was not visited, return false
		for (bool Visited : IsVisited)
		{
			if (Visited == false)
				return false;
		}
		return true;
	}
}