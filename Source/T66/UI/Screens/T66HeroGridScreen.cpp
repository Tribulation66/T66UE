// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66HeroGridScreen.h"
#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"

namespace
{
	const FSlateBrush* ResolveHeroGridBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* RelativePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel);
	}

	const FSlateBrush* GetHeroGridModalShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveHeroGridBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/basic_panel_normal.png"),
			FMargin(0.067f, 0.043f, 0.067f, 0.043f),
			TEXT("HeroGridModalShell"));
	}

	const FSlateBrush* GetHeroGridTileBrush(const bool bSelected)
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Neutral;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Selected;
		return bSelected
			? ResolveHeroGridBrush(
				Selected,
				TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/basic_panel_normal.png"),
				FMargin(0.067f, 0.043f, 0.067f, 0.043f),
				TEXT("HeroGridTileSelected"))
			: ResolveHeroGridBrush(
				Neutral,
				TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/basic_panel_normal.png"),
				FMargin(0.067f, 0.043f, 0.067f, 0.043f),
				TEXT("HeroGridTileNeutral"));
	}

	const FSlateBrush* GetHeroGridCompactButtonBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveHeroGridBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/basic_button_normal.png"),
			FMargin(0.093f, 0.213f, 0.093f, 0.213f),
			TEXT("HeroGridCompactButton"));
	}

	const FSlateBrush* GetHeroGridAvatarFrameBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveHeroGridBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Slots/basic_slot_normal.png"),
			FMargin(0.20f, 0.20f, 0.20f, 0.20f),
			TEXT("HeroGridAvatarFrame"));
	}

	const FSlateBrush* GetHeroGridSceneBackgroundBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveHeroGridBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/ScreenArt/MainMenu/main_menu_scene_plate_imagegen_20260425_v1.png"),
			FMargin(0.f),
			TEXT("HeroGridSceneBackground"));
	}

	TSharedRef<SWidget> MakeHeroGridModalShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		if (const FSlateBrush* ShellBrush = GetHeroGridModalShellBrush())
		{
			return SNew(SBorder)
				.BorderImage(ShellBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FT66Style::Tokens::Panel)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeHeroGridTextButton(const FT66ButtonParams& Params)
	{
		FT66ButtonParams ButtonParams = Params;
		if (!ButtonParams.CustomContent.IsValid())
		{
			const float ButtonHeight = ButtonParams.Height > 0.f ? ButtonParams.Height : 44.f;
			ButtonParams
				.SetPadding(FMargin(6.f, 2.f))
				.SetContent(T66ScreenSlateHelpers::MakeFilledButtonText(
					ButtonParams,
					ButtonHeight,
					TAttribute<FSlateColor>(FSlateColor(FLinearColor(0.97f, 0.94f, 0.84f, 1.f))),
					TAttribute<FLinearColor>(FLinearColor(0.f, 0.f, 0.f, 0.68f))));
		}

		ButtonParams
			.SetUseGlow(false)
			.SetUseDotaPlateOverlay(true)
			.SetDotaPlateOverrideBrush(GetHeroGridCompactButtonBrush())
			.SetTextColor(FLinearColor(0.97f, 0.94f, 0.84f, 1.f))
			.SetTextShadowOffset(FVector2D(1.f, 1.f))
			.SetStateTextShadowColors(
				FLinearColor(0.f, 0.f, 0.f, 0.68f),
				FLinearColor(0.f, 0.f, 0.f, 0.78f),
				FLinearColor(0.f, 0.f, 0.f, 0.85f));
		return FT66Style::MakeButton(ButtonParams);
	}

	TSharedRef<SWidget> MakeHeroGridTile(
		const FT66ButtonParams& ButtonParams,
		const FLinearColor& /*BackgroundColor*/,
		const TSharedRef<SWidget>& Content,
		const T66ScreenSlateHelpers::FResponsiveGridModalMetrics& Metrics,
		const bool bSelected)
	{
		const float TileSize = Metrics.TileSize;
		const float InnerInset = FMath::Clamp(TileSize * 0.08f, 6.f, 16.f);
		const bool bEnabled = ButtonParams.IsEnabled.Get(true);
		const FLinearColor InteriorColor = bSelected
			? FLinearColor(0.050f, 0.055f, 0.070f, bEnabled ? 0.94f : 0.38f)
			: FLinearColor(0.012f, 0.018f, 0.026f, bEnabled ? 0.92f : 0.34f);

		TSharedRef<SOverlay> TileContent = SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(InteriorColor)
			]
			+ SOverlay::Slot()
			.Padding(InnerInset + 5.f)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					Content
				]
			];

		if (const FSlateBrush* AvatarFrame = GetHeroGridAvatarFrameBrush())
		{
			TileContent->AddSlot()
			[
				SNew(SImage)
				.Image(AvatarFrame)
				.ColorAndOpacity(bSelected
					? FLinearColor(1.15f, 1.06f, 0.78f, 1.0f)
					: FLinearColor(0.82f, 0.86f, 0.78f, 0.92f))
			];
		}

		return SNew(SBox)
			.WidthOverride(TileSize)
			.HeightOverride(TileSize)
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(ButtonParams.OnClicked, TileContent)
					.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
					.SetColor(FLinearColor::Transparent)
					.SetPadding(FMargin(0.f))
					.SetEnabled(ButtonParams.IsEnabled))
			];
	}
}

UT66HeroGridScreen::UT66HeroGridScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::HeroGrid;
	bIsModal = true;
}

UT66LocalizationSubsystem* UT66HeroGridScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

void UT66HeroGridScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	AllHeroIDs.Empty();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		AllHeroIDs = GI->GetAllHeroIDs();
	}
}

TSharedRef<SWidget> UT66HeroGridScreen::BuildSlateUI()
{
	// Ensure hero list is populated (BuildSlateUI may run before OnScreenActivated)
	if (AllHeroIDs.Num() == 0)
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			AllHeroIDs = GI->GetAllHeroIDs();
		}
	}

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66UITexturePoolSubsystem* TexPool = nullptr;
	if (UGameInstance* GI0 = UGameplayStatics::GetGameInstance(this))
	{
		TexPool = GI0->GetSubsystem<UT66UITexturePoolSubsystem>();
	}

	FText TitleText = Loc ? Loc->GetText_HeroGrid() : NSLOCTEXT("T66.HeroGrid", "Title", "HERO GRID");
	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const float SharedTopInset = (UIManager && UIManager->IsFrontendTopBarVisible())
		? UIManager->GetFrontendTopBarContentHeight()
		: 0.f;
	const T66ScreenSlateHelpers::FResponsiveGridModalMetrics GridMetrics =
		T66ScreenSlateHelpers::MakeResponsiveGridModalMetrics(AllHeroIDs.Num(), SafeFrameSize);

	HeroPortraitBrushes.Reset();
	HeroPortraitBrushes.Reserve(AllHeroIDs.Num());

	TSharedRef<SGridPanel> GridPanel = SNew(SGridPanel);
	for (int32 Index = 0; Index < AllHeroIDs.Num(); Index++)
	{
		const int32 Row = Index / GridMetrics.Columns;
		const int32 Col = Index % GridMetrics.Columns;
		FName HeroID = AllHeroIDs[Index];
		FHeroData HeroData;
		FLinearColor SpriteColor = FLinearColor(0.25f, 0.25f, 0.3f, 1.0f);
		const bool bSelected = GI && GI->SelectedHeroID == HeroID;
		TSharedPtr<FSlateBrush> PortraitBrush;
		if (GI && GI->GetHeroData(HeroID, HeroData))
		{
			SpriteColor = HeroData.PlaceholderColor;
			const TSoftObjectPtr<UTexture2D> PortraitSoft = GI->ResolveHeroPortrait(HeroData, GI->SelectedHeroBodyType, ET66HeroPortraitVariant::Half);
			if (!PortraitSoft.IsNull())
			{
				PortraitBrush = T66ScreenSlateHelpers::MakeSlateBrush(FVector2D(256.0f, 256.0f));
				HeroPortraitBrushes.Add(PortraitBrush);
				if (TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, PortraitBrush, HeroID, /*bClearWhileLoading*/ true);
				}
			}
		}
		FName HeroIDCopy = HeroID;
		const TSharedRef<SWidget> PortraitContent = PortraitBrush.IsValid()
			? StaticCastSharedRef<SWidget>(SNew(SImage).Image(PortraitBrush.Get()))
			: StaticCastSharedRef<SWidget>(SNew(SSpacer));
		GridPanel->AddSlot(Col, Row)
			.Padding(GridMetrics.TileGap * 0.5f)
			[
				MakeHeroGridTile(
					FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateLambda([this, HeroIDCopy]() { return HandleHeroClicked(HeroIDCopy); })),
					SpriteColor,
					PortraitContent,
					GridMetrics,
					bSelected)
			];
	}

	T66ScreenSlateHelpers::AddUniformGridPaddingSlots(*GridPanel, AllHeroIDs.Num(), GridMetrics);

	const TSharedRef<SWidget> ModalContent = MakeHeroGridModalShell(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.f, 0.f, 0.f, 18.f)
		[
			SNew(STextBlock)
			.Text(TitleText)
			.Font(FT66Style::Tokens::FontBold(28))
			.ColorAndOpacity(FT66Style::Tokens::Text)
			.ShadowOffset(FVector2D(1.f, 1.f))
			.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.7f))
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(GridMetrics.GridWidth)
			.HeightOverride(GridMetrics.GridHeight)
			[
				GridPanel
			]
		]
		,
		FMargin(36.f, 28.f, 36.f, 30.f));

	const TSharedRef<SWidget> Modal = T66ScreenSlateHelpers::MakeCenteredScrimModal(
		ModalContent,
		FMargin(0.f, SharedTopInset, 0.f, 0.f),
		GridMetrics.ModalWidth,
		GridMetrics.ModalHeight,
		true);
	if (const FSlateBrush* SceneBackgroundBrush = GetHeroGridSceneBackgroundBrush())
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Image(SceneBackgroundBrush)
				.ColorAndOpacity(FLinearColor(0.88f, 0.88f, 0.88f, 1.0f))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.035f, 0.48f))
			]
			+ SOverlay::Slot()
			[
				Modal
			];
	}
	return Modal;
}

FReply UT66HeroGridScreen::HandleHeroClicked(FName HeroID)
{
	// Tell the underlying Hero Selection screen to preview this hero
	if (UIManager)
	{
		UT66ScreenBase* Underlying = UIManager->GetCurrentScreen();
		if (UT66HeroSelectionScreen* HeroScreen = Cast<UT66HeroSelectionScreen>(Underlying))
		{
			HeroScreen->PreviewHero(HeroID);
		}
	}
	CloseModal();
	return FReply::Handled();
}

FReply UT66HeroGridScreen::HandleCloseClicked()
{
	CloseModal();
	return FReply::Handled();
}

