// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66IdolAltarOverlayWidget.h"

#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66OverlayChromeStyle.h"
#include "UI/Style/T66Style.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	UT66IdolManagerSubsystem* GetIdolManager(UWorld* World)
	{
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
	}

	const FSlateBrush* GetIdolAltarWhiteBrush()
	{
		return FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
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

	FTextBlockStyle BuildIdolCardBodyStyle()
	{
		FTextBlockStyle Style = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");
		Style.SetFont(FT66Style::Tokens::FontRegular(15));
		Style.SetColorAndOpacity(FT66Style::Tokens::TextMuted);
		return Style;
	}

	const FTextBlockStyle& GetIdolCardBodyStyle()
	{
		static const FTextBlockStyle Style = BuildIdolCardBodyStyle();
		return Style;
	}

	const ISlateStyle& GetIdolCardRichTextStyle()
	{
		static TSharedPtr<FSlateStyleSet> StyleInstance;
		if (!StyleInstance.IsValid())
		{
			StyleInstance = MakeShared<FSlateStyleSet>(TEXT("T66.IdolAltarCardRichText"));

			const FTextBlockStyle BodyStyle = BuildIdolCardBodyStyle();
			FTextBlockStyle StrongStyle = BodyStyle;
			StrongStyle.SetFont(FT66Style::Tokens::FontBold(15));
			StrongStyle.SetColorAndOpacity(FT66Style::Tokens::Text);

			StyleInstance->Set("strong", StrongStyle);
		}

		return *StyleInstance.Get();
	}

	TArray<FString> GetHighlightCandidates(const ET66AttackCategory Category)
	{
		switch (Category)
		{
		case ET66AttackCategory::Pierce:
			return { TEXT("pierces"), TEXT("pierce") };
		case ET66AttackCategory::Bounce:
			return { TEXT("bounces"), TEXT("bounce") };
		case ET66AttackCategory::AOE:
			return { TEXT("aoe") };
		case ET66AttackCategory::DOT:
			return { TEXT("damage over time"), TEXT("over time") };
		default:
			return {};
		}
	}

	FText BuildIdolCardDescriptionMarkup(
		UT66LocalizationSubsystem* Loc,
		const FName IdolID,
		const ET66AttackCategory Category)
	{
		const FText Description = Loc ? Loc->GetText_IdolTooltip(IdolID) : FText::GetEmpty();
		FString DescriptionString = Description.ToString();
		if (DescriptionString.IsEmpty())
		{
			return Description;
		}

		const FString SearchString = DescriptionString.ToLower();
		for (const FString& Candidate : GetHighlightCandidates(Category))
		{
			const int32 MatchIndex = SearchString.Find(Candidate, ESearchCase::IgnoreCase);
			if (MatchIndex == INDEX_NONE)
			{
				continue;
			}

			const FString MatchedText = DescriptionString.Mid(MatchIndex, Candidate.Len());
			const FString Markup = FString::Printf(TEXT("<strong>%s</>"), *MatchedText.ToUpper());
			DescriptionString = DescriptionString.Left(MatchIndex) + Markup + DescriptionString.Mid(MatchIndex + Candidate.Len());
			return FText::FromString(DescriptionString);
		}

		return Description;
	}

	TSharedRef<SWidget> MakeAltarButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		TSharedPtr<SWidget>& OutButton,
		TSharedPtr<SBorder>& OutBackground,
		TSharedPtr<STextBlock>& OutText,
		float MinWidth = 150.f)
	{
		OutBackground.Reset();
		TSharedPtr<STextBlock> TextWidget;
		TSharedRef<SWidget> Button = T66OverlayChromeStyle::MakeButton(
			T66OverlayChromeStyle::MakeButtonParams(Label, OnClicked, ET66OverlayChromeButtonFamily::Primary)
			.SetMinWidth(MinWidth)
			.SetMinHeight(46.f)
			.SetPadding(FMargin(16.f, 10.f))
			.SetContent(
				SAssignNew(TextWidget, STextBlock)
				.Text(Label)
				.Font(FT66Style::Tokens::FontBold(16))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)));

		OutButton = Button;
		OutText = TextWidget;
		return Button;
	}

	void SetActionButtonState(
		const TSharedPtr<SWidget>& Button,
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
					? (bDanger ? FT66Style::Tokens::Danger : FT66Style::Tokens::Success)
					: FT66Style::ButtonPressed());
		}

		if (Text.IsValid())
		{
			Text->SetColorAndOpacity(bEnabled ? (bDanger ? FT66Style::Tokens::Danger : FT66Style::Tokens::Text) : FT66Style::Tokens::TextMuted);
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

void UT66IdolAltarOverlayWidget::ConsumeSelectionBudget(const int32 SlotIndex)
{
	AT66IdolAltar* Altar = SourceAltar.Get();
	if (!Altar)
	{
		return;
	}

	const bool bConsumedCatchUp = Altar->CatchUpSelectionsRemaining > 0;
	if (bConsumedCatchUp)
	{
		Altar->CatchUpSelectionsRemaining = FMath::Max(0, Altar->CatchUpSelectionsRemaining - 1);
	}

	if (Altar->RemainingSelections > 0)
	{
		Altar->RemainingSelections = FMath::Max(0, Altar->RemainingSelections - 1);
	}

	if (IsTutorialSingleOfferMode())
	{
		Altar->SetTutorialOfferConsumedCatchUp(bConsumedCatchUp);
	}
	else
	{
		Altar->SetSelectedStockSlotConsumedCatchUp(SlotIndex, bConsumedCatchUp);
	}

	if (UWorld* World = GetWorld())
	{
		if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World))
		{
			IdolManager->SetRemainingCatchUpIdolPicks(Altar->CatchUpSelectionsRemaining);
		}
	}
}

void UT66IdolAltarOverlayWidget::RefundSelectionBudget(const int32 SlotIndex)
{
	AT66IdolAltar* Altar = SourceAltar.Get();
	if (!Altar)
	{
		return;
	}

	bool bRefundCatchUp = false;
	if (IsTutorialSingleOfferMode())
	{
		bRefundCatchUp = Altar->DidTutorialOfferConsumeCatchUp();
		Altar->SetTutorialOfferConsumedCatchUp(false);
	}
	else
	{
		bRefundCatchUp = Altar->DidSelectedStockSlotConsumeCatchUp(SlotIndex);
		Altar->SetSelectedStockSlotConsumedCatchUp(SlotIndex, false);
	}

	if (bRefundCatchUp)
	{
		++Altar->CatchUpSelectionsRemaining;
	}

	++Altar->RemainingSelections;

	if (UWorld* World = GetWorld())
	{
		if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World))
		{
			IdolManager->SetRemainingCatchUpIdolPicks(Altar->CatchUpSelectionsRemaining);
		}
	}
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
	const FText TakeLabel = NSLOCTEXT("T66.IdolAltar", "Take", "TAKE");
	const float IdolCardWidth = 286.f;
	const float IdolCardHeight = 510.f;
	const float IdolIconSize = 206.f;
	const float IdolCardGap = 18.f;
	const float IdolCardPadding = 12.f;
	const float IdolNameHeight = 52.f;
	const float IdolDescriptionHeight = 86.f;

	OfferCardBoxes.SetNum(OfferSlotsPerCategory);
	OfferNameTexts.SetNum(OfferSlotsPerCategory);
	OfferDescriptionTexts.SetNum(OfferSlotsPerCategory);
	OfferIconImages.SetNum(OfferSlotsPerCategory);
	OfferIconBrushes.SetNum(OfferSlotsPerCategory);
	OfferTileBorders.SetNum(OfferSlotsPerCategory);
	OfferIconBorders.SetNum(OfferSlotsPerCategory);
	OfferButtons.SetNum(OfferSlotsPerCategory);
	OfferButtonBorders.SetNum(OfferSlotsPerCategory);
	OfferButtonTexts.SetNum(OfferSlotsPerCategory);

	for (int32 SlotIndex = 0; SlotIndex < OfferSlotsPerCategory; ++SlotIndex)
	{
		OfferIconBrushes[SlotIndex] = MakeShared<FSlateBrush>();
		OfferIconBrushes[SlotIndex]->DrawAs = ESlateBrushDrawType::Image;
		OfferIconBrushes[SlotIndex]->ImageSize = FVector2D(IdolIconSize, IdolIconSize);
	}

	TSharedRef<SHorizontalBox> CardRow = SNew(SHorizontalBox);
	for (int32 SlotIndex = 0; SlotIndex < OfferSlotsPerCategory; ++SlotIndex)
	{
		TSharedRef<SWidget> ActionButton = MakeAltarButton(
			TakeLabel,
			FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnToggleSlot, SlotIndex),
			OfferButtons[SlotIndex],
			OfferButtonBorders[SlotIndex],
			OfferButtonTexts[SlotIndex],
			220.f);

		CardRow->AddSlot()
		.AutoWidth()
		.Padding(SlotIndex > 0 ? FMargin(IdolCardGap, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			SAssignNew(OfferCardBoxes[SlotIndex], SBox)
			.WidthOverride(IdolCardWidth)
			.HeightOverride(IdolCardHeight)
			[
				SAssignNew(OfferTileBorders[SlotIndex], SBorder)
				.BorderImage(T66OverlayChromeStyle::GetBrush(ET66OverlayChromeBrush::OfferCardNormal))
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(FMargin(IdolCardPadding))
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
					.BorderBackgroundColor(FT66Style::Tokens::Panel2)
					.Padding(FMargin(0.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBox)
							.HeightOverride(IdolNameHeight)
							.VAlign(VAlign_Center)
							[
								SAssignNew(OfferNameTexts[SlotIndex], STextBlock)
								.Text(FText::GetEmpty())
								.Font(FT66Style::Tokens::FontBold(18))
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
							]
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 0.f)
						[
							SAssignNew(OfferIconBorders[SlotIndex], SBorder)
							.BorderImage(T66OverlayChromeStyle::GetBrush(ET66OverlayChromeBrush::SlotNormal))
							.BorderBackgroundColor(FLinearColor::White)
							.Padding(4.f)
							[
								SNew(SBox)
								.WidthOverride(IdolIconSize)
								.HeightOverride(IdolIconSize)
								[
									SAssignNew(OfferIconImages[SlotIndex], SImage)
									.Image(OfferIconBrushes[SlotIndex].Get())
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(SBox)
							.HeightOverride(IdolDescriptionHeight)
							.VAlign(VAlign_Center)
							[
								SAssignNew(OfferDescriptionTexts[SlotIndex], SRichTextBlock)
								.Text(FText::GetEmpty())
								.TextStyle(&GetIdolCardBodyStyle())
								.DecoratorStyleSet(&GetIdolCardRichTextStyle())
								.AutoWrapText(true)
								.Justification(ETextJustify::Center)
							]
						]
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SNew(SSpacer)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
						[
							ActionButton
						]
					]
				]
			]
		];
	}

	TSharedPtr<SWidget> BackButton;
	TSharedPtr<SBorder> BackButtonBorder;
	TSharedPtr<STextBlock> BackButtonText;

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
			.BorderImage(GetIdolAltarWhiteBrush())
			.BorderBackgroundColor(FLinearColor(0.008f, 0.010f, 0.017f, 0.97f))
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			.BorderBackgroundColor(FLinearColor::Transparent)
			.Padding(VerticalSafeInsets)
			[
				SNew(SBox)
				.WidthOverride(SurfaceWidthAttr)
				[
					T66OverlayChromeStyle::MakePanel(
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
							.Font(FT66Style::Tokens::FontBold(42))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(110.f)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
					[
						SAssignNew(StatusText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontRegular(15))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 0.f).HAlign(HAlign_Center)
					[
						CardRow
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f).HAlign(HAlign_Center)
					[
						MakeAltarButton(
							Loc ? Loc->GetText_Reroll() : NSLOCTEXT("T66.IdolAltar", "Reroll", "REROLL"),
							FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnReroll),
							RerollButton,
							RerollButtonBorder,
							RerollButtonText,
							180.f)
					]
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						SNew(SSpacer)
					]
					,
					ET66OverlayChromeBrush::CasinoShellPanel,
					FMargin(28.f))
				]
			]
		];

	SetActionButtonState(BackButton, BackButtonBorder, BackButtonText, true, false);
	SetActionButtonState(RerollButton, RerollButtonBorder, RerollButtonText, !IsTutorialSingleOfferMode(), false);
	if (IsTutorialSingleOfferMode() && StatusText.IsValid())
	{
		StatusText->SetText(NSLOCTEXT("T66.IdolAltar", "TutorialSingleOffer", "Aria prepared one idol for this lesson."));
	}

	RefreshStock();
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

	if (RerollButton.IsValid())
	{
		RerollButton->SetVisibility(IsTutorialSingleOfferMode() ? EVisibility::Collapsed : EVisibility::Visible);
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
		const int32 EquippedSlot = FindEquippedSlotByIdolID(IdolManager, IdolID);
		const bool bOwned = EquippedSlot != INDEX_NONE;
		const bool bSelected = bTutorialSingleOffer
			? (bOwned && !HasSelectionsRemaining())
			: IdolManager->IsIdolStockSlotSelected(SlotIndex);
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
			OfferCardBoxes[VisibleSlotIndex]->SetVisibility(bHasItem ? EVisibility::Visible : EVisibility::Collapsed);
		}

		FIdolData IdolData;
		const bool bHasData = bHasItem && GI && GI->GetIdolData(IdolID, IdolData);
		const TSoftObjectPtr<UTexture2D> IdolIconSoft = bHasData ? IdolData.GetIconForRarity(OfferedRarity) : TSoftObjectPtr<UTexture2D>();
		const FLinearColor RarityColor = bHasItem ? FItemData::GetItemRarityColor(OfferedRarity) : FT66Style::Tokens::Panel2;

		if (OfferNameTexts.IsValidIndex(VisibleSlotIndex) && OfferNameTexts[VisibleSlotIndex].IsValid())
		{
			OfferNameTexts[VisibleSlotIndex]->SetText(
				bHasItem
					? (Loc ? Loc->GetText_IdolDisplayName(IdolID) : FText::FromName(IdolID))
					: FText::GetEmpty());
		}

		if (OfferDescriptionTexts.IsValidIndex(VisibleSlotIndex) && OfferDescriptionTexts[VisibleSlotIndex].IsValid())
		{
			OfferDescriptionTexts[VisibleSlotIndex]->SetText(
				bHasData
					? BuildIdolCardDescriptionMarkup(Loc, IdolID, IdolData.Category)
					: FText::GetEmpty());
			OfferDescriptionTexts[VisibleSlotIndex]->SetVisibility(bHasData ? EVisibility::Visible : EVisibility::Collapsed);
		}

		if (OfferTileBorders.IsValidIndex(VisibleSlotIndex) && OfferTileBorders[VisibleSlotIndex].IsValid())
		{
			OfferTileBorders[VisibleSlotIndex]->SetBorderBackgroundColor(bSelected ? FLinearColor(1.f, 0.94f, 0.78f, 1.f) : FLinearColor::White);
			OfferTileBorders[VisibleSlotIndex]->SetToolTip(nullptr);
		}

		if (OfferIconBorders.IsValidIndex(VisibleSlotIndex) && OfferIconBorders[VisibleSlotIndex].IsValid())
		{
			OfferIconBorders[VisibleSlotIndex]->SetBorderBackgroundColor(RarityColor);
			OfferIconBorders[VisibleSlotIndex]->SetToolTip(nullptr);
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
			OfferIconImages[VisibleSlotIndex]->SetToolTip(nullptr);
		}

		if (OfferButtonTexts.IsValidIndex(VisibleSlotIndex) && OfferButtonTexts[VisibleSlotIndex].IsValid())
		{
			OfferButtonTexts[VisibleSlotIndex]->SetText(
				bSelected
					? NSLOCTEXT("T66.IdolAltar", "Return", "RETURN")
					: NSLOCTEXT("T66.IdolAltar", "Take", "TAKE"));
		}

		SetActionButtonState(
			OfferButtons.IsValidIndex(VisibleSlotIndex) ? OfferButtons[VisibleSlotIndex] : TSharedPtr<SWidget>(),
			OfferButtonBorders.IsValidIndex(VisibleSlotIndex) ? OfferButtonBorders[VisibleSlotIndex] : TSharedPtr<SBorder>(),
			OfferButtonTexts.IsValidIndex(VisibleSlotIndex) ? OfferButtonTexts[VisibleSlotIndex] : TSharedPtr<STextBlock>(),
			bSelected || bCanTake,
			bSelected);
	}
}

FReply UT66IdolAltarOverlayWidget::OnToggleSlot(int32 SlotIndex)
{
	UWorld* World = GetWorld();
	UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World);
	if (!IdolManager)
	{
		return FReply::Handled();
	}

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const bool bTutorialSingleOffer = IsTutorialSingleOfferMode();
	const int32 StockIndex = GetOfferStockIndexForVisibleSlot(SlotIndex);
	const TArray<FName>& Stock = IdolManager->GetIdolStockIDs();
	const FName IdolID = bTutorialSingleOffer
		? GetTutorialOfferedIdolID()
		: (Stock.IsValidIndex(StockIndex) ? Stock[StockIndex] : NAME_None);
	const bool bSelected = bTutorialSingleOffer
		? (FindEquippedSlotByIdolID(IdolManager, IdolID) != INDEX_NONE && !HasSelectionsRemaining())
		: IdolManager->IsIdolStockSlotSelected(StockIndex);

	if (IdolID.IsNone())
	{
		return FReply::Handled();
	}

	if (bSelected)
	{
		const int32 EquippedSlot = FindEquippedSlotByIdolID(IdolManager, IdolID);
		if (EquippedSlot == INDEX_NONE || !IdolManager->SellEquippedIdolInSlot(EquippedSlot))
		{
			if (StatusText.IsValid())
			{
				StatusText->SetText(NSLOCTEXT("T66.IdolAltar", "ReturnFailed", "Nothing to return."));
			}
			return FReply::Handled();
		}

		RefundSelectionBudget(StockIndex);

		if (StatusText.IsValid())
		{
			StatusText->SetText(FText::Format(
				NSLOCTEXT("T66.IdolAltar", "ReturnSuccess", "Returned {0}."),
				Loc ? Loc->GetText_IdolDisplayName(IdolID) : FText::FromName(IdolID)));
		}

		RefreshStock();
		return FReply::Handled();
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
	const bool bWasUpgrade = EquippedBefore.Contains(IdolID);

	const bool bSelectionApplied = bTutorialSingleOffer
		? IdolManager->SelectIdolFromAltar(IdolID)
		: IdolManager->SelectIdolFromStock(StockIndex);
	if (!bSelectionApplied)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(Loc ? Loc->GetText_IdolAltarNoEmptySlot() : NSLOCTEXT("T66.IdolAltar", "NoEmptySlot", "No empty idol slot."));
		}
		return FReply::Handled();
	}

	ConsumeSelectionBudget(StockIndex);

	if (StatusText.IsValid())
	{
		StatusText->SetText(
			bWasUpgrade
				? NSLOCTEXT("T66.IdolAltar", "UpgradeApplied", "Upgraded idol.")
				: (Loc ? Loc->GetText_IdolAltarEquipped() : NSLOCTEXT("T66.IdolAltar", "Equipped", "Equipped.")));
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
