#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	Cells.reserve(Rows * Cols);
	for (int Row{ 0 }; Row < Rows; ++Row)
	{
		for (int Col{ 0 }; Col < Cols; ++Col)
		{
			Cells.emplace_back(-(Width / 2) + (Col * CellWidth), -(Height / 2) + (Row * CellHeight), 
				CellWidth, CellHeight);
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	Cells[PositionToIndex(Agent.GetPosition())].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	const int OldIndex{ PositionToIndex(OldPos) };
	const int CurIndex{ PositionToIndex(Agent.GetPosition()) };
	
	if (OldIndex != CurIndex)
	{
		Cells[OldIndex].Agents.remove(&Agent);
		Cells[CurIndex].Agents.push_back(&Agent);
	}
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	// TODO Register the neighbors for the provided agent
	// TODO Only check the cells that are within the radius of the neighborhood
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	for (Cell Cell : Cells)
	{
		FVector2D LeftBottom{ Cell.GetRectPoints()[0] };
		FVector2D LeftTop{ Cell.GetRectPoints()[1] };
		FVector2D RightTop{ Cell.GetRectPoints()[2] };
		FVector2D RightBottom{ Cell.GetRectPoints()[3] };
		
		DrawDebugLine(pWorld, FVector(LeftBottom.X, LeftBottom.Y, 1.f), FVector(RightBottom.X, RightBottom.Y, 1.f),
			FColor(0, 0, 255, 100));
		DrawDebugLine(pWorld, FVector(LeftTop.X, LeftTop.Y, 1.f), FVector(RightTop.X, RightTop.Y, 1.f),
			FColor(0, 0, 255, 100));
		DrawDebugLine(pWorld, FVector(LeftBottom.X, LeftBottom.Y, 1.f), FVector(LeftTop.X, LeftTop.Y, 1.f),
			FColor(0, 0, 255, 100));
		DrawDebugLine(pWorld, FVector(RightTop.X, RightTop.Y, 1.f), FVector(RightBottom.X, RightBottom.Y, 1.f),
			FColor(0, 0, 255, 100));
	}
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	const int Col{ static_cast<int>(Pos.X / CellWidth) };
	const int Row{ static_cast<int>(Pos.Y / CellHeight) };
	return int((Row * NrOfCols) + Col);
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}