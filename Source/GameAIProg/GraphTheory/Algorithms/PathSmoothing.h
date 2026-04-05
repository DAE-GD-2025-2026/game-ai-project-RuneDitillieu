#pragma once
#include <vector>

#include "NavGraphPathfinding.h"
#include "VectorTypes.h"
#include "Movement/Pathfinding/Navmesh/TriPolygon.h"
#include "Shared/Graph/Graph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

namespace GameAI
{
	class SSFA final
{
public:
	//=== SSFA Functions ===
	//--- References ---
	//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
	//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
	static std::vector<NavLine> FindPortals(std::vector<Node*> const & Path, TriPolygon const & NavPoly)
	{
		// Container
		std::vector<NavLine> Portals = {};
		
		// add start position
		Portals.emplace_back(Path[0]->GetPosition(), Path[0]->GetPosition());
		
		// For each node received, get it's corresponding line
		for (int idx{ 1 }; idx < Path.size() - 1; ++idx)
		{
			auto line = std::find_if(NavPoly.GetEdges().begin(), NavPoly.GetEdges().end(), 
				[Path, idx, NavPoly](const TriPolygon::Edge edge)
				{
					if (NavPoly.FindEdgeIndex(edge).has_value())
					{
						int edgeIdx{ NavPoly.FindEdgeIndex(edge).value() };
						int portalIdx{ dynamic_cast<NavGraphNode*>(Path[idx])->GetEdgeIdx() };
						return edgeIdx == portalIdx;
					}
					else
					{
						return false;
					}
				});
			
			// determine it's "orientation" based on the required path (left-right vs right-left) - p1 should be right point
			FVector vertex1{ line->GetP1(NavPoly) };
			FVector vertex2{ line->GetP2(NavPoly) };
			FVector p1{}, p2{};
			
			FVector prevNodePos{ Path[idx - 1]->GetPosition().X, Path[idx - 1]->GetPosition().Y, 0 };
			FVector cross{ FVector::CrossProduct(vertex1 - prevNodePos, vertex2 - prevNodePos) };
			
			if (cross.Z < 0) // cw
			{
				p1 = vertex1;
				p2 = vertex2;
			}
			else if (cross.Z > 0) // ccw
			{
				p1 = vertex2;
				p2 = vertex1;
			}
			
			// Store portal
			Portals.emplace_back(FVector2D(p1.X, p1.Y), FVector2D(p2.X, p2.Y));
		}
		
		// add end position
		Portals.emplace_back(Path.back()->GetPosition(), Path.back()->GetPosition());
				
		return Portals;
	}

	static std::vector<FVector2D> OptimizePortals( std::vector<NavLine> const & Portals, TriPolygon const & NavPoly)
	{
		std::vector<FVector2D> Path{};
		
		// init apex position and add it to the path
		FVector2D apexPoint{ Portals[0].P1 + (Portals[0].P2 - Portals[0].P1) / 2.f };
		Path.emplace_back(apexPoint);
		
		// init legs from apex position to first portal
		FVector2D rightLeg{ Portals[1].P1 - apexPoint };
		FVector2D leftLeg{ Portals[1].P2 - apexPoint };
		int rightLegIdx{ 1 };
		int leftLegIdx{ 1 };
		
		
		//P1 == right point of portal, P2 == left point of portal
		for (int idx{ 1 }; idx < Portals.size(); ++idx)
		{
			NavLine currentPortal = Portals[idx];
			
			//--- RIGHT CHECK ---
			//1. See if moving funnel inwards - RIGHT
			FVector2D newRightLeg{ currentPortal.P1 - apexPoint };
			FVector crossRR{ FVector::CrossProduct(FVector(rightLeg.X, rightLeg.Y, 0),  
				FVector(newRightLeg.X, newRightLeg.Y, 0)) };
			
			if (crossRR.Z <= 0) // inwards
			{
				//2. See if new line degenerates a line segment - RIGHT
				FVector crossRL{ FVector::CrossProduct(FVector(leftLeg.X, leftLeg.Y, 0),  
					FVector(newRightLeg.X, newRightLeg.Y, 0)) };
				if (crossRL.Z <= 0) // cross over left leg
				{
					// Leftleg becomes new apex point
					apexPoint += leftLeg;
					leftLegIdx = idx;
					rightLegIdx = idx;
					Path.emplace_back(apexPoint);
					
					// Calculate new legs (if not the end)
					if (idx + 1 < Portals.size())
					{
						rightLeg = Portals[rightLegIdx].P1 - apexPoint;
						leftLeg = Portals[leftLegIdx].P2 - apexPoint;
						continue;
					}
				}
				else
				{
					rightLeg = newRightLeg;
					rightLegIdx = idx;
				}
			}

			//--- LEFT CHECK ---
			//1. See if moving funnel inwards - LEFT
			FVector2D newLeftLeg{ currentPortal.P2 - apexPoint };
			FVector crossLL{ FVector::CrossProduct(FVector(leftLeg.X, leftLeg.Y, 0),  
				FVector(newLeftLeg.X, newLeftLeg.Y, 0)) };
			
			if (crossLL.Z >= 0) // inwards
			{
				//2. See if new line degenerates a line segment - LEFT
				FVector crossLR{ FVector::CrossProduct(FVector(rightLeg.X, rightLeg.Y, 0),  
					FVector(newLeftLeg.X, newLeftLeg.Y, 0)) };
				if (crossLR.Z >= 0) // cross over right leg
				{
					// Rightleg becomes new apex point
					apexPoint += rightLeg;
					rightLegIdx = idx;
					leftLegIdx = idx;
					Path.emplace_back(apexPoint);
					
					// Calculate new legs (if not the end)
					if (idx + 1 < Portals.size())
					{
						rightLeg = Portals[rightLegIdx].P1 - apexPoint;
						leftLeg = Portals[leftLegIdx].P2 - apexPoint;
						continue;
					}
				}
				else
				{
					leftLeg = newLeftLeg;
					leftLegIdx = idx;
				}
			}
		}
		
		// Add last path point
		Path.emplace_back(Portals.back().P1);
		return Path;
	}
private:
	SSFA() {};
	~SSFA() {};
};
}
