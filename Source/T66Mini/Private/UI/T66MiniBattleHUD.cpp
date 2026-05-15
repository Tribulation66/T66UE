// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66MiniBattleHUD.h"

#include "Core/T66PlayerSettingsSubsystem.h"
#include "CanvasItem.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
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
#include "GameFramework/InputSettings.h"
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

	void T66MiniDrawHeart(UCanvas* Canvas, UTexture2D* HeartTexture, const float X, const float Y, const float Size, const float FillAmount)
	{
		if (!Canvas)
		{
			return;
		}

		T66MiniDrawBox(Canvas, X - 2.f, Y - 2.f, Size + 4.f, Size + 4.f, FLinearColor(0.02f, 0.02f, 0.03f, 0.92f));
		T66MiniDrawTexture(Canvas, HeartTexture, X, Y, Size, Size, FLinearColor(0.26f, 0.08f, 0.08f, 0.52f));
		if (FillAmount > 0.f)
		{
			const float FilledHeight = Size * FMath::Clamp(FillAmount, 0.f, 1.f);
			T66MiniDrawTexture(Canvas, HeartTexture, X, Y + (Size - FilledHeight), Size, FilledHeight, FLinearColor::White);
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

	void T66MiniDrawScaledText(
		UCanvas* Canvas,
		const UFont* Font,
		const FString& Text,
		const float X,
		const float Y,
		const FLinearColor& Color,
		const float Scale = 1.f,
		const bool bCenter = false)
	{
		if (!Canvas || !Font)
		{
			return;
		}

		FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), Font, Color);
		TextItem.Scale = FVector2D(Scale, Scale);
		TextItem.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.86f));
		if (bCenter)
		{
			TextItem.bCentreX = true;
		}
		Canvas->DrawItem(TextItem);
	}

	void T66MiniDrawPanel(
		UCanvas* Canvas,
		const float X,
		const float Y,
		const float W,
		const float H,
		const FLinearColor& AccentColor,
		const FLinearColor& FillColor = FLinearColor(0.03f, 0.04f, 0.06f, 0.90f))
	{
		T66MiniDrawBox(Canvas, X + 4.f, Y + 6.f, W, H, FLinearColor(0.f, 0.f, 0.f, 0.18f));
		T66MiniDrawBox(Canvas, X, Y, W, H, FLinearColor(0.00f, 0.00f, 0.00f, 0.34f));
		T66MiniDrawBox(Canvas, X + 1.f, Y + 1.f, W - 2.f, H - 2.f, FillColor);
		T66MiniDrawBox(Canvas, X + 1.f, Y + 1.f, W - 2.f, 4.f, AccentColor);
		T66MiniDrawBox(Canvas, X + 1.f, Y + 18.f, W - 2.f, 1.f, FLinearColor(1.f, 1.f, 1.f, 0.05f));
	}

	void T66MiniDrawInsetPanel(
		UCanvas* Canvas,
		const float X,
		const float Y,
		const float W,
		const float H,
		const FLinearColor& Tint)
	{
		T66MiniDrawBox(Canvas, X, Y, W, H, FLinearColor(0.02f, 0.03f, 0.04f, 0.84f));
		T66MiniDrawBox(Canvas, X + 1.f, Y + 1.f, W - 2.f, H - 2.f, Tint);
	}

	FString T66MiniFormatSeconds(const float Seconds)
	{
		const int32 TotalSeconds = FMath::Max(0, FMath::CeilToInt(Seconds));
		return FString::Printf(TEXT("%d:%02d"), TotalSeconds / 60, TotalSeconds % 60);
	}

	FString T66MiniReadableId(const FName Id, const FString& Prefix = FString())
	{
		FString Label = Id.ToString();
		if (!Prefix.IsEmpty())
		{
			Label.RemoveFromStart(Prefix);
		}
		Label.ReplaceInline(TEXT("_"), TEXT(" "));
		return Label;
	}

	FKey T66MiniFindActionKey(const FName ActionName, const FKey& DefaultKey)
	{
		if (const UInputSettings* Settings = UInputSettings::GetInputSettings())
		{
			for (const FInputActionKeyMapping& Mapping : Settings->GetActionMappings())
			{
				if (Mapping.ActionName == ActionName && Mapping.Key.IsValid() && !Mapping.Key.IsGamepadKey() && !Mapping.Key.IsTouch())
				{
					return Mapping.Key;
				}
			}
		}

		return DefaultKey;
	}

	FString T66MiniCompactKeyLabel(const FKey& Key)
	{
		if (Key == EKeys::LeftMouseButton)
		{
			return TEXT("LMB");
		}

		if (Key == EKeys::RightMouseButton)
		{
			return TEXT("RMB");
		}

		if (Key == EKeys::Escape)
		{
			return TEXT("ESC");
		}

		FString Label = Key.IsValid() ? Key.GetDisplayName().ToString().ToUpper() : FString(TEXT("-"));
		Label.ReplaceInline(TEXT("LEFT MOUSE BUTTON"), TEXT("LMB"));
		Label.ReplaceInline(TEXT("RIGHT MOUSE BUTTON"), TEXT("RMB"));
		Label.ReplaceInline(TEXT("SPACE BAR"), TEXT("SPACE"));
		return Label;
	}
}

void AT66MiniBattleHUD::BeginPlay()
{
	Super::BeginPlay();
	RefreshCachedUiState();
}

void AT66MiniBattleHUD::RefreshCachedUiState()
{
	CachedPauseKey = T66MiniFindActionKey(FName(TEXT("MiniPause")), EKeys::Escape);
	CachedInteractKey = T66MiniFindActionKey(FName(TEXT("MiniInteract")), EKeys::LeftMouseButton);
	CachedUltimateKey = T66MiniFindActionKey(FName(TEXT("MiniUltimate")), EKeys::RightMouseButton);
	CachedPauseKeyLabel = T66MiniCompactKeyLabel(CachedPauseKey);
	CachedInteractKeyLabel = T66MiniCompactKeyLabel(CachedInteractKey);
	CachedUltimateKeyLabel = T66MiniCompactKeyLabel(CachedUltimateKey);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66MiniVisualSubsystem* VisualSubsystem = GameInstance->GetSubsystem<UT66MiniVisualSubsystem>())
		{
			HeartTexture = VisualSubsystem->LoadTextureByAssetPath(TEXT("/Game/UI/Sprites/UI/Hearts/T_Heart_Red.T_Heart_Red"));
			UltimateIconTexture = VisualSubsystem->LoadHudTexture(TEXT("Ult_Generic"));
			PassiveIconTexture = VisualSubsystem->LoadHudTexture(TEXT("Passive_Generic"));
			QuickReviveIconTexture = VisualSubsystem->LoadHudTexture(TEXT("QuickRevive"));
			MouseLeftIconTexture = VisualSubsystem->LoadHudTexture(TEXT("MouseLeft"));
			MouseRightIconTexture = VisualSubsystem->LoadHudTexture(TEXT("MouseRight"));
			InteractHandIconTexture = VisualSubsystem->LoadHudTexture(TEXT("Interact_Hand"));
		}
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
	const UT66PlayerSettingsSubsystem* PlayerSettings = GameInstance ? GameInstance->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
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
	const float SafeMargin = 18.f;
	const float TopY = 18.f;
	if (!HeartTexture || !UltimateIconTexture || !PassiveIconTexture || !QuickReviveIconTexture || !MouseLeftIconTexture || !MouseRightIconTexture || !InteractHandIconTexture)
	{
		RefreshCachedUiState();
	}
	const FT66MiniDifficultyDefinition* DifficultyDefinition = DataSubsystem ? DataSubsystem->FindDifficulty(MiniGameState->DifficultyID) : nullptr;
	const FT66MiniHeroDefinition* HeroDefinition = DataSubsystem ? DataSubsystem->FindHero(PlayerPawn->GetHeroID()) : nullptr;
	const FT66MiniCompanionDefinition* CompanionDefinition = DataSubsystem ? DataSubsystem->FindCompanion(PlayerPawn->GetSelectedCompanionID()) : nullptr;
	const FT66MiniWaveDefinition* WaveDefinition = DataSubsystem ? DataSubsystem->FindWave(MiniGameState->DifficultyID, MiniGameState->WaveIndex) : nullptr;
	const int32 MaxStageIndex = DataSubsystem ? FMath::Max(1, DataSubsystem->GetMaxStageIndexForDifficulty(MiniGameState->DifficultyID)) : 10;
	const FString HeroName = PlayerPawn->GetHeroDisplayName().IsEmpty()
		? T66MiniReadableId(MiniGameState->HeroID)
		: PlayerPawn->GetHeroDisplayName();
	const FString DifficultyName = DifficultyDefinition && !DifficultyDefinition->DisplayName.IsEmpty()
		? DifficultyDefinition->DisplayName
		: T66MiniReadableId(MiniGameState->DifficultyID);
	const FString CompanionName = CompanionDefinition && !CompanionDefinition->DisplayName.IsEmpty()
		? CompanionDefinition->DisplayName
		: T66MiniReadableId(MiniGameState->CompanionID);
	const float CompanionHealing = (SaveSubsystem && CompanionDefinition)
		? SaveSubsystem->GetCompanionHealingPerSecond(CompanionDefinition->CompanionID, DataSubsystem)
		: 0.0f;
	const int32 CompanionUnity = (SaveSubsystem && CompanionDefinition)
		? SaveSubsystem->GetCompanionUnityStagesCleared(CompanionDefinition->CompanionID, DataSubsystem)
		: 0;
	const float WaveDurationSeconds = WaveDefinition ? WaveDefinition->DurationSeconds : FMath::Max(60.f, MiniGameState->WaveSecondsRemaining);
	const float WaveRemainingPct = WaveDurationSeconds > 0.f
		? FMath::Clamp(MiniGameState->WaveSecondsRemaining / WaveDurationSeconds, 0.f, 1.f)
		: 0.f;
	const float XpPct = PlayerPawn->GetExperienceToNextLevel() > 0.f
		? FMath::Clamp(PlayerPawn->GetExperience() / PlayerPawn->GetExperienceToNextLevel(), 0.f, 1.f)
		: 0.f;
	const float UltPct = PlayerPawn->GetUltimateCooldownDuration() > 0.f
		? 1.f - (PlayerPawn->GetUltimateCooldownRemaining() / PlayerPawn->GetUltimateCooldownDuration())
		: 1.f;
	const FLinearColor DifficultyAccent = DifficultyDefinition ? DifficultyDefinition->AccentColor : FLinearColor(0.36f, 0.50f, 0.76f, 1.f);
	const FLinearColor GoldAccent(0.98f, 0.84f, 0.30f, 1.f);
	const FLinearColor TealAccent(0.46f, 0.86f, 0.94f, 1.f);
	const FLinearColor GreenAccent(0.62f, 0.98f, 0.76f, 1.f);
	const FLinearColor DangerAccent(1.00f, 0.42f, 0.34f, 1.f);
	const FLinearColor MutedText(0.72f, 0.78f, 0.84f, 1.f);
	const FLinearColor PrimaryText(0.96f, 0.97f, 1.00f, 1.f);
	const FLinearColor PanelFill(0.03f, 0.04f, 0.06f, 0.82f);
	const FLinearColor LowTimeColor = MiniGameState->WaveSecondsRemaining <= 12.f ? DangerAccent : GoldAccent;

	const auto DrawChip = [&](const float X, const float Y, const float W, const FString& Label, const FString& Value, const FLinearColor& Tint)
	{
		T66MiniDrawInsetPanel(Canvas, X, Y, W, 30.f, FLinearColor(Tint.R * 0.16f, Tint.G * 0.16f, Tint.B * 0.16f, 0.88f));
		T66MiniDrawScaledText(Canvas, BodyFont, Label.ToUpper(), X + 8.f, Y + 4.f, MutedText, 0.78f);
		T66MiniDrawScaledText(Canvas, BodyFont, Value, X + 8.f, Y + 15.f, Tint, 0.90f);
	};

	const auto DrawStatRow = [&](const float X, const float Y, const FString& LeftLabel, const FString& LeftValue, const FString& RightLabel, const FString& RightValue)
	{
		T66MiniDrawScaledText(Canvas, BodyFont, LeftLabel, X, Y, MutedText, 0.78f);
		T66MiniDrawScaledText(Canvas, BodyFont, LeftValue, X + 4.f, Y + 12.f, PrimaryText, 0.94f);
		T66MiniDrawScaledText(Canvas, BodyFont, RightLabel, X + 128.f, Y, MutedText, 0.78f);
		T66MiniDrawScaledText(Canvas, BodyFont, RightValue, X + 132.f, Y + 12.f, PrimaryText, 0.94f);
	};

	const float HeroPanelX = SafeMargin;
	const float HeroPanelY = TopY;
	const float HeroPanelW = 430.f;
	const float HeroPanelH = 196.f;
	T66MiniDrawPanel(Canvas, HeroPanelX, HeroPanelY, HeroPanelW, HeroPanelH, DifficultyAccent, PanelFill);
	T66MiniDrawInsetPanel(Canvas, HeroPanelX + 16.f, HeroPanelY + 18.f, 96.f, 96.f, FLinearColor(0.10f, 0.11f, 0.15f, 0.92f));
	if (HeroDefinition)
	{
		T66MiniDrawTexture(Canvas, VisualSubsystem ? VisualSubsystem->LoadTextureByAssetPath(HeroDefinition->PortraitPath) : nullptr, HeroPanelX + 20.f, HeroPanelY + 22.f, 88.f, 88.f);
	}
	else
	{
		T66MiniDrawScaledText(Canvas, TitleFont, HeroName.Left(1).ToUpper(), HeroPanelX + 64.f, HeroPanelY + 34.f, PrimaryText, 1.20f, true);
	}
	T66MiniDrawScaledText(Canvas, TitleFont, HeroName.ToUpper(), HeroPanelX + 128.f, HeroPanelY + 18.f, PrimaryText, 1.06f);
	T66MiniDrawScaledText(Canvas, BodyFont, DifficultyName.ToUpper(), HeroPanelX + 128.f, HeroPanelY + 54.f, DifficultyAccent, 1.02f);
	T66MiniDrawInsetPanel(Canvas, HeroPanelX + 128.f, HeroPanelY + 80.f, 128.f, 30.f, FLinearColor(DifficultyAccent.R * 0.18f, DifficultyAccent.G * 0.18f, DifficultyAccent.B * 0.18f, 0.90f));
	T66MiniDrawScaledText(Canvas, BodyFont, FString::Printf(TEXT("STAGE %d / %d"), MiniGameState->WaveIndex, MaxStageIndex), HeroPanelX + 140.f, HeroPanelY + 87.f, GoldAccent, 1.0f);
	T66MiniDrawScaledText(Canvas, BodyFont, FString::Printf(TEXT("HP %.0f / %.0f"), PlayerPawn->GetCurrentHealth(), PlayerPawn->GetMaxHealth()), HeroPanelX + 16.f, HeroPanelY + 124.f, PrimaryText, 1.03f);
	T66MiniDrawScaledText(
		Canvas,
		BodyFont,
		FString::Printf(TEXT("%s   HPS %.1f   UNITY %d"), *CompanionName.ToUpper(), CompanionHealing, CompanionUnity),
		HeroPanelX + 16.f,
		HeroPanelY + 146.f,
		MutedText,
		0.94f);
	if (PlayerPawn->HasQuickReviveReady())
	{
		T66MiniDrawInsetPanel(Canvas, HeroPanelX + HeroPanelW - 138.f, HeroPanelY + 142.f, 120.f, 28.f, FLinearColor(0.10f, 0.18f, 0.12f, 0.92f));
		if (QuickReviveIconTexture)
		{
			T66MiniDrawTexture(Canvas, QuickReviveIconTexture, HeroPanelX + HeroPanelW - 128.f, HeroPanelY + 147.f, 18.f, 18.f);
		}
		T66MiniDrawScaledText(Canvas, BodyFont, TEXT("REVIVE READY"), HeroPanelX + HeroPanelW - 104.f, HeroPanelY + 149.f, GreenAccent, 0.88f);
	}
	const int32 MaxHearts = PlayerPawn->GetMaxHearts();
	const float HeartSize = 26.f;
	const float HeartSpacing = 7.f;
	float HeartX = HeroPanelX + 16.f;
	const float HeartY = HeroPanelY + 164.f;
	for (int32 HeartIndex = 0; HeartIndex < MaxHearts && HeartIndex < 10; ++HeartIndex)
	{
		T66MiniDrawHeart(Canvas, HeartTexture, HeartX, HeartY, HeartSize, PlayerPawn->GetHeartFill(HeartIndex));
		HeartX += HeartSize + HeartSpacing;
	}

	const float TimerPanelW = 330.f;
	const float TimerPanelH = 136.f;
	const float TimerPanelX = (ScreenW * 0.5f) - (TimerPanelW * 0.5f);
	T66MiniDrawPanel(Canvas, TimerPanelX, TopY, TimerPanelW, TimerPanelH, LowTimeColor, PanelFill);
	T66MiniDrawScaledText(Canvas, BodyFont, TEXT("STAGE TIMER"), ScreenW * 0.5f, TopY + 14.f, MutedText, 1.0f, true);
	T66MiniDrawScaledText(Canvas, TitleFont, T66MiniFormatSeconds(MiniGameState->WaveSecondsRemaining), ScreenW * 0.5f, TopY + 38.f, PrimaryText, 1.36f, true);
	T66MiniDrawBar(Canvas, TimerPanelX + 20.f, TopY + 74.f, TimerPanelW - 40.f, 16.f, WaveRemainingPct, LowTimeColor);
	DrawChip(TimerPanelX + 20.f, TopY + 98.f, 84.f, TEXT("Stage"), FString::Printf(TEXT("%d/%d"), MiniGameState->WaveIndex, MaxStageIndex), PrimaryText);
	DrawChip(TimerPanelX + 116.f, TopY + 98.f, 82.f, TEXT("Gold"), FString::FromInt(PlayerPawn->GetGold()), GoldAccent);
	DrawChip(TimerPanelX + 210.f, TopY + 98.f, 86.f, TEXT("Mats"), FString::FromInt(PlayerPawn->GetMaterials()), TealAccent);

	const float CombatPanelW = 374.f;
	const float CombatPanelH = 170.f;
	const float CombatPanelX = ScreenW - CombatPanelW - SafeMargin;
	const float CombatPanelY = TopY;
	T66MiniDrawPanel(Canvas, CombatPanelX, CombatPanelY, CombatPanelW, CombatPanelH, FLinearColor(0.44f, 0.52f, 0.70f, 1.f), PanelFill);
	T66MiniDrawScaledText(Canvas, TitleFont, TEXT("COMBAT"), CombatPanelX + 18.f, CombatPanelY + 12.f, PrimaryText, 1.02f);
	DrawStatRow(CombatPanelX + 18.f, CombatPanelY + 46.f, TEXT("DMG"), FString::Printf(TEXT("%.1f"), PlayerPawn->GetDamageStat()), TEXT("ATK"), FString::Printf(TEXT("%.1f"), PlayerPawn->GetAttackSpeedStat()));
	DrawStatRow(CombatPanelX + 18.f, CombatPanelY + 82.f, TEXT("ARM"), FString::Printf(TEXT("%.1f"), PlayerPawn->GetArmorStat()), TEXT("CRIT"), FString::Printf(TEXT("%.0f%%"), PlayerPawn->GetCritChance() * 100.f));
	DrawStatRow(CombatPanelX + 18.f, CombatPanelY + 118.f, TEXT("REGEN"), FString::Printf(TEXT("%.1f"), PlayerPawn->GetPassiveRegenPerSecond()), TEXT("EVADE"), FString::Printf(TEXT("%.0f%%"), PlayerPawn->GetEvasionChance() * 100.f));
	T66MiniDrawScaledText(Canvas, BodyFont, FString::Printf(TEXT("RANGE %.0f   LIFE STEAL %.0f%%"), PlayerPawn->GetAttackRange(), PlayerPawn->GetLifeStealChance() * 100.f), CombatPanelX + 18.f, CombatPanelY + 148.f, MutedText, 0.92f);

	const float AbilityPanelY = CombatPanelY + CombatPanelH + 14.f;
	T66MiniDrawPanel(Canvas, CombatPanelX, AbilityPanelY, CombatPanelW, 138.f, GoldAccent, PanelFill);
	if (UltimateIconTexture)
	{
		T66MiniDrawTexture(Canvas, UltimateIconTexture, CombatPanelX + 18.f, AbilityPanelY + 16.f, 30.f, 30.f);
	}
	T66MiniDrawScaledText(Canvas, TitleFont, TEXT("ULTIMATE"), CombatPanelX + (UltimateIconTexture ? 58.f : 18.f), AbilityPanelY + 12.f, PrimaryText, 1.0f);
	if (MouseRightIconTexture)
	{
		T66MiniDrawTexture(Canvas, MouseRightIconTexture, CombatPanelX + CombatPanelW - 118.f, AbilityPanelY + 18.f, 22.f, 22.f);
	}
	T66MiniDrawScaledText(
		Canvas,
		BodyFont,
		PlayerPawn->IsUltimateReady() ? TEXT("READY") : FString::Printf(TEXT("%.1fs"), PlayerPawn->GetUltimateCooldownRemaining()),
		CombatPanelX + CombatPanelW - 90.f,
		AbilityPanelY + 21.f,
		PlayerPawn->IsUltimateReady() ? GreenAccent : MutedText,
		1.0f);
	T66MiniDrawScaledText(Canvas, BodyFont, PlayerPawn->GetUltimateLabel().ToUpper(), CombatPanelX + 18.f, AbilityPanelY + 54.f, GoldAccent, 1.0f);
	T66MiniDrawBar(Canvas, CombatPanelX + 18.f, AbilityPanelY + 82.f, CombatPanelW - 36.f, 16.f, UltPct, GoldAccent);
	if (PassiveIconTexture)
	{
		T66MiniDrawTexture(Canvas, PassiveIconTexture, CombatPanelX + 18.f, AbilityPanelY + 110.f, 20.f, 20.f);
	}
	T66MiniDrawScaledText(Canvas, BodyFont, FString::Printf(TEXT("PASSIVE  %s"), *PlayerPawn->GetPassiveLabel().ToUpper()), CombatPanelX + 46.f, AbilityPanelY + 112.f, MutedText, 0.92f);
	if (PlayerPawn->HasQuickReviveReady())
	{
		T66MiniDrawInsetPanel(Canvas, CombatPanelX + CombatPanelW - 118.f, AbilityPanelY + 82.f, 102.f, 22.f, FLinearColor(0.10f, 0.18f, 0.12f, 0.92f));
		T66MiniDrawScaledText(Canvas, BodyFont, TEXT("REVIVE"), CombatPanelX + CombatPanelW - 90.f, AbilityPanelY + 87.f, GreenAccent, 0.84f);
	}

	const float LoadoutDockW = 520.f;
	const float LoadoutDockH = 132.f;
	const float LoadoutDockX = (ScreenW * 0.5f) - (LoadoutDockW * 0.5f);
	const float LoadoutDockY = ScreenH - LoadoutDockH - 18.f;
	T66MiniDrawPanel(Canvas, LoadoutDockX, LoadoutDockY, LoadoutDockW, LoadoutDockH, FLinearColor(0.32f, 0.62f, 0.96f, 1.f), PanelFill);
	T66MiniDrawInsetPanel(Canvas, LoadoutDockX + 16.f, LoadoutDockY + 16.f, 94.f, 42.f, FLinearColor(0.10f, 0.14f, 0.20f, 0.94f));
	T66MiniDrawScaledText(Canvas, BodyFont, TEXT("LEVEL"), LoadoutDockX + 28.f, LoadoutDockY + 22.f, MutedText, 0.88f);
	T66MiniDrawScaledText(Canvas, TitleFont, FString::FromInt(PlayerPawn->GetHeroLevel()), LoadoutDockX + 86.f, LoadoutDockY + 19.f, PrimaryText, 1.0f, true);
	T66MiniDrawScaledText(
		Canvas,
		BodyFont,
		FString::Printf(TEXT("XP %.0f / %.0f"), PlayerPawn->GetExperience(), PlayerPawn->GetExperienceToNextLevel()),
		LoadoutDockX + 128.f,
		LoadoutDockY + 22.f,
		MutedText,
		0.92f);
	T66MiniDrawBar(Canvas, LoadoutDockX + 128.f, LoadoutDockY + 42.f, LoadoutDockW - 148.f, 16.f, XpPct, FLinearColor(0.32f, 0.70f, 1.0f, 1.f));
	if (DataSubsystem)
	{
		const TArray<FName>& EquippedIdolIDs = PlayerPawn->GetEquippedIdolIDs();
		const float IdolSlotSize = 74.f;
		const float IdolSpacing = 14.f;
		float IdolX = LoadoutDockX + 128.f;
		const float IdolY = LoadoutDockY + 66.f;
		for (int32 Index = 0; Index < UT66MiniFrontendStateSubsystem::MaxIdolSlots; ++Index)
		{
			T66MiniDrawInsetPanel(Canvas, IdolX, IdolY, IdolSlotSize, IdolSlotSize, FLinearColor(0.08f, 0.09f, 0.13f, 0.96f));
			if (EquippedIdolIDs.IsValidIndex(Index))
			{
				if (const FT66MiniIdolDefinition* IdolDefinition = DataSubsystem->FindIdol(EquippedIdolIDs[Index]))
				{
					T66MiniDrawTexture(Canvas, VisualSubsystem ? VisualSubsystem->LoadTextureByAssetPath(IdolDefinition->IconPath) : nullptr, IdolX + 6.f, IdolY + 6.f, IdolSlotSize - 12.f, IdolSlotSize - 12.f);
				}
			}
			IdolX += IdolSlotSize + IdolSpacing;
		}
	}

	if (DataSubsystem)
	{
		const TArray<FName>& OwnedItemIDs = PlayerPawn->GetOwnedItemIDs();
		TArray<TPair<FName, int32>> InventoryStacks;
		TMap<FName, int32> InventoryStackLookup;
		const int32 MaxInventorySlots = 6;
		for (int32 ItemIndex = OwnedItemIDs.Num() - 1; ItemIndex >= 0; --ItemIndex)
		{
			const FName ItemID = OwnedItemIDs[ItemIndex];
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

		const float InventoryPanelW = 374.f;
		const float InventoryPanelH = 220.f;
		const float InventoryPanelX = ScreenW - InventoryPanelW - SafeMargin;
		const float InventoryPanelY = ScreenH - InventoryPanelH - 18.f;
		const float InventorySlotSize = 68.f;
		const float InventorySlotPadding = 14.f;
		const int32 InventoryColumns = 3;
		const float InventoryGridX = InventoryPanelX + 22.f;
		const float InventoryGridY = InventoryPanelY + 66.f;

		T66MiniDrawPanel(Canvas, InventoryPanelX, InventoryPanelY, InventoryPanelW, InventoryPanelH, FLinearColor(0.76f, 0.58f, 0.88f, 1.f), PanelFill);
		T66MiniDrawScaledText(Canvas, TitleFont, TEXT("BAG"), InventoryPanelX + 18.f, InventoryPanelY + 12.f, PrimaryText, 1.0f);
		T66MiniDrawInsetPanel(Canvas, InventoryPanelX + InventoryPanelW - 132.f, InventoryPanelY + 14.f, 112.f, 28.f, FLinearColor(0.14f, 0.10f, 0.18f, 0.94f));
		T66MiniDrawScaledText(Canvas, BodyFont, FString::Printf(TEXT("%d STACKS"), InventoryStacks.Num()), InventoryPanelX + InventoryPanelW - 116.f, InventoryPanelY + 20.f, GoldAccent, 0.92f);

		for (int32 SlotIndex = 0; SlotIndex < MaxInventorySlots; ++SlotIndex)
		{
			const int32 Column = SlotIndex % InventoryColumns;
			const int32 Row = SlotIndex / InventoryColumns;
			const float SlotX = InventoryGridX + (Column * (InventorySlotSize + InventorySlotPadding));
			const float SlotY = InventoryGridY + (Row * (InventorySlotSize + InventorySlotPadding));
			const bool bFilledSlot = InventoryStacks.IsValidIndex(SlotIndex);
			T66MiniDrawInsetPanel(Canvas, SlotX, SlotY, InventorySlotSize, InventorySlotSize, bFilledSlot ? FLinearColor(0.12f, 0.11f, 0.16f, 0.98f) : FLinearColor(0.05f, 0.06f, 0.08f, 0.88f));
			if (!bFilledSlot)
			{
				continue;
			}

			const TPair<FName, int32>& Entry = InventoryStacks[SlotIndex];
			if (const FT66MiniItemDefinition* ItemDefinition = DataSubsystem->FindItem(Entry.Key))
			{
				UTexture2D* ItemTexture = VisualSubsystem
					? VisualSubsystem->LoadItemTexture(ItemDefinition->ItemID, ItemDefinition->IconPath)
					: nullptr;
				if (ItemTexture)
				{
					T66MiniDrawTexture(Canvas, ItemTexture, SlotX + 8.f, SlotY + 8.f, InventorySlotSize - 16.f, InventorySlotSize - 16.f);
				}
				else
				{
					T66MiniDrawScaledText(Canvas, BodyFont, T66MiniInventoryLabelForItem(Entry.Key), SlotX + (InventorySlotSize * 0.5f), SlotY + 24.f, PrimaryText, 0.88f, true);
				}
				T66MiniDrawCountBadge(Canvas, BodyFont, Entry.Value, SlotX + InventorySlotSize - 24.f, SlotY + 4.f);
			}
		}

		if (OwnedItemIDs.Num() == 0)
		{
			T66MiniDrawScaledText(Canvas, BodyFont, TEXT("Loot bags and crates feed this bag."), InventoryPanelX + 18.f, InventoryPanelY + 190.f, MutedText, 0.92f);
		}
		else if (InventoryStacks.Num() > MaxInventorySlots)
		{
			T66MiniDrawScaledText(Canvas, BodyFont, FString::Printf(TEXT("+%d MORE"), InventoryStacks.Num() - MaxInventorySlots), InventoryPanelX + 18.f, InventoryPanelY + 190.f, MutedText, 0.92f);
		}
	}

	AT66MiniEnemyBase* BossEnemy = MiniGameMode ? MiniGameMode->GetActiveBossEnemy() : nullptr;

	if (BossEnemy)
	{
		const float BossPct = BossEnemy->GetMaxHealth() > 0.f ? BossEnemy->GetCurrentHealth() / BossEnemy->GetMaxHealth() : 0.f;
		const float BossBarW = 452.f;
		const float BossBarX = (ScreenW * 0.5f) - (BossBarW * 0.5f);
		const float BossBarY = 142.f;
		T66MiniDrawPanel(Canvas, BossBarX - 10.f, BossBarY - 20.f, BossBarW + 20.f, 56.f, DangerAccent, FLinearColor(0.10f, 0.03f, 0.03f, 0.88f));
		T66MiniDrawScaledText(Canvas, BodyFont, T66MiniReadableId(BossEnemy->GetEnemyID()).ToUpper(), ScreenW * 0.5f, BossBarY - 10.f, FLinearColor(1.0f, 0.84f, 0.70f), 0.88f, true);
		T66MiniDrawBar(Canvas, BossBarX, BossBarY + 12.f, BossBarW, 14.f, BossPct, FLinearColor(0.94f, 0.24f, 0.20f, 1.f));
	}
	else if (MiniGameMode && MiniGameMode->HasBossTelegraphActive())
	{
		const float AlertW = 360.f;
		T66MiniDrawPanel(Canvas, (ScreenW * 0.5f) - (AlertW * 0.5f), 142.f, AlertW, 42.f, DangerAccent, FLinearColor(0.20f, 0.04f, 0.04f, 0.82f));
		T66MiniDrawScaledText(Canvas, BodyFont, FString::Printf(TEXT("BOSS INBOUND  %.1fs"), MiniGameMode->GetBossTelegraphRemaining()), ScreenW * 0.5f, 154.f, FLinearColor(1.0f, 0.86f, 0.68f), 0.90f, true);
	}

	if (RunState)
	{
		const float ControlsX = SafeMargin;
		const float ControlsY = ScreenH - 40.f;
		const float ControlsW = 420.f;
		T66MiniDrawInsetPanel(Canvas, ControlsX, ControlsY - 4.f, ControlsW, 28.f, FLinearColor(0.05f, 0.06f, 0.09f, 0.86f));
		if (CachedInteractKey == EKeys::LeftMouseButton && CachedUltimateKey == EKeys::RightMouseButton && MouseLeftIconTexture && MouseRightIconTexture)
		{
			T66MiniDrawTexture(Canvas, MouseLeftIconTexture, ControlsX + 10.f, ControlsY - 2.f, 18.f, 18.f);
			if (InteractHandIconTexture)
			{
				T66MiniDrawTexture(Canvas, InteractHandIconTexture, ControlsX + 32.f, ControlsY - 2.f, 18.f, 18.f);
			}
			T66MiniDrawScaledText(Canvas, BodyFont, TEXT("INTERACT"), ControlsX + 56.f, ControlsY, MutedText, 0.84f);
			T66MiniDrawScaledText(Canvas, BodyFont, TEXT("|"), ControlsX + 122.f, ControlsY, FLinearColor(0.42f, 0.46f, 0.52f), 0.84f);
			T66MiniDrawTexture(Canvas, MouseRightIconTexture, ControlsX + 138.f, ControlsY - 2.f, 18.f, 18.f);
			if (UltimateIconTexture)
			{
				T66MiniDrawTexture(Canvas, UltimateIconTexture, ControlsX + 160.f, ControlsY - 2.f, 18.f, 18.f);
			}
			T66MiniDrawScaledText(Canvas, BodyFont, TEXT("ULTIMATE"), ControlsX + 182.f, ControlsY, MutedText, 0.84f);
			T66MiniDrawScaledText(Canvas, BodyFont, TEXT("|"), ControlsX + 252.f, ControlsY, FLinearColor(0.42f, 0.46f, 0.52f), 0.84f);
			T66MiniDrawScaledText(Canvas, BodyFont, FString::Printf(TEXT("%s PAUSE"), *CachedPauseKeyLabel), ControlsX + 268.f, ControlsY, MutedText, 0.84f);
		}
		else
		{
			const FString ControlsLabel = FString::Printf(
				TEXT("%s INTERACT  |  %s ULTIMATE  |  %s PAUSE"),
				*CachedInteractKeyLabel,
				*CachedUltimateKeyLabel,
				*CachedPauseKeyLabel);
			T66MiniDrawScaledText(Canvas, BodyFont, ControlsLabel, ControlsX + 12.f, ControlsY, MutedText, 0.84f);
		}
	}

	if (MiniGameMode && GetOwningPlayerController() && (!PlayerSettings || PlayerSettings->GetShowDamageNumbers()))
	{
		for (const FT66MiniCombatTextEntry& Entry : MiniGameMode->GetCombatTexts())
		{
			FVector2D ScreenPosition = FVector2D::ZeroVector;
			if (!GetOwningPlayerController()->ProjectWorldLocationToScreen(Entry.WorldLocation, ScreenPosition))
			{
				continue;
			}

			const float Alpha = FMath::Clamp(Entry.RemainingTime / 0.9f, 0.f, 1.f);
			T66MiniDrawText(Canvas, TitleFont, Entry.Label, ScreenPosition.X, ScreenPosition.Y, FLinearColor(Entry.Color.R, Entry.Color.G, Entry.Color.B, Alpha), true);
		}
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
							: nullptr;
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
