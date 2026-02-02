// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66WheelOverlayWidget.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RngSubsystem.h"
#include "Gameplay/T66PlayerController.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Styling/CoreStyle.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/TransformCalculus2D.h"
#include "Rendering/SlateRenderTransform.h"

static int32 RollWheelGold(FRandomStream& Rng, ET66Rarity R)
{
	// Simple v0 payout tables (tunable later).
	switch (R)
	{
	case ET66Rarity::Black:
	{
		static const int32 V[] = { 25, 50, 75, 100 };
		return V[Rng.RandRange(0, 3)];
	}
	case ET66Rarity::Red:
	{
		static const int32 V[] = { 100, 150, 200, 300 };
		return V[Rng.RandRange(0, 3)];
	}
	case ET66Rarity::Yellow:
	{
		static const int32 V[] = { 200, 300, 400, 600 };
		return V[Rng.RandRange(0, 3)];
	}
	case ET66Rarity::White:
	{
		static const int32 V[] = { 500, 750, 1000, 1500 };
		return V[Rng.RandRange(0, 3)];
	}
	default:
		return 50;
	}
}

void UT66WheelOverlayWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpinTickHandle);
		World->GetTimerManager().ClearTimer(ResolveHandle);
		World->GetTimerManager().ClearTimer(CloseHandle);
	}

	// Safety: overlays can be destroyed outside "Back" path (level change, etc).
	// If we leave the player in UI input mode, gameplay can feel frozen.
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && !PC->IsPaused())
		{
			PC->RestoreGameplayInputMode();
		}
	}
	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66WheelOverlayWidget::RebuildWidget()
{
	UT66LocalizationSubsystem* Loc = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	const FText Title = Loc ? Loc->GetText_WheelSpinTitle() : NSLOCTEXT("T66.Wheel", "Title", "WHEEL SPIN");
	const FText SpinTxt = Loc ? Loc->GetText_Spin() : NSLOCTEXT("T66.Wheel", "Spin", "SPIN");
	const FText BackTxt = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	// Visual: a wheel disk that we rotate while spinning.
	const FLinearColor WheelC = FT66RarityUtil::GetRarityColor(WheelRarity);

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
						.Text(Title)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
					[
						SAssignNew(StatusText, STextBlock)
						.Text(Loc ? Loc->GetText_PressSpinToRollGold() : NSLOCTEXT("T66.Wheel", "PressSpinToRollGold", "Press SPIN to roll gold."))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
						.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.f))
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 12.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SBox)
							.WidthOverride(220.f)
							.HeightOverride(220.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.14f, 1.f))
							]
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(170.f)
							.HeightOverride(170.f)
							[
								SAssignNew(WheelDisk, SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(WheelC)
								.Padding(0.f)
								[
									SNew(SOverlay)
									// Spokes
									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(SBox)
										.WidthOverride(150.f)
										.HeightOverride(10.f)
										[
											SNew(SBorder)
											.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
											.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.10f, 0.9f))
										]
									]
									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(SBox)
										.WidthOverride(10.f)
										.HeightOverride(150.f)
										[
											SNew(SBorder)
											.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
											.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.10f, 0.9f))
										]
									]
									// Marker so rotation is obvious
									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Top)
									.Padding(0.f, 8.f, 0.f, 0.f)
									[
										SNew(SBox)
										.WidthOverride(18.f)
										.HeightOverride(18.f)
										[
											SNew(SBorder)
											.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
											.BorderBackgroundColor(FLinearColor(0.95f, 0.95f, 0.98f, 1.f))
										]
									]
									// Hub
									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(SBox)
										.WidthOverride(22.f)
										.HeightOverride(22.f)
										[
											SNew(SBorder)
											.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
											.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 1.f))
										]
									]
								]
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 14.f, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
						[
							SNew(SBox).WidthOverride(180.f).HeightOverride(44.f)
							[
								SAssignNew(SpinButton, SButton)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66WheelOverlayWidget::OnSpin))
								.ButtonColorAndOpacity(FLinearColor(0.25f, 0.55f, 0.25f, 1.f))
								[
									SNew(STextBlock)
									.Text(SpinTxt)
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
						[
							SNew(SBox).WidthOverride(180.f).HeightOverride(44.f)
							[
								SAssignNew(BackButton, SButton)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66WheelOverlayWidget::OnBack))
								.ButtonColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.35f, 1.f))
								[
									SNew(STextBlock)
									.Text(BackTxt)
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
					]
				]
			]
		];
}

void UT66WheelOverlayWidget::TickSpin()
{
	if (!bSpinning || !WheelDisk.IsValid()) return;
	UWorld* World = GetWorld();
	if (!World) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	float Delta = Now - LastSpinTickTimeSeconds;
	LastSpinTickTimeSeconds = Now;
	// Clamp so a hitch doesn't jump straight to resolve.
	Delta = FMath::Clamp(Delta, 0.f, 0.05f);
	SpinElapsed += Delta;
	const float Alpha = FMath::Clamp(SpinElapsed / FMath::Max(0.01f, SpinDuration), 0.f, 1.f);
	const float Ease = FMath::InterpEaseOut(0.f, 1.f, Alpha, 3.f);
	const float Angle = StartAngleDeg + (TotalAngleDeg * Ease);

	WheelDisk->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	WheelDisk->SetRenderTransform(FSlateRenderTransform(FTransform2D(FQuat2D(FMath::DegreesToRadians(Angle)))));

	if (Alpha >= 1.f)
	{
		ResolveSpin();
	}
}

void UT66WheelOverlayWidget::ResolveSpin()
{
	if (!bSpinning) return;
	bSpinning = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpinTickHandle);
	}

	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState && PendingGold > 0)
	{
		RunState->AddGold(PendingGold);
	}

	if (StatusText.IsValid())
	{
		UT66LocalizationSubsystem* Loc = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
		const FText Fmt = Loc ? Loc->GetText_YouWonGoldFormat() : NSLOCTEXT("T66.Wheel", "YouWonGoldFormat", "You won {0} gold.");
		StatusText->SetText(FText::Format(Fmt, FText::AsNumber(PendingGold)));
	}
	if (SpinButton.IsValid()) SpinButton->SetEnabled(true);
	if (BackButton.IsValid()) BackButton->SetEnabled(true);

	// Close a moment later so the result is visible.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CloseHandle, this, &UT66WheelOverlayWidget::CloseAfterResolve, 0.6f, false);
	}
}

void UT66WheelOverlayWidget::CloseAfterResolve()
{
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
}

FReply UT66WheelOverlayWidget::OnSpin()
{
	if (bSpinning) return FReply::Handled();
	UWorld* World = GetWorld();
	if (!World) return FReply::Handled();

	FRandomStream SpinRng(static_cast<int32>(FPlatformTime::Cycles())); // visual-only randomness (not luck-affected)

	int32 NewPendingGold = 50;
	int32 MinGold = 0;
	int32 MaxGold = 0;
	bool bHasGoldRange = false;
	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UT66RngSubsystem* RngSub = GI->GetSubsystem<UT66RngSubsystem>())
		{
			UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			if (RunState)
			{
				RngSub->UpdateLuckStat(RunState->GetLuckStat());
			}

			const UT66RngTuningConfig* Tuning = RngSub->GetTuning();
			if (Tuning)
			{
				const FT66FloatRange Range =
					(WheelRarity == ET66Rarity::Black) ? Tuning->WheelGoldRange_Black :
					(WheelRarity == ET66Rarity::Red) ? Tuning->WheelGoldRange_Red :
					(WheelRarity == ET66Rarity::Yellow) ? Tuning->WheelGoldRange_Yellow :
					(WheelRarity == ET66Rarity::White) ? Tuning->WheelGoldRange_White :
					Tuning->WheelGoldRange_Black;

				MinGold = FMath::FloorToInt(FMath::Min(Range.Min, Range.Max));
				MaxGold = FMath::CeilToInt(FMath::Max(Range.Min, Range.Max));
				bHasGoldRange = true;

				FRandomStream& Stream = RngSub->GetRunStream();
				NewPendingGold = FMath::Max(0, FMath::RoundToInt(RngSub->RollFloatRangeBiased(Range, Stream)));
			}

			if (RunState && bHasGoldRange)
			{
				RunState->RecordLuckQuantityRoll(FName(TEXT("WheelGold")), NewPendingGold, MinGold, MaxGold);
			}
		}
	}
	PendingGold = NewPendingGold;

	if (StatusText.IsValid())
	{
		UT66LocalizationSubsystem* Loc = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
		StatusText->SetText(Loc ? Loc->GetText_Spinning() : NSLOCTEXT("T66.Wheel", "Spinning", "Spinning..."));
	}

	// Start spin animation.
	bSpinning = true;
	SpinElapsed = 0.f;
	LastSpinTickTimeSeconds = static_cast<float>(World->GetTimeSeconds());
	StartAngleDeg = 0.f;
	TotalAngleDeg = static_cast<float>(SpinRng.RandRange(5, 9)) * 360.f + static_cast<float>(SpinRng.RandRange(0, 359));

	if (SpinButton.IsValid()) SpinButton->SetEnabled(false);
	if (BackButton.IsValid()) BackButton->SetEnabled(false);

	World->GetTimerManager().ClearTimer(SpinTickHandle);
	World->GetTimerManager().SetTimer(SpinTickHandle, this, &UT66WheelOverlayWidget::TickSpin, 0.016f, true);

	return FReply::Handled();
}

FReply UT66WheelOverlayWidget::OnBack()
{
	if (bSpinning) return FReply::Handled();
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
	return FReply::Handled();
}

