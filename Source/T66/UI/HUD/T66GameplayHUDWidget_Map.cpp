// Copyright Tribulation 66. All Rights Reserved.

#include "UI/HUD/T66GameplayHUDWidget_Private.h"

void UT66GameplayHUDWidget::SetFullMapOpen(bool bOpen)
{
	bFullMapOpen = bOpen;
	if (FullMapOverlayBorder.IsValid())
	{
		FullMapOverlayBorder->SetVisibility(bFullMapOpen ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (bFullMapOpen)
	{
		RefreshMapData();
	}
}


void UT66GameplayHUDWidget::ToggleFullMap()
{
	SetFullMapOpen(!bFullMapOpen);
}


void UT66GameplayHUDWidget::UpdateTowerMapReveal(const FVector& PlayerLocation)
{
	UWorld* World = GetWorld();
	AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	if (!GameMode || !GameMode->IsUsingTowerMainMapLayout())
	{
		return;
	}

	const int32 FloorNumber = GameMode->GetTowerFloorIndexForLocation(PlayerLocation);
	if (FloorNumber == INDEX_NONE)
	{
		return;
	}

	static constexpr float TowerRevealPointSpacing = 900.0f;
	TArray<FVector2D>& RevealPoints = TowerRevealPointsByFloor.FindOrAdd(FloorNumber);
	const FVector2D PlayerXY(PlayerLocation.X, PlayerLocation.Y);
	if (RevealPoints.Num() <= 0 || FVector2D::DistSquared(RevealPoints.Last(), PlayerXY) >= FMath::Square(TowerRevealPointSpacing))
	{
		RevealPoints.Add(PlayerXY);
	}
}


bool UT66GameplayHUDWidget::IsTowerMapRevealPointVisible(int32 FloorNumber, const FVector2D& WorldXY) const
{
	static constexpr float TowerRevealRadius = 2600.0f;
	const TArray<FVector2D>* RevealPoints = TowerRevealPointsByFloor.Find(FloorNumber);
	if (!RevealPoints || RevealPoints->Num() <= 0)
	{
		return false;
	}

	const float RevealRadiusSq = FMath::Square(TowerRevealRadius);
	for (const FVector2D& RevealPoint : *RevealPoints)
	{
		if (FVector2D::DistSquared(RevealPoint, WorldXY) <= RevealRadiusSq)
		{
			return true;
		}
	}

	return false;
}


void UT66GameplayHUDWidget::RefreshMapData()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("GameplayHUD::RefreshMapData"));
	static constexpr float MinimapEnemyMarkerRadius = 2400.f;
	static constexpr float MinimapEnemyMarkerRadiusSq = MinimapEnemyMarkerRadius * MinimapEnemyMarkerRadius;
	static constexpr int32 MaxMinimapEnemyMarkers = 48;

	if (!MinimapWidget.IsValid() && !FullMapWidget.IsValid())
	{
		return;
	}

	// If neither minimap nor full map is visible, skip all work.
	const bool bMinimapVisible = MinimapPanelBox.IsValid() && (MinimapPanelBox->GetVisibility() != EVisibility::Collapsed);
	if (!bMinimapVisible && !bFullMapOpen)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Try multiple paths to find the player pawn — GetOwningPlayerPawn can return null
	// if the pawn hasn't been possessed yet when the HUD widget was first created.
	const APawn* P = GetOwningPlayerPawn();
	if (!P)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			P = PC->GetPawn();
		}
	}
	if (!P)
	{
		P = UGameplayStatics::GetPlayerPawn(World, 0);
	}
	const FVector PL = P ? P->GetActorLocation() : FVector::ZeroVector;
	const FVector2D PlayerXY(PL.X, PL.Y);
	UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
	AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode());
	T66TowerMapTerrain::FLayout TowerLayout;
	const bool bTowerLayout = GameMode && GameMode->GetTowerMainMapLayout(TowerLayout);
	const T66TowerMapTerrain::FFloor* ActiveTowerFloor = nullptr;
	int32 ActiveTowerFloorNumber = INDEX_NONE;
	FVector2D ActiveTowerFloorCenter = FVector2D::ZeroVector;
	FVector2D ActiveTowerFloorHalfExtents = FVector2D::ZeroVector;
	TArray<FVector2D> ActiveTowerPolygon;
	TArray<FBox2D> ActiveTowerMazeWallBoxes;
	FVector2D ActiveTowerHoleCenter = FVector2D::ZeroVector;
	FVector2D ActiveTowerHoleHalfExtents = FVector2D::ZeroVector;
	bool bHasActiveTowerHole = false;
	static const TArray<FVector2D> EmptyRevealPoints;
	static const TArray<FVector2D> EmptyTowerPolygon;
	static const TArray<FBox2D> EmptyTowerMazeWallBoxes;
	static constexpr float TowerMapRevealRadius = 2600.0f;
	const TArray<FVector2D>* ActiveTowerRevealPoints = nullptr;
	const FSlateBrush* ActiveTowerBackgroundBrush = nullptr;
	FLinearColor ActiveTowerMapTint = FLinearColor::White;
	FLinearColor ActiveTowerWallFillColor = FLinearColor(0.14f, 0.11f, 0.09f, 1.0f);
	FLinearColor ActiveTowerWallStrokeColor = FLinearColor(0.92f, 0.86f, 0.72f, 1.0f);

	FVector2D PlayerDirectionWorldXY(1.0f, 0.0f);
	if (P)
	{
		const FVector PlayerVelocity = P->GetVelocity();
		const FVector2D PlayerVelocityXY(PlayerVelocity.X, PlayerVelocity.Y);
		if (PlayerVelocityXY.SizeSquared() > 25.0f)
		{
			PlayerDirectionWorldXY = PlayerVelocityXY.GetSafeNormal();
		}
		else
		{
			const FVector Forward = P->GetActorForwardVector();
			const FVector2D ForwardXY(Forward.X, Forward.Y);
			if (!ForwardXY.IsNearlyZero())
			{
				PlayerDirectionWorldXY = ForwardXY.GetSafeNormal();
			}
		}
	}

	if (bTowerLayout)
	{
		ActiveTowerFloorNumber = GameMode->GetTowerFloorIndexForLocation(PL);
		if (ActiveTowerFloorNumber == INDEX_NONE)
		{
			ActiveTowerFloorNumber = GameMode->GetCurrentTowerFloorIndex();
		}

		if (ActiveTowerFloorNumber != INDEX_NONE)
		{
			if (LastTowerRevealFloorNumber != INDEX_NONE && ActiveTowerFloorNumber > LastTowerRevealFloorNumber)
			{
				TowerRevealPointsByFloor.FindOrAdd(ActiveTowerFloorNumber).Reset();
			}

			LastTowerRevealFloorNumber = ActiveTowerFloorNumber;
		}

		if (P)
		{
			UpdateTowerMapReveal(PL);
		}

		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.FloorNumber != ActiveTowerFloorNumber)
			{
				continue;
			}

			ActiveTowerFloor = &Floor;
			ActiveTowerFloorCenter = FVector2D(Floor.Center.X, Floor.Center.Y);
			ActiveTowerFloorHalfExtents = FVector2D(Floor.BoundsHalfExtent, Floor.BoundsHalfExtent);
			T66TowerMapTerrain::TryGetFloorPolygon(TowerLayout, Floor.FloorNumber, ActiveTowerPolygon);
			ActiveTowerMazeWallBoxes = Floor.MazeWallBoxes;
			bHasActiveTowerHole = Floor.bHasDropHole;
			ActiveTowerHoleCenter = FVector2D(Floor.HoleCenter.X, Floor.HoleCenter.Y);
			ActiveTowerHoleHalfExtents = Floor.HoleHalfExtent;
			const FT66TowerMinimapArtStyle TowerArtStyle = GetTowerMinimapArtStyle(Floor.Theme);
			ActiveTowerBackgroundBrush = GetTowerMinimapBackgroundBrush(Floor.Theme);
			ActiveTowerMapTint = TowerArtStyle.MapTint;
			ActiveTowerWallFillColor = TowerArtStyle.WallFill;
			ActiveTowerWallStrokeColor = TowerArtStyle.WallStroke;
			break;
		}

		ActiveTowerRevealPoints = TowerRevealPointsByFloor.Find(ActiveTowerFloorNumber);
	}
	else
	{
		LastTowerRevealFloorNumber = INDEX_NONE;
	}

	if (MinimapWidget.IsValid())
	{
		MinimapWidget->SetPlayerDirectionWorldXY(PlayerDirectionWorldXY);
		if (bTowerLayout && ActiveTowerFloor)
		{
			MinimapWidget->SetFullWorldBounds(
				ActiveTowerFloorCenter - ActiveTowerFloorHalfExtents,
				ActiveTowerFloorCenter + ActiveTowerFloorHalfExtents);
			MinimapWidget->SetMinimapHalfExtent(ActiveTowerFloor->BoundsHalfExtent);
			MinimapWidget->SetRevealMask(true, ActiveTowerRevealPoints ? *ActiveTowerRevealPoints : EmptyRevealPoints, TowerMapRevealRadius);
			MinimapWidget->SetTowerPolygon(ActiveTowerPolygon);
			MinimapWidget->SetTowerHole(bHasActiveTowerHole, ActiveTowerHoleCenter, ActiveTowerHoleHalfExtents);
			MinimapWidget->SetThemedFloorArt(
				ActiveTowerBackgroundBrush,
				ActiveTowerMazeWallBoxes,
				ActiveTowerMapTint,
				ActiveTowerWallFillColor,
				ActiveTowerWallStrokeColor);
			MinimapWidget->SetLockFullMapToBounds(true);
		}
		else
		{
			MinimapWidget->SetMinimapHalfExtent(2500.0f);
			MinimapWidget->SetRevealMask(false, EmptyRevealPoints, 0.0f);
			MinimapWidget->SetTowerPolygon(EmptyTowerPolygon);
			MinimapWidget->SetTowerHole(false);
			MinimapWidget->SetThemedFloorArt(nullptr, EmptyTowerMazeWallBoxes, FLinearColor::White, FLinearColor::White, FLinearColor::White);
			MinimapWidget->SetLockFullMapToBounds(false);
		}
	}

	if (FullMapWidget.IsValid())
	{
		FullMapWidget->SetPlayerDirectionWorldXY(PlayerDirectionWorldXY);
		if (bTowerLayout && ActiveTowerFloor)
		{
			FullMapWidget->SetFullWorldBounds(
				ActiveTowerFloorCenter - ActiveTowerFloorHalfExtents,
				ActiveTowerFloorCenter + ActiveTowerFloorHalfExtents);
			FullMapWidget->SetLockFullMapToBounds(true);
			FullMapWidget->SetRevealMask(true, ActiveTowerRevealPoints ? *ActiveTowerRevealPoints : EmptyRevealPoints, TowerMapRevealRadius);
			FullMapWidget->SetTowerPolygon(ActiveTowerPolygon);
			FullMapWidget->SetTowerHole(bHasActiveTowerHole, ActiveTowerHoleCenter, ActiveTowerHoleHalfExtents);
			FullMapWidget->SetThemedFloorArt(
				ActiveTowerBackgroundBrush,
				ActiveTowerMazeWallBoxes,
				ActiveTowerMapTint,
				ActiveTowerWallFillColor,
				ActiveTowerWallStrokeColor);
		}
		else
		{
			FullMapWidget->SetFullWorldBounds(FVector2D(-50000.0f, -50000.0f), FVector2D(50000.0f, 50000.0f));
			FullMapWidget->SetLockFullMapToBounds(false);
			FullMapWidget->SetRevealMask(false, EmptyRevealPoints, 0.0f);
			FullMapWidget->SetTowerPolygon(EmptyTowerPolygon);
			FullMapWidget->SetTowerHole(false);
			FullMapWidget->SetThemedFloorArt(nullptr, EmptyTowerMazeWallBoxes, FLinearColor::White, FLinearColor::White, FLinearColor::White);
		}
	}

	auto ShouldShowTowerFloorLocation = [&](const FVector& Location) -> bool
	{
		if (!bTowerLayout || !GameMode || ActiveTowerFloorNumber == INDEX_NONE)
		{
			return true;
		}

		if (GameMode->GetTowerFloorIndexForLocation(Location) != ActiveTowerFloorNumber)
		{
			return false;
		}

		return IsTowerMapRevealPointVisible(ActiveTowerFloorNumber, FVector2D(Location.X, Location.Y));
	};

	const float Now = static_cast<float>(World->GetTimeSeconds());
	const bool bWorldChanged = (MapCacheWorld.Get() != World);
	const bool bNeedsFullRefresh = bWorldChanged || (MapCacheLastRefreshTime < 0.f) || ((Now - MapCacheLastRefreshTime) >= MapCacheRefreshIntervalSeconds);

	TArray<FT66MapMarker> Markers;
	const int32 EnemyReserve = bFullMapOpen
		? (Registry ? Registry->GetEnemies().Num() : 0)
		: MaxMinimapEnemyMarkers;
	Markers.Reserve(MapCache.Num() + EnemyReserve + 8);

	if (bNeedsFullRefresh)
	{
		MapCacheWorld = World;
		MapCacheLastRefreshTime = Now;
		MapCache.Reset();

		// [GOLD] Use the actor registry instead of 4x TActorIterator (O(1) list lookup).
		if (Registry)
		{
			// NPCs use icon markers on the minimap.
			for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
			{
				AT66HouseNPCBase* NPC = WeakNPC.Get();
				if (!NPC) continue;
				MapCache.Add({ NPC, EMapCacheMarkerType::NPC, NPC->NPCColor, NPC->NPCName, NPC->NPCID });
			}
			// Stage gate / interactable.
			for (const TWeakObjectPtr<AT66StageGate>& WeakGate : Registry->GetStageGates())
			{
				AT66StageGate* A = WeakGate.Get();
				if (!A) continue;
				MapCache.Add({ A, EMapCacheMarkerType::Gate, FT66Style::Accent2(), NSLOCTEXT("T66.Map", "Gate", "GATE"), FName(TEXT("Gate")) });
			}
			// Miasma boundary uses a dedicated icon marker on the full map.
			for (const TWeakObjectPtr<AT66MiasmaBoundary>& WeakMiasma : Registry->GetMiasmaBoundaries())
			{
				AT66MiasmaBoundary* A = WeakMiasma.Get();
				if (!A) continue;
				MapCache.Add({ A, EMapCacheMarkerType::Miasma, FLinearColor(0.65f, 0.15f, 0.85f, 0.78f), FText::GetEmpty(), FName(TEXT("Miasma")) });
			}

			for (const TWeakObjectPtr<AT66WorldInteractableBase>& WeakInteractable : Registry->GetWorldInteractables())
			{
				AT66WorldInteractableBase* Interactable = WeakInteractable.Get();
				if (!Interactable || Interactable->bConsumed)
				{
					continue;
				}

				if (Cast<AT66ChestInteractable>(Interactable))
				{
					MapCache.Add({ Interactable, EMapCacheMarkerType::POI, FT66Style::Accent2(), FText::GetEmpty(), FName(TEXT("Chest")) });
				}
				else if (Cast<AT66CrateInteractable>(Interactable))
				{
					MapCache.Add({ Interactable, EMapCacheMarkerType::POI, FT66Style::MinimapNeutral(), FText::GetEmpty(), FName(TEXT("Crate")) });
				}
				else if (Cast<AT66StageCatchUpLootInteractable>(Interactable))
				{
					MapCache.Add({ Interactable, EMapCacheMarkerType::POI, FT66Style::Accent2(), FText::GetEmpty(), FName(TEXT("CatchUpLoot")) });
				}
			}

			for (const TWeakObjectPtr<AT66LootBagPickup>& WeakLootBag : Registry->GetLootBags())
			{
				AT66LootBagPickup* LootBag = WeakLootBag.Get();
				if (!LootBag)
				{
					continue;
				}

				MapCache.Add({ LootBag, EMapCacheMarkerType::POI, FT66Style::Accent2(), FText::GetEmpty(), FName(TEXT("LootBag")) });
			}

			UE_LOG(LogT66HUD, Verbose, TEXT("[GOLD] RefreshMapData: used ActorRegistry (NPCs=%d, Gates=%d, Enemies=%d, Miasma=%d, Interactables=%d, LootBags=%d)"),
				Registry->GetNPCs().Num(), Registry->GetStageGates().Num(),
				Registry->GetEnemies().Num(), Registry->GetMiasmaBoundaries().Num(),
				Registry->GetWorldInteractables().Num(), Registry->GetLootBags().Num());
		}
	}

	// Build markers from cache (positions only; cache has static Color/Label).
	for (const FMapCacheEntry& E : MapCache)
	{
		AActor* A = E.Actor.Get();
		if (!IsValid(A)) continue;
		FT66MapMarker M;
		const FVector L = A->GetActorLocation();
		if (!ShouldShowTowerFloorLocation(L))
		{
			continue;
		}
		M.WorldXY = FVector2D(L.X, L.Y);
		M.Color = E.Color;
		M.Label = E.Label;

		switch (E.Type)
		{
		case EMapCacheMarkerType::NPC:
			M.Visual = ET66MapMarkerVisual::Icon;
			M.IconBrush = GetMinimapSymbolBrush(E.MarkerKey.IsNone() ? FName(TEXT("NPC")) : E.MarkerKey);
			M.DrawSize = FVector2D(12.f, 12.f);
			break;
		case EMapCacheMarkerType::Gate:
			M.Visual = ET66MapMarkerVisual::Icon;
			M.IconBrush = GetMinimapSymbolBrush(E.MarkerKey.IsNone() ? FName(TEXT("Gate")) : E.MarkerKey);
			M.DrawSize = FVector2D(12.f, 12.f);
			break;
		case EMapCacheMarkerType::Miasma:
			M.Visual = ET66MapMarkerVisual::Icon;
			M.IconBrush = GetMinimapSymbolBrush(E.MarkerKey.IsNone() ? FName(TEXT("Miasma")) : E.MarkerKey);
			M.DrawSize = FVector2D(12.f, 12.f);
			break;
		case EMapCacheMarkerType::POI:
			M.Visual = ET66MapMarkerVisual::Icon;
			M.IconBrush = GetMinimapSymbolBrush(E.MarkerKey);
			M.DrawSize = FVector2D(12.f, 12.f);
			break;
		case EMapCacheMarkerType::Enemy:
		default:
			M.Visual = ET66MapMarkerVisual::Dot;
			M.DrawSize = FVector2D(6.f, 6.f);
			break;
		}

		Markers.Add(M);
	}

	if (Registry)
	{
		int32 AddedEnemyMarkers = 0;
		for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Registry->GetEnemies())
		{
			AT66EnemyBase* Enemy = WeakEnemy.Get();
			if (!IsValid(Enemy))
			{
				continue;
			}

			const FVector EnemyLoc = Enemy->GetActorLocation();
			if (!ShouldShowTowerFloorLocation(EnemyLoc))
			{
				continue;
			}
			const FVector2D EnemyXY(EnemyLoc.X, EnemyLoc.Y);
			if (!bFullMapOpen && FVector2D::DistSquared(PlayerXY, EnemyXY) > MinimapEnemyMarkerRadiusSq)
			{
				continue;
			}

			FT66MapMarker EnemyMarker;
			EnemyMarker.WorldXY = EnemyXY;
			EnemyMarker.Color = FT66Style::MinimapEnemy();
			EnemyMarker.Label = FText::GetEmpty();
			EnemyMarker.Visual = ET66MapMarkerVisual::Dot;
			EnemyMarker.DrawSize = FVector2D(6.f, 6.f);
			Markers.Add(MoveTemp(EnemyMarker));

			++AddedEnemyMarkers;
			if (!bFullMapOpen && AddedEnemyMarkers >= MaxMinimapEnemyMarkers)
			{
				break;
			}
		}
	}

	const FSlateBrush* PlayerMarkerBrush = (PortraitBrush.IsValid() && PortraitBrush->GetResourceObject())
		? PortraitBrush.Get()
		: nullptr;

	if (MinimapWidget.IsValid())
	{
		MinimapWidget->SetSnapshot(PlayerXY, Markers, PlayerMarkerBrush);
	}
	if (bFullMapOpen && FullMapWidget.IsValid())
	{
		FullMapWidget->SetSnapshot(PlayerXY, Markers, PlayerMarkerBrush);
	}
}

