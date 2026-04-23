// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66TDMainMenuScreen.h"

#include "Core/T66TDDataSubsystem.h"
#include "Core/T66TDFrontendStateSubsystem.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "UI/T66UITypes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SInvalidationPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FVector2D TDBgSize(1920.f, 1080.f);

	void EnsureBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
	}

	void SetupLooseBrush(
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& TextureHandle,
		const TCHAR* RelativePath,
		const FVector2D& ImageSize,
		const bool bGenerateMips,
		const TCHAR* DebugLabel)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
		}
		EnsureBrush(Brush, ImageSize);

		if (!TextureHandle.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (!FPaths::FileExists(CandidatePath))
				{
					continue;
				}

				UTexture2D* Texture = bGenerateMips
					? T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Trilinear, DebugLabel)
					: T66RuntimeUITextureAccess::ImportFileTexture(CandidatePath, TextureFilter::TF_Trilinear, false, DebugLabel);
				if (Texture)
				{
					TextureHandle.Reset(Texture);
					break;
				}
			}
		}

		Brush->SetResourceObject(TextureHandle.Get());
	}

	TSharedRef<SWidget> MakeOptionalImageLayer(const TSharedPtr<FSlateBrush>& Brush, const float Opacity)
	{
		if (!Brush.IsValid() || !Brush->GetResourceObject())
		{
			return SNew(SBox);
		}

		return SNew(SImage)
			.Image(Brush.Get())
			.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity));
	}
}

UT66TDMainMenuScreen::UT66TDMainMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::TDMainMenu;
	bIsModal = false;
}

void UT66TDMainMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
}

void UT66TDMainMenuScreen::OnScreenDeactivated_Implementation()
{
	ReleaseRetainedSlateState();
	Super::OnScreenDeactivated_Implementation();
}

void UT66TDMainMenuScreen::NativeDestruct()
{
	ReleaseRetainedSlateState();
	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66TDMainMenuScreen::BuildSlateUI()
{
	RequestMenuTextures();

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66TDDataSubsystem* TDDataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66TDDataSubsystem>() : nullptr;
	const int32 HeroCount = TDDataSubsystem ? TDDataSubsystem->GetHeroes().Num() : 0;
	const int32 DifficultyCount = TDDataSubsystem ? TDDataSubsystem->GetDifficulties().Num() : 0;
	const int32 MapCount = TDDataSubsystem ? TDDataSubsystem->GetMaps().Num() : 0;

	const FLinearColor BrightText(0.95f, 0.93f, 0.92f, 1.0f);
	const FLinearColor MutedText(0.78f, 0.74f, 0.72f, 1.0f);
	const FLinearColor ShellFill(0.020f, 0.010f, 0.014f, 0.90f);
	const FLinearColor PanelFill(0.090f, 0.050f, 0.058f, 0.92f);
	const FLinearColor CardFill(0.120f, 0.060f, 0.070f, 0.95f);

	auto MakeSectionTitle = [&](const FText& Text) -> TSharedRef<SWidget>
	{
		FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Bold"), 13);
		Font.LetterSpacing = 112;
		return SNew(STextBlock).Text(Text).Font(Font).ColorAndOpacity(FLinearColor(0.92f, 0.74f, 0.42f, 1.0f));
	};

	auto MakeInfoCard = [&](const FText& Title, const FText& Body) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(CardFill)
			.Padding(FMargin(12.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(Title)
					.Font(FT66Style::MakeFont(TEXT("Bold"), 12))
					.ColorAndOpacity(BrightText)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Body)
					.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
					.ColorAndOpacity(MutedText)
					.AutoWrapText(true)
				]
			];
	};

	auto MakeMenuButton = [&](const FText& Text, FReply (UT66TDMainMenuScreen::*ClickFunc)()) -> TSharedRef<SWidget>
	{
		FT66ButtonParams Params(Text, FOnClicked::CreateUObject(this, ClickFunc), ET66ButtonType::Success);
		Params
			.SetMinWidth(520.f)
			.SetHeight(82.f)
			.SetFontSize(20)
			.SetPadding(FMargin(16.f, 10.f, 16.f, 8.f))
			.SetUseGlow(false)
			.SetUseDotaPlateOverlay(true)
			.SetDotaPlateOverrideBrush(PrimaryCTAFillBrush.Get())
			.SetTextColor(FLinearColor(0.97f, 0.96f, 0.94f, 1.0f))
			.SetStateTextShadowColors(
				FLinearColor(0.f, 0.f, 0.f, 0.36f),
				FLinearColor(0.f, 0.f, 0.f, 0.42f),
				FLinearColor(0.f, 0.f, 0.f, 0.28f))
			.SetTextShadowOffset(FVector2D(0.f, 1.f));
		return SNew(SBox).WidthOverride(520.f)[FT66Style::MakeButton(Params)];
	};

	const TSharedRef<SWidget> LeftShell =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(ShellFill)
		.Padding(FMargin(14.f))
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[MakeSectionTitle(NSLOCTEXT("T66TD.MainMenu", "ModeTitle", "REGULAR MODE"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
				[
					MakeInfoCard(
						NSLOCTEXT("T66TD.MainMenu", "FormatTitle", "Format"),
						NSLOCTEXT("T66TD.MainMenu", "FormatBody", "Chadpocalypse TD starts with one ruleset only: Regular. Pick one of five difficulties, then pick one of four maps inside that tier."))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
				[
					MakeInfoCard(
						NSLOCTEXT("T66TD.MainMenu", "RosterTitle", "Roster"),
						FText::Format(
							NSLOCTEXT("T66TD.MainMenu", "RosterBody", "{0} hero towers mirror the Chadpocalypse cast. Idol families remain the augmentation layer instead of importing another weapon taxonomy."),
							FText::AsNumber(HeroCount)))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
				[
					MakeInfoCard(
						NSLOCTEXT("T66TD.MainMenu", "ScopeTitle", "Scope"),
						FText::Format(
							NSLOCTEXT("T66TD.MainMenu", "ScopeBody", "{0} difficulties x 4 maps each = {1} launch maps. Every tier maps to the same tower themes already used by the main game."),
							FText::AsNumber(DifficultyCount),
							FText::AsNumber(MapCount)))
				]
			]
		];

	const TSharedRef<SWidget> CenterShell =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 0.82f))
		.Padding(FMargin(18.f, 16.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[MakeMenuButton(NSLOCTEXT("T66TD.MainMenu", "RegularCTA", "REGULAR"), &UT66TDMainMenuScreen::HandleNewGameClicked)]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 16.f, 0.f, 0.f)[MakeMenuButton(NSLOCTEXT("T66TD.MainMenu", "MapsCTA", "DIFFICULTIES & MAPS"), &UT66TDMainMenuScreen::HandleMapBrowserClicked)]
		];

	const TSharedRef<SWidget> RightShell =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(ShellFill)
		.Padding(FMargin(14.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[MakeSectionTitle(NSLOCTEXT("T66TD.MainMenu", "TierPreviewTitle", "TIER PREVIEW"))]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				MakeInfoCard(NSLOCTEXT("T66TD.MainMenu", "EasyTitle", "Easy -> Dungeon"), NSLOCTEXT("T66TD.MainMenu", "EasyBody", "Stone corridors, tight early routing, and readable first-contact pressure."))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				MakeInfoCard(NSLOCTEXT("T66TD.MainMenu", "MediumTitle", "Medium -> Forest"), NSLOCTEXT("T66TD.MainMenu", "MediumBody", "Overgrowth, visibility breaks, and more side-lane tempo checks."))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				MakeInfoCard(NSLOCTEXT("T66TD.MainMenu", "HardTitle", "Hard -> Ocean"), NSLOCTEXT("T66TD.MainMenu", "HardBody", "Split routes, tide pacing, and fast leak punishment if the front line folds."))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				MakeInfoCard(NSLOCTEXT("T66TD.MainMenu", "EndTitle", "Very Hard / Impossible"), NSLOCTEXT("T66TD.MainMenu", "EndBody", "Martian and Hell tiers push multi-lane pressure, shielded waves, and boss checks without needing a separate campaign mode."))
			]
		];

	FSlateFontInfo TitleFont = FT66Style::MakeFont(TEXT("Black"), 38);
	TitleFont.LetterSpacing = 46;

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()[MakeOptionalImageLayer(BackdropBrush, 0.98f)]
			+ SOverlay::Slot()[MakeOptionalImageLayer(ForegroundBrush, 0.96f)]
			+ SOverlay::Slot()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.42f))]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 42.f, 0.f, 0.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66TD.MainMenu", "Title", "CHADPOCALYPSE TD"))
					.Font(TitleFont)
					.ColorAndOpacity(FLinearColor(0.98f, 0.74f, 0.26f, 0.30f))
					.RenderTransform(FSlateRenderTransform(FVector2D(2.f, 2.f)))
					.ShadowOffset(FVector2D(2.f, 2.f))
					.ShadowColorAndOpacity(FLinearColor(1.0f, 0.72f, 0.18f, 0.42f))
				]
				+ SOverlay::Slot()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66TD.MainMenu", "Title", "CHADPOCALYPSE TD"))
					.Font(TitleFont)
					.ColorAndOpacity(FLinearColor(0.03f, 0.02f, 0.02f, 0.98f))
					.ShadowOffset(FVector2D(2.f, 2.f))
					.ShadowColorAndOpacity(FLinearColor(1.0f, 0.72f, 0.18f, 0.42f))
				]
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(FMargin(20.f, 20.f, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(304.f)
				.HeightOverride(40.f)
				[
						FT66Style::MakeButton(
							FT66ButtonParams(
								NSLOCTEXT("T66TD.MainMenu", "BackToMainMenu", "BACK TO MENU"),
								FOnClicked::CreateUObject(this, &UT66TDMainMenuScreen::HandleBackToMainMenuClicked),
								ET66ButtonType::Neutral)
						.SetMinWidth(304.f)
						.SetHeight(40.f)
						.SetFontSize(12)
						.SetPadding(FMargin(14.f, 8.f, 14.f, 6.f))
						.SetUseGlow(false)
						.SetTextColor(FLinearColor(0.94f, 0.95f, 0.97f, 1.0f)))
				]
			]
			+ SOverlay::Slot()
			[
				SNew(SOverlay)
				+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(FMargin(20.f, 0.f, 0.f, 16.f))
				[
					SNew(SBox).WidthOverride(382.f).HeightOverride(560.f).Clipping(EWidgetClipping::ClipToBounds)
					[
						SNew(SInvalidationPanel)[LeftShell]
					]
				]
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 126.f)
				[
					SNew(SBox).WidthOverride(560.f).HeightOverride(214.f)
					[
						SNew(SInvalidationPanel)[CenterShell]
					]
				]
				+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(FMargin(0.f, 0.f, 20.f, 16.f))
				[
					SNew(SBox).WidthOverride(382.f).HeightOverride(560.f).Clipping(EWidgetClipping::ClipToBounds)
					[
						SNew(SInvalidationPanel)[RightShell]
					]
				]
			]
		];
}

FReply UT66TDMainMenuScreen::HandleBackToMainMenuClicked()
{
	NavigateTo(ET66ScreenType::MainMenu);
	return FReply::Handled();
}

FReply UT66TDMainMenuScreen::HandleNewGameClicked()
{
	if (UT66TDFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->BeginNewRun();
	}

	NavigateTo(ET66ScreenType::TDDifficultySelect);
	return FReply::Handled();
}

FReply UT66TDMainMenuScreen::HandleMapBrowserClicked()
{
	return HandleNewGameClicked();
}

void UT66TDMainMenuScreen::RequestMenuTextures()
{
	SetupLooseBrush(BackdropBrush, BackdropTexture, TEXT("SourceAssets/TD/Maps/Backgrounds/TD_Menu_Backdrop_01.png"), TDBgSize, true, TEXT("TDMenuBackdrop"));
	SetupLooseBrush(ForegroundBrush, ForegroundTexture, TEXT("SourceAssets/TD/Maps/Backgrounds/TD_Menu_Foreground_01.png"), TDBgSize, true, TEXT("TDMenuForeground"));
	SetupLooseBrush(PrimaryCTAFillBrush, PrimaryCTAFillTexture, TEXT("RuntimeDependencies/T66/UI/MiniMainMenu/mini_mainmenu_cta_fill_green.png"), FVector2D(1024.f, 232.f), false, TEXT("TDMenuCTA"));
}

void UT66TDMainMenuScreen::ReleaseRetainedSlateState()
{
	BackdropBrush.Reset();
	BackdropTexture.Reset();
	ForegroundBrush.Reset();
	ForegroundTexture.Reset();
	PrimaryCTAFillBrush.Reset();
	PrimaryCTAFillTexture.Reset();
}
