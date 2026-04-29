// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66ArcadeSelectionWidget.h"

#include "Gameplay/T66ArcadeInteractableBase.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

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
	constexpr float GArcadeSelectorCabinetWidth = 1500.f;
	constexpr float GArcadeSelectorCabinetHeight = 838.f;
	constexpr float GArcadeSelectorScreenWidth = 900.f;
	constexpr float GArcadeSelectorScreenHeight = 340.f;
	constexpr int32 GArcadeSelectorColumns = 2;
}

TSharedRef<SWidget> UT66ArcadeSelectionWidget::RebuildWidget()
{
	LoadCabinetArtworkBrush();

	GameOptions.Reset();
	if (AT66ArcadeInteractableBase* Source = GetSourceInteractable())
	{
		GameOptions = Source->BuildArcadeSelectionOptions();
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
				.Font(FT66Style::Tokens::FontBold(22))
				.ColorAndOpacity(FT66Style::Tokens::Danger)
				.Justification(ETextJustify::Center)
			]);

	const TSharedRef<SWidget> CabinetScreenContent =
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
					.Font(FT66Style::Tokens::FontBold(26))
					.ColorAndOpacity(FLinearColor(0.25f, 0.92f, 1.f, 1.f))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Arcade", "ArcadeSelectorCreditReadout", "CREDIT 01"))
					.Font(FT66Style::Tokens::FontBold(18))
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

	const TSharedRef<SWidget> CabinetScreen =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			CabinetScreenContent
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
				.Font(FT66Style::Tokens::FontBold(20))
				.ColorAndOpacity(FLinearColor(1.f, 0.74f, 0.26f, 1.f))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				FT66Style::MakeButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.Arcade", "ArcadeSelectorExit", "Exit"),
						FOnClicked::CreateUObject(this, &UT66ArcadeSelectionWidget::HandleExitClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(170.f)
					.SetHeight(48.f)
					.SetFontSize(18))
			]
		];

	const TSharedRef<SWidget> Cabinet =
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
			BuildCabinetArtworkLayer(1.f)
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
				.Font(FT66Style::Tokens::FontBold(44))
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
				CabinetScreen
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
			.WidthOverride(GArcadeSelectorCabinetWidth)
			.HeightOverride(GArcadeSelectorCabinetHeight)
			[
				Cabinet
			]
		];

	return FT66Style::MakeResponsiveRoot(Root);
}

void UT66ArcadeSelectionWidget::LoadCabinetArtworkBrush()
{
	if (CabinetArtworkTexture.IsValid())
	{
		return;
	}

	for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(TEXT("SourceAssets/Arcade/Selector/arcade_selector_front_cabinet.png")))
	{
		if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Trilinear, TEXT("ArcadeSelectorCabinet")))
		{
			CabinetArtworkTexture.Reset(Texture);
			CabinetArtworkBrush = FSlateBrush();
			CabinetArtworkBrush.SetResourceObject(Texture);
			CabinetArtworkBrush.DrawAs = ESlateBrushDrawType::Image;
			CabinetArtworkBrush.Tiling = ESlateBrushTileType::NoTile;
			CabinetArtworkBrush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
			return;
		}
	}
}

TSharedRef<SWidget> UT66ArcadeSelectionWidget::BuildCabinetArtworkLayer(const float Opacity) const
{
	if (!CabinetArtworkTexture.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SImage)
		.Image(&CabinetArtworkBrush)
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
	const FText DisplayName = GameData.DisplayName.IsEmpty()
		? ResolveGameCode(GameData.ArcadeGameType)
		: GameData.DisplayName;

	return SNew(SBox)
		.HeightOverride(104.f)
		[
			FT66Style::MakeBareButton(
				FT66BareButtonParams(
					FOnClicked::CreateUObject(this, &UT66ArcadeSelectionWidget::HandleGameClicked, GameData.ArcadeGameType),
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
										.Font(FT66Style::Tokens::FontBold(18))
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
										.Font(FT66Style::Tokens::FontBold(22))
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.f, 4.f, 0.f, 0.f)
									[
										SNew(STextBlock)
										.Text(ResolveGameFlavorText(GameData.ArcadeGameType))
										.Font(FT66Style::Tokens::FontRegular(15))
										.ColorAndOpacity(FT66Style::Tokens::TextMuted)
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
									.Font(FT66Style::Tokens::FontBold(16))
									.ColorAndOpacity(Accent)
								]
							]
						]
					])
				.SetColor(FLinearColor::Transparent)
				.SetPadding(FMargin(0.f)))
		];
}

FReply UT66ArcadeSelectionWidget::HandleGameClicked(const ET66ArcadeGameType GameType)
{
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
	switch (GameType)
	{
	case ET66ArcadeGameType::WhackAMole:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeWhack", "MOL");
	case ET66ArcadeGameType::Topwar:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeTopwar", "TOP");
	case ET66ArcadeGameType::GoldMiner:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeGoldMiner", "GLD");
	case ET66ArcadeGameType::RuneSwipe:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeRuneSwipe", "RUN");
	case ET66ArcadeGameType::CartSwitcher:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeCartSwitcher", "CRT");
	case ET66ArcadeGameType::CrystalDash:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeCrystalDash", "DSH");
	case ET66ArcadeGameType::PotionPour:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodePotionPour", "POT");
	case ET66ArcadeGameType::RelicStack:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeRelicStack", "REL");
	case ET66ArcadeGameType::ShieldParry:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeShieldParry", "SHD");
	case ET66ArcadeGameType::MimicMemory:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeMimicMemory", "MEM");
	case ET66ArcadeGameType::BombSorter:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeBombSorter", "BOM");
	case ET66ArcadeGameType::LanternLeap:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeLanternLeap", "LMP");
	case ET66ArcadeGameType::BladeSweep:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeBladeSweep", "BLD");
	default:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorCodeDefault", "???");
	}
}

FText UT66ArcadeSelectionWidget::ResolveGameFlavorText(const ET66ArcadeGameType GameType) const
{
	switch (GameType)
	{
	case ET66ArcadeGameType::WhackAMole:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorWhack", "Fast target bonks across a lit 3x3 board.");
	case ET66ArcadeGameType::Topwar:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorTopwar", "Choose power gates and grow the squad score.");
	case ET66ArcadeGameType::GoldMiner:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorGoldMiner", "Swing, hook, and reel treasure from the pit.");
	case ET66ArcadeGameType::RuneSwipe:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorRuneSwipe", "Tap the glowing rune chain before it fades.");
	case ET66ArcadeGameType::CartSwitcher:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorCartSwitcher", "Switch mine tracks into the active lane.");
	case ET66ArcadeGameType::CrystalDash:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorCrystalDash", "Dash through crystals while avoiding hazards.");
	case ET66ArcadeGameType::PotionPour:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorPotionPour", "Stop the pour on the glowing mark.");
	case ET66ArcadeGameType::RelicStack:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorRelicStack", "Drop moving relics over the center stack.");
	case ET66ArcadeGameType::ShieldParry:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorShieldParry", "Parry the lit projectile direction.");
	case ET66ArcadeGameType::MimicMemory:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorMimicMemory", "Repeat the chest sequence under pressure.");
	case ET66ArcadeGameType::BombSorter:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorBombSorter", "Sort the lit bomb into the matching chute.");
	case ET66ArcadeGameType::LanternLeap:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorLanternLeap", "Leap onto the glowing lantern platform.");
	case ET66ArcadeGameType::BladeSweep:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorBladeSweep", "Sweep cursed fruit and dodge bad tiles.");
	default:
		return NSLOCTEXT("T66.Arcade", "ArcadeSelectorFlavorDefault", "Boot the selected cabinet cartridge.");
	}
}

FLinearColor UT66ArcadeSelectionWidget::ResolveGameAccentColor(const ET66ArcadeGameType GameType, const int32 Index) const
{
	switch (GameType)
	{
	case ET66ArcadeGameType::WhackAMole:
		return FLinearColor(0.24f, 0.84f, 0.42f, 1.f);
	case ET66ArcadeGameType::Topwar:
		return FLinearColor(0.95f, 0.44f, 0.16f, 1.f);
	case ET66ArcadeGameType::GoldMiner:
		return FLinearColor(0.95f, 0.76f, 0.20f, 1.f);
	case ET66ArcadeGameType::ShieldParry:
		return FLinearColor(0.18f, 0.72f, 1.f, 1.f);
	case ET66ArcadeGameType::MimicMemory:
		return FLinearColor(0.78f, 0.32f, 0.96f, 1.f);
	case ET66ArcadeGameType::BombSorter:
		return FLinearColor(0.94f, 0.18f, 0.14f, 1.f);
	default:
	{
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
	}
}
