// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66MiniBattleHUD.h"

#include "CanvasItem.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Gameplay/T66MiniEnemyBase.h"
#include "Gameplay/T66MiniGameMode.h"
#include "Gameplay/T66MiniGameState.h"
#include "Gameplay/T66MiniPlayerController.h"
#include "Gameplay/T66MiniPlayerPawn.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "Save/T66MiniRunSaveGame.h"

namespace
{
	UTexture* T66MiniEnsureTextureReady(UTexture* Texture)
	{
		return (Texture && Texture->GetResource()) ? Texture : nullptr;
	}

	UTexture* T66MiniHudWhiteTexture()
	{
		static TWeakObjectPtr<UTexture> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
		}

		return Cached.Get();
	}

	UTexture2D* T66MiniLoadHudTexture(const FString& AssetPath)
	{
		static TMap<FString, TWeakObjectPtr<UTexture2D>> Cache;
		if (AssetPath.IsEmpty())
		{
			return nullptr;
		}

		if (TWeakObjectPtr<UTexture2D>* Found = Cache.Find(AssetPath))
		{
			if (Found->IsValid())
			{
				return Found->Get();
			}

			Cache.Remove(AssetPath);
		}

		UTexture2D* Loaded = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *AssetPath));
		Cache.Add(AssetPath, Loaded);
		return Loaded;
	}

	UTexture2D* T66MiniHeartTexture()
	{
		return T66MiniLoadHudTexture(TEXT("/Game/UI/Sprites/UI/Hearts/T_Heart_Red.T_Heart_Red"));
	}

	FString T66MiniInventoryLabelForItem(const FName ItemID)
	{
		FString Label = ItemID.ToString();
		Label.RemoveFromStart(TEXT("Item_"));
		Label.ReplaceInline(TEXT("_"), TEXT(" "));
		if (Label.Len() > 10)
		{
			Label = Label.Left(10);
		}

		return Label.ToUpper();
	}

	void T66MiniDrawBox(UCanvas* Canvas, const float X, const float Y, const float W, const float H, const FLinearColor& Color)
	{
		UTexture* WhiteTexture = T66MiniEnsureTextureReady(T66MiniHudWhiteTexture());
		if (!Canvas || !WhiteTexture)
		{
			return;
		}

		FCanvasTileItem Tile(FVector2D(X, Y), WhiteTexture->GetResource(), FVector2D(W, H), Color);
		Tile.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(Tile);
	}

	void T66MiniDrawText(UCanvas* Canvas, const UFont* Font, const FString& Text, const float X, const float Y, const FLinearColor& Color, const bool bCenter = false)
	{
		if (!Canvas || !Font)
		{
			return;
		}

		FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), Font, Color);
		TextItem.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.8f));
		if (bCenter)
		{
			TextItem.bCentreX = true;
		}
		Canvas->DrawItem(TextItem);
	}

	void T66MiniDrawBar(UCanvas* Canvas, const float X, const float Y, const float W, const float H, const float NormalizedValue, const FLinearColor& FillColor)
	{
		T66MiniDrawBox(Canvas, X, Y, W, H, FLinearColor(0.05f, 0.06f, 0.08f, 0.88f));
		T66MiniDrawBox(Canvas, X + 2.f, Y + 2.f, (W - 4.f) * FMath::Clamp(NormalizedValue, 0.f, 1.f), H - 4.f, FillColor);
	}

	void T66MiniDrawTexture(UCanvas* Canvas, UTexture2D* Texture, const float X, const float Y, const float W, const float H, const FLinearColor& Tint = FLinearColor::White)
	{
		UTexture* ReadyTexture = T66MiniEnsureTextureReady(Texture);
		if (!Canvas || !ReadyTexture)
		{
			return;
		}

		FCanvasTileItem Tile(FVector2D(X, Y), ReadyTexture->GetResource(), FVector2D(W, H), Tint);
		Tile.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(Tile);
	}

	void T66MiniDrawHeart(UCanvas* Canvas, const float X, const float Y, const float Size, const float FillAmount)
	{
		if (!Canvas)
		{
			return;
		}

		T66MiniDrawBox(Canvas, X - 2.f, Y - 2.f, Size + 4.f, Size + 4.f, FLinearColor(0.02f, 0.02f, 0.03f, 0.92f));
		T66MiniDrawTexture(Canvas, T66MiniHeartTexture(), X, Y, Size, Size, FLinearColor(0.26f, 0.08f, 0.08f, 0.52f));
		if (FillAmount > 0.f)
		{
			const float FilledHeight = Size * FMath::Clamp(FillAmount, 0.f, 1.f);
			T66MiniDrawTexture(Canvas, T66MiniHeartTexture(), X, Y + (Size - FilledHeight), Size, FilledHeight, FLinearColor::White);
		}
	}

	void T66MiniDrawCountBadge(UCanvas* Canvas, const UFont* Font, const int32 Count, const float X, const float Y)
	{
		if (!Canvas || !Font || Count <= 1)
		{
			return;
		}

		T66MiniDrawBox(Canvas, X - 2.f, Y - 2.f, 24.f, 18.f, FLinearColor(0.98f, 0.78f, 0.24f, 0.96f));
		T66MiniDrawText(Canvas, Font, FString::Printf(TEXT("x%d"), Count), X + 10.f, Y - 1.f, FLinearColor(0.08f, 0.08f, 0.10f), true);
	}
}

void AT66MiniBattleHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniVisualSubsystem* VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	const UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	const AT66MiniGameState* MiniGameState = GetWorld() ? GetWorld()->GetGameState<AT66MiniGameState>() : nullptr;
	const AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr;
	const AT66MiniPlayerPawn* PlayerPawn = GetOwningPlayerController() ? Cast<AT66MiniPlayerPawn>(GetOwningPlayerController()->GetPawn()) : nullptr;
	const UFont* TitleFont = GEngine ? GEngine->GetMediumFont() : nullptr;
	const UFont* BodyFont = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!TitleFont || !BodyFont || !MiniGameState || !PlayerPawn)
	{
		return;
	}

	const float ScreenW = Canvas->SizeX;
	const float ScreenH = Canvas->SizeY;
	const float LeftX = 20.f;
	const float TopY = 18.f;
	const float CardW = 330.f;
	const float CardH = 156.f;
	const float TimerCardW = 240.f;
	const float TimerCardH = 90.f;
	const float TimerX = (ScreenW * 0.5f) - (TimerCardW * 0.5f);
	const float RightCardW = 300.f;
	const float RightCardH = 146.f;
	const float RightX = ScreenW - RightCardW - 22.f;
	const float RightY = 18.f;

	T66MiniDrawBox(Canvas, LeftX, TopY, CardW, CardH, FLinearColor(0.04f, 0.05f, 0.07f, 0.86f));
	if (DataSubsystem)
	{
		const FString HeroName = PlayerPawn->GetHeroDisplayName().IsEmpty() ? MiniGameState->HeroID.ToString() : PlayerPawn->GetHeroDisplayName();
		if (const FT66MiniHeroDefinition* HeroDefinition = DataSubsystem->FindHero(PlayerPawn->GetHeroID()))
		{
			T66MiniDrawTexture(Canvas, T66MiniLoadHudTexture(HeroDefinition->PortraitPath), LeftX + 16.f, TopY + 16.f, 72.f, 72.f);
		}
		T66MiniDrawText(Canvas, TitleFont, HeroName.ToUpper(), LeftX + 100.f, TopY + 14.f, FLinearColor::White);
	}

	T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("Difficulty: %s"), *MiniGameState->DifficultyID.ToString()), LeftX + 100.f, TopY + 50.f, FLinearColor(0.84f, 0.88f, 0.94f));
	T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("Wave %d / 5"), MiniGameState->WaveIndex), LeftX + 100.f, TopY + 72.f, FLinearColor(0.96f, 0.86f, 0.42f));
	if (DataSubsystem)
	{
		const FT66MiniCompanionDefinition* CompanionDefinition = DataSubsystem->FindCompanion(MiniGameState->CompanionID);
		const float CompanionHealing = (SaveSubsystem && CompanionDefinition)
			? SaveSubsystem->GetCompanionHealingPerSecond(CompanionDefinition->CompanionID, DataSubsystem)
			: 0.0f;
		const int32 CompanionUnity = (SaveSubsystem && CompanionDefinition)
			? SaveSubsystem->GetCompanionUnityStagesCleared(CompanionDefinition->CompanionID, DataSubsystem)
			: 0;
		T66MiniDrawText(
			Canvas,
			BodyFont,
			FString::Printf(
				TEXT("Companion: %s  HPS %.1f  Unity %d"),
				CompanionDefinition ? *CompanionDefinition->DisplayName : *MiniGameState->CompanionID.ToString(),
				CompanionHealing,
				CompanionUnity),
			LeftX + 16.f,
			TopY + 110.f,
			FLinearColor(0.78f, 0.92f, 0.80f, 1.0f));
	}

	T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("HP %.0f / %.0f"), PlayerPawn->GetCurrentHealth(), PlayerPawn->GetMaxHealth()), LeftX + 16.f, TopY + 88.f, FLinearColor::White);
	const int32 MaxHearts = PlayerPawn->GetMaxHearts();
	const float HeartSize = 22.f;
	const float HeartSpacing = 6.f;
	float HeartX = LeftX + 16.f;
	const float HeartY = TopY + 128.f;
	for (int32 HeartIndex = 0; HeartIndex < MaxHearts && HeartIndex < 10; ++HeartIndex)
	{
		T66MiniDrawHeart(Canvas, HeartX, HeartY, HeartSize, PlayerPawn->GetHeartFill(HeartIndex));
		HeartX += HeartSize + HeartSpacing;
	}

	T66MiniDrawBox(Canvas, TimerX, TopY, TimerCardW, TimerCardH, FLinearColor(0.04f, 0.05f, 0.07f, 0.86f));
	T66MiniDrawText(Canvas, BodyFont, TEXT("WAVE TIMER"), ScreenW * 0.5f, TopY + 14.f, FLinearColor(0.82f, 0.86f, 0.92f), true);
	T66MiniDrawText(Canvas, TitleFont, FString::Printf(TEXT("%.0f"), FMath::CeilToFloat(MiniGameState->WaveSecondsRemaining)), ScreenW * 0.5f, TopY + 34.f, FLinearColor::White, true);
	T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("Materials %d   Gold %d   LV %d"), PlayerPawn->GetMaterials(), PlayerPawn->GetGold(), PlayerPawn->GetHeroLevel()), ScreenW * 0.5f, TopY + 66.f, FLinearColor(0.96f, 0.86f, 0.42f), true);

	T66MiniDrawBox(Canvas, RightX, RightY, RightCardW, RightCardH, FLinearColor(0.04f, 0.05f, 0.07f, 0.86f));
	T66MiniDrawText(Canvas, TitleFont, TEXT("COMBAT"), RightX + 16.f, RightY + 12.f, FLinearColor::White);
	T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("DMG %.1f   ATK %.1f"), PlayerPawn->GetDamageStat(), PlayerPawn->GetAttackSpeedStat()), RightX + 16.f, RightY + 50.f, FLinearColor(0.90f, 0.92f, 0.96f));
	T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("ARM %.1f   CRIT %.0f%%"), PlayerPawn->GetArmorStat(), PlayerPawn->GetCritChance() * 100.f), RightX + 16.f, RightY + 72.f, FLinearColor(0.90f, 0.92f, 0.96f));
	T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("REGEN %.1f   EVADE %.0f%%"), PlayerPawn->GetPassiveRegenPerSecond(), PlayerPawn->GetEvasionChance() * 100.f), RightX + 16.f, RightY + 94.f, FLinearColor(0.90f, 0.92f, 0.96f));
	T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("RANGE %.0f   LS %.0f%%"), PlayerPawn->GetAttackRange(), PlayerPawn->GetLifeStealChance() * 100.f), RightX + 16.f, RightY + 116.f, FLinearColor(0.90f, 0.92f, 0.96f));

	const float XpPct = PlayerPawn->GetExperienceToNextLevel() > 0.f ? PlayerPawn->GetExperience() / PlayerPawn->GetExperienceToNextLevel() : 0.f;
	const float XpBarW = 360.f;
	const float XpBarX = (ScreenW * 0.5f) - (XpBarW * 0.5f);
	const float XpBarY = ScreenH - 88.f;
	T66MiniDrawBox(Canvas, XpBarX - 8.f, XpBarY - 8.f, XpBarW + 16.f, 54.f, FLinearColor(0.04f, 0.05f, 0.07f, 0.86f));
	T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("LEVEL %d"), PlayerPawn->GetHeroLevel()), XpBarX, XpBarY - 2.f, FLinearColor::White);
	T66MiniDrawBar(Canvas, XpBarX, XpBarY + 18.f, XpBarW, 14.f, XpPct, FLinearColor(0.22f, 0.62f, 0.96f, 1.f));

	if (ActiveRun && DataSubsystem)
	{
		float IdolX = (ScreenW * 0.5f) - 150.f;
		const float IdolY = ScreenH - 150.f;
		for (int32 Index = 0; Index < 4; ++Index)
		{
			T66MiniDrawBox(Canvas, IdolX, IdolY, 64.f, 64.f, FLinearColor(0.06f, 0.07f, 0.10f, 0.90f));
			if (ActiveRun->EquippedIdolIDs.IsValidIndex(Index))
			{
				if (const FT66MiniIdolDefinition* IdolDefinition = DataSubsystem->FindIdol(ActiveRun->EquippedIdolIDs[Index]))
				{
					T66MiniDrawTexture(Canvas, T66MiniLoadHudTexture(IdolDefinition->IconPath), IdolX + 8.f, IdolY + 8.f, 48.f, 48.f);
				}
			}
			IdolX += 76.f;
		}
	}

	if (ActiveRun && DataSubsystem)
	{
		TArray<TPair<FName, int32>> InventoryStacks;
		TMap<FName, int32> InventoryStackLookup;
		const int32 MaxInventorySlots = 8;
		for (int32 ItemIndex = ActiveRun->OwnedItemIDs.Num() - 1; ItemIndex >= 0; --ItemIndex)
		{
			const FName ItemID = ActiveRun->OwnedItemIDs[ItemIndex];
			if (ItemID.IsNone())
			{
				continue;
			}

			if (const int32* ExistingIndex = InventoryStackLookup.Find(ItemID))
			{
				++InventoryStacks[*ExistingIndex].Value;
				continue;
			}

			InventoryStackLookup.Add(ItemID, InventoryStacks.Num());
			InventoryStacks.Emplace(ItemID, 1);
		}

		const float InventoryPanelW = 372.f;
		const float InventoryPanelH = 224.f;
		const float InventoryPanelX = ScreenW - InventoryPanelW - 18.f;
		const float InventoryPanelY = ScreenH - InventoryPanelH - 92.f;
		const float InventorySlotSize = 64.f;
		const float InventorySlotPadding = 10.f;
		const int32 InventoryColumns = 4;
		const float InventoryGridX = InventoryPanelX + 18.f;
		const float InventoryGridY = InventoryPanelY + 104.f;

		T66MiniDrawBox(Canvas, InventoryPanelX, InventoryPanelY, InventoryPanelW, InventoryPanelH, FLinearColor(0.03f, 0.04f, 0.06f, 0.94f));
		T66MiniDrawBox(Canvas, InventoryPanelX + 10.f, InventoryPanelY + 10.f, InventoryPanelW - 20.f, 34.f, FLinearColor(0.10f, 0.12f, 0.18f, 0.98f));
		T66MiniDrawText(Canvas, TitleFont, TEXT("INVENTORY"), InventoryPanelX + 20.f, InventoryPanelY + 12.f, FLinearColor::White);
		T66MiniDrawText(Canvas, BodyFont, TEXT("LATEST PICKUPS"), InventoryPanelX + 20.f, InventoryPanelY + 48.f, FLinearColor(0.78f, 0.82f, 0.88f));

		const float PillY = InventoryPanelY + 72.f;
		T66MiniDrawBox(Canvas, InventoryPanelX + 18.f, PillY, 98.f, 22.f, FLinearColor(0.18f, 0.14f, 0.05f, 0.92f));
		T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("Gold %d"), PlayerPawn->GetGold()), InventoryPanelX + 26.f, PillY + 2.f, FLinearColor(0.98f, 0.84f, 0.30f));
		T66MiniDrawBox(Canvas, InventoryPanelX + 124.f, PillY, 98.f, 22.f, FLinearColor(0.08f, 0.14f, 0.18f, 0.92f));
		T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("Mats %d"), PlayerPawn->GetMaterials()), InventoryPanelX + 132.f, PillY + 2.f, FLinearColor(0.72f, 0.88f, 0.96f));
		T66MiniDrawBox(Canvas, InventoryPanelX + 230.f, PillY, 124.f, 22.f, FLinearColor(0.16f, 0.10f, 0.18f, 0.92f));
		T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("Items %d"), ActiveRun->OwnedItemIDs.Num()), InventoryPanelX + 238.f, PillY + 2.f, FLinearColor(0.96f, 0.82f, 0.98f));

		for (int32 SlotIndex = 0; SlotIndex < MaxInventorySlots; ++SlotIndex)
		{
			const int32 Column = SlotIndex % InventoryColumns;
			const int32 Row = SlotIndex / InventoryColumns;
			const float SlotX = InventoryGridX + (Column * (InventorySlotSize + InventorySlotPadding));
			const float SlotY = InventoryGridY + (Row * (InventorySlotSize + InventorySlotPadding));

			const bool bFilledSlot = InventoryStacks.IsValidIndex(SlotIndex);
			T66MiniDrawBox(Canvas, SlotX, SlotY, InventorySlotSize, InventorySlotSize, bFilledSlot ? FLinearColor(0.32f, 0.26f, 0.10f, 0.94f) : FLinearColor(0.06f, 0.07f, 0.10f, 0.92f));
			T66MiniDrawBox(Canvas, SlotX + 2.f, SlotY + 2.f, InventorySlotSize - 4.f, InventorySlotSize - 4.f, bFilledSlot ? FLinearColor(0.10f, 0.10f, 0.14f, 1.0f) : FLinearColor(0.03f, 0.03f, 0.04f, 1.0f));
			if (!bFilledSlot)
			{
				continue;
			}

			const TPair<FName, int32>& Entry = InventoryStacks[SlotIndex];
			const FName ItemID = Entry.Key;
			if (const FT66MiniItemDefinition* ItemDefinition = DataSubsystem->FindItem(ItemID))
			{
				UTexture2D* ItemTexture = VisualSubsystem
					? VisualSubsystem->LoadItemTexture(ItemDefinition->ItemID, ItemDefinition->IconPath)
					: T66MiniLoadHudTexture(ItemDefinition->IconPath);
				if (ItemTexture)
				{
					T66MiniDrawTexture(Canvas, ItemTexture, SlotX + 8.f, SlotY + 8.f, InventorySlotSize - 16.f, InventorySlotSize - 16.f);
				}
				else
				{
					T66MiniDrawText(Canvas, BodyFont, T66MiniInventoryLabelForItem(ItemID), SlotX + (InventorySlotSize * 0.5f), SlotY + 24.f, FLinearColor::White, true);
				}
			}

			T66MiniDrawCountBadge(Canvas, BodyFont, Entry.Value, SlotX + InventorySlotSize - 26.f, SlotY + 6.f);
		}

		if (ActiveRun->OwnedItemIDs.Num() == 0)
		{
			T66MiniDrawText(Canvas, BodyFont, TEXT("Pick up item bags or loot crates to fill this inventory."), InventoryPanelX + 20.f, InventoryPanelY + 188.f, FLinearColor(0.78f, 0.82f, 0.88f));
		}
		else if (InventoryStacks.Num() > MaxInventorySlots)
		{
			T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("+%d more stacks"), InventoryStacks.Num() - MaxInventorySlots), InventoryPanelX + 20.f, InventoryPanelY + 188.f, FLinearColor(0.78f, 0.82f, 0.88f));
		}
	}

	AT66MiniEnemyBase* BossEnemy = nullptr;
	for (TActorIterator<AT66MiniEnemyBase> It(GetWorld()); It; ++It)
	{
		if (It->IsBossEnemy() && !It->IsEnemyDead())
		{
			BossEnemy = *It;
			break;
		}
	}

	if (BossEnemy)
	{
		const float BossPct = BossEnemy->GetMaxHealth() > 0.f ? BossEnemy->GetCurrentHealth() / BossEnemy->GetMaxHealth() : 0.f;
		const float BossBarW = 440.f;
		const float BossBarX = (ScreenW * 0.5f) - (BossBarW * 0.5f);
		const float BossBarY = 116.f;
		T66MiniDrawBox(Canvas, BossBarX - 10.f, BossBarY - 14.f, BossBarW + 20.f, 44.f, FLinearColor(0.10f, 0.03f, 0.03f, 0.86f));
		T66MiniDrawText(Canvas, BodyFont, BossEnemy->GetEnemyID().ToString().ToUpper(), ScreenW * 0.5f, BossBarY - 10.f, FLinearColor(0.98f, 0.78f, 0.62f), true);
		T66MiniDrawBar(Canvas, BossBarX, BossBarY + 10.f, BossBarW, 14.f, BossPct, FLinearColor(0.92f, 0.22f, 0.18f, 1.f));
	}
	else if (MiniGameMode && MiniGameMode->HasBossTelegraphActive())
	{
		T66MiniDrawBox(Canvas, (ScreenW * 0.5f) - 170.f, 118.f, 340.f, 38.f, FLinearColor(0.30f, 0.06f, 0.04f, 0.82f));
		T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("BOSS INBOUND  %.1fs"), MiniGameMode->GetBossTelegraphRemaining()), ScreenW * 0.5f, 126.f, FLinearColor(1.0f, 0.84f, 0.62f), true);
	}

	if (RunState)
	{
		T66MiniDrawText(Canvas, BodyFont, TEXT("ESC PAUSES  |  MINI AUTO-SAVES DURING BATTLE"), 24.f, ScreenH - 34.f, FLinearColor(0.78f, 0.82f, 0.88f));
	}

	if (const AT66MiniPlayerController* MiniController = Cast<AT66MiniPlayerController>(GetOwningPlayerController()))
	{
		if (MiniController->IsLootCratePresentationActive())
		{
			const double DurationSeconds = FMath::Max(0.01, MiniController->GetLootCratePresentationDurationSeconds());
			const double ElapsedSeconds = MiniController->GetLootCratePresentationElapsedSeconds();
			const float Alpha = FMath::Clamp(static_cast<float>(ElapsedSeconds / DurationSeconds), 0.f, 1.f);
			const float Ease = FMath::InterpEaseOut(0.f, 1.f, Alpha, 5.2f);
			const TArray<FName>& StripItemIDs = MiniController->GetLootCrateStripItemIDs();
			const float OverlayPanelW = 560.f;
			const float OverlayPanelH = 220.f;
			const float OverlayX = (ScreenW * 0.5f) - (OverlayPanelW * 0.5f);
			const float OverlayY = (ScreenH * 0.5f) - (OverlayPanelH * 0.5f);
			const float ViewportW = 460.f;
			const float ViewportH = 92.f;
			const float TileSize = 88.f;
			const float TileStride = 92.f;
			const float WinnerCenterX = (static_cast<float>(MiniController->GetLootCrateWinnerIndex()) * TileStride) + (TileSize * 0.5f);
			const float TotalScrollDistance = FMath::Max(0.f, WinnerCenterX - (ViewportW * 0.5f));
			const float CurrentScrollDistance = TotalScrollDistance * Ease;
			const float StripOriginX = OverlayX + 50.f - CurrentScrollDistance;
			const float StripOriginY = OverlayY + 72.f;
			const float ViewportX = OverlayX + 50.f;
			const float ViewportY = OverlayY + 72.f;

			T66MiniDrawBox(Canvas, 0.f, 0.f, ScreenW, ScreenH, FLinearColor(0.f, 0.f, 0.f, 0.66f));
			T66MiniDrawBox(Canvas, OverlayX, OverlayY, OverlayPanelW, OverlayPanelH, FLinearColor(0.03f, 0.04f, 0.06f, 0.98f));
			T66MiniDrawText(Canvas, TitleFont, TEXT("LOOT CRATE"), ScreenW * 0.5f, OverlayY + 18.f, FLinearColor::White, true);
			T66MiniDrawText(Canvas, BodyFont, TEXT("Mini battle paused while the crate opens"), ScreenW * 0.5f, OverlayY + 44.f, FLinearColor(0.78f, 0.82f, 0.88f), true);
			T66MiniDrawBox(Canvas, ViewportX - 4.f, ViewportY - 4.f, ViewportW + 8.f, ViewportH + 8.f, FLinearColor(0.01f, 0.01f, 0.02f, 0.96f));

			for (int32 Index = 0; Index < StripItemIDs.Num(); ++Index)
			{
				const float TileX = StripOriginX + (Index * TileStride);
				if (TileX + TileSize < ViewportX || TileX > ViewportX + ViewportW)
				{
					continue;
				}

				T66MiniDrawBox(Canvas, TileX, StripOriginY, TileSize, TileSize, FLinearColor(0.08f, 0.10f, 0.14f, 0.96f));
				T66MiniDrawBox(Canvas, TileX + 2.f, StripOriginY + 2.f, TileSize - 4.f, TileSize - 4.f, FLinearColor(0.03f, 0.03f, 0.04f, 1.f));
				if (DataSubsystem)
				{
					if (const FT66MiniItemDefinition* ItemDefinition = DataSubsystem->FindItem(StripItemIDs[Index]))
					{
						UTexture2D* ItemTexture = VisualSubsystem
							? VisualSubsystem->LoadItemTexture(ItemDefinition->ItemID, ItemDefinition->IconPath)
							: T66MiniLoadHudTexture(ItemDefinition->IconPath);
						if (ItemTexture)
						{
							T66MiniDrawTexture(Canvas, ItemTexture, TileX + 10.f, StripOriginY + 10.f, TileSize - 20.f, TileSize - 20.f);
						}
						else
						{
							T66MiniDrawText(Canvas, BodyFont, T66MiniInventoryLabelForItem(ItemDefinition->ItemID), TileX + (TileSize * 0.5f), StripOriginY + 32.f, FLinearColor::White, true);
						}
					}
				}
			}

			const float HighlightX = ViewportX + (ViewportW * 0.5f) - ((TileSize + 4.f) * 0.5f);
			T66MiniDrawBox(Canvas, HighlightX, ViewportY - 2.f, TileSize + 4.f, ViewportH + 4.f, FLinearColor(1.0f, 0.82f, 0.24f, 0.16f));
			T66MiniDrawBox(Canvas, ViewportX + (ViewportW * 0.5f) - 2.f, ViewportY - 6.f, 4.f, ViewportH + 12.f, FLinearColor(1.0f, 0.82f, 0.24f, 0.92f));
			T66MiniDrawText(Canvas, BodyFont, FString::Printf(TEXT("Opening %.1fs"), FMath::Max(0.f, static_cast<float>(DurationSeconds - ElapsedSeconds))), ScreenW * 0.5f, OverlayY + 176.f, FLinearColor(0.96f, 0.86f, 0.42f), true);
			T66MiniDrawText(Canvas, BodyFont, MiniController->GetLootCrateRewardItemID().ToString(), ScreenW * 0.5f, OverlayY + 196.f, FLinearColor::White, true);
		}
	}
}
