// Copyright Tribulation 66. All Rights Reserved.

#include "UI/WidgetGames/T66WidgetGameRegistry.h"

#include "UI/T66ArcadePopupWidget.h"
#include "UI/T66GoldMinerArcadeWidget.h"
#include "UI/T66BladeSweepArcadeWidget.h"
#include "UI/T66TopwarArcadeWidget.h"
#include "UI/T66WhackAMoleArcadeWidget.h"
#include "Core/T66DeprecatedFeatureSettings.h"
#include "Core/T66ReleaseVariantSubsystem.h"
#include "UI/Gambler/T66BlackJackGameWidget.h"
#include "UI/Gambler/T66BoxOpeningGameWidget.h"
#include "UI/Gambler/T66CoinFlipGameWidget.h"
#include "UI/Gambler/T66LotteryGameWidget.h"
#include "UI/Gambler/T66PlinkoGameWidget.h"
#include "UI/Gambler/T66RpsGameWidget.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	FT66WidgetGameDescriptor MakeArcadeDescriptor(
		const ET66ArcadeGameType GameType,
		const FName GameID,
		const FText DisplayName,
		const FText Description,
		const FText ShortCode,
		const FLinearColor AccentColor,
		TSubclassOf<UUserWidget> WidgetClass,
		const int32 SortOrder,
		const bool bUsesCustomPaint)
	{
		FT66WidgetGameDescriptor Descriptor;
		Descriptor.GameID = GameID;
		Descriptor.LegacyGameID = Descriptor.GameID;
		Descriptor.Category = ET66WidgetGameCategory::Arcade;
		Descriptor.PlayModel = ET66WidgetGamePlayModel::RealtimePopup;
		Descriptor.DisplayName = DisplayName;
		Descriptor.Description = Description;
		Descriptor.ShortCode = ShortCode;
		Descriptor.AccentColor = AccentColor;
		Descriptor.DemoGateID = Descriptor.GameID;
		Descriptor.DemoGateKind = ET66WidgetGameDemoGateKind::ArcadeAllowList;
		Descriptor.LaunchKind = ET66WidgetGameLaunchKind::ArcadePopup;
		Descriptor.WidgetClass = WidgetClass;
		Descriptor.ArcadeGameType = GameType;
		Descriptor.AssetRoot = FName(TEXT("RuntimeDependencies/T66/Arcade"));
		Descriptor.SortOrder = SortOrder;
		Descriptor.Capabilities.bUsesCustomPaint = bUsesCustomPaint;
		Descriptor.Capabilities.bUsesScoreResult = true;
		return Descriptor;
	}

	FT66WidgetGameDescriptor MakeCasinoDescriptor(
		const FName GameID,
		const FName LegacyGameID,
		const ET66WidgetGamePlayModel PlayModel,
		const FText DisplayName,
		const FText Description,
		const FText ShortCode,
		const FLinearColor AccentColor,
		TSubclassOf<UUserWidget> WidgetClass,
		const int32 SortOrder)
	{
		FT66WidgetGameDescriptor Descriptor;
		Descriptor.GameID = GameID;
		Descriptor.LegacyGameID = LegacyGameID;
		Descriptor.Category = ET66WidgetGameCategory::Casino;
		Descriptor.PlayModel = PlayModel;
		Descriptor.DisplayName = DisplayName;
		Descriptor.Description = Description;
		Descriptor.ShortCode = ShortCode;
		Descriptor.AccentColor = AccentColor;
		Descriptor.DemoGateID = LegacyGameID;
		Descriptor.DemoGateKind = ET66WidgetGameDemoGateKind::CasinoAllowList;
		Descriptor.LaunchKind = ET66WidgetGameLaunchKind::CasinoChildWidget;
		Descriptor.WidgetClass = WidgetClass;
		Descriptor.CasinoPageID = LegacyGameID;
		Descriptor.AssetRoot = FName(TEXT("/Game/UI/Sprites/Games"));
		Descriptor.SortOrder = SortOrder;
		Descriptor.Capabilities.bUsesWager = true;
		Descriptor.Capabilities.bUsesScoreResult = true;
		return Descriptor;
	}

	FT66WidgetGameDescriptor MakeFrontendDescriptor(
		const FName GameID,
		const FName LegacyGameID,
		const ET66WidgetGamePlayModel PlayModel,
		const ET66ScreenType FrontendScreenType,
		const FText DisplayName,
		const FText Description,
		const FText ShortCode,
		const FLinearColor AccentColor,
		const FName AssetRoot,
		const FName BackendGameToken,
		const int32 SortOrder,
		const bool bUsesCustomPaint,
		const bool bUsesPersistentRun)
	{
		FT66WidgetGameDescriptor Descriptor;
		Descriptor.GameID = GameID;
		Descriptor.LegacyGameID = LegacyGameID;
		Descriptor.Category = ET66WidgetGameCategory::Frontend;
		Descriptor.PlayModel = PlayModel;
		Descriptor.DisplayName = DisplayName;
		Descriptor.Description = Description;
		Descriptor.ShortCode = ShortCode;
		Descriptor.AccentColor = AccentColor;
		Descriptor.DemoGateID = LegacyGameID;
		Descriptor.DemoGateKind = ET66WidgetGameDemoGateKind::FrontendMinigameLocked;
		Descriptor.LaunchKind = ET66WidgetGameLaunchKind::FrontendScreen;
		Descriptor.FrontendScreenType = FrontendScreenType;
		Descriptor.AssetRoot = AssetRoot;
		Descriptor.BackendGameToken = BackendGameToken;
		Descriptor.SortOrder = SortOrder;
		Descriptor.Capabilities.bUsesCustomPaint = bUsesCustomPaint;
		Descriptor.Capabilities.bUsesPersistentRun = bUsesPersistentRun;
		Descriptor.Capabilities.bUsesScoreResult = true;
		return Descriptor;
	}

	const TArray<FT66WidgetGameDescriptor>& GetDescriptorStorage()
	{
		static const TArray<FT66WidgetGameDescriptor> Descriptors = {
			MakeArcadeDescriptor(
				ET66ArcadeGameType::WhackAMole,
				FName(TEXT("Arcade_WhackAMole")),
				NSLOCTEXT("T66.ArcadeCatalog", "WhackAMolePrototypeName", "WHAC-A-MOLE COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "WhackAMoleDescription", "Fast target bonks across a lit 3x3 board."),
				NSLOCTEXT("T66.ArcadeCatalog", "WhackAMoleCode", "MOL"),
				FLinearColor(0.24f, 0.84f, 0.42f, 1.f),
				UT66WhackAMoleArcadeWidget::StaticClass(),
				10,
				false),
			MakeArcadeDescriptor(
				ET66ArcadeGameType::Topwar,
				FName(TEXT("Arcade_Topwar")),
				NSLOCTEXT("T66.ArcadeCatalog", "TopwarPrototypeName", "TOP WAR COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "TopwarDescription", "Choose power gates and grow the squad score."),
				NSLOCTEXT("T66.ArcadeCatalog", "TopwarCode", "TOP"),
				FLinearColor(0.95f, 0.44f, 0.16f, 1.f),
				UT66TopwarArcadeWidget::StaticClass(),
				20,
				false),
			MakeArcadeDescriptor(
				ET66ArcadeGameType::GoldMiner,
				FName(TEXT("Arcade_GoldMiner")),
				NSLOCTEXT("T66.ArcadeCatalog", "GoldMinerPrototypeName", "GOLD MINER COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "GoldMinerDescription", "Swing, hook, and reel treasure from the pit."),
				NSLOCTEXT("T66.ArcadeCatalog", "GoldMinerCode", "GLD"),
				FLinearColor(0.95f, 0.76f, 0.20f, 1.f),
				UT66GoldMinerArcadeWidget::StaticClass(),
				30,
				false),
			MakeArcadeDescriptor(
				ET66ArcadeGameType::BladeSweep,
				FName(TEXT("Arcade_BladeSweep")),
				NSLOCTEXT("T66.ArcadeCatalog", "BladeSweepPrototypeName", "FRUIT NINJA COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "BladeSweepDescription", "Sweep cursed fruit and dodge bad tiles."),
				NSLOCTEXT("T66.ArcadeCatalog", "BladeSweepCode", "BLD"),
				FLinearColor(0.98f, 0.24f, 0.46f, 1.f),
				UT66BladeSweepArcadeWidget::StaticClass(),
				40,
				false),
			MakeArcadeDescriptor(
				ET66ArcadeGameType::RuneSwipe,
				FName(TEXT("Arcade_RuneSwipe")),
				NSLOCTEXT("T66.ArcadeCatalog", "RuneSwipePrototypeName", "PIANO TILES COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "RuneSwipeDescription", "Tap the glowing rune chain before it fades."),
				NSLOCTEXT("T66.ArcadeCatalog", "RuneSwipeCode", "RUN"),
				FLinearColor(0.16f, 0.82f, 0.78f, 1.f),
				UT66BladeSweepArcadeWidget::StaticClass(),
				50,
				false),
			MakeArcadeDescriptor(
				ET66ArcadeGameType::CartSwitcher,
				FName(TEXT("Arcade_CartSwitcher")),
				NSLOCTEXT("T66.ArcadeCatalog", "CartSwitcherPrototypeName", "TRAIN TRACK COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "CartSwitcherDescription", "Switch mine tracks into the active lane."),
				NSLOCTEXT("T66.ArcadeCatalog", "CartSwitcherCode", "CRT"),
				FLinearColor(0.94f, 0.56f, 0.18f, 1.f),
				UT66BladeSweepArcadeWidget::StaticClass(),
				60,
				false),
			MakeArcadeDescriptor(
				ET66ArcadeGameType::CrystalDash,
				FName(TEXT("Arcade_CrystalDash")),
				NSLOCTEXT("T66.ArcadeCatalog", "CrystalDashPrototypeName", "SUBWAY SURFERS COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "CrystalDashDescription", "Dash through crystals while avoiding hazards."),
				NSLOCTEXT("T66.ArcadeCatalog", "CrystalDashCode", "DSH"),
				FLinearColor(0.62f, 0.72f, 1.f, 1.f),
				UT66BladeSweepArcadeWidget::StaticClass(),
				70,
				false),
			MakeArcadeDescriptor(
				ET66ArcadeGameType::PotionPour,
				FName(TEXT("Arcade_PotionPour")),
				NSLOCTEXT("T66.ArcadeCatalog", "PotionPourPrototypeName", "PERFECT POUR COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "PotionPourDescription", "Stop the pour on the glowing mark."),
				NSLOCTEXT("T66.ArcadeCatalog", "PotionPourCode", "POT"),
				FLinearColor(0.90f, 0.32f, 0.56f, 1.f),
				UT66BladeSweepArcadeWidget::StaticClass(),
				80,
				false),
			MakeArcadeDescriptor(
				ET66ArcadeGameType::RelicStack,
				FName(TEXT("Arcade_RelicStack")),
				NSLOCTEXT("T66.ArcadeCatalog", "RelicStackPrototypeName", "STACK COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "RelicStackDescription", "Drop moving relics over the center stack."),
				NSLOCTEXT("T66.ArcadeCatalog", "RelicStackCode", "REL"),
				FLinearColor(0.72f, 0.92f, 0.28f, 1.f),
				UT66BladeSweepArcadeWidget::StaticClass(),
				90,
				false),
			MakeArcadeDescriptor(
				ET66ArcadeGameType::ShieldParry,
				FName(TEXT("Arcade_ShieldParry")),
				NSLOCTEXT("T66.ArcadeCatalog", "ShieldParryPrototypeName", "ARROW PARRY COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "ShieldParryDescription", "Parry the lit projectile direction."),
				NSLOCTEXT("T66.ArcadeCatalog", "ShieldParryCode", "SHD"),
				FLinearColor(0.18f, 0.72f, 1.f, 1.f),
				UT66BladeSweepArcadeWidget::StaticClass(),
				100,
				false),
			MakeArcadeDescriptor(
				ET66ArcadeGameType::MimicMemory,
				FName(TEXT("Arcade_MimicMemory")),
				NSLOCTEXT("T66.ArcadeCatalog", "MimicMemoryPrototypeName", "SIMON SAYS COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "MimicMemoryDescription", "Repeat the chest sequence under pressure."),
				NSLOCTEXT("T66.ArcadeCatalog", "MimicMemoryCode", "MEM"),
				FLinearColor(0.88f, 0.12f, 0.10f, 1.f),
				UT66BladeSweepArcadeWidget::StaticClass(),
				110,
				false),
			MakeArcadeDescriptor(
				ET66ArcadeGameType::BombSorter,
				FName(TEXT("Arcade_BombSorter")),
				NSLOCTEXT("T66.ArcadeCatalog", "BombSorterPrototypeName", "SORTING GAME COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "BombSorterDescription", "Sort the lit bomb into the matching chute."),
				NSLOCTEXT("T66.ArcadeCatalog", "BombSorterCode", "BOM"),
				FLinearColor(0.94f, 0.18f, 0.14f, 1.f),
				UT66BladeSweepArcadeWidget::StaticClass(),
				120,
				false),
			MakeArcadeDescriptor(
				ET66ArcadeGameType::LanternLeap,
				FName(TEXT("Arcade_LanternLeap")),
				NSLOCTEXT("T66.ArcadeCatalog", "LanternLeapPrototypeName", "DOODLE JUMP COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "LanternLeapDescription", "Leap onto the glowing lantern platform."),
				NSLOCTEXT("T66.ArcadeCatalog", "LanternLeapCode", "LMP"),
				FLinearColor(0.54f, 0.95f, 0.36f, 1.f),
				UT66BladeSweepArcadeWidget::StaticClass(),
				130,
				false),

			MakeCasinoDescriptor(
				FName(TEXT("Casino_CoinFlip")),
				FName(TEXT("CoinFlip")),
				ET66WidgetGamePlayModel::TurnCasino,
				NSLOCTEXT("T66.WidgetGames", "CasinoCoinFlipName", "Coin Flip"),
				NSLOCTEXT("T66.WidgetGames", "CasinoCoinFlipDescription", "Bet on heads or tails against the gambler."),
				NSLOCTEXT("T66.WidgetGames", "CasinoCoinFlipCode", "COIN"),
				FLinearColor(0.95f, 0.76f, 0.20f, 1.f),
				UT66CoinFlipGameWidget::StaticClass(),
				110),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_Rps")),
				FName(TEXT("RockPaperScissors")),
				ET66WidgetGamePlayModel::TurnCasino,
				NSLOCTEXT("T66.WidgetGames", "CasinoRpsName", "Rock Paper Scissors"),
				NSLOCTEXT("T66.WidgetGames", "CasinoRpsDescription", "Pick rock, paper, or scissors against the gambler."),
				NSLOCTEXT("T66.WidgetGames", "CasinoRpsCode", "RPS"),
				FLinearColor(0.62f, 0.72f, 1.f, 1.f),
				UT66RpsGameWidget::StaticClass(),
				120),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_BlackJack")),
				FName(TEXT("BlackJack")),
				ET66WidgetGamePlayModel::TurnCasino,
				NSLOCTEXT("T66.WidgetGames", "CasinoBlackJackName", "BlackJack"),
				NSLOCTEXT("T66.WidgetGames", "CasinoBlackJackDescription", "Play a wagered hand against the gambler."),
				NSLOCTEXT("T66.WidgetGames", "CasinoBlackJackCode", "BJ"),
				FLinearColor(0.24f, 0.84f, 0.42f, 1.f),
				UT66BlackJackGameWidget::StaticClass(),
				130),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_Lottery")),
				FName(TEXT("Lottery")),
				ET66WidgetGamePlayModel::PhysicalCasino,
				NSLOCTEXT("T66.WidgetGames", "CasinoLotteryName", "Lottery"),
				NSLOCTEXT("T66.WidgetGames", "CasinoLotteryDescription", "Pick numbers for a chance payout."),
				NSLOCTEXT("T66.WidgetGames", "CasinoLotteryCode", "LOT"),
				FLinearColor(0.90f, 0.32f, 0.56f, 1.f),
				UT66LotteryGameWidget::StaticClass(),
				140),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_Plinko")),
				FName(TEXT("Plinko")),
				ET66WidgetGamePlayModel::PhysicalCasino,
				NSLOCTEXT("T66.WidgetGames", "CasinoPlinkoName", "Plinko"),
				NSLOCTEXT("T66.WidgetGames", "CasinoPlinkoDescription", "Drop the ball through pegs for a wagered payout."),
				NSLOCTEXT("T66.WidgetGames", "CasinoPlinkoCode", "PLK"),
				FLinearColor(0.16f, 0.82f, 0.78f, 1.f),
				UT66PlinkoGameWidget::StaticClass(),
				150),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_CupGame")),
				FName(TEXT("BoxOpening")),
				ET66WidgetGamePlayModel::PhysicalCasino,
				NSLOCTEXT("T66.WidgetGames", "CasinoBoxOpeningName", "Box Opening"),
				NSLOCTEXT("T66.WidgetGames", "CasinoBoxOpeningDescription", "Open a wagered box and reveal the result."),
				NSLOCTEXT("T66.WidgetGames", "CasinoBoxOpeningCode", "BOX"),
				FLinearColor(0.98f, 0.24f, 0.46f, 1.f),
				UT66BoxOpeningGameWidget::StaticClass(),
				160),

			MakeFrontendDescriptor(
				FName(TEXT("Frontend_TD")),
				FName(TEXT("TD")),
				ET66WidgetGamePlayModel::RealtimeBoard,
				ET66ScreenType::TDMainMenu,
				NSLOCTEXT("T66.WidgetGames", "FrontendTDName", "Chadpocalypse TD"),
				NSLOCTEXT("T66.WidgetGames", "FrontendTDDescription", "Tower-defense minigame launched from the Minigames screen."),
				NSLOCTEXT("T66.WidgetGames", "FrontendTDCode", "TD"),
				FLinearColor(0.24f, 0.84f, 0.42f, 1.f),
				FName(TEXT("/Game/T66TD")),
				FName(TEXT("TD")),
				210,
				true,
				true),
			MakeFrontendDescriptor(
				FName(TEXT("Frontend_Deck")),
				FName(TEXT("Deck")),
				ET66WidgetGamePlayModel::CardRun,
				ET66ScreenType::DeckMainMenu,
				NSLOCTEXT("T66.WidgetGames", "FrontendDeckName", "Deck Builder"),
				NSLOCTEXT("T66.WidgetGames", "FrontendDeckDescription", "Card-run minigame launched from the Minigames screen."),
				NSLOCTEXT("T66.WidgetGames", "FrontendDeckCode", "DECK"),
				FLinearColor(0.72f, 0.42f, 0.96f, 1.f),
				FName(TEXT("/Game/T66Deck")),
				FName(TEXT("Deck")),
				220,
				false,
				true),
			MakeFrontendDescriptor(
				FName(TEXT("Frontend_Idle")),
				FName(TEXT("Idle")),
				ET66WidgetGamePlayModel::IdleProgression,
				ET66ScreenType::IdleMainMenu,
				NSLOCTEXT("T66.WidgetGames", "FrontendIdleName", "Idle"),
				NSLOCTEXT("T66.WidgetGames", "FrontendIdleDescription", "Idle progression minigame launched from the Minigames screen."),
				NSLOCTEXT("T66.WidgetGames", "FrontendIdleCode", "IDLE"),
				FLinearColor(0.95f, 0.76f, 0.20f, 1.f),
				FName(TEXT("/Game/T66Idle")),
				FName(TEXT("Idle")),
				230,
				false,
				true),
			MakeFrontendDescriptor(
				FName(TEXT("Frontend_Mini")),
				FName(TEXT("Mini")),
				ET66WidgetGamePlayModel::RealtimeBoard,
				ET66ScreenType::MiniMainMenu,
				NSLOCTEXT("T66.WidgetGames", "FrontendMiniName", "Mini Chadpocalypse"),
				NSLOCTEXT("T66.WidgetGames", "FrontendMiniDescription", "Widget battle minigame launched from the Minigames screen."),
				NSLOCTEXT("T66.WidgetGames", "FrontendMiniCode", "MINI"),
				FLinearColor(0.18f, 0.72f, 1.f, 1.f),
				FName(TEXT("/Game/T66Mini")),
				FName(TEXT("Mini")),
				240,
				true,
				true),
		};

		return Descriptors;
	}

	FName ResolveDemoGateID(const FT66WidgetGameDescriptor& Descriptor)
	{
		return Descriptor.DemoGateID.IsNone() ? Descriptor.GameID : Descriptor.DemoGateID;
	}

	const UT66ReleaseVariantSubsystem* GetReleaseVariantSubsystem(const UObject* WorldContext)
	{
		const UGameInstance* GameInstance = WorldContext ? UGameplayStatics::GetGameInstance(WorldContext) : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UT66ReleaseVariantSubsystem>() : nullptr;
	}
}

TConstArrayView<FT66WidgetGameDescriptor> T66WidgetGames::Registry::GetAllDescriptors()
{
	return GetDescriptorStorage();
}

const FT66WidgetGameDescriptor* T66WidgetGames::Registry::FindDescriptor(const FName GameID)
{
	if (GameID.IsNone())
	{
		return nullptr;
	}

	for (const FT66WidgetGameDescriptor& Descriptor : GetDescriptorStorage())
	{
		if (Descriptor.GameID == GameID)
		{
			return &Descriptor;
		}
	}

	return nullptr;
}

const FT66WidgetGameDescriptor* T66WidgetGames::Registry::FindByLegacyID(const FName LegacyID)
{
	if (LegacyID.IsNone())
	{
		return nullptr;
	}

	for (const FT66WidgetGameDescriptor& Descriptor : GetDescriptorStorage())
	{
		if (Descriptor.LegacyGameID == LegacyID)
		{
			return &Descriptor;
		}
	}

	return nullptr;
}

const FT66WidgetGameDescriptor* T66WidgetGames::Registry::FindByArcadeGameType(
	const ET66ArcadeGameType GameType)
{
	if (GameType == ET66ArcadeGameType::None || GameType == ET66ArcadeGameType::Random)
	{
		return nullptr;
	}

	for (const FT66WidgetGameDescriptor& Descriptor : GetDescriptorStorage())
	{
		if (Descriptor.Category == ET66WidgetGameCategory::Arcade && Descriptor.ArcadeGameType == GameType)
		{
			return &Descriptor;
		}
	}

	return nullptr;
}

void T66WidgetGames::Registry::GetByCategory(
	const ET66WidgetGameCategory Category,
	TArray<const FT66WidgetGameDescriptor*>& Out)
{
	Out.Reset();

	for (const FT66WidgetGameDescriptor& Descriptor : GetDescriptorStorage())
	{
		if (Descriptor.Category == Category)
		{
			Out.Add(&Descriptor);
		}
	}
}

void T66WidgetGames::Registry::GetArcadeDescriptors(
	TArray<const FT66WidgetGameDescriptor*>& OutDescriptors)
{
	GetByCategory(ET66WidgetGameCategory::Arcade, OutDescriptors);
	OutDescriptors.Sort([](const FT66WidgetGameDescriptor& Lhs, const FT66WidgetGameDescriptor& Rhs)
	{
		return Lhs.SortOrder < Rhs.SortOrder;
	});
}

FName T66WidgetGames::Registry::GetArcadeRowID(const ET66ArcadeGameType GameType)
{
	if (const FT66WidgetGameDescriptor* Descriptor = FindByArcadeGameType(GameType))
	{
		return Descriptor->GameID;
	}

	return NAME_None;
}

bool T66WidgetGames::Registry::IsAvailable(
	const UObject* WorldContext,
	const FT66WidgetGameDescriptor& Descriptor)
{
	if (Descriptor.Category == ET66WidgetGameCategory::Arcade
		&& T66DeprecatedFeatures::AreArcadeGamesDisabled())
	{
		return false;
	}

	if (Descriptor.DemoGateKind == ET66WidgetGameDemoGateKind::FrontendMinigameLocked
		&& T66DeprecatedFeatures::AreMinigamesDisabled())
	{
		return false;
	}

	const FName DemoGateID = ResolveDemoGateID(Descriptor);
	switch (Descriptor.DemoGateKind)
	{
	case ET66WidgetGameDemoGateKind::None:
		return true;

	case ET66WidgetGameDemoGateKind::ArcadeAllowList:
		if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetReleaseVariantSubsystem(WorldContext))
		{
			return ReleaseVariant->IsArcadeGameAllowed(DemoGateID);
		}
		return true;

	case ET66WidgetGameDemoGateKind::CasinoAllowList:
		if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetReleaseVariantSubsystem(WorldContext))
		{
			return ReleaseVariant->IsCasinoGameAllowed(DemoGateID);
		}
		return true;

	case ET66WidgetGameDemoGateKind::FrontendMinigameLocked:
		if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetReleaseVariantSubsystem(WorldContext))
		{
			return !ReleaseVariant->IsDemoModeActive();
		}
		return true;

	default:
		return true;
	}
}

TSubclassOf<UUserWidget> T66WidgetGames::Registry::ResolveWidgetClass(
	const FT66WidgetGameDescriptor& Descriptor)
{
	return Descriptor.WidgetClass;
}
