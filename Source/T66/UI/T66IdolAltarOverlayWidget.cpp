// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66IdolAltarOverlayWidget.h"

#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/Dota/T66DotaSlate.h"
#include "UI/Dota/T66DotaTheme.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	static constexpr int32 T66IdolAltarView_Offers = 0;
	static constexpr int32 T66IdolAltarView_Trade = 1;

	UT66IdolManagerSubsystem* GetIdolManager(UWorld* World)
	{
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
	}

	const FSlateBrush* GetWhiteBrush()
	{
		return FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	}

	FLinearColor DarkenColor(const FLinearColor& Color, float Amount)
	{
		return FLinearColor(
			FMath::Clamp(Color.R * Amount, 0.f, 1.f),
			FMath::Clamp(Color.G * Amount, 0.f, 1.f),
			FMath::Clamp(Color.B * Amount, 0.f, 1.f),
			Color.A);
	}

	int32 FindEquippedSlotByIdolID(const UT66IdolManagerSubsystem* IdolManager, const FName IdolID)
	{
		if (!IdolManager || IdolID.IsNone())
		{
			return INDEX_NONE;
		}

		const TArray<FName>& Equipped = IdolManager->GetEquippedIdols();
		for (int32 SlotIndex = 0; SlotIndex < Equipped.Num(); ++SlotIndex)
		{
			if (Equipped[SlotIndex] == IdolID)
			{
				return SlotIndex;
			}
		}

		return INDEX_NONE;
	}

	FText GetOfferSectionTitle(int32 CategoryIndex)
	{
		switch (CategoryIndex)
		{
		case 0: return NSLOCTEXT("T66.IdolAltar", "PierceSection", "PIERCE");
		case 1: return NSLOCTEXT("T66.IdolAltar", "AOESection", "AOE");
		case 2: return NSLOCTEXT("T66.IdolAltar", "BounceSection", "BOUNCE");
		case 3: return NSLOCTEXT("T66.IdolAltar", "DotSection", "DOT");
		default: return FText::GetEmpty();
		}
	}

	TSharedPtr<IToolTip> MakeIdolTooltip(const FText& Title, const FText& Description)
	{
		if (Title.IsEmpty() && Description.IsEmpty())
		{
			return nullptr;
		}

		return SNew(SToolTip)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
				.BorderBackgroundColor(FT66Style::Tokens::Bg)
				.Padding(FMargin(10.f, 8.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[
						SNew(STextBlock)
						.Text(Title)
						.Font(FT66Style::Tokens::FontBold(14))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(Description)
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
						.WrapTextAt(280.f)
					]
				]
			];
	}

	TSharedRef<SWidget> MakeAltarButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		TSharedPtr<SButton>& OutButton,
		TSharedPtr<SBorder>& OutBackground,
		TSharedPtr<STextBlock>& OutText,
		float MinWidth = 150.f)
	{
		return SAssignNew(OutButton, SButton)
			.ButtonStyle(FCoreStyle::Get(), "NoBorder")
			.OnClicked(OnClicked)
			[
				SAssignNew(OutBackground, SBorder)
				.BorderImage(GetWhiteBrush())
				.BorderBackgroundColor(FT66DotaTheme::ButtonNeutral())
				.Padding(FMargin(16.f, 10.f))
				[
					SNew(SBox)
					.MinDesiredWidth(MinWidth)
					.HAlign(HAlign_Center)
					[
						SAssignNew(OutText, STextBlock)
						.Text(Label)
						.Font(FT66DotaTheme::MakeFont(TEXT("Bold"), 16))
						.ColorAndOpacity(FT66DotaTheme::Text())
						.Justification(ETextJustify::Center)
					]
				]
			];
	}

	void SetActionButtonState(
		const TSharedPtr<SButton>& Button,
		const TSharedPtr<SBorder>& Background,
		const TSharedPtr<STextBlock>& Text,
		const bool bEnabled,
		const bool bDanger = false)
	{
		if (Button.IsValid())
		{
			Button->SetEnabled(bEnabled);
		}

		if (Background.IsValid())
		{
			Background->SetBorderBackgroundColor(
				bEnabled
					? (bDanger ? FT66DotaTheme::DangerButton() : FT66DotaTheme::SuccessButton())
					: FT66DotaTheme::ButtonPressed());
		}

		if (Text.IsValid())
		{
			Text->SetColorAndOpacity(bEnabled ? FT66DotaTheme::Text() : FT66DotaTheme::TextMuted());
		}
	}

	void SetTabButtonState(
		const TSharedPtr<SBorder>& Background,
		const TSharedPtr<STextBlock>& Text,
		const bool bActive)
	{
		if (Background.IsValid())
		{
			Background->SetBorderBackgroundColor(bActive ? FT66DotaTheme::Accent() : FT66DotaTheme::ButtonNeutral());
		}

		if (Text.IsValid())
		{
			Text->SetColorAndOpacity(bActive ? FT66DotaTheme::Text() : FT66DotaTheme::TextMuted());
		}
	}
}

void UT66IdolAltarOverlayWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World))
		{
			IdolManager->IdolStateChanged.RemoveDynamic(this, &UT66IdolAltarOverlayWidget::HandleIdolsChanged);
		}
	}

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && !PC->IsPaused())
		{
			PC->RestoreGameplayInputMode();
		}
	}

	Super::NativeDestruct();
}

void UT66IdolAltarOverlayWidget::HandleIdolsChanged()
{
	RefreshStock();
}

int32 UT66IdolAltarOverlayWidget::GetOfferStockIndexForVisibleSlot(const int32 VisibleSlotIndex) const
{
	return (FMath::Clamp(ActiveOfferCategoryIndex, 0, OfferCategoryCount - 1) * OfferSlotsPerCategory)
		+ FMath::Clamp(VisibleSlotIndex, 0, OfferSlotsPerCategory - 1);
}

bool UT66IdolAltarOverlayWidget::HasSelectionsRemaining() const
{
	const AT66IdolAltar* Altar = SourceAltar.Get();
	return !Altar || Altar->RemainingSelections > 0;
}

bool UT66IdolAltarOverlayWidget::ConsumeSelectionBudget()
{
	AT66IdolAltar* Altar = SourceAltar.Get();
	if (!Altar)
	{
		return false;
	}

	if (Altar->CatchUpSelectionsRemaining > 0)
	{
		if (UWorld* World = GetWorld())
		{
			if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World))
			{
				IdolManager->ConsumeCatchUpIdolPick();
			}
		}

		Altar->CatchUpSelectionsRemaining = FMath::Max(0, Altar->CatchUpSelectionsRemaining - 1);
	}

	if (Altar->RemainingSelections > 0)
	{
		Altar->RemainingSelections = FMath::Max(0, Altar->RemainingSelections - 1);
	}

	return Altar->RemainingSelections <= 0;
}

bool UT66IdolAltarOverlayWidget::IsTutorialSingleOfferMode() const
{
	const AT66IdolAltar* Altar = SourceAltar.Get();
	return Altar && Altar->bUseTutorialSingleOffer;
}

FName UT66IdolAltarOverlayWidget::GetTutorialOfferedIdolID() const
{
	const AT66IdolAltar* Altar = SourceAltar.Get();
	return Altar ? Altar->TutorialOfferedIdolID : NAME_None;
}

TSharedRef<SWidget> UT66IdolAltarOverlayWidget::RebuildWidget()
{
	UWorld* World = GetWorld();
	UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World);
	UT66LocalizationSubsystem* Loc = nullptr;
	if (World)
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
		}
	}

	if (IdolManager)
	{
		IdolManager->EnsureIdolStock();
		IdolManager->IdolStateChanged.RemoveDynamic(this, &UT66IdolAltarOverlayWidget::HandleIdolsChanged);
		IdolManager->IdolStateChanged.AddDynamic(this, &UT66IdolAltarOverlayWidget::HandleIdolsChanged);
	}

	const FText AltarTitle = Loc ? Loc->GetText_IdolAltarTitle() : NSLOCTEXT("T66.IdolAltar", "Title", "IDOL ALTAR");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	OffersButtonLabel = NSLOCTEXT("T66.IdolAltar", "Offers", "OFFERS");
	TradeButtonLabel = NSLOCTEXT("T66.IdolAltar", "Trade", "TRADE");
	const FText TakeLabel = NSLOCTEXT("T66.IdolAltar", "Take", "TAKE");
	const FText ReturnLabel = NSLOCTEXT("T66.IdolAltar", "Return", "RETURN");
	const FText RerollLabel = Loc ? Loc->GetText_Reroll() : NSLOCTEXT("T66.IdolAltar", "Reroll", "REROLL");

	OfferCardBoxes.SetNum(OfferSlotsPerCategory);
	OfferNameTexts.SetNum(OfferSlotsPerCategory);
	OfferIconImages.SetNum(OfferSlotsPerCategory);
	OfferIconBrushes.SetNum(OfferSlotsPerCategory);
	OfferTileBorders.SetNum(OfferSlotsPerCategory);
	OfferIconBorders.SetNum(OfferSlotsPerCategory);
	OfferButtons.SetNum(OfferSlotsPerCategory);
	OfferButtonBorders.SetNum(OfferSlotsPerCategory);
	OfferButtonTexts.SetNum(OfferSlotsPerCategory);

	TradeCardBoxes.SetNum(TradeSlotCount);
	TradeNameTexts.SetNum(TradeSlotCount);
	TradeIconImages.SetNum(TradeSlotCount);
	TradeIconBrushes.SetNum(TradeSlotCount);
	TradeTileBorders.SetNum(TradeSlotCount);
	TradeIconBorders.SetNum(TradeSlotCount);
	TradeButtons.SetNum(TradeSlotCount);
	TradeButtonBorders.SetNum(TradeSlotCount);
	TradeButtonTexts.SetNum(TradeSlotCount);

	for (int32 SlotIndex = 0; SlotIndex < OfferSlotsPerCategory; ++SlotIndex)
	{
		OfferIconBrushes[SlotIndex] = MakeShared<FSlateBrush>();
		OfferIconBrushes[SlotIndex]->DrawAs = ESlateBrushDrawType::Image;
		OfferIconBrushes[SlotIndex]->ImageSize = FVector2D(220.f, 220.f);
	}

	for (int32 SlotIndex = 0; SlotIndex < TradeSlotCount; ++SlotIndex)
	{
		TradeIconBrushes[SlotIndex] = MakeShared<FSlateBrush>();
		TradeIconBrushes[SlotIndex]->DrawAs = ESlateBrushDrawType::Image;
		TradeIconBrushes[SlotIndex]->ImageSize = FVector2D(220.f, 220.f);
	}

	TSharedRef<SVerticalBox> OffersContent = SNew(SVerticalBox);
	TSharedRef<SHorizontalBox> CategoryRow = SNew(SHorizontalBox);
	for (int32 SlotIndex = 0; SlotIndex < OfferSlotsPerCategory; ++SlotIndex)
	{
		TSharedRef<SWidget> ActionButton = MakeAltarButton(
			TakeLabel,
			FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnSelectSlot, SlotIndex),
			OfferButtons[SlotIndex],
			OfferButtonBorders[SlotIndex],
			OfferButtonTexts[SlotIndex],
			200.f);

		CategoryRow->AddSlot()
			.FillWidth(1.f)
			.Padding(SlotIndex > 0 ? FMargin(14.f, 0.f, 0.f, 0.f) : FMargin(0.f))
			[
				SAssignNew(OfferCardBoxes[SlotIndex], SBox)
				.MinDesiredHeight(470.f)
				[
					SNew(SBorder)
					.BorderImage(GetWhiteBrush())
					.BorderBackgroundColor(FT66DotaTheme::SlotOuter())
					.Padding(1.f)
					[
						SAssignNew(OfferTileBorders[SlotIndex], SBorder)
						.BorderImage(GetWhiteBrush())
						.BorderBackgroundColor(FT66DotaTheme::Border())
						.Padding(1.f)
						[
							SNew(SBorder)
							.BorderImage(GetWhiteBrush())
							.BorderBackgroundColor(FT66DotaTheme::SlotFill())
							.Padding(FMargin(16.f))
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SAssignNew(OfferNameTexts[SlotIndex], STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66DotaTheme::MakeFont(TEXT("Bold"), 22))
									.ColorAndOpacity(FT66DotaTheme::Text())
									.Justification(ETextJustify::Center)
									.AutoWrapText(true)
								]
								+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f, 20.f, 0.f, 20.f)
								[
									SAssignNew(OfferIconBorders[SlotIndex], SBorder)
									.BorderImage(GetWhiteBrush())
									.BorderBackgroundColor(FT66DotaTheme::Border())
									.Padding(3.f)
									[
										SNew(SBorder)
										.BorderImage(GetWhiteBrush())
										.BorderBackgroundColor(FT66DotaTheme::PanelInner())
										.Padding(8.f)
										[
											SNew(SBox)
											.WidthOverride(220.f)
											.HeightOverride(220.f)
											[
												SAssignNew(OfferIconImages[SlotIndex], SImage)
												.Image(OfferIconBrushes[SlotIndex].Get())
												.ColorAndOpacity(FLinearColor::White)
											]
										]
									]
								]
								+ SVerticalBox::Slot().FillHeight(1.f)
								[
									SNew(SSpacer)
								]
								+ SVerticalBox::Slot().AutoHeight()
								[
									ActionButton
								]
							]
						]
					]
				]
			];
	}

	OffersContent->AddSlot().AutoHeight()
	[
		SAssignNew(CategoryTitleText, STextBlock)
		.Text(GetOfferSectionTitle(ActiveOfferCategoryIndex))
		.Font(FT66DotaTheme::MakeFont(TEXT("Bold"), 20))
		.ColorAndOpacity(FT66DotaTheme::Accent2())
	];

	OffersContent->AddSlot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
	[
		CategoryRow
	];

	OffersContent->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f, 0.f, 0.f)
	[
		MakeAltarButton(
			RerollLabel,
			FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnReroll),
			RerollButton,
			RerollButtonBorder,
			RerollButtonText,
			180.f)
	];

	TSharedRef<SHorizontalBox> TradeRow = SNew(SHorizontalBox);
	for (int32 SlotIndex = 0; SlotIndex < TradeSlotCount; ++SlotIndex)
	{
		TSharedRef<SWidget> ActionButton = MakeAltarButton(
			ReturnLabel,
			FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnSellSlot, SlotIndex),
			TradeButtons[SlotIndex],
			TradeButtonBorders[SlotIndex],
			TradeButtonTexts[SlotIndex],
			200.f);

		TradeRow->AddSlot()
			.FillWidth(1.f)
			.Padding(SlotIndex > 0 ? FMargin(14.f, 0.f, 0.f, 0.f) : FMargin(0.f))
			[
				SAssignNew(TradeCardBoxes[SlotIndex], SBox)
				.MinDesiredHeight(470.f)
				[
					SNew(SBorder)
					.BorderImage(GetWhiteBrush())
					.BorderBackgroundColor(FT66DotaTheme::SlotOuter())
					.Padding(1.f)
					[
						SAssignNew(TradeTileBorders[SlotIndex], SBorder)
						.BorderImage(GetWhiteBrush())
						.BorderBackgroundColor(FT66DotaTheme::Border())
						.Padding(1.f)
						[
							SNew(SBorder)
							.BorderImage(GetWhiteBrush())
							.BorderBackgroundColor(FT66DotaTheme::SlotFill())
							.Padding(FMargin(16.f))
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SAssignNew(TradeNameTexts[SlotIndex], STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66DotaTheme::MakeFont(TEXT("Bold"), 22))
									.ColorAndOpacity(FT66DotaTheme::Text())
									.Justification(ETextJustify::Center)
									.AutoWrapText(true)
								]
								+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f, 20.f, 0.f, 20.f)
								[
									SAssignNew(TradeIconBorders[SlotIndex], SBorder)
									.BorderImage(GetWhiteBrush())
									.BorderBackgroundColor(FT66DotaTheme::Border())
									.Padding(3.f)
									[
										SNew(SBorder)
										.BorderImage(GetWhiteBrush())
										.BorderBackgroundColor(FT66DotaTheme::PanelInner())
										.Padding(8.f)
										[
											SNew(SBox)
											.WidthOverride(220.f)
											.HeightOverride(220.f)
											[
												SAssignNew(TradeIconImages[SlotIndex], SImage)
												.Image(TradeIconBrushes[SlotIndex].Get())
												.ColorAndOpacity(FLinearColor::White)
											]
										]
									]
								]
								+ SVerticalBox::Slot().FillHeight(1.f)
								[
									SNew(SSpacer)
								]
								+ SVerticalBox::Slot().AutoHeight()
								[
									ActionButton
								]
							]
						]
					]
				]
			];
	}

	TSharedRef<SWidget> OffersView = SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			OffersContent
		];

	TSharedRef<SWidget> TradeView = SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.IdolAltar", "CurrentIdols", "CURRENT IDOLS"))
				.Font(FT66DotaTheme::MakeFont(TEXT("Bold"), 18))
				.ColorAndOpacity(FT66DotaTheme::Accent2())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				TradeRow
			]
		];

	TSharedPtr<SButton> BackButton;
	TSharedPtr<SBorder> BackButtonBorder;
	TSharedPtr<STextBlock> BackButtonText;
	TSharedPtr<SButton> OffersTabButton;
	TSharedPtr<SButton> TradeTabButton;

	const TAttribute<FMargin> VerticalSafeInsets = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		const FMargin SafeInsets = FT66Style::GetSafeFrameInsets();
		return FMargin(0.f, SafeInsets.Top, 0.f, SafeInsets.Bottom);
	});

	const TAttribute<FOptionalSize> SurfaceWidthAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		return FOptionalSize(FMath::Max(1.f, FT66Style::GetViewportLogicalSize().X));
	});

	TSharedRef<SWidget> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(GetWhiteBrush())
			.BorderBackgroundColor(FLinearColor::Black)
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(GetWhiteBrush())
			.BorderBackgroundColor(FLinearColor::Transparent)
			.Padding(VerticalSafeInsets)
			[
				SNew(SBox)
				.WidthOverride(SurfaceWidthAttr)
				[
					FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							MakeAltarButton(
								BackText,
								FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnBack),
								BackButton,
								BackButtonBorder,
								BackButtonText,
								110.f)
						]
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(AltarTitle)
							.Font(FT66DotaTheme::MakeFont(TEXT("Bold"), 54))
							.ColorAndOpacity(FT66DotaTheme::Text())
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeAltarButton(
									OffersButtonLabel,
									FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnShowOffers),
									OffersTabButton,
									OffersTabBorder,
									OffersTabText,
									120.f)
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
							[
								MakeAltarButton(
									TradeButtonLabel,
									FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnShowTrade),
									TradeTabButton,
									TradeTabBorder,
									TradeTabText,
									120.f)
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
					[
						SAssignNew(StatusText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66DotaTheme::MakeFont(TEXT("Regular"), 15))
						.ColorAndOpacity(FT66DotaTheme::TextMuted())
					]
					+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 18.f, 0.f, 0.f)
					[
						SAssignNew(ViewSwitcher, SWidgetSwitcher)
						+ SWidgetSwitcher::Slot()
						[
							OffersView
						]
						+ SWidgetSwitcher::Slot()
						[
							TradeView
						]
					],
					FT66PanelParams(ET66PanelType::Panel)
						.SetColor(FT66Style::Tokens::Panel)
						.SetPadding(FMargin(28.f)))
				]
			]
		];

	SetActionButtonState(BackButton, BackButtonBorder, BackButtonText, true, false);
	SetActionButtonState(RerollButton, RerollButtonBorder, RerollButtonText, true, false);
	if (IsTutorialSingleOfferMode() && StatusText.IsValid())
	{
		StatusText->SetText(NSLOCTEXT("T66.IdolAltar", "TutorialSingleOffer", "Aria prepared one idol for this lesson."));
	}
	RefreshStock();
	RefreshViewState();
	return FT66Style::MakeResponsiveRoot(Root);
}

void UT66IdolAltarOverlayWidget::RefreshStock()
{
	UWorld* World = GetWorld();
	UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World);
	if (!IdolManager)
	{
		return;
	}

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	const TArray<FName>& Stock = IdolManager->GetIdolStockIDs();
	const TArray<FName>& Equipped = IdolManager->GetEquippedIdols();
	const bool bHasEmptySlot = Equipped.Contains(NAME_None);
	const bool bHasSelectionAllowance = HasSelectionsRemaining();
	const bool bTutorialSingleOffer = IsTutorialSingleOfferMode();
	const FName TutorialOfferedIdolID = GetTutorialOfferedIdolID();

	if (CategoryTitleText.IsValid())
	{
		CategoryTitleText->SetText(
			bTutorialSingleOffer
				? NSLOCTEXT("T66.IdolAltar", "TutorialOfferSection", "GUIDE OFFER")
				: GetOfferSectionTitle(ActiveOfferCategoryIndex));
	}

	for (int32 VisibleSlotIndex = 0; VisibleSlotIndex < OfferSlotsPerCategory; ++VisibleSlotIndex)
	{
		const int32 SlotIndex = GetOfferStockIndexForVisibleSlot(VisibleSlotIndex);
		const bool bHasItem = bTutorialSingleOffer
			? (VisibleSlotIndex == 0 && !TutorialOfferedIdolID.IsNone())
			: (Stock.IsValidIndex(SlotIndex) && !Stock[SlotIndex].IsNone());
		const FName IdolID = bTutorialSingleOffer
			? TutorialOfferedIdolID
			: (bHasItem ? Stock[SlotIndex] : NAME_None);
		const bool bSelected = bTutorialSingleOffer ? false : IdolManager->IsIdolStockSlotSelected(SlotIndex);
		const int32 EquippedSlot = FindEquippedSlotByIdolID(IdolManager, IdolID);
		const bool bOwned = EquippedSlot != INDEX_NONE;
		const int32 CurrentTierValue = bOwned ? IdolManager->GetEquippedIdolLevelInSlot(EquippedSlot) : 0;
		const ET66ItemRarity OfferedRarity = bTutorialSingleOffer
			? ET66ItemRarity::Black
			: (bHasItem ? IdolManager->GetIdolStockRarityInSlot(SlotIndex) : ET66ItemRarity::Black);
		const bool bAtMaxRarity = bOwned && CurrentTierValue >= UT66IdolManagerSubsystem::MaxIdolLevel;
		const bool bCanTake = bHasItem
			&& !bSelected
			&& bHasSelectionAllowance
			&& ((bOwned && !bAtMaxRarity) || (!bOwned && bHasEmptySlot));

		if (OfferCardBoxes.IsValidIndex(VisibleSlotIndex) && OfferCardBoxes[VisibleSlotIndex].IsValid())
		{
			OfferCardBoxes[VisibleSlotIndex]->SetVisibility((bHasItem && !bSelected) ? EVisibility::Visible : EVisibility::Collapsed);
		}

		FIdolData IdolData;
		const bool bHasData = bHasItem && GI && GI->GetIdolData(IdolID, IdolData);
		const TSoftObjectPtr<UTexture2D> IdolIconSoft = bHasData ? IdolData.GetIconForRarity(OfferedRarity) : TSoftObjectPtr<UTexture2D>();
		const FLinearColor RarityColor = bHasItem ? FItemData::GetItemRarityColor(OfferedRarity) : FT66DotaTheme::Border();
		const TSharedPtr<IToolTip> IdolTooltip = (bHasData && Loc)
			? MakeIdolTooltip(Loc->GetText_IdolDisplayName(IdolID), Loc->GetText_IdolTooltip(IdolID))
			: nullptr;

		if (OfferNameTexts.IsValidIndex(VisibleSlotIndex) && OfferNameTexts[VisibleSlotIndex].IsValid())
		{
			OfferNameTexts[VisibleSlotIndex]->SetText(
				bHasItem
					? (Loc ? Loc->GetText_IdolDisplayName(IdolID) : FText::FromName(IdolID))
					: FText::GetEmpty());
		}

		if (OfferTileBorders.IsValidIndex(VisibleSlotIndex) && OfferTileBorders[VisibleSlotIndex].IsValid())
		{
			OfferTileBorders[VisibleSlotIndex]->SetBorderBackgroundColor(DarkenColor(RarityColor, 0.72f));
			OfferTileBorders[VisibleSlotIndex]->SetToolTip(IdolTooltip);
		}

		if (OfferIconBorders.IsValidIndex(VisibleSlotIndex) && OfferIconBorders[VisibleSlotIndex].IsValid())
		{
			OfferIconBorders[VisibleSlotIndex]->SetBorderBackgroundColor(RarityColor);
			OfferIconBorders[VisibleSlotIndex]->SetToolTip(IdolTooltip);
		}

		if (OfferIconBrushes.IsValidIndex(VisibleSlotIndex) && OfferIconBrushes[VisibleSlotIndex].IsValid())
		{
			if (!IdolIconSoft.IsNull() && TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, IdolIconSoft, this, OfferIconBrushes[VisibleSlotIndex], FName(TEXT("IdolOffer"), VisibleSlotIndex + 1), true);
			}
			else
			{
				OfferIconBrushes[VisibleSlotIndex]->SetResourceObject(nullptr);
			}
		}

		if (OfferIconImages.IsValidIndex(VisibleSlotIndex) && OfferIconImages[VisibleSlotIndex].IsValid())
		{
			OfferIconImages[VisibleSlotIndex]->SetVisibility(!IdolIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Hidden);
			OfferIconImages[VisibleSlotIndex]->SetToolTip(IdolTooltip);
		}

		if (OfferButtonTexts.IsValidIndex(VisibleSlotIndex) && OfferButtonTexts[VisibleSlotIndex].IsValid())
		{
			OfferButtonTexts[VisibleSlotIndex]->SetText(NSLOCTEXT("T66.IdolAltar", "Take", "TAKE"));
		}

		SetActionButtonState(
			OfferButtons.IsValidIndex(VisibleSlotIndex) ? OfferButtons[VisibleSlotIndex] : TSharedPtr<SButton>(),
			OfferButtonBorders.IsValidIndex(VisibleSlotIndex) ? OfferButtonBorders[VisibleSlotIndex] : TSharedPtr<SBorder>(),
			OfferButtonTexts.IsValidIndex(VisibleSlotIndex) ? OfferButtonTexts[VisibleSlotIndex] : TSharedPtr<STextBlock>(),
			bCanTake,
			false);
	}

	RefreshTrade();
	RefreshViewState();
}

void UT66IdolAltarOverlayWidget::RefreshTrade()
{
	UWorld* World = GetWorld();
	UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World);
	if (!IdolManager)
	{
		return;
	}

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	const TArray<FName>& Equipped = IdolManager->GetEquippedIdols();

	for (int32 SlotIndex = 0; SlotIndex < TradeSlotCount; ++SlotIndex)
	{
		const bool bHasItem = Equipped.IsValidIndex(SlotIndex) && !Equipped[SlotIndex].IsNone();
		const FName IdolID = bHasItem ? Equipped[SlotIndex] : NAME_None;
		const ET66ItemRarity CurrentRarity = bHasItem ? IdolManager->GetEquippedIdolRarityInSlot(SlotIndex) : ET66ItemRarity::Black;
		const FLinearColor RarityColor = bHasItem ? FItemData::GetItemRarityColor(CurrentRarity) : FT66DotaTheme::Border();

		if (TradeCardBoxes.IsValidIndex(SlotIndex) && TradeCardBoxes[SlotIndex].IsValid())
		{
			TradeCardBoxes[SlotIndex]->SetVisibility(bHasItem ? EVisibility::Visible : EVisibility::Collapsed);
		}

		FIdolData IdolData;
		const bool bHasData = bHasItem && GI && GI->GetIdolData(IdolID, IdolData);
		const TSoftObjectPtr<UTexture2D> IdolIconSoft = bHasData ? IdolData.GetIconForRarity(CurrentRarity) : TSoftObjectPtr<UTexture2D>();
		const TSharedPtr<IToolTip> IdolTooltip = (bHasData && Loc)
			? MakeIdolTooltip(Loc->GetText_IdolDisplayName(IdolID), Loc->GetText_IdolTooltip(IdolID))
			: nullptr;

		if (TradeNameTexts.IsValidIndex(SlotIndex) && TradeNameTexts[SlotIndex].IsValid())
		{
			TradeNameTexts[SlotIndex]->SetText(
				bHasItem
					? (Loc ? Loc->GetText_IdolDisplayName(IdolID) : FText::FromName(IdolID))
					: FText::GetEmpty());
		}

		if (TradeTileBorders.IsValidIndex(SlotIndex) && TradeTileBorders[SlotIndex].IsValid())
		{
			TradeTileBorders[SlotIndex]->SetBorderBackgroundColor(DarkenColor(RarityColor, 0.72f));
			TradeTileBorders[SlotIndex]->SetToolTip(IdolTooltip);
		}

		if (TradeIconBorders.IsValidIndex(SlotIndex) && TradeIconBorders[SlotIndex].IsValid())
		{
			TradeIconBorders[SlotIndex]->SetBorderBackgroundColor(RarityColor);
			TradeIconBorders[SlotIndex]->SetToolTip(IdolTooltip);
		}

		if (TradeIconBrushes.IsValidIndex(SlotIndex) && TradeIconBrushes[SlotIndex].IsValid())
		{
			if (!IdolIconSoft.IsNull() && TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, IdolIconSoft, this, TradeIconBrushes[SlotIndex], FName(TEXT("IdolTrade"), SlotIndex + 1), true);
			}
			else
			{
				TradeIconBrushes[SlotIndex]->SetResourceObject(nullptr);
			}
		}

		if (TradeIconImages.IsValidIndex(SlotIndex) && TradeIconImages[SlotIndex].IsValid())
		{
			TradeIconImages[SlotIndex]->SetVisibility(!IdolIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Hidden);
			TradeIconImages[SlotIndex]->SetToolTip(IdolTooltip);
		}

		if (TradeButtonTexts.IsValidIndex(SlotIndex) && TradeButtonTexts[SlotIndex].IsValid())
		{
			TradeButtonTexts[SlotIndex]->SetText(NSLOCTEXT("T66.IdolAltar", "Return", "RETURN"));
		}

		SetActionButtonState(
			TradeButtons.IsValidIndex(SlotIndex) ? TradeButtons[SlotIndex] : TSharedPtr<SButton>(),
			TradeButtonBorders.IsValidIndex(SlotIndex) ? TradeButtonBorders[SlotIndex] : TSharedPtr<SBorder>(),
			TradeButtonTexts.IsValidIndex(SlotIndex) ? TradeButtonTexts[SlotIndex] : TSharedPtr<STextBlock>(),
			bHasItem,
			true);
	}
}

void UT66IdolAltarOverlayWidget::RefreshViewState()
{
	if (ViewSwitcher.IsValid())
	{
		ViewSwitcher->SetActiveWidgetIndex(FMath::Clamp(ActiveViewIndex, T66IdolAltarView_Offers, T66IdolAltarView_Trade));
	}

	SetTabButtonState(OffersTabBorder, OffersTabText, ActiveViewIndex == T66IdolAltarView_Offers);
	SetTabButtonState(TradeTabBorder, TradeTabText, ActiveViewIndex == T66IdolAltarView_Trade);
}

FReply UT66IdolAltarOverlayWidget::OnSelectSlot(int32 SlotIndex)
{
	UWorld* World = GetWorld();
	UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World);
	if (!IdolManager)
	{
		return FReply::Handled();
	}

	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	if (!HasSelectionsRemaining())
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(NSLOCTEXT("T66.IdolAltar", "NoSelectionsRemaining", "No idol selections remain."));
		}
		return FReply::Handled();
	}

	const TArray<FName>& EquippedBefore = IdolManager->GetEquippedIdols();
	const bool bTutorialSingleOffer = IsTutorialSingleOfferMode();
	const int32 StockIndex = GetOfferStockIndexForVisibleSlot(SlotIndex);
	const TArray<FName>& Stock = IdolManager->GetIdolStockIDs();
	const FName IdolID = bTutorialSingleOffer
		? GetTutorialOfferedIdolID()
		: (Stock.IsValidIndex(StockIndex) ? Stock[StockIndex] : NAME_None);
	const bool bWasUpgrade = EquippedBefore.Contains(IdolID);

	const bool bSelectionApplied = bTutorialSingleOffer
		? IdolManager->SelectIdolFromAltar(IdolID)
		: IdolManager->SelectIdolFromStock(StockIndex);
	if (bSelectionApplied)
	{
		const bool bShouldClose = ConsumeSelectionBudget();
		if (StatusText.IsValid())
		{
			StatusText->SetText(
				bWasUpgrade
					? NSLOCTEXT("T66.IdolAltar", "UpgradeApplied", "Upgraded idol.")
					: (Loc ? Loc->GetText_IdolAltarEquipped() : NSLOCTEXT("T66.IdolAltar", "Equipped", "Equipped.")));
		}

		if (bShouldClose)
		{
			if (AT66IdolAltar* Altar = SourceAltar.Get())
			{
				Altar->Destroy();
			}
			SourceAltar.Reset();
			return OnBack();
		}
	}
	else
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(Loc ? Loc->GetText_IdolAltarNoEmptySlot() : NSLOCTEXT("T66.IdolAltar", "NoEmptySlot", "No empty idol slot."));
		}
	}

	RefreshStock();
	return FReply::Handled();
}

FReply UT66IdolAltarOverlayWidget::OnReroll()
{
	if (IsTutorialSingleOfferMode())
	{
		return FReply::Handled();
	}

	ActiveOfferCategoryIndex = (ActiveOfferCategoryIndex + 1) % OfferCategoryCount;
	RefreshStock();
	return FReply::Handled();
}

FReply UT66IdolAltarOverlayWidget::OnSellSlot(int32 SlotIndex)
{
	UWorld* World = GetWorld();
	UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World);
	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	if (!IdolManager)
	{
		return FReply::Handled();
	}

	const TArray<FName>& Equipped = IdolManager->GetEquippedIdols();
	const FName IdolID = Equipped.IsValidIndex(SlotIndex) ? Equipped[SlotIndex] : NAME_None;
	if (!IdolManager->SellEquippedIdolInSlot(SlotIndex))
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(NSLOCTEXT("T66.IdolAltar", "ReturnFailed", "Nothing to return."));
		}
		return FReply::Handled();
	}

	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::Format(
			NSLOCTEXT("T66.IdolAltar", "ReturnSuccess", "Returned {0}."),
			Loc ? Loc->GetText_IdolDisplayName(IdolID) : FText::FromName(IdolID)));
	}

	RefreshStock();
	return FReply::Handled();
}

FReply UT66IdolAltarOverlayWidget::OnShowOffers()
{
	ActiveViewIndex = T66IdolAltarView_Offers;
	RefreshViewState();
	return FReply::Handled();
}

FReply UT66IdolAltarOverlayWidget::OnShowTrade()
{
	if (IsTutorialSingleOfferMode())
	{
		return FReply::Handled();
	}

	ActiveViewIndex = T66IdolAltarView_Trade;
	RefreshViewState();
	return FReply::Handled();
}

FReply UT66IdolAltarOverlayWidget::OnBack()
{
	if (UWorld* World = GetWorld())
	{
		if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World))
		{
			IdolManager->IdolStateChanged.RemoveDynamic(this, &UT66IdolAltarOverlayWidget::HandleIdolsChanged);
		}
	}

	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}

	return FReply::Handled();
}
