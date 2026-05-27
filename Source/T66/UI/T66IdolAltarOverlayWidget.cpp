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
#include "UI/Style/T66FlatStyle.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateStyle.h"
#include "Types/WidgetActiveTimerDelegate.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWidget.h"
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
		FTextBlockStyle Style;
		Style.SetFont(FT66FlatStyle::MakeFont(15));
		Style.SetColorAndOpacity(FT66FlatStyle::SecondaryText());
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
			StrongStyle.SetFont(FT66FlatStyle::MakeBoldFont(15));
			StrongStyle.SetColorAndOpacity(FT66FlatStyle::PrimaryText());

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
		float MinWidth = 150.f,
		FName Tag = NAME_None)
	{
		OutBackground.Reset();
		TSharedPtr<STextBlock> TextWidget;
		TSharedRef<SWidget> Button = FT66FlatStyle::MakeFlatToggleGroupButton(
			ET66FlatState::Default,
				SAssignNew(TextWidget, STextBlock)
				.Text(Label)
				.Font(FT66FlatStyle::MakeBoldFont(16))
				.ColorAndOpacity(FT66FlatStyle::PrimaryText())
				.Justification(ETextJustify::Center),
			OnClicked,
			FMargin(16.f, 10.f),
			MinWidth,
			46.f,
			true,
			Tag);

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
					? (bDanger ? FT66FlatStyle::SelectedBorder() : FT66FlatStyle::DefaultBorder())
					: FT66FlatStyle::DisabledBorder());
		}

		if (Text.IsValid())
		{
			Text->SetColorAndOpacity(bEnabled ? (bDanger ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText()) : FT66FlatStyle::DisabledText());
		}
	}
}

void UT66IdolAltarOverlayWidget::NativeDestruct()
{
	CommitPendingSelectionIfNeeded();
	StopAnimationActiveTimer();

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
	(void)SlotIndex;
	AT66IdolAltar* Altar = SourceAltar.Get();
	if (!Altar)
	{
		return;
	}

	if (Altar->RemainingSelections > 0)
	{
		Altar->RemainingSelections = FMath::Max(0, Altar->RemainingSelections - 1);
	}
}

void UT66IdolAltarOverlayWidget::RefundSelectionBudget(const int32 SlotIndex)
{
	(void)SlotIndex;
	AT66IdolAltar* Altar = SourceAltar.Get();
	if (!Altar)
	{
		return;
	}

	++Altar->RemainingSelections;
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
	OfferRevealAlphas.Init(1.f, OfferSlotsPerCategory);
	OfferLiftOffsets.Init(0.f, OfferSlotsPerCategory);
	OfferGlowAlphas.Init(0.f, OfferSlotsPerCategory);
	OfferSelectionAlphas.Init(0.f, OfferSlotsPerCategory);
	OfferBaseBorderColors.Init(FT66FlatStyle::DefaultBorder(), OfferSlotsPerCategory);
	OfferGlowColors.Init(FT66FlatStyle::SelectedBorder(), OfferSlotsPerCategory);

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
			220.f,
			FName(*FString::Printf(TEXT("IdolAltar.TakeButton.%d"), SlotIndex)));

		CardRow->AddSlot()
		.AutoWidth()
		.Padding(SlotIndex > 0 ? FMargin(IdolCardGap, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			SAssignNew(OfferCardBoxes[SlotIndex], SBox)
			.WidthOverride(IdolCardWidth)
			.HeightOverride(IdolCardHeight)
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(IdolCardPadding),
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FT66FlatStyle::DefaultFill())
					.Padding(FMargin(0.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBox)
							.HeightOverride(IdolNameHeight)
							.VAlign(VAlign_Center)
							[
								FT66FlatStyle::AttachMetadata(
									SAssignNew(OfferNameTexts[SlotIndex], STextBlock)
								.Text(FText::GetEmpty())
									.Font(FT66FlatStyle::MakeBoldFont(18))
									.ColorAndOpacity(FT66FlatStyle::PrimaryText())
								.Justification(ETextJustify::Center)
									.AutoWrapText(true),
									FName(*FString::Printf(TEXT("IdolAltar.CardTitle.%d"), SlotIndex)),
									TEXT("Label.Header"),
									ET66FlatState::Default,
									TOptional<FLinearColor>(),
									false,
									NAME_None,
									true)
							]
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 0.f)
						[
							SAssignNew(OfferIconBorders[SlotIndex], SBorder)
							.BorderImage(GetIdolAltarWhiteBrush())
							.BorderBackgroundColor(FT66FlatStyle::DefaultBorder())
							.Padding(4.f)
							[
								SNew(SBox)
								.WidthOverride(IdolIconSize)
								.HeightOverride(IdolIconSize)
								[
									FT66FlatStyle::AttachMetadata(
										SAssignNew(OfferIconImages[SlotIndex], SImage)
										.Image(OfferIconBrushes[SlotIndex].Get())
										.ColorAndOpacity(FLinearColor::White),
										FName(*FString::Printf(TEXT("IdolAltar.CardIcon.%d"), SlotIndex)),
										TEXT("Icon"),
										ET66FlatState::Default)
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(SBox)
							.HeightOverride(IdolDescriptionHeight)
							.VAlign(VAlign_Center)
							[
								FT66FlatStyle::AttachMetadata(
									SAssignNew(OfferDescriptionTexts[SlotIndex], SRichTextBlock)
								.Text(FText::GetEmpty())
								.TextStyle(&GetIdolCardBodyStyle())
								.DecoratorStyleSet(&GetIdolCardRichTextStyle())
								.AutoWrapText(true)
									.Justification(ETextJustify::Center),
									FName(*FString::Printf(TEXT("IdolAltar.CardDescription.%d"), SlotIndex)),
									TEXT("Label.Body"),
									ET66FlatState::Default,
									TOptional<FLinearColor>(),
									false,
									NAME_None,
									true)
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
					],
					&OfferTileBorders[SlotIndex],
					FName(*FString::Printf(TEXT("IdolAltar.Card.%d"), SlotIndex)))
			]
		];
	}

	TSharedPtr<SWidget> BackButton;
	TSharedPtr<SBorder> BackButtonBorder;
	TSharedPtr<STextBlock> BackButtonText;

	const TAttribute<FMargin> VerticalSafeInsets = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		const FMargin SafeInsets = FT66FlatStyle::GetSafeFrameInsets();
		return FMargin(0.f, SafeInsets.Top, 0.f, SafeInsets.Bottom);
	});

	const TAttribute<FOptionalSize> SurfaceWidthAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		return FOptionalSize(FMath::Max(1.f, FT66FlatStyle::GetViewportLogicalSize().X));
	});

	TSharedRef<SWidget> Root =
		FT66FlatStyle::AttachMetadata(
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(GetIdolAltarWhiteBrush())
				.BorderBackgroundColor(FLinearColor(
					FT66FlatStyle::BackgroundColor().R,
					FT66FlatStyle::BackgroundColor().G,
					FT66FlatStyle::BackgroundColor().B,
					0.97f)),
				FName(TEXT("IdolAltar.Backdrop")),
				TEXT("Panel"),
				ET66FlatState::Default)
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
					FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(28.f),
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
								110.f,
								FName(TEXT("IdolAltar.BackButton")))
						]
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							FT66FlatStyle::AttachMetadata(
								SNew(STextBlock)
								.Text(AltarTitle)
								.Font(FT66FlatStyle::MakeBoldFont(42))
								.ColorAndOpacity(FT66FlatStyle::PrimaryText()),
								FName(TEXT("IdolAltar.Title")),
								TEXT("Label.Title"),
								ET66FlatState::Default,
								TOptional<FLinearColor>(),
								false,
								NAME_None,
								true)
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(110.f)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
					[
						FT66FlatStyle::AttachMetadata(
							SAssignNew(StatusText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66FlatStyle::MakeFont(15))
							.ColorAndOpacity(FT66FlatStyle::SecondaryText()),
							FName(TEXT("IdolAltar.StatusText")),
							TEXT("Label.Caption"),
							ET66FlatState::Default,
							TOptional<FLinearColor>(),
							false,
							NAME_None,
							true)
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
							180.f,
							FName(TEXT("IdolAltar.RerollButton")))
					]
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						SNew(SSpacer)
					]
					,
					nullptr,
					FName(TEXT("IdolAltar.Panel")))
				]
			]
		],
		FName(TEXT("IdolAltar.Root")),
		TEXT("Overlay"),
		ET66FlatState::Default);

	SetActionButtonState(BackButton, BackButtonBorder, BackButtonText, true, false);
	SetActionButtonState(RerollButton, RerollButtonBorder, RerollButtonText, !IsTutorialSingleOfferMode(), false);
	if (IsTutorialSingleOfferMode() && StatusText.IsValid())
	{
		StatusText->SetText(NSLOCTEXT("T66.IdolAltar", "TutorialSingleOffer", "Aria prepared one idol for this lesson."));
	}

	RefreshStock();
	TSharedRef<SWidget> ResponsiveRoot = FT66FlatStyle::MakeResponsiveRoot(Root);
	StartRevealAnimation(ResponsiveRoot);
	return ResponsiveRoot;
}

void UT66IdolAltarOverlayWidget::RefreshStock()
{
	UWorld* World = GetWorld();
	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World);
	if (!IdolManager)
	{
		return;
	}

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
		const bool bCanTake = bHasItem
			&& !bSelected
			&& bHasSelectionAllowance
			&& !bOwned
			&& bHasEmptySlot;
		const ET66ItemRarity OfferRarity = bTutorialSingleOffer
			? ET66ItemRarity::Black
			: IdolManager->GetIdolStockRarityInSlot(SlotIndex);
		const FLinearColor RarityColor = bHasItem ? FItemData::GetItemRarityColor(OfferRarity) : FT66FlatStyle::Tokens::Panel2;
		if (OfferBaseBorderColors.IsValidIndex(VisibleSlotIndex))
		{
			OfferBaseBorderColors[VisibleSlotIndex] = bSelected ? FT66FlatStyle::SelectedBorder() : RarityColor;
		}

		if (OfferCardBoxes.IsValidIndex(VisibleSlotIndex) && OfferCardBoxes[VisibleSlotIndex].IsValid())
		{
			OfferCardBoxes[VisibleSlotIndex]->SetVisibility(bHasItem ? EVisibility::Visible : EVisibility::Collapsed);
		}

		FIdolData IdolData;
		const bool bHasData = bHasItem && GI && GI->GetIdolData(IdolID, IdolData);
		const TSoftObjectPtr<UTexture2D> IdolIconSoft = bHasData ? IdolData.GetIconForRarity(OfferRarity) : TSoftObjectPtr<UTexture2D>();
		if (OfferGlowColors.IsValidIndex(VisibleSlotIndex))
		{
			OfferGlowColors[VisibleSlotIndex] = RarityColor;
		}

		if (OfferNameTexts.IsValidIndex(VisibleSlotIndex) && OfferNameTexts[VisibleSlotIndex].IsValid())
		{
			FText NameText = FText::GetEmpty();
			if (bHasItem)
			{
				const FText IdolName = Loc ? Loc->GetText_IdolDisplayName(IdolID) : FText::FromName(IdolID);
				const FText RarityName = Loc ? Loc->GetText_ItemRarityName(OfferRarity) : FText::GetEmpty();
				NameText = RarityName.IsEmpty()
					? IdolName
					: FText::Format(NSLOCTEXT("T66.IdolAltar", "IdolNameWithRarity", "{0}\n{1}"), IdolName, RarityName);
			}
			OfferNameTexts[VisibleSlotIndex]->SetText(NameText);
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
			OfferTileBorders[VisibleSlotIndex]->SetBorderBackgroundColor(bSelected ? FT66FlatStyle::SelectedBorder() : RarityColor);
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
			!bSelectionAnimationActive && (bSelected || bCanTake),
			bSelected);
	}

	ApplyOfferAnimationVisuals();
}

FReply UT66IdolAltarOverlayWidget::OnToggleSlot(int32 SlotIndex)
{
	if (bSelectionAnimationActive)
	{
		return FReply::Handled();
	}

	UWorld* World = GetWorld();
	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World);
	if (!IdolManager)
	{
		return FReply::Handled();
	}

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
	const bool bWasUpgrade = false;
	const int32 ExistingEquippedSlot = FindEquippedSlotByIdolID(IdolManager, IdolID);
	const bool bHasEmptySlot = EquippedBefore.Contains(NAME_None);
	if (ExistingEquippedSlot != INDEX_NONE)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(NSLOCTEXT("T66.IdolAltar", "AlreadyOwned", "That idol is already bound."));
		}
		return FReply::Handled();
	}
	if (!bHasEmptySlot)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(Loc ? Loc->GetText_IdolAltarNoEmptySlot() : NSLOCTEXT("T66.IdolAltar", "NoEmptySlot", "No empty idol slot."));
		}
		return FReply::Handled();
	}

	if (StatusText.IsValid())
	{
		StatusText->SetText(NSLOCTEXT("T66.IdolAltar", "SelectionRevealing", "Binding idol..."));
	}

	StartSelectionAnimation(SlotIndex, StockIndex, IdolID, bTutorialSingleOffer, bWasUpgrade);
	return FReply::Handled();
}

void UT66IdolAltarOverlayWidget::RegisterMarkerHandlers()
{
	MarkerDispatcher.ClearHandlers();
	MarkerDispatcher.RegisterAudioMarker(FName(TEXT("Idol.CardReveal")), FName(TEXT("UI.Click")), this);
	MarkerDispatcher.RegisterAudioMarker(FName(TEXT("Idol.SelectionPulse")), FName(TEXT("UI.Confirm")), this);
	MarkerDispatcher.RegisterHandler(
		FName(TEXT("Idol.SelectionCommit")),
		[this](const FT66AnimationMarkerEvent&)
		{
			CommitPendingSelectionIfNeeded();
		});
}

void UT66IdolAltarOverlayWidget::StartRevealAnimation(const TSharedRef<SWidget>& OwningWidget)
{
	RegisterMarkerHandlers();
	RevealAnimationGroup = FT66AnimationGroup();
	bRevealAnimationActive = true;

	for (int32 SlotIndex = 0; SlotIndex < OfferSlotsPerCategory; ++SlotIndex)
	{
		if (OfferRevealAlphas.IsValidIndex(SlotIndex))
		{
			OfferRevealAlphas[SlotIndex] = 0.f;
		}
		if (OfferLiftOffsets.IsValidIndex(SlotIndex))
		{
			OfferLiftOffsets[SlotIndex] = IsTutorialSingleOfferMode() && SlotIndex == 0 ? 30.f : 18.f;
		}
		if (OfferGlowAlphas.IsValidIndex(SlotIndex))
		{
			OfferGlowAlphas[SlotIndex] = 0.f;
		}

		FT66AnimationSequence CardSequence;
		const float Delay = IsTutorialSingleOfferMode() ? (SlotIndex == 0 ? 0.f : 0.16f) : static_cast<float>(SlotIndex) * 0.10f;
		if (Delay > KINDA_SMALL_NUMBER)
		{
			FT66AnimationTimeline Hold(FName(*FString::Printf(TEXT("Idol.Card%d.RevealDelay"), SlotIndex)));
			Hold.SetDuration(Delay);
			Hold.SetCurve(FT66AnimationCurveSpec(ET66AnimationCurve::Linear));
			CardSequence.AddTimeline(MoveTemp(Hold));
		}

		FT66AnimationTimeline Reveal(FName(*FString::Printf(TEXT("Idol.Card%d.Reveal"), SlotIndex)));
		Reveal.SetDuration(IsTutorialSingleOfferMode() && SlotIndex == 0 ? 0.55f : 0.40f);
		Reveal.SetCurve(FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic));
		TWeakObjectPtr<UT66IdolAltarOverlayWidget> WeakThis(this);
		Reveal.SetProgressCallback([WeakThis, SlotIndex](const float CurveValue)
		{
			if (UT66IdolAltarOverlayWidget* Self = WeakThis.Get())
			{
				const float Alpha = FMath::Clamp(CurveValue, 0.f, 1.f);
				if (Self->OfferRevealAlphas.IsValidIndex(SlotIndex))
				{
					Self->OfferRevealAlphas[SlotIndex] = Alpha;
				}
				if (Self->OfferLiftOffsets.IsValidIndex(SlotIndex))
				{
					Self->OfferLiftOffsets[SlotIndex] = FMath::Lerp(18.f, 0.f, Alpha);
				}
				if (Self->OfferGlowAlphas.IsValidIndex(SlotIndex))
				{
					Self->OfferGlowAlphas[SlotIndex] = 0.45f * Alpha;
				}
			}
		});
		Reveal.AddMarker({ FName(TEXT("Idol.CardReveal")), ET66AnimationMarkerType::ProgressBased, 0.05f, 0.f, ET66AnimationMarkerFirePolicy::Once, FName(TEXT("UI.Click")) });
		CardSequence.AddTimeline(MoveTemp(Reveal));
		RevealAnimationGroup.AddSequence(MoveTemp(CardSequence));
	}

	RevealAnimationGroup.Play();
	StartAnimationActiveTimer(OwningWidget);
	ApplyOfferAnimationVisuals();
}

void UT66IdolAltarOverlayWidget::StartSelectionAnimation(
	const int32 VisibleSlotIndex,
	const int32 StockIndex,
	const FName IdolID,
	const bool bTutorialSingleOffer,
	const bool bWasUpgrade)
{
	if (!OfferCardBoxes.IsValidIndex(VisibleSlotIndex))
	{
		return;
	}

	PendingSelection = FPendingSelection{};
	PendingSelection.bPending = true;
	PendingSelection.VisibleSlotIndex = VisibleSlotIndex;
	PendingSelection.StockIndex = StockIndex;
	PendingSelection.IdolID = IdolID;
	PendingSelection.bTutorialSingleOffer = bTutorialSingleOffer;
	PendingSelection.bWasUpgrade = bWasUpgrade;

	RegisterMarkerHandlers();
	bSelectionAnimationActive = true;
	SelectionAnimationSequence = FT66AnimationSequence();

	TWeakObjectPtr<UT66IdolAltarOverlayWidget> WeakThis(this);
	FT66AnimationTimeline Pulse(FName(TEXT("Idol.SelectionPulse")));
	Pulse.SetDuration(0.42f);
	Pulse.SetCurve(FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic));
	Pulse.SetProgressCallback([WeakThis, VisibleSlotIndex](const float CurveValue)
	{
		if (UT66IdolAltarOverlayWidget* Self = WeakThis.Get())
		{
			const float Alpha = FMath::Clamp(CurveValue, 0.f, 1.f);
			for (int32 SlotIndex = 0; SlotIndex < Self->OfferSelectionAlphas.Num(); ++SlotIndex)
			{
				Self->OfferSelectionAlphas[SlotIndex] = (SlotIndex == VisibleSlotIndex) ? Alpha : 0.f;
			}
			if (Self->OfferGlowAlphas.IsValidIndex(VisibleSlotIndex))
			{
				Self->OfferGlowAlphas[VisibleSlotIndex] = FMath::Lerp(Self->OfferGlowAlphas[VisibleSlotIndex], 1.f, Alpha);
			}
		}
	});
	Pulse.AddMarker({ FName(TEXT("Idol.SelectionPulse")), ET66AnimationMarkerType::ProgressBased, 0.05f, 0.f, ET66AnimationMarkerFirePolicy::Once, FName(TEXT("UI.Confirm")) });
	Pulse.AddMarker({ FName(TEXT("Idol.SelectionCommit")), ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, FName(TEXT("Idol.SelectionCommit")) });
	SelectionAnimationSequence.AddTimeline(MoveTemp(Pulse));
	SelectionAnimationSequence.Play();

	if (TSharedPtr<SWidget> Widget = AnimationActiveTimerWidget.Pin())
	{
		StartAnimationActiveTimer(Widget.ToSharedRef());
	}
	ApplyOfferAnimationVisuals();
	RefreshStock();
}

void UT66IdolAltarOverlayWidget::StartAnimationActiveTimer(const TSharedRef<SWidget>& OwningWidget)
{
	StopAnimationActiveTimer();
	AnimationActiveTimerWidget = OwningWidget;
	AnimationActiveTimerHandle = OwningWidget->RegisterActiveTimer(
		0.f,
		FWidgetActiveTimerDelegate::CreateUObject(this, &UT66IdolAltarOverlayWidget::HandleAnimationActiveTimer));
}

void UT66IdolAltarOverlayWidget::StopAnimationActiveTimer()
{
	TSharedPtr<SWidget> Widget = AnimationActiveTimerWidget.Pin();
	if (Widget.IsValid() && AnimationActiveTimerHandle.IsValid())
	{
		Widget->UnRegisterActiveTimer(AnimationActiveTimerHandle.ToSharedRef());
	}

	AnimationActiveTimerHandle.Reset();
	AnimationActiveTimerWidget.Reset();
}

EActiveTimerReturnType UT66IdolAltarOverlayWidget::HandleAnimationActiveTimer(double CurrentTime, const float DeltaTime)
{
	(void)CurrentTime;

	TickAnimations(DeltaTime);
	if (TSharedPtr<SWidget> Widget = AnimationActiveTimerWidget.Pin())
	{
		Widget->Invalidate(EInvalidateWidgetReason::Paint);
	}

	return (bRevealAnimationActive || bSelectionAnimationActive) ? EActiveTimerReturnType::Continue : EActiveTimerReturnType::Stop;
}

void UT66IdolAltarOverlayWidget::TickAnimations(const float DeltaSeconds)
{
	TArray<FT66AnimationMarkerEvent> MarkerEvents;
	if (bRevealAnimationActive)
	{
		RevealAnimationGroup.Tick(FMath::Clamp(DeltaSeconds, 0.f, 0.05f), MarkerEvents);
		if (T66Animation::IsTerminalState(RevealAnimationGroup.GetState()))
		{
			bRevealAnimationActive = false;
		}
	}
	if (bSelectionAnimationActive)
	{
		SelectionAnimationSequence.Tick(FMath::Clamp(DeltaSeconds, 0.f, 0.05f), MarkerEvents);
		if (T66Animation::IsTerminalState(SelectionAnimationSequence.GetState()))
		{
			bSelectionAnimationActive = false;
			CommitPendingSelectionIfNeeded();
			ClearPendingSelection();
			RefreshStock();
		}
	}

	MarkerDispatcher.Dispatch(MarkerEvents);
	ApplyOfferAnimationVisuals();
}

void UT66IdolAltarOverlayWidget::ApplyOfferAnimationVisuals()
{
	for (int32 SlotIndex = 0; SlotIndex < OfferSlotsPerCategory; ++SlotIndex)
	{
		const float RevealAlpha = OfferRevealAlphas.IsValidIndex(SlotIndex) ? OfferRevealAlphas[SlotIndex] : 1.f;
		const float SelectionAlpha = OfferSelectionAlphas.IsValidIndex(SlotIndex) ? OfferSelectionAlphas[SlotIndex] : 0.f;
		const bool bOtherDimmed = bSelectionAnimationActive
			&& PendingSelection.bPending
			&& SlotIndex != PendingSelection.VisibleSlotIndex;
		const float DimAlpha = bOtherDimmed ? 0.60f : 1.f;
		if (OfferCardBoxes.IsValidIndex(SlotIndex) && OfferCardBoxes[SlotIndex].IsValid())
		{
			const float Lift = (OfferLiftOffsets.IsValidIndex(SlotIndex) ? OfferLiftOffsets[SlotIndex] : 0.f) - (20.f * SelectionAlpha);
			OfferCardBoxes[SlotIndex]->SetRenderOpacity(RevealAlpha * DimAlpha);
			OfferCardBoxes[SlotIndex]->SetRenderTransform(FSlateRenderTransform(FVector2D(0.f, Lift)));
		}
		if (OfferTileBorders.IsValidIndex(SlotIndex) && OfferTileBorders[SlotIndex].IsValid())
		{
			const FLinearColor BaseColor = OfferBaseBorderColors.IsValidIndex(SlotIndex) ? OfferBaseBorderColors[SlotIndex] : FT66FlatStyle::DefaultBorder();
			const FLinearColor GlowColor = OfferGlowColors.IsValidIndex(SlotIndex) ? OfferGlowColors[SlotIndex] : FT66FlatStyle::SelectedBorder();
			const float GlowAlpha = OfferGlowAlphas.IsValidIndex(SlotIndex) ? FMath::Clamp(OfferGlowAlphas[SlotIndex] + SelectionAlpha * 0.5f, 0.f, 1.f) : 0.f;
			OfferTileBorders[SlotIndex]->SetBorderBackgroundColor(FMath::Lerp(BaseColor, GlowColor, GlowAlpha));
		}
	}
}

void UT66IdolAltarOverlayWidget::CommitPendingSelectionIfNeeded()
{
	if (!PendingSelection.bPending || PendingSelection.bCommitAttempted)
	{
		return;
	}

	PendingSelection.bCommitAttempted = true;
	UWorld* World = GetWorld();
	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66IdolManagerSubsystem* IdolManager = GetIdolManager(World);
	if (!IdolManager || PendingSelection.IdolID.IsNone())
	{
		return;
	}

	const bool bSelectionApplied = PendingSelection.bTutorialSingleOffer
		? IdolManager->SelectIdolFromAltar(PendingSelection.IdolID)
		: IdolManager->SelectIdolFromStock(PendingSelection.StockIndex);
	if (!bSelectionApplied)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(Loc ? Loc->GetText_IdolAltarNoEmptySlot() : NSLOCTEXT("T66.IdolAltar", "NoEmptySlot", "No empty idol slot."));
		}
		return;
	}

	ConsumeSelectionBudget(PendingSelection.StockIndex);
	if (StatusText.IsValid())
	{
		StatusText->SetText(
			PendingSelection.bWasUpgrade
				? NSLOCTEXT("T66.IdolAltar", "UpgradeApplied", "Upgraded idol.")
				: (Loc ? Loc->GetText_IdolAltarEquipped() : NSLOCTEXT("T66.IdolAltar", "Equipped", "Equipped.")));
	}
}

void UT66IdolAltarOverlayWidget::ClearPendingSelection()
{
	PendingSelection = FPendingSelection{};
	for (float& SelectionAlpha : OfferSelectionAlphas)
	{
		SelectionAlpha = 0.f;
	}
}

FReply UT66IdolAltarOverlayWidget::OnReroll()
{
	if (IsTutorialSingleOfferMode())
	{
		return FReply::Handled();
	}

	ActiveOfferCategoryIndex = (ActiveOfferCategoryIndex + 1) % OfferCategoryCount;
	RefreshStock();
	if (TSharedPtr<SWidget> Widget = AnimationActiveTimerWidget.Pin())
	{
		StartRevealAnimation(Widget.ToSharedRef());
	}
	return FReply::Handled();
}

FReply UT66IdolAltarOverlayWidget::OnBack()
{
	CommitPendingSelectionIfNeeded();

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
