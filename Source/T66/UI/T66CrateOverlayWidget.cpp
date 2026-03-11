// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CrateOverlayWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RngTuningConfig.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/Style/T66Style.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UT66CrateOverlayWidget::GenerateStrip()
{
	StripItems.Empty();

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;

	if (RngSub && RunState)
	{
		RngSub->UpdateLuckStat(RunState->GetLuckStat());
	}

	FRandomStream Rng(static_cast<int32>(FPlatformTime::Cycles()));

	FT66RarityWeights CrateWeights;
	CrateWeights.Black = 0.70f;
	CrateWeights.Red = 0.20f;
	CrateWeights.Yellow = 0.08f;
	CrateWeights.White = 0.02f;

	const ET66Rarity WinRarity = (RngSub) ? RngSub->RollRarityWeighted(CrateWeights, Rng) : ET66Rarity::Black;
	WinnerItemID = T66GI ? T66GI->GetRandomItemIDForLootRarity(WinRarity) : FName(TEXT("Unknown"));

	for (int32 i = 0; i < StripItemCount; ++i)
	{
		FCrateItemEntry Entry;
		if (i == WinnerPosition)
		{
			Entry.ItemID = WinnerItemID;
			Entry.Rarity = WinRarity;
			WinnerIndex = i;
		}
		else
		{
			ET66Rarity DecoyRarity = ET66Rarity::Black;
			const float Roll = Rng.GetFraction();
			const float Total = CrateWeights.Black + CrateWeights.Red + CrateWeights.Yellow + CrateWeights.White;
			float Acc = CrateWeights.Black / Total;
			if (Roll < Acc) DecoyRarity = ET66Rarity::Black;
			else { Acc += CrateWeights.Red / Total; if (Roll < Acc) DecoyRarity = ET66Rarity::Red;
			else { Acc += CrateWeights.Yellow / Total; if (Roll < Acc) DecoyRarity = ET66Rarity::Yellow;
			else DecoyRarity = ET66Rarity::White; }}

			Entry.ItemID = T66GI ? T66GI->GetRandomItemIDForLootRarity(DecoyRarity) : FName(TEXT("Decoy"));
			Entry.Rarity = DecoyRarity;
		}
		Entry.Color = FT66RarityUtil::GetRarityColor(Entry.Rarity);
		StripItems.Add(Entry);
	}

	TotalScrollDistance = static_cast<float>(WinnerPosition - 2) * ItemTileWidth;
}

void UT66CrateOverlayWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScrollTickHandle);
		World->GetTimerManager().ClearTimer(ResolveHandle);
		World->GetTimerManager().ClearTimer(CloseHandle);
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

TSharedRef<SWidget> UT66CrateOverlayWidget::RebuildWidget()
{
	GenerateStrip();

	TSharedPtr<SHorizontalBox> StripRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < StripItems.Num(); ++i)
	{
		const FCrateItemEntry& Entry = StripItems[i];
		StripRow->AddSlot()
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(ItemTileWidth - 4.f)
				.HeightOverride(ItemTileWidth - 4.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(Entry.Color)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromName(Entry.ItemID))
						.Font(FT66Style::Tokens::FontRegular(9))
						.ColorAndOpacity(FLinearColor::White)
						.Justification(ETextJustify::Center)
					]
				]
			];
	}

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.9f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.f))
				.Padding(26.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Crate", "Title", "OPEN CRATE"))
						.Font(FT66Style::Tokens::FontBold(28))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
					[
						SAssignNew(StatusText, STextBlock)
						.Text(NSLOCTEXT("T66.Crate", "PressOpen", "Press OPEN to reveal an item."))
						.Font(FT66Style::Tokens::FontRegular(14))
						.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.f))
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 12.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SBox)
							.WidthOverride(ItemTileWidth * 5.f)
							.HeightOverride(ItemTileWidth + 10.f)
							.Clipping(EWidgetClipping::ClipToBounds)
							[
								SAssignNew(StripContainer, SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
								.Padding(0.f)
								[
									StripRow.ToSharedRef()
								]
							]
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Top)
						[
							SNew(SBox)
							.WidthOverride(4.f)
							.HeightOverride(ItemTileWidth + 10.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(1.f, 1.f, 1.f, 0.8f))
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 14.f, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
						[
							SNew(SBox).MinDesiredWidth(180.f).HeightOverride(44.f)
							[
								SAssignNew(OpenButton, SButton)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66CrateOverlayWidget::OnOpen))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Success)
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("T66.Crate", "Open", "OPEN"))
									.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
								]
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
						[
							SNew(SBox).MinDesiredWidth(180.f).HeightOverride(44.f)
							[
								SAssignNew(BackButton, SButton)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66CrateOverlayWidget::OnBack))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("T66.Crate", "Back", "BACK"))
									.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
								]
							]
						]
					]
				]
			]
		];
}

void UT66CrateOverlayWidget::TickScroll()
{
	if (!bScrolling || !StripContainer.IsValid()) return;
	UWorld* World = GetWorld();
	if (!World) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	float Delta = Now - LastTickTimeSeconds;
	LastTickTimeSeconds = Now;
	Delta = FMath::Clamp(Delta, 0.f, 0.05f);
	ScrollElapsed += Delta;

	const float Alpha = FMath::Clamp(ScrollElapsed / FMath::Max(0.01f, ScrollDuration), 0.f, 1.f);
	const float Ease = FMath::InterpEaseOut(0.f, 1.f, Alpha, 3.f);
	CurrentScrollOffset = TotalScrollDistance * Ease;

	const FMargin Margin(-CurrentScrollOffset, 0.f, 0.f, 0.f);
	StripContainer->SetPadding(Margin);

	if (Alpha >= 1.f)
	{
		ResolveOpen();
	}
}

void UT66CrateOverlayWidget::ResolveOpen()
{
	if (!bScrolling) return;
	bScrolling = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScrollTickHandle);
	}

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState && !WinnerItemID.IsNone())
	{
		RunState->AddItem(WinnerItemID);
	}

	if (StatusText.IsValid())
	{
		const FText Msg = FText::Format(
			NSLOCTEXT("T66.Crate", "YouGotFormat", "You received: {0}!"),
			FText::FromName(WinnerItemID));
		StatusText->SetText(Msg);
	}
	if (OpenButton.IsValid()) OpenButton->SetEnabled(true);
	if (BackButton.IsValid()) BackButton->SetEnabled(true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CloseHandle, this, &UT66CrateOverlayWidget::CloseAfterResolve, 1.2f, false);
	}
}

void UT66CrateOverlayWidget::CloseAfterResolve()
{
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
}

FReply UT66CrateOverlayWidget::OnOpen()
{
	if (bScrolling) return FReply::Handled();
	UWorld* World = GetWorld();
	if (!World) return FReply::Handled();

	if (StatusText.IsValid())
	{
		StatusText->SetText(NSLOCTEXT("T66.Crate", "Opening", "Opening..."));
	}

	bScrolling = true;
	ScrollElapsed = 0.f;
	CurrentScrollOffset = 0.f;
	LastTickTimeSeconds = static_cast<float>(World->GetTimeSeconds());

	if (OpenButton.IsValid()) OpenButton->SetEnabled(false);
	if (BackButton.IsValid()) BackButton->SetEnabled(false);

	World->GetTimerManager().ClearTimer(ScrollTickHandle);
	World->GetTimerManager().SetTimer(ScrollTickHandle, this, &UT66CrateOverlayWidget::TickScroll, 0.016f, true);

	return FReply::Handled();
}

FReply UT66CrateOverlayWidget::OnBack()
{
	if (bScrolling) return FReply::Handled();
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
	return FReply::Handled();
}
