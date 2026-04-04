// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PreviewStageEnvironment.h"

#include "Gameplay/T66VisualUtil.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66PreviewEnvironment, Log, All);

namespace
{
	const TCHAR* PreviewFallbackBlockMaterialPath = TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock");
	const TCHAR* PreviewGroundBaseMaterialPath = TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit");
	const TCHAR* EasyBlockTexturePath = TEXT("/Game/World/Terrain/Megabonk/T_MegabonkBlock.T_MegabonkBlock");

	struct FDifficultyThemeAssetInfo
	{
		const TCHAR* FolderName = nullptr;
		const TCHAR* AssetSuffix = nullptr;
	};

	struct FT66PreviewPropSpec
	{
		const TCHAR* Name;
		const TCHAR* MeshPath;
		FVector RelativeLocation;
		FRotator RelativeRotation;
		FVector RelativeScale;
	};

	const FT66PreviewPropSpec EasyFarmPreviewProps[] =
	{
		{ TEXT("PreviewBarn"), TEXT("/Game/World/Props/Barn.Barn"), FVector(3200.f, -2500.f, 0.f), FRotator(0.f, -18.f, 0.f), FVector(1.0f, 1.0f, 1.0f) },
		{ TEXT("PreviewWindmill"), TEXT("/Game/World/Props/Windmill.Windmill"), FVector(4200.f, 2600.f, 0.f), FRotator(0.f, 180.f, 0.f), FVector(1.0f, 1.0f, 1.0f) },
		{ TEXT("PreviewSilo"), TEXT("/Game/World/Props/Silo.Silo"), FVector(3400.f, 1400.f, 0.f), FRotator(0.f, 170.f, 0.f), FVector(1.0f, 1.0f, 1.0f) },
		{ TEXT("PreviewTreeLeft"), TEXT("/Game/World/Props/Tree3.Tree3"), FVector(1700.f, -2100.f, 0.f), FRotator::ZeroRotator, FVector(1.05f, 1.05f, 1.05f) },
		{ TEXT("PreviewTreeRight"), TEXT("/Game/World/Props/Tree.Tree"), FVector(1700.f, 2200.f, 0.f), FRotator::ZeroRotator, FVector(1.0f, 1.0f, 1.0f) },
		{ TEXT("PreviewTreeBack"), TEXT("/Game/World/Props/Tree2.Tree2"), FVector(2800.f, 3000.f, 0.f), FRotator::ZeroRotator, FVector(1.1f, 1.1f, 1.1f) },
		{ TEXT("PreviewTractor"), TEXT("/Game/World/Props/Tractor.Tractor"), FVector(1900.f, -950.f, 0.f), FRotator(0.f, 22.f, 0.f), FVector(1.0f, 1.0f, 1.0f) },
		{ TEXT("PreviewHay"), TEXT("/Game/World/Props/Haybell.Haybell"), FVector(1750.f, 950.f, 0.f), FRotator(0.f, -30.f, 0.f), FVector(0.95f, 0.95f, 0.95f) },
		{ TEXT("PreviewScarecrow"), TEXT("/Game/World/Props/Scarecrow.Scarecrow"), FVector(2150.f, 240.f, 0.f), FRotator(0.f, 180.f, 0.f), FVector(1.0f, 1.0f, 1.0f) },
		{ TEXT("PreviewRocks"), TEXT("/Game/World/Props/Rocks.Rocks"), FVector(1350.f, 1550.f, 0.f), FRotator(0.f, 145.f, 0.f), FVector(1.0f, 1.0f, 1.0f) },
	};

	FDifficultyThemeAssetInfo GetDifficultyThemeAssetInfo(ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Medium: return { TEXT("VeryHardGraveyard"), TEXT("VeryHardGraveyard") };
		case ET66Difficulty::Hard: return { TEXT("ImpossibleNorthPole"), TEXT("ImpossibleNorthPole") };
		case ET66Difficulty::VeryHard: return { TEXT("PerditionMars"), TEXT("PerditionMars") };
		case ET66Difficulty::Impossible: return { TEXT("FinalHell"), TEXT("FinalHell") };
		case ET66Difficulty::Easy:
		default:
			return {};
		}
	}

	UTexture* LoadPreviewBlockTexture(ET66Difficulty Difficulty)
	{
		if (Difficulty == ET66Difficulty::Easy)
		{
			return LoadObject<UTexture>(nullptr, EasyBlockTexturePath);
		}

		const FDifficultyThemeAssetInfo ThemeInfo = GetDifficultyThemeAssetInfo(Difficulty);
		if (!ThemeInfo.FolderName || !ThemeInfo.AssetSuffix)
		{
			return LoadObject<UTexture>(nullptr, EasyBlockTexturePath);
		}

		const FString AssetName = FString::Printf(TEXT("T_MegabonkBlock_%s"), ThemeInfo.AssetSuffix);
		const FString AssetPath = FString::Printf(
			TEXT("/Game/World/Terrain/MegabonkThemes/%s/%s.%s"),
			ThemeInfo.FolderName,
			*AssetName,
			*AssetName);

		if (UTexture2D* ThemeTexture = LoadObject<UTexture2D>(nullptr, *AssetPath))
		{
			return ThemeTexture;
		}

		UE_LOG(LogT66PreviewEnvironment, Warning, TEXT("[PREVIEW] Missing preview block texture override: %s"), *AssetPath);
		return LoadObject<UTexture>(nullptr, EasyBlockTexturePath);
	}

	UMaterialInterface* BuildPreviewGroundMaterial(UStaticMeshComponent* GroundComponent, ET66Difficulty Difficulty)
	{
		if (!GroundComponent)
		{
			return nullptr;
		}

		UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, PreviewGroundBaseMaterialPath);
		UTexture* BlockTexture = LoadPreviewBlockTexture(Difficulty);
		if (BaseMaterial && BlockTexture)
		{
			if (UMaterialInstanceDynamic* GroundMID = UMaterialInstanceDynamic::Create(BaseMaterial, GroundComponent, TEXT("PreviewGroundMID")))
			{
				GroundMID->SetTextureParameterValue(TEXT("DiffuseColorMap"), BlockTexture);
				GroundMID->SetTextureParameterValue(TEXT("BaseColorTexture"), BlockTexture);
				GroundMID->SetScalarParameterValue(TEXT("Brightness"), 1.0f);
				GroundMID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
				GroundMID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
				return GroundMID;
			}
		}

		if (UMaterialInterface* FallbackBlockMaterial = LoadObject<UMaterialInterface>(nullptr, PreviewFallbackBlockMaterialPath))
		{
			return FallbackBlockMaterial;
		}

		return nullptr;
	}

	UStaticMeshComponent* CreatePreviewPropComponent(
		AActor* Owner,
		USceneComponent* RootComponent,
		const FT66PreviewPropSpec& Spec)
	{
		if (!Owner || !RootComponent)
		{
			return nullptr;
		}

		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Spec.MeshPath);
		if (!Mesh)
		{
			return nullptr;
		}

		UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner, *FString::Printf(TEXT("%sComponent"), Spec.Name));
		if (!Component)
		{
			return nullptr;
		}

		Component->SetupAttachment(RootComponent);
		Component->SetMobility(EComponentMobility::Movable);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetStaticMesh(Mesh);
		Component->SetRelativeScale3D(Spec.RelativeScale);
		Component->SetRelativeRotation(Spec.RelativeRotation);
		Component->SetRelativeLocation(FVector(Spec.RelativeLocation.X, Spec.RelativeLocation.Y, 0.f));
		FT66VisualUtil::GroundMeshToActorOrigin(Component, Mesh);
		Component->AddRelativeLocation(FVector(0.f, 0.f, Spec.RelativeLocation.Z));
		Component->SetVisibility(false, true);
		Component->SetHiddenInGame(true, true);

		Owner->AddInstanceComponent(Component);
		Component->RegisterComponent();
		return Component;
	}
}

void T66PreviewStageEnvironment::ApplyPreviewGroundMaterial(UStaticMeshComponent* GroundComponent, ET66Difficulty Difficulty)
{
	if (!GroundComponent)
	{
		return;
	}

	if (UMaterialInterface* GroundMat = BuildPreviewGroundMaterial(GroundComponent, Difficulty))
	{
		GroundComponent->SetMaterial(0, GroundMat);
	}
}

void T66PreviewStageEnvironment::CreateEasyFarmPreviewProps(
	AActor* Owner,
	USceneComponent* RootComponent,
	TArray<TObjectPtr<UStaticMeshComponent>>& OutComponents)
{
	if (!Owner || !RootComponent || OutComponents.Num() > 0)
	{
		return;
	}

	for (const FT66PreviewPropSpec& Spec : EasyFarmPreviewProps)
	{
		if (UStaticMeshComponent* Component = CreatePreviewPropComponent(Owner, RootComponent, Spec))
		{
			OutComponents.Add(Component);
		}
	}
}

void T66PreviewStageEnvironment::SetPreviewPropsVisibility(
	const TArray<TObjectPtr<UStaticMeshComponent>>& Props,
	bool bVisible)
{
	for (UStaticMeshComponent* Component : Props)
	{
		if (!Component)
		{
			continue;
		}

		Component->SetVisibility(bVisible, true);
		Component->SetHiddenInGame(!bVisible, true);
	}
}

bool T66PreviewStageEnvironment::ShouldShowEasyProps(ET66Difficulty Difficulty)
{
	return Difficulty == ET66Difficulty::Easy;
}
