// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "UI/T66GameplayHUDWidget.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66CommunityContentSubsystem.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66LeaderboardPacingUtils.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66StageGate.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66StageCatchUpLootInteractable.h"
#include "Gameplay/T66OuroborosNPC.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66MiasmaBoundary.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66ItemCardTextUtils.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66CrateOverlayWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Rendering/DrawElements.h"
#include "Math/TransformCalculus2D.h"
#include "Rendering/SlateRenderTransform.h"
#include "Engine/Texture2D.h"
#include "Engine/DataTable.h"
// [GOLD] EngineUtils.h removed — TActorIterator replaced by ActorRegistry.
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SToolTip.h"

DECLARE_LOG_CATEGORY_EXTERN(LogT66HUD, Log, All);

namespace
{
	static FLinearColor WithAlpha(const FLinearColor& Color, float Alpha)
	{
		FLinearColor Out = Color;
		Out.A = Alpha;
		return Out;
	}

	static FText GetBossPartDisplayName(const FT66BossPartSnapshot& Part)
	{
		switch (Part.HitZoneType)
		{
		case ET66HitZoneType::Head:
			return NSLOCTEXT("T66.GameplayHUD", "BossPartHead", "Head");
		case ET66HitZoneType::Core:
			return NSLOCTEXT("T66.GameplayHUD", "BossPartCore", "Core");
		case ET66HitZoneType::WeakPoint:
			return NSLOCTEXT("T66.GameplayHUD", "BossPartWeakPoint", "Weak Point");
		case ET66HitZoneType::LeftArm:
			return NSLOCTEXT("T66.GameplayHUD", "BossPartLeftArm", "Left Arm");
		case ET66HitZoneType::RightArm:
			return NSLOCTEXT("T66.GameplayHUD", "BossPartRightArm", "Right Arm");
		case ET66HitZoneType::LeftLeg:
			return NSLOCTEXT("T66.GameplayHUD", "BossPartLeftLeg", "Left Leg");
		case ET66HitZoneType::RightLeg:
			return NSLOCTEXT("T66.GameplayHUD", "BossPartRightLeg", "Right Leg");
		case ET66HitZoneType::Body:
			return NSLOCTEXT("T66.GameplayHUD", "BossPartBody", "Body");
		case ET66HitZoneType::None:
		default:
			return Part.PartID.IsNone() ? NSLOCTEXT("T66.GameplayHUD", "BossPartGeneric", "Part") : FText::FromName(Part.PartID);
		}
	}

	static TSoftObjectPtr<UTexture2D> ResolveGameplayUltimateIcon(const FName HeroID, const ET66UltimateType UltimateType)
	{
		if (HeroID == FName(TEXT("Hero_1")) && UltimateType == ET66UltimateType::SpearStorm)
		{
			return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Abilities/Hero_1/T_Hero_1_Ultimate.T_Hero_1_Ultimate")));
		}

		return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/ULTS/KnightULT.KnightULT")));
	}

	static TSoftObjectPtr<UTexture2D> ResolveGameplayPassiveIcon(const FName HeroID, const ET66PassiveType PassiveType)
	{
		if (HeroID == FName(TEXT("Hero_1")) && PassiveType == ET66PassiveType::IronWill)
		{
			return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Abilities/Hero_1/T_Hero_1_Passive.T_Hero_1_Passive")));
		}

		return ResolveGameplayUltimateIcon(HeroID, ET66UltimateType::None);
	}

	static TSoftObjectPtr<UTexture2D> ResolveGameplayHeartIcon(const int32 TierIndex)
	{
		switch (FMath::Clamp(TierIndex, 0, 4))
		{
		case 0: return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/Hearts/T_Heart_Red.T_Heart_Red")));
		case 1: return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/Hearts/T_Heart_Blue.T_Heart_Blue")));
		case 2: return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/Hearts/T_Heart_Green.T_Heart_Green")));
		case 3: return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/Hearts/T_Heart_Purple.T_Heart_Purple")));
		case 4:
		default:
			return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/Hearts/T_Heart_Gold.T_Heart_Gold")));
		}
	}

	static TSoftObjectPtr<UTexture2D> ResolveGameplayBlessingHeartIcon()
	{
		return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/Hearts/T_Heart_Blessed.T_Heart_Blessed")));
	}

	static FText ResolveGameplayUltimateInputHint(const ET66UltimateType UltimateType)
	{
		return (UltimateType == ET66UltimateType::SpearStorm)
			? NSLOCTEXT("T66.GameplayHUD", "UltKeybindRmb", "RMB")
			: NSLOCTEXT("T66.GameplayHUD", "UltKeybindDefault", "R");
	}

	static int32 ResolveDisplayedStageNumber(
		const UT66PlayerExperienceSubSystem* PlayerExperience,
		ET66Difficulty Difficulty,
		int32 CurrentStage)
	{
		if (CurrentStage <= 0)
		{
			return 1;
		}

		if (!PlayerExperience)
		{
			return CurrentStage;
		}

		const int32 StartStage = FMath::Max(1, PlayerExperience->GetDifficultyStartStage(Difficulty));
		const int32 StageOffset = FMath::Max(0, CurrentStage - StartStage);
		if (Difficulty == ET66Difficulty::Impossible)
		{
			return (StageOffset % 3) + 1;
		}

		return (StageOffset % 4) + 1;
	}

	static FText BuildDisplayedStageText(
		UT66LocalizationSubsystem* Loc,
		const UT66PlayerExperienceSubSystem* PlayerExperience,
		ET66Difficulty Difficulty,
		int32 CurrentStage,
		bool bInStageCatchUp)
	{
		if (bInStageCatchUp)
		{
			return NSLOCTEXT("T66.GameplayHUD", "StageCatchUp", "Stage: Catch Up");
		}

		const FText Fmt = Loc ? Loc->GetText_StageNumberFormat() : NSLOCTEXT("T66.GameplayHUD", "StageNumberFormat", "Stage number: {0}");
		return FText::Format(Fmt, FText::AsNumber(ResolveDisplayedStageNumber(PlayerExperience, Difficulty, CurrentStage)));
	}

	static const FSlateBrush* ResolveHeartBrushForDisplay(
		const TArray<TSharedPtr<FSlateBrush>>& HeartBrushes,
		const TSharedPtr<FSlateBrush>& BlessingBrush,
		const TSharedPtr<FSlateBrush>& FallbackBrush,
		const bool bBlessed,
		const int32 TierIndex)
	{
		if (bBlessed && BlessingBrush.IsValid() && BlessingBrush->GetResourceObject())
		{
			return BlessingBrush.Get();
		}

		if (HeartBrushes.IsValidIndex(TierIndex) && HeartBrushes[TierIndex].IsValid() && HeartBrushes[TierIndex]->GetResourceObject())
		{
			return HeartBrushes[TierIndex].Get();
		}

		return FallbackBrush.Get();
	}

	static FLinearColor GetBossPartFillColor(const ET66HitZoneType HitZoneType, const bool bIsAlive)
	{
		if (!bIsAlive)
		{
			return FLinearColor(0.22f, 0.22f, 0.22f, 0.95f);
		}

		switch (HitZoneType)
		{
		case ET66HitZoneType::Head:
			return FLinearColor(0.95f, 0.28f, 0.18f, 0.98f);
		case ET66HitZoneType::Core:
			return FLinearColor(0.88f, 0.12f, 0.12f, 0.98f);
		case ET66HitZoneType::WeakPoint:
			return FLinearColor(0.95f, 0.70f, 0.18f, 0.98f);
		case ET66HitZoneType::LeftArm:
		case ET66HitZoneType::RightArm:
			return FLinearColor(0.82f, 0.26f, 0.42f, 0.98f);
		case ET66HitZoneType::LeftLeg:
		case ET66HitZoneType::RightLeg:
			return FLinearColor(0.72f, 0.18f, 0.18f, 0.98f);
		case ET66HitZoneType::Body:
		case ET66HitZoneType::None:
		default:
			return FLinearColor(0.84f, 0.18f, 0.18f, 0.98f);
		}
	}

	static TMap<FString, TStrongObjectPtr<UTexture2D>> GHudRuntimeTextureCache;

	static UTexture2D* LoadRuntimeHudFileTexture(const FString& RelativePath, TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		const TArray<FString> CandidatePaths = T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath);

		for (const FString& FullPath : CandidatePaths)
		{
			if (!FPaths::FileExists(FullPath))
			{
				continue;
			}

			const FString CacheKey = FString::Printf(TEXT("%s|%d"), *FullPath, static_cast<int32>(Filter));
			if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GHudRuntimeTextureCache.Find(CacheKey))
			{
				return CachedTexture->Get();
			}

			UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
				FullPath,
				Filter,
				true,
				TEXT("HudRuntimeTexture"));
			if (!Texture)
			{
				continue;
			}

			GHudRuntimeTextureCache.Add(CacheKey, TStrongObjectPtr<UTexture2D>(Texture));
			return Texture;
		}

		return nullptr;
	}

	static void BindRuntimeHudBrush(
		const TSharedPtr<FSlateBrush>& Brush,
		const FString& RelativePath,
		const FVector2D& DesiredSize,
		TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->ImageSize = DesiredSize;
		Brush->SetResourceObject(LoadRuntimeHudFileTexture(RelativePath, Filter));
	}

	static void BindHudAssetBrush(
		const TSharedPtr<FSlateBrush>& Brush,
		const TCHAR* AssetPath,
		const FVector2D& DesiredSize,
		TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->ImageSize = DesiredSize;
		Brush->SetResourceObject(T66RuntimeUITextureAccess::LoadAssetTexture(AssetPath, Filter, TEXT("HudAssetTexture")));
	}

	static FString GetChestRewardClosedRelativePath(const ET66Rarity Rarity)
	{
		switch (Rarity)
		{
		case ET66Rarity::Red: return TEXT("RuntimeDependencies/T66/UI/ChestRewards/chest_reward_red_closed.png");
		case ET66Rarity::Yellow: return TEXT("RuntimeDependencies/T66/UI/ChestRewards/chest_reward_yellow_closed.png");
		case ET66Rarity::White: return TEXT("RuntimeDependencies/T66/UI/ChestRewards/chest_reward_white_closed.png");
		case ET66Rarity::Black:
		default: return TEXT("RuntimeDependencies/T66/UI/ChestRewards/chest_reward_black_closed.png");
		}
	}

	static FString GetChestRewardOpenRelativePath(const ET66Rarity Rarity)
	{
		switch (Rarity)
		{
		case ET66Rarity::Red: return TEXT("RuntimeDependencies/T66/UI/ChestRewards/chest_reward_red_open.png");
		case ET66Rarity::Yellow: return TEXT("RuntimeDependencies/T66/UI/ChestRewards/chest_reward_yellow_open.png");
		case ET66Rarity::White: return TEXT("RuntimeDependencies/T66/UI/ChestRewards/chest_reward_white_open.png");
		case ET66Rarity::Black:
		default: return TEXT("RuntimeDependencies/T66/UI/ChestRewards/chest_reward_black_open.png");
		}
	}

	static FString GetChestRewardFallbackRelativePath(const ET66Rarity Rarity)
	{
		switch (Rarity)
		{
		case ET66Rarity::Red: return TEXT("SourceAssets/ItemSprites/Item_TreasureChest_red.png");
		case ET66Rarity::Yellow: return TEXT("SourceAssets/ItemSprites/Item_TreasureChest_yellow.png");
		case ET66Rarity::White: return TEXT("SourceAssets/ItemSprites/Item_TreasureChest_white.png");
		case ET66Rarity::Black:
		default: return TEXT("SourceAssets/ItemSprites/Item_TreasureChest_black.png");
		}
	}

	static const TCHAR* GetChestRewardFallbackAssetPath(const ET66Rarity Rarity)
	{
		switch (Rarity)
		{
		case ET66Rarity::Red: return TEXT("/Game/Items/Sprites/Item_TreasureChest_red.Item_TreasureChest_red");
		case ET66Rarity::Yellow: return TEXT("/Game/Items/Sprites/Item_TreasureChest_yellow.Item_TreasureChest_yellow");
		case ET66Rarity::White: return TEXT("/Game/Items/Sprites/Item_TreasureChest_white.Item_TreasureChest_white");
		case ET66Rarity::Black:
		default: return TEXT("/Game/Items/Sprites/Item_TreasureChest_black.Item_TreasureChest_black");
		}
	}

	static FString GetGoldCurrencyRelativePath()
	{
		return TEXT("RuntimeDependencies/T66/UI/Currency/gold_icon.png");
	}

	static FString GetDebtCurrencyRelativePath()
	{
		return TEXT("RuntimeDependencies/T66/UI/Currency/debt_icon.png");
	}

	enum class ET66MapMarkerVisual : uint8
	{
		Dot,
		Icon,
	};

	static TObjectPtr<UTexture2D> GMinimapIconAtlas = nullptr;
	static bool bTriedMinimapIconAtlasLoad = false;

	static UTexture2D* GetMinimapIconAtlas()
	{
		if (!bTriedMinimapIconAtlasLoad)
		{
			bTriedMinimapIconAtlasLoad = true;
			for (const FString& AtlasPath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(
				TEXT("RuntimeDependencies/T66/UI/Minimap/#1 - Transparent Icons.png")))
			{
				if (!FPaths::FileExists(AtlasPath))
				{
					continue;
				}

				GMinimapIconAtlas = T66RuntimeUITextureAccess::ImportFileTexture(
					AtlasPath,
					TextureFilter::TF_Trilinear,
					false,
					TEXT("MinimapIconAtlas"));
				if (GMinimapIconAtlas)
				{
					GMinimapIconAtlas->AddToRoot();
					break;
				}
			}
		}

		return GMinimapIconAtlas.Get();
	}

	static TSharedPtr<FSlateBrush> MakeAtlasBrushFromPixels(float X, float Y, float W, float H, const FVector2D& DrawSize)
	{
		UTexture2D* Atlas = GetMinimapIconAtlas();
		if (!Atlas || Atlas->GetSizeX() <= 0 || Atlas->GetSizeY() <= 0)
		{
			return nullptr;
		}

		const float AtlasW = static_cast<float>(Atlas->GetSizeX());
		const float AtlasH = static_cast<float>(Atlas->GetSizeY());

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = DrawSize;
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(Atlas);
		Brush->SetUVRegion(FBox2f(
			FVector2f(X / AtlasW, Y / AtlasH),
			FVector2f((X + W) / AtlasW, (Y + H) / AtlasH)));
		return Brush;
	}

	struct FT66LooseMinimapIconSpec
	{
		const TCHAR* RelativePath = nullptr;
		FVector2D DrawSize = FVector2D(20.0f, 20.0f);
	};

	static const FT66LooseMinimapIconSpec* FindLooseMinimapIconSpec(const FName Key)
	{
		static const TMap<FName, FT66LooseMinimapIconSpec> Specs = {
			{ FName(TEXT("NPC")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/npc.png"), FVector2D(22.f, 18.f) } },
			{ FName(TEXT("Vendor")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/vendor.png"), FVector2D(20.f, 20.f) } },
			{ FName(TEXT("SupportVendor")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/support_vendor.png"), FVector2D(20.f, 20.f) } },
			{ FName(TEXT("Gambler")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/gambler.png"), FVector2D(20.f, 20.f) } },
			{ FName(TEXT("Saint")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/saint.png"), FVector2D(20.f, 20.f) } },
			{ FName(TEXT("Ouroboros")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/ouroboros.png"), FVector2D(20.f, 20.f) } },
			{ FName(TEXT("Collector")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/collector.png"), FVector2D(20.f, 20.f) } },
			{ FName(TEXT("Trickster")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/trickster.png"), FVector2D(20.f, 20.f) } },
			{ FName(TEXT("Gate")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/gate.png"), FVector2D(20.f, 20.f) } },
			{ FName(TEXT("Miasma")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/miasma.png"), FVector2D(20.f, 20.f) } },
			{ FName(TEXT("Chest")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/chest.png"), FVector2D(20.f, 20.f) } },
			{ FName(TEXT("Crate")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/crate.png"), FVector2D(20.f, 20.f) } },
			{ FName(TEXT("LootBag")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/loot_bag.png"), FVector2D(20.f, 20.f) } },
			{ FName(TEXT("CatchUpLoot")), { TEXT("RuntimeDependencies/T66/UI/Minimap/Icons/catch_up_loot.png"), FVector2D(20.f, 20.f) } },
		};

		return Specs.Find(Key);
	}

	static TSharedPtr<FSlateBrush> MakeLooseMinimapBrush(const FT66LooseMinimapIconSpec& Spec)
	{
		if (!Spec.RelativePath || !Spec.RelativePath[0])
		{
			return nullptr;
		}

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = Spec.DrawSize;
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(LoadRuntimeHudFileTexture(Spec.RelativePath, TextureFilter::TF_Trilinear));
		return Brush->GetResourceObject() ? Brush : nullptr;
	}

	static const FSlateBrush* GetMinimapSymbolBrush(FName Key)
	{
		static TMap<FName, TSharedPtr<FSlateBrush>> Brushes;

		if (const TSharedPtr<FSlateBrush>* Found = Brushes.Find(Key))
		{
			return Found->IsValid() ? Found->Get() : nullptr;
		}

		if (const FT66LooseMinimapIconSpec* Spec = FindLooseMinimapIconSpec(Key))
		{
			if (TSharedPtr<FSlateBrush> LooseBrush = MakeLooseMinimapBrush(*Spec))
			{
				Brushes.Add(Key, LooseBrush);
				return LooseBrush.Get();
			}
		}

		static bool bAtlasInitialized = false;
		if (!bAtlasInitialized)
		{
			bAtlasInitialized = true;
			Brushes.Add(FName(TEXT("NPC")), MakeAtlasBrushFromPixels(95.f, 6.f, 30.f, 24.f, FVector2D(22.f, 18.f)));
			Brushes.Add(FName(TEXT("Gate")), MakeAtlasBrushFromPixels(1.f, 41.f, 31.f, 31.f, FVector2D(20.f, 20.f)));
			Brushes.Add(FName(TEXT("Miasma")), MakeAtlasBrushFromPixels(432.f, 57.f, 31.f, 31.f, FVector2D(20.f, 20.f)));
		}

		if (const TSharedPtr<FSlateBrush>* AtlasBrush = Brushes.Find(Key))
		{
			return AtlasBrush->IsValid() ? AtlasBrush->Get() : nullptr;
		}

		if (const TSharedPtr<FSlateBrush>* Fallback = Brushes.Find(FName(TEXT("NPC"))))
		{
			return Fallback->IsValid() ? Fallback->Get() : nullptr;
		}

		return nullptr;
	}

	static FLinearColor GetMinimapMarkerTint(const FSlateBrush* Brush, const FLinearColor& MarkerColor, const bool bMinimap)
	{
		if (!Brush || !Brush->GetResourceObject())
		{
			return FLinearColor(MarkerColor.R, MarkerColor.G, MarkerColor.B, bMinimap ? 0.95f : 0.98f);
		}

		UTexture2D* Atlas = GetMinimapIconAtlas();
		if (Atlas && Brush->GetResourceObject() == Atlas)
		{
			return FLinearColor(MarkerColor.R, MarkerColor.G, MarkerColor.B, bMinimap ? 0.95f : 0.98f);
		}

		return FLinearColor(1.f, 1.f, 1.f, bMinimap ? 0.95f : 0.98f);
	}

	struct FT66TowerMinimapArtStyle
	{
		FString RelativePath;
		FLinearColor MapTint = FLinearColor::White;
		FLinearColor WallFill = FLinearColor(0.14f, 0.11f, 0.09f, 1.0f);
		FLinearColor WallStroke = FLinearColor(0.92f, 0.86f, 0.72f, 1.0f);
	};

	static FT66TowerMinimapArtStyle GetTowerMinimapArtStyle(const T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme)
	{
		switch (Theme)
		{
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Forest:
			return {
				TEXT("RuntimeDependencies/T66/UI/Minimap/Backgrounds/tower_forest_background.png"),
				FLinearColor(0.95f, 1.00f, 0.95f, 1.0f),
				FLinearColor(0.17f, 0.20f, 0.14f, 1.0f),
				FLinearColor(0.78f, 0.88f, 0.58f, 1.0f)
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Ocean:
			return {
				TEXT("RuntimeDependencies/T66/UI/Minimap/Backgrounds/tower_ocean_background.png"),
				FLinearColor(0.95f, 0.98f, 1.00f, 1.0f),
				FLinearColor(0.11f, 0.15f, 0.18f, 1.0f),
				FLinearColor(0.64f, 0.82f, 0.96f, 1.0f)
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Martian:
			return {
				TEXT("RuntimeDependencies/T66/UI/Minimap/Backgrounds/tower_martian_background.png"),
				FLinearColor(1.00f, 0.97f, 0.95f, 1.0f),
				FLinearColor(0.20f, 0.11f, 0.08f, 1.0f),
				FLinearColor(0.94f, 0.64f, 0.48f, 1.0f)
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Hell:
			return {
				TEXT("RuntimeDependencies/T66/UI/Minimap/Backgrounds/tower_hell_background.png"),
				FLinearColor(1.00f, 0.95f, 0.94f, 1.0f),
				FLinearColor(0.18f, 0.07f, 0.06f, 1.0f),
				FLinearColor(0.94f, 0.42f, 0.30f, 1.0f)
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon:
		default:
			return {
				TEXT("RuntimeDependencies/T66/UI/Minimap/Backgrounds/tower_dungeon_background.png"),
				FLinearColor(1.00f, 0.98f, 0.94f, 1.0f),
				FLinearColor(0.15f, 0.12f, 0.10f, 1.0f),
				FLinearColor(0.92f, 0.84f, 0.68f, 1.0f)
			};
		}
	}

	static const FSlateBrush* GetTowerMinimapBackgroundBrush(const T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme)
	{
		static TMap<int32, TSharedPtr<FSlateBrush>> Brushes;

		const int32 Key = static_cast<int32>(Theme);
		if (const TSharedPtr<FSlateBrush>* Existing = Brushes.Find(Key))
		{
			return (Existing->IsValid() && (*Existing)->GetResourceObject()) ? Existing->Get() : nullptr;
		}

		const FT66TowerMinimapArtStyle Style = GetTowerMinimapArtStyle(Theme);
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = FVector2D(1024.0f, 1024.0f);
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(LoadRuntimeHudFileTexture(Style.RelativePath, TextureFilter::TF_Trilinear));
		Brushes.Add(Key, Brush);

		return Brush->GetResourceObject() ? Brush.Get() : nullptr;
	}

	static float GetSquaredDistanceToBox2D(const FVector2D& Point, const FBox2D& Box)
	{
		const float Dx = (Point.X < Box.Min.X)
			? (Box.Min.X - Point.X)
			: ((Point.X > Box.Max.X) ? (Point.X - Box.Max.X) : 0.0f);
		const float Dy = (Point.Y < Box.Min.Y)
			? (Box.Min.Y - Point.Y)
			: ((Point.Y > Box.Max.Y) ? (Point.Y - Box.Max.Y) : 0.0f);
		return (Dx * Dx) + (Dy * Dy);
	}

	static constexpr float GT66BottomLeftHudScale = 0.70f;
	static constexpr float GT66BottomLeftPortraitPanelSize = 152.f;
	static constexpr float GT66BottomLeftSidePanelWidth = 132.f;
	static constexpr float GT66BottomLeftIdolSlotSize = 58.f;
	static constexpr float GT66BottomLeftIdolSlotPad = 1.f;
	static constexpr float GT66BottomLeftAbilityGap = 4.f;
	static constexpr float GT66BottomLeftLevelBadgeSize = 42.f;
	static constexpr float GT66BottomLeftAbilityColumnHeight = GT66BottomLeftPortraitPanelSize;
	static constexpr float GT66BottomLeftAbilityBoxSize = (GT66BottomLeftAbilityColumnHeight - GT66BottomLeftAbilityGap) * 0.5f;
	static constexpr float GT66BottomLeftSectionOuterPadding = 2.f;
	static constexpr float GT66DisplayedHeartColumnGap = 0.f;
	static constexpr float GT66DisplayedHeartRowGap = 0.f;
	static constexpr float GT66DisplayedHeartAreaWidth = GT66BottomLeftPortraitPanelSize - (GT66BottomLeftSectionOuterPadding * 2.f);
	static constexpr float GT66DisplayedHeartAreaHeight = GT66BottomLeftAbilityBoxSize - (GT66BottomLeftSectionOuterPadding * 2.f);
	static constexpr float GT66DisplayedHeartWidth = (GT66DisplayedHeartAreaWidth - (GT66DisplayedHeartColumnGap * 4.f)) / 5.f;
	static constexpr float GT66DisplayedHeartHeight = (GT66DisplayedHeartAreaHeight - GT66DisplayedHeartRowGap) * 0.5f;
	static constexpr int32 GT66DisplayedHeartCount = UT66RunStateSubsystem::DefaultMaxHearts * 2;
	static constexpr float GT66BottomLeftBlackPanelChrome = 10.f;
	static constexpr float GT66BottomLeftMainPlateWidth = GT66BottomLeftPortraitPanelSize + GT66BottomLeftAbilityBoxSize + GT66BottomLeftBlackPanelChrome;
	static constexpr float GT66BottomLeftMainPlateHeight = GT66BottomLeftPortraitPanelSize + GT66BottomLeftAbilityBoxSize + GT66BottomLeftBlackPanelChrome;
	static constexpr float GT66BottomLeftIdolPlateWidth = GT66BottomLeftSidePanelWidth + GT66BottomLeftBlackPanelChrome;
	static constexpr float GT66BottomLeftCombinedPlateWidth = GT66BottomLeftIdolPlateWidth + GT66BottomLeftMainPlateWidth;
	static constexpr float GT66VisibleBottomLeftCombinedPlateWidth = GT66BottomLeftCombinedPlateWidth * GT66BottomLeftHudScale;
	static constexpr float GT66VisibleBottomLeftMainPlateHeight = GT66BottomLeftMainPlateHeight * GT66BottomLeftHudScale;
	static constexpr float GT66MediaPanelW = GT66VisibleBottomLeftCombinedPlateWidth;
	static constexpr float GT66MediaPanelAspect = 600.f / 330.f;
	static constexpr float GT66MediaPanelH = GT66MediaPanelW * GT66MediaPanelAspect;
	static constexpr float GT66MediaBottomOffset = 12.f + GT66VisibleBottomLeftMainPlateHeight + 4.f;

	// DPS meter reuses the same tier ladder as hearts/skulls, with broad thresholds so the color
	// changes are meaningful across early and late runs.
	static int32 GetDPSTierIndex(int32 DPS)
	{
		if (DPS <= 0) return -1;
		if (DPS >= 2000) return 5;
		if (DPS >= 1000) return 4;
		if (DPS >= 500) return 3;
		if (DPS >= 250) return 2;
		if (DPS >= 100) return 1;
		return 0;
	}

	static FLinearColor GetDPSColor(int32 DPS)
	{
		const int32 Tier = GetDPSTierIndex(DPS);
		return Tier >= 0 ? FT66RarityUtil::GetTierColor(Tier) : FT66Style::Tokens::TextMuted;
	}

	static const TCHAR* GetStatGradeLabel(int32 Value)
	{
		const int32 ClampedValue = FMath::Max(0, Value);
		if (ClampedValue <= 3)  return TEXT("F");
		if (ClampedValue <= 7)  return TEXT("D-");
		if (ClampedValue <= 11) return TEXT("D");
		if (ClampedValue <= 15) return TEXT("D+");
		if (ClampedValue <= 19) return TEXT("C-");
		if (ClampedValue <= 23) return TEXT("C");
		if (ClampedValue <= 27) return TEXT("C+");
		if (ClampedValue <= 31) return TEXT("B-");
		if (ClampedValue <= 35) return TEXT("B");
		if (ClampedValue <= 39) return TEXT("B+");
		if (ClampedValue <= 43) return TEXT("A-");
		if (ClampedValue <= 47) return TEXT("A");
		if (ClampedValue <= 51) return TEXT("A+");
		if (ClampedValue <= 55) return TEXT("S");
		if (ClampedValue <= 59) return TEXT("S+");
		if (ClampedValue <= 63) return TEXT("S++");
		return TEXT("S+++");
	}

	static FText MakeGradeStatText(const TCHAR* Label, int32 Value)
	{
		return FText::FromString(FString::Printf(TEXT("%s: %s"), Label, GetStatGradeLabel(Value)));
	}

	static bool IsKeyboardMouseKey(const FKey& Key)
	{
		if (!Key.IsValid())
		{
			return false;
		}

		if (Key.IsMouseButton())
		{
			return true;
		}

		return !Key.IsGamepadKey() && !Key.IsTouch();
	}

	static FText GetActionKeyText(FName ActionName)
	{
		if (const UInputSettings* Settings = UInputSettings::GetInputSettings())
		{
			for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
			{
				if (Mapping.ActionName == ActionName && IsKeyboardMouseKey(Mapping.Key))
				{
					return Mapping.Key.GetDisplayName();
				}
			}

			for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
			{
				if (Mapping.ActionName == ActionName)
				{
					return Mapping.Key.GetDisplayName();
				}
			}
		}

		return FText::GetEmpty();
	}

	static FText BuildSkipCountdownText(float RemainingSeconds, FName ActionName = NAME_None)
	{
		const FText SecondsText = FText::FromString(FString::Printf(TEXT("%.1f"), FMath::Max(0.f, RemainingSeconds)));
		if (!ActionName.IsNone())
		{
			const FText KeyText = GetActionKeyText(ActionName);
			if (!KeyText.IsEmpty())
			{
				return FText::Format(
					NSLOCTEXT("T66.Presentation", "SkipKeyCountdown", "Skip: {0} ({1}s)"),
					KeyText,
					SecondsText);
			}
		}

		return FText::Format(
			NSLOCTEXT("T66.Presentation", "SkipCountdown", "Skip {0}s"),
			SecondsText);
	}

	static int32 T66RarityToStageIndex(const ET66Rarity Rarity)
	{
		switch (Rarity)
		{
		case ET66Rarity::Red: return 1;
		case ET66Rarity::Yellow: return 2;
		case ET66Rarity::White: return 3;
		case ET66Rarity::Black:
		default: return 0;
		}
	}

	static ET66Rarity T66StageIndexToRarity(const int32 StageIndex)
	{
		switch (StageIndex)
		{
		case 1: return ET66Rarity::Red;
		case 2: return ET66Rarity::Yellow;
		case 3: return ET66Rarity::White;
		case 0:
		default: return ET66Rarity::Black;
		}
	}

	static float GetAchievementProgress01(const FAchievementData& Achievement)
	{
		return Achievement.RequirementCount > 0
			? static_cast<float>(FMath::Clamp(Achievement.CurrentProgress, 0, Achievement.RequirementCount))
				/ static_cast<float>(Achievement.RequirementCount)
			: (Achievement.bIsUnlocked ? 1.0f : 0.0f);
	}

	static int32 GetAchievementRemainingCount(const FAchievementData& Achievement)
	{
		return FMath::Max(0, Achievement.RequirementCount - FMath::Clamp(Achievement.CurrentProgress, 0, Achievement.RequirementCount));
	}

	static int32 GetAchievementOrderKey(const FName AchievementID)
	{
		int32 NumericPart = 0;
		bool bHasDigit = false;
		for (const TCHAR Character : AchievementID.ToString())
		{
			if (FChar::IsDigit(Character))
			{
				NumericPart = (NumericPart * 10) + (Character - TEXT('0'));
				bHasDigit = true;
			}
		}

		return bHasDigit ? NumericPart : MAX_int32;
	}

	/** Creates a custom tooltip widget (background + text) for HUD item/idol hover. Returns null if InText is empty. */
	static TSharedPtr<IToolTip> CreateCustomTooltip(const FText& InText)
	{
		if (InText.IsEmpty()) return nullptr;
		const ISlateStyle& Style = FT66Style::Get();
		const FTextBlockStyle& TextBody = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");
		return SNew(SToolTip)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
				.BorderBackgroundColor(FT66Style::Tokens::Bg)
				.Padding(FMargin(8.f, 6.f))
				[
					SNew(STextBlock)
					.Text(InText)
					.TextStyle(&TextBody)
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.AutoWrapText(true)
					.WrapTextAt(280.f)
				]
			];
	}

	/** Creates a rich tooltip (title + description) for stat/ability hover. */
	static TSharedPtr<IToolTip> CreateRichTooltip(const FText& Title, const FText& Description)
	{
		if (Title.IsEmpty() && Description.IsEmpty()) return nullptr;
		return SNew(SToolTip)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
				.BorderBackgroundColor(FT66Style::Tokens::Bg)
				.Padding(FMargin(10.f, 8.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[
						SNew(STextBlock)
						.Text(Title)
						.Font(FT66Style::Tokens::FontBold(14))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(Description)
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
						.WrapTextAt(280.f)
					]
				]
			];
	}
}

class ST66RingWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(ST66RingWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		Percent01 = 0.f;
		Thickness = 4.f;
	}

	void SetPercent(float InPercent01)
	{
		Percent01 = FMath::Clamp(InPercent01, 0.f, 1.f);
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(52.f, 52.f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y));
		const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);
		const float Radius = (MinDim * 0.5f) - Thickness;

		const int32 NumSeg = 64;
		const float StartAngle = -PI * 0.5f;

		auto MakeCirclePoints = [&](int32 SegCount) -> TArray<FVector2D>
		{
			TArray<FVector2D> Pts;
			Pts.Reserve(SegCount + 1);
			for (int32 i = 0; i <= SegCount; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(NumSeg);
				const float A = StartAngle + (2.f * PI * T);
				Pts.Add(Center + FVector2D(FMath::Cos(A) * Radius, FMath::Sin(A) * Radius));
			}
			return Pts;
		};

		// Solid black filled circle background (darker and more solid).
		{
			const float FillRadius = Radius * 0.5f;
			TArray<FVector2D> FillPts;
			FillPts.Reserve(NumSeg + 1);
			for (int32 i = 0; i <= NumSeg; ++i)
			{
				const float A = (2.f * PI * static_cast<float>(i)) / static_cast<float>(NumSeg);
				FillPts.Add(Center + FVector2D(FMath::Cos(A) * FillRadius, FMath::Sin(A) * FillRadius));
			}
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				FillPts,
				ESlateDrawEffect::None,
				FLinearColor(0.02f, 0.02f, 0.02f, 1.f),
				true,
				Radius
			);
		}

		// Background ring (dark solid outline).
		{
			const TArray<FVector2D> Pts = MakeCirclePoints(NumSeg);
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				Pts,
				ESlateDrawEffect::None,
				FLinearColor(0.05f, 0.05f, 0.05f, 1.f),
				true,
				Thickness
			);
		}

		// Progress ring (gold / amber).
		const int32 ProgSeg = FMath::Clamp(FMath::RoundToInt(Percent01 * static_cast<float>(NumSeg)), 0, NumSeg);
		if (ProgSeg > 0)
		{
			TArray<FVector2D> Pts;
			Pts.Reserve(ProgSeg + 1);
			for (int32 i = 0; i <= ProgSeg; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(NumSeg);
				const float A = StartAngle + (2.f * PI * T);
				Pts.Add(Center + FVector2D(FMath::Cos(A) * Radius, FMath::Sin(A) * Radius));
			}
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 2,
				AllottedGeometry.ToPaintGeometry(),
				Pts,
				ESlateDrawEffect::None,
				FLinearColor(0.85f, 0.65f, 0.10f, 1.f),
				false,
				Thickness
			);
		}

		return LayerId + 3;
	}

private:
	float Percent01 = 0.f;
	float Thickness = 3.f;
};

class ST66DotWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(ST66DotWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		DotColor = FLinearColor::White;
	}

	void SetDotColor(const FLinearColor& InColor)
	{
		DotColor = InColor;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(12.f, 12.f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y));
		const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);
		const float Radius = (MinDim * 0.5f) - 0.5f;

		static constexpr int32 NumSeg = 24;
		TArray<FVector2D> Pts;
		Pts.Reserve(NumSeg + 1);
		for (int32 i = 0; i <= NumSeg; ++i)
		{
			const float T = static_cast<float>(i) / static_cast<float>(NumSeg);
			const float A = 2.f * PI * T;
			Pts.Add(Center + FVector2D(FMath::Cos(A) * Radius, FMath::Sin(A) * Radius));
		}

		// UE5.7 Slate doesn't expose a convex fill helper, so we draw an extremely thick stroked circle.
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			Pts,
			ESlateDrawEffect::None,
			DotColor,
			true,
			MinDim
		);

		return LayerId + 1;
	}

private:
	FLinearColor DotColor = FLinearColor::White;
};

class ST66CrosshairWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(ST66CrosshairWidget) {}
		SLATE_ARGUMENT(bool, Locked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		bLocked = InArgs._Locked;
	}

	void SetLocked(const bool bInLocked)
	{
		if (bLocked == bInLocked)
		{
			return;
		}

		bLocked = bInLocked;
		Invalidate(EInvalidateWidget::Paint);
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(28.f, 28.f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);
		const FLinearColor CrosshairColor = bLocked
			? FLinearColor(1.f, 0.28f, 0.18f, 0.98f)
			: FLinearColor(0.95f, 0.95f, 1.f, 0.85f);

		const float Gap = 4.f;
		const float Len = 8.f;
		const float Thick = 2.f;

		auto Line = [&](const FVector2D& A, const FVector2D& B)
		{
			TArray<FVector2D> Pts;
			Pts.Reserve(2);
			Pts.Add(A);
			Pts.Add(B);
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				Pts,
				ESlateDrawEffect::None,
				CrosshairColor,
				false,
				Thick
			);
		};

		if (bLocked)
		{
			const float TimeSeconds = static_cast<float>(FSlateApplication::Get().GetCurrentTime());
			const float Pulse = 1.f + (0.08f * (0.5f + (0.5f * FMath::Sin(TimeSeconds * 8.f))));
			const float RingRadius = 10.f * Pulse;

			static constexpr int32 NumSeg = 24;
			TArray<FVector2D> RingPoints;
			RingPoints.Reserve(NumSeg + 1);
			for (int32 SegmentIndex = 0; SegmentIndex <= NumSeg; ++SegmentIndex)
			{
				const float T = static_cast<float>(SegmentIndex) / static_cast<float>(NumSeg);
				const float Angle = 2.f * PI * T;
				RingPoints.Add(Center + FVector2D(FMath::Cos(Angle) * RingRadius, FMath::Sin(Angle) * RingRadius));
			}

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				RingPoints,
				ESlateDrawEffect::None,
				FLinearColor(0.f, 0.f, 0.f, 0.55f),
				true,
				4.f);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				RingPoints,
				ESlateDrawEffect::None,
				CrosshairColor,
				true,
				2.5f);
		}

		// Left / Right / Up / Down ticks
		Line(Center + FVector2D(-(Gap + Len), 0.f), Center + FVector2D(-Gap, 0.f));
		Line(Center + FVector2D(Gap, 0.f), Center + FVector2D(Gap + Len, 0.f));
		Line(Center + FVector2D(0.f, -(Gap + Len)), Center + FVector2D(0.f, -Gap));
		Line(Center + FVector2D(0.f, Gap), Center + FVector2D(0.f, Gap + Len));

		// Tiny center dot.
		{
			static constexpr int32 NumSeg = 16;
			const float R = 1.5f;
			TArray<FVector2D> Pts;
			Pts.Reserve(NumSeg + 1);
			for (int32 i = 0; i <= NumSeg; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(NumSeg);
				const float A = 2.f * PI * T;
				Pts.Add(Center + FVector2D(FMath::Cos(A) * R, FMath::Sin(A) * R));
			}
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				Pts,
				ESlateDrawEffect::None,
				CrosshairColor,
				true,
				4.f
			);
		}

		return LayerId + (bLocked ? 2 : 1);
	}

private:
	bool bLocked = false;
};

class ST66ScopedSniperWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(ST66ScopedSniperWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(FT66Style::Tokens::ReferenceLayoutWidth, FT66Style::Tokens::ReferenceLayoutHeight);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);
		const float ScopeDiameter = FMath::Min(Size.Y * 0.82f, Size.X * 0.62f);
		const float ScopeRadius = ScopeDiameter * 0.5f;
		const FVector2D ScopeTopLeft = Center - FVector2D(ScopeRadius, ScopeRadius);
		const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

		auto DrawSolidBox = [&](const FVector2D& Pos, const FVector2D& BoxSize, const FLinearColor& Color)
		{
			if (BoxSize.X <= 0.f || BoxSize.Y <= 0.f) return;
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(FVector2f(BoxSize), FSlateLayoutTransform(FVector2f(Pos))),
				WhiteBrush,
				ESlateDrawEffect::None,
				Color);
		};

		// Hard matte around the scope window. This keeps the focused, CS-style tunneled view
		// without requiring a dedicated scope texture asset.
		DrawSolidBox(FVector2D(0.f, 0.f), FVector2D(FMath::Max(0.f, ScopeTopLeft.X), Size.Y), FLinearColor::Black);
		DrawSolidBox(FVector2D(ScopeTopLeft.X + ScopeDiameter, 0.f), FVector2D(FMath::Max(0.f, Size.X - (ScopeTopLeft.X + ScopeDiameter)), Size.Y), FLinearColor::Black);
		DrawSolidBox(FVector2D(ScopeTopLeft.X, 0.f), FVector2D(ScopeDiameter, FMath::Max(0.f, ScopeTopLeft.Y)), FLinearColor::Black);
		DrawSolidBox(FVector2D(ScopeTopLeft.X, ScopeTopLeft.Y + ScopeDiameter), FVector2D(ScopeDiameter, FMath::Max(0.f, Size.Y - (ScopeTopLeft.Y + ScopeDiameter))), FLinearColor::Black);

		auto DrawLine = [&](const FVector2D& A, const FVector2D& B, const FLinearColor& Color, const float Thickness)
		{
			TArray<FVector2D> Points;
			Points.Add(A);
			Points.Add(B);
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				Points,
				ESlateDrawEffect::None,
				Color,
				false,
				Thickness);
		};

		// Scope ring.
		{
			static constexpr int32 NumSeg = 72;
			TArray<FVector2D> RingPoints;
			RingPoints.Reserve(NumSeg + 1);
			for (int32 Index = 0; Index <= NumSeg; ++Index)
			{
				const float T = static_cast<float>(Index) / static_cast<float>(NumSeg);
				const float Angle = 2.f * PI * T;
				RingPoints.Add(Center + FVector2D(FMath::Cos(Angle) * ScopeRadius, FMath::Sin(Angle) * ScopeRadius));
			}

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				RingPoints,
				ESlateDrawEffect::None,
				FLinearColor(0.95f, 0.95f, 1.f, 0.95f),
				true,
				2.f);
		}

		// Thin reticle lines like a CS sniper scope.
		const float LineExtent = ScopeRadius - 18.f;
		DrawLine(Center + FVector2D(-LineExtent, 0.f), Center + FVector2D(LineExtent, 0.f), FLinearColor(0.95f, 0.95f, 1.f, 0.95f), 1.5f);
		DrawLine(Center + FVector2D(0.f, -LineExtent), Center + FVector2D(0.f, LineExtent), FLinearColor(0.95f, 0.95f, 1.f, 0.95f), 1.5f);

		return LayerId + 2;
	}
};

struct FT66MapMarker
{
	FVector2D WorldXY = FVector2D::ZeroVector;
	FLinearColor Color = FLinearColor::White;
	FText Label = FText::GetEmpty();
	ET66MapMarkerVisual Visual = ET66MapMarkerVisual::Dot;
	const FSlateBrush* IconBrush = nullptr;
	FVector2D DrawSize = FVector2D(10.f, 10.f);
};

class ST66WorldMapWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(ST66WorldMapWidget) {}
		SLATE_ARGUMENT(bool, bMinimap)
		SLATE_ARGUMENT(bool, bShowLabels)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		bMinimap = InArgs._bMinimap;
		bShowLabels = InArgs._bShowLabels;

		// 100k map: half-extent 50000
		FullWorldMin = FVector2D(-50000.f, -50000.f);
		FullWorldMax = FVector2D(50000.f, 50000.f);
		MinimapHalfExtent = 2500.f;

		// Safety: never draw markers outside the widget bounds.
		SetClipping(EWidgetClipping::ClipToBounds);
	}

	void SetSnapshot(const FVector2D& InPlayerWorldXY, const TArray<FT66MapMarker>& InMarkers, const FSlateBrush* InPlayerBrush = nullptr)
	{
		PlayerWorldXY = InPlayerWorldXY;
		Markers = InMarkers;
		PlayerBrush = InPlayerBrush;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetFullWorldBounds(const FVector2D& InMin, const FVector2D& InMax)
	{
		FullWorldMin = InMin;
		FullWorldMax = InMax;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetMinimapHalfExtent(float InHalfExtent)
	{
		MinimapHalfExtent = FMath::Max(250.f, InHalfExtent);
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetLockFullMapToBounds(bool bInLockFullMapToBounds)
	{
		bLockFullMapToBounds = bInLockFullMapToBounds;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetRevealMask(bool bInUseRevealMask, const TArray<FVector2D>& InRevealWorldPoints, float InRevealWorldRadius)
	{
		bUseRevealMask = bInUseRevealMask;
		RevealWorldPoints = InRevealWorldPoints;
		RevealWorldRadius = FMath::Max(InRevealWorldRadius, 0.0f);
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetTowerPolygon(const TArray<FVector2D>& InTowerPolygonWorldVertices)
	{
		TowerPolygonWorldVertices = InTowerPolygonWorldVertices;
		bHasTowerPolygon = TowerPolygonWorldVertices.Num() >= 3;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetTowerHole(bool bInHasTowerHole, const FVector2D& InTowerHoleCenter = FVector2D::ZeroVector, const FVector2D& InTowerHoleHalfExtents = FVector2D::ZeroVector)
	{
		bHasTowerHole = bInHasTowerHole;
		TowerHoleCenter = InTowerHoleCenter;
		TowerHoleHalfExtents = InTowerHoleHalfExtents;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetThemedFloorArt(
		const FSlateBrush* InBackgroundBrush,
		const TArray<FBox2D>& InTowerMazeWallBoxes,
		const FLinearColor& InMapArtTint,
		const FLinearColor& InWallFillColor,
		const FLinearColor& InWallStrokeColor)
	{
		BackgroundBrush = InBackgroundBrush;
		TowerMazeWallBoxes = InTowerMazeWallBoxes;
		MapArtTint = InMapArtTint;
		MapWallFillColor = InWallFillColor;
		MapWallStrokeColor = InWallStrokeColor;
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void SetPlayerDirectionWorldXY(const FVector2D& InPlayerDirectionWorldXY)
	{
		PlayerDirectionWorldXY = InPlayerDirectionWorldXY.IsNearlyZero()
			? FVector2D(1.0f, 0.0f)
			: InPlayerDirectionWorldXY.GetSafeNormal();
		Invalidate(EInvalidateWidgetReason::Paint);
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return bMinimap ? FVector2D(228.f, 228.f) : FVector2D(1024.f, 640.f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		if (Size.X <= 1.f || Size.Y <= 1.f)
		{
			return LayerId;
		}

		// World bounds for projection.
		FVector2D WorldMin = FullWorldMin;
		FVector2D WorldMax = FullWorldMax;
		if (bMinimap)
		{
			if (bLockFullMapToBounds)
			{
				WorldMin = FullWorldMin;
				WorldMax = FullWorldMax;
			}
			else
			{
				WorldMin = PlayerWorldXY - FVector2D(MinimapHalfExtent, MinimapHalfExtent);
				WorldMax = PlayerWorldXY + FVector2D(MinimapHalfExtent, MinimapHalfExtent);
			}
		}
		else if (bLockFullMapToBounds)
		{
			WorldMin = FullWorldMin;
			WorldMax = FullWorldMax;
		}
		else
		{
			// Full map: keep full-world scale, but center the view on the player.
			const FVector2D FullSpan = FullWorldMax - FullWorldMin;
			const FVector2D Half = FullSpan * 0.5f;
			WorldMin = PlayerWorldXY - Half;
			WorldMax = PlayerWorldXY + Half;
		}
		const FVector2D WorldSpan = WorldMax - WorldMin;
		if (WorldSpan.X <= 1.f || WorldSpan.Y <= 1.f)
		{
			return LayerId;
		}

		auto WorldToLocal = [&](const FVector2D& W) -> FVector2D
		{
			const float NX = (W.X - WorldMin.X) / WorldSpan.X;
			const float NY = (W.Y - WorldMin.Y) / WorldSpan.Y;
			// Y up in world -> map up on screen (invert Y because Slate Y grows down).
			return FVector2D(NX * Size.X, (1.f - NY) * Size.Y);
		};

		auto ToPaintGeo = [&](const FVector2D& Pos, const FVector2D& LocalSize) -> FPaintGeometry
		{
			return AllottedGeometry.ToPaintGeometry(
				FVector2f(static_cast<float>(LocalSize.X), static_cast<float>(LocalSize.Y)),
				FSlateLayoutTransform(FVector2f(static_cast<float>(Pos.X), static_cast<float>(Pos.Y))));
		};

		const bool bUsingMapArt = BackgroundBrush && BackgroundBrush->GetResourceObject();
		const FLinearColor MapBackgroundColor = (bUseRevealMask || bUsingMapArt)
			? FLinearColor::Black
			: (bMinimap ? FT66Style::MinimapBackground() : FT66Style::Background());
		const FLinearColor MapTerrainColor = bUseRevealMask
			? WithAlpha(FT66Style::MinimapTerrain(), bMinimap ? 0.08f : 0.06f)
			: WithAlpha(FT66Style::MinimapTerrain(), bMinimap ? 0.42f : 0.22f);
		const FLinearColor RevealedTerrainColor = WithAlpha(FT66Style::MinimapTerrain() * FLinearColor(1.10f, 1.02f, 0.98f, 1.0f), bMinimap ? 0.72f : 0.42f);
		const FLinearColor GridColor = bUseRevealMask
			? WithAlpha(FT66Style::MinimapGrid(), bMinimap ? 0.18f : 0.12f)
			: FT66Style::MinimapGrid();
		const FLinearColor OutlineColor = WithAlpha(FT66Style::Border(), 0.88f);

		// Solid dark background fill.
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			FCoreStyle::Get().GetBrush("WhiteBrush"),
			ESlateDrawEffect::None,
			MapBackgroundColor
		);

		auto DrawTowerPolygonFill = [&](const int32 DrawLayerId, const FLinearColor& FillColor)
		{
			if (!bHasTowerPolygon || TowerPolygonWorldVertices.Num() < 3)
			{
				return;
			}

			float PolygonMinY = TNumericLimits<float>::Max();
			float PolygonMaxY = TNumericLimits<float>::Lowest();
			for (const FVector2D& Vertex : TowerPolygonWorldVertices)
			{
				PolygonMinY = FMath::Min(PolygonMinY, Vertex.Y);
				PolygonMaxY = FMath::Max(PolygonMaxY, Vertex.Y);
			}

			static constexpr float PolygonBandHeightWorld = 320.0f;
			for (float BandMinY = PolygonMinY; BandMinY < PolygonMaxY - 1.0f; BandMinY += PolygonBandHeightWorld)
			{
				const float BandMaxY = FMath::Min(BandMinY + PolygonBandHeightWorld, PolygonMaxY);
				float BandMinX = 0.0f;
				float BandMaxX = 0.0f;
				if (!T66TowerMapTerrain::TryGetPolygonBandXRange(TowerPolygonWorldVertices, BandMinY, BandMaxY, BandMinX, BandMaxX))
				{
					continue;
				}

				const FVector2D A = WorldToLocal(FVector2D(BandMinX, BandMaxY));
				const FVector2D B = WorldToLocal(FVector2D(BandMaxX, BandMinY));
				const FVector2D TL(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y));
				const FVector2D BR(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y));
				const FVector2D BoxSize = BR - TL;
				if (BoxSize.X <= 1.0f || BoxSize.Y <= 1.0f)
				{
					continue;
				}

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					DrawLayerId,
					ToPaintGeo(TL, BoxSize),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					FillColor);
			}
		};

		auto DrawTowerPolygonOutline = [&](const int32 DrawLayerId, const FLinearColor& StrokeColor, const float Thickness)
		{
			if (!bHasTowerPolygon || TowerPolygonWorldVertices.Num() < 3)
			{
				return;
			}

			TArray<FVector2D> Outline;
			Outline.Reserve(TowerPolygonWorldVertices.Num() + 1);
			for (const FVector2D& Vertex : TowerPolygonWorldVertices)
			{
				Outline.Add(WorldToLocal(Vertex));
			}
			Outline.Add(WorldToLocal(TowerPolygonWorldVertices[0]));

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				DrawLayerId,
				AllottedGeometry.ToPaintGeometry(),
				Outline,
				ESlateDrawEffect::None,
				StrokeColor,
				true,
				Thickness);
		};

		auto DrawBackgroundTextureWorldRect = [&](const int32 DrawLayerId, const FVector2D& RectWorldMin, const FVector2D& RectWorldMax, const float Alpha)
		{
			if (!bUsingMapArt)
			{
				return;
			}

			const FVector2D BackgroundWorldMin = FullWorldMin;
			const FVector2D BackgroundWorldMax = FullWorldMax;
			const FVector2D BackgroundWorldSpan = BackgroundWorldMax - BackgroundWorldMin;
			if (BackgroundWorldSpan.X <= 1.0f || BackgroundWorldSpan.Y <= 1.0f)
			{
				return;
			}

			const FVector2D ClampedWorldMin(
				FMath::Clamp(RectWorldMin.X, BackgroundWorldMin.X, BackgroundWorldMax.X),
				FMath::Clamp(RectWorldMin.Y, BackgroundWorldMin.Y, BackgroundWorldMax.Y));
			const FVector2D ClampedWorldMax(
				FMath::Clamp(RectWorldMax.X, BackgroundWorldMin.X, BackgroundWorldMax.X),
				FMath::Clamp(RectWorldMax.Y, BackgroundWorldMin.Y, BackgroundWorldMax.Y));
			if (ClampedWorldMax.X <= ClampedWorldMin.X || ClampedWorldMax.Y <= ClampedWorldMin.Y)
			{
				return;
			}

			const FVector2D A = WorldToLocal(FVector2D(ClampedWorldMin.X, ClampedWorldMax.Y));
			const FVector2D B = WorldToLocal(FVector2D(ClampedWorldMax.X, ClampedWorldMin.Y));
			const FVector2D TL(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y));
			const FVector2D BR(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y));
			const FVector2D BoxSize = BR - TL;
			if (BoxSize.X <= 1.0f || BoxSize.Y <= 1.0f)
			{
				return;
			}

			const float U0 = (ClampedWorldMin.X - BackgroundWorldMin.X) / BackgroundWorldSpan.X;
			const float U1 = (ClampedWorldMax.X - BackgroundWorldMin.X) / BackgroundWorldSpan.X;
			const float V0 = 1.0f - ((ClampedWorldMax.Y - BackgroundWorldMin.Y) / BackgroundWorldSpan.Y);
			const float V1 = 1.0f - ((ClampedWorldMin.Y - BackgroundWorldMin.Y) / BackgroundWorldSpan.Y);

			FSlateBrush LocalBrush = *BackgroundBrush;
			LocalBrush.TintColor = FSlateColor(FLinearColor(MapArtTint.R, MapArtTint.G, MapArtTint.B, MapArtTint.A * Alpha));
			LocalBrush.SetUVRegion(FBox2f(
				FVector2f(U0, V0),
				FVector2f(U1, V1)));

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				DrawLayerId,
				ToPaintGeo(TL, BoxSize),
				&LocalBrush,
				ESlateDrawEffect::None,
				FLinearColor::White);
		};

		auto DrawTowerPolygonTexture = [&](const int32 DrawLayerId, const float Alpha)
		{
			if (!bUsingMapArt || !bHasTowerPolygon || TowerPolygonWorldVertices.Num() < 3)
			{
				return;
			}

			float PolygonMinY = TNumericLimits<float>::Max();
			float PolygonMaxY = TNumericLimits<float>::Lowest();
			for (const FVector2D& Vertex : TowerPolygonWorldVertices)
			{
				PolygonMinY = FMath::Min(PolygonMinY, Vertex.Y);
				PolygonMaxY = FMath::Max(PolygonMaxY, Vertex.Y);
			}

			static constexpr float PolygonBandHeightWorld = 320.0f;
			for (float BandMinY = PolygonMinY; BandMinY < PolygonMaxY - 1.0f; BandMinY += PolygonBandHeightWorld)
			{
				const float BandMaxY = FMath::Min(BandMinY + PolygonBandHeightWorld, PolygonMaxY);
				float BandMinX = 0.0f;
				float BandMaxX = 0.0f;
				if (!T66TowerMapTerrain::TryGetPolygonBandXRange(TowerPolygonWorldVertices, BandMinY, BandMaxY, BandMinX, BandMaxX))
				{
					continue;
				}

				DrawBackgroundTextureWorldRect(
					DrawLayerId,
					FVector2D(BandMinX, BandMinY),
					FVector2D(BandMaxX, BandMaxY),
					Alpha);
			}
		};

		auto DrawRevealedMapArtCircle = [&](const int32 DrawLayerId, const FVector2D& RevealWorldPoint, const float RadiusWorld, const float Alpha)
		{
			if (!bUsingMapArt || RadiusWorld <= KINDA_SMALL_NUMBER)
			{
				return;
			}

			static constexpr int32 RevealBandCount = 12;
			for (int32 BandIndex = 0; BandIndex < RevealBandCount; ++BandIndex)
			{
				const float T0 = static_cast<float>(BandIndex) / static_cast<float>(RevealBandCount);
				const float T1 = static_cast<float>(BandIndex + 1) / static_cast<float>(RevealBandCount);
				const float LocalMinY = FMath::Lerp(-RadiusWorld, RadiusWorld, T0);
				const float LocalMaxY = FMath::Lerp(-RadiusWorld, RadiusWorld, T1);
				const float SampleY = (LocalMinY + LocalMaxY) * 0.5f;
				const float HalfWidth = FMath::Sqrt(FMath::Max(0.0f, (RadiusWorld * RadiusWorld) - (SampleY * SampleY)));
				float SegmentMinX = RevealWorldPoint.X - HalfWidth;
				float SegmentMaxX = RevealWorldPoint.X + HalfWidth;
				if (bHasTowerPolygon)
				{
					float PolygonMinX = 0.0f;
					float PolygonMaxX = 0.0f;
					if (!T66TowerMapTerrain::TryGetPolygonBandXRange(
						TowerPolygonWorldVertices,
						RevealWorldPoint.Y + LocalMinY,
						RevealWorldPoint.Y + LocalMaxY,
						PolygonMinX,
						PolygonMaxX))
					{
						continue;
					}

					SegmentMinX = FMath::Max(SegmentMinX, PolygonMinX);
					SegmentMaxX = FMath::Min(SegmentMaxX, PolygonMaxX);
					if (SegmentMaxX <= SegmentMinX)
					{
						continue;
					}
				}

				DrawBackgroundTextureWorldRect(
					DrawLayerId,
					FVector2D(SegmentMinX, RevealWorldPoint.Y + LocalMinY),
					FVector2D(SegmentMaxX, RevealWorldPoint.Y + LocalMaxY),
					Alpha);
			}
		};

		auto IsWorldBoxRevealed = [&](const FBox2D& WorldBox) -> bool
		{
			if (!bUseRevealMask || RevealWorldRadius <= KINDA_SMALL_NUMBER)
			{
				return true;
			}

			const float RevealRadiusSq = FMath::Square(RevealWorldRadius + 140.0f);
			for (const FVector2D& RevealWorldPoint : RevealWorldPoints)
			{
				if (GetSquaredDistanceToBox2D(RevealWorldPoint, WorldBox) <= RevealRadiusSq)
				{
					return true;
				}
			}

			return false;
		};

		auto DrawTowerMazeWalls = [&](const int32 DrawLayerId)
		{
			if (TowerMazeWallBoxes.Num() <= 0)
			{
				return;
			}

			const FLinearColor WallFill = WithAlpha(MapWallFillColor, bUseRevealMask ? 0.96f : 0.88f);
			const FLinearColor WallStroke = WithAlpha(MapWallStrokeColor, bUseRevealMask ? 0.92f : 0.82f);
			for (const FBox2D& WallBox : TowerMazeWallBoxes)
			{
				if (!IsWorldBoxRevealed(WallBox))
				{
					continue;
				}

				const FVector2D A = WorldToLocal(FVector2D(WallBox.Min.X, WallBox.Max.Y));
				const FVector2D B = WorldToLocal(FVector2D(WallBox.Max.X, WallBox.Min.Y));
				const FVector2D TL(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y));
				const FVector2D BR(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y));
				const FVector2D BoxSize = BR - TL;
				if (BoxSize.X <= 1.0f || BoxSize.Y <= 1.0f)
				{
					continue;
				}

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					DrawLayerId,
					ToPaintGeo(TL, BoxSize),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					WallFill);

				const TArray<FVector2D> Outline = {
					TL,
					FVector2D(BR.X, TL.Y),
					BR,
					FVector2D(TL.X, BR.Y),
					TL
				};
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					DrawLayerId + 1,
					AllottedGeometry.ToPaintGeometry(),
					Outline,
					ESlateDrawEffect::None,
					WallStroke,
					true,
					bMinimap ? 1.0f : 1.4f);
			}
		};

		// Terrain texture/fill. Tower floors can use dedicated map art that reveals out of black.
		{
			if (bUsingMapArt)
			{
				if (bUseRevealMask && RevealWorldRadius > KINDA_SMALL_NUMBER)
				{
					for (const FVector2D& RevealWorldPoint : RevealWorldPoints)
					{
						DrawRevealedMapArtCircle(LayerId + 1, RevealWorldPoint, RevealWorldRadius, bMinimap ? 0.98f : 0.94f);
					}
				}
				else if (bHasTowerPolygon)
				{
					DrawTowerPolygonTexture(LayerId + 1, bMinimap ? 0.98f : 0.94f);
				}
				else
				{
					DrawBackgroundTextureWorldRect(LayerId + 1, FullWorldMin, FullWorldMax, bMinimap ? 0.98f : 0.94f);
				}
			}
			else
			{
				if (bHasTowerPolygon && !bUseRevealMask)
				{
					DrawTowerPolygonFill(LayerId + 1, MapTerrainColor);
				}
				else if (!bUseRevealMask)
				{
					const FVector2D Inset(6.f, 6.f);
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						LayerId + 1,
						ToPaintGeo(Inset, Size - (Inset * 2.f)),
						FCoreStyle::Get().GetBrush("WhiteBrush"),
						ESlateDrawEffect::None,
						MapTerrainColor
					);
				}

				if (!bUseRevealMask && !bHasTowerPolygon)
				{
					const FLinearColor LaneColor = WithAlpha(FT66Style::MinimapTerrain() * FLinearColor(0.7f, 0.7f, 0.7f, 1.f), 0.75f);
					const TArray<FVector2D> LaneA = {
						FVector2D(Size.X * 0.08f, Size.Y * 0.78f),
						FVector2D(Size.X * 0.24f, Size.Y * 0.62f),
						FVector2D(Size.X * 0.40f, Size.Y * 0.46f),
						FVector2D(Size.X * 0.58f, Size.Y * 0.30f),
						FVector2D(Size.X * 0.84f, Size.Y * 0.12f)
					};
					const TArray<FVector2D> LaneB = {
						FVector2D(Size.X * 0.16f, Size.Y * 0.16f),
						FVector2D(Size.X * 0.34f, Size.Y * 0.30f),
						FVector2D(Size.X * 0.52f, Size.Y * 0.48f),
						FVector2D(Size.X * 0.72f, Size.Y * 0.68f),
						FVector2D(Size.X * 0.90f, Size.Y * 0.84f)
					};
					FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), LaneA, ESlateDrawEffect::None, LaneColor, true, bMinimap ? 10.f : 16.f);
					FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), LaneB, ESlateDrawEffect::None, WithAlpha(LaneColor, 0.50f), true, bMinimap ? 8.f : 12.f);
				}
			}
		}

		if (!bUsingMapArt && bUseRevealMask && RevealWorldRadius > KINDA_SMALL_NUMBER)
		{
			for (const FVector2D& RevealWorldPoint : RevealWorldPoints)
			{
				static constexpr int32 RevealBandCount = 12;
				for (int32 BandIndex = 0; BandIndex < RevealBandCount; ++BandIndex)
				{
					const float T0 = static_cast<float>(BandIndex) / static_cast<float>(RevealBandCount);
					const float T1 = static_cast<float>(BandIndex + 1) / static_cast<float>(RevealBandCount);
					const float LocalMinY = FMath::Lerp(-RevealWorldRadius, RevealWorldRadius, T0);
					const float LocalMaxY = FMath::Lerp(-RevealWorldRadius, RevealWorldRadius, T1);
					const float SampleY = (LocalMinY + LocalMaxY) * 0.5f;
					const float HalfWidth = FMath::Sqrt(FMath::Max(0.0f, (RevealWorldRadius * RevealWorldRadius) - (SampleY * SampleY)));
					float SegmentMinX = RevealWorldPoint.X - HalfWidth;
					float SegmentMaxX = RevealWorldPoint.X + HalfWidth;
					if (bHasTowerPolygon)
					{
						float PolygonMinX = 0.0f;
						float PolygonMaxX = 0.0f;
						if (!T66TowerMapTerrain::TryGetPolygonBandXRange(
							TowerPolygonWorldVertices,
							RevealWorldPoint.Y + LocalMinY,
							RevealWorldPoint.Y + LocalMaxY,
							PolygonMinX,
							PolygonMaxX))
						{
							continue;
						}

						SegmentMinX = FMath::Max(SegmentMinX, PolygonMinX);
						SegmentMaxX = FMath::Min(SegmentMaxX, PolygonMaxX);
						if (SegmentMaxX <= SegmentMinX)
						{
							continue;
						}
					}

					const FVector2D A = WorldToLocal(FVector2D(SegmentMinX, RevealWorldPoint.Y + LocalMaxY));
					const FVector2D B = WorldToLocal(FVector2D(SegmentMaxX, RevealWorldPoint.Y + LocalMinY));
					const FVector2D TL(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y));
					const FVector2D BR(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y));
					const FVector2D BoxSize = BR - TL;
					if (BoxSize.X <= 1.0f || BoxSize.Y <= 1.0f)
					{
						continue;
					}

					FSlateDrawElement::MakeBox(
						OutDrawElements,
						LayerId + 2,
						ToPaintGeo(TL, BoxSize),
						FCoreStyle::Get().GetBrush("WhiteBrush"),
						ESlateDrawEffect::None,
						RevealedTerrainColor);
				}
			}
		}

		DrawTowerMazeWalls(LayerId + 3);

		// Background grid (subtle).
		if (!bUsingMapArt && !bUseRevealMask)
		{
			static constexpr int32 GridLines = 6;
			for (int32 i = 1; i < GridLines; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(GridLines);
				const float X = Size.X * T;
				const float Y = Size.Y * T;
				const TArray<FVector2D> V = { FVector2D(X, 0.f), FVector2D(X, Size.Y) };
				const TArray<FVector2D> H = { FVector2D(0.f, Y), FVector2D(Size.X, Y) };
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(), V, ESlateDrawEffect::None, GridColor, true, 1.f);
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(), H, ESlateDrawEffect::None, GridColor, true, 1.f);
			}
		}

		// Border outline.
		{
			const TArray<FVector2D> Border = {
				FVector2D(0.f, 0.f), FVector2D(Size.X, 0.f),
				FVector2D(Size.X, Size.Y), FVector2D(0.f, Size.Y),
				FVector2D(0.f, 0.f) };
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 4, AllottedGeometry.ToPaintGeometry(),
				Border, ESlateDrawEffect::None, OutlineColor, true, 2.f);
			if (bHasTowerPolygon && !bUseRevealMask)
			{
				DrawTowerPolygonOutline(LayerId + 4, WithAlpha(OutlineColor, 0.78f), bMinimap ? 1.4f : 2.0f);
			}
		}

		// Areas (full map only): Start / Main / Boss.
		if (!bMinimap && !bUseRevealMask)
		{
			struct FArea
			{
				FText Label;
				FVector2D Center;
				FVector2D HalfExtents;
				FLinearColor Fill;
				FLinearColor Stroke;
			};

			const TArray<FArea> Areas = {
				{ NSLOCTEXT("T66.Map", "AreaStart", "START"), FVector2D(-10000.f, 0.f), FVector2D(3000.f, 3000.f), WithAlpha(FT66Style::MinimapFriendly(), 0.10f), WithAlpha(FT66Style::MinimapFriendly(), 0.28f) },
				{ NSLOCTEXT("T66.Map", "AreaMain",  "MAIN"),  FVector2D(0.f, 0.f),      FVector2D(10000.f, 10000.f), WithAlpha(FT66Style::MinimapNeutral(), 0.06f), WithAlpha(FT66Style::MinimapNeutral(), 0.18f) },
				{ NSLOCTEXT("T66.Map", "AreaBoss",  "BOSS"),  FVector2D(10000.f, 0.f),  FVector2D(3000.f, 3000.f), WithAlpha(FT66Style::MinimapEnemy(), 0.08f), WithAlpha(FT66Style::MinimapEnemy(), 0.25f) },
			};

			for (const FArea& A : Areas)
			{
				const FVector2D Min = WorldToLocal(A.Center - A.HalfExtents);
				const FVector2D Max = WorldToLocal(A.Center + A.HalfExtents);
				const FVector2D TL(FMath::Min(Min.X, Max.X), FMath::Min(Min.Y, Max.Y));
				const FVector2D BR(FMath::Max(Min.X, Max.X), FMath::Max(Min.Y, Max.Y));
				const FVector2D BoxSize = BR - TL;
				if (BoxSize.X <= 1.f || BoxSize.Y <= 1.f) continue;

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 2,
					ToPaintGeo(TL, BoxSize),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					A.Fill
				);

				const TArray<FVector2D> Outline = { TL, FVector2D(BR.X, TL.Y), BR, FVector2D(TL.X, BR.Y), TL };
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(), Outline, ESlateDrawEffect::None, A.Stroke, true, 1.0f);

				const FVector2D CenterLocal = WorldToLocal(A.Center);
				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId + 4,
					ToPaintGeo(CenterLocal + FVector2D(-50.f, -10.f), FVector2D(100.f, 20.f)),
					A.Label,
					FT66Style::Tokens::FontBold(14),
					ESlateDrawEffect::None,
					WithAlpha(FT66Style::TextMuted(), 0.80f)
				);
			}
		}

		if (bHasTowerHole)
		{
			const FBox2D HoleWorldBox(TowerHoleCenter - TowerHoleHalfExtents, TowerHoleCenter + TowerHoleHalfExtents);
			if (IsWorldBoxRevealed(HoleWorldBox))
			{
				const FVector2D HoleMin = WorldToLocal(TowerHoleCenter - TowerHoleHalfExtents);
				const FVector2D HoleMax = WorldToLocal(TowerHoleCenter + TowerHoleHalfExtents);
				const FVector2D TL(FMath::Min(HoleMin.X, HoleMax.X), FMath::Min(HoleMin.Y, HoleMax.Y));
				const FVector2D BR(FMath::Max(HoleMin.X, HoleMax.X), FMath::Max(HoleMin.Y, HoleMax.Y));
				const FVector2D HoleSize = BR - TL;
				if (HoleSize.X > 1.0f && HoleSize.Y > 1.0f)
				{
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						LayerId + 4,
						ToPaintGeo(TL, HoleSize),
						FCoreStyle::Get().GetBrush("WhiteBrush"),
						ESlateDrawEffect::None,
						WithAlpha(MapBackgroundColor, bUseRevealMask ? 0.95f : 0.82f));

					const TArray<FVector2D> HoleOutline = { TL, FVector2D(BR.X, TL.Y), BR, FVector2D(TL.X, BR.Y), TL };
					FSlateDrawElement::MakeLines(
						OutDrawElements,
						LayerId + 5,
						AllottedGeometry.ToPaintGeometry(),
						HoleOutline,
						ESlateDrawEffect::None,
						WithAlpha(OutlineColor, 0.72f),
						true,
						bMinimap ? 1.5f : 2.0f);
				}
			}
		}

		auto IsInsideBounds = [&](const FVector2D& P) -> bool
		{
			return P.X >= 0.f && P.X <= Size.X && P.Y >= 0.f && P.Y <= Size.Y;
		};

		auto ClampToBounds = [&](const FVector2D& P) -> FVector2D
		{
			return FVector2D(
				FMath::Clamp(P.X, 0.f, Size.X),
				FMath::Clamp(P.Y, 0.f, Size.Y));
		};

		// Player marker: directional arrow on the minimap, portrait/diamond fallback on the full map.
		{
			const FVector2D P = ClampToBounds(WorldToLocal(PlayerWorldXY));
			const FVector2D ScreenDirection = FVector2D(PlayerDirectionWorldXY.X, -PlayerDirectionWorldXY.Y).GetSafeNormal();
			const FVector2D ScreenPerp(-ScreenDirection.Y, ScreenDirection.X);
			if (bMinimap)
			{
				const float ArrowTipLength = 10.0f;
				const float ArrowBackLength = 5.0f;
				const float ArrowHalfWidth = 4.0f;
				const FVector2D Tip = P + (ScreenDirection * ArrowTipLength);
				const FVector2D Left = P - (ScreenDirection * ArrowBackLength) + (ScreenPerp * ArrowHalfWidth);
				const FVector2D Right = P - (ScreenDirection * ArrowBackLength) - (ScreenPerp * ArrowHalfWidth);
				const FVector2D Tail = P - (ScreenDirection * (ArrowBackLength + 4.0f));
				const TArray<FVector2D> ArrowOutline = {
					Tip,
					Left,
					Tail,
					Right,
					Tip
				};

				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId + 6,
					AllottedGeometry.ToPaintGeometry(),
					ArrowOutline,
					ESlateDrawEffect::None,
					FT66Style::PanelOuter(),
					true,
					4.0f);

				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId + 7,
					AllottedGeometry.ToPaintGeometry(),
					ArrowOutline,
					ESlateDrawEffect::None,
					FT66Style::MinimapFriendly(),
					true,
					2.6f);
			}
			else if (PlayerBrush && PlayerBrush->GetResourceObject())
			{
				const float MarkerSize = bMinimap ? 18.f : 24.f;
				const FVector2D TL = ClampToBounds(P - FVector2D(MarkerSize * 0.5f, MarkerSize * 0.5f));
				const FVector2D InnerTL = TL + FVector2D(2.f, 2.f);
				const FVector2D InnerSize = FVector2D(MarkerSize - 4.f, MarkerSize - 4.f);

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 5,
					ToPaintGeo(TL - FVector2D(1.f, 1.f), FVector2D(MarkerSize + 2.f, MarkerSize + 2.f)),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					FT66Style::PanelOuter());

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 6,
					ToPaintGeo(TL, FVector2D(MarkerSize, MarkerSize)),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					FT66Style::MinimapFriendly());

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 7,
					ToPaintGeo(InnerTL, InnerSize),
					PlayerBrush,
					ESlateDrawEffect::None,
					FLinearColor::White);
			}
			else
			{
				const float R = bMinimap ? 5.f : 6.f;
				const TArray<FVector2D> Diamond = {
					FVector2D(P.X, P.Y - R),
					FVector2D(P.X + R, P.Y),
					FVector2D(P.X, P.Y + R),
					FVector2D(P.X - R, P.Y),
					FVector2D(P.X, P.Y - R)
				};
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId + 5,
					AllottedGeometry.ToPaintGeometry(),
					Diamond,
					ESlateDrawEffect::None,
					FT66Style::MinimapFriendly(),
					true,
					2.5f
				);
			}
		}

		// Entity markers (NPCs, gates, enemies, etc.).
		for (const FT66MapMarker& M : Markers)
		{
			const FVector2D RawP = WorldToLocal(M.WorldXY);
			if (bMinimap && !IsInsideBounds(RawP))
			{
				continue;
			}

			const FVector2D P = ClampToBounds(RawP);

			if (M.Visual == ET66MapMarkerVisual::Icon && M.IconBrush && M.IconBrush->GetResourceObject())
			{
				const FVector2D IconSize = bMinimap ? M.DrawSize : (M.DrawSize + FVector2D(4.f, 4.f));
				const FVector2D TL = P - (IconSize * 0.5f);

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 8,
					ToPaintGeo(TL - FVector2D(1.f, 1.f), IconSize + FVector2D(2.f, 2.f)),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					FT66Style::PanelOuter());

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 9,
					ToPaintGeo(TL, IconSize),
					M.IconBrush,
					ESlateDrawEffect::None,
					GetMinimapMarkerTint(M.IconBrush, M.Color, bMinimap));
			}
			else
			{
				const float R = bMinimap ? 3.0f : 4.0f;
				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 8,
					ToPaintGeo(P - FVector2D(R + 1.f, R + 1.f), FVector2D((R + 1.f) * 2.f, (R + 1.f) * 2.f)),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					FT66Style::PanelOuter()
				);

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 9,
					ToPaintGeo(P - FVector2D(R, R), FVector2D(R * 2.f, R * 2.f)),
					FCoreStyle::Get().GetBrush("WhiteBrush"),
					ESlateDrawEffect::None,
					FLinearColor(M.Color.R, M.Color.G, M.Color.B, bMinimap ? 0.96f : 0.98f)
				);
			}

			if (bShowLabels && !M.Label.IsEmpty())
			{
				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId + 10,
					ToPaintGeo(ClampToBounds(P + FVector2D(6.f, -10.f)), FVector2D(260.f, 20.f)),
					M.Label,
					FT66Style::Tokens::FontBold(bMinimap ? 10 : 12),
					ESlateDrawEffect::None,
					WithAlpha(FT66Style::Text(), bMinimap ? 0.70f : 0.92f)
				);
			}
		}

		return LayerId + 10;
	}

private:
	bool bMinimap = false;
	bool bShowLabels = false;
	bool bLockFullMapToBounds = false;
	bool bUseRevealMask = false;
	bool bHasTowerHole = false;
	bool bHasTowerPolygon = false;

	FVector2D FullWorldMin = FVector2D::ZeroVector;
	FVector2D FullWorldMax = FVector2D::ZeroVector;

	FVector2D PlayerWorldXY = FVector2D::ZeroVector;
	FVector2D PlayerDirectionWorldXY = FVector2D(1.0f, 0.0f);
	TArray<FT66MapMarker> Markers;
	TArray<FVector2D> RevealWorldPoints;
	TArray<FBox2D> TowerMazeWallBoxes;
	const FSlateBrush* BackgroundBrush = nullptr;
	const FSlateBrush* PlayerBrush = nullptr;
	FLinearColor MapArtTint = FLinearColor::White;
	FLinearColor MapWallFillColor = FLinearColor(0.14f, 0.11f, 0.09f, 1.0f);
	FLinearColor MapWallStrokeColor = FLinearColor(0.92f, 0.86f, 0.72f, 1.0f);

	float MinimapHalfExtent = 2500.f;
	float RevealWorldRadius = 0.0f;
	FVector2D TowerHoleCenter = FVector2D::ZeroVector;
	FVector2D TowerHoleHalfExtents = FVector2D::ZeroVector;
	TArray<FVector2D> TowerPolygonWorldVertices;
};

namespace
{
	struct FT66ResolvedBeatTarget
	{
		bool bValid = false;
		bool bSupportsPacing = false;
		int64 Score = 0;
		float TimeSeconds = 0.f;
		FString EntryId;
		FString LocalRunSummarySlotName;
	};

	FString MakeHudBeatTargetCacheKey(
		const ET66LeaderboardType Type,
		const ET66PartySize PartySize,
		const ET66Difficulty Difficulty,
		const ET66LeaderboardFilter Filter)
	{
		const TCHAR* const TypeToken = (Type == ET66LeaderboardType::Score) ? TEXT("score") : TEXT("speedrun");
		const TCHAR* const PartyToken =
			(PartySize == ET66PartySize::Duo) ? TEXT("duo") :
			(PartySize == ET66PartySize::Trio) ? TEXT("trio") :
			(PartySize == ET66PartySize::Quad) ? TEXT("quad") : TEXT("solo");
		const TCHAR* const DifficultyToken =
			(Difficulty == ET66Difficulty::Medium) ? TEXT("medium") :
			(Difficulty == ET66Difficulty::Hard) ? TEXT("hard") :
			(Difficulty == ET66Difficulty::VeryHard) ? TEXT("veryhard") :
			(Difficulty == ET66Difficulty::Impossible) ? TEXT("impossible") : TEXT("easy");
		const TCHAR* const FilterToken =
			(Filter == ET66LeaderboardFilter::Friends) ? TEXT("friends") :
			(Filter == ET66LeaderboardFilter::Streamers) ? TEXT("streamers") : TEXT("global");

		return FString::Printf(TEXT("%s_alltime_%s_%s_%s"), TypeToken, PartyToken, DifficultyToken, FilterToken);
	}

	void EnsureHudBeatTargetLeaderboardRequested(
		UT66BackendSubsystem* Backend,
		const ET66LeaderboardType Type,
		const ET66PartySize PartySize,
		const ET66Difficulty Difficulty,
		const ET66LeaderboardFilter Filter)
	{
		if (!Backend || !Backend->IsBackendConfigured())
		{
			return;
		}

		const FString CacheKey = MakeHudBeatTargetCacheKey(Type, PartySize, Difficulty, Filter);
		if (Backend->HasCachedLeaderboard(CacheKey))
		{
			return;
		}

		const FString TypeToken = (Type == ET66LeaderboardType::Score) ? TEXT("score") : TEXT("speedrun");
		const FString PartyToken =
			(PartySize == ET66PartySize::Duo) ? TEXT("duo") :
			(PartySize == ET66PartySize::Trio) ? TEXT("trio") :
			(PartySize == ET66PartySize::Quad) ? TEXT("quad") : TEXT("solo");
		const FString DifficultyToken =
			(Difficulty == ET66Difficulty::Medium) ? TEXT("medium") :
			(Difficulty == ET66Difficulty::Hard) ? TEXT("hard") :
			(Difficulty == ET66Difficulty::VeryHard) ? TEXT("veryhard") :
			(Difficulty == ET66Difficulty::Impossible) ? TEXT("impossible") : TEXT("easy");
		const FString FilterToken =
			(Filter == ET66LeaderboardFilter::Friends) ? TEXT("friends") :
			(Filter == ET66LeaderboardFilter::Streamers) ? TEXT("streamers") : TEXT("global");

		Backend->FetchLeaderboard(TypeToken, TEXT("alltime"), PartyToken, DifficultyToken, FilterToken);
	}

	bool FindStagePacingPointForStage(const UT66LeaderboardRunSummarySaveGame* Snapshot, const int32 Stage, FT66StagePacingPoint& OutPoint)
	{
		if (!Snapshot)
		{
			return false;
		}

		for (const FT66StagePacingPoint& Point : Snapshot->StagePacingPoints)
		{
			if (Point.Stage == Stage)
			{
				OutPoint = Point;
				return true;
			}
		}

		return false;
	}

	FText FormatHudTimerValue(const float Seconds)
	{
		const float ClampedSeconds = FMath::Max(0.f, Seconds);
		const int32 Minutes = FMath::FloorToInt(ClampedSeconds / 60.f);
		const int32 WholeSeconds = FMath::FloorToInt(FMath::Fmod(ClampedSeconds, 60.f));
		const int32 Centiseconds = FMath::FloorToInt(FMath::Fmod(ClampedSeconds * 100.f, 100.f));

		FNumberFormattingOptions TwoDigits;
		TwoDigits.MinimumIntegralDigits = 2;
		return FText::Format(
			NSLOCTEXT("T66.GameplayHUD", "TimerValueFormat", "{0}:{1}.{2}"),
			FText::AsNumber(Minutes),
			FText::AsNumber(WholeSeconds, &TwoDigits),
			FText::AsNumber(Centiseconds, &TwoDigits));
	}
}

