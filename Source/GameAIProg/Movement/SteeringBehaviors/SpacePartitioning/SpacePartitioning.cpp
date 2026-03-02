#include "SpacePartitioning.h"
#include "Shared/DebugHelpers.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float Left = BoundingBox.Min.X;
	const float Bottom = BoundingBox.Min.Y;
	const float Width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float Height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ Left , Bottom  },
		{ Left , Bottom + Height  },
		{ Left + Width , Bottom + Height },
		{ Left + Width , Bottom  },
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
	const int CellIndex{ PositionToIndex(Agent.GetPosition()) };
	Cells[CellIndex].Agents.push_back(&Agent);
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
	// create boundingbox rect
	const FRect NeighborhoodRect{ FVector2D(Agent.GetPosition().X - QueryRadius, Agent.GetPosition().Y - QueryRadius),
	FVector2D(Agent.GetPosition().X + QueryRadius, Agent.GetPosition().Y + QueryRadius) };
	
	NrOfNeighbors = 0;
	
	for (Cell Cell : Cells)
	{
		if (DoRectsOverlap(Cell.BoundingBox,NeighborhoodRect))
		{
			// check for neighbors only in cells within boundingbox
			for (ASteeringAgent* const PossibleNeighbor : Cell.Agents)
			{
				if (PossibleNeighbor != &Agent)
				{
					const FVector2D VecToAgent = PossibleNeighbor->GetPosition() - Agent.GetPosition();
					if (VecToAgent.Length() <= QueryRadius)
					{
						Neighbors[NrOfNeighbors] = PossibleNeighbor;
						++NrOfNeighbors;
					}
				}
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	// draw cells
	for (Cell Cell : Cells)
	{
		FVector2D BottomLeft{ Cell.GetRectPoints()[0] };
		FVector2D TopLeft{ Cell.GetRectPoints()[1] };
		FVector2D TopRight{ Cell.GetRectPoints()[2] };
		FVector2D BottomRight{ Cell.GetRectPoints()[3] };
		
		Debug::DrawDebugRect(pWorld, TopLeft, TopRight, BottomLeft, BottomRight, FColor(100, 150, 255, 255));
	}
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	const int Col{ static_cast<int>((Pos.X + SpaceWidth / 2) / CellWidth) };
	const int Row{ static_cast<int>((Pos.Y + SpaceHeight / 2) / CellHeight) };
	return Row * NrOfCols + Col;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}