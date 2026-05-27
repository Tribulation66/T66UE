// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66VersusArcadeScreen.h"

#include "Core/T66DeprecatedFeatureSettings.h"
#include "Gameplay/T66PlayerController.h"
#include "Styling/CoreStyle.h"
#include "UI/T66DemoModeUIUtils.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/WidgetGames/T66WidgetGameRegistry.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 GVersusArcadeColumns = 4;

	FLinearColor ResolveArcadeAccentColor(const ET66ArcadeGameType GameType, const int32 Index)
	{
		if (const FT66WidgetGameDescriptor* Descriptor = T66WidgetGames::Registry::FindByArcadeGameType(GameType))
		{
			return Descriptor->AccentColor;
		}

		static const FLinearColor Palette[] =
		{
			FLinearColor(0.16f, 0.82f, 0.78f, 1.f),
			FLinearColor(0.94f, 0.56f, 0.18f, 1.f),
			FLinearColor(0.62f, 0.72f, 1.f, 1.f),
			FLinearColor(0.90f, 0.32f, 0.56f, 1.f),
			FLinearColor(0.72f, 0.92f, 0.28f, 1.f),
		};
		return Palette[FMath::Abs(Index) % UE_ARRAY_COUNT(Palette)];
	}

	TSharedRef<SWidget> MakeVersusText(
		const FText& Text,
		const int32 FontSize,
		const FLinearColor& Color,
		const bool bBold = true,
		const ETextJustify::Type Justification = ETextJustify::Left,
		const bool bAutoWrap = false)
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justification)
			.AutoWrapText(bAutoWrap)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds);
	}
}

UT66VersusArcadeScreen::UT66VersusArcadeScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::VersusMainMenu;
	bIsModal = false;
}

TSharedRef<SWidget> UT66VersusArcadeScreen::BuildSlateUI()
{
	TArray<const FT66WidgetGameDescriptor*> ArcadeDescriptors;
	T66WidgetGames::Registry::GetArcadeDescriptors(ArcadeDescriptors);
	TSharedRef<SUniformGridPanel> ArcadeGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(10.f));

	for (int32 Index = 0; Index < ArcadeDescriptors.Num(); ++Index)
	{
		if (!ArcadeDescriptors[Index])
		{
			continue;
		}

		ArcadeGrid->AddSlot(Index % GVersusArcadeColumns, Index / GVersusArcadeColumns)
		[
			BuildArcadeTile(ArcadeDescriptors[Index]->ArcadeGameType, Index)
		];
	}

	const TSharedRef<SWidget> Content =
		SNew(SBox)
		.Padding(FMargin(42.f, 34.f, 42.f, 34.f))
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
					MakeVersusText(NSLOCTEXT("T66.VersusArcade", "Title", "VERSUS"), 64, FT66FlatStyle::PrimaryText(), true)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					FT66FlatStyle::MakeFlatButton(
						ET66FlatState::Default,
						NSLOCTEXT("T66.VersusArcade", "BackButton", "BACK"),
						FOnClicked::CreateUObject(this, &UT66VersusArcadeScreen::HandleBackClicked),
						nullptr,
						nullptr,
						FMargin(22.f, 8.f),
						180.f,
						56.f,
						true,
						22,
						FName(TEXT("Versus.BackButton")))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 14.f, 0.f, 0.f)
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(24.f),
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							MakeVersusText(NSLOCTEXT("T66.VersusArcade", "PlayHeader", "ARCADE PROTOTYPES"), 28, FT66FlatStyle::SelectedText(), true)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 8.f, 0.f, 0.f)
						[
							MakeVersusText(NSLOCTEXT("T66.VersusArcade", "PlayBody", "Pick a prototype directly, or roll one at random."), 20, FT66FlatStyle::SecondaryText(), false)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						FT66FlatStyle::MakeFlatButton(
							ET66FlatState::Selected,
							NSLOCTEXT("T66.VersusArcade", "PlayRandom", "PLAY RANDOM"),
							FOnClicked::CreateUObject(this, &UT66VersusArcadeScreen::HandlePlayRandomClicked),
							nullptr,
							nullptr,
							FMargin(24.f, 10.f),
							270.f,
							64.f,
							true,
							25,
							FName(TEXT("Versus.PlayRandomButton")))
					],
					nullptr,
					FName(TEXT("Versus.HeaderPanel")))
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(0.f, 18.f, 0.f, 0.f)
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(18.f),
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						ArcadeGrid
					],
					nullptr,
					FName(TEXT("Versus.ArcadeGridPanel")))
			]
		];

	return T66ScreenSlateHelpers::MakeTopBarScreenRoot(
		UIManager,
		Content,
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black),
		FLinearColor::Transparent,
		FMargin(0.f, 0.f, 0.f, 0.f));
}

TSharedRef<SWidget> UT66VersusArcadeScreen::BuildArcadeTile(const ET66ArcadeGameType GameType, const int32 Index)
{
	const FT66WidgetGameDescriptor* Descriptor = T66WidgetGames::Registry::FindByArcadeGameType(GameType);
	const FLinearColor Accent = ResolveArcadeAccentColor(GameType, Index);
	const FName TileTag(*FString::Printf(TEXT("Versus.ArcadeTile.%02d"), Index + 1));
	const bool bArcadeAllowed = IsArcadeGameAllowed(GameType);
	const FText ShortCode = Descriptor ? Descriptor->ShortCode : NSLOCTEXT("T66.ArcadeCatalog", "UnknownCode", "???");
	const FText DisplayName = Descriptor ? Descriptor->DisplayName : NSLOCTEXT("T66.ArcadeCatalog", "UnknownPrototypeName", "ARCADE COPY");
	const FText Description = Descriptor ? Descriptor->Description : NSLOCTEXT("T66.ArcadeCatalog", "UnknownDescription", "Boot the selected arcade machine cartridge.");

	const TSharedRef<SWidget> TileContent =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.015f, 0.017f, 0.024f, 1.f))
		.Padding(FMargin(16.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.f, 0.f, 12.f, 0.f)
				[
					FT66FlatStyle::MakeFlatPanel(
						ET66FlatState::Selected,
						FMargin(10.f, 6.f),
						MakeVersusText(ShortCode, 17, FT66FlatStyle::PrimaryText(), true, ETextJustify::Center),
						nullptr,
						NAME_None)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					MakeVersusText(DisplayName, 20, FT66FlatStyle::PrimaryText(), true, ETextJustify::Left, true)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 12.f, 0.f, 0.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Accent)
				.Padding(FMargin(0.f))
				[
					SNew(SBox)
					.HeightOverride(4.f)
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(0.f, 12.f, 0.f, 0.f)
			[
				MakeVersusText(Description, 16, FT66FlatStyle::SecondaryText(), false, ETextJustify::Left, true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 14.f, 0.f, 0.f)
			.HAlign(HAlign_Right)
			[
				MakeVersusText(NSLOCTEXT("T66.VersusArcade", "StartLabel", "START"), 17, bArcadeAllowed ? Accent : FT66FlatStyle::TextMuted(), true)
			]
		];

	FOnClicked TileClicked;
	if (bArcadeAllowed)
	{
		TileClicked = FOnClicked::CreateUObject(this, &UT66VersusArcadeScreen::HandleGameClicked, GameType);
	}

	const TSharedRef<SWidget> TileButton = FT66FlatStyle::MakeFlatToggleGroupButton(
		bArcadeAllowed ? ET66FlatState::Default : ET66FlatState::Disabled,
		TileContent,
		MoveTemp(TileClicked),
		FMargin(0.f),
		0.f,
		204.f,
		bArcadeAllowed,
		TileTag);

	return T66DemoModeUI::WrapWithComingSoonOverlay(
		TileButton,
		!bArcadeAllowed,
		this,
		FName(*(TileTag.ToString() + TEXT(".DemoOverlay"))));
}

bool UT66VersusArcadeScreen::IsArcadeGameAllowed(const ET66ArcadeGameType GameType) const
{
	if (T66DeprecatedFeatures::AreArcadeGamesDisabled())
	{
		return false;
	}

	const FT66WidgetGameDescriptor* Descriptor = T66WidgetGames::Registry::FindByArcadeGameType(GameType);
	return !Descriptor || T66WidgetGames::Registry::IsAvailable(this, *Descriptor);
}

bool UT66VersusArcadeScreen::LaunchArcadeGame(const ET66ArcadeGameType GameType)
{
	if (!IsArcadeGameAllowed(GameType))
	{
		return false;
	}

	FT66ArcadeInteractableData ArcadeData;
	if (!T66WidgetGames::Registry::BuildArcadeSessionDataForGame(this, GameType, ArcadeData))
	{
		return false;
	}

	if (AT66PlayerController* T66PC = GetOwningPlayer<AT66PlayerController>())
	{
		return T66PC->OpenArcadePopupFromFrontend(ArcadeData);
	}

	return false;
}

void UT66VersusArcadeScreen::OnBackClicked()
{
	if (T66DeprecatedFeatures::AreMinigamesDisabled())
	{
		NavigateTo(ET66ScreenType::MainMenu);
		return;
	}

	NavigateTo(ET66ScreenType::Minigames);
}

FReply UT66VersusArcadeScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66VersusArcadeScreen::HandlePlayRandomClicked()
{
	TArray<const FT66WidgetGameDescriptor*> ArcadeDescriptors;
	T66WidgetGames::Registry::GetArcadeDescriptors(ArcadeDescriptors);
	TArray<ET66ArcadeGameType> AllowedGameTypes;
	AllowedGameTypes.Reserve(ArcadeDescriptors.Num());
	for (const FT66WidgetGameDescriptor* Descriptor : ArcadeDescriptors)
	{
		if (Descriptor && IsArcadeGameAllowed(Descriptor->ArcadeGameType))
		{
			AllowedGameTypes.Add(Descriptor->ArcadeGameType);
		}
	}

	if (AllowedGameTypes.Num() > 0)
	{
		LaunchArcadeGame(AllowedGameTypes[FMath::RandHelper(AllowedGameTypes.Num())]);
	}
	return FReply::Handled();
}

FReply UT66VersusArcadeScreen::HandleGameClicked(const ET66ArcadeGameType GameType)
{
	LaunchArcadeGame(GameType);
	return FReply::Handled();
}
