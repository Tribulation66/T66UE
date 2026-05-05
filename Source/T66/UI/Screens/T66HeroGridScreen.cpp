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
		const TCHAR* DebugLabel,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel,
			Filter);
	}

	const FSlateBrush* GetHeroGridModalShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveHeroGridBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Modals/HeroGrid/Panels/herogrid_panels_inner_panel_normal.png"),
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
				TEXT("SourceAssets/UI/Reference/Modals/HeroGrid/Panels/herogrid_panels_inner_panel_normal.png"),
				FMargin(0.067f, 0.043f, 0.067f, 0.043f),
				TEXT("HeroGridTileSelected"))
			: ResolveHeroGridBrush(
				Neutral,
				TEXT("SourceAssets/UI/Reference/Modals/HeroGrid/Panels/herogrid_panels_inner_panel_normal.png"),
				FMargin(0.067f, 0.043f, 0.067f, 0.043f),
				TEXT("HeroGridTileNeutral"));
	}

	enum class ET66HeroGridButtonState : uint8
	{
		Normal,
		Hovered,
		Pressed,
		Disabled
	};

	const FSlateBrush* GetHeroGridCompactButtonBrush(const ET66HeroGridButtonState State)
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Normal;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Hovered;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Pressed;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Disabled;

		T66RuntimeUIBrushAccess::FOptionalTextureBrush* Entry = &Normal;
		const TCHAR* StateName = TEXT("normal");
		const TCHAR* DebugLabel = TEXT("HeroGridCompactButtonNormal");
		switch (State)
		{
		case ET66HeroGridButtonState::Hovered:
			Entry = &Hovered;
			StateName = TEXT("hover");
			DebugLabel = TEXT("HeroGridCompactButtonHover");
			break;
		case ET66HeroGridButtonState::Pressed:
			Entry = &Pressed;
			StateName = TEXT("pressed");
			DebugLabel = TEXT("HeroGridCompactButtonPressed");
			break;
		case ET66HeroGridButtonState::Disabled:
			Entry = &Disabled;
			StateName = TEXT("disabled");
			DebugLabel = TEXT("HeroGridCompactButtonDisabled");
			break;
		case ET66HeroGridButtonState::Normal:
		default:
			break;
		}

		return ResolveHeroGridBrush(
			*Entry,
			*T66ScreenSlateHelpers::MakeReferenceChromeButtonAssetPath(TEXT("Pill"), StateName),
			FMargin(0.f),
			DebugLabel,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetHeroGridAvatarPlateBrush(const bool bSelected)
	{
		return T66ScreenSlateHelpers::GetReferenceSharedBrush(
			bSelected ? TEXT("Slots/Grid/portrait_slot_selected.png") : TEXT("Slots/Grid/portrait_slot_normal.png"),
			FMargin(0.f),
			bSelected ? TEXT("HeroGridAvatarPlateSelectedV15") : TEXT("HeroGridAvatarPlateNormalV15"));
	}

	const FSlateBrush* GetHeroGridSceneBackgroundBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveHeroGridBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Modals/HeroGrid/ScreenArt/herogrid_screen_art_mainmenu_main_menu_scene_plate_v1.png"),
			FMargin(0.f),
			TEXT("HeroGridSceneBackground"));
	}

	TSharedRef<SWidget> MakeHeroGridModalShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return T66ScreenSlateHelpers::MakeReferenceSharedBorder(
			TEXT("Panels/Modal/modal_shell_medium.png"),
			Content,
			FMargin(0.075f, 0.105f, 0.075f, 0.105f),
			Padding,
			TEXT("HeroGridModalShellV14"),
			FT66Style::Tokens::Panel);
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

		return T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton(
			ButtonParams.OnClicked,
			ButtonParams.CustomContent.ToSharedRef(),
			GetHeroGridCompactButtonBrush(ET66HeroGridButtonState::Normal),
			GetHeroGridCompactButtonBrush(ET66HeroGridButtonState::Hovered),
			GetHeroGridCompactButtonBrush(ET66HeroGridButtonState::Pressed),
			GetHeroGridCompactButtonBrush(ET66HeroGridButtonState::Disabled),
			ButtonParams.MinWidth,
			ButtonParams.Height > 0.f ? ButtonParams.Height : 44.f,
			ButtonParams.Padding.Left >= 0.f ? ButtonParams.Padding : FMargin(6.f, 2.f),
			ButtonParams.IsEnabled,
			ButtonParams.Visibility);
	}

	TSharedRef<SWidget> MakeHeroGridTile(
		const FT66ButtonParams& ButtonParams,
		const FLinearColor& /*BackgroundColor*/,
		const TSharedRef<SWidget>& Content,
		const T66ScreenSlateHelpers::FResponsiveGridModalMetrics& Metrics,
		const bool bSelected)
	{
		const float TileSize = Metrics.TileSize;
		const float InnerInset = FMath::Clamp(TileSize * 0.23f, 18.f, 38.f);

		TSharedRef<SOverlay> TileContent = SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(GetHeroGridAvatarPlateBrush(bSelected))
				.ColorAndOpacity(FLinearColor::White)
			]
			+ SOverlay::Slot()
			.Padding(InnerInset)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					Content
				]
			];

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
	constexpr int32 HeroGridVisibleSlotCount = 16;
	T66ScreenSlateHelpers::FResponsiveGridModalMetrics GridMetrics =
		T66ScreenSlateHelpers::MakeResponsiveGridModalMetrics(HeroGridVisibleSlotCount, SafeFrameSize);
	GridMetrics.ModalWidth = FMath::Min(GridMetrics.ModalWidth, GridMetrics.GridWidth + 250.0f);

	HeroPortraitBrushes.Reset();
	HeroPortraitBrushes.Reserve(FMath::Min(AllHeroIDs.Num(), HeroGridVisibleSlotCount));

	TSharedRef<SGridPanel> GridPanel = SNew(SGridPanel);
	for (int32 Index = 0; Index < HeroGridVisibleSlotCount; Index++)
	{
		const int32 Row = Index / GridMetrics.Columns;
		const int32 Col = Index % GridMetrics.Columns;
		const bool bHasHero = AllHeroIDs.IsValidIndex(Index);
		FName HeroID = bHasHero ? AllHeroIDs[Index] : NAME_None;
		FHeroData HeroData;
		FLinearColor SpriteColor = FLinearColor(0.25f, 0.25f, 0.3f, 1.0f);
		const bool bSelected = bHasHero && GI && GI->SelectedHeroID == HeroID;
		TSharedPtr<FSlateBrush> PortraitBrush;
		if (bHasHero && GI && GI->GetHeroData(HeroID, HeroData))
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
		const FOnClicked TileClicked = bHasHero
			? FOnClicked::CreateLambda([this, HeroIDCopy]() { return HandleHeroClicked(HeroIDCopy); })
			: FOnClicked::CreateLambda([]() { return FReply::Handled(); });
		GridPanel->AddSlot(Col, Row)
			.Padding(GridMetrics.TileGap * 0.5f)
			[
				MakeHeroGridTile(
					FT66ButtonParams(FText::GetEmpty(), TileClicked)
						.SetEnabled(bHasHero),
					SpriteColor,
					PortraitContent,
					GridMetrics,
					bSelected)
			];
	}

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

