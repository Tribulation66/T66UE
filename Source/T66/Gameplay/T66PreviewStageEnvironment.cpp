// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PreviewStageEnvironment.h"

#include "Gameplay/T66TowerMapTerrain.h"
#include "Gameplay/T66TowerThemeVisuals.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	const TCHAR* PreviewFallbackBlockMaterialPath = TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock");

	const FName PreviewBackdropWallName(TEXT("PreviewBackdropWall"));
	const FName PreviewLeftWallName(TEXT("PreviewLeftWall"));
	const FName PreviewRightWallName(TEXT("PreviewRightWall"));
	const FName PreviewCeilingName(TEXT("PreviewCeiling"));
	const FName PreviewDecorSlotNames[] = {
		FName(TEXT("PreviewDecorSlot0")),
		FName(TEXT("PreviewDecorSlot1")),
		FName(TEXT("PreviewDecorSlot2")),
		FName(TEXT("PreviewDecorSlot3")),
		FName(TEXT("PreviewDecorSlot4")),
		FName(TEXT("PreviewDecorSlot5"))
	};

	struct FT66PreviewThemeProfile
	{
		T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme = T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon;
		bool bHideBackdropWalls = false;
		bool bShowCeiling = true;
	};

	struct FT66PreviewDecorSpec
	{
		const TCHAR* MeshPath = nullptr;
		FVector RelativeLocation = FVector::ZeroVector;
		FRotator RelativeRotation = FRotator::ZeroRotator;
		FVector RelativeScale = FVector::OneVector;
		bool bApplyThemeMaterial = false;
	};

	const FT66PreviewDecorSpec ForestDecorSpecs[] = {
		{ TEXT("/Game/World/Props/Tree3.Tree3"), FVector(980.f, -920.f, 0.f), FRotator(0.f, -8.f, 0.f), FVector(1.18f, 1.18f, 1.18f), false },
		{ TEXT("/Game/World/Props/Tree.Tree"),  FVector(1140.f, -260.f, 0.f), FRotator::ZeroRotator, FVector(1.05f, 1.05f, 1.05f), false },
		{ TEXT("/Game/World/Props/Tree2.Tree2"), FVector(1110.f, 320.f, 0.f), FRotator(0.f, 6.f, 0.f), FVector(1.10f, 1.10f, 1.10f), false },
		{ TEXT("/Game/World/Props/Tree.Tree"),  FVector(930.f, 920.f, 0.f), FRotator::ZeroRotator, FVector(1.16f, 1.16f, 1.16f), false },
		{ TEXT("/Game/World/Props/Bush.Bush"),  FVector(430.f, -620.f, 0.f), FRotator(0.f, 18.f, 0.f), FVector(1.10f, 1.10f, 1.10f), false },
		{ TEXT("/Game/World/Props/Bush.Bush"),  FVector(390.f, 620.f, 0.f), FRotator(0.f, -12.f, 0.f), FVector(1.00f, 1.00f, 1.00f), false }
	};

	const FT66PreviewDecorSpec OceanDecorSpecs[] = {
		{ TEXT("/Game/World/Props/Grass.Grass"), FVector(930.f, -880.f, 0.f), FRotator(0.f, 12.f, 0.f), FVector(1.55f, 1.55f, 1.30f), true },
		{ TEXT("/Game/World/Props/Bush.Bush"),   FVector(1120.f, -280.f, 0.f), FRotator(0.f, -12.f, 0.f), FVector(1.10f, 1.10f, 1.10f), true },
		{ TEXT("/Game/World/Props/Rocks.Rocks"), FVector(1160.f, 340.f, 0.f), FRotator(0.f, 22.f, 0.f), FVector(1.16f, 1.16f, 1.16f), true },
		{ TEXT("/Game/World/Props/Grass.Grass"), FVector(900.f, 880.f, 0.f), FRotator(0.f, -18.f, 0.f), FVector(1.45f, 1.45f, 1.28f), true },
		{ TEXT("/Game/World/Props/Rock.Rock"),   FVector(420.f, -560.f, 0.f), FRotator(0.f, 48.f, 0.f), FVector(1.10f, 1.10f, 1.10f), true },
		{ TEXT("/Game/World/Props/Rock.Rock"),   FVector(380.f, 580.f, 0.f), FRotator(0.f, -40.f, 0.f), FVector(1.00f, 1.00f, 1.00f), true }
	};

	const FT66PreviewDecorSpec MartianDecorSpecs[] = {
		{ TEXT("/Game/World/Props/Boulder.Boulder"), FVector(980.f, -860.f, 0.f), FRotator(0.f, -6.f, 0.f), FVector(1.10f, 1.10f, 1.10f), true },
		{ TEXT("/Game/World/Props/Rocks.Rocks"),    FVector(1150.f, -230.f, 0.f), FRotator(0.f, 18.f, 0.f), FVector(1.22f, 1.22f, 1.22f), true },
		{ TEXT("/Game/World/Props/Rock.Rock"),      FVector(1170.f, 340.f, 0.f), FRotator(0.f, 30.f, 0.f), FVector(1.15f, 1.15f, 1.15f), true },
		{ TEXT("/Game/World/Props/Boulder.Boulder"), FVector(920.f, 900.f, 0.f), FRotator(0.f, -10.f, 0.f), FVector(1.04f, 1.04f, 1.04f), true },
		{ TEXT("/Game/World/Props/Branch.Branch"),  FVector(420.f, -600.f, 0.f), FRotator(0.f, 36.f, 0.f), FVector(1.25f, 1.25f, 1.25f), true },
		{ TEXT("/Game/World/Props/Rock.Rock"),      FVector(360.f, 600.f, 0.f), FRotator(0.f, -28.f, 0.f), FVector(1.08f, 1.08f, 1.08f), true }
	};

	const FT66PreviewDecorSpec HellDecorSpecs[] = {
		{ TEXT("/Game/World/Props/Rock.Rock"),       FVector(950.f, -860.f, 0.f), FRotator(0.f, 22.f, 0.f), FVector(1.18f, 1.18f, 1.18f), true },
		{ TEXT("/Game/World/Props/Stump.Stump"),     FVector(1130.f, -220.f, 0.f), FRotator(0.f, -14.f, 0.f), FVector(1.18f, 1.18f, 1.18f), true },
		{ TEXT("/Game/World/Props/Branch.Branch"),   FVector(1170.f, 340.f, 0.f), FRotator(0.f, 34.f, 0.f), FVector(1.30f, 1.30f, 1.30f), true },
		{ TEXT("/Game/World/Props/Boulder.Boulder"), FVector(930.f, 900.f, 0.f), FRotator(0.f, -18.f, 0.f), FVector(1.05f, 1.05f, 1.05f), true },
		{ TEXT("/Game/World/Props/Rocks.Rocks"),     FVector(430.f, -610.f, 0.f), FRotator(0.f, 30.f, 0.f), FVector(1.20f, 1.20f, 1.20f), true },
		{ TEXT("/Game/World/Props/Stump.Stump"),     FVector(360.f, 620.f, 0.f), FRotator(0.f, -24.f, 0.f), FVector(1.06f, 1.06f, 1.06f), true }
	};

	FT66PreviewThemeProfile ResolvePreviewThemeProfile(const ET66Difficulty Difficulty)
	{
		const int32 GameplayLevelNumber = T66TowerMapTerrain::ResolveGameplayLevelNumberForDifficulty(Difficulty);
		const T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme =
			T66TowerMapTerrain::ResolveGameplayLevelTheme(GameplayLevelNumber);

		switch (Theme)
		{
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Forest:
			return {
				Theme,
				true,
				true
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Ocean:
			return {
				Theme,
				false,
				true
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Martian:
			return {
				Theme,
				false,
				true
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Hell:
			return {
				Theme,
				false,
				true
			};
		default:
			return {
				T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon,
				false,
				true
			};
		}
	}

	const FT66PreviewDecorSpec* GetDecorSpecsForTheme(const T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme, int32& OutCount)
	{
		OutCount = 0;

		switch (Theme)
		{
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Forest:
			OutCount = UE_ARRAY_COUNT(ForestDecorSpecs);
			return ForestDecorSpecs;
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Ocean:
			OutCount = UE_ARRAY_COUNT(OceanDecorSpecs);
			return OceanDecorSpecs;
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Martian:
			OutCount = UE_ARRAY_COUNT(MartianDecorSpecs);
			return MartianDecorSpecs;
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Hell:
			OutCount = UE_ARRAY_COUNT(HellDecorSpecs);
			return HellDecorSpecs;
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon:
		default:
			return nullptr;
		}
	}

	UStaticMeshComponent* FindPreviewComponent(AActor* Owner, const FName& ComponentName)
	{
		if (!Owner)
		{
			return nullptr;
		}

		TInlineComponentArray<UStaticMeshComponent*> Components(Owner);
		for (UStaticMeshComponent* Component : Components)
		{
			if (Component && Component->GetFName() == ComponentName)
			{
				return Component;
			}
		}

		return nullptr;
	}

	UStaticMeshComponent* EnsurePreviewComponent(
		AActor* Owner,
		USceneComponent* RootComponent,
		const FName& ComponentName)
	{
		if (!Owner || !RootComponent)
		{
			return nullptr;
		}

		if (UStaticMeshComponent* Existing = FindPreviewComponent(Owner, ComponentName))
		{
			return Existing;
		}

		UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner, ComponentName);
		if (!Component)
		{
			return nullptr;
		}

		Component->SetupAttachment(RootComponent);
		Component->SetMobility(EComponentMobility::Movable);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetGenerateOverlapEvents(false);
		Component->SetCanEverAffectNavigation(false);
		Component->SetVisibility(false, true);
		Component->SetHiddenInGame(true, true);

		Owner->AddInstanceComponent(Component);
		Component->RegisterComponent();
		return Component;
	}

	void SetPreviewComponentVisible(UStaticMeshComponent* Component, const bool bVisible)
	{
		if (!Component)
		{
			return;
		}

		Component->SetVisibility(bVisible, true);
		Component->SetHiddenInGame(!bVisible, true);
	}

	void ApplyMaterialToComponent(UStaticMeshComponent* Component, UMaterialInterface* Material)
	{
		if (!Component || !Material)
		{
			return;
		}

		const int32 MaterialSlots = FMath::Max(Component->GetNumMaterials(), 1);
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlots; ++MaterialIndex)
		{
			Component->SetMaterial(MaterialIndex, Material);
		}
	}

	void ResetComponentMaterials(UStaticMeshComponent* Component)
	{
		if (!Component)
		{
			return;
		}

		Component->EmptyOverrideMaterials();
	}

	void ConfigureBackdropBox(
		UStaticMeshComponent* Component,
		const FVector& RelativeLocation,
		const FVector& DesiredHalfExtents)
	{
		if (!Component)
		{
			return;
		}

		if (UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube())
		{
			Component->SetStaticMesh(CubeMesh);
			const FVector BoundsHalfExtent = CubeMesh->GetBounds().BoxExtent;
			Component->SetRelativeScale3D(FVector(
				DesiredHalfExtents.X / FMath::Max(BoundsHalfExtent.X, 1.0f),
				DesiredHalfExtents.Y / FMath::Max(BoundsHalfExtent.Y, 1.0f),
				DesiredHalfExtents.Z / FMath::Max(BoundsHalfExtent.Z, 1.0f)));
		}

		Component->SetRelativeLocation(RelativeLocation);
		Component->SetRelativeRotation(FRotator::ZeroRotator);
	}

	UMaterialInterface* ResolvePreviewSurfaceMaterial(
		UObject* Outer,
		const FT66PreviewThemeProfile& ThemeProfile,
		const bool bWallSurface,
		const bool bRoofSurface)
	{
		T66TowerThemeVisuals::FResolvedTheme Theme;
		if (T66TowerThemeVisuals::ResolveTheme(Outer, ThemeProfile.Theme, false, Theme))
		{
			if (bRoofSurface)
			{
				return Theme.RoofMaterial ? Theme.RoofMaterial : (Theme.WallMaterial ? Theme.WallMaterial : Theme.FloorMaterial);
			}

			if (bWallSurface)
			{
				return Theme.WallMaterial ? Theme.WallMaterial : Theme.FloorMaterial;
			}

			if (Theme.FloorMaterial)
			{
				return Theme.FloorMaterial;
			}
		}

		return LoadObject<UMaterialInterface>(nullptr, PreviewFallbackBlockMaterialPath);
	}

	UMaterialInterface* CreateSelectionDungeonMaterial(
		UObject* Outer,
		const FLinearColor& BaseColor)
	{
		UMaterialInterface* PreviewBaseMaterial = FT66VisualUtil::GetFlatColorMaterial();
		if (!PreviewBaseMaterial)
		{
			return nullptr;
		}

		UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(PreviewBaseMaterial, Outer);
		if (!MaterialInstance)
		{
			return PreviewBaseMaterial;
		}

		FT66VisualUtil::ConfigureFlatColorMaterial(MaterialInstance, BaseColor);
		return MaterialInstance;
	}

	void ConfigureDecorSlot(
		UStaticMeshComponent* Component,
		const FT66PreviewDecorSpec* Spec,
		UMaterialInterface* ThemeMaterial,
		const bool bVisible)
	{
		if (!Component)
		{
			return;
		}

		if (!Spec || !bVisible)
		{
			SetPreviewComponentVisible(Component, false);
			return;
		}

		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Spec->MeshPath);
		if (!Mesh)
		{
			Component->SetStaticMesh(nullptr);
			SetPreviewComponentVisible(Component, false);
			return;
		}

		Component->SetStaticMesh(Mesh);
		Component->SetRelativeScale3D(Spec->RelativeScale);
		Component->SetRelativeRotation(Spec->RelativeRotation);
		Component->SetRelativeLocation(FVector(Spec->RelativeLocation.X, Spec->RelativeLocation.Y, 0.0f));
		FT66VisualUtil::GroundMeshToActorOrigin(Component, Mesh);
		Component->AddRelativeLocation(FVector(0.0f, 0.0f, Spec->RelativeLocation.Z));

		if (Spec->bApplyThemeMaterial && ThemeMaterial)
		{
			ApplyMaterialToComponent(Component, ThemeMaterial);
		}
		else
		{
			ResetComponentMaterials(Component);
		}

		SetPreviewComponentVisible(Component, true);
	}
}

void T66PreviewStageEnvironment::EnsurePreviewEnvironmentBuilt(
	AActor* Owner,
	USceneComponent* RootComponent)
{
	if (!Owner || !RootComponent)
	{
		return;
	}

	ConfigureBackdropBox(
		EnsurePreviewComponent(Owner, RootComponent, PreviewBackdropWallName),
		FVector(1150.0f, 0.0f, 430.0f),
		FVector(70.0f, 860.0f, 430.0f));

	ConfigureBackdropBox(
		EnsurePreviewComponent(Owner, RootComponent, PreviewLeftWallName),
		FVector(620.0f, -980.0f, 390.0f),
		FVector(650.0f, 70.0f, 390.0f));

	ConfigureBackdropBox(
		EnsurePreviewComponent(Owner, RootComponent, PreviewRightWallName),
		FVector(620.0f, 980.0f, 390.0f),
		FVector(650.0f, 70.0f, 390.0f));

	ConfigureBackdropBox(
		EnsurePreviewComponent(Owner, RootComponent, PreviewCeilingName),
		FVector(760.0f, 0.0f, 870.0f),
		FVector(670.0f, 860.0f, 40.0f));

	for (const FName& DecorSlotName : PreviewDecorSlotNames)
	{
		(void)EnsurePreviewComponent(Owner, RootComponent, DecorSlotName);
	}
}

void T66PreviewStageEnvironment::ApplyPreviewEnvironment(
	AActor* Owner,
	UStaticMeshComponent* GroundComponent,
	ET66Difficulty Difficulty,
	ET66PreviewStageMode PreviewStageMode,
	bool bVisible)
{
	if (!Owner || !GroundComponent)
	{
		return;
	}

	EnsurePreviewEnvironmentBuilt(Owner, Owner->GetRootComponent());

	const FT66PreviewThemeProfile ThemeProfile = ResolvePreviewThemeProfile(Difficulty);
	UMaterialInterface* GroundMaterial = ResolvePreviewSurfaceMaterial(GroundComponent, ThemeProfile, false, false);
	UMaterialInterface* WallMaterial = ResolvePreviewSurfaceMaterial(Owner, ThemeProfile, true, false);
	UMaterialInterface* RoofMaterial = ResolvePreviewSurfaceMaterial(Owner, ThemeProfile, false, true);
	const bool bSelectionDungeonStage =
		PreviewStageMode == ET66PreviewStageMode::Selection
		&& ThemeProfile.Theme == T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon;

	if (bSelectionDungeonStage)
	{
		if (UMaterialInterface* SelectionGroundMaterial = CreateSelectionDungeonMaterial(
			GroundComponent,
			FLinearColor(0.065f, 0.055f, 0.047f, 1.0f)))
		{
			GroundMaterial = SelectionGroundMaterial;
		}
		if (UMaterialInterface* SelectionWallMaterial = CreateSelectionDungeonMaterial(
			Owner,
			FLinearColor(0.038f, 0.032f, 0.042f, 1.0f)))
		{
			WallMaterial = SelectionWallMaterial;
		}
		if (UMaterialInterface* SelectionRoofMaterial = CreateSelectionDungeonMaterial(
			Owner,
			FLinearColor(0.018f, 0.015f, 0.024f, 1.0f)))
		{
			RoofMaterial = SelectionRoofMaterial;
		}
	}

	if (!WallMaterial)
	{
		WallMaterial = GroundMaterial;
	}
	if (!RoofMaterial)
	{
		RoofMaterial = WallMaterial ? WallMaterial : GroundMaterial;
	}

	ApplyMaterialToComponent(GroundComponent, GroundMaterial);

	const bool bShowSelectionDressing = bVisible && PreviewStageMode == ET66PreviewStageMode::Selection;
	const bool bShowBackdropWalls = bShowSelectionDressing && !ThemeProfile.bHideBackdropWalls;
	const bool bShowCeiling = bShowSelectionDressing && ThemeProfile.bShowCeiling;

	if (UStaticMeshComponent* BackdropWall = FindPreviewComponent(Owner, PreviewBackdropWallName))
	{
		ApplyMaterialToComponent(BackdropWall, WallMaterial);
		SetPreviewComponentVisible(BackdropWall, bShowBackdropWalls);
	}
	if (UStaticMeshComponent* LeftWall = FindPreviewComponent(Owner, PreviewLeftWallName))
	{
		ApplyMaterialToComponent(LeftWall, WallMaterial);
		SetPreviewComponentVisible(LeftWall, bShowBackdropWalls);
	}
	if (UStaticMeshComponent* RightWall = FindPreviewComponent(Owner, PreviewRightWallName))
	{
		ApplyMaterialToComponent(RightWall, WallMaterial);
		SetPreviewComponentVisible(RightWall, bShowBackdropWalls);
	}
	if (UStaticMeshComponent* Ceiling = FindPreviewComponent(Owner, PreviewCeilingName))
	{
		ApplyMaterialToComponent(Ceiling, RoofMaterial);
		SetPreviewComponentVisible(Ceiling, bShowCeiling);
	}

	int32 DecorSpecCount = 0;
	const FT66PreviewDecorSpec* DecorSpecs = GetDecorSpecsForTheme(ThemeProfile.Theme, DecorSpecCount);
	for (int32 DecorIndex = 0; DecorIndex < UE_ARRAY_COUNT(PreviewDecorSlotNames); ++DecorIndex)
	{
		const FT66PreviewDecorSpec* DecorSpec = (DecorSpecs && DecorIndex < DecorSpecCount) ? &DecorSpecs[DecorIndex] : nullptr;
		ConfigureDecorSlot(
			FindPreviewComponent(Owner, PreviewDecorSlotNames[DecorIndex]),
			DecorSpec,
			WallMaterial,
			bShowSelectionDressing);
	}
}
