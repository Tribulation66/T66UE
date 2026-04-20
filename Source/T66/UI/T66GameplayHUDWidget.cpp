// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GameplayHUDWidget.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
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

DEFINE_LOG_CATEGORY_STATIC(LogT66HUD, Log, All);

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

TSharedRef<SWidget> UT66GameplayHUDWidget::RebuildWidget()
{
	return FT66Style::MakeResponsiveRoot(BuildSlateUI());
}

UT66RunStateSubsystem* UT66GameplayHUDWidget::GetRunState() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}

UT66DamageLogSubsystem* UT66GameplayHUDWidget::GetDamageLog() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr;
}

bool UT66GameplayHUDWidget::IsPausePresentationActive() const
{
	const AT66PlayerController* T66PC = Cast<AT66PlayerController>(GetOwningPlayer());
	if (!T66PC || !T66PC->IsPaused())
	{
		return false;
	}

	const UT66UIManager* LocalUIManager = T66PC->GetUIManager();
	return LocalUIManager
		&& LocalUIManager->IsModalActive()
		&& LocalUIManager->GetCurrentModalType() == ET66ScreenType::PauseMenu;
}

TSharedRef<SWidget> UT66GameplayHUDWidget::BuildPauseStatsPanel() const
{
	return T66StatsPanelSlate::MakeEssentialStatsPanel(
		GetRunState(),
		GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr,
		GT66MediaPanelW,
		true,
		-7,
		-6,
		true);
}

TSharedRef<SWidget> UT66GameplayHUDWidget::BuildPauseAchievementsPanel() const
{
	UGameInstance* GI = GetGameInstance();
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PlayerSettings = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;

	TArray<FAchievementData> TrackedAchievements = Achievements ? Achievements->GetAllAchievements() : TArray<FAchievementData>{};
	TrackedAchievements = TrackedAchievements.FilterByPredicate([Achievements](const FAchievementData& Achievement)
	{
		const bool bClaimed = Achievements ? Achievements->IsAchievementClaimed(Achievement.AchievementID) : false;
		const int32 Progress = FMath::Clamp(Achievement.CurrentProgress, 0, Achievement.RequirementCount);
		return !bClaimed
			&& !Achievement.bIsUnlocked
			&& Achievement.RequirementCount > 0
			&& Progress < Achievement.RequirementCount;
	});

	TrackedAchievements.Sort([PlayerSettings](const FAchievementData& Left, const FAchievementData& Right)
	{
		const bool bLeftFavorite = PlayerSettings && PlayerSettings->IsFavoriteAchievement(Left.AchievementID);
		const bool bRightFavorite = PlayerSettings && PlayerSettings->IsFavoriteAchievement(Right.AchievementID);
		if (bLeftFavorite != bRightFavorite)
		{
			return bLeftFavorite && !bRightFavorite;
		}

		const float LeftProgress = GetAchievementProgress01(Left);
		const float RightProgress = GetAchievementProgress01(Right);
		if (!FMath::IsNearlyEqual(LeftProgress, RightProgress))
		{
			return LeftProgress > RightProgress;
		}

		const int32 LeftRemaining = GetAchievementRemainingCount(Left);
		const int32 RightRemaining = GetAchievementRemainingCount(Right);
		if (LeftRemaining != RightRemaining)
		{
			return LeftRemaining < RightRemaining;
		}

		const int32 LeftOrder = GetAchievementOrderKey(Left.AchievementID);
		const int32 RightOrder = GetAchievementOrderKey(Right.AchievementID);
		if (LeftOrder != RightOrder)
		{
			return LeftOrder < RightOrder;
		}

		return Left.AchievementID.LexicalLess(Right.AchievementID);
	});

	TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
	const int32 MaxDisplayedAchievements = 7;
	const int32 DisplayCount = FMath::Min(TrackedAchievements.Num(), MaxDisplayedAchievements);

	if (DisplayCount <= 0)
	{
		Rows->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.PauseMenu", "NoTrackedAchievements", "No in-progress achievements right now."))
			.Font(FT66Style::Tokens::FontRegular(10))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			.AutoWrapText(true)
		];
	}
	else
	{
		for (int32 Index = 0; Index < DisplayCount; ++Index)
		{
			const FAchievementData& Achievement = TrackedAchievements[Index];
			const int32 Progress = FMath::Clamp(Achievement.CurrentProgress, 0, Achievement.RequirementCount);
			const int32 Requirement = FMath::Max(1, Achievement.RequirementCount);
			const float Progress01 = GetAchievementProgress01(Achievement);
			const bool bFavorited = PlayerSettings && PlayerSettings->IsFavoriteAchievement(Achievement.AchievementID);
			const FLinearColor ProgressColor = bFavorited
				? FLinearColor(0.94f, 0.78f, 0.28f, 1.0f)
				: FT66Style::Tokens::Success;
			const FText ProgressText = FText::Format(
				NSLOCTEXT("T66.PauseMenu", "PauseAchievementProgress", "{0}/{1}"),
				FText::AsNumber(Progress),
				FText::AsNumber(Requirement));

			Rows->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.03f, 0.04f, 0.06f, 0.96f))
				.ToolTipText(Achievement.Description)
				.Padding(FMargin(10.f, 6.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(Achievement.DisplayName)
							.Font(FT66Style::Tokens::FontBold(11))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.AutoWrapText(true)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(8.f, 0.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(bFavorited ? TEXT("\u2605") : TEXT("")))
							.Font(FT66Style::Tokens::FontRegular(12))
							.ColorAndOpacity(ProgressColor)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 2.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(Achievement.Description)
						.Font(FT66Style::Tokens::FontRegular(9))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 6.f, 0.f, 0.f)
					[
						SNew(SProgressBar)
						.Percent(Progress01)
						.FillColorAndOpacity(ProgressColor)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 4.f, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						[
							SNew(STextBlock)
							.Text(ProgressText)
							.Font(FT66Style::Tokens::FontBold(10))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(FText::Format(
								NSLOCTEXT("T66.PauseMenu", "PauseAchievementRemaining", "{0} left"),
								FText::AsNumber(GetAchievementRemainingCount(Achievement))))
							.Font(FT66Style::Tokens::FontRegular(9))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
				]
			];
		}
	}

	return SNew(SBox)
		.WidthOverride(BottomRightInventoryPanelWidth)
		[
			FT66Style::MakePanel(
				SNew(SBox)
				.HeightOverride(420.f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						Rows
					]
				],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 12.f, 12.f, 10.f)))
		];
}

void UT66GameplayHUDWidget::RefreshPausePresentation()
{
	const bool bPausePresentationActive = IsPausePresentationActive();
	bLastPausePresentationActive = bPausePresentationActive;

	if (PauseBackdropBorder.IsValid())
	{
		PauseBackdropBorder->SetVisibility(bPausePresentationActive ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
	}

	if (PauseStatsPanelBox.IsValid())
	{
		PauseStatsPanelBox->SetVisibility(bPausePresentationActive ? EVisibility::Visible : EVisibility::Collapsed);
		PauseStatsPanelBox->SetContent(bPausePresentationActive ? BuildPauseStatsPanel() : StaticCastSharedRef<SWidget>(SNew(SSpacer)));
	}

	if (PauseAchievementsPanelBox.IsValid())
	{
		PauseAchievementsPanelBox->SetVisibility(EVisibility::Collapsed);
		PauseAchievementsPanelBox->SetContent(StaticCastSharedRef<SWidget>(SNew(SSpacer)));
	}

	if (IsMediaViewerOpen())
	{
		RequestTikTokWebView2OverlaySync();
	}
}

void UT66GameplayHUDWidget::MarkHUDDirty()
{
	bHUDDirty = true;
}

void UT66GameplayHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const bool bPausePresentationActive = IsPausePresentationActive();
	if (bPausePresentationActive != bLastPausePresentationActive)
	{
		RefreshPausePresentation();
	}

	if (bHUDDirty)
	{
		RefreshHUD();
	}

	RefreshSpeedRunTimers();
	DPSRefreshAccumSeconds += InDeltaTime;
	if (DPSRefreshAccumSeconds >= DPSRefreshIntervalSeconds)
	{
		DPSRefreshAccumSeconds = 0.f;
		RefreshDPS();
	}

	const APawn* RevealPawn = GetOwningPlayerPawn();
	if (!RevealPawn)
	{
		if (const APlayerController* PC = GetOwningPlayer())
		{
			RevealPawn = PC->GetPawn();
		}
	}
	if (RevealPawn)
	{
		TowerRevealAccumSeconds += InDeltaTime;
		if (TowerRevealAccumSeconds >= TowerRevealIntervalSeconds)
		{
			TowerRevealAccumSeconds = 0.f;
			UpdateTowerMapReveal(RevealPawn->GetActorLocation());
		}
	}
	else
	{
		TowerRevealAccumSeconds = TowerRevealIntervalSeconds;
	}

	if (bPickupCardVisible && PickupCardBox.IsValid())
	{
		PickupCardRemainingSeconds = FMath::Max(0.f, PickupCardRemainingSeconds - InDeltaTime);
		if (PickupCardSkipText.IsValid())
		{
			PickupCardSkipText->SetText(BuildSkipCountdownText(PickupCardRemainingSeconds, FName(TEXT("Interact"))));
		}

		const float FadeAlpha = (PickupCardRemainingSeconds > PickupCardFadeOutSeconds)
			? 1.f
			: FMath::Clamp(PickupCardRemainingSeconds / PickupCardFadeOutSeconds, 0.f, 1.f);
		PickupCardBox->SetRenderOpacity(FadeAlpha);

		if (PickupCardRemainingSeconds <= 0.f)
		{
			HidePickupCard();
		}
	}

	TickChestRewardPresentation(InDeltaTime);
	TryShowQueuedPresentation();

	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		const bool bScoped = T66PC->IsHeroOneScopeViewEnabled();
		const bool bCrosshairLocked = T66PC->HasAttackLockedEnemy();
		if (CenterCrosshairWidget.IsValid() && bCrosshairLocked != bLastCrosshairLocked)
		{
			CenterCrosshairWidget->SetLocked(bCrosshairLocked);
			bLastCrosshairLocked = bCrosshairLocked;
		}
		if (CenterCrosshairBox.IsValid())
		{
			CenterCrosshairBox->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
			FVector2D CrosshairScreenPosition = FVector2D::ZeroVector;
			if (T66PC->GetAttackLockScreenPosition(CrosshairScreenPosition))
			{
				int32 ViewportSizeX = 0;
				int32 ViewportSizeY = 0;
				T66PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
				const FVector2D ViewportCenter(
					static_cast<float>(ViewportSizeX) * 0.5f,
					static_cast<float>(ViewportSizeY) * 0.5f);
				CenterCrosshairBox->SetRenderTransform(FSlateRenderTransform(CrosshairScreenPosition - ViewportCenter));
			}
			else
			{
				CenterCrosshairBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
			}
			if (bScoped != bLastScopedHudVisible)
			{
				CenterCrosshairBox->SetVisibility(bScoped ? EVisibility::Collapsed : EVisibility::HitTestInvisible);
			}
		}
		if (ScopedSniperOverlayBorder.IsValid())
		{
			if (bScoped != bLastScopedHudVisible)
			{
				ScopedSniperOverlayBorder->SetVisibility(bScoped ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
			}
		}
		if (bScoped)
		{
			if (ScopedUltTimerText.IsValid())
			{
				const int32 UltDisplayTenths = FMath::RoundToInt(FMath::Max(0.f, T66PC->GetHeroOneScopedUltRemainingSeconds()) * 10.f);
				if (!bLastScopedHudVisible || UltDisplayTenths != LastScopedUltDisplayTenths)
				{
					LastScopedUltDisplayTenths = UltDisplayTenths;
					ScopedUltTimerText->SetText(FText::FromString(FString::Printf(TEXT("ULT %.1fs"), static_cast<float>(UltDisplayTenths) / 10.f)));
				}
			}
			if (ScopedShotCooldownText.IsValid())
			{
				const float ShotCooldown = T66PC->GetHeroOneScopedShotCooldownRemainingSeconds();
				const bool bShotReady = ShotCooldown <= 0.f;
				const int32 ShotDisplayCentis = bShotReady ? 0 : FMath::RoundToInt(FMath::Max(0.f, ShotCooldown) * 100.f);
				if (!bLastScopedHudVisible
					|| bShotReady != bLastScopedShotReady
					|| (!bShotReady && ShotDisplayCentis != LastScopedShotDisplayCentis))
				{
					bLastScopedShotReady = bShotReady;
					LastScopedShotDisplayCentis = ShotDisplayCentis;
					ScopedShotCooldownText->SetText(
						bShotReady
							? NSLOCTEXT("T66.HUD", "ScopedShotReady", "SHOT READY")
							: FText::FromString(FString::Printf(TEXT("SHOT %.2fs"), static_cast<float>(ShotDisplayCentis) / 100.f)));
				}
			}
		}
		else if (bLastScopedHudVisible)
		{
			LastScopedUltDisplayTenths = INDEX_NONE;
			LastScopedShotDisplayCentis = INDEX_NONE;
			bLastScopedShotReady = false;
		}

		bLastScopedHudVisible = bScoped;
	}
	else
	{
		if (CenterCrosshairWidget.IsValid() && bLastCrosshairLocked)
		{
			CenterCrosshairWidget->SetLocked(false);
		}
		if (CenterCrosshairBox.IsValid())
		{
			CenterCrosshairBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
			CenterCrosshairBox->SetVisibility(EVisibility::HitTestInvisible);
		}
		if (ScopedSniperOverlayBorder.IsValid())
		{
			ScopedSniperOverlayBorder->SetVisibility(EVisibility::Collapsed);
		}
		bLastCrosshairLocked = false;
		bLastScopedHudVisible = false;
		LastScopedUltDisplayTenths = INDEX_NONE;
		LastScopedShotDisplayCentis = INDEX_NONE;
		bLastScopedShotReady = false;
	}
}

void UT66GameplayHUDWidget::RefreshHeroStats()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	if (StatDamageText.IsValid())
		StatDamageText->SetText(MakeGradeStatText(TEXT("Dmg"), RunState->GetDamageStat()));
	if (StatAttackSpeedText.IsValid())
		StatAttackSpeedText->SetText(MakeGradeStatText(TEXT("AS"), RunState->GetAttackSpeedStat()));
	if (StatAttackScaleText.IsValid())
		StatAttackScaleText->SetText(MakeGradeStatText(TEXT("Scale"), RunState->GetScaleStat()));
	if (StatArmorText.IsValid())
		StatArmorText->SetText(MakeGradeStatText(TEXT("Armor"), RunState->GetArmorStat()));
	if (StatEvasionText.IsValid())
		StatEvasionText->SetText(MakeGradeStatText(TEXT("Eva"), RunState->GetEvasionStat()));
	if (StatLuckText.IsValid())
		StatLuckText->SetText(MakeGradeStatText(TEXT("Luck"), RunState->GetLuckStat()));
}

void UT66GameplayHUDWidget::RefreshDPS()
{
	if (!DPSText.IsValid()) return;

	UT66DamageLogSubsystem* DamageLog = GetDamageLog();
	const int32 DisplayDPS = DamageLog ? FMath::RoundToInt(FMath::Max(0.f, DamageLog->GetRollingDPS())) : 0;
	const FLinearColor DPSColor = GetDPSColor(DisplayDPS);
	if (DisplayDPS != LastDisplayedDPS || !LastDisplayedDPSColor.Equals(DPSColor))
	{
		LastDisplayedDPS = DisplayDPS;
		LastDisplayedDPSColor = DPSColor;
		DPSText->SetText(FText::Format(
			NSLOCTEXT("T66.GameplayHUD", "DPSFormat", "DPS {0}"),
			FText::AsNumber(DisplayDPS)));
		DPSText->SetColorAndOpacity(FSlateColor(DPSColor));
	}
}

void UT66GameplayHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	RunState->HeartsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHearts);
	RunState->GoldChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->DebtChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->InventoryChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->InventoryChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->PanelVisibilityChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->ScoreChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->StageChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->StageTimerChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshStageAndTimer);
	RunState->SpeedRunTimerChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshSpeedRunTimers);
	RunState->BossChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshBossBar);
	RunState->DifficultyChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->CowardiceGatesTakenChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
		{
			IdolManager->IdolStateChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		}
	}
	RunState->HeroProgressChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->UltimateChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->SurvivalChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->QuickReviveChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshQuickReviveState);
	RunState->StatusEffectsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshStatusEffects);
	RunState->TutorialHintChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshTutorialHint);
	RunState->TutorialSubtitleChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshTutorialSubtitle);
	RunState->DevCheatsChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		}
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->OnMediaViewerOpenChanged.AddDynamic(this, &UT66GameplayHUDWidget::HandleMediaViewerOpenChanged);
		}
		if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Ach->AchievementsUnlocked.AddDynamic(this, &UT66GameplayHUDWidget::HandleAchievementsUnlocked);
			Ach->AchievementsStateChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		}
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnLeaderboardDataReady.AddUObject(this, &UT66GameplayHUDWidget::HandleBackendLeaderboardDataReady);
			Backend->OnRunSummaryReady.AddUObject(this, &UT66GameplayHUDWidget::HandleBackendRunSummaryReady);
		}
	}

	// Loot prompt should update immediately on overlap changes (no stage-timer polling).
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->NearbyLootBagChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshLootPrompt);
	}

	// Map/minimap refresh (lightweight, throttled timer; no per-frame UI thinking).
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(MapRefreshTimerHandle, this, &UT66GameplayHUDWidget::RefreshMapData, 0.5f, true);
		if (FPSText.IsValid() || ElevationText.IsValid())
		{
			World->GetTimerManager().SetTimer(FPSTimerHandle, this, &UT66GameplayHUDWidget::RefreshFPS, 0.25f, true);
		}
	}

	// Bottom-left HUD scale (anchor bottom-left)
	if (BottomLeftHUDBox.IsValid())
	{
		BottomLeftHUDBox->SetRenderTransformPivot(FVector2D(0.f, 1.f));
		BottomLeftHUDBox->SetRenderTransform(FSlateRenderTransform(FTransform2D(GT66BottomLeftHudScale)));
	}

	ApplyInventoryInspectMode();
	TowerRevealPointsByFloor.Reset();

	// Passive and Ultimate ability tooltips (hero-specific)
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	FHeroData HD;
	if (T66GI && T66GI->GetSelectedHeroData(HD) && Loc)
	{
		if (PassiveBorder.IsValid())
		{
			PassiveBorder->SetToolTip(CreateRichTooltip(Loc->GetText_PassiveName(HD.PassiveType), Loc->GetText_PassiveDescription(HD.PassiveType)));
		}
		if (UltimateBorder.IsValid())
		{
			UltimateBorder->SetToolTip(CreateRichTooltip(Loc->GetText_UltimateName(HD.UltimateType), Loc->GetText_UltimateDescription(HD.UltimateType)));
		}
	}

	MarkHUDDirty();
	RefreshHUD();
	RefreshTutorialHint();
	RefreshTutorialSubtitle();
	RefreshSpeedRunTimers();
	RefreshDPS();
	RefreshLootPrompt();
	RefreshHearts();
	RefreshQuickReviveState();
	RefreshStatusEffects();
}

void UT66GameplayHUDWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MapRefreshTimerHandle);
		World->GetTimerManager().ClearTimer(FPSTimerHandle);
		World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
		World->GetTimerManager().ClearTimer(WheelSpinTickHandle);
		World->GetTimerManager().ClearTimer(WheelResolveHandle);
		World->GetTimerManager().ClearTimer(WheelCloseHandle);
		World->GetTimerManager().ClearTimer(AchievementNotificationTimerHandle);
	}

	HidePickupCard();
	HideChestReward();
	QueuedChestRewards.Reset();
	QueuedPickupCards.Reset();
	if (UT66CrateOverlayWidget* Overlay = ActiveCrateOverlay.Get())
	{
		Overlay->RemoveFromParent();
		ActiveCrateOverlay.Reset();
	}

	UT66RunStateSubsystem* RunState = GetRunState();
	if (RunState)
	{
		RunState->HeartsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHearts);
		RunState->GoldChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->DebtChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->PanelVisibilityChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->ScoreChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->StageChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshStageAndTimer);
		RunState->SpeedRunTimerChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshSpeedRunTimers);
		RunState->BossChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshBossBar);
		RunState->DifficultyChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->CowardiceGatesTakenChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
			{
				IdolManager->IdolStateChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
			}
		}
		RunState->HeroProgressChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->UltimateChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->SurvivalChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->QuickReviveChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshQuickReviveState);
		RunState->TutorialHintChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshTutorialHint);
		RunState->TutorialSubtitleChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshTutorialSubtitle);
		RunState->DevCheatsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->StatusEffectsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshStatusEffects);
	}
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		}
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->OnMediaViewerOpenChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::HandleMediaViewerOpenChanged);
		}
		if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Ach->AchievementsUnlocked.RemoveDynamic(this, &UT66GameplayHUDWidget::HandleAchievementsUnlocked);
			Ach->AchievementsStateChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		}
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnLeaderboardDataReady.RemoveAll(this);
			Backend->OnRunSummaryReady.RemoveAll(this);
		}
	}

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->NearbyLootBagChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshLootPrompt);
	}
	TowerRevealPointsByFloor.Reset();
	bHasAppliedMediaViewerOpenState = false;
	Super::NativeDestruct();
}

bool UT66GameplayHUDWidget::IsTikTokPlaceholderVisible() const
{
	return IsMediaViewerOpen();
}

void UT66GameplayHUDWidget::ToggleTikTokPlaceholder()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->ToggleMediaViewer();
			return;
		}
	}
}

bool UT66GameplayHUDWidget::IsMediaViewerOpen() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (const UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			return MV->IsMediaViewerOpen();
		}
	}
	return false;
}

void UT66GameplayHUDWidget::HandleMediaViewerOpenChanged(bool /*bIsOpen*/)
{
	UpdateTikTokVisibility(true);
}

void UT66GameplayHUDWidget::HandleBackendLeaderboardDataReady(const FString& Key)
{
	static_cast<void>(Key);
	MarkHUDDirty();
}

void UT66GameplayHUDWidget::HandleBackendRunSummaryReady(const FString& EntryId)
{
	static_cast<void>(EntryId);
	MarkHUDDirty();
}

void UT66GameplayHUDWidget::HandleAchievementsUnlocked(const TArray<FName>& NewlyUnlockedIDs)
{
	AchievementNotificationQueue.Append(NewlyUnlockedIDs);
	ShowNextAchievementNotification();
}

void UT66GameplayHUDWidget::ShowNextAchievementNotification()
{
	if (AchievementNotificationQueue.Num() == 0)
	{
		if (AchievementNotificationBox.IsValid())
		{
			AchievementNotificationBox->SetVisibility(EVisibility::Collapsed);
		}
		return;
	}
	UGameInstance* GI = GetGameInstance();
	UT66AchievementsSubsystem* Ach = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (!Ach || !AchievementNotificationBorder.IsValid() || !AchievementNotificationTitleText.IsValid() || !AchievementNotificationBox.IsValid())
	{
		return;
	}
	const FName AchievementID = AchievementNotificationQueue[0];
	const TArray<FAchievementData> All = Ach->GetAllAchievements();
	const FAchievementData* Data = All.FindByPredicate([&AchievementID](const FAchievementData& A) { return A.AchievementID == AchievementID; });
	if (!Data)
	{
		AchievementNotificationQueue.RemoveAt(0);
		ShowNextAchievementNotification();
		return;
	}
	auto GetTierBorderColor = [](ET66AchievementTier Tier) -> FLinearColor
	{
		switch (Tier)
		{
			case ET66AchievementTier::Black: return FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
			case ET66AchievementTier::Red:   return FLinearColor(0.6f, 0.15f, 0.15f, 1.0f);
			case ET66AchievementTier::Yellow: return FLinearColor(0.6f, 0.5f, 0.1f, 1.0f);
			case ET66AchievementTier::White: return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
			default: return FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
		}
	};
	AchievementNotificationBorder->SetBorderBackgroundColor(GetTierBorderColor(Data->Tier));
	AchievementNotificationTitleText->SetText(Data->DisplayName);
	AchievementNotificationBox->SetVisibility(EVisibility::Visible);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AchievementNotificationTimerHandle, this, &UT66GameplayHUDWidget::HideAchievementNotificationAndShowNext, AchievementNotificationDisplaySeconds, false);
	}
}

void UT66GameplayHUDWidget::HideAchievementNotificationAndShowNext()
{
	if (AchievementNotificationQueue.Num() > 0)
	{
		AchievementNotificationQueue.RemoveAt(0);
	}
	ShowNextAchievementNotification();
}

void UT66GameplayHUDWidget::RequestTikTokWebView2OverlaySync()
{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	// Defer one tick so Slate has a valid cached geometry for the panel.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
		World->GetTimerManager().SetTimerForNextTick(this, &UT66GameplayHUDWidget::SyncTikTokWebView2OverlayToPlaceholder);
	}
#endif
}

void UT66GameplayHUDWidget::SyncTikTokWebView2OverlayToPlaceholder()
{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	if (!IsMediaViewerOpen()) return;
	// Use the inner video box (below tabs) so the WebView2 overlay doesn't cover the tab buttons.
	TSharedPtr<SBox> SyncBox = MediaViewerVideoBox.IsValid() ? MediaViewerVideoBox : TikTokPlaceholderBox;
	if (!SyncBox.IsValid()) return;
	if (!FSlateApplication::IsInitialized()) return;

	// If the widget is collapsed, geometry may be invalid. Guard against 0-size.
	const FGeometry Geo = SyncBox->GetCachedGeometry();
	const FVector2D LocalSize = Geo.GetLocalSize();
	if (LocalSize.X < 4.f || LocalSize.Y < 4.f)
	{
		return;
	}

	const TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(SyncBox.ToSharedRef());
	if (!Window.IsValid())
	{
		return;
	}

	// Compute rect in window client coordinates (pixels). In practice, Slate's "screen space" aligns with window screen space.
	const FVector2D AbsTL = Geo.LocalToAbsolute(FVector2D::ZeroVector);
	const FVector2D AbsBR = Geo.LocalToAbsolute(LocalSize);

	// Treat Slate absolute space as desktop screen coordinates, then let Win32 do ScreenToClient against the real HWND.
	const int32 X0 = FMath::RoundToInt(AbsTL.X);
	const int32 Y0 = FMath::RoundToInt(AbsTL.Y);
	const int32 X1 = FMath::RoundToInt(AbsBR.X);
	const int32 Y1 = FMath::RoundToInt(AbsBR.Y);
	const FIntRect Rect(FIntPoint(X0, Y0), FIntPoint(X1, Y1));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->SetTikTokWebView2ScreenRect(Rect);
		}
	}
#endif
}

void UT66GameplayHUDWidget::UpdateTikTokVisibility(bool bForce)
{
	const bool bOpen = IsMediaViewerOpen();
	const bool bStateChanged = !bHasAppliedMediaViewerOpenState || (bLastAppliedMediaViewerOpenState != bOpen);
	if (!bForce && !bStateChanged)
	{
		return;
	}

	bHasAppliedMediaViewerOpenState = true;
	bLastAppliedMediaViewerOpenState = bOpen;

#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	// On Windows, TikTok UI is rendered by a native WebView2 overlay, but we keep the Slate panel visible
	// as a layout anchor so we can position the overlay correctly.
	const EVisibility Vis = bOpen ? EVisibility::Visible : EVisibility::Collapsed;
#else
	const EVisibility Vis = bOpen ? EVisibility::Visible : EVisibility::Collapsed;
#endif
	if (TikTokPlaceholderBox.IsValid())
	{
		TikTokPlaceholderBox->SetVisibility(Vis);
	}
	UE_LOG(LogT66HUD, Verbose, TEXT("[TIKTOK] Viewer %s"), bOpen ? TEXT("OPEN") : TEXT("CLOSED"));
	if (bOpen)
	{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
		// Align native WebView2 overlay to the phone panel location (and keep it aligned if window DPI/position changes).
		SyncTikTokWebView2OverlayToPlaceholder();
		RequestTikTokWebView2OverlaySync();
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
			World->GetTimerManager().SetTimer(TikTokOverlaySyncHandle, this, &UT66GameplayHUDWidget::SyncTikTokWebView2OverlayToPlaceholder, 0.50f, true);
		}
#endif
	}
	else
	{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
		}
#endif
	}
}

void UT66GameplayHUDWidget::StartWheelSpin(ET66Rarity WheelRarity)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Ensure panels exist.
	if (!WheelSpinBox.IsValid() || !WheelSpinDisk.IsValid() || !WheelSpinText.IsValid() || !WheelSpinSkipText.IsValid())
	{
		return;
	}

	if (ActiveCrateOverlay.IsValid() || bChestRewardVisible || bWheelPanelOpen)
	{
		return;
	}

	if (bPickupCardVisible && !ActivePickupCardItemID.IsNone())
	{
		QueuedPickupCards.InsertDefaulted(0, 1);
		FQueuedPickupCard& QueuedPickup = QueuedPickupCards[0];
		QueuedPickup.ItemID = ActivePickupCardItemID;
		QueuedPickup.ItemRarity = ActivePickupCardRarity;
	}
	HidePickupCard();

	// Reset any previous state.
	World->GetTimerManager().ClearTimer(WheelSpinTickHandle);
	World->GetTimerManager().ClearTimer(WheelResolveHandle);
	World->GetTimerManager().ClearTimer(WheelCloseHandle);

	ActiveWheelRarity = WheelRarity;
	bWheelPanelOpen = true;
	bWheelSpinning = true;
	WheelSpinElapsed = 0.f;
	WheelSpinDuration = ChestRewardDisplaySeconds;
	WheelStartAngleDeg = 0.f;
	WheelTotalAngleDeg = 0.f;
	WheelLastTickTimeSeconds = static_cast<float>(World->GetTimeSeconds());

	// Roll pending gold immediately; award on resolve.
	FRandomStream SpinRng(static_cast<int32>(FPlatformTime::Cycles())); // visual-only randomness (not luck-affected)

	int32 PendingGold = 50;
	int32 MinGold = 0;
	int32 MaxGold = 0;
	bool bHasGoldRange = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RngSubsystem* RngSub = GI->GetSubsystem<UT66RngSubsystem>())
		{
			UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			if (RunState)
			{
				RngSub->UpdateLuckStat(RunState->GetLuckStat());
			}

			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
			{
				if (UT66PlayerExperienceSubSystem* PlayerExperience = GI->GetSubsystem<UT66PlayerExperienceSubSystem>())
				{
					const FT66FloatRange Range = PlayerExperience->GetDifficultyWheelGoldRange(T66GI->SelectedDifficulty, WheelRarity);

					MinGold = FMath::FloorToInt(FMath::Min(Range.Min, Range.Max));
					MaxGold = FMath::CeilToInt(FMath::Max(Range.Min, Range.Max));
					bHasGoldRange = true;

					FRandomStream& Stream = RngSub->GetRunStream();
					PendingGold = FMath::Max(0, FMath::RoundToInt(RngSub->RollFloatRangeBiased(Range, Stream)));
					const int32 DrawIndex = RngSub->GetLastRunDrawIndex();
					const int32 PreDrawSeed = RngSub->GetLastRunPreDrawSeed();
					if (RunState)
					{
						RunState->RecordLuckQuantityFloatRollRounded(
							FName(TEXT("WheelGold")),
							PendingGold,
							MinGold,
							MaxGold,
							Range.Min,
							Range.Max,
							DrawIndex,
							PreDrawSeed);
					}
				}
			}
		}
	}
	WheelPendingGold = PendingGold;

	if (GoldCurrencyBrush.IsValid())
	{
		BindRuntimeHudBrush(GoldCurrencyBrush, GetGoldCurrencyRelativePath(), FVector2D(32.f, 32.f));
	}

	// Tint the wheel texture by rarity color.
	WheelSpinDisk->SetColorAndOpacity(FT66RarityUtil::GetRarityColor(WheelRarity));
	WheelSpinDisk->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	WheelSpinDisk->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	WheelSpinText->SetText(FText::Format(
		NSLOCTEXT("T66.Wheel", "GoldCounterFormat", "+{0}"),
		FText::AsNumber(0)));
	WheelSpinSkipText->SetText(BuildSkipCountdownText(WheelSpinDuration, FName(TEXT("Interact"))));

	WheelSpinBox->SetVisibility(EVisibility::Visible);
	WheelSpinBox->SetRenderOpacity(1.f);

	// Big spin: multiple rotations + random offset.
	WheelTotalAngleDeg = static_cast<float>(SpinRng.RandRange(5, 9)) * 360.f + static_cast<float>(SpinRng.RandRange(0, 359));

	// 30Hz is plenty for a simple HUD spin and reduces timer overhead on low-end CPUs.
	World->GetTimerManager().SetTimer(WheelSpinTickHandle, this, &UT66GameplayHUDWidget::TickWheelSpin, 0.033f, true);
}

void UT66GameplayHUDWidget::StartCrateOpen()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	HidePickupCard();
	if (ActiveCrateOverlay.IsValid() || bChestRewardVisible || bWheelPanelOpen)
	{
		return;
	}

	UT66CrateOverlayWidget* Overlay = CreateWidget<UT66CrateOverlayWidget>(PC, UT66CrateOverlayWidget::StaticClass());
	if (Overlay)
	{
		Overlay->SetPresentationHost(this);
		Overlay->SetVisibility(ESlateVisibility::HitTestInvisible);
		Overlay->AddToViewport(100);
		ActiveCrateOverlay = Overlay;
	}
}

void UT66GameplayHUDWidget::StartChestReward(const ET66Rarity ChestRarity, const int32 GoldAmount)
{
	if (!ChestRewardBox.IsValid())
	{
		return;
	}

	if (ActiveCrateOverlay.IsValid() || bChestRewardVisible || bWheelPanelOpen)
	{
		FQueuedChestReward& QueuedReward = QueuedChestRewards.AddDefaulted_GetRef();
		QueuedReward.Rarity = ChestRarity;
		QueuedReward.GoldAmount = GoldAmount;
		return;
	}

	if (bPickupCardVisible && !ActivePickupCardItemID.IsNone())
	{
		QueuedPickupCards.InsertDefaulted(0, 1);
		FQueuedPickupCard& QueuedPickup = QueuedPickupCards[0];
		QueuedPickup.ItemID = ActivePickupCardItemID;
		QueuedPickup.ItemRarity = ActivePickupCardRarity;
	}
	HidePickupCard();

	if (GoldCurrencyBrush.IsValid())
	{
		BindRuntimeHudBrush(GoldCurrencyBrush, GetGoldCurrencyRelativePath(), FVector2D(32.f, 32.f));
	}

	const FVector2D ChestImageSize(108.f, 108.f);
	static constexpr ET66Rarity ChestPresentationArtRarity = ET66Rarity::Yellow;
	if (ChestRewardClosedBrush.IsValid())
	{
		BindRuntimeHudBrush(ChestRewardClosedBrush, GetChestRewardClosedRelativePath(ChestPresentationArtRarity), ChestImageSize);
		if (!ChestRewardClosedBrush->GetResourceObject())
		{
			BindHudAssetBrush(ChestRewardClosedBrush, GetChestRewardFallbackAssetPath(ChestPresentationArtRarity), ChestImageSize);
		}
		if (!ChestRewardClosedBrush->GetResourceObject())
		{
			BindRuntimeHudBrush(ChestRewardClosedBrush, GetChestRewardFallbackRelativePath(ChestPresentationArtRarity), ChestImageSize);
		}
	}
	if (ChestRewardOpenBrush.IsValid())
	{
		BindRuntimeHudBrush(ChestRewardOpenBrush, GetChestRewardOpenRelativePath(ChestPresentationArtRarity), ChestImageSize);
		if (!ChestRewardOpenBrush->GetResourceObject())
		{
			BindHudAssetBrush(ChestRewardOpenBrush, GetChestRewardFallbackAssetPath(ChestPresentationArtRarity), ChestImageSize);
		}
		if (!ChestRewardOpenBrush->GetResourceObject())
		{
			BindRuntimeHudBrush(ChestRewardOpenBrush, GetChestRewardFallbackRelativePath(ChestPresentationArtRarity), ChestImageSize);
		}
	}

	if (ChestRewardClosedImage.IsValid())
	{
		ChestRewardClosedImage->SetImage(ChestRewardClosedBrush.Get());
	}
	if (ChestRewardOpenImage.IsValid())
	{
		ChestRewardOpenImage->SetImage(ChestRewardOpenBrush.Get());
	}
	if (ChestRewardClosedBox.IsValid())
	{
		ChestRewardClosedBox->SetRenderOpacity(1.f);
	}
	if (ChestRewardOpenBox.IsValid())
	{
		ChestRewardOpenBox->SetRenderOpacity(0.f);
	}

	ActiveChestRewardRarity = ChestRarity;
	ChestRewardTargetGold = FMath::Max(0, GoldAmount);
	ChestRewardDisplayedGold = 0;
	ChestRewardMinimumDisplayedGold = 0;
	ChestRewardElapsedSeconds = 0.f;
	ChestRewardRemainingSeconds = ChestRewardDisplaySeconds;
	bChestRewardVisible = true;

	if (ChestRewardCounterText.IsValid())
	{
		ChestRewardCounterText->SetText(FText::Format(
			NSLOCTEXT("T66.ChestReward", "GoldCounterFormat", "+{0}"),
			FText::AsNumber(0)));
	}
	if (ChestRewardSkipText.IsValid())
	{
		ChestRewardSkipText->SetText(BuildSkipCountdownText(ChestRewardDisplaySeconds, FName(TEXT("Interact"))));
	}

	if (ChestRewardBox.IsValid())
	{
		ChestRewardBox->SetVisibility(EVisibility::Visible);
		ChestRewardBox->SetRenderOpacity(1.f);
	}
	RefreshChestRewardVisualState();

	for (int32 CoinIndex = 0; CoinIndex < ChestRewardCoinBoxes.Num(); ++CoinIndex)
	{
		if (ChestRewardCoinBoxes[CoinIndex].IsValid())
		{
			ChestRewardCoinBoxes[CoinIndex]->SetVisibility(EVisibility::Visible);
			ChestRewardCoinBoxes[CoinIndex]->SetRenderOpacity(0.f);
		}
		if (ChestRewardCoinImages.IsValidIndex(CoinIndex) && ChestRewardCoinImages[CoinIndex].IsValid())
		{
			ChestRewardCoinImages[CoinIndex]->SetImage(GoldCurrencyBrush.Get());
		}
	}
}

bool UT66GameplayHUDWidget::TrySkipActivePresentation()
{
	if (UT66CrateOverlayWidget* Overlay = ActiveCrateOverlay.Get())
	{
		Overlay->RequestSkip();
		return true;
	}

	if (bWheelPanelOpen)
	{
		if (bWheelSpinning)
		{
			WheelSpinElapsed = WheelSpinDuration;
			if (WheelSpinText.IsValid())
			{
				WheelSpinText->SetText(FText::Format(
					NSLOCTEXT("T66.Wheel", "GoldCounterFormat", "+{0}"),
					FText::AsNumber(WheelPendingGold)));
			}
			ResolveWheelSpin();
		}
		else
		{
			CloseWheelSpin();
		}
		return true;
	}

	if (bChestRewardVisible)
	{
		const ET66Rarity CurrentDisplayRarity = ResolveChestRewardDisplayedRarity(FMath::Max(ChestRewardDisplayedGold, ChestRewardMinimumDisplayedGold));
		if (CurrentDisplayRarity == ActiveChestRewardRarity)
		{
			ChestRewardDisplayedGold = ChestRewardTargetGold;
			ChestRewardMinimumDisplayedGold = ChestRewardTargetGold;
			RefreshChestRewardVisualState();
			HideChestReward();
		}
		else
		{
			const int32 CurrentStageIndex = T66RarityToStageIndex(CurrentDisplayRarity);
			const int32 FinalStageIndex = T66RarityToStageIndex(ActiveChestRewardRarity);
			const ET66Rarity NextDisplayRarity = T66StageIndexToRarity(FMath::Clamp(CurrentStageIndex + 1, 0, FinalStageIndex));
			ChestRewardMinimumDisplayedGold = FMath::Max(ChestRewardMinimumDisplayedGold, GetChestRewardRevealThresholdGold(NextDisplayRarity));
			ChestRewardDisplayedGold = FMath::Max(ChestRewardDisplayedGold, ChestRewardMinimumDisplayedGold);
			RefreshChestRewardVisualState();
		}
		return true;
	}

	if (bPickupCardVisible)
	{
		HidePickupCard();
		return true;
	}

	return false;
}

void UT66GameplayHUDWidget::ClearActiveCratePresentation(UT66CrateOverlayWidget* Overlay)
{
	if (!Overlay || ActiveCrateOverlay.Get() == Overlay)
	{
		ActiveCrateOverlay.Reset();
	}
}

void UT66GameplayHUDWidget::TickWheelSpin()
{
	if (!bWheelSpinning || !WheelSpinDisk.IsValid()) return;
	UWorld* World = GetWorld();
	if (!World) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	float Delta = Now - WheelLastTickTimeSeconds;
	WheelLastTickTimeSeconds = Now;
	Delta = FMath::Clamp(Delta, 0.f, 0.05f);

	WheelSpinElapsed += Delta;
	const float Alpha = FMath::Clamp(WheelSpinElapsed / FMath::Max(0.01f, WheelSpinDuration), 0.f, 1.f);
	const float Ease = FMath::InterpEaseOut(0.f, 1.f, Alpha, 4.2f);
	const float Angle = WheelStartAngleDeg + (WheelTotalAngleDeg * Ease);

	WheelSpinDisk->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	WheelSpinDisk->SetRenderTransform(FSlateRenderTransform(FTransform2D(FQuat2D(FMath::DegreesToRadians(Angle)))));
	if (WheelSpinText.IsValid())
	{
		const float CounterEase = FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.5f);
		const int32 DisplayedGold = FMath::RoundToInt(static_cast<float>(WheelPendingGold) * CounterEase);
		WheelSpinText->SetText(FText::Format(
			NSLOCTEXT("T66.Wheel", "GoldCounterFormat", "+{0}"),
			FText::AsNumber(DisplayedGold)));
	}
	if (WheelSpinSkipText.IsValid())
	{
		WheelSpinSkipText->SetText(BuildSkipCountdownText(FMath::Max(0.f, WheelSpinDuration - WheelSpinElapsed), FName(TEXT("Interact"))));
	}

	if (Alpha >= 1.f)
	{
		ResolveWheelSpin();
	}
}

void UT66GameplayHUDWidget::ResolveWheelSpin()
{
	if (!bWheelSpinning) return;
	bWheelSpinning = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WheelSpinTickHandle);
	}

	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		if (WheelPendingGold > 0)
		{
			RunState->AddGold(WheelPendingGold);
		}
	}
	CloseWheelSpin();
}

void UT66GameplayHUDWidget::CloseWheelSpin()
{
	bWheelPanelOpen = false;
	bWheelSpinning = false;
	WheelSpinElapsed = 0.f;
	WheelPendingGold = 0;
	WheelLastTickTimeSeconds = 0.f;
	WheelTotalAngleDeg = 0.f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WheelSpinTickHandle);
		World->GetTimerManager().ClearTimer(WheelResolveHandle);
		World->GetTimerManager().ClearTimer(WheelCloseHandle);
	}
	if (WheelSpinBox.IsValid())
	{
		WheelSpinBox->SetVisibility(EVisibility::Collapsed);
		WheelSpinBox->SetRenderOpacity(1.f);
	}
	if (WheelSpinDisk.IsValid())
	{
		WheelSpinDisk->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	if (WheelSpinText.IsValid())
	{
		WheelSpinText->SetText(FText::GetEmpty());
	}
	if (WheelSpinSkipText.IsValid())
	{
		WheelSpinSkipText->SetText(FText::GetEmpty());
	}

	TryShowQueuedPresentation();
}

int32 UT66GameplayHUDWidget::GetChestRewardRevealThresholdGold(const ET66Rarity Rarity) const
{
	if (Rarity == ET66Rarity::Black)
	{
		return 0;
	}

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI ? T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const ET66Rarity PriorRarity = [Rarity]() -> ET66Rarity
	{
		switch (Rarity)
		{
		case ET66Rarity::Red: return ET66Rarity::Black;
		case ET66Rarity::Yellow: return ET66Rarity::Red;
		case ET66Rarity::White: return ET66Rarity::Yellow;
		case ET66Rarity::Black:
		default: return ET66Rarity::Black;
		}
	}();
	if (PlayerExperience)
	{
		const FT66IntRange GoldRange = PlayerExperience->GetDifficultyChestGoldRange(Difficulty, PriorRarity);
		return FMath::Max(0, FMath::Max(GoldRange.Min, GoldRange.Max) + 1);
	}

	switch (Rarity)
	{
	case ET66Rarity::Red: return 76;
	case ET66Rarity::Yellow: return 181;
	case ET66Rarity::White: return 381;
	case ET66Rarity::Black:
	default: return 0;
	}
}

ET66Rarity UT66GameplayHUDWidget::ResolveChestRewardDisplayedRarity(const int32 DisplayedGold) const
{
	ET66Rarity ResolvedRarity = ET66Rarity::Black;
	const int32 FinalStageIndex = T66RarityToStageIndex(ActiveChestRewardRarity);
	for (int32 StageIndex = 1; StageIndex <= FinalStageIndex; ++StageIndex)
	{
		const ET66Rarity StageRarity = T66StageIndexToRarity(StageIndex);
		if (DisplayedGold >= GetChestRewardRevealThresholdGold(StageRarity))
		{
			ResolvedRarity = StageRarity;
			continue;
		}

		break;
	}

	return ResolvedRarity;
}

void UT66GameplayHUDWidget::RefreshChestRewardVisualState()
{
	const ET66Rarity DisplayRarity = ResolveChestRewardDisplayedRarity(FMath::Max(ChestRewardDisplayedGold, ChestRewardMinimumDisplayedGold));
	if (ChestRewardTileBorder.IsValid())
	{
		const FLinearColor AccentColor = FT66RarityUtil::GetRarityColor(DisplayRarity) * 0.58f + FLinearColor(0.06f, 0.05f, 0.04f, 0.52f);
		ChestRewardTileBorder->SetBorderBackgroundColor(AccentColor);
	}

	if (ChestRewardCounterText.IsValid())
	{
		ChestRewardCounterText->SetText(FText::Format(
			NSLOCTEXT("T66.ChestReward", "GoldCounterFormat", "+{0}"),
			FText::AsNumber(FMath::Max(0, ChestRewardDisplayedGold))));
	}
}

void UT66GameplayHUDWidget::TickChestRewardPresentation(const float InDeltaTime)
{
	if (!bChestRewardVisible || !ChestRewardBox.IsValid())
	{
		return;
	}

	ChestRewardElapsedSeconds += InDeltaTime;
	ChestRewardRemainingSeconds = FMath::Max(0.f, ChestRewardRemainingSeconds - InDeltaTime);

	const float CounterProgress = FMath::Clamp(ChestRewardElapsedSeconds / 1.05f, 0.f, 1.f);
	const float CounterEase = FMath::InterpEaseOut(0.f, 1.f, CounterProgress, 2.5f);
	ChestRewardDisplayedGold = FMath::RoundToInt(static_cast<float>(ChestRewardTargetGold) * CounterEase);
	ChestRewardDisplayedGold = FMath::Max(ChestRewardDisplayedGold, ChestRewardMinimumDisplayedGold);
	if (ChestRewardRemainingSeconds <= 0.f)
	{
		ChestRewardDisplayedGold = ChestRewardTargetGold;
	}

	RefreshChestRewardVisualState();
	if (ChestRewardSkipText.IsValid())
	{
		ChestRewardSkipText->SetText(BuildSkipCountdownText(ChestRewardRemainingSeconds, FName(TEXT("Interact"))));
	}

	const float FadeAlpha = (ChestRewardRemainingSeconds > ChestRewardFadeOutSeconds)
		? 1.f
		: FMath::Clamp(ChestRewardRemainingSeconds / ChestRewardFadeOutSeconds, 0.f, 1.f);
	ChestRewardBox->SetRenderOpacity(FadeAlpha);

	const float OpenAlpha = FMath::Clamp((ChestRewardElapsedSeconds - 0.08f) / 0.16f, 0.f, 1.f);
	if (ChestRewardClosedBox.IsValid())
	{
		ChestRewardClosedBox->SetRenderOpacity(1.f - OpenAlpha);
		ChestRewardClosedBox->SetWidthOverride(92.f + (1.f - OpenAlpha) * 12.f);
		ChestRewardClosedBox->SetHeightOverride(92.f + (1.f - OpenAlpha) * 12.f);
		ChestRewardClosedBox->SetRenderTransform(FSlateRenderTransform(FVector2D(0.f, (1.f - OpenAlpha) * 4.f)));
	}
	if (ChestRewardOpenBox.IsValid())
	{
		const float OpenScaleAlpha = FMath::Clamp((ChestRewardElapsedSeconds - 0.02f) / 0.22f, 0.f, 1.f);
		ChestRewardOpenBox->SetRenderOpacity(OpenAlpha);
		ChestRewardOpenBox->SetWidthOverride(96.f + OpenScaleAlpha * 16.f);
		ChestRewardOpenBox->SetHeightOverride(96.f + OpenScaleAlpha * 16.f);
		ChestRewardOpenBox->SetRenderTransform(FSlateRenderTransform(FVector2D(0.f, (1.f - OpenScaleAlpha) * 8.f)));
	}

	const float CoinStartTime = 0.12f;
	const float CoinLoopSeconds = 1.1f;
	for (int32 CoinIndex = 0; CoinIndex < ChestRewardCoinBoxes.Num(); ++CoinIndex)
	{
		TSharedPtr<SBox> CoinBox = ChestRewardCoinBoxes[CoinIndex];
		if (!CoinBox.IsValid())
		{
			continue;
		}

		const float PerCoinDelay = CoinStartTime + (CoinIndex * 0.06f);
		const float LocalTime = ChestRewardElapsedSeconds - PerCoinDelay;
		if (LocalTime <= 0.f || ChestRewardRemainingSeconds <= 0.08f)
		{
			CoinBox->SetRenderOpacity(0.f);
			continue;
		}

		const float LoopProgress = FMath::Fmod(LocalTime, CoinLoopSeconds) / CoinLoopSeconds;
		const float ArcAlpha = FMath::Sin(LoopProgress * PI);
		const float Spread = 10.f + CoinIndex * 4.f;
		const float LateralDrift = FMath::Sin((CoinIndex * 0.85f) + (LoopProgress * PI * 2.1f)) * Spread;
		const float VerticalDrift = -58.f * LoopProgress - (CoinIndex % 2 == 0 ? 8.f : 0.f);
		const float CoinSize = 10.f + (ArcAlpha * 12.f);

		CoinBox->SetWidthOverride(CoinSize);
		CoinBox->SetHeightOverride(CoinSize);
		CoinBox->SetRenderTransform(FSlateRenderTransform(FVector2D(LateralDrift, VerticalDrift)));
		CoinBox->SetRenderOpacity(ArcAlpha * OpenAlpha * FadeAlpha);
	}

	if (ChestRewardRemainingSeconds <= 0.f)
	{
		HideChestReward();
	}
}

void UT66GameplayHUDWidget::HideChestReward()
{
	bChestRewardVisible = false;
	ChestRewardRemainingSeconds = 0.f;
	ChestRewardElapsedSeconds = 0.f;
	ChestRewardTargetGold = 0;
	ChestRewardDisplayedGold = 0;
	ChestRewardMinimumDisplayedGold = 0;
	if (ChestRewardBox.IsValid())
	{
		ChestRewardBox->SetVisibility(EVisibility::Collapsed);
		ChestRewardBox->SetRenderOpacity(1.f);
	}
	if (ChestRewardClosedBox.IsValid())
	{
		ChestRewardClosedBox->SetRenderOpacity(1.f);
		ChestRewardClosedBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	if (ChestRewardOpenBox.IsValid())
	{
		ChestRewardOpenBox->SetRenderOpacity(0.f);
		ChestRewardOpenBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	if (ChestRewardCounterText.IsValid())
	{
		ChestRewardCounterText->SetText(FText::GetEmpty());
	}
	if (ChestRewardSkipText.IsValid())
	{
		ChestRewardSkipText->SetText(FText::GetEmpty());
	}
	for (const TSharedPtr<SBox>& CoinBox : ChestRewardCoinBoxes)
	{
		if (CoinBox.IsValid())
		{
			CoinBox->SetVisibility(EVisibility::Collapsed);
			CoinBox->SetRenderOpacity(0.f);
			CoinBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
		}
	}
}

void UT66GameplayHUDWidget::TryShowQueuedPresentation()
{
	if (ActiveCrateOverlay.IsValid() || bChestRewardVisible || bPickupCardVisible || bWheelPanelOpen)
	{
		return;
	}

	if (QueuedChestRewards.Num() > 0)
	{
		const FQueuedChestReward NextChestReward = QueuedChestRewards[0];
		QueuedChestRewards.RemoveAt(0);
		StartChestReward(NextChestReward.Rarity, NextChestReward.GoldAmount);
		return;
	}

	if (QueuedPickupCards.Num() > 0)
	{
		const FQueuedPickupCard NextPickup = QueuedPickupCards[0];
		QueuedPickupCards.RemoveAt(0);
		ShowPickupItemCard(NextPickup.ItemID, NextPickup.ItemRarity);
	}
}

FReply UT66GameplayHUDWidget::OnToggleImmortality()
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->ToggleDevImmortality();
	}
	return FReply::Handled();
}

FReply UT66GameplayHUDWidget::OnTogglePower()
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->ToggleDevPower();
	}
	return FReply::Handled();
}

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

void UT66GameplayHUDWidget::RefreshFPS()
{
	if (!FPSText.IsValid() && !ElevationText.IsValid()) return;
	UWorld* World = GetWorld();
	const float Delta = World ? World->GetDeltaSeconds() : 0.f;
	const int32 FPS = (Delta > 0.f) ? FMath::RoundToInt(1.f / Delta) : 0;
	if (FPSText.IsValid())
	{
		FPSText->SetText(FText::FromString(FString::Printf(TEXT("FPS: %d"), FPS)));
	}

	if (ElevationText.IsValid())
	{
		const APawn* Pawn = GetOwningPlayerPawn();
		if (!Pawn)
		{
			if (const APlayerController* PC = GetOwningPlayer())
			{
				Pawn = PC->GetPawn();
			}
		}

		float GroundElevation = Pawn ? Pawn->GetActorLocation().Z : 0.f;
		if (const AT66HeroBase* Hero = Cast<AT66HeroBase>(Pawn))
		{
			if (const UCapsuleComponent* Capsule = Hero->GetCapsuleComponent())
			{
				GroundElevation -= Capsule->GetScaledCapsuleHalfHeight();
			}
		}

		ElevationText->SetText(FText::FromString(FString::Printf(TEXT("ELV: %d"), FMath::RoundToInt(GroundElevation))));
	}
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

void UT66GameplayHUDWidget::RefreshEconomy()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// Net Worth
	if (NetWorthText.IsValid())
	{
		const int32 NetWorth = RunState->GetNetWorth();
		NetWorthText->SetText(FText::AsNumber(NetWorth));

		const FLinearColor NetWorthColor = NetWorth > 0
			? FT66Style::Tokens::Success
			: (NetWorth < 0 ? FT66Style::Tokens::Danger : FT66Style::Tokens::Text);
		NetWorthText->SetColorAndOpacity(FSlateColor(NetWorthColor));
	}

	// Gold
	if (GoldText.IsValid())
	{
		GoldText->SetText(FText::AsNumber(RunState->GetCurrentGold()));
	}

	// Owe (Debt) in red
	if (DebtText.IsValid())
	{
		DebtText->SetText(FText::AsNumber(RunState->GetCurrentDebt()));
	}

	// Score
	if (ScoreText.IsValid())
	{
		ScoreText->SetText(FText::AsNumber(RunState->GetCurrentScore()));
	}
	if (ScoreMultiplierText.IsValid())
	{
		ScoreMultiplierText->SetVisibility(EVisibility::Collapsed);
	}
}

void UT66GameplayHUDWidget::RefreshTutorialHint()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// Tutorial hint (above crosshair)
	if (TutorialHintBorder.IsValid() && TutorialHintLine1Text.IsValid() && TutorialHintLine2Text.IsValid())
	{
		const bool bShow = RunState->IsTutorialHintVisible() && (!RunState->GetTutorialHintLine1().IsEmpty() || !RunState->GetTutorialHintLine2().IsEmpty());
		TutorialHintBorder->SetVisibility(bShow ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
		if (bShow)
		{
			const FText L1 = RunState->GetTutorialHintLine1();
			const FText L2 = RunState->GetTutorialHintLine2();
			TutorialHintLine1Text->SetText(L1);
			TutorialHintLine2Text->SetText(L2);
			TutorialHintLine2Text->SetVisibility(L2.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible);
		}
	}
}

void UT66GameplayHUDWidget::RefreshTutorialSubtitle()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	if (TutorialSubtitleBorder.IsValid() && TutorialSubtitleSpeakerText.IsValid() && TutorialSubtitleBodyText.IsValid())
	{
		const bool bShow = RunState->IsTutorialSubtitleVisible() && !RunState->GetTutorialSubtitleText().IsEmpty();
		TutorialSubtitleBorder->SetVisibility(bShow ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
		if (bShow)
		{
			const FText Speaker = RunState->GetTutorialSubtitleSpeaker();
			TutorialSubtitleSpeakerText->SetText(Speaker);
			TutorialSubtitleSpeakerText->SetVisibility(Speaker.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible);
			TutorialSubtitleBodyText->SetText(RunState->GetTutorialSubtitleText());
		}
	}
}

void UT66GameplayHUDWidget::RefreshStageAndTimer()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	// Stage number
	if (StageText.IsValid())
	{
		if (RunState->IsInStageCatchUp())
		{
			StageText->SetText(NSLOCTEXT("T66.GameplayHUD", "StageCatchUp", "Stage: Catch Up"));
		}
		else
		{
			const FText Fmt = Loc ? Loc->GetText_StageNumberFormat() : NSLOCTEXT("T66.GameplayHUD", "StageNumberFormat", "Stage number: {0}");
			StageText->SetText(FText::Format(Fmt, FText::AsNumber(RunState->GetCurrentStage())));
		}

		bool bTowerBloodActive = false;
		if (const UWorld* World = GetWorld())
		{
			if (const AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode()))
			{
				static constexpr float TowerBloodDelaySeconds = 120.0f;
				const float Elapsed = UT66RunStateSubsystem::StageTimerDurationSeconds - RunState->GetStageTimerSecondsRemaining();
				bTowerBloodActive = GameMode->IsUsingTowerMainMapLayout()
					&& RunState->GetStageTimerActive()
					&& Elapsed >= TowerBloodDelaySeconds;
			}
		}

		StageText->SetColorAndOpacity(bTowerBloodActive ? FSlateColor(FLinearColor(0.95f, 0.18f, 0.20f, 1.0f)) : FSlateColor(FT66Style::Tokens::Text));
	}

	// (Central countdown timer removed)
}

void UT66GameplayHUDWidget::RefreshBeatTargets()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	UT66PlayerSettingsSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66BackendSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!RunState || !T66GI || !PS || !LB)
	{
		return;
	}

	auto ResolveTarget = [PS, LB, Backend, T66GI](const ET66LeaderboardType Type, const FT66BeatTargetSelection& Selection, FT66ResolvedBeatTarget& OutTarget)
	{
		OutTarget = FT66ResolvedBeatTarget{};

		switch (Selection.Mode)
		{
		case ET66BeatTargetSelectionMode::PersonalBest:
			if (Type == ET66LeaderboardType::Score)
			{
				FT66LocalScoreRecord Record;
				if (LB->GetLocalBestScoreRecord(T66GI->SelectedDifficulty, T66GI->SelectedPartySize, Record) && Record.BestScore > 0)
				{
					OutTarget.bValid = true;
					OutTarget.Score = Record.BestScore;
					OutTarget.bSupportsPacing = false;
				}
			}
			else
			{
				FT66LocalCompletedRunTimeRecord Record;
				if (LB->GetLocalBestCompletedRunTimeRecord(T66GI->SelectedDifficulty, T66GI->SelectedPartySize, Record) && Record.BestCompletedSeconds > 0.01f)
				{
					OutTarget.bValid = true;
					OutTarget.TimeSeconds = Record.BestCompletedSeconds;
					OutTarget.bSupportsPacing = false;
				}
			}
			return;

		case ET66BeatTargetSelectionMode::FriendsTop:
		case ET66BeatTargetSelectionMode::StreamersTop:
		case ET66BeatTargetSelectionMode::GlobalTop:
		{
			const ET66LeaderboardFilter Filter =
				(Selection.Mode == ET66BeatTargetSelectionMode::FriendsTop) ? ET66LeaderboardFilter::Friends :
				(Selection.Mode == ET66BeatTargetSelectionMode::StreamersTop) ? ET66LeaderboardFilter::Streamers :
				ET66LeaderboardFilter::Global;

			EnsureHudBeatTargetLeaderboardRequested(Backend, Type, T66GI->SelectedPartySize, T66GI->SelectedDifficulty, Filter);
			if (!Backend)
			{
				return;
			}

			const FString CacheKey = MakeHudBeatTargetCacheKey(Type, T66GI->SelectedPartySize, T66GI->SelectedDifficulty, Filter);
			if (!Backend->HasCachedLeaderboard(CacheKey))
			{
				return;
			}

			const TArray<FLeaderboardEntry> Entries = Backend->GetCachedLeaderboard(CacheKey);
			if (Entries.Num() <= 0)
			{
				return;
			}

			const FLeaderboardEntry& Entry = Entries[0];
			OutTarget.bValid = true;
			OutTarget.Score = Entry.Score;
			OutTarget.TimeSeconds = Entry.TimeSeconds;
			OutTarget.EntryId = Entry.EntryId;
			OutTarget.bSupportsPacing = (Filter == ET66LeaderboardFilter::Global) && Entry.bHasRunSummary && !Entry.EntryId.IsEmpty();
			return;
		}

		case ET66BeatTargetSelectionMode::FavoriteRun:
		{
			FT66FavoriteLeaderboardRun Favorite;
			if (!PS->FindFavoriteLeaderboardRun(Selection.FavoriteEntryId, Favorite))
			{
				return;
			}
			if (Favorite.Difficulty != T66GI->SelectedDifficulty
				|| Favorite.PartySize != T66GI->SelectedPartySize)
			{
				return;
			}

			OutTarget.bValid = true;
			OutTarget.Score = Favorite.Score;
			OutTarget.TimeSeconds = Favorite.TimeSeconds;
			OutTarget.EntryId = Favorite.EntryId;
			OutTarget.bSupportsPacing = (Favorite.Filter == ET66LeaderboardFilter::Global) && Favorite.bHasRunSummary && !Favorite.EntryId.IsEmpty();
			return;
		}

		default:
			return;
		}
	};

	auto LoadPacingSummary = [Backend](const FT66ResolvedBeatTarget& Target) -> UT66LeaderboardRunSummarySaveGame*
	{
		if (!Target.bSupportsPacing)
		{
			return nullptr;
		}

		if (!Target.LocalRunSummarySlotName.IsEmpty())
		{
			if (UGameplayStatics::DoesSaveGameExist(Target.LocalRunSummarySlotName, 0))
			{
				return Cast<UT66LeaderboardRunSummarySaveGame>(UGameplayStatics::LoadGameFromSlot(Target.LocalRunSummarySlotName, 0));
			}
			return nullptr;
		}

		if (!Backend || Target.EntryId.IsEmpty())
		{
			return nullptr;
		}

		if (Backend->HasCachedRunSummary(Target.EntryId))
		{
			return Backend->GetCachedRunSummary(Target.EntryId);
		}

		Backend->FetchRunSummary(Target.EntryId);
		return nullptr;
	};

	FT66ResolvedBeatTarget ScoreTarget;
	ResolveTarget(ET66LeaderboardType::Score, PS->GetScoreToBeatSelection(), ScoreTarget);

	if (ScoreTargetText.IsValid())
	{
		if (PS->GetShowScoreToBeat() && ScoreTarget.bValid)
		{
			ScoreTargetText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "ScoreTargetFormat", "Score to Beat {0}"),
				FText::AsNumber(ScoreTarget.Score)));
			ScoreTargetText->SetVisibility(EVisibility::Visible);
		}
		else
		{
			ScoreTargetText->SetVisibility(EVisibility::Collapsed);
		}
	}

	if (ScorePacingText.IsValid())
	{
		UT66LeaderboardRunSummarySaveGame* PacingSummary = PS->GetShowScorePacing() ? LoadPacingSummary(ScoreTarget) : nullptr;
		FT66StagePacingPoint PacingPoint;
		if (PS->GetShowScorePacing()
			&& !RunState->IsInStageCatchUp()
			&& PacingSummary
			&& FindStagePacingPointForStage(PacingSummary, RunState->GetCurrentStage(), PacingPoint)
			&& PacingPoint.Score > 0)
		{
			ScorePacingText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "ScorePacingFormat", "Score Pace {0}"),
				FText::AsNumber(PacingPoint.Score)));
			ScorePacingText->SetVisibility(EVisibility::Visible);
		}
		else
		{
			ScorePacingText->SetVisibility(EVisibility::Collapsed);
		}
	}

	FT66ResolvedBeatTarget TimeTarget;
	ResolveTarget(ET66LeaderboardType::SpeedRun, PS->GetTimeToBeatSelection(), TimeTarget);
	const bool bShowLiveRunTime = PS->GetSpeedRunMode();

	if (SpeedRunTargetText.IsValid())
	{
		if (bShowLiveRunTime && PS->GetShowTimeToBeat() && TimeTarget.bValid && !RunState->IsInStageCatchUp())
		{
			SpeedRunTargetText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetFormat", "Time to Beat {0}"),
				FormatHudTimerValue(TimeTarget.TimeSeconds)));
			SpeedRunTargetText->SetVisibility(EVisibility::Visible);
		}
		else
		{
			SpeedRunTargetText->SetVisibility(EVisibility::Collapsed);
		}
	}

	if (SpeedRunPacingText.IsValid())
	{
		UT66LeaderboardRunSummarySaveGame* PacingSummary = (bShowLiveRunTime && PS->GetShowTimePacing()) ? LoadPacingSummary(TimeTarget) : nullptr;
		FT66StagePacingPoint PacingPoint;
		if (bShowLiveRunTime
			&& PS->GetShowTimePacing()
			&& !RunState->IsInStageCatchUp()
			&& PacingSummary
			&& FindStagePacingPointForStage(PacingSummary, RunState->GetCurrentStage(), PacingPoint)
			&& PacingPoint.ElapsedSeconds > 0.f)
		{
			SpeedRunPacingText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "TimePacingFormat", "Time Pace {0}"),
				FormatHudTimerValue(PacingPoint.ElapsedSeconds)));
			SpeedRunPacingText->SetVisibility(EVisibility::Visible);
		}
		else
		{
			SpeedRunPacingText->SetVisibility(EVisibility::Collapsed);
		}
	}
}

void UT66GameplayHUDWidget::RefreshSpeedRunTimers()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66PlayerSettingsSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;

	// Time (speedrun timer): counts up after leaving the start area (visibility toggled by player setting)
	if (SpeedRunText.IsValid())
	{
		const bool bShow = PS ? PS->GetSpeedRunMode() : false;
		const EVisibility DesiredVisibility = bShow ? EVisibility::Visible : EVisibility::Collapsed;
		if (SpeedRunText->GetVisibility() != DesiredVisibility)
		{
			SpeedRunText->SetVisibility(DesiredVisibility);
		}
		if (bShow)
		{
			const float Secs = FMath::Max(0.f, RunState->GetCurrentRunElapsedSeconds());
			// [GOLD] HUD text cache: only reformat when the displayed centisecond value changes (avoids FText::Format every frame).
			const int32 TotalCs = FMath::FloorToInt(Secs * 100.f);
			if (TotalCs != LastDisplayedSpeedRunTotalCs)
			{
				LastDisplayedSpeedRunTotalCs = TotalCs;
				SpeedRunText->SetText(FText::Format(
					NSLOCTEXT("T66.GameplayHUD", "SpeedRunTimerFormat", "Time {0}"),
					FormatHudTimerValue(Secs)));
			}
		}
		else
		{
			LastDisplayedSpeedRunTotalCs = -1;
		}
	}
}

bool UT66GameplayHUDWidget::DoesBossPartBarTopologyMatch(const TArray<FT66BossPartSnapshot>& BossParts) const
{
	if (BossPartBarRows.Num() != BossParts.Num())
	{
		return false;
	}

	for (int32 Index = 0; Index < BossParts.Num(); ++Index)
	{
		const FT66BossPartBarRow& ExistingRow = BossPartBarRows[Index];
		const FT66BossPartSnapshot& Part = BossParts[Index];
		if (ExistingRow.PartID != Part.PartID || ExistingRow.HitZoneType != Part.HitZoneType)
		{
			return false;
		}
	}

	return true;
}

void UT66GameplayHUDWidget::RebuildBossPartBars(
	const TArray<FT66BossPartSnapshot>& BossParts,
	const FLinearColor& BossBarBackgroundColor)
{
	if (!BossPartBarsBox.IsValid())
	{
		BossPartBarRows.Reset();
		return;
	}

	BossPartBarsBox->ClearChildren();
	BossPartBarRows.Reset();
	BossPartBarRows.Reserve(BossParts.Num());

	static constexpr float BossBarWidth = 560.f;

	for (const FT66BossPartSnapshot& Part : BossParts)
	{
		FT66BossPartBarRow& Row = BossPartBarRows.AddDefaulted_GetRef();
		Row.PartID = Part.PartID;
		Row.HitZoneType = Part.HitZoneType;

		BossPartBarsBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BossBarBackgroundColor)
				.Padding(0.f)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			[
				SAssignNew(Row.FillBox, SBox)
				.HeightOverride(18.f)
				.WidthOverride(BossBarWidth)
				[
					SAssignNew(Row.FillBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(GetBossPartFillColor(Part.HitZoneType, Part.IsAlive()))
					.Padding(0.f)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(Row.Text, STextBlock)
				.Font(FT66Style::Tokens::FontBold(11))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
		];
	}
}

void UT66GameplayHUDWidget::RefreshBossBar()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	static constexpr float BossBarWidth = 560.f;
	const FLinearColor BossBarBackgroundColor = FT66Style::IsDotaTheme()
		? FT66Style::BossBarBackground()
		: FLinearColor(0.08f, 0.08f, 0.08f, 0.9f);

	// Boss health bar: visible only when boss awakened
	const bool bBossActive = RunState->GetBossActive();
	if (BossBarContainerBox.IsValid())
	{
		const EVisibility DesiredVisibility = bBossActive ? EVisibility::Visible : EVisibility::Collapsed;
		if (BossBarContainerBox->GetVisibility() != DesiredVisibility)
		{
			BossBarContainerBox->SetVisibility(DesiredVisibility);
		}
	}

	if (bBossActive)
	{
		const int32 BossHP = RunState->GetBossCurrentHP();
		const int32 BossMax = FMath::Max(1, RunState->GetBossMaxHP());
		if (!bLastBossBarVisible || BossHP != LastDisplayedBossCurrentHP || BossMax != LastDisplayedBossMaxHP)
		{
			const float Pct = static_cast<float>(BossHP) / static_cast<float>(BossMax);
			if (BossBarFillBox.IsValid())
			{
				BossBarFillBox->SetWidthOverride(FMath::Clamp(BossBarWidth * Pct, 0.f, BossBarWidth));
			}
			if (BossBarText.IsValid())
			{
				BossBarText->SetText(FText::Format(
					NSLOCTEXT("T66.Common", "Fraction", "{0}/{1}"),
					FText::AsNumber(BossHP),
					FText::AsNumber(BossMax)));
			}

			LastDisplayedBossCurrentHP = BossHP;
			LastDisplayedBossMaxHP = BossMax;
		}
		bLastBossBarVisible = true;

		if (BossPartBarsBox.IsValid())
		{
			const TArray<FT66BossPartSnapshot>& BossParts = RunState->GetBossPartSnapshots();
			const bool bShowPartBars = BossParts.Num() > 1;
			const EVisibility DesiredPartVisibility = bShowPartBars ? EVisibility::Visible : EVisibility::Collapsed;
			if (BossPartBarsBox->GetVisibility() != DesiredPartVisibility)
			{
				BossPartBarsBox->SetVisibility(DesiredPartVisibility);
			}

			if (bShowPartBars)
			{
				if (!DoesBossPartBarTopologyMatch(BossParts))
				{
					// Rebuild only when the boss part layout changes; damage updates should reuse the existing rows.
					RebuildBossPartBars(BossParts, BossBarBackgroundColor);
				}

				for (int32 Index = 0; Index < BossParts.Num() && Index < BossPartBarRows.Num(); ++Index)
				{
					const FT66BossPartSnapshot& Part = BossParts[Index];
					FT66BossPartBarRow& Row = BossPartBarRows[Index];
					const int32 PartMaxHP = FMath::Max(1, Part.MaxHP);
					const int32 PartCurrentHP = FMath::Clamp(Part.CurrentHP, 0, PartMaxHP);
					const float PartPct = static_cast<float>(PartCurrentHP) / static_cast<float>(PartMaxHP);
					const bool bPartAlive = Part.IsAlive();

					if (Row.FillBox.IsValid() && (Row.LastCurrentHP != PartCurrentHP || Row.LastMaxHP != PartMaxHP))
					{
						Row.FillBox->SetWidthOverride(FMath::Clamp(BossBarWidth * PartPct, 0.f, BossBarWidth));
					}

					if (Row.FillBorder.IsValid() && (Row.bLastAlive != bPartAlive || Row.LastCurrentHP == INDEX_NONE))
					{
						Row.FillBorder->SetBorderBackgroundColor(GetBossPartFillColor(Part.HitZoneType, bPartAlive));
					}

					if (Row.Text.IsValid() && (Row.LastCurrentHP != PartCurrentHP || Row.LastMaxHP != PartMaxHP))
					{
						Row.Text->SetText(FText::Format(
							NSLOCTEXT("T66.GameplayHUD", "BossPartFraction", "{0} {1}/{2}"),
							GetBossPartDisplayName(Part),
							FText::AsNumber(PartCurrentHP),
							FText::AsNumber(PartMaxHP)));
					}

					Row.LastCurrentHP = PartCurrentHP;
					Row.LastMaxHP = PartMaxHP;
					Row.bLastAlive = bPartAlive;
				}
			}
		}
	}
	else
	{
		if (BossPartBarsBox.IsValid())
		{
			BossPartBarsBox->SetVisibility(EVisibility::Collapsed);
		}
		LastDisplayedBossCurrentHP = INDEX_NONE;
		LastDisplayedBossMaxHP = INDEX_NONE;
		bLastBossBarVisible = false;
	}
}

void UT66GameplayHUDWidget::RefreshLootPrompt()
{
	// No longer show accept/reject prompt; item is added immediately on interact, then item card popup is shown.
	if (LootPromptBox.IsValid())
	{
		LootPromptBox->SetVisibility(EVisibility::Collapsed);
	}
}

void UT66GameplayHUDWidget::ShowPickupItemCard(FName ItemID, ET66ItemRarity ItemRarity)
{
	if (ItemID.IsNone() || !PickupCardBox.IsValid()) return;
	if (ActiveCrateOverlay.IsValid() || bChestRewardVisible || bWheelPanelOpen)
	{
		FQueuedPickupCard& QueuedPickup = QueuedPickupCards.AddDefaulted_GetRef();
		QueuedPickup.ItemID = ItemID;
		QueuedPickup.ItemRarity = ItemRarity;
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GetRunState();

	FItemData D;
	const bool bHasData = GI && GI->GetItemData(ItemID, D);

	if (PickupCardNameText.IsValid())
	{
		PickupCardNameText->SetText(Loc ? Loc->GetText_ItemDisplayNameForRarity(ItemID, ItemRarity) : FText::FromName(ItemID));
	}
	if (PickupCardDescText.IsValid())
	{
		if (!bHasData)
		{
			PickupCardDescText->SetText(FText::GetEmpty());
		}
		else
		{
			int32 MainValue = 0;
			const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
			float Line2Multiplier = 0.f;
			if (RunState)
			{
				const TArray<FT66InventorySlot>& Slots = RunState->GetInventorySlots();
				if (Slots.Num() > 0 && Slots.Last().ItemTemplateID == ItemID)
				{
					MainValue = Slots.Last().Line1RolledValue;
					Line2Multiplier = Slots.Last().GetLine2Multiplier();
				}
				if (ItemID == FName(TEXT("Item_GamblersToken")))
				{
					MainValue = RunState->GetActiveGamblersTokenLevel();
				}
			}
			PickupCardDescText->SetText(T66ItemCardTextUtils::BuildItemCardDescription(Loc, D, ItemRarity, MainValue, ScaleMult, Line2Multiplier));
		}
	}
	if (!PickupCardIconBrush.IsValid())
	{
		PickupCardIconBrush = MakeShared<FSlateBrush>();
		PickupCardIconBrush->DrawAs = ESlateBrushDrawType::Image;
		PickupCardIconBrush->ImageSize = FVector2D(PickupCardWidth, PickupCardWidth);
	}
	const TSoftObjectPtr<UTexture2D> PickupIconSoft = bHasData ? D.GetIconForRarity(ItemRarity) : TSoftObjectPtr<UTexture2D>();
	if (!PickupIconSoft.IsNull())
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			T66SlateTexture::BindSharedBrushAsync(TexPool, PickupIconSoft, this, PickupCardIconBrush, FName(TEXT("HUDPickupCard")), true);
		}
	}
	if (PickupCardIconImage.IsValid())
	{
		PickupCardIconImage->SetImage(PickupCardIconBrush.Get());
		PickupCardIconImage->SetVisibility(!PickupIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (PickupCardTileBorder.IsValid())
	{
		const FLinearColor AccentColor = bHasData
			? (FItemData::GetItemRarityColor(ItemRarity) * 0.52f + FLinearColor(0.05f, 0.05f, 0.06f, 0.48f))
			: FT66Style::Tokens::Panel;
		PickupCardTileBorder->SetBorderBackgroundColor(AccentColor);
	}
	if (PickupCardIconBorder.IsValid())
	{
		PickupCardIconBorder->SetBorderBackgroundColor(FLinearColor(0.04f, 0.04f, 0.05f, 1.f));
	}
	if (PickupCardSkipText.IsValid())
	{
		PickupCardSkipText->SetText(BuildSkipCountdownText(PickupCardDisplaySeconds, FName(TEXT("Interact"))));
	}

	ActivePickupCardItemID = ItemID;
	ActivePickupCardRarity = ItemRarity;
	bPickupCardVisible = true;
	PickupCardRemainingSeconds = PickupCardDisplaySeconds;
	PickupCardBox->SetVisibility(EVisibility::Visible);
	PickupCardBox->SetRenderOpacity(1.f);
}

void UT66GameplayHUDWidget::HidePickupCard()
{
	bPickupCardVisible = false;
	PickupCardRemainingSeconds = 0.f;
	ActivePickupCardItemID = NAME_None;
	ActivePickupCardRarity = ET66ItemRarity::Black;
	if (PickupCardBox.IsValid())
	{
		PickupCardBox->SetVisibility(EVisibility::Collapsed);
		PickupCardBox->SetRenderOpacity(1.f);
	}
	if (PickupCardSkipText.IsValid())
	{
		PickupCardSkipText->SetText(FText::GetEmpty());
	}
}

void UT66GameplayHUDWidget::RefreshHearts()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// Hearts: split each gameplay slot into two visual segments so the HUD shows ten hearts total.
	for (int32 i = 0; i < HeartImages.Num(); ++i)
	{
		if (!HeartImages[i].IsValid()) continue;
		const int32 SlotIndex = i / 2;
		const float SlotFill = RunState->GetHeartSlotFill(SlotIndex);
		const float SegmentStart = (i % 2 == 0) ? 0.f : 0.5f;
		const float SegmentFill = FMath::Clamp((SlotFill - SegmentStart) / 0.5f, 0.f, 1.f);
		const int32 HeartTier = RunState->GetHeartSlotTier(SlotIndex);
		HeartImages[i]->SetImage(ResolveHeartBrushForDisplay(
			HeartTierBrushes,
			HeartBlessingBrush,
			HeartBrush,
			RunState->IsSaintBlessingActive(),
			HeartTier));
		HeartImages[i]->SetColorAndOpacity(FLinearColor::White);
		if (HeartFillBoxes.IsValidIndex(i) && HeartFillBoxes[i].IsValid())
		{
			HeartFillBoxes[i]->SetWidthOverride(GT66DisplayedHeartWidth * SegmentFill);
			HeartFillBoxes[i]->SetVisibility(SegmentFill > 0.01f ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}
}

void UT66GameplayHUDWidget::RefreshQuickReviveState()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState)
	{
		return;
	}

	if (QuickReviveIconRowBox.IsValid())
	{
		QuickReviveIconRowBox->SetVisibility(RunState->HasQuickReviveCharge() ? EVisibility::Visible : EVisibility::Collapsed);
	}

	if (QuickReviveDownedOverlayBorder.IsValid())
	{
		const bool bDowned = RunState->IsInQuickReviveDownedState();
		QuickReviveDownedOverlayBorder->SetVisibility(bDowned ? EVisibility::Visible : EVisibility::Collapsed);
		if (QuickReviveDownedText.IsValid())
		{
			const int32 SecondsRemaining = FMath::Max(1, FMath::CeilToInt(RunState->GetQuickReviveDownedSecondsRemaining()));
			QuickReviveDownedText->SetText(FText::Format(
				NSLOCTEXT("T66.GameplayHUD", "QuickReviveDownedCountdown", "REVIVING IN {0}"),
				FText::AsNumber(SecondsRemaining)));
		}
	}
}

void UT66GameplayHUDWidget::RefreshStatusEffects()
{
}

void UT66GameplayHUDWidget::RefreshHUD()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("GameplayHUD::RefreshHUD"));

	bHUDDirty = false;
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66GameInstance* GIAsT66 = Cast<UT66GameInstance>(GetGameInstance());

	RefreshEconomy();
	RefreshStageAndTimer();
	RefreshBeatTargets();
	RefreshBossBar();
	RefreshHeroStats();

	// Portrait frame stays neutral; heart tier is already conveyed by the heart row and other HUD accents.
	if (PortraitBorder.IsValid())
	{
		PortraitBorder->SetBorderBackgroundColor(RunState->IsSaintBlessingActive()
			? FLinearColor(0.92f, 0.92f, 0.98f, 1.f)
			: (FT66Style::IsDotaTheme()
				? FT66Style::PanelInner()
				: FLinearColor(0.12f, 0.12f, 0.14f, 1.f)));
	}
	const FName DesiredPortraitHeroID = GIAsT66 ? GIAsT66->SelectedHeroID : NAME_None;
	const ET66BodyType DesiredPortraitBodyType = GIAsT66 ? GIAsT66->SelectedHeroBodyType : ET66BodyType::TypeA;
	ET66HeroPortraitVariant DesiredPortraitVariant = RunState->IsSaintBlessingActive()
		? ET66HeroPortraitVariant::Invincible
		: ET66HeroPortraitVariant::Half;
	const int32 HeartsRemaining = RunState->GetCurrentHearts();
	if (!RunState->IsSaintBlessingActive() && HeartsRemaining <= 1)
	{
		DesiredPortraitVariant = ET66HeroPortraitVariant::Low;
	}
	else if (!RunState->IsSaintBlessingActive() && HeartsRemaining >= 5)
	{
		DesiredPortraitVariant = ET66HeroPortraitVariant::Full;
	}

	const bool bPortraitStateChanged = !bPortraitStateInitialized
		|| LastPortraitHeroID != DesiredPortraitHeroID
		|| LastPortraitBodyType != DesiredPortraitBodyType
		|| LastPortraitVariant != DesiredPortraitVariant;

	if (bPortraitStateChanged)
	{
		bPortraitStateInitialized = true;
		LastPortraitHeroID = DesiredPortraitHeroID;
		LastPortraitBodyType = DesiredPortraitBodyType;
		LastPortraitVariant = DesiredPortraitVariant;

		TSoftObjectPtr<UTexture2D> PortraitSoft;
		if (GIAsT66 && !DesiredPortraitHeroID.IsNone())
		{
			FHeroData HeroData;
			if (GIAsT66->GetHeroData(DesiredPortraitHeroID, HeroData))
			{
				PortraitSoft = GIAsT66->ResolveHeroPortrait(HeroData, DesiredPortraitBodyType, DesiredPortraitVariant);
			}
		}

		bLastPortraitHasRef = !PortraitSoft.IsNull();
		if (PortraitBrush.IsValid())
		{
			UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
			if (PortraitSoft.IsNull() || !TexPool)
			{
				PortraitBrush->SetResourceObject(nullptr);
			}
			else
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, PortraitBrush, FName(TEXT("HUDPortrait")), /*bClearWhileLoading*/ true);
			}
		}
	}
	if (PortraitImage.IsValid())
	{
		const EVisibility DesiredVisibility = bLastPortraitHasRef ? EVisibility::Visible : EVisibility::Collapsed;
		if (PortraitImage->GetVisibility() != DesiredVisibility)
		{
			PortraitImage->SetVisibility(DesiredVisibility);
		}
	}
	if (PortraitPlaceholderText.IsValid())
	{
		const EVisibility DesiredVisibility = bLastPortraitHasRef ? EVisibility::Collapsed : EVisibility::Visible;
		if (PortraitPlaceholderText->GetVisibility() != DesiredVisibility)
		{
			PortraitPlaceholderText->SetVisibility(DesiredVisibility);
		}
	}

	FHeroData SelectedHeroData;
	const bool bHasSelectedHeroData = GIAsT66 && GIAsT66->GetSelectedHeroData(SelectedHeroData);
	const FName DesiredAbilityHeroID = bHasSelectedHeroData ? SelectedHeroData.HeroID : NAME_None;
	const ET66UltimateType DesiredUltimateType = bHasSelectedHeroData ? SelectedHeroData.UltimateType : ET66UltimateType::None;
	const ET66PassiveType DesiredPassiveType = bHasSelectedHeroData ? SelectedHeroData.PassiveType : ET66PassiveType::None;
	const bool bAbilityStateChanged = !bAbilityStateInitialized
		|| LastAbilityHeroID != DesiredAbilityHeroID
		|| LastUltimateType != DesiredUltimateType
		|| LastPassiveType != DesiredPassiveType;

	if (bAbilityStateChanged)
	{
		bAbilityStateInitialized = true;
		LastAbilityHeroID = DesiredAbilityHeroID;
		LastUltimateType = DesiredUltimateType;
		LastPassiveType = DesiredPassiveType;

		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (UltimateBrush.IsValid())
		{
			const TSoftObjectPtr<UTexture2D> UltimateSoft = ResolveGameplayUltimateIcon(DesiredAbilityHeroID, DesiredUltimateType);
			if (TexPool && !UltimateSoft.IsNull())
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, UltimateSoft, this, UltimateBrush, FName(TEXT("HUDUltimate")), false);
			}
			else
			{
				UltimateBrush->SetResourceObject(nullptr);
			}
		}

		if (PassiveBrush.IsValid())
		{
			const TSoftObjectPtr<UTexture2D> PassiveSoft = ResolveGameplayPassiveIcon(DesiredAbilityHeroID, DesiredPassiveType);
			if (TexPool && !PassiveSoft.IsNull())
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, PassiveSoft, this, PassiveBrush, FName(TEXT("HUDPassive")), false);
			}
			else
			{
				PassiveBrush->SetResourceObject(nullptr);
			}
		}
	}

	if (UltimateInputHintText.IsValid())
	{
		UltimateInputHintText->SetText(ResolveGameplayUltimateInputHint(DesiredUltimateType));
	}

	// Hero level + XP progress ring
	if (LevelRingWidget.IsValid())
	{
		LevelRingWidget->SetPercent(RunState->GetHeroXP01());
	}
	if (LevelText.IsValid())
	{
		LevelText->SetText(FText::Format(
			NSLOCTEXT("T66.HUD", "LevelOutOf99", "{0}/99"),
			FText::AsNumber(FMath::Clamp(RunState->GetHeroLevel(), 0, UT66RunStateSubsystem::MaxHeroLevel))));
	}

	// Ultimate (R) — show cooldown overlay with countdown when on cooldown, hide when ready
	{
		const bool bReady = RunState->IsUltimateReady();
		if (UltimateCooldownOverlay.IsValid())
		{
			UltimateCooldownOverlay->SetVisibility(bReady ? EVisibility::Collapsed : EVisibility::Visible);
		}
		if (UltimateText.IsValid() && !bReady)
		{
			const int32 Sec = FMath::CeilToInt(RunState->GetUltimateCooldownRemainingSeconds());
			UltimateText->SetText(FText::AsNumber(FMath::Max(0, Sec)));
		}
		if (UltimateBorder.IsValid())
		{
			// Subtle glow tint when ready, neutral border otherwise
			UltimateBorder->SetBorderBackgroundColor(bReady ? FLinearColor(0.08f, 0.08f, 0.10f, 1.f) : FLinearColor(0.08f, 0.08f, 0.10f, 1.f));
		}
	}

	// Passive stack badge (Rallying Blow: show circle + stack count)
	{
		const bool bRallyingBlow = (RunState->GetPassiveType() == ET66PassiveType::RallyingBlow);
		const int32 Stacks = RunState->GetRallyStacks();
		if (PassiveStackBadgeBox.IsValid())
		{
			PassiveStackBadgeBox->SetVisibility(bRallyingBlow ? EVisibility::Visible : EVisibility::Collapsed);
		}
		if (PassiveStackText.IsValid())
		{
			PassiveStackText->SetText(FText::AsNumber(FMath::Max(0, Stacks)));
		}
	}

	// Difficulty (Skulls): 5-slot compression with tier colors (no half-skulls).
	{
		const int32 Skulls = FMath::Max(0, RunState->GetDifficultySkulls());

		// Color tier changes every 5 skulls, but filling within a tier is 1..5.
		// Skull 1-5 => Tier 0, Within 1..5; Skull 6 => Tier 1, Within 1, etc.
		int32 Tier = 0;
		int32 Within = 0;
		if (Skulls > 0)
		{
			Tier = (Skulls - 1) / 5;
			Within = ((Skulls - 1) % 5) + 1;
		}
		const FLinearColor TierC = FT66RarityUtil::GetTierColor(Tier);
		for (int32 i = 0; i < DifficultyImages.Num(); ++i)
		{
			if (!DifficultyImages[i].IsValid()) continue;
			const bool bFilled = (i < Within);
			// Show skull only if slot should be filled; hide empty slots so skulls appear one-by-one.
			DifficultyImages[i]->SetVisibility(bFilled ? EVisibility::Visible : EVisibility::Collapsed);
			if (bFilled)
			{
				// Tier 0 = white skulls; higher tiers get the tier color.
				DifficultyImages[i]->SetColorAndOpacity(Tier == 0 ? FLinearColor::White : TierC);
			}
		}
		if (DifficultyRowBox.IsValid())
		{
			DifficultyRowBox->SetVisibility(Skulls > 0 ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}

	// Cowardice (clowns): show N clowns for gates taken this segment (resets after Coliseum).
	{
		const int32 Clowns = FMath::Max(0, RunState->GetCowardiceGatesTaken());
		for (int32 i = 0; i < ClownImages.Num(); ++i)
		{
			if (!ClownImages[i].IsValid()) continue;
			const bool bFilled = (i < Clowns);
			ClownImages[i]->SetVisibility(bFilled ? EVisibility::Visible : EVisibility::Collapsed);
			if (bFilled)
			{
				ClownImages[i]->SetColorAndOpacity(FLinearColor::White);
			}
		}
		if (CowardiceRowBox.IsValid())
		{
			CowardiceRowBox->SetVisibility(Clowns > 0 ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}

	// Score multiplier color: theme for initial/tier 0, tier color for higher tiers
	if (ScoreMultiplierText.IsValid())
	{
		ScoreMultiplierText->SetColorAndOpacity(FT66Style::Tokens::Text);
	}

	// Dev toggles (immortality / power)
	if (ImmortalityButtonText.IsValid())
	{
		const bool bOn = RunState->GetDevImmortalityEnabled();
		ImmortalityButtonText->SetText(bOn
			? NSLOCTEXT("T66.Dev", "ImmortalityOn", "IMMORTALITY: ON")
			: NSLOCTEXT("T66.Dev", "ImmortalityOff", "IMMORTALITY: OFF"));
		ImmortalityButtonText->SetColorAndOpacity(bOn ? FLinearColor(0.20f, 0.85f, 0.35f, 1.f) : FT66Style::Tokens::Text);
	}
	if (PowerButtonText.IsValid())
	{
		const bool bOn = RunState->GetDevPowerEnabled();
		PowerButtonText->SetText(bOn
			? NSLOCTEXT("T66.Dev", "PowerOn", "POWER: ON")
			: NSLOCTEXT("T66.Dev", "PowerOff", "POWER: OFF"));
		PowerButtonText->SetColorAndOpacity(bOn ? FLinearColor(0.95f, 0.80f, 0.20f, 1.f) : FT66Style::Tokens::Text);
	}

	// Idol slots: rarity-colored when equipped, dark teal when empty.
	UT66IdolManagerSubsystem* IdolManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
	const TArray<FName>& EquippedIdols = IdolManager ? IdolManager->GetEquippedIdols() : RunState->GetEquippedIdols();
	const TArray<FName>& Idols = EquippedIdols;
	for (int32 i = 0; i < IdolSlotBorders.Num(); ++i)
	{
		if (!IdolSlotBorders[i].IsValid()) continue;
		FLinearColor C = FLinearColor(0.08f, 0.14f, 0.12f, 0.92f);
		TSoftObjectPtr<UTexture2D> IdolIconSoft;
		TSharedPtr<IToolTip> IdolTooltipWidget;
		if (i < Idols.Num() && !Idols[i].IsNone())
		{
			const ET66ItemRarity IdolRarity = IdolManager ? IdolManager->GetEquippedIdolRarityInSlot(i) : RunState->GetEquippedIdolRarityInSlot(i);
			C = FItemData::GetItemRarityColor(IdolRarity);
			if (GIAsT66)
			{
				FIdolData IdolData;
				if (GIAsT66->GetIdolData(Idols[i], IdolData))
				{
					IdolIconSoft = IdolData.GetIconForRarity(IdolRarity);
					if (Loc)
					{
						IdolTooltipWidget = CreateRichTooltip(
							Loc->GetText_IdolDisplayName(Idols[i]),
							Loc->GetText_IdolTooltip(Idols[i]));
					}
					else
					{
						IdolTooltipWidget = CreateCustomTooltip(FText::FromName(Idols[i]));
					}
				}
			}
		}
		IdolSlotBorders[i]->SetBorderBackgroundColor(C);
		if (IdolSlotContainers.IsValidIndex(i) && IdolSlotContainers[i].IsValid())
		{
			IdolSlotContainers[i]->SetToolTip(IdolTooltipWidget);
		}

		if (IdolSlotBrushes.IsValidIndex(i) && IdolSlotBrushes[i].IsValid())
		{
			UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
			if (IdolIconSoft.IsNull() || !TexPool)
			{
				IdolSlotBrushes[i]->SetResourceObject(nullptr);
			}
			else
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, IdolIconSoft, this, IdolSlotBrushes[i], FName(TEXT("HUDIdol"), i + 1), /*bClearWhileLoading*/ true);
			}
		}
		if (IdolSlotImages.IsValidIndex(i) && IdolSlotImages[i].IsValid())
		{
			IdolSlotImages[i]->SetVisibility(!IdolIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}

	// Inventory slots: item color + hover tooltip, grey when empty
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	const TArray<FName>& EquippedInventory = RunState->GetInventory();
	const TArray<FT66InventorySlot>& EquippedInventorySlots = RunState->GetInventorySlots();
	const TArray<FName>& Inv = EquippedInventory;
	const TArray<FT66InventorySlot>& InvSlots = EquippedInventorySlots;
	if (InvSlots.Num() > InventorySlotBorders.Num())
	{
		FT66Style::DeferRebuild(this);
		return;
	}
	for (int32 i = 0; i < InventorySlotBorders.Num(); ++i)
	{
		if (!InventorySlotBorders[i].IsValid()) continue;

		FLinearColor SlotColor = FLinearColor(0.f, 0.f, 0.f, 0.25f);
		FText Tooltip = FText::GetEmpty();
		TSoftObjectPtr<UTexture2D> SlotIconSoft;
		if (i < Inv.Num() && !Inv[i].IsNone())
		{
			const FName ItemID = Inv[i];
			FItemData D;
			if (T66GI && T66GI->GetItemData(ItemID, D))
			{
				SlotColor = InvSlots.IsValidIndex(i) ? FItemData::GetItemRarityColor(InvSlots[i].Rarity) : FT66Style::Tokens::Panel2;
				TArray<FText> TipLines;
				TipLines.Reserve(8);
				const ET66ItemRarity SlotRarity = InvSlots.IsValidIndex(i) ? InvSlots[i].Rarity : ET66ItemRarity::Black;
				TipLines.Add(Loc ? Loc->GetText_ItemDisplayNameForRarity(ItemID, SlotRarity) : FText::FromName(ItemID));

				// Icon (optional). Do NOT sync-load in gameplay UI; request via the UI texture pool.
				SlotIconSoft = D.GetIconForRarity(SlotRarity);

				int32 MainValue = 0;
				if (InvSlots.IsValidIndex(i))
				{
					MainValue = InvSlots[i].Line1RolledValue;
				}
				const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
				const FText CardDesc = T66ItemCardTextUtils::BuildItemCardDescription(Loc, D, SlotRarity, MainValue, ScaleMult, InvSlots.IsValidIndex(i) ? InvSlots[i].GetLine2Multiplier() : 0.f);
				if (!CardDesc.IsEmpty())
				{
					TipLines.Add(CardDesc);
				}
				{
					int32 SellValue = 0;
					if (RunState && i >= 0 && i < InvSlots.Num())
					{
						SellValue = RunState->GetSellGoldForInventorySlot(InvSlots[i]);
					}
					if (SellValue > 0)
					{
						TipLines.Add(FText::Format(
							NSLOCTEXT("T66.ItemTooltip", "SellValueGold", "Sell: {0} gold"),
							FText::AsNumber(SellValue)));
					}
				}

				Tooltip = TipLines.Num() > 0 ? FText::Join(NSLOCTEXT("T66.Common", "NewLine", "\n"), TipLines) : FText::GetEmpty();
			}
			else
			{
				SlotColor = FLinearColor(0.95f, 0.15f, 0.15f, 1.f);
				Tooltip = FText::FromName(ItemID);
			}
		}
		InventorySlotBorders[i]->SetBorderBackgroundColor(SlotColor);
		const FName CurrentItemID = (i < Inv.Num()) ? Inv[i] : NAME_None;
		if (!CachedInventorySlotIDs.IsValidIndex(i) || CachedInventorySlotIDs[i] != CurrentItemID)
		{
			if (CachedInventorySlotIDs.IsValidIndex(i)) CachedInventorySlotIDs[i] = CurrentItemID;
			if (InventorySlotContainers.IsValidIndex(i) && InventorySlotContainers[i].IsValid())
			{
				InventorySlotContainers[i]->SetToolTip(CreateCustomTooltip(Tooltip));
			}
		}

		if (InventorySlotBrushes.IsValidIndex(i) && InventorySlotBrushes[i].IsValid())
		{
			UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
			if (SlotIconSoft.IsNull() || !TexPool)
			{
				InventorySlotBrushes[i]->SetResourceObject(nullptr);
			}
			else
			{
					T66SlateTexture::BindSharedBrushAsync(TexPool, SlotIconSoft, this, InventorySlotBrushes[i], FName(TEXT("HUDInv"), i + 1), /*bClearWhileLoading*/ true);
			}
		}
		if (InventorySlotImages.IsValidIndex(i) && InventorySlotImages[i].IsValid())
		{
			InventorySlotImages[i]->SetVisibility(!SlotIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Hidden);
		}
	}

	// Panel visibility: each element follows HUD toggle only if enabled in Settings (HUD tab).
	UGameInstance* GIHud = GetGameInstance();
	UT66PlayerSettingsSubsystem* HUDPS = GIHud ? GIHud->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	const bool bPanelsVisible = RunState->GetHUDPanelsVisible();
	auto ElemVis = [HUDPS, bPanelsVisible](bool bAffects) -> EVisibility
	{
		// If this element is not in the toggle set, always visible. Otherwise follow global panels state.
		if (!HUDPS || !bAffects) return EVisibility::Visible;
		return bPanelsVisible ? EVisibility::Visible : EVisibility::Collapsed;
	};
	if (InventoryPanelBox.IsValid()) InventoryPanelBox->SetVisibility(ElemVis(HUDPS ? HUDPS->GetHudToggleAffectsInventory() : true));
	if (MinimapPanelBox.IsValid()) MinimapPanelBox->SetVisibility(ElemVis(HUDPS ? HUDPS->GetHudToggleAffectsMinimap() : true));
	if (IdolSlotsPanelBox.IsValid()) IdolSlotsPanelBox->SetVisibility(ElemVis(HUDPS ? HUDPS->GetHudToggleAffectsIdolSlots() : true));
	if (PortraitStatPanelBox.IsValid()) PortraitStatPanelBox->SetVisibility(ElemVis(HUDPS ? HUDPS->GetHudToggleAffectsPortraitStats() : true));

	// Wheel spin panel: hide when all toggled panels would be collapsed (any one visible is enough to show wheel in its slot).
	const bool bAnyPanelVisible = (!HUDPS || HUDPS->GetHudToggleAffectsInventory() || HUDPS->GetHudToggleAffectsMinimap() || HUDPS->GetHudToggleAffectsIdolSlots() || HUDPS->GetHudToggleAffectsPortraitStats())
		? bPanelsVisible
		: true;
	RefreshPausePresentation();
	UpdateTikTokVisibility();
	if (WheelSpinBox.IsValid())
	{
		if (!bAnyPanelVisible)
		{
			WheelSpinBox->SetVisibility(EVisibility::Collapsed);
		}
		else
		{
			WheelSpinBox->SetVisibility(bWheelPanelOpen ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}
}

TSharedRef<SWidget> UT66GameplayHUDWidget::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const FText StageInit = Loc ? FText::Format(Loc->GetText_StageNumberFormat(), FText::AsNumber(1)) : NSLOCTEXT("T66.GameplayHUD", "StageNumberInit", "Stage number: 1");
	const FText NetWorthInit = FText::AsNumber(0);
	const FText GoldInit = FText::AsNumber(0);
	const FText OweInit = FText::AsNumber(0);
	const FText ScoreLabelText = Loc ? Loc->GetText_ScoreLabel() : NSLOCTEXT("T66.GameplayHUD", "ScoreLabel", "Score:");
	const FText PortraitLabel = Loc ? Loc->GetText_PortraitPlaceholder() : NSLOCTEXT("T66.GameplayHUD", "PortraitLabel", "PORTRAIT");
	NetWorthText.Reset();
	PortraitStatPanelBox.Reset();
	PortraitPlaceholderText.Reset();
	DifficultyRowBox.Reset();
	CowardiceRowBox.Reset();
	BossPartBarRows.Reset();
	const bool bDotaTheme = FT66Style::IsDotaTheme();
	const FLinearColor SlotOuterColor = bDotaTheme ? FT66Style::SlotOuter() : FLinearColor(0.f, 0.f, 0.f, 0.25f);
	const FLinearColor SlotFrameColor = bDotaTheme ? FT66Style::SlotInner() : FLinearColor(0.45f, 0.55f, 0.50f, 0.5f);
	const FLinearColor SlotFillColor = bDotaTheme ? FT66Style::SlotFill() : FLinearColor(0.f, 0.f, 0.f, 0.f);
	const FLinearColor BossBarBackgroundColor = bDotaTheme ? FT66Style::BossBarBackground() : FLinearColor(0.08f, 0.08f, 0.08f, 0.9f);
	const FLinearColor BossBarFillColor = bDotaTheme ? FT66Style::BossBarFill() : FLinearColor(0.9f, 0.1f, 0.1f, 0.95f);
	const FLinearColor PromptBackgroundColor = bDotaTheme ? FT66Style::PromptBackground() : FLinearColor(0.02f, 0.02f, 0.03f, 0.65f);
	const FLinearColor DialogueBackgroundColor = bDotaTheme ? FT66Style::PromptBackground() : FLinearColor(0.06f, 0.06f, 0.08f, 0.85f);
	const int32 InventorySlotWidgetCount = UT66RunStateSubsystem::MaxInventorySlots;

	HeartBorders.SetNum(GT66DisplayedHeartCount);
	HeartFillBoxes.SetNum(GT66DisplayedHeartCount);
	HeartImages.SetNum(GT66DisplayedHeartCount);
	HeartTierBrushes.SetNum(5);
	DifficultyBorders.SetNum(5);
	DifficultyImages.SetNum(5);
	ClownImages.SetNum(5);
	IdolSlotBorders.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	IdolSlotContainers.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	IdolSlotImages.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	IdolSlotBrushes.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	IdolLevelDotBorders.Empty();
	CachedIdolSlotIDs.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	InventorySlotBorders.SetNum(InventorySlotWidgetCount);
	InventorySlotContainers.SetNum(InventorySlotWidgetCount);
	InventorySlotImages.SetNum(InventorySlotWidgetCount);
	InventorySlotBrushes.SetNum(InventorySlotWidgetCount);
	CachedInventorySlotIDs.SetNum(InventorySlotWidgetCount);
	ChestRewardCoinBoxes.SetNum(ChestRewardCoinCount);
	ChestRewardCoinImages.SetNum(ChestRewardCoinCount);
	StatusEffectDots.SetNum(3);
	StatusEffectDotBoxes.SetNum(3);
	WorldDialogueOptionBorders.SetNum(3);
	WorldDialogueOptionTexts.SetNum(3);
	static constexpr float BossBarWidth = 560.f;

	// Brushes for icons (kept alive by shared pointers).
	if (!LootPromptIconBrush.IsValid())
	{
		LootPromptIconBrush = MakeShared<FSlateBrush>();
		LootPromptIconBrush->DrawAs = ESlateBrushDrawType::Image;
		LootPromptIconBrush->ImageSize = FVector2D(28.f, 28.f);
	}
	if (!GoldCurrencyBrush.IsValid())
	{
		GoldCurrencyBrush = MakeShared<FSlateBrush>();
		GoldCurrencyBrush->DrawAs = ESlateBrushDrawType::Image;
		GoldCurrencyBrush->ImageSize = FVector2D(20.f, 20.f);
		GoldCurrencyBrush->Tiling = ESlateBrushTileType::NoTile;
		GoldCurrencyBrush->SetResourceObject(nullptr);
	}
	if (!DebtCurrencyBrush.IsValid())
	{
		DebtCurrencyBrush = MakeShared<FSlateBrush>();
		DebtCurrencyBrush->DrawAs = ESlateBrushDrawType::Image;
		DebtCurrencyBrush->ImageSize = FVector2D(20.f, 20.f);
		DebtCurrencyBrush->Tiling = ESlateBrushTileType::NoTile;
		DebtCurrencyBrush->SetResourceObject(nullptr);
	}
	BindRuntimeHudBrush(GoldCurrencyBrush, GetGoldCurrencyRelativePath(), FVector2D(20.f, 20.f));
	BindRuntimeHudBrush(DebtCurrencyBrush, GetDebtCurrencyRelativePath(), FVector2D(20.f, 20.f));
	if (!ChestRewardClosedBrush.IsValid())
	{
		ChestRewardClosedBrush = MakeShared<FSlateBrush>();
		ChestRewardClosedBrush->DrawAs = ESlateBrushDrawType::Image;
		ChestRewardClosedBrush->ImageSize = FVector2D(108.f, 108.f);
		ChestRewardClosedBrush->Tiling = ESlateBrushTileType::NoTile;
		ChestRewardClosedBrush->SetResourceObject(nullptr);
	}
	if (!ChestRewardOpenBrush.IsValid())
	{
		ChestRewardOpenBrush = MakeShared<FSlateBrush>();
		ChestRewardOpenBrush->DrawAs = ESlateBrushDrawType::Image;
		ChestRewardOpenBrush->ImageSize = FVector2D(108.f, 108.f);
		ChestRewardOpenBrush->Tiling = ESlateBrushTileType::NoTile;
		ChestRewardOpenBrush->SetResourceObject(nullptr);
	}
	if (!PortraitBrush.IsValid())
	{
		PortraitBrush = MakeShared<FSlateBrush>();
		PortraitBrush->DrawAs = ESlateBrushDrawType::Image;
		PortraitBrush->ImageSize = FVector2D(GT66BottomLeftPortraitPanelSize, GT66BottomLeftPortraitPanelSize);
	}
	if (!UltimateBrush.IsValid())
	{
		UltimateBrush = MakeShared<FSlateBrush>();
		UltimateBrush->DrawAs = ESlateBrushDrawType::Image;
		UltimateBrush->ImageSize = FVector2D(GT66BottomLeftAbilityBoxSize, GT66BottomLeftAbilityBoxSize);
		UltimateBrush->Tiling = ESlateBrushTileType::NoTile;
		UltimateBrush->SetResourceObject(nullptr);
	}
	if (!PassiveBrush.IsValid())
	{
		PassiveBrush = MakeShared<FSlateBrush>();
		PassiveBrush->DrawAs = ESlateBrushDrawType::Image;
		PassiveBrush->ImageSize = FVector2D(GT66BottomLeftAbilityBoxSize, GT66BottomLeftAbilityBoxSize);
		PassiveBrush->Tiling = ESlateBrushTileType::NoTile;
		PassiveBrush->SetResourceObject(nullptr);
	}
	// Load fallback ability textures via the texture pool.
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> UltSoft = ResolveGameplayUltimateIcon(NAME_None, ET66UltimateType::None);
			T66SlateTexture::BindSharedBrushAsync(TexPool, UltSoft, this, UltimateBrush, FName(TEXT("HUDUltimate")), false);
			const TSoftObjectPtr<UTexture2D> PassiveSoft = ResolveGameplayPassiveIcon(NAME_None, ET66PassiveType::None);
			T66SlateTexture::BindSharedBrushAsync(TexPool, PassiveSoft, this, PassiveBrush, FName(TEXT("HUDPassive")), false);
		}
	}
	// Wheel spin texture
	{
		WheelTextureBrush = FSlateBrush();
		WheelTextureBrush.ImageSize = FVector2D(124.f, 124.f);
		WheelTextureBrush.DrawAs = ESlateBrushDrawType::Image;
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> WheelSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Wheel/Firefly_Gemini_Flash_Remove_background_286654.Firefly_Gemini_Flash_Remove_background_286654")));
			T66SlateTexture::BindBrushAsync(TexPool, WheelSoft, this, WheelTextureBrush, FName(TEXT("HUDWheel")), /*bClearWhileLoading*/ true);
		}
	}
	// Heart sprite brush
	if (!HeartBrush.IsValid())
	{
		HeartBrush = MakeShared<FSlateBrush>();
		HeartBrush->DrawAs = ESlateBrushDrawType::Image;
		HeartBrush->ImageSize = FVector2D(GT66DisplayedHeartWidth, GT66DisplayedHeartHeight);
		HeartBrush->Tiling = ESlateBrushTileType::NoTile;
		HeartBrush->SetResourceObject(nullptr);
	}
	if (!HeartBlessingBrush.IsValid())
	{
		HeartBlessingBrush = MakeShared<FSlateBrush>();
		HeartBlessingBrush->DrawAs = ESlateBrushDrawType::Image;
		HeartBlessingBrush->ImageSize = FVector2D(GT66DisplayedHeartWidth, GT66DisplayedHeartHeight);
		HeartBlessingBrush->Tiling = ESlateBrushTileType::NoTile;
		HeartBlessingBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> HeartSoft = ResolveGameplayHeartIcon(0);
			T66SlateTexture::BindSharedBrushAsync(TexPool, HeartSoft, this, HeartBrush, FName(TEXT("HUDHeartFallback")), false);
			T66SlateTexture::BindSharedBrushAsync(TexPool, ResolveGameplayBlessingHeartIcon(), this, HeartBlessingBrush, FName(TEXT("HUDHeartBlessing")), false);
			for (int32 TierIndex = 0; TierIndex < HeartTierBrushes.Num(); ++TierIndex)
			{
				if (!HeartTierBrushes[TierIndex].IsValid())
				{
					HeartTierBrushes[TierIndex] = MakeShared<FSlateBrush>();
					HeartTierBrushes[TierIndex]->DrawAs = ESlateBrushDrawType::Image;
					HeartTierBrushes[TierIndex]->ImageSize = FVector2D(GT66DisplayedHeartWidth, GT66DisplayedHeartHeight);
					HeartTierBrushes[TierIndex]->Tiling = ESlateBrushTileType::NoTile;
					HeartTierBrushes[TierIndex]->SetResourceObject(nullptr);
				}

				T66SlateTexture::BindSharedBrushAsync(
					TexPool,
					ResolveGameplayHeartIcon(TierIndex),
					this,
					HeartTierBrushes[TierIndex],
					FName(*FString::Printf(TEXT("HUDHeart_%d"), TierIndex)),
					false);
			}
		}
	}
	if (!QuickReviveBrush.IsValid())
	{
		QuickReviveBrush = MakeShared<FSlateBrush>();
		QuickReviveBrush->DrawAs = ESlateBrushDrawType::Image;
		QuickReviveBrush->ImageSize = FVector2D(26.f, 26.f);
		QuickReviveBrush->Tiling = ESlateBrushTileType::NoTile;
		QuickReviveBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> QuickReviveSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Interactables/QuickReviveIcon.QuickReviveIcon")));
			T66SlateTexture::BindSharedBrushAsync(TexPool, QuickReviveSoft, this, QuickReviveBrush, FName(TEXT("HUDQuickRevive")), false);
		}
	}

	auto ConfigureCrispUISprite = [](UTexture2D* Tex)
	{
		if (!Tex)
		{
			return;
		}

		Tex->bForceMiplevelsToBeResident = true;
		Tex->NeverStream = true;
		Tex->Filter = TextureFilter::TF_Nearest;
		Tex->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		Tex->UpdateResource();
	};
	// Skull sprite brush
	if (!SkullBrush.IsValid())
	{
		SkullBrush = MakeShared<FSlateBrush>();
		SkullBrush->DrawAs = ESlateBrushDrawType::Image;
		SkullBrush->ImageSize = FVector2D(38.f, 38.f);
		SkullBrush->Tiling = ESlateBrushTileType::NoTile;
		SkullBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> SkullSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/SKULL.SKULL")));
			T66SlateTexture::BindSharedBrushAsync(TexPool, SkullSoft, this, SkullBrush, FName(TEXT("HUDSkull")), false);
			if (UTexture2D* LoadedSkull = TexPool->GetLoadedTexture(SkullSoft))
			{
				ConfigureCrispUISprite(LoadedSkull);
			}
			TexPool->RequestTexture(SkullSoft, this, FName(TEXT("HUDSkullConfig")), [ConfigureCrispUISprite](UTexture2D* LoadedSkull)
			{
				ConfigureCrispUISprite(LoadedSkull);
			});
		}
	}
	// Clown sprite brush (cowardice gates taken; same size as skull).
	if (!ClownBrush.IsValid())
	{
		ClownBrush = MakeShared<FSlateBrush>();
		ClownBrush->DrawAs = ESlateBrushDrawType::Image;
		ClownBrush->ImageSize = FVector2D(38.f, 38.f);
		ClownBrush->Tiling = ESlateBrushTileType::NoTile;
		ClownBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> ClownSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/CLOWN.CLOWN")));
			T66SlateTexture::BindSharedBrushAsync(TexPool, ClownSoft, this, ClownBrush, FName(TEXT("HUDClown")), false);
			if (UTexture2D* LoadedClown = TexPool->GetLoadedTexture(ClownSoft))
			{
				ConfigureCrispUISprite(LoadedClown);
			}
			TexPool->RequestTexture(ClownSoft, this, FName(TEXT("HUDClownConfig")), [ConfigureCrispUISprite](UTexture2D* LoadedClown)
			{
				ConfigureCrispUISprite(LoadedClown);
			});
		}
	}
	for (int32 i = 0; i < IdolSlotBrushes.Num(); ++i)
	{
		IdolSlotBrushes[i] = MakeShared<FSlateBrush>();
		IdolSlotBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		IdolSlotBrushes[i]->ImageSize = FVector2D(GT66BottomLeftIdolSlotSize, GT66BottomLeftIdolSlotSize);
	}
	for (int32 i = 0; i < InventorySlotBrushes.Num(); ++i)
	{
		InventorySlotBrushes[i] = MakeShared<FSlateBrush>();
		InventorySlotBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		// Size is assigned below where InvSlotSize is known; keep a safe default now.
		InventorySlotBrushes[i]->ImageSize = FVector2D(36.f, 36.f);
	}

	// Difficulty row (5-slot skull sprites).
	static constexpr float MinimapWidth = MinimapPanelWidth;
	static constexpr float DiffGap = 1.f;
	static constexpr float DiffSize = 30.f;
	TSharedRef<SHorizontalBox> DifficultyRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < DifficultyBorders.Num(); ++i)
	{
		TSharedPtr<SImage> DiffImg;
		DifficultyRowRef->AddSlot()
			.AutoWidth()
			.Padding(i == 0 ? 0.f : DiffGap, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(DiffSize)
				.HeightOverride(DiffSize)
				[
					SAssignNew(DiffImg, SImage)
					.Image(SkullBrush.Get())
					.ColorAndOpacity(FLinearColor::White)
					.Visibility(EVisibility::Collapsed) // Start hidden; skulls appear one-by-one
				]
			];
		DifficultyImages[i] = DiffImg;
	}

	// Cowardice row (5-slot clown sprites, below skulls; one per cowardice gate taken).
	TSharedRef<SHorizontalBox> CowardiceRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < ClownImages.Num(); ++i)
	{
		TSharedPtr<SImage> ClownImg;
		CowardiceRowRef->AddSlot()
			.AutoWidth()
			.Padding(i == 0 ? 0.f : DiffGap, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(DiffSize)
				.HeightOverride(DiffSize)
				[
					SAssignNew(ClownImg, SImage)
					.Image(ClownBrush.Get())
					.ColorAndOpacity(FLinearColor::White)
					.Visibility(EVisibility::Collapsed)
				]
			];
		ClownImages[i] = ClownImg;
	}

	// Build a 10-heart display as two rows of five segments.
	static constexpr float HeartWidth = GT66DisplayedHeartWidth;
	static constexpr float HeartHeight = GT66DisplayedHeartHeight;
	static constexpr float HeartPad = GT66DisplayedHeartColumnGap;
	static constexpr float HeartRowGap = GT66DisplayedHeartRowGap;
	static constexpr float TopStripPanelHeight = GT66BottomLeftAbilityBoxSize;
	TSharedRef<SHorizontalBox> TopHeartsRowRef = SNew(SHorizontalBox);
	TSharedRef<SHorizontalBox> BottomHeartsRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < GT66DisplayedHeartCount; ++i)
	{
		TSharedPtr<SBox> HeartFillBox;
		TSharedPtr<SImage> HeartImg;
		TSharedRef<SHorizontalBox> TargetRow = (i < 5) ? TopHeartsRowRef : BottomHeartsRowRef;
		TargetRow->AddSlot()
			.FillWidth(1.f)
			.Padding((i % 5) > 0 ? HeartPad : 0.f, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(HeartWidth)
				.HeightOverride(HeartHeight)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Image(HeartBrush.Get())
						.ColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.28f, 0.35f))
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					[
						SAssignNew(HeartFillBox, SBox)
						.WidthOverride(HeartWidth)
						.HeightOverride(HeartHeight)
						.Clipping(EWidgetClipping::ClipToBounds)
						[
							SNew(SBox)
							.WidthOverride(HeartWidth)
							.HeightOverride(HeartHeight)
							[
								SAssignNew(HeartImg, SImage)
								.Image(HeartBrush.Get())
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
				]
			];
		HeartFillBoxes[i] = HeartFillBox;
		HeartImages[i] = HeartImg;
	}
	TSharedRef<SWidget> HeartsRowRef =
		SNew(SBox)
		.WidthOverride(GT66DisplayedHeartAreaWidth)
		.HeightOverride(GT66DisplayedHeartAreaHeight)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Fill)
			[
				TopHeartsRowRef
			]
			+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Fill).Padding(0.f, HeartRowGap, 0.f, 0.f)
			[
				BottomHeartsRowRef
			]
		];

	TSharedRef<SWidget> QuickReviveIconRowRef =
		SAssignNew(QuickReviveIconRowBox, SBox)
		.Visibility(EVisibility::Collapsed)
		.WidthOverride(GT66BottomLeftAbilityBoxSize)
		.HeightOverride(GT66BottomLeftAbilityBoxSize)
		[
			SAssignNew(QuickReviveIconImage, SImage)
			.Image(QuickReviveBrush.Get())
			.ColorAndOpacity(FLinearColor::White)
		];

	// Status effect dots row (above hearts): burn / chill / curse
	TSharedRef<SHorizontalBox> StatusDotsRowRef =
		SNew(SHorizontalBox)
		.Visibility(EVisibility::Collapsed);
	for (int32 i = 0; i < 3; ++i)
	{
		TSharedPtr<SBox> DotBox;
		TSharedPtr<ST66DotWidget> Dot;
		StatusDotsRowRef->AddSlot()
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SAssignNew(DotBox, SBox)
				.WidthOverride(8.f)
				.HeightOverride(8.f)
				.Visibility(EVisibility::Collapsed)
				[
					SAssignNew(Dot, ST66DotWidget)
				]
			];
		StatusEffectDotBoxes[i] = DotBox;
		StatusEffectDots[i] = Dot;
	}

	// Idol slots: 2x2 grid sized to match the stats panel footprint.
	TSharedRef<SGridPanel> IdolSlotsRef = SNew(SGridPanel);
	static constexpr int32 IdolColumns = 2;
	static constexpr float IdolSlotPad = GT66BottomLeftIdolSlotPad;
	static constexpr float IdolSlotSize = GT66BottomLeftIdolSlotSize;
	const float IdolPanelMinWidth = GT66BottomLeftSidePanelWidth;
	const float PortraitPanelSize = GT66BottomLeftPortraitPanelSize;
	const float InventoryPanelVisibleWidth = BottomRightInventoryPanelWidth;
	const float InventoryPanelVisibleHeight = BottomRightInventoryPanelHeight;
	const float AbilityColumnWidth = GT66BottomLeftAbilityBoxSize;
	const float AbilityIconSize = GT66BottomLeftAbilityBoxSize;
	const float AbilityLabelBarHeight = 16.f;
	const float AbilityInputBadgeWidth = 28.f;
	const float AbilityInputBadgeHeight = 18.f;
	const float AbilityIconInset = 6.f;
	const float BottomLeftColumnGap = 0.f;
	const FLinearColor BottomLeftPanelOuterColor(0.f, 0.f, 0.f, 0.95f);
	const FLinearColor BottomLeftPanelInnerColor(0.f, 0.f, 0.f, 1.f);
	const FLinearColor BottomLeftPanelTitleColor = FT66Style::TextMuted();
	const FLinearColor BottomLeftPanelDividerColor = FT66Style::Border() * FLinearColor(1.f, 1.f, 1.f, 0.6f);
	const FLinearColor IdolSectionBorderColor(0.45f, 0.63f, 0.78f, 0.95f);
	const FLinearColor PortraitSectionBorderColor(0.72f, 0.58f, 0.32f, 0.95f);
	const FLinearColor AbilitySectionBorderColor(0.54f, 0.74f, 0.94f, 0.95f);
	const FLinearColor HeartsSectionBorderColor(0.72f, 0.34f, 0.34f, 0.95f);
	const FLinearColor SharedSectionFillColor(0.03f, 0.03f, 0.04f, 1.f);
	const FLinearColor LevelTextColor = bDotaTheme ? FT66Style::Accent2() : FLinearColor(0.90f, 0.75f, 0.20f, 1.f);
	TSharedRef<SWidget> LevelBadgeRef =
		SNew(SBox)
		.WidthOverride(GT66BottomLeftLevelBadgeSize)
		.HeightOverride(GT66BottomLeftLevelBadgeSize)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SAssignNew(LevelRingWidget, ST66RingWidget)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(LevelText, STextBlock)
				.Text(FText::AsNumber(1))
				.Font(FT66Style::Tokens::FontBold(11))
				.ColorAndOpacity(LevelTextColor)
				.Justification(ETextJustify::Center)
			]
		];
	auto MakeBottomLeftBlackPanel = [&](const FText& Title, const TSharedRef<SWidget>& Content, const FMargin& InnerPadding) -> TSharedRef<SWidget>
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BottomLeftPanelOuterColor)
				.Padding(3.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BottomLeftPanelInnerColor)
					.Padding(InnerPadding)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 2.f)
						[
							SNew(STextBlock)
							.Text(Title)
							.Font(FT66Style::Tokens::FontBold(13))
							.ColorAndOpacity(BottomLeftPanelTitleColor)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(4.f, 0.f, 4.f, 3.f)
						[
							SNew(SBox)
							.HeightOverride(1.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(BottomLeftPanelDividerColor)
							]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							Content
						]
					]
				]
			];
	};
	auto MakeBottomLeftBlackPanelNoTitle = [&](const TSharedRef<SWidget>& Content, const FMargin& InnerPadding) -> TSharedRef<SWidget>
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BottomLeftPanelOuterColor)
				.Padding(3.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BottomLeftPanelInnerColor)
					.Padding(InnerPadding)
					[
						Content
					]
				]
			];
	};
	auto MakeBottomLeftSectionPanel = [&](const TSharedRef<SWidget>& Content, const FMargin& InnerPadding, const FLinearColor& BorderColor) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(BorderColor)
			.Padding(2.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(SharedSectionFillColor)
				.Padding(InnerPadding)
				[
					Content
				]
			];
	};
	auto MakeCurrencyReadout = [&](const TSharedPtr<FSlateBrush>& IconBrush, TSharedPtr<STextBlock>& OutValueText, const FText& InitialValue, const FLinearColor& ValueColor) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(18.f)
				.HeightOverride(18.f)
				[
					SNew(SImage)
					.Image(IconBrush.Get())
					.ColorAndOpacity(FLinearColor::White)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SAssignNew(OutValueText, STextBlock)
				.Text(InitialValue)
				.Font(FT66Style::Tokens::FontBold(13))
				.ColorAndOpacity(ValueColor)
			];
	};
	for (int32 i = 0; i < UT66IdolManagerSubsystem::MaxEquippedIdolSlots; ++i)
	{
		TSharedPtr<SBorder> IdolBorder;
		const int32 Row = i / IdolColumns;
		const int32 Col = i % IdolColumns;
		IdolSlotsRef->AddSlot(Col, Row)
			.Padding(IdolSlotPad)
			[
				SAssignNew(IdolSlotContainers[i], SBox)
				.WidthOverride(IdolSlotSize)
				.HeightOverride(IdolSlotSize)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
					SAssignNew(IdolBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(SlotOuterColor)
					.Padding(1.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(SlotFrameColor)
							.Padding(1.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(SlotFillColor)
							]
						]
						+ SOverlay::Slot()
						.VAlign(VAlign_Top)
						.Padding(1.f, 1.f, 1.f, 0.f)
						[
							SNew(SBox)
							.HeightOverride(2.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.97f, 1.0f, bDotaTheme ? 0.12f : 0.08f))
							]
						]
						+ SOverlay::Slot()
						.VAlign(VAlign_Bottom)
						.Padding(1.f, 0.f, 1.f, 1.f)
						[
							SNew(SBox)
							.HeightOverride(2.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.42f))
							]
						]
					]
					]
					+ SOverlay::Slot()
					[
						SAssignNew(IdolSlotImages[i], SImage)
						.Image(IdolSlotBrushes.IsValidIndex(i) && IdolSlotBrushes[i].IsValid() ? IdolSlotBrushes[i].Get() : nullptr)
						.ColorAndOpacity(FLinearColor::White)
						.Visibility(EVisibility::Collapsed)
					]
				]
			];
		IdolSlotBorders[i] = IdolBorder;
	}

	// Inventory: fixed two-row strip, stretched edge-to-edge so the full inventory fits without scrolling.
	static constexpr int32 InvCols = 10;
	const int32 InvRows = FMath::Max(1, FMath::DivideAndRoundUp(InventorySlotBorders.Num(), InvCols));
	const float InvSlotSize = BottomRightInventorySlotSize;
	static constexpr float InvSlotPad = 2.f;
	const FLinearColor InvSlotBorderColor = SlotFrameColor;
	for (int32 i = 0; i < InventorySlotBrushes.Num(); ++i)
	{
		if (InventorySlotBrushes[i].IsValid())
		{
			InventorySlotBrushes[i]->ImageSize = FVector2D(InvSlotSize - 4.f, InvSlotSize - 4.f);
		}
	}
	TSharedRef<SVerticalBox> InvGridRows = SNew(SVerticalBox);
	int32 SlotIndex = 0;
	for (int32 Row = 0; Row < InvRows; ++Row)
	{
		TSharedRef<SHorizontalBox> RowBox = SNew(SHorizontalBox);
		for (int32 Col = 0; Col < InvCols; ++Col)
		{
			if (!InventorySlotContainers.IsValidIndex(SlotIndex))
			{
				break;
			}
			TSharedPtr<SBorder> SlotBorder;
			TSharedPtr<SImage> SlotImage;
			const int32 ThisSlotIndex = SlotIndex;
			RowBox->AddSlot()
				.FillWidth(1.f)
				.Padding(InvSlotPad, InvSlotPad)
				[
					SAssignNew(InventorySlotContainers[ThisSlotIndex], SBox)
					.HeightOverride(InvSlotSize)
					[
						SNew(SOverlay)
						// Transparent slot bg with thin border outline
						+ SOverlay::Slot()
						[
							SAssignNew(SlotBorder, SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(SlotOuterColor)
							.Padding(1.f)
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(InvSlotBorderColor)
									.Padding(1.f)
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(SlotFillColor)
									]
								]
								+ SOverlay::Slot()
								.VAlign(VAlign_Top)
								.Padding(1.f, 1.f, 1.f, 0.f)
								[
									SNew(SBox)
									.HeightOverride(2.f)
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(FLinearColor(0.95f, 0.97f, 1.0f, bDotaTheme ? 0.12f : 0.08f))
									]
								]
								+ SOverlay::Slot()
								.VAlign(VAlign_Bottom)
								.Padding(1.f, 0.f, 1.f, 1.f)
								[
									SNew(SBox)
									.HeightOverride(2.f)
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.42f))
									]
								]
							]
						]
						// Item icon on top
						+ SOverlay::Slot()
						.Padding(2.f)
						[
							SAssignNew(SlotImage, SImage)
							.Image(InventorySlotBrushes.IsValidIndex(ThisSlotIndex) && InventorySlotBrushes[ThisSlotIndex].IsValid()
								? InventorySlotBrushes[ThisSlotIndex].Get()
								: nullptr)
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				];

			if (InventorySlotBorders.IsValidIndex(ThisSlotIndex))
			{
				InventorySlotBorders[ThisSlotIndex] = SlotBorder;
			}
			if (InventorySlotImages.IsValidIndex(ThisSlotIndex))
			{
				InventorySlotImages[ThisSlotIndex] = SlotImage;
			}
			SlotIndex++;
		}
		InvGridRows->AddSlot().AutoHeight().HAlign(HAlign_Fill)[ RowBox ];
	}
	TSharedRef<SWidget> InvGridRef =
		SNew(SBox)
		.HAlign(HAlign_Fill)
		[
			InvGridRows
		];
	auto MakeInventoryEconomySection = [&](const FLinearColor& GoldValueColor, const FLinearColor& DebtValueColor, const FLinearColor& NetWorthValueColor, const FLinearColor& DividerColor) -> TSharedRef<SWidget>
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(2.f, 0.f, 2.f, 4.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 14.f, 0.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.GameplayHUD", "InventoryHeader", "Inventory"))
					.Font(FT66Style::Tokens::FontBold(12))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
				[
					MakeCurrencyReadout(GoldCurrencyBrush, GoldText, GoldInit, GoldValueColor)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
				[
					MakeCurrencyReadout(DebtCurrencyBrush, DebtText, OweInit, DebtValueColor)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					MakeCurrencyReadout(DebtCurrencyBrush, NetWorthText, NetWorthInit, NetWorthValueColor)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SSpacer)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				SNew(SBox)
				.HeightOverride(1.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(DividerColor)
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				InvGridRef
			];
	};

	const TAttribute<FMargin> TopCenterBossPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 12.f, 0.f, 0.f);
	});

	const TAttribute<FMargin> TopCenterLootPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 48.f, 0.f, 0.f);
	});

	const TAttribute<FMargin> TopLeftHudPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(12.f, 12.f, 0.f, 0.f);
	});

	const TAttribute<FMargin> TopRightHudPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 12.f, 12.f, 0.f);
	});

	const TAttribute<FMargin> PauseLeftStatsPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(12.f, 112.f, 0.f, 0.f);
	});

	const TAttribute<FMargin> PauseRightAchievementsPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 286.f, 12.f, 0.f);
	});

	const TAttribute<FMargin> RightCenterHudPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 0.f, 12.f, 0.f);
	});

	const TAttribute<FMargin> BottomLeftHudPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(12.f, 0.f, 0.f, 12.f);
	});

	const TAttribute<FMargin> BottomLeftMediaPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(12.f, 0.f, 0.f, GT66MediaBottomOffset);
	});

	const TAttribute<FMargin> BottomRightInventoryPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 0.f, 12.f, 0.f);
	});

	const TAttribute<FMargin> BottomCenterAchievementPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 0.f, 0.f, 36.f);
	});

	const TAttribute<FMargin> BottomRightPickupPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(
			0.f,
			0.f,
			12.f,
			UT66GameplayHUDWidget::BottomRightInventoryPanelHeight + UT66GameplayHUDWidget::BottomRightPresentationGap);
	});

	const TAttribute<FOptionalSize> FullMapWidthAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66Style::GetSafeFrameSize();
		const float Width = FMath::Clamp(SafeFrame.X - 72.f, 720.f, 1100.f);
		return FOptionalSize(Width);
	});

	const TAttribute<FOptionalSize> FullMapHeightAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66Style::GetSafeFrameSize();
		const float Height = FMath::Clamp(SafeFrame.Y - 88.f, 420.f, 680.f);
		return FOptionalSize(Height);
	});

	TSharedRef<SOverlay> ChestRewardArtOverlay = SNew(SOverlay);
	ChestRewardArtOverlay->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SAssignNew(ChestRewardClosedBox, SBox)
		.WidthOverride(104.f)
		.HeightOverride(104.f)
		[
			SAssignNew(ChestRewardClosedImage, SImage)
			.Image(ChestRewardClosedBrush.Get())
			.ColorAndOpacity(FLinearColor::White)
		]
	];
	ChestRewardArtOverlay->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
		[
			SAssignNew(ChestRewardOpenBox, SBox)
			.WidthOverride(112.f)
			.HeightOverride(112.f)
			[
				SAssignNew(ChestRewardOpenImage, SImage)
				.Image(ChestRewardOpenBrush.Get())
				.ColorAndOpacity(FLinearColor::White)
			]
		];
	for (int32 CoinIndex = 0; CoinIndex < ChestRewardCoinCount; ++CoinIndex)
	{
		ChestRewardArtOverlay->AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(ChestRewardCoinBoxes[CoinIndex], SBox)
			.WidthOverride(18.f)
			.HeightOverride(18.f)
			.Visibility(EVisibility::Collapsed)
			[
				SAssignNew(ChestRewardCoinImages[CoinIndex], SImage)
				.Image(GoldCurrencyBrush.Get())
				.ColorAndOpacity(FLinearColor::White)
			]
		];
	}

	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(PauseBackdropBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.96f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(TopCenterBossPadding)
		[
			SAssignNew(BossBarContainerBox, SBox)
			.WidthOverride(BossBarWidth)
			.Visibility(EVisibility::Collapsed)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(BossBarBackgroundColor)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(BossBarFillBox, SBox)
						.HeightOverride(28.f)
						.WidthOverride(BossBarWidth)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(BossBarFillColor)
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(BossBarText, STextBlock)
						.Text(FText::Format(
							NSLOCTEXT("T66.Common", "Fraction", "{0}/{1}"),
							FText::AsNumber(100),
							FText::AsNumber(100)))
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 4.f, 0.f, 0.f)
				[
					SAssignNew(BossPartBarsBox, SVerticalBox)
					.Visibility(EVisibility::Collapsed)
				]
			]
		]
		// Top-center loot prompt (non-blocking)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(TopCenterLootPadding)
		[
			SAssignNew(LootPromptBox, SBox)
			.WidthOverride(760.f)
			.HeightOverride(40.f)
			.Visibility(EVisibility::Collapsed)
			[
				SAssignNew(LootPromptBorder, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(PromptBackgroundColor)
				.Padding(10.f, 6.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(28.f)
						.HeightOverride(28.f)
						[
							SAssignNew(LootPromptIconImage, SImage)
							.Image(LootPromptIconBrush.Get())
							.ColorAndOpacity(FLinearColor::White)
							.Visibility(EVisibility::Collapsed)
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SAssignNew(LootPromptText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(14))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				]
			]
		]
		// (Central countdown timer removed — stage timer info available in top-left stats)
		// In-world NPC dialogue (positioned via RenderTransform; hidden by default)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		[
			SAssignNew(WorldDialogueBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.RenderTransform(FSlateRenderTransform(FVector2D(0.f, 0.f)))
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(WorldDialogueOptionBorders[0], SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(DialogueBackgroundColor)
						.Padding(10.f, 6.f)
						[
							SAssignNew(WorldDialogueOptionTexts[0], STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
					[
						SAssignNew(WorldDialogueOptionBorders[1], SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(DialogueBackgroundColor)
						.Padding(10.f, 6.f)
						[
							SAssignNew(WorldDialogueOptionTexts[1], STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
					[
						SAssignNew(WorldDialogueOptionBorders[2], SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(DialogueBackgroundColor)
						.Padding(10.f, 6.f)
						[
							SAssignNew(WorldDialogueOptionTexts[2], STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
				,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(12.f)
			)
			]
		]
		// Media viewer panel. Source is selected from Settings.
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(PauseLeftStatsPadding)
		[
			SAssignNew(PauseStatsPanelBox, SBox)
			.Visibility(EVisibility::Collapsed)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(PauseRightAchievementsPadding)
		[
			SAssignNew(PauseAchievementsPanelBox, SBox)
			.Visibility(EVisibility::Collapsed)
		]
		// Media viewer panel. Source is selected from Settings.
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(BottomLeftMediaPadding)
		[
			SAssignNew(TikTokPlaceholderBox, SBox)
			.WidthOverride(GT66MediaPanelW)
			.HeightOverride(GT66MediaPanelH)
			[
				FT66Style::MakePanel(
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 0.55f))
					.Padding(2.f)
					[
						SAssignNew(MediaViewerVideoBox, SBox)
						[
							SAssignNew(TikTokContentBox, SBox)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 1.f))
							]
						]
					]
				,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(6.f)
				)
			]
		]
		// Top-left stats (score + speedrun time) — themed panel and text
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(TopLeftHudPadding)
		[
			bDotaTheme
				? FT66Style::MakeHudPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(ScoreLabelText)
							.Font(FT66Style::Tokens::FontBold(10))
							.ColorAndOpacity(FSlateColor(FT66Style::Text()))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
						[
							SAssignNew(ScoreText, STextBlock)
							.Text(FText::AsNumber(0))
							.Font(FT66Style::Tokens::FontBold(10))
							.ColorAndOpacity(FSlateColor(FT66Style::Text()))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(ScorePacingText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScorePacingDefault", "Score Pace --"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(ScoreTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScoreTargetDefault", "Score to Beat --"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunDefault", "Time 0:00.00"))
						.Font(FT66Style::Tokens::FontBold(10))
						.ColorAndOpacity(FSlateColor(FT66Style::Text()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunPacingText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "TimePacingDefault", "Time Pace --:--.--"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetDefault", "Time to Beat --:--.--"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					],
					FMargin(6.f, 4.f))
				: FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(ScoreLabelText)
							.Font(FT66Style::Tokens::FontBold(10))
							.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SAssignNew(ScoreText, STextBlock)
							.Text(FText::AsNumber(0))
							.Font(FT66Style::Tokens::FontBold(10))
							.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
					[
						SAssignNew(ScorePacingText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScorePacingDefault", "Score Pace --"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::TextMuted))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(ScoreTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScoreTargetDefault", "Score to Beat --"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::TextMuted))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunDefault", "Time 0:00.00"))
						.Font(FT66Style::Tokens::FontBold(10))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunPacingText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "TimePacingDefault", "Time Pace --:--.--"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::TextMuted))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetDefault", "Time to Beat --:--.--"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::TextMuted))
						.Visibility(EVisibility::Collapsed)
					],
					FT66PanelParams(ET66PanelType::Panel).SetPadding(6.f)
				)
		]

		// Wheel spin HUD animation (same lane/layout as chest reward)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(BottomRightPickupPadding)
		[
			SAssignNew(WheelSpinBox, SBox)
			.WidthOverride(PickupCardWidth)
			.HeightOverride(PickupCardHeight)
			.Visibility(EVisibility::Collapsed)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.24f, 0.24f, 0.28f, 1.f))
				.Padding(2.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.f))
					.Padding(0.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor::Black)
							.Padding(FMargin(8.f, 6.f))
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
									[
										SNew(SBox)
										.WidthOverride(18.f)
										.HeightOverride(18.f)
										[
											SNew(SImage)
											.Image(GoldCurrencyBrush.Get())
											.ColorAndOpacity(FLinearColor::White)
										]
									]
									+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
									[
										SAssignNew(WheelSpinText, STextBlock)
										.Text(FText::GetEmpty())
										.Font(FT66Style::Tokens::FontBold(18))
										.ColorAndOpacity(FLinearColor(0.98f, 0.83f, 0.24f, 1.f))
										.Justification(ETextJustify::Center)
									]
								]
							]
						]
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.05f, 1.f))
							.Padding(FMargin(6.f))
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SBox)
									.WidthOverride(124.f)
									.HeightOverride(124.f)
									[
										SAssignNew(WheelSpinDisk, SImage)
										.Image(&WheelTextureBrush)
									]
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 1.f))
							.Padding(FMargin(8.f, 4.f))
							[
								SAssignNew(WheelSpinSkipText, STextBlock)
								.Text(FText::GetEmpty())
								.Font(FT66Style::Tokens::FontBold(11))
								.ColorAndOpacity(FLinearColor::White)
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
			]
		]

		// Bottom-left portrait stack with a tighter, uniform panel footprint.
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(BottomLeftHudPadding)
		[
			SAssignNew(BottomLeftHUDBox, SBox)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Bottom)
				[
					SAssignNew(IdolSlotsPanelBox, SBox)
					[
						MakeBottomLeftBlackPanel(
							NSLOCTEXT("T66.GameplayHUD", "IdolsHeader", "IDOLS"),
							SNew(SBox)
							.WidthOverride(IdolPanelMinWidth)
							[
								MakeBottomLeftSectionPanel(
									SNew(SBox)
									.HAlign(HAlign_Center)
									[
										IdolSlotsRef
									],
									FMargin(3.f),
									IdolSectionBorderColor)
							],
							FMargin(2.f, 2.f))
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(BottomLeftColumnGap, 0.f, 0.f, 0.f)
				.VAlign(VAlign_Bottom)
				[
					SAssignNew(PortraitStatPanelBox, SBox)
					[
						MakeBottomLeftBlackPanelNoTitle(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SBox)
								.WidthOverride(PortraitPanelSize)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot().AutoHeight()
									[
										SNew(SBox)
										.WidthOverride(PortraitPanelSize)
										.HeightOverride(TopStripPanelHeight)
										[
											MakeBottomLeftSectionPanel(
												SNew(SOverlay)
												+ SOverlay::Slot()
												[
													HeartsRowRef
												]
												+ SOverlay::Slot()
												.HAlign(HAlign_Center)
												.VAlign(VAlign_Top)
												.Padding(0.f, 2.f, 0.f, 0.f)
												[
													StatusDotsRowRef
												],
												FMargin(0.f),
												HeartsSectionBorderColor)
										]
									]
									+ SVerticalBox::Slot().AutoHeight()
									[
										SNew(SBox)
										.WidthOverride(PortraitPanelSize)
										.HeightOverride(PortraitPanelSize)
										[
											MakeBottomLeftSectionPanel(
												bDotaTheme
													? StaticCastSharedRef<SWidget>(
														SNew(SOverlay)
														+ SOverlay::Slot()
														[
															SNew(SBorder)
															.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
															.BorderBackgroundColor(FT66Style::PanelOuter())
															.Padding(1.f)
															[
																SNew(SBorder)
																.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
																.BorderBackgroundColor(FT66Style::Border())
																.Padding(1.f)
																[
																	SAssignNew(PortraitBorder, SBorder)
																	.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
																	.BorderBackgroundColor(FT66Style::PanelInner())
																]
															]
														]
														+ SOverlay::Slot()
														[
															SAssignNew(PortraitImage, SImage)
															.Image(PortraitBrush.Get())
															.ColorAndOpacity(FLinearColor::White)
															.Visibility(EVisibility::Collapsed)
														]
														+ SOverlay::Slot()
														.HAlign(HAlign_Right)
														.VAlign(VAlign_Top)
														.Padding(4.f)
														[
															LevelBadgeRef
														]
														+ SOverlay::Slot()
														.HAlign(HAlign_Center)
														.VAlign(VAlign_Center)
														[
															SAssignNew(PortraitPlaceholderText, STextBlock)
															.Text(PortraitLabel)
															.Font(FT66Style::Tokens::FontBold(11))
															.ColorAndOpacity(FT66Style::TextMuted())
															.Justification(ETextJustify::Center)
															.Visibility(EVisibility::Visible)
														]
														)
													: StaticCastSharedRef<SWidget>(
														SNew(SOverlay)
														+ SOverlay::Slot()
														[
															SAssignNew(PortraitBorder, SBorder)
															.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
															.BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.14f, 1.f))
														]
														+ SOverlay::Slot()
														[
															SAssignNew(PortraitImage, SImage)
															.Image(PortraitBrush.Get())
															.ColorAndOpacity(FLinearColor::White)
															.Visibility(EVisibility::Collapsed)
														]
														+ SOverlay::Slot()
														.HAlign(HAlign_Right)
														.VAlign(VAlign_Top)
														.Padding(4.f)
														[
															LevelBadgeRef
														]
														+ SOverlay::Slot()
														.HAlign(HAlign_Center)
														.VAlign(VAlign_Center)
														[
															SAssignNew(PortraitPlaceholderText, STextBlock)
															.Text(PortraitLabel)
															.Font(FT66Style::Tokens::FontBold(11))
															.ColorAndOpacity(FT66Style::Tokens::TextMuted)
															.Justification(ETextJustify::Center)
															.Visibility(EVisibility::Visible)
														]
														),
												FMargin(2.f),
												PortraitSectionBorderColor)
										]
									]
								]
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(BottomLeftColumnGap, 0.f, 0.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(AbilityColumnWidth)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot().AutoHeight()
									[
										SNew(SBox)
										.WidthOverride(AbilityColumnWidth)
										.HeightOverride(TopStripPanelHeight)
										[
											MakeBottomLeftSectionPanel(
												SNew(SBox)
												.HAlign(HAlign_Center)
												.VAlign(VAlign_Center)
												[
													QuickReviveIconRowRef
												],
												FMargin(3.f),
												AbilitySectionBorderColor)
										]
									]
									+ SVerticalBox::Slot().AutoHeight()
									[
										SNew(SBox)
										.WidthOverride(AbilityColumnWidth)
										.HeightOverride(PortraitPanelSize)
										[
											MakeBottomLeftSectionPanel(
												SNew(SVerticalBox)
												+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, GT66BottomLeftAbilityGap)
												[
													SNew(SBox)
													.WidthOverride(AbilityIconSize)
													.HeightOverride(AbilityIconSize)
													[
														SAssignNew(UltimateBorder, SBorder)
														.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
														.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.05f, 1.f))
														.Padding(0.f)
														[
															SNew(SOverlay)
															+ SOverlay::Slot()
															.Padding(AbilityIconInset, AbilityIconInset, AbilityIconInset, AbilityLabelBarHeight + 2.f)
															[
																SNew(SScaleBox)
																.Stretch(EStretch::ScaleToFit)
																[
																	SAssignNew(UltimateImage, SImage)
																	.Image(UltimateBrush.Get())
																	.ColorAndOpacity(FLinearColor::White)
																]
															]
															+ SOverlay::Slot()
															[
																SAssignNew(UltimateCooldownOverlay, SBorder)
																.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
																.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.65f))
																.HAlign(HAlign_Center)
																.VAlign(VAlign_Center)
																.Visibility(EVisibility::Collapsed)
																[
																	SAssignNew(UltimateText, STextBlock)
																	.Text(FText::GetEmpty())
																	.Font(FT66Style::Tokens::FontBold(16))
																	.ColorAndOpacity(FLinearColor::White)
																	.Justification(ETextJustify::Center)
																]
															]
															+ SOverlay::Slot()
															.HAlign(HAlign_Right)
															.VAlign(VAlign_Top)
															[
																SNew(SBox)
																.WidthOverride(AbilityInputBadgeWidth)
																.HeightOverride(AbilityInputBadgeHeight)
																[
																	SNew(SBorder)
																	.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
																	.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.75f))
																	.HAlign(HAlign_Center)
																	.VAlign(VAlign_Center)
																	[
																		SAssignNew(UltimateInputHintText, STextBlock)
																		.Text(NSLOCTEXT("T66.GameplayHUD", "UltKeybindDefault", "R"))
																		.Font(FT66Style::Tokens::FontBold(8))
																		.ColorAndOpacity(FLinearColor::White)
																		.Justification(ETextJustify::Center)
																	]
																]
															]
															+ SOverlay::Slot()
															.VAlign(VAlign_Bottom)
															[
																SNew(SBorder)
																.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
																.BorderBackgroundColor(FLinearColor::Black)
																.Padding(FMargin(0.f, 1.f))
																[
																	SNew(STextBlock)
																	.Text(NSLOCTEXT("T66.GameplayHUD", "UltLabelBar", "ULT"))
																	.Font(FT66Style::Tokens::FontBold(8))
																	.ColorAndOpacity(FLinearColor::White)
																	.Justification(ETextJustify::Center)
																]
															]
														]
													]
												]
												+ SVerticalBox::Slot().AutoHeight()
												[
													SNew(SBox)
													.WidthOverride(AbilityIconSize)
													.HeightOverride(AbilityIconSize)
													[
														SNew(SOverlay)
														+ SOverlay::Slot()
														[
															SAssignNew(PassiveBorder, SBorder)
															.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
															.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.05f, 1.f))
															.Padding(0.f)
															[
																SNew(SOverlay)
																+ SOverlay::Slot()
																.Padding(AbilityIconInset, AbilityIconInset, AbilityIconInset, AbilityLabelBarHeight + 2.f)
																[
																	SNew(SScaleBox)
																	.Stretch(EStretch::ScaleToFit)
																	[
																		SAssignNew(PassiveImage, SImage)
																		.Image(PassiveBrush.Get())
																		.ColorAndOpacity(FLinearColor::White)
																	]
																]
																+ SOverlay::Slot()
																.VAlign(VAlign_Bottom)
																[
																	SNew(SBorder)
																	.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
																	.BorderBackgroundColor(FLinearColor::Black)
																	.Padding(FMargin(0.f, 1.f))
																	[
																		SNew(STextBlock)
																		.Text(NSLOCTEXT("T66.GameplayHUD", "PassiveLabelBar", "PASSIVE"))
																		.Font(FT66Style::Tokens::FontBold(7))
																		.ColorAndOpacity(FLinearColor::White)
																		.Justification(ETextJustify::Center)
																	]
																]
															]
														]
														+ SOverlay::Slot()
														.HAlign(HAlign_Right)
														.VAlign(VAlign_Bottom)
														.Padding(0.f, 0.f, 4.f, AbilityLabelBarHeight + 4.f)
														[
															SAssignNew(PassiveStackBadgeBox, SBox)
															.WidthOverride(20.f)
															.HeightOverride(20.f)
															[
																SNew(SOverlay)
																+ SOverlay::Slot()
																[
																	SNew(SBorder)
																	.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Circle"))
																	.BorderBackgroundColor(FLinearColor(0.12f, 0.10f, 0.08f, 0.95f))
																	.Padding(0.f)
																	.HAlign(HAlign_Center)
																	.VAlign(VAlign_Center)
																	[
																		SAssignNew(PassiveStackText, STextBlock)
																		.Text(FText::AsNumber(0))
																		.Font(FT66Style::Tokens::FontBold(9))
																		.ColorAndOpacity(FLinearColor(0.95f, 0.75f, 0.25f, 1.f))
																		.Justification(ETextJustify::Center)
																	]
																]
															]
														]
													]
												],
												FMargin(2.f),
												AbilitySectionBorderColor)
										]
									]
								]
							],
							FMargin(2.f, 2.f))
					]
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(TopRightHudPadding)
		[
			SAssignNew(MinimapPanelBox, SBox)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBox)
					.WidthOverride(MinimapWidth)
					.HeightOverride(MinimapWidth)
					[
						bDotaTheme
							? FT66Style::MakeMinimapFrame(
								SAssignNew(MinimapWidget, ST66WorldMapWidget)
								.bMinimap(true)
								.bShowLabels(false),
								FMargin(8.f))
							: StaticCastSharedRef<SWidget>(
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									FT66Style::MakePanel(
										SAssignNew(MinimapWidget, ST66WorldMapWidget)
										.bMinimap(true)
										.bShowLabels(false),
										FT66PanelParams(ET66PanelType::Panel2).SetPadding(8.f)
									)
								])
					]
				]
				// Stage number + skulls beneath minimap, grouped in one compact black panel.
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)
				[
					SNew(SBox)
					.WidthOverride(MinimapWidth)
					[
						bDotaTheme
							? FT66Style::MakeHudPanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SAssignNew(StageText, STextBlock)
								.Text(StageInit)
								.Font(FT66Style::Tokens::FontBold(11))
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.Justification(ETextJustify::Center)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(DifficultyRowBox, SBox)
								.Visibility(EVisibility::Collapsed)
								[
									DifficultyRowRef
								]
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)
							[
								SAssignNew(CowardiceRowBox, SBox)
								.Visibility(EVisibility::Collapsed)
								[
									CowardiceRowRef
								]
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(ImmortalityButton, SButton)
								.Visibility(EVisibility::Collapsed)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnToggleImmortality))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								.ContentPadding(FMargin(5.f, 2.f))
								[
									SAssignNew(ImmortalityButtonText, STextBlock)
									.Text(NSLOCTEXT("T66.Dev", "ImmortalityOff", "IMMORTALITY: OFF"))
									.Font(FT66Style::Tokens::FontBold(7))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(PowerButton, SButton)
								.Visibility(EVisibility::Collapsed)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnTogglePower))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								.ContentPadding(FMargin(5.f, 2.f))
								[
									SAssignNew(PowerButtonText, STextBlock)
									.Text(NSLOCTEXT("T66.Dev", "PowerOff", "POWER: OFF"))
									.Font(FT66Style::Tokens::FontBold(7))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							],
							FMargin(6.f, 4.f))
							: FT66Style::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SAssignNew(StageText, STextBlock)
								.Text(StageInit)
								.Font(FT66Style::Tokens::FontBold(11))
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.Justification(ETextJustify::Center)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(DifficultyRowBox, SBox)
								.Visibility(EVisibility::Collapsed)
								[
									DifficultyRowRef
								]
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)
							[
								SAssignNew(CowardiceRowBox, SBox)
								.Visibility(EVisibility::Collapsed)
								[
									CowardiceRowRef
								]
							]
							// Dev toggles are hidden in the current HUD pass.
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(ImmortalityButton, SButton)
								.Visibility(EVisibility::Collapsed)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnToggleImmortality))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								.ContentPadding(FMargin(5.f, 2.f))
								[
									SAssignNew(ImmortalityButtonText, STextBlock)
									.Text(NSLOCTEXT("T66.Dev", "ImmortalityOff", "IMMORTALITY: OFF"))
									.Font(FT66Style::Tokens::FontBold(7))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
							// Dev toggles are hidden in the current HUD pass.
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(PowerButton, SButton)
								.Visibility(EVisibility::Collapsed)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnTogglePower))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								.ContentPadding(FMargin(5.f, 2.f))
								[
									SAssignNew(PowerButtonText, STextBlock)
									.Text(NSLOCTEXT("T66.Dev", "PowerOff", "POWER: OFF"))
									.Font(FT66Style::Tokens::FontBold(7))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							],
							FT66PanelParams(ET66PanelType::Panel).SetPadding(6.f))
					]
				]
			]
		]
		// Inventory panel bottom-right (Gold/Owe and grid); FPS is above idol panel on the left
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(BottomRightInventoryPadding)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(InventoryPanelBox, SBox)
				.WidthOverride(InventoryPanelVisibleWidth)
				.HeightOverride(InventoryPanelVisibleHeight)
				[
					bDotaTheme
						? FT66Style::MakeHudPanel(
							MakeInventoryEconomySection(
								FT66Style::Accent2(),
								FT66Style::Danger(),
								FT66Style::Tokens::Success,
								WithAlpha(FT66Style::Border(), 0.65f)),
							FMargin(8.f, 6.f))
						: StaticCastSharedRef<SWidget>(
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.30f, 0.38f, 0.35f, 0.85f))
								.Padding(3.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.08f, 0.14f, 0.12f, 0.92f))
									.Padding(FMargin(10.f, 8.f))
									[
										MakeInventoryEconomySection(
											FLinearColor(0.85f, 0.75f, 0.20f, 1.f),
											FT66Style::Tokens::Danger,
											FT66Style::Tokens::Success,
											FLinearColor(0.55f, 0.65f, 0.58f, 0.55f))
									]
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
							[
								SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top)
							[
								SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom)
							[
								SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom)
							[
								SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
								]
							])
				]
			]
		]
		// Achievement unlock notification (lower-center lane, clear of inventory and idol panels)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(BottomCenterAchievementPadding)
		[
			SAssignNew(AchievementNotificationBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.WidthOverride(280.f)
			[
				SAssignNew(AchievementNotificationBorder, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
				.Padding(3.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FT66Style::Tokens::Panel)
					.Padding(FMargin(10.f, 8.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SAssignNew(AchievementNotificationTitleText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.AutoWrapText(true)
							.WrapTextAt(256.f)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.GameplayHUD", "AchievementUnlocked", "Unlocked!"))
							.Font(FT66Style::Tokens::FontRegular(14))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
				]
			]
		]
		// Chest reward presentation (same lane as pickup card, non-pausing)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(BottomRightPickupPadding)
		[
			SAssignNew(ChestRewardBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.WidthOverride(PickupCardWidth)
			.HeightOverride(PickupCardHeight)
			[
				SAssignNew(ChestRewardTileBorder, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Tokens::Panel)
				.Padding(2.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.f))
					.Padding(0.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor::Black)
							.Padding(FMargin(8.f, 6.f))
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(SBox)
									.WidthOverride(18.f)
									.HeightOverride(18.f)
									[
										SNew(SImage)
										.Image(GoldCurrencyBrush.Get())
										.ColorAndOpacity(FLinearColor::White)
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
								[
									SAssignNew(ChestRewardCounterText, STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66Style::Tokens::FontBold(18))
									.ColorAndOpacity(FLinearColor(0.98f, 0.83f, 0.24f, 1.f))
								]
							]
						]
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.05f, 0.04f, 0.02f, 1.f))
							.Padding(FMargin(4.f, 6.f))
							[
								ChestRewardArtOverlay
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 1.f))
							.Padding(FMargin(8.f, 4.f))
							[
								SAssignNew(ChestRewardSkipText, STextBlock)
								.Text(FText::GetEmpty())
								.Font(FT66Style::Tokens::FontBold(11))
								.ColorAndOpacity(FLinearColor::White)
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
			]
		]
		// Pickup item card (right side, bottom of card just above inventory)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(BottomRightPickupPadding)
		[
			SAssignNew(PickupCardBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.WidthOverride(PickupCardWidth)
			.HeightOverride(PickupCardHeight)
			[
				SAssignNew(PickupCardTileBorder, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Tokens::Panel)
				.Padding(2.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.f))
					.Padding(0.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SAssignNew(PickupCardIconBorder, SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.04f, 0.04f, 0.05f, 1.f))
							.Padding(FMargin(6.f))
							[
								SNew(SBox)
								.HeightOverride(104.f)
								[
									SNew(SScaleBox)
									.Stretch(EStretch::ScaleToFit)
									.StretchDirection(EStretchDirection::Both)
									[
										SAssignNew(PickupCardIconImage, SImage)
										.Image(PickupCardIconBrush.Get())
										.ColorAndOpacity(FLinearColor::White)
										.Visibility(EVisibility::Collapsed)
									]
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor::Black)
							.Padding(FMargin(10.f, 8.f))
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SAssignNew(PickupCardNameText, STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66Style::Tokens::FontBold(12))
									.ColorAndOpacity(FLinearColor::White)
									.AutoWrapText(true)
									.WrapTextAt(PickupCardWidth - 20.f)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
								[
									SAssignNew(PickupCardDescText, STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66Style::Tokens::FontRegular(10))
									.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.92f))
									.AutoWrapText(true)
									.WrapTextAt(PickupCardWidth - 20.f)
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 1.f))
							.Padding(FMargin(8.f, 4.f))
							[
								SAssignNew(PickupCardSkipText, STextBlock)
								.Text(FText::GetEmpty())
								.Font(FT66Style::Tokens::FontBold(11))
								.ColorAndOpacity(FLinearColor::White)
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
			]
		]
		// Tutorial subtitle (guide dialogue)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 0.f, 92.f)
		[
			SNew(SBox)
			.MinDesiredWidth(860.f)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(TutorialSubtitleSpeakerText, STextBlock)
						.Visibility(EVisibility::Collapsed)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FLinearColor(0.95f, 0.72f, 0.38f, 1.f))
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(TutorialSubtitleBodyText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontRegular(18))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
						.WrapTextAt(820.f)
					]
				,
				FT66PanelParams(ET66PanelType::Panel)
					.SetPadding(FMargin(18.f, 12.f))
					.SetVisibility(EVisibility::Collapsed),
				&TutorialSubtitleBorder
			)
			]
		]
		// Tutorial hint (above crosshair)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0.f, -220.f, 0.f, 0.f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SAssignNew(TutorialHintLine1Text, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)
				[
					SAssignNew(TutorialHintLine2Text, STextBlock)
					.Visibility(EVisibility::Collapsed)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontRegular(14))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.Justification(ETextJustify::Center)
				]
			,
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 8.f)).SetVisibility(EVisibility::Collapsed),
			&TutorialHintBorder
		)
		]
		// Center crosshair (screen center; camera unchanged)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0.f)
		[
			SAssignNew(CenterCrosshairBox, SBox)
			.WidthOverride(28.f)
			.HeightOverride(28.f)
			[
				SAssignNew(CenterCrosshairWidget, ST66CrosshairWidget)
				.Locked(false)
			]
		]
		// Hero 1 scoped sniper overlay (first-person aim view + ult timers)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(ScopedSniperOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
			.Padding(0.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(ST66ScopedSniperWidget)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(0.f, 24.f, 0.f, 0.f)
				[
					FT66Style::MakePanel(
						SAssignNew(ScopedUltTimerText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(20))
						.ColorAndOpacity(FT66Style::Tokens::Text),
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(12.f, 8.f)))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Bottom)
				.Padding(0.f, 0.f, 0.f, 42.f)
				[
					FT66Style::MakePanel(
						SAssignNew(ScopedShotCooldownText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FT66Style::Tokens::Text),
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f, 8.f)))
				]
			]
		]
		// Quick Revive downed overlay
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(QuickReviveDownedOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.42f, 0.42f, 0.42f, 0.58f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(QuickReviveDownedText, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(28))
				.ColorAndOpacity(FLinearColor::White)
				.Justification(ETextJustify::Center)
			]
		]
		// Curse (visibility) overlay (always on top)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(CurseOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.05f, 0.0f, 0.08f, 0.40f))
		]
		// Full-screen map overlay (OpenFullMap / M)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(FullMapOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.78f))
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					FT66Style::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.Map", "Title", "MAP"))
								.Font(FT66Style::Tokens::FontBold(18))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.Map", "CloseHint", "[M] Close"))
								.Font(FT66Style::Tokens::FontBold(12))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBox)
							.WidthOverride(FullMapWidthAttr)
							.HeightOverride(FullMapHeightAttr)
							[
								FT66Style::MakePanel(
									SAssignNew(FullMapWidget, ST66WorldMapWidget)
									.bMinimap(false)
									.bShowLabels(true),
									FT66PanelParams(ET66PanelType::Panel2).SetPadding(10.f)
								)
							]
						]
					,
					FT66PanelParams(ET66PanelType::Panel)
				)
				]
			]
		];

	ApplyInventoryInspectMode();
	return Root;
}

static void T66_ApplyWorldDialogueSelection(
	const TArray<TSharedPtr<SBorder>>& OptionBorders,
	const TArray<TSharedPtr<STextBlock>>& OptionTexts,
	int32 SelectedIndex)
{
	for (int32 i = 0; i < OptionBorders.Num(); ++i)
	{
		const bool bSelected = (i == SelectedIndex);
		if (OptionBorders[i].IsValid())
		{
			OptionBorders[i]->SetBorderBackgroundColor(bSelected
				? (FT66Style::IsDotaTheme() ? FT66Style::SelectionFill() : FLinearColor(0.18f, 0.18f, 0.26f, 0.95f))
				: (FT66Style::IsDotaTheme() ? FT66Style::PromptBackground() : FLinearColor(0.06f, 0.06f, 0.08f, 0.85f)));
		}
		if (OptionTexts.IsValidIndex(i) && OptionTexts[i].IsValid())
		{
			OptionTexts[i]->SetColorAndOpacity(bSelected ? FT66Style::Tokens::Text : FT66Style::Tokens::TextMuted);
		}
	}
}

void UT66GameplayHUDWidget::ShowWorldDialogue(const TArray<FText>& Options, int32 SelectedIndex)
{
	if (!WorldDialogueBox.IsValid()) return;
	if (Options.Num() < 2) return;
	if (WorldDialogueOptionTexts.Num() < 3) return;

	for (int32 i = 0; i < 3; ++i)
	{
		const bool bHasOption = Options.IsValidIndex(i);
		if (WorldDialogueOptionTexts[i].IsValid())
		{
			WorldDialogueOptionTexts[i]->SetText(bHasOption ? Options[i] : FText::GetEmpty());
		}
		if (WorldDialogueOptionBorders.IsValidIndex(i) && WorldDialogueOptionBorders[i].IsValid())
		{
			WorldDialogueOptionBorders[i]->SetVisibility(bHasOption ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}
	T66_ApplyWorldDialogueSelection(WorldDialogueOptionBorders, WorldDialogueOptionTexts, SelectedIndex);
	WorldDialogueBox->SetVisibility(EVisibility::Visible);
}

void UT66GameplayHUDWidget::HideWorldDialogue()
{
	if (!WorldDialogueBox.IsValid()) return;
	WorldDialogueBox->SetVisibility(EVisibility::Collapsed);
}

void UT66GameplayHUDWidget::SetWorldDialogueSelection(int32 SelectedIndex)
{
	T66_ApplyWorldDialogueSelection(WorldDialogueOptionBorders, WorldDialogueOptionTexts, SelectedIndex);
}

void UT66GameplayHUDWidget::SetWorldDialogueScreenPosition(const FVector2D& ScreenPos)
{
	if (!WorldDialogueBox.IsValid()) return;
	WorldDialogueBox->SetRenderTransform(FSlateRenderTransform(ScreenPos));
}

bool UT66GameplayHUDWidget::IsWorldDialogueVisible() const
{
	return WorldDialogueBox.IsValid() && WorldDialogueBox->GetVisibility() == EVisibility::Visible;
}

void UT66GameplayHUDWidget::SetInteractive(bool bInteractive)
{
	SetVisibility(bInteractive ? ESlateVisibility::Visible : ESlateVisibility::SelfHitTestInvisible);
}

void UT66GameplayHUDWidget::SetInventoryInspectMode(bool bEnabled)
{
	if (bInventoryInspectMode == bEnabled)
	{
		return;
	}

	bInventoryInspectMode = bEnabled;
	ApplyInventoryInspectMode();
}

void UT66GameplayHUDWidget::ApplyInventoryInspectMode()
{
	const float Scale = bInventoryInspectMode ? BottomRightInventoryInspectScale : 1.f;

	if (InventoryPanelBox.IsValid())
	{
		InventoryPanelBox->SetWidthOverride(BottomRightInventoryPanelWidth * Scale);
		InventoryPanelBox->SetHeightOverride(BottomRightInventoryPanelHeight * Scale);
	}

	const float SlotSize = BottomRightInventorySlotSize * Scale;
	const float IconSize = FMath::Max(8.f, SlotSize - 4.f);
	for (const TSharedPtr<SBox>& SlotContainer : InventorySlotContainers)
	{
		if (SlotContainer.IsValid())
		{
			SlotContainer->SetHeightOverride(SlotSize);
		}
	}

	for (const TSharedPtr<FSlateBrush>& SlotBrush : InventorySlotBrushes)
	{
		if (SlotBrush.IsValid())
		{
			SlotBrush->ImageSize = FVector2D(IconSize, IconSize);
		}
	}
}
