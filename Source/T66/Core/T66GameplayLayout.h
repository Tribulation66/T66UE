#pragma once

#include "CoreMinimal.h"

namespace T66GameplayLayout
{
	static constexpr float WorldHalfExtent = 50000.f;
	static constexpr float AreaHalfHeightY = 4545.f;
	static constexpr float CorridorHalfHeightY = 1000.f;
	static constexpr float GateExitClearanceDepthX = 5400.f;
	static constexpr float GateExitClearanceHalfHeightY = 2600.f;

	static constexpr float StartPartitionWestX = -40000.f;
	static constexpr float StartAreaEastX = -30909.f;
	static constexpr float StartMainEntranceX = -27273.f;

	static constexpr float BossMainEntranceX = 27273.f;
	static constexpr float BossAreaWestX = 30909.f;
	static constexpr float BossPartitionEastX = 40000.f;
	static constexpr float MinGameplayGroundNormalZ = 0.86f;

	inline FVector GetStartAreaCenter(float Z = 0.f)
	{
		return FVector((StartPartitionWestX + StartAreaEastX) * 0.5f, 0.f, Z);
	}

	inline FVector GetBossAreaCenter(float Z = 0.f)
	{
		return FVector((BossAreaWestX + BossPartitionEastX) * 0.5f, 0.f, Z);
	}

	inline FVector GetStartGateLocation(float Z = 0.f)
	{
		return FVector(StartMainEntranceX, 0.f, Z);
	}

	inline FVector GetBossGateLocation(float Z = 0.f)
	{
		return FVector(BossAreaWestX, 0.f, Z);
	}

	inline FVector GetCowardiceGateLocation(float Z = 0.f)
	{
		return FVector(BossAreaWestX - 900.f, -260.f, Z);
	}

	inline FVector GetTricksterLocation(float Z = 0.f)
	{
		return FVector(BossAreaWestX - 1250.f, 420.f, Z);
	}

	inline bool IsInsideStartArea2D(const FVector& Location, float Margin = 0.f)
	{
		return Location.X >= (StartPartitionWestX - Margin)
			&& Location.X <= (StartAreaEastX + Margin)
			&& FMath::Abs(Location.Y) <= (AreaHalfHeightY + Margin);
	}

	inline bool IsInsideBossArea2D(const FVector& Location, float Margin = 0.f)
	{
		return Location.X >= (BossAreaWestX - Margin)
			&& Location.X <= (BossPartitionEastX + Margin)
			&& FMath::Abs(Location.Y) <= (AreaHalfHeightY + Margin);
	}

	inline bool IsInsideStartCorridor2D(const FVector& Location, float Margin = 0.f)
	{
		return Location.X >= (StartAreaEastX - Margin)
			&& Location.X <= (StartMainEntranceX + Margin)
			&& FMath::Abs(Location.Y) <= (CorridorHalfHeightY + Margin);
	}

	inline bool IsInsideBossCorridor2D(const FVector& Location, float Margin = 0.f)
	{
		return Location.X >= (BossMainEntranceX - Margin)
			&& Location.X <= (BossAreaWestX + Margin)
			&& FMath::Abs(Location.Y) <= (CorridorHalfHeightY + Margin);
	}

	inline bool IsInsideReservedTraversalZone2D(const FVector& Location, float Margin = 0.f)
	{
		return IsInsideStartArea2D(Location, Margin)
			|| IsInsideBossArea2D(Location, Margin)
			|| IsInsideStartCorridor2D(Location, Margin)
			|| IsInsideBossCorridor2D(Location, Margin);
	}

	inline bool IsInsideStartGateExitClearance2D(const FVector& Location, float Margin = 0.f)
	{
		return Location.X >= (StartMainEntranceX - Margin)
			&& Location.X <= (StartMainEntranceX + GateExitClearanceDepthX + Margin)
			&& FMath::Abs(Location.Y) <= (GateExitClearanceHalfHeightY + Margin);
	}

	inline bool IsInsideBossGateExitClearance2D(const FVector& Location, float Margin = 0.f)
	{
		return Location.X >= (BossMainEntranceX - GateExitClearanceDepthX - Margin)
			&& Location.X <= (BossMainEntranceX + Margin)
			&& FMath::Abs(Location.Y) <= (GateExitClearanceHalfHeightY + Margin);
	}

	inline bool IsInsideTraversalKeepClearZone2D(const FVector& Location, float Margin = 0.f)
	{
		return IsInsideReservedTraversalZone2D(Location, Margin)
			|| IsInsideStartGateExitClearance2D(Location, Margin)
			|| IsInsideBossGateExitClearance2D(Location, Margin);
	}

	inline bool IsValidGameplayGroundNormal(const FVector& SurfaceNormal, float MinNormalZ = MinGameplayGroundNormalZ)
	{
		return SurfaceNormal.GetSafeNormal().Z >= MinNormalZ;
	}

	inline bool DoAxisRangesOverlap(float MinA, float MaxA, float MinB, float MaxB)
	{
		return MaxA >= MinB && MaxB >= MinA;
	}

	inline bool DoesRectOverlapReservedTraversalZone2D(float MinX, float MaxX, float MinY, float MaxY, float Margin = 0.f)
	{
		auto OverlapsRect = [=](float ZoneMinX, float ZoneMaxX, float ZoneMinY, float ZoneMaxY) -> bool
		{
			return DoAxisRangesOverlap(MinX, MaxX, ZoneMinX - Margin, ZoneMaxX + Margin)
				&& DoAxisRangesOverlap(MinY, MaxY, ZoneMinY - Margin, ZoneMaxY + Margin);
		};

		return OverlapsRect(StartPartitionWestX, StartAreaEastX, -AreaHalfHeightY, AreaHalfHeightY)
			|| OverlapsRect(StartAreaEastX, StartMainEntranceX, -CorridorHalfHeightY, CorridorHalfHeightY)
			|| OverlapsRect(BossMainEntranceX, BossAreaWestX, -CorridorHalfHeightY, CorridorHalfHeightY)
			|| OverlapsRect(BossAreaWestX, BossPartitionEastX, -AreaHalfHeightY, AreaHalfHeightY);
	}

	inline bool DoesRectOverlapTraversalKeepClearZone2D(float MinX, float MaxX, float MinY, float MaxY, float Margin = 0.f)
	{
		auto OverlapsRect = [=](float ZoneMinX, float ZoneMaxX, float ZoneMinY, float ZoneMaxY) -> bool
		{
			return DoAxisRangesOverlap(MinX, MaxX, ZoneMinX - Margin, ZoneMaxX + Margin)
				&& DoAxisRangesOverlap(MinY, MaxY, ZoneMinY - Margin, ZoneMaxY + Margin);
		};

		return DoesRectOverlapReservedTraversalZone2D(MinX, MaxX, MinY, MaxY, Margin)
			|| OverlapsRect(StartMainEntranceX, StartMainEntranceX + GateExitClearanceDepthX, -GateExitClearanceHalfHeightY, GateExitClearanceHalfHeightY)
			|| OverlapsRect(BossMainEntranceX - GateExitClearanceDepthX, BossMainEntranceX, -GateExitClearanceHalfHeightY, GateExitClearanceHalfHeightY);
	}
}
