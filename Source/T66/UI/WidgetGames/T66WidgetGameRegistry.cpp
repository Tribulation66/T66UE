// Copyright Tribulation 66. All Rights Reserved.

#include "UI/WidgetGames/T66WidgetGameRegistry.h"

#include "Core/T66ReleaseVariantSubsystem.h"
#include "UI/Gambler/T66CoinFlipGameWidget.h"
#include "UI/Gambler/T66FindJokerGameWidget.h"
#include "UI/Gambler/T66GuessCupGameWidget.h"
#include "UI/Gambler/T66StickPickGameWidget.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	FT66WidgetGameDescriptor MakeCasinoDescriptor(
		const FName GameID,
		const FName DemoGateID,
		const FText DisplayName,
		const FText Description,
		const FText ShortCode,
		const FLinearColor AccentColor,
		TSubclassOf<UUserWidget> WidgetClass,
		const int32 SortOrder)
	{
		FT66WidgetGameDescriptor Descriptor;
		Descriptor.GameID = GameID;
		Descriptor.LegacyGameID = DemoGateID;
		Descriptor.Category = ET66WidgetGameCategory::Casino;
		Descriptor.PlayModel = ET66WidgetGamePlayModel::TurnCasino;
		Descriptor.DisplayName = DisplayName;
		Descriptor.Description = Description;
		Descriptor.ShortCode = ShortCode;
		Descriptor.AccentColor = AccentColor;
		Descriptor.DemoGateID = DemoGateID;
		Descriptor.DemoGateKind = ET66WidgetGameDemoGateKind::CasinoAllowList;
		Descriptor.LaunchKind = ET66WidgetGameLaunchKind::CasinoChildWidget;
		Descriptor.WidgetClass = WidgetClass;
		Descriptor.CasinoPageID = DemoGateID;
		Descriptor.AssetRoot = FName(TEXT("/Game/UI/Sprites/Games"));
		Descriptor.SortOrder = SortOrder;
		Descriptor.Capabilities.bUsesWager = true;
		Descriptor.Capabilities.bUsesScoreResult = true;
		return Descriptor;
	}

	const TArray<FT66WidgetGameDescriptor>& GetDescriptorStorage()
	{
		static const TArray<FT66WidgetGameDescriptor> Descriptors = {
			MakeCasinoDescriptor(
				FName(TEXT("Casino_CoinFlip")),
				FName(TEXT("CoinFlip")),
				NSLOCTEXT("T66.WidgetGames", "CasinoCoinFlipName", "Coin Flip"),
				NSLOCTEXT("T66.WidgetGames", "CasinoCoinFlipDescription", "Bet on heads or tails. Correct side pays 2x."),
				NSLOCTEXT("T66.WidgetGames", "CasinoCoinFlipCode", "2X"),
				FLinearColor(0.95f, 0.76f, 0.20f, 1.f),
				UT66CoinFlipGameWidget::StaticClass(),
				110),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_GuessTheCup")),
				FName(TEXT("GuessTheCup")),
				NSLOCTEXT("T66.WidgetGames", "CasinoGuessTheCupName", "Guess the Cup"),
				NSLOCTEXT("T66.WidgetGames", "CasinoGuessTheCupDescription", "Pick the cup hiding the token. Correct cup pays 3x."),
				NSLOCTEXT("T66.WidgetGames", "CasinoGuessTheCupCode", "3X"),
				FLinearColor(0.62f, 0.72f, 1.f, 1.f),
				UT66GuessCupGameWidget::StaticClass(),
				120),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_PickLongestShortestStick")),
				FName(TEXT("PickLongestShortestStick")),
				NSLOCTEXT("T66.WidgetGames", "CasinoPickStickName", "Pick the Stick"),
				NSLOCTEXT("T66.WidgetGames", "CasinoPickStickDescription", "Pick the called longest or shortest stick. Correct stick pays 5x."),
				NSLOCTEXT("T66.WidgetGames", "CasinoPickStickCode", "5X"),
				FLinearColor(0.24f, 0.84f, 0.42f, 1.f),
				UT66StickPickGameWidget::StaticClass(),
				130),
			MakeCasinoDescriptor(
				FName(TEXT("Casino_FindJoker")),
				FName(TEXT("FindJoker")),
				NSLOCTEXT("T66.WidgetGames", "CasinoFindJokerName", "Find the Joker"),
				NSLOCTEXT("T66.WidgetGames", "CasinoFindJokerDescription", "Pick the card hiding the Joker. Correct card pays 10x."),
				NSLOCTEXT("T66.WidgetGames", "CasinoFindJokerCode", "10X"),
				FLinearColor(0.98f, 0.24f, 0.46f, 1.f),
				UT66FindJokerGameWidget::StaticClass(),
				140),
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

void T66WidgetGames::Registry::GetByCategory(const ET66WidgetGameCategory Category, TArray<const FT66WidgetGameDescriptor*>& Out)
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

bool T66WidgetGames::Registry::IsAvailable(const UObject* WorldContext, const FT66WidgetGameDescriptor& Descriptor)
{
	const FName DemoGateID = ResolveDemoGateID(Descriptor);
	switch (Descriptor.DemoGateKind)
	{
	case ET66WidgetGameDemoGateKind::None:
		return true;

	case ET66WidgetGameDemoGateKind::CasinoAllowList:
		if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GetReleaseVariantSubsystem(WorldContext))
		{
			return ReleaseVariant->IsCasinoGameAllowed(DemoGateID);
		}
		return true;

	default:
		return true;
	}
}

TSubclassOf<UUserWidget> T66WidgetGames::Registry::ResolveWidgetClass(const FT66WidgetGameDescriptor& Descriptor)
{
	return Descriptor.WidgetClass;
}
