// Copyright Tribulation 66. All Rights Reserved.

#include "UI/WidgetGames/T66WidgetGameRegistry.h"

#include "Gameplay/T66ArcadeGameCatalog.h"
#include "UI/T66ArcadePopupWidget.h"
#include "UI/T66GoldMinerArcadeWidget.h"
#include "UI/T66QuickArcadeWidget.h"
#include "UI/T66TopwarArcadeWidget.h"
#include "UI/T66WhackAMoleArcadeWidget.h"
#include "UI/T66DemoModeUIUtils.h"

namespace
{
	FT66WidgetGameDescriptor MakeArcadeDescriptor(
		const ET66ArcadeGameType GameType,
		TSubclassOf<UUserWidget> WidgetClass,
		const int32 SortOrder,
		const bool bUsesCustomPaint)
	{
		FT66WidgetGameDescriptor Descriptor;
		Descriptor.GameID = T66ArcadeGameCatalog::GetRowID(GameType);
		Descriptor.LegacyGameID = Descriptor.GameID;
		Descriptor.Category = ET66WidgetGameCategory::Arcade;
		Descriptor.PlayModel = ET66WidgetGamePlayModel::RealtimePopup;
		Descriptor.DisplayName = T66ArcadeGameCatalog::GetPrototypeDisplayName(GameType);
		Descriptor.Description = T66ArcadeGameCatalog::GetDescription(GameType);
		Descriptor.ShortCode = T66ArcadeGameCatalog::GetShortCode(GameType);
		Descriptor.AccentColor = T66ArcadeGameCatalog::GetAccentColor(GameType, SortOrder);
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
			MakeArcadeDescriptor(ET66ArcadeGameType::WhackAMole, UT66WhackAMoleArcadeWidget::StaticClass(), 10, false),
			MakeArcadeDescriptor(ET66ArcadeGameType::Topwar, UT66TopwarArcadeWidget::StaticClass(), 20, false),
			MakeArcadeDescriptor(ET66ArcadeGameType::GoldMiner, UT66GoldMinerArcadeWidget::StaticClass(), 30, false),
			MakeArcadeDescriptor(ET66ArcadeGameType::BladeSweep, UT66QuickArcadeWidget::StaticClass(), 40, false),

			MakeCasinoDescriptor(
				FName(TEXT("Casino_CoinFlip")),
				FName(TEXT("CoinFlip")),
				ET66WidgetGamePlayModel::TurnCasino,
				NSLOCTEXT("T66.WidgetGames", "CasinoCoinFlipName", "Coin Flip"),
				NSLOCTEXT("T66.WidgetGames", "CasinoCoinFlipDescription", "Bet on heads or tails against the gambler."),
				NSLOCTEXT("T66.WidgetGames", "CasinoCoinFlipCode", "COIN"),
				FLinearColor(0.95f, 0.76f, 0.20f, 1.f),
				110),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_Rps")),
				FName(TEXT("RockPaperScissors")),
				ET66WidgetGamePlayModel::TurnCasino,
				NSLOCTEXT("T66.WidgetGames", "CasinoRpsName", "Rock Paper Scissors"),
				NSLOCTEXT("T66.WidgetGames", "CasinoRpsDescription", "Pick rock, paper, or scissors against the gambler."),
				NSLOCTEXT("T66.WidgetGames", "CasinoRpsCode", "RPS"),
				FLinearColor(0.62f, 0.72f, 1.f, 1.f),
				120),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_BlackJack")),
				FName(TEXT("BlackJack")),
				ET66WidgetGamePlayModel::TurnCasino,
				NSLOCTEXT("T66.WidgetGames", "CasinoBlackJackName", "BlackJack"),
				NSLOCTEXT("T66.WidgetGames", "CasinoBlackJackDescription", "Play a wagered hand against the gambler."),
				NSLOCTEXT("T66.WidgetGames", "CasinoBlackJackCode", "BJ"),
				FLinearColor(0.24f, 0.84f, 0.42f, 1.f),
				130),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_Lottery")),
				FName(TEXT("Lottery")),
				ET66WidgetGamePlayModel::PhysicalCasino,
				NSLOCTEXT("T66.WidgetGames", "CasinoLotteryName", "Lottery"),
				NSLOCTEXT("T66.WidgetGames", "CasinoLotteryDescription", "Pick numbers for a chance payout."),
				NSLOCTEXT("T66.WidgetGames", "CasinoLotteryCode", "LOT"),
				FLinearColor(0.90f, 0.32f, 0.56f, 1.f),
				140),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_Plinko")),
				FName(TEXT("Plinko")),
				ET66WidgetGamePlayModel::PhysicalCasino,
				NSLOCTEXT("T66.WidgetGames", "CasinoPlinkoName", "Plinko"),
				NSLOCTEXT("T66.WidgetGames", "CasinoPlinkoDescription", "Drop the ball through pegs for a wagered payout."),
				NSLOCTEXT("T66.WidgetGames", "CasinoPlinkoCode", "PLK"),
				FLinearColor(0.16f, 0.82f, 0.78f, 1.f),
				150),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_BoxOpening")),
				FName(TEXT("BoxOpening")),
				ET66WidgetGamePlayModel::PhysicalCasino,
				NSLOCTEXT("T66.WidgetGames", "CasinoBoxOpeningName", "Box Opening"),
				NSLOCTEXT("T66.WidgetGames", "CasinoBoxOpeningDescription", "Open a wagered box and reveal the result."),
				NSLOCTEXT("T66.WidgetGames", "CasinoBoxOpeningCode", "BOX"),
				FLinearColor(0.98f, 0.24f, 0.46f, 1.f),
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

bool T66WidgetGames::Registry::IsAvailable(
	const UObject* WorldContext,
	const FT66WidgetGameDescriptor& Descriptor)
{
	const FName DemoGateID = ResolveDemoGateID(Descriptor);
	switch (Descriptor.DemoGateKind)
	{
	case ET66WidgetGameDemoGateKind::None:
		return true;

	case ET66WidgetGameDemoGateKind::ArcadeAllowList:
		return T66DemoModeUI::IsArcadeGameAllowed(WorldContext, DemoGateID);

	case ET66WidgetGameDemoGateKind::CasinoAllowList:
		return T66DemoModeUI::IsCasinoGameAllowed(WorldContext, DemoGateID);

	case ET66WidgetGameDemoGateKind::FrontendMinigameLocked:
		return !T66DemoModeUI::IsDemoModeActive(WorldContext);

	default:
		return true;
	}
}

TSubclassOf<UUserWidget> T66WidgetGames::Registry::ResolveWidgetClass(
	const FT66WidgetGameDescriptor& Descriptor)
{
	return Descriptor.WidgetClass;
}
