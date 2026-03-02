#pragma once

namespace Debug
{
	inline void DrawTopDownDebugCircle(UWorld* pWorld, const FVector& Center, float Radius, FColor&& Color)
	{
		DrawDebugCircle(pWorld, Center, Radius, 16, Color, 
			false, -1, 0, 0,
			FVector(0, 1, 0), FVector(1, 0, 0), false);
	}
	
	inline void DrawDebugRect(UWorld* pWorld, const FVector2D& TopLeft, const FVector2D& TopRight, 
		const FVector2D& BottomLeft, const FVector2D& BottomRight, const FColor& Color)
	{
		DrawDebugLine(pWorld, FVector(BottomLeft.X, BottomLeft.Y, 1.f), 
			FVector(BottomRight.X, BottomRight.Y, 1.f), Color);
		DrawDebugLine(pWorld, FVector(TopLeft.X, TopLeft.Y, 1.f), 
			FVector(TopRight.X, TopRight.Y, 1.f), Color);
		DrawDebugLine(pWorld, FVector(BottomLeft.X, BottomLeft.Y, 1.f), 
			FVector(TopLeft.X, TopLeft.Y, 1.f), Color);
		DrawDebugLine(pWorld, FVector(TopRight.X, TopRight.Y, 1.f), 
			FVector(BottomRight.X, BottomRight.Y, 1.f), Color);
	}
	
	inline void DrawDebugRect(UWorld* pWorld, const FVector2D& Center, float HalfWidth, float HalfHeight, const FColor& Color)
	{
		FVector2D TopLeft(Center.X - HalfWidth, Center.Y - HalfHeight);
		FVector2D TopRight(Center.X + HalfWidth, Center.Y - HalfHeight);
		FVector2D BottomLeft(Center.X - HalfWidth, Center.Y + HalfHeight);
		FVector2D BottomRight(Center.X + HalfWidth, Center.Y + HalfHeight);
		
		DrawDebugRect(pWorld, TopLeft, TopRight, BottomLeft, BottomRight, Color);
	}
}
