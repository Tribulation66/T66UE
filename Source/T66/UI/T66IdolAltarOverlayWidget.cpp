// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66IdolAltarOverlayWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Data/T66DataTypes.h"
#include "Engine/Texture2D.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SNullWidget.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"

static UT66RunStateSubsystem* GetRunState(UWorld* World)
{
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}

void UT66IdolAltarOverlayWidget::NativeDestruct()
{
	// Unbind delegate.
	if (UWorld* World = GetWorld())
	{
		if (UT66RunStateSubsystem* RunState = GetRunState(World))
		{
			RunState->IdolsChanged.RemoveDynamic(this, &UT66IdolAltarOverlayWidget::HandleIdolsChanged);
		}
	}

	// Safety: restore gameplay input.
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

TSharedRef<SWidget> UT66IdolAltarOverlayWidget::RebuildWidget()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunState(World);
	UT66LocalizationSubsystem* Loc = nullptr;
	if (World)
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
		}
	}

	// Ensure stock is generated.
	if (RunState)
	{
		RunState->EnsureIdolStock();

		// Bind delegate (event-driven refresh).
		RunState->IdolsChanged.RemoveDynamic(this, &UT66IdolAltarOverlayWidget::HandleIdolsChanged);
		RunState->IdolsChanged.AddDynamic(this, &UT66IdolAltarOverlayWidget::HandleIdolsChanged);
	}

	const ISlateStyle& Style = FT66Style::Get();
	const FTextBlockStyle& TextTitle = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Title");
	const FTextBlockStyle& TextHeading = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading");
	const FTextBlockStyle& TextBody = Style.GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");

	const FText AltarTitle = Loc ? Loc->GetText_IdolAltarTitle() : NSLOCTEXT("T66.IdolAltar", "Title", "IDOL ALTAR");
	const FText RerollText = Loc ? Loc->GetText_Reroll() : NSLOCTEXT("T66.Vendor", "Reroll", "REROLL");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	// Initialize per-slot arrays.
	IdolNameTexts.SetNum(SlotCount);
	IdolDescTexts.SetNum(SlotCount);
	IdolIconImages.SetNum(SlotCount);
	IdolIconBrushes.SetNum(SlotCount);
	IdolTileBorders.SetNum(SlotCount);
	IdolIconBorders.SetNum(SlotCount);
	SelectButtons.SetNum(SlotCount);
	SelectButtonTexts.SetNum(SlotCount);

	for (int32 i = 0; i < SlotCount; ++i)
	{
		IdolIconBrushes[i] = MakeShared<FSlateBrush>();
		IdolIconBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		IdolIconBrushes[i]->ImageSize = FVector2D(200.f, 200.f);
	}

	// Build the 3 idol card row.
	TSharedRef<SHorizontalBox> IdolRow = SNew(SHorizontalBox);

	for (int32 i = 0; i < SlotCount; ++i)
	{
		const FText SelectLabel = Loc ? Loc->GetText_IdolAltarSelect() : NSLOCTEXT("T66.IdolAltar", "Select", "SELECT");

		TSharedRef<SWidget> SelectBtnWidget = FT66Style::MakeButton(
			FT66ButtonParams(
				SelectLabel,
				FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnSelectSlot, i),
				ET66ButtonType::Primary)
			.SetMinWidth(160.f)
			.SetPadding(FMargin(8.f, 6.f))
			.SetContent(
				SAssignNew(SelectButtonTexts[i], STextBlock)
				.Text(SelectLabel)
				.Font(FT66Style::Tokens::FontBold(14))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			)
		);
		SelectButtons[i] = SelectBtnWidget;

		IdolRow->AddSlot()
			.AutoWidth()
			.Padding(i > 0 ? FMargin(FT66Style::Tokens::Space6, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			SNew(SBox)
			.WidthOverride(220.f)
			.HeightOverride(420.f)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					// 1. Name at top
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(IdolNameTexts[i], STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&TextHeading)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					// 2. Large icon (centered)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)
						[
							FT66Style::MakePanel(
								SNew(SBox)
								.WidthOverride(200.f)
								.HeightOverride(200.f)
								[
									SAssignNew(IdolIconImages[i], SImage)
									.Image(IdolIconBrushes[i].Get())
									.ColorAndOpacity(FLinearColor::White)
								],
								FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f),
								&IdolIconBorders[i])
						]
					]
					// 3. Description (category + tooltip)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f)
					[
						SAssignNew(IdolDescTexts[i], STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&TextBody)
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
					// 4. Select button
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f)
					[
						SelectBtnWidget
					]
				,
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4),
					&IdolTileBorders[i])
			]
		];
	}

	TSharedRef<SWidget> Root =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			// Top row: Back (left), Title (center)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					FT66Style::MakeButton(
						FT66ButtonParams(BackText,
							FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnBack),
							ET66ButtonType::Neutral)
						.SetMinWidth(0.f)
						.SetPadding(FMargin(20.f, 12.f))
					)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(AltarTitle)
					.TextStyle(&TextTitle)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SSpacer)
				]
			]
			// Status text
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				SAssignNew(StatusText, STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
			// Idol cards (each card is its own panel, spaced apart)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, FT66Style::Tokens::Space6, 0.f, 0.f)
			[
				IdolRow
			]
			// Reroll button (centered beneath the idol panels, larger)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, FT66Style::Tokens::Space6, 0.f, 0.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(RerollText,
						FOnClicked::CreateUObject(this, &UT66IdolAltarOverlayWidget::OnReroll),
						ET66ButtonType::Neutral)
					.SetMinWidth(200.f)
					.SetPadding(FMargin(28.f, 14.f))
				)
			]
		,
			FT66PanelParams(ET66PanelType::Bg).SetPadding(24.f).SetColor(FT66Style::Tokens::Bg));

	RefreshStock();
	return Root;
}

void UT66IdolAltarOverlayWidget::RefreshStock()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunState(World);
	if (!RunState) return;

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	const TArray<FName>& Stock = RunState->GetIdolStockIDs();

	for (int32 i = 0; i < SlotCount; ++i)
	{
		const bool bHasItem = Stock.IsValidIndex(i) && !Stock[i].IsNone();
		const bool bSelected = RunState->IsIdolStockSlotSelected(i);

		FIdolData D;
		const bool bHasData = bHasItem && GI && GI->GetIdolData(Stock[i], D);

		// Name
		if (IdolNameTexts.IsValidIndex(i) && IdolNameTexts[i].IsValid())
		{
			IdolNameTexts[i]->SetText(bHasItem
				? (Loc ? Loc->GetText_IdolDisplayName(Stock[i]) : FText::FromName(Stock[i]))
				: NSLOCTEXT("T66.Common", "Empty", "EMPTY"));
		}

		// Description
		if (IdolDescTexts.IsValidIndex(i) && IdolDescTexts[i].IsValid())
		{
			if (bHasData && Loc)
			{
				const FText CatName = Loc->GetText_IdolCategoryName(D.Category);
				const FText Tooltip = Loc->GetText_IdolTooltip(Stock[i]);
				IdolDescTexts[i]->SetText(FText::Format(
					NSLOCTEXT("T66.IdolAltar", "IdolCardDescFormat", "{0}\n{1}"),
					CatName, Tooltip));
			}
			else
			{
				IdolDescTexts[i]->SetText(FText::GetEmpty());
			}
		}

		// Icon border color (idol tint)
		if (IdolIconBorders.IsValidIndex(i) && IdolIconBorders[i].IsValid())
		{
			const FLinearColor C = bHasItem ? UT66RunStateSubsystem::GetIdolColor(Stock[i]) : FT66Style::Tokens::Panel2;
			IdolIconBorders[i]->SetBorderBackgroundColor(C);
		}

		// Icon texture (async load)
		if (IdolIconBrushes.IsValidIndex(i) && IdolIconBrushes[i].IsValid())
		{
			if (bHasData && !D.Icon.IsNull() && TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, D.Icon, this, IdolIconBrushes[i], FName(TEXT("IdolStock"), i + 1), /*bClearWhileLoading*/ true);
			}
			else
			{
				IdolIconBrushes[i]->SetResourceObject(nullptr);
			}
		}
		if (IdolIconImages.IsValidIndex(i) && IdolIconImages[i].IsValid())
		{
			const bool bHasIcon = bHasData && !D.Icon.IsNull();
			IdolIconImages[i]->SetVisibility(bHasIcon ? EVisibility::Visible : EVisibility::Hidden);
		}

		// Tile border
		if (IdolTileBorders.IsValidIndex(i) && IdolTileBorders[i].IsValid())
		{
			IdolTileBorders[i]->SetBorderBackgroundColor(bSelected ? FT66Style::Tokens::Panel2 : FT66Style::Tokens::Panel);
		}

		// Select button
		if (SelectButtons.IsValidIndex(i) && SelectButtons[i].IsValid())
		{
			SelectButtons[i]->SetEnabled(bHasItem && !bSelected);
		}
		if (SelectButtonTexts.IsValidIndex(i) && SelectButtonTexts[i].IsValid())
		{
			if (bSelected)
			{
				SelectButtonTexts[i]->SetText(Loc ? Loc->GetText_IdolAltarSelected() : NSLOCTEXT("T66.IdolAltar", "Selected", "SELECTED"));
			}
			else
			{
				SelectButtonTexts[i]->SetText(Loc ? Loc->GetText_IdolAltarSelect() : NSLOCTEXT("T66.IdolAltar", "Select", "SELECT"));
			}
		}
	}
}

FReply UT66IdolAltarOverlayWidget::OnSelectSlot(int32 SlotIndex)
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = GetRunState(World);
	if (!RunState) return FReply::Handled();

	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	const bool bApplied = RunState->SelectIdolFromStock(SlotIndex);
	if (bApplied)
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(Loc ? Loc->GetText_IdolAltarEquipped() : NSLOCTEXT("T66.IdolAltar", "Equipped", "Equipped."));
		}
	}
	else
	{
		// Determine why it failed for a helpful message.
		const TArray<FName>& Stock = RunState->GetIdolStockIDs();
		const bool bAlreadySelected = RunState->IsIdolStockSlotSelected(SlotIndex);
		if (bAlreadySelected)
		{
			if (StatusText.IsValid())
			{
				StatusText->SetText(Loc ? Loc->GetText_IdolAltarAlreadyEquipped() : NSLOCTEXT("T66.IdolAltar", "AlreadyEquipped", "Already equipped."));
			}
		}
		else
		{
			// Check if it's a max-level or no-slot situation.
			if (StatusText.IsValid())
			{
				StatusText->SetText(Loc ? Loc->GetText_IdolAltarNoEmptySlot() : NSLOCTEXT("T66.IdolAltar", "NoEmptySlot", "No empty idol slot."));
			}
		}
	}

	// Update button labels and state immediately so SELECT → SELECTED is visible without waiting for the next tick or click.
	RefreshStock();
	return FReply::Handled();
}

FReply UT66IdolAltarOverlayWidget::OnReroll()
{
	if (UWorld* World = GetWorld())
	{
		if (UT66RunStateSubsystem* RunState = GetRunState(World))
		{
			RunState->RerollIdolStock();
		}
	}
	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::GetEmpty());
	}
	// RefreshStock is called automatically via IdolsChanged delegate.
	return FReply::Handled();
}

FReply UT66IdolAltarOverlayWidget::OnBack()
{
	// Unbind delegate before closing.
	if (UWorld* World = GetWorld())
	{
		if (UT66RunStateSubsystem* RunState = GetRunState(World))
		{
			RunState->IdolsChanged.RemoveDynamic(this, &UT66IdolAltarOverlayWidget::HandleIdolsChanged);
		}
	}

	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
	return FReply::Handled();
}
