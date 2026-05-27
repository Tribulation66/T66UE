// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66ArcadeSelectionWidget.h"

#include "Gameplay/T66ArcadeInteractableBase.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/T66DemoModeUIUtils.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66Style.h"
#include "UI/WidgetGames/T66WidgetGameRegistry.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float GArcadeSelectorMachineWidth = 1500.f;
	constexpr float GArcadeSelectorMachineHeight = 838.f;
	constexpr float GArcadeSelectorScreenWidth = 900.f;
	constexpr float GArcadeSelectorScreenHeight = 340.f;
	constexpr int32 GArcadeSelectorColumns = 2;
}

TSharedRef<SWidget> UT66ArcadeSelectionWidget::RebuildWidget()
{
	LoadMachineArtworkBrush();

	GameOptions.Reset();
	if (AT66ArcadeInteractableBase* Source = GetSourceInteractable())
	{
		GameOptions = Source->BuildArcadeSelectionOptions();
	}
	else
	{
		TArray<const FT66WidgetGameDescriptor*> ArcadeDescriptors;
		T66WidgetGames::Registry::GetArcadeDescriptors(ArcadeDescriptors);
		for (const FT66WidgetGameDescriptor* Descriptor : ArcadeDescriptors)
		{
			if (!Descriptor)
			{
				continue;
			}

			FT66ArcadeInteractableData GameData;
			if (T66WidgetGames::Registry::BuildArcadeSessionDataForGame(this, Descriptor->ArcadeGameType, GameData))
			{
				GameOptions.Add(MoveTemp(GameData));
			}
		}
	}

	TSharedRef<SUniformGridPanel> GameGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(8.f));

	for (int32 Index = 0; Index < GameOptions.Num(); ++Index)
	{
		GameGrid->AddSlot(Index % GArcadeSelectorColumns, Index / GArcadeSelectorColumns)
		[
			BuildGameButton(GameOptions[Index], Index)
		];
	}

	const TSharedRef<SWidget> GameListContent = GameOptions.Num() > 0
		? StaticCastSharedRef<SWidget>(GameGrid)
		: StaticCastSharedRef<SWidget>(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.06f, 0.07f, 0.09f, 1.f))
			.Padding(FMargin(20.f))
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Arcade", "ArcadeSelectorNoGames", "NO GAME CARTRIDGES FOUND"))
				.Font(FT66FlatStyle::Tokens::FontBold(22))
				.ColorAndOpacity(FT66FlatStyle::Tokens::Danger)
				.Justification(ETextJustify::Center)
			]);

	const TSharedRef<SWidget> MachineScreenContent =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.006f, 0.012f, 0.020f, 0.86f))
		.Padding(FMargin(18.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Arcade", "ArcadeSelectorScreenTitle", "SELECT GAME"))
					.Font(FT66FlatStyle::Tokens::FontBold(26))
					.ColorAndOpacity(FLinearColor(0.25f, 0.92f, 1.f, 1.f))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Arcade", "ArcadeSelectorCreditReadout", "CREDIT 01"))
					.Font(FT66FlatStyle::Tokens::FontBold(18))
					.ColorAndOpacity(FLinearColor(1.f, 0.78f, 0.22f, 1.f))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.06f, 0.13f, 0.16f, 1.f))
				.Padding(FMargin(0.f))
				[
					SNew(SBox)
					.HeightOverride(4.f)
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(0.f, 16.f, 0.f, 0.f)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Vertical)
				+ SScrollBox::Slot()
				[
					GameListContent
				]
			]
		];

	const TSharedRef<SWidget> MachineScreen =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			MachineScreenContent
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			BuildCrtOverlay()
		];

	const TSharedRef<SWidget> ControlDeck =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.035f, 0.014f, 0.032f, 0.82f))
		.Padding(FMargin(18.f, 14.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Arcade", "ArcadeSelectorDeckLabel", "CHADCADE MULTI-CART"))
				.Font(FT66FlatStyle::Tokens::FontBold(20))
				.ColorAndOpacity(FLinearColor(1.f, 0.74f, 0.26f, 1.f))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				FT66FlatStyle::MakeButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.Arcade", "ArcadeSelectorExit", "Exit"),
						FOnClicked::CreateUObject(this, &UT66ArcadeSelectionWidget::HandleExitClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(170.f)
					.SetHeight(48.f)
					.SetFontSize(18))
			]
		];

	const TSharedRef<SWidget> Machine =
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.12f, 0.025f, 0.070f, 1.f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			BuildMachineArtworkLayer(1.f)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.f, 32.f, 0.f, 0.f))
		[
			SNew(SBox)
			.WidthOverride(920.f)
			.HeightOverride(82.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Arcade", "ArcadeSelectorMarquee", "CHADCADE"))
				.Font(FT66FlatStyle::Tokens::FontBold(44))
				.ColorAndOpacity(FLinearColor(1.f, 0.93f, 0.38f, 1.f))
				.ShadowOffset(FVector2D(0.f, 3.f))
				.ShadowColorAndOpacity(FLinearColor(0.34f, 0.f, 0.08f, 0.90f))
				.Justification(ETextJustify::Center)
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.f, 192.f, 0.f, 0.f))
		[
			SNew(SBox)
			.WidthOverride(GArcadeSelectorScreenWidth)
			.HeightOverride(GArcadeSelectorScreenHeight)
			[
				MachineScreen
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0.f, 0.f, 0.f, 42.f))
		[
			SNew(SBox)
			.WidthOverride(900.f)
			[
				ControlDeck
			]
		];

	const TSharedRef<SWidget> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.78f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(28.f))
		[
			SNew(SBox)
			.WidthOverride(GArcadeSelectorMachineWidth)
			.HeightOverride(GArcadeSelectorMachineHeight)
			[
				Machine
			]
		];

	return FT66FlatStyle::MakeResponsiveRoot(Root);
}

void UT66ArcadeSelectionWidget::LoadMachineArtworkBrush()
{
	if (MachineArtworkTexture.IsValid())
	{
		return;
	}

	for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(TEXT("SourceAssets/Arcade/Selector/arcade_selector_front_machine.png")))
	{
		if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Trilinear, TEXT("ArcadeSelectorMachine")))
		{
			MachineArtworkTexture.Reset(Texture);
			MachineArtworkBrush = FSlateBrush();
			MachineArtworkBrush.SetResourceObject(Texture);
			MachineArtworkBrush.DrawAs = ESlateBrushDrawType::Image;
			MachineArtworkBrush.Tiling = ESlateBrushTileType::NoTile;
			MachineArtworkBrush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
			return;
		}
	}
}

TSharedRef<SWidget> UT66ArcadeSelectionWidget::BuildMachineArtworkLayer(const float Opacity) const
{
	if (!MachineArtworkTexture.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SImage)
		.Image(&MachineArtworkBrush)
		.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, FMath::Clamp(Opacity, 0.f, 1.f)));
}

TSharedRef<SWidget> UT66ArcadeSelectionWidget::BuildCrtOverlay() const
{
	TSharedRef<SVerticalBox> Scanlines = SNew(SVerticalBox);
	for (int32 Index = 0; Index < 84; ++Index)
	{
		Scanlines->AddSlot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(1.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.18f))
			]
		];

		Scanlines->AddSlot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(4.f)
		];
	}

	return SNew(SOverlay)
		.Visibility(EVisibility::HitTestInvisible)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			Scanlines
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.08f, 0.22f, 0.25f, 0.10f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.0f))
			.Padding(FMargin(12.f))
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.15f, 0.95f, 1.f, 0.055f))
			]
		];
}

TSharedRef<SWidget> UT66ArcadeSelectionWidget::BuildGameButton(const FT66ArcadeInteractableData& GameData, const int32 Index)
{
	const FLinearColor Accent = ResolveGameAccentColor(GameData.ArcadeGameType, Index);
	const FText DisplayName = [GameType = GameData.ArcadeGameType]()
	{
		if (const FT66WidgetGameDescriptor* Descriptor = T66WidgetGames::Registry::FindByArcadeGameType(GameType))
		{
			return Descriptor->DisplayName;
		}

		return NSLOCTEXT("T66.ArcadeCatalog", "UnknownPrototypeName", "ARCADE COPY");
	}();
	const FT66WidgetGameDescriptor* GameDescriptor = T66WidgetGames::Registry::FindByArcadeGameType(GameData.ArcadeGameType);
	const bool bArcadeAllowed = !GameDescriptor || T66WidgetGames::Registry::IsAvailable(this, *GameDescriptor);

	FOnClicked GameClicked;
	if (bArcadeAllowed)
	{
		GameClicked = FOnClicked::CreateUObject(this, &UT66ArcadeSelectionWidget::HandleGameClicked, GameData.ArcadeGameType);
	}

	const TSharedRef<SWidget> Button = FT66FlatStyle::MakeBareButton(
		FT66BareButtonParams(
			MoveTemp(GameClicked),
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.025f, 0.030f, 0.040f, 1.f))
			.Padding(FMargin(4.f))
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Accent)
				.Padding(FMargin(4.f))
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.015f, 0.018f, 0.025f, 1.f))
					.Padding(FMargin(14.f, 10.f))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.f, 0.f, 14.f, 0.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(Accent)
							.Padding(FMargin(10.f, 8.f))
							[
								SNew(STextBlock)
								.Text(ResolveGameCode(GameData.ArcadeGameType))
								.Font(FT66FlatStyle::Tokens::FontBold(18))
								.ColorAndOpacity(FLinearColor(0.02f, 0.02f, 0.03f, 1.f))
								.Justification(ETextJustify::Center)
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.VAlign(VAlign_Center)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(DisplayName)
								.Font(FT66FlatStyle::Tokens::FontBold(22))
								.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 4.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(ResolveGameFlavorText(GameData.ArcadeGameType))
								.Font(FT66FlatStyle::Tokens::FontRegular(15))
								.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
								.AutoWrapText(true)
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(14.f, 0.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.Arcade", "ArcadeSelectorStartLabel", "START"))
							.Font(FT66FlatStyle::Tokens::FontBold(16))
							.ColorAndOpacity(bArcadeAllowed ? Accent : FT66FlatStyle::Tokens::TextMuted)
						]
					]
				]
			])
		.SetColor(FLinearColor::Transparent)
		.SetPadding(FMargin(0.f))
		.SetEnabled(bArcadeAllowed));

	return SNew(SBox)
		.HeightOverride(104.f)
		[
			T66DemoModeUI::WrapWithComingSoonOverlay(
				Button,
				!bArcadeAllowed,
				this,
				FName(*FString::Printf(TEXT("ArcadeSelector.Game.%02d.DemoOverlay"), Index + 1)))
		];
}

FReply UT66ArcadeSelectionWidget::HandleGameClicked(const ET66ArcadeGameType GameType)
{
	const FT66WidgetGameDescriptor* GameDescriptor = T66WidgetGames::Registry::FindByArcadeGameType(GameType);
	if (GameDescriptor && !T66WidgetGames::Registry::IsAvailable(this, *GameDescriptor))
	{
		return FReply::Handled();
	}

	AT66ArcadeInteractableBase* Source = GetSourceInteractable();
	if (!Source)
	{
		return HandleExitClicked();
	}

	FT66ArcadeInteractableData SelectedGameData;
	if (!Source->BuildArcadeSessionDataForGame(GameType, SelectedGameData))
	{
		return FReply::Handled();
	}

	if (AT66PlayerController* T66PC = GetOwningPlayer<AT66PlayerController>())
	{
		T66PC->HandleArcadeGameSelected(this, SelectedGameData);
	}
	return FReply::Handled();
}

FReply UT66ArcadeSelectionWidget::HandleExitClicked()
{
	if (AT66PlayerController* T66PC = GetOwningPlayer<AT66PlayerController>())
	{
		T66PC->CloseArcadePopup(false, 0);
		return FReply::Handled();
	}

	if (AT66ArcadeInteractableBase* Source = GetSourceInteractable())
	{
		Source->HandleArcadePopupDismissedWithoutResult();
	}
	RemoveFromParent();
	return FReply::Handled();
}

FText UT66ArcadeSelectionWidget::ResolveGameCode(const ET66ArcadeGameType GameType) const
{
	if (const FT66WidgetGameDescriptor* Descriptor = T66WidgetGames::Registry::FindByArcadeGameType(GameType))
	{
		return Descriptor->ShortCode;
	}

	return NSLOCTEXT("T66.ArcadeCatalog", "UnknownCode", "???");
}

FText UT66ArcadeSelectionWidget::ResolveGameFlavorText(const ET66ArcadeGameType GameType) const
{
	if (const FT66WidgetGameDescriptor* Descriptor = T66WidgetGames::Registry::FindByArcadeGameType(GameType))
	{
		return Descriptor->Description;
	}

	return NSLOCTEXT("T66.ArcadeCatalog", "UnknownDescription", "Boot the selected arcade machine cartridge.");
}

FLinearColor UT66ArcadeSelectionWidget::ResolveGameAccentColor(const ET66ArcadeGameType GameType, const int32 Index) const
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
