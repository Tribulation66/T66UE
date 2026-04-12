// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66TemporaryBuffSelectionScreen.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Style/T66Style.h"
#include "UI/T66TemporaryBuffUIUtils.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

UT66TemporaryBuffSelectionScreen::UT66TemporaryBuffSelectionScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::TemporaryBuffSelection;
	bIsModal = true;
}

UT66LocalizationSubsystem* UT66TemporaryBuffSelectionScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	return nullptr;
}

UT66BuffSubsystem* UT66TemporaryBuffSelectionScreen::GetBuffSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66BuffSubsystem>();
	}

	return nullptr;
}

void UT66TemporaryBuffSelectionScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	ForceRebuildSlate();
}

TSharedRef<SWidget> UT66TemporaryBuffSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66BuffSubsystem* Buffs = GetBuffSubsystem();
	UT66UITexturePoolSubsystem* TexPool = nullptr;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
	}

	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const float ModalWidth = FMath::Min(SafeFrameSize.X * 0.90f, 1180.0f);
	const float ModalHeight = FMath::Min(SafeFrameSize.Y * 0.90f, 720.0f);
	const int32 Columns = 5;
	const float CardGap = 10.0f;
	const float CardWidth = (ModalWidth - 96.0f - CardGap * (Columns - 1)) / Columns;
	const int32 FocusedSlotIndex = Buffs ? Buffs->GetSelectedSingleUseBuffEditSlotIndex() : 0;
	const TArray<ET66SecondaryStatType> ActiveLoadoutSlots = Buffs ? Buffs->GetSelectedSingleUseBuffSlots() : TArray<ET66SecondaryStatType>{};

	const FText TitleText = NSLOCTEXT("T66.TempBuffs", "EditTitle", "SELECT TEMP BUFFS");
	const FText BuyMoreText = NSLOCTEXT("T66.TempBuffs", "BuyMore", "BUY MORE");
	const FText CloseText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText EmptyText = NSLOCTEXT("T66.TempBuffs", "EmptyOwned", "Pick a buff for the highlighted slot. You can buy missing buffs directly from a slot.");
	const FText SetSlotText = NSLOCTEXT("T66.TempBuffs", "SetSlot", "SET SLOT");
	const FText BuySlotText = NSLOCTEXT("T66.TempBuffs", "BuySlot", "BUY");
	const FText ClearSlotText = NSLOCTEXT("T66.TempBuffs", "ClearSlot", "CLEAR");
	const FText SlotHintText = NSLOCTEXT("T66.TempBuffs", "SlotHint", "Click a top slot, then pick a buff below.");
	const FText EmptySlotText = NSLOCTEXT("T66.TempBuffs", "EmptyPresetSlot", "+");

	BuffIconBrushes.Reset();

	TArray<TSharedPtr<FSlateBrush>> LoadoutSlotBrushes;
	LoadoutSlotBrushes.SetNum(UT66BuffSubsystem::MaxSelectedSingleUseBuffs);
	for (int32 SlotIndex = 0; SlotIndex < UT66BuffSubsystem::MaxSelectedSingleUseBuffs; ++SlotIndex)
	{
		const ET66SecondaryStatType SlotStat = ActiveLoadoutSlots.IsValidIndex(SlotIndex) ? ActiveLoadoutSlots[SlotIndex] : ET66SecondaryStatType::None;
		if (T66IsLiveSecondaryStatType(SlotStat))
		{
			LoadoutSlotBrushes[SlotIndex] = T66TemporaryBuffUI::CreateSecondaryBuffBrush(TexPool, this, SlotStat, FVector2D(42.f, 42.f));
			BuffIconBrushes.Add(LoadoutSlotBrushes[SlotIndex]);
		}
	}

	auto MakeLoadoutSlotWidget = [this, Buffs, FocusedSlotIndex, ActiveLoadoutSlots, LoadoutSlotBrushes, BuySlotText, ClearSlotText, EmptySlotText](int32 SlotIndex) -> TSharedRef<SWidget>
	{
		const ET66SecondaryStatType SlotStat = ActiveLoadoutSlots.IsValidIndex(SlotIndex) ? ActiveLoadoutSlots[SlotIndex] : ET66SecondaryStatType::None;
		const bool bFilled = T66IsLiveSecondaryStatType(SlotStat);
		const bool bOwnedForSlot = Buffs ? Buffs->IsSelectedSingleUseBuffSlotOwned(SlotIndex) : true;
		const bool bFocused = SlotIndex == FocusedSlotIndex;
		const FLinearColor ShellColor = bFocused ? FLinearColor(0.22f, 0.28f, 0.36f, 1.0f) : FT66Style::Tokens::Panel;
		const FLinearColor InnerColor = bFilled
			? (bOwnedForSlot ? FT66Style::Tokens::Panel2 : FLinearColor(0.20f, 0.10f, 0.10f, 1.0f))
			: FT66Style::Tokens::Panel2;

		const TSharedRef<SWidget> SlotContent = bFilled
			? StaticCastSharedRef<SWidget>(
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					[
						SNew(SImage)
					.Image(LoadoutSlotBrushes.IsValidIndex(SlotIndex) && LoadoutSlotBrushes[SlotIndex].IsValid() ? LoadoutSlotBrushes[SlotIndex].Get() : nullptr)
					.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, bOwnedForSlot ? 1.0f : 0.55f))
				])
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(EmptySlotText)
				.Font(FT66Style::Tokens::FontBold(20))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted));

		const TSharedRef<SWidget> SlotAction = !bFilled
			? StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.HeightOverride(22.f))
			: StaticCastSharedRef<SWidget>(
						FT66Style::MakeButton(
							FT66ButtonParams(
								bOwnedForSlot ? ClearSlotText : BuySlotText,
								bOwnedForSlot
									? FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotCleared, SlotIndex)
									: FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotPurchased, SlotIndex),
								bOwnedForSlot ? ET66ButtonType::Neutral : ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetHeight(22.f)
					.SetFontSize(9)
					.SetPadding(FMargin(6.f, 4.f, 6.f, 3.f))
					.SetColor(bOwnedForSlot ? FT66Style::Tokens::Panel2 : FT66Style::Tokens::Accent)));

		return FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				FT66Style::MakeButton(
					FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotClicked, SlotIndex), ET66ButtonType::Neutral)
					.SetMinWidth(0.f)
					.SetHeight(60.f)
					.SetPadding(FMargin(0.f))
					.SetColor(InnerColor)
					.SetContent(
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(InnerColor)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SlotContent
						]))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 6.f, 0.f, 0.f)
			[
				SlotAction
			],
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(ShellColor)
				.SetPadding(FMargin(4.f, 4.f, 4.f, 3.f)));
	};

	const TArray<ET66SecondaryStatType> AllBuffs = UT66BuffSubsystem::GetAllSingleUseBuffTypes();
	TSharedRef<SGridPanel> Grid = SNew(SGridPanel);
	for (int32 Index = 0; Index < AllBuffs.Num(); ++Index)
	{
		const ET66SecondaryStatType StatType = AllBuffs[Index];
		const int32 OwnedCount = Buffs ? Buffs->GetOwnedSingleUseBuffCount(StatType) : 0;
		const int32 AssignedCount = Buffs ? Buffs->GetSelectedSingleUseBuffSlotAssignedCountForStat(StatType) : 0;
		const bool bFocusedSlotMatches = ActiveLoadoutSlots.IsValidIndex(FocusedSlotIndex) && ActiveLoadoutSlots[FocusedSlotIndex] == StatType;
		const FText NameText = Loc ? Loc->GetText_SecondaryStatName(StatType) : FText::FromString(TEXT("?"));
		const FText DescText = Loc ? Loc->GetText_SecondaryStatDescription(StatType) : FText::GetEmpty();
		const FText OwnedCountText = FText::Format(NSLOCTEXT("T66.TempBuffs", "OwnedCount", "Owned: {0}"), FText::AsNumber(OwnedCount));
		const FText AssignedCountText = FText::Format(NSLOCTEXT("T66.TempBuffs", "AssignedCount", "Slotted: {0}"), FText::AsNumber(AssignedCount));

		TSharedPtr<FSlateBrush> Brush = T66TemporaryBuffUI::CreateSecondaryBuffBrush(TexPool, this, StatType, FVector2D(52.f, 52.f));
		BuffIconBrushes.Add(Brush);

		const TSharedRef<SWidget> IconWidget = Brush.IsValid()
			? StaticCastSharedRef<SWidget>(
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					SNew(SImage)
					.Image(Brush.Get())
				])
			: StaticCastSharedRef<SWidget>(SNew(SSpacer));

		const int32 Row = Index / Columns;
		const int32 Col = Index % Columns;
		Grid->AddSlot(Col, Row)
			.Padding(Col < Columns - 1 ? FMargin(0.f, 0.f, CardGap, CardGap) : FMargin(0.f, 0.f, 0.f, CardGap))
			[
				SNew(SBox)
				.WidthOverride(CardWidth)
				[
					FT66Style::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(0.f, 0.f, 0.f, 6.f)
						[
							SNew(SBox)
							.WidthOverride(52.f)
							.HeightOverride(52.f)
							[
								IconWidget
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(NameText)
							.Font(FT66Style::Tokens::FontBold(9))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						.Padding(0.f, 4.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(DescText)
							.Font(FT66Style::Tokens::FontRegular(8))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 8.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(OwnedCountText)
							.Font(FT66Style::Tokens::FontRegular(8))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(AssignedCountText)
							.Font(FT66Style::Tokens::FontRegular(8))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 8.f, 0.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(SetSlotText, FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleAssignBuffToFocusedLoadoutSlot, StatType), ET66ButtonType::Neutral)
								.SetMinWidth(0.f)
								.SetHeight(34.f)
								.SetFontSize(10)
								.SetColor(bFocusedSlotMatches ? FT66Style::Tokens::Accent : FT66Style::Tokens::Panel2))
						],
						FT66PanelParams(ET66PanelType::Panel)
							.SetColor(bFocusedSlotMatches ? FLinearColor(0.20f, 0.28f, 0.20f, 1.0f) : FT66Style::Tokens::Panel)
							.SetPadding(FMargin(8.f)))
				]
			];
	}

	const TSharedRef<SWidget> Content = AllBuffs.Num() > 0
		? StaticCastSharedRef<SWidget>(
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				Grid
			])
		: StaticCastSharedRef<SWidget>(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::Tokens::Panel)
			.Padding(FMargin(20.f))
			[
				SNew(STextBlock)
				.Text(EmptyText)
				.Font(FT66Style::Tokens::FontRegular(12))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
			]);

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::Scrim())
		[
			SNew(SBox)
			.WidthOverride(ModalWidth)
			.HeightOverride(ModalHeight)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Panel())
				.Padding(FMargin(24.f, 20.f))
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
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(TitleText)
								.Font(FT66Style::Tokens::FontBold(22))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f)
							[
								SNew(SSpacer)
								.Size(FVector2D(1.f, 1.f))
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(BuyMoreText, FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleBuyMoreClicked), ET66ButtonType::Primary)
								.SetMinWidth(160.f)
								.SetHeight(38.f)
								.SetFontSize(12))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 12.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(SlotHintText)
						.Font(FT66Style::Tokens::FontRegular(10))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 14.f, 0.f, 16.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[MakeLoadoutSlotWidget(0)]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[MakeLoadoutSlotWidget(1)]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[MakeLoadoutSlotWidget(2)]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[MakeLoadoutSlotWidget(3)]
						+ SHorizontalBox::Slot().AutoWidth()[MakeLoadoutSlotWidget(4)]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						Content
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 16.f, 0.f, 0.f)
					[
						FT66Style::MakeButton(
							FT66ButtonParams(CloseText, FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleCloseClicked), ET66ButtonType::Neutral)
							.SetMinWidth(140.f)
							.SetHeight(38.f)
							.SetFontSize(12))
					]
				]
			]
		];
}

FReply UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotClicked(int32 SlotIndex)
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		Buffs->SetSelectedSingleUseBuffEditSlotIndex(SlotIndex);
	}

	RefreshScreen();
	return FReply::Handled();
}

FReply UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotCleared(int32 SlotIndex)
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		if (Buffs->ClearSelectedSingleUseBuffSlot(SlotIndex))
		{
			RefreshScreen();
		}
	}

	return FReply::Handled();
}

FReply UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotPurchased(int32 SlotIndex)
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		if (Buffs->PurchaseSelectedSingleUseBuffSlot(SlotIndex))
		{
			RefreshScreen();
		}
	}

	return FReply::Handled();
}

FReply UT66TemporaryBuffSelectionScreen::HandleAssignBuffToFocusedLoadoutSlot(ET66SecondaryStatType StatType)
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		if (Buffs->SetSelectedSingleUseBuffSlot(Buffs->GetSelectedSingleUseBuffEditSlotIndex(), StatType))
		{
			const TArray<ET66SecondaryStatType> Slots = Buffs->GetSelectedSingleUseBuffSlots();
			for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
			{
				if (Slots[SlotIndex] == ET66SecondaryStatType::None)
				{
					Buffs->SetSelectedSingleUseBuffEditSlotIndex(SlotIndex);
					break;
				}
			}

			RefreshScreen();
		}
	}

	return FReply::Handled();
}

FReply UT66TemporaryBuffSelectionScreen::HandleBuyMoreClicked()
{
	ShowModal(ET66ScreenType::TemporaryBuffShop);
	return FReply::Handled();
}

FReply UT66TemporaryBuffSelectionScreen::HandleCloseClicked()
{
	CloseModal();
	return FReply::Handled();
}
