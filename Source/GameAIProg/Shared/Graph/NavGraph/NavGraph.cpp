#include "NavGraph.h"
#include "NavGraphNode.h"

GameAI::NavGraph::NavGraph(std::unique_ptr<TriPolygon> && NavPoly)
	: Graph{false}
	, pNavPoly{std::move(NavPoly)}
{
	CreateNavigationGraph();
}

GameAI::NavGraph::NavGraph(const NavGraph& Other)
	: Graph(false)
{
	Nodes.reserve(Other.Nodes.size());
	for (std::unique_ptr<Node> const & OtherNode : Other.Nodes)
	{
		Nodes.push_back(std::make_unique<NavGraphNode>(*dynamic_cast<NavGraphNode*>(OtherNode.get())));
	}
        
	Connections.reserve(Other.Connections.size());
	for (std::unique_ptr<Connection> const & OtherConnection : Other.Connections)
	{
		Connections.push_back(std::make_unique<Connection>(*OtherConnection.get()));
	}
}

std::unique_ptr<GameAI::NavGraph> GameAI::NavGraph::Clone() const
{
	return std::make_unique<NavGraph>(*this);
}

int GameAI::NavGraph::GetNodeIdFromEdgeIndex(int EdgeIdx) const
{
	if (EdgeIdx >= 0)
	{
		for (auto const & pNode : Nodes)
		{
			if (reinterpret_cast<NavGraphNode*>(pNode.get())->GetEdgeIdx() == EdgeIdx)
			{
				return pNode->GetId();
			}
		}
	}
	
	return Graphs::InvalidNodeId;
}

void GameAI::NavGraph::CreateNavigationGraph()
{
	// for every edge
	for (auto& line : pNavPoly->GetEdges())
	{
		int connectedTriangles{ 0 };
		for (auto& triangle : pNavPoly->GetTriangles())
		{
			if (triangle.HasEdge(line))
			{
				++connectedTriangles;
				if (connectedTriangles >= 2)
					break;
			}
		}
		
		// if at least 2 triangles share this edge, add a node in the middle of this edge
		if (connectedTriangles >= 2)
		{
			FVector middle{ line.GetP1(*pNavPoly.get()) + ((line.GetP2(*pNavPoly.get()) - line.GetP1(*pNavPoly.get())) / 2.f) };
			std::optional<int> lineId{ pNavPoly->FindEdgeIndex(line) };
			AddNode(std::make_unique<NavGraphNode>(FVector2D(middle.X, middle.Y), lineId.value()));
		}
	}
	
	// for every triangle
	for (auto& triangle : pNavPoly->GetTriangles())
	{
		std::vector<int> tempNodeIds{};
		for (auto& line : triangle.GetEdges())
		{
			int nodeId{ GetNodeIdFromEdgeIndex(pNavPoly->FindEdgeIndex(line).value()) };
			if (nodeId != -1)
			{
				tempNodeIds.emplace_back(nodeId);
			}
		}
		
		// connect all nodes on the triangle
		if (tempNodeIds.size() > 1)
		{
			AddConnection(tempNodeIds[0], tempNodeIds[1]);
		}
		if (tempNodeIds.size() == 3)
		{
			AddConnection(tempNodeIds[1], tempNodeIds[2]);
			AddConnection(tempNodeIds[2], tempNodeIds[0]);
		}
	}
	
	// set the weight to the distance
	SetConnectionCostsToDistances();
}
