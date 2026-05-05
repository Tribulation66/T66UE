// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66CompanionGridScreen.h"
#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Screens/T66CompanionSelectionScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "Core/T66CompanionUnlockSubsystem.h"
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
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SOverlay.h"

namespace
{
	enum class ET66CompanionGridPlateFamily : uint8
	{
		CompactNeutral,
		TileNeutral,
		TileSelected
	};

	enum class ET66CompanionGridPlateState : uint8
	{
		Normal,
		Hovered,
		Pressed,
		Disabled
	};

	struct FCompanionGridPlateBrushSet
	{
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Normal;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Hovered;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Pressed;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Disabled;
	};

	const FSlateBrush* ResolveCompanionGridBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const FString& RelativePath,
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

	FCompanionGridPlateBrushSet& GetCompanionGridPlateBrushSet(const ET66CompanionGridPlateFamily Family)
	{
		static FCompanionGridPlateBrushSet CompactNeutral;
		static FCompanionGridPlateBrushSet TileNeutral;
		static FCompanionGridPlateBrushSet TileSelected;

		switch (Family)
		{
		case ET66CompanionGridPlateFamily::TileSelected:
			return TileSelected;
		case ET66CompanionGridPlateFamily::TileNeutral:
			return TileNeutral;
		case ET66CompanionGridPlateFamily::CompactNeutral:
		default:
			return CompactNeutral;
		}
	}

	FString GetCompanionGridPlatePath(const ET66CompanionGridPlateFamily Family, const ET66CompanionGridPlateState State)
	{
		if (State == ET66CompanionGridPlateState::Disabled)
		{
			return (Family == ET66CompanionGridPlateFamily::TileNeutral || Family == ET66CompanionGridPlateFamily::TileSelected)
				? TEXT("SourceAssets/UI/Reference/Modals/CompanionGrid/Panels/companiongrid_panels_inner_panel_normal.png")
				: TEXT("SourceAssets/UI/Reference/Modals/CompanionGrid/Buttons/companiongrid_buttons_pill_disabled.png");
		}

		if (Family == ET66CompanionGridPlateFamily::TileSelected)
		{
			return TEXT("SourceAssets/UI/Reference/Modals/CompanionGrid/Panels/companiongrid_panels_inner_panel_normal.png");
		}

		if (Family == ET66CompanionGridPlateFamily::TileNeutral)
		{
			if (State == ET66CompanionGridPlateState::Hovered)
			{
				return TEXT("SourceAssets/UI/Reference/Modals/CompanionGrid/Panels/companiongrid_panels_inner_panel_normal.png");
			}
			if (State == ET66CompanionGridPlateState::Pressed)
			{
				return TEXT("SourceAssets/UI/Reference/Modals/CompanionGrid/Panels/companiongrid_panels_inner_panel_normal.png");
			}
			return TEXT("SourceAssets/UI/Reference/Modals/CompanionGrid/Panels/companiongrid_panels_inner_panel_normal.png");
		}

		const TCHAR* Suffix = State == ET66CompanionGridPlateState::Pressed
			? TEXT("pressed")
			: (State == ET66CompanionGridPlateState::Hovered ? TEXT("hover") : TEXT("normal"));
		return T66ScreenSlateHelpers::MakeReferenceChromeButtonAssetPath(TEXT("Pill"), Suffix);
	}

	FMargin GetCompanionGridPlateMargin(const ET66CompanionGridPlateFamily Family, const ET66CompanionGridPlateState /*State*/)
	{
		if (Family == ET66CompanionGridPlateFamily::TileNeutral || Family == ET66CompanionGridPlateFamily::TileSelected)
		{
			return FMargin(0.067f, 0.043f, 0.067f, 0.043f);
		}
		return FMargin(0.f);
	}

	const FSlateBrush* ResolveCompanionGridPlateBrush(
		const ET66CompanionGridPlateFamily Family,
		const ET66CompanionGridPlateState State)
	{
		FCompanionGridPlateBrushSet& Set = GetCompanionGridPlateBrushSet(Family);
		T66RuntimeUIBrushAccess::FOptionalTextureBrush* Entry = &Set.Normal;
		if (State == ET66CompanionGridPlateState::Hovered)
		{
			Entry = &Set.Hovered;
		}
		else if (State == ET66CompanionGridPlateState::Pressed)
		{
			Entry = &Set.Pressed;
		}
		else if (State == ET66CompanionGridPlateState::Disabled)
		{
			Entry = &Set.Disabled;
		}

		return ResolveCompanionGridBrush(
			*Entry,
			GetCompanionGridPlatePath(Family, State),
			GetCompanionGridPlateMargin(Family, State),
			TEXT("CompanionGridPlate"),
			Family == ET66CompanionGridPlateFamily::CompactNeutral ? TextureFilter::TF_Nearest : TextureFilter::TF_Trilinear);
	}

	const FSlateBrush* GetCompanionGridModalShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionGridBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Modals/CompanionGrid/Panels/companiongrid_panels_inner_panel_normal.png"),
			FMargin(0.067f, 0.043f, 0.067f, 0.043f),
			TEXT("CompanionGridModalShell"));
	}

	const FSlateBrush* GetCompanionGridAvatarPlateBrush(const bool bSelected, const bool bLocked)
	{
		const TCHAR* RelativePath = bLocked
			? TEXT("Slots/Grid/portrait_slot_locked.png")
			: (bSelected ? TEXT("Slots/Grid/portrait_slot_selected.png") : TEXT("Slots/Grid/portrait_slot_normal.png"));
		const TCHAR* DebugLabel = bLocked
			? TEXT("CompanionGridAvatarPlateLockedV15")
			: (bSelected ? TEXT("CompanionGridAvatarPlateSelectedV15") : TEXT("CompanionGridAvatarPlateNormalV15"));
		return T66ScreenSlateHelpers::GetReferenceSharedBrush(RelativePath, FMargin(0.f), DebugLabel);
	}

	const FSlateBrush* GetCompanionGridSceneBackgroundBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionGridBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Modals/CompanionGrid/ScreenArt/companiongrid_screen_art_mainmenu_main_menu_scene_plate_v1.png"),
			FMargin(0.f),
			TEXT("CompanionGridSceneBackground"));
	}

	class ST66CompanionGridPlateButton : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66CompanionGridPlateButton)
			: _PlateFamily(ET66CompanionGridPlateFamily::CompactNeutral)
			, _MinWidth(0.f)
			, _Height(0.f)
			, _ContentPadding(FMargin(0.f))
			, _IsEnabled(true)
		{
		}
			SLATE_ATTRIBUTE(ET66CompanionGridPlateFamily, PlateFamily)
			SLATE_ARGUMENT(float, MinWidth)
			SLATE_ARGUMENT(float, Height)
			SLATE_ARGUMENT(FMargin, ContentPadding)
			SLATE_ARGUMENT(TAttribute<bool>, IsEnabled)
			SLATE_EVENT(FOnClicked, OnClicked)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			PlateFamily = InArgs._PlateFamily;
			ContentPadding = InArgs._ContentPadding;
			OwnedButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			OwnedButtonStyle.SetNormalPadding(FMargin(0.f));
			OwnedButtonStyle.SetPressedPadding(FMargin(0.f));

			ChildSlot
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						InArgs._OnClicked,
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(this, &ST66CompanionGridPlateButton::GetCurrentBrush)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
							.Padding(this, &ST66CompanionGridPlateButton::GetContentPadding)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								InArgs._Content.Widget
							]
						])
					.SetButtonStyle(&OwnedButtonStyle)
					.SetPadding(FMargin(0.f))
					.SetEnabled(InArgs._IsEnabled)
					.SetWidth(InArgs._MinWidth)
					.SetHeight(InArgs._Height),
					&Button)
			];
		}

	private:
		const FSlateBrush* GetCurrentBrush() const
		{
			const ET66CompanionGridPlateFamily Family = PlateFamily.Get(ET66CompanionGridPlateFamily::CompactNeutral);
			if (!Button.IsValid() || !Button->IsEnabled())
			{
				return ResolveCompanionGridPlateBrush(Family, ET66CompanionGridPlateState::Disabled);
			}
			if (Button->IsPressed())
			{
				return ResolveCompanionGridPlateBrush(Family, ET66CompanionGridPlateState::Pressed);
			}
			if (Button->IsHovered())
			{
				return ResolveCompanionGridPlateBrush(Family, ET66CompanionGridPlateState::Hovered);
			}
			return ResolveCompanionGridPlateBrush(Family, ET66CompanionGridPlateState::Normal);
		}

		FMargin GetContentPadding() const
		{
			if (Button.IsValid() && Button->IsPressed())
			{
				return FMargin(
					ContentPadding.Left,
					ContentPadding.Top + 1.f,
					ContentPadding.Right,
					FMath::Max(0.f, ContentPadding.Bottom - 1.f));
			}
			return ContentPadding;
		}

		TAttribute<ET66CompanionGridPlateFamily> PlateFamily;
		FMargin ContentPadding = FMargin(0.f);
		FButtonStyle OwnedButtonStyle;
		TSharedPtr<SButton> Button;
	};

	TSharedRef<SWidget> MakeCompanionGridPlateButton(
		const FT66ButtonParams& Params,
		const ET66CompanionGridPlateFamily Family,
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding)
	{
		if (!ResolveCompanionGridPlateBrush(Family, ET66CompanionGridPlateState::Normal))
		{
			FT66ButtonParams FallbackParams = Params;
			FallbackParams.SetContent(Content);
			return FT66Style::MakeButton(FallbackParams);
		}

		if (Family == ET66CompanionGridPlateFamily::CompactNeutral)
		{
			return T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton(
				Params.OnClicked,
				Content,
				ResolveCompanionGridPlateBrush(Family, ET66CompanionGridPlateState::Normal),
				ResolveCompanionGridPlateBrush(Family, ET66CompanionGridPlateState::Hovered),
				ResolveCompanionGridPlateBrush(Family, ET66CompanionGridPlateState::Pressed),
				ResolveCompanionGridPlateBrush(Family, ET66CompanionGridPlateState::Disabled),
				Params.MinWidth,
				Params.Height,
				Padding,
				Params.IsEnabled);
		}

		return SNew(ST66CompanionGridPlateButton)
			.PlateFamily(Family)
			.MinWidth(Params.MinWidth)
			.Height(Params.Height)
			.ContentPadding(Padding)
			.IsEnabled(Params.IsEnabled)
			.OnClicked(Params.OnClicked)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakeCompanionGridTextButton(const FT66ButtonParams& Params, const ET66CompanionGridPlateFamily Family)
	{
		const float ButtonHeight = Params.Height > 0.f ? Params.Height : 44.f;
		const FMargin Padding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(6.f, 2.f);
		const TSharedRef<SWidget> Content = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: T66ScreenSlateHelpers::MakeFilledButtonText(
				Params,
				ButtonHeight,
				TAttribute<FSlateColor>(FSlateColor(FT66Style::Tokens::Text)),
				TAttribute<FLinearColor>(FLinearColor(0.f, 0.f, 0.f, 0.68f)));

		return MakeCompanionGridPlateButton(Params, Family, Content, Padding);
	}

	TSharedRef<SWidget> MakeCompanionGridTile(
		const FT66ButtonParams& ButtonParams,
		const FLinearColor& /*BackgroundColor*/,
		const TSharedRef<SWidget>& Content,
		const T66ScreenSlateHelpers::FResponsiveGridModalMetrics& Metrics,
		const bool bSelected)
	{
		const float TileSize = Metrics.TileSize;
		const float InnerInset = FMath::Clamp(TileSize * 0.23f, 16.f, 34.f);
		const bool bEnabled = ButtonParams.IsEnabled.Get(true);

		TSharedRef<SOverlay> TileContent = SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(GetCompanionGridAvatarPlateBrush(bSelected, !bEnabled))
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

		FT66ButtonParams TileParams = ButtonParams;
		TileParams.SetMinWidth(TileSize).SetHeight(TileSize);
		return SNew(SBox)
			.WidthOverride(TileSize)
			.HeightOverride(TileSize)
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(TileParams.OnClicked, TileContent)
					.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
					.SetColor(FLinearColor::Transparent)
					.SetPadding(FMargin(0.f))
					.SetEnabled(TileParams.IsEnabled))
			];
	}

	TSharedRef<SWidget> MakeCompanionGridModalShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return T66ScreenSlateHelpers::MakeReferenceSharedBorder(
			TEXT("Panels/Modal/modal_shell_medium.png"),
			Content,
			FMargin(0.075f, 0.105f, 0.075f, 0.105f),
			Padding,
			TEXT("CompanionGridModalShellV14"),
			FT66Style::Tokens::Panel);
	}
}

UT66CompanionGridScreen::UT66CompanionGridScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::CompanionGrid;
	bIsModal = true;
}

UT66LocalizationSubsystem* UT66CompanionGridScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

void UT66CompanionGridScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	AllCompanionIDs.Empty();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		AllCompanionIDs = GI->GetAllCompanionIDs();
	}
}

TSharedRef<SWidget> UT66CompanionGridScreen::BuildSlateUI()
{
	if (AllCompanionIDs.Num() == 0)
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			AllCompanionIDs = GI->GetAllCompanionIDs();
		}
	}

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	FText TitleText = Loc ? Loc->GetText_CompanionGrid() : NSLOCTEXT("T66.CompanionGrid", "Title", "COMPANION GRID");
	FText NoCompanionText = Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.CompanionGrid", "NoCompanion", "NO COMPANION");
	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const float SharedTopInset = (UIManager && UIManager->IsFrontendTopBarVisible())
		? UIManager->GetFrontendTopBarContentHeight()
		: 0.f;

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	TArray<FName> IDsWithNone;
	IDsWithNone.Add(NAME_None);
	IDsWithNone.Append(AllCompanionIDs);

	constexpr int32 CompanionGridVisibleSlotCount = 25;
	T66ScreenSlateHelpers::FResponsiveGridModalMetrics GridMetrics =
		T66ScreenSlateHelpers::MakeResponsiveGridModalMetrics(CompanionGridVisibleSlotCount, SafeFrameSize);
	GridMetrics.ModalWidth = FMath::Min(GridMetrics.ModalWidth, GridMetrics.GridWidth + 260.0f);

	CompanionPortraitBrushes.Reset();
	CompanionPortraitBrushes.Reserve(FMath::Min(IDsWithNone.Num(), CompanionGridVisibleSlotCount));

	TSharedRef<SGridPanel> GridPanel = SNew(SGridPanel);
	for (int32 Index = 0; Index < CompanionGridVisibleSlotCount; Index++)
	{
		const int32 Row = Index / GridMetrics.Columns;
		const int32 Col = Index % GridMetrics.Columns;
		const bool bHasEntry = IDsWithNone.IsValidIndex(Index);
		FName CompanionID = bHasEntry ? IDsWithNone[Index] : NAME_None;
		FCompanionData Data;
		FLinearColor SpriteColor = FLinearColor(0.35f, 0.25f, 0.25f, 1.0f);
		bool bUnlocked = bHasEntry;
		if (bHasEntry && !CompanionID.IsNone() && GI)
		{
			if (GI->GetCompanionData(CompanionID, Data))
			{
				SpriteColor = Data.PlaceholderColor;
			}
			if (UT66CompanionUnlockSubsystem* Unlocks = GI->GetSubsystem<UT66CompanionUnlockSubsystem>())
			{
				bUnlocked = Unlocks->IsCompanionUnlocked(CompanionID);
			}
		}
		if (bHasEntry && !CompanionID.IsNone() && !bUnlocked)
		{
			SpriteColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
		}

		TSharedPtr<FSlateBrush> PortraitBrush;
		if (bHasEntry && !CompanionID.IsNone() && GI && GI->GetCompanionData(CompanionID, Data))
		{
			TSoftObjectPtr<UTexture2D> PortraitSoft = !Data.SelectionPortrait.IsNull() ? Data.SelectionPortrait : Data.Portrait;
			if (PortraitSoft.IsNull())
			{
				CompanionPortraitBrushes.Add(nullptr);
			}
			else
			{
				PortraitBrush = T66ScreenSlateHelpers::MakeSlateBrush(FVector2D(256.0f, 256.0f));
				CompanionPortraitBrushes.Add(PortraitBrush);
				if (TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, PortraitBrush, CompanionID, /*bClearWhileLoading*/ true);
				}
			}
		}
		else
		{
			CompanionPortraitBrushes.Add(nullptr);
		}

		FName CompanionIDCopy = CompanionID;
		TSharedPtr<FSlateBrush> PortraitBrushCopy = PortraitBrush;
		const bool bSelected = bHasEntry && GI && GI->SelectedCompanionID == CompanionIDCopy;
		const TSharedRef<SWidget> PortraitContent = PortraitBrushCopy.IsValid()
			? StaticCastSharedRef<SWidget>(SNew(SImage).Image(PortraitBrushCopy.Get()))
			: StaticCastSharedRef<SWidget>(SNew(SSpacer));
		GridPanel->AddSlot(Col, Row)
			.Padding(GridMetrics.TileGap * 0.5f)
			[
				MakeCompanionGridTile(
					FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateLambda([this, CompanionIDCopy]() { return HandleCompanionClicked(CompanionIDCopy); }))
						.SetEnabled(bHasEntry && (CompanionIDCopy.IsNone() || bUnlocked)),
					SpriteColor,
					PortraitContent,
					GridMetrics,
					bSelected)
			];
	}

	const TSharedRef<SWidget> ModalContent = MakeCompanionGridModalShell(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.0f, 0.0f, 0.0f, 18.0f)
		[
			SNew(STextBlock)
			.Text(TitleText)
			.Font(FT66Style::Tokens::FontBold(28))
			.ColorAndOpacity(FT66Style::Tokens::Text)
			.ShadowOffset(FVector2D(1.f, 1.f))
			.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.7f))
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
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
		FMargin(0.0f, SharedTopInset, 0.0f, 0.0f),
		GridMetrics.ModalWidth,
		GridMetrics.ModalHeight,
		true);
	if (const FSlateBrush* SceneBackgroundBrush = GetCompanionGridSceneBackgroundBrush())
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

FReply UT66CompanionGridScreen::HandleCompanionClicked(FName CompanionID)
{
	// Ignore locked companions (grid buttons should already be disabled, but keep it robust).
	if (!CompanionID.IsNone())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (UT66CompanionUnlockSubsystem* Unlocks = GI->GetSubsystem<UT66CompanionUnlockSubsystem>())
			{
				if (!Unlocks->IsCompanionUnlocked(CompanionID))
				{
					return FReply::Handled();
				}
			}
		}
	}

	if (UIManager)
	{
		UT66ScreenBase* Underlying = UIManager->GetCurrentScreen();
		if (UT66CompanionSelectionScreen* CompScreen = Cast<UT66CompanionSelectionScreen>(Underlying))
		{
			if (CompanionID.IsNone())
			{
				CompScreen->SelectNoCompanion();
			}
			else
			{
				CompScreen->PreviewCompanion(CompanionID);
			}
		}
		else if (UT66HeroSelectionScreen* HeroScreen = Cast<UT66HeroSelectionScreen>(Underlying))
		{
			if (CompanionID.IsNone())
			{
				HeroScreen->SelectNoCompanion();
			}
			else
			{
				HeroScreen->PreviewCompanion(CompanionID);
			}
		}
	}
	CloseModal();
	return FReply::Handled();
}

FReply UT66CompanionGridScreen::HandleCloseClicked()
{
	CloseModal();
	return FReply::Handled();
}

