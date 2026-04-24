// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66WheelOverlayWidget.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RngSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/Style/T66Style.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
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
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	const FText Title = Loc ? Loc->GetText_WheelSpinTitle() : NSLOCTEXT("T66.Wheel", "Title", "WHEEL SPIN");
	const FText SpinTxt = Loc ? Loc->GetText_Spin() : NSLOCTEXT("T66.Wheel", "Spin", "SPIN");
	const FText BackTxt = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	// Visual: a wheel disk that we rotate while spinning.
	const FLinearColor WheelC = FT66RarityUtil::GetRarityColor(WheelRarity);

	TSharedRef<SWidget> Root = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.010f, 0.008f, 0.008f, 0.88f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.034f, 0.029f, 0.024f, 0.98f))
				.Padding(26.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(STextBlock)
						.Text(Title)
						.Font(FT66Style::Tokens::FontBold(28))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
					[
						SAssignNew(StatusText, STextBlock)
						.Text(Loc ? Loc->GetText_PressSpinToRollGold() : NSLOCTEXT("T66.Wheel", "PressSpinToRollGold", "Press SPIN to roll gold."))
						.Font(FT66Style::Tokens::FontRegular(14))
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
									.BorderBackgroundColor(FLinearColor(0.58f, 0.42f, 0.22f, 0.96f))
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
											.BorderBackgroundColor(FLinearColor(0.030f, 0.026f, 0.022f, 0.92f))
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
											.BorderBackgroundColor(FLinearColor(0.030f, 0.026f, 0.022f, 0.92f))
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
											.BorderBackgroundColor(FLinearColor(0.98f, 0.78f, 0.34f, 1.f))
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
											.BorderBackgroundColor(FLinearColor(0.060f, 0.048f, 0.030f, 1.f))
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
							SNew(SBox).MinDesiredWidth(180.f).HeightOverride(58.f)
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									SNew(SImage)
									.Visibility(EVisibility::HitTestInvisible)
									.Image(FT66Style::GetInRunButtonPlateBrush(ET66ButtonType::Primary))
								]
								+ SOverlay::Slot()
								[
									SAssignNew(SpinButton, SButton)
									.HAlign(HAlign_Center).VAlign(VAlign_Center)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66WheelOverlayWidget::OnSpin))
									.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.FlatTransparent"))
									.ButtonColorAndOpacity(FLinearColor::White)
									.ContentPadding(FMargin(12.f, 8.f))
									[
										SNew(STextBlock)
										.Text(SpinTxt)
										.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
										.ShadowOffset(FVector2D(1.f, 1.f))
										.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.92f))
									]
								]
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
						[
							SNew(SBox).MinDesiredWidth(180.f).HeightOverride(58.f)
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									SNew(SImage)
									.Visibility(EVisibility::HitTestInvisible)
									.Image(FT66Style::GetInRunButtonPlateBrush(ET66ButtonType::Neutral))
								]
								+ SOverlay::Slot()
								[
									SAssignNew(BackButton, SButton)
									.HAlign(HAlign_Center).VAlign(VAlign_Center)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66WheelOverlayWidget::OnBack))
									.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.FlatTransparent"))
									.ButtonColorAndOpacity(FLinearColor::White)
									.ContentPadding(FMargin(12.f, 8.f))
									[
										SNew(STextBlock)
										.Text(BackTxt)
										.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
										.ShadowOffset(FVector2D(1.f, 1.f))
										.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.92f))
									]
								]
							]
						]
					]
				]
			]
		];

	return FT66Style::MakeResponsiveRoot(Root);
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

	if (UWorld* ExistingWorld = GetWorld())
	{
		ExistingWorld->GetTimerManager().ClearTimer(SpinTickHandle);
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState && PendingGold > 0)
	{
		RunState->AddGold(PendingGold);
	}

	if (StatusText.IsValid())
	{
		UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
		const FText Fmt = Loc ? Loc->GetText_YouWonGoldFormat() : NSLOCTEXT("T66.Wheel", "YouWonGoldFormat", "You won {0} gold.");
		StatusText->SetText(FText::Format(Fmt, FText::AsNumber(PendingGold)));
	}
	if (SpinButton.IsValid()) SpinButton->SetEnabled(true);
	if (BackButton.IsValid()) BackButton->SetEnabled(true);

	// Close a moment later so the result is visible.
	if (UWorld* ExistingWorld = GetWorld())
	{
		ExistingWorld->GetTimerManager().SetTimer(CloseHandle, this, &UT66WheelOverlayWidget::CloseAfterResolve, 0.6f, false);
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
				RngSub->UpdateLuckStat(RunState->GetEffectiveLuckBiasStat());
			}

			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
			{
				if (UT66PlayerExperienceSubSystem* PlayerExperience = GI->GetSubsystem<UT66PlayerExperienceSubSystem>())
				{
					const FT66FloatRange Range = PlayerExperience->GetDifficultyWheelGoldRange(T66GI->SelectedDifficulty, WheelRarity);

					MinGold = FMath::FloorToInt(FMath::Min(Range.Min, Range.Max));
					MaxGold = FMath::CeilToInt(FMath::Max(Range.Min, Range.Max));
					bHasGoldRange = true;

					FRandomStream& Stream = RngSub->GetRunStream();
					NewPendingGold = FMath::Max(0, FMath::RoundToInt(RngSub->RollFloatRangeBiased(Range, Stream)));
					const int32 DrawIndex = RngSub->GetLastRunDrawIndex();
					const int32 PreDrawSeed = RngSub->GetLastRunPreDrawSeed();
					if (RunState)
					{
						RunState->RecordLuckQuantityFloatRollRounded(
							FName(TEXT("WheelGold")),
							NewPendingGold,
							MinGold,
							MaxGold,
							Range.Min,
							Range.Max,
							DrawIndex,
							PreDrawSeed);
					}
				}
			}
		}
	}
	PendingGold = NewPendingGold;

	if (StatusText.IsValid())
	{
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
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
	World->GetTimerManager().SetTimer(SpinTickHandle, this, &UT66WheelOverlayWidget::TickSpin, 0.033f, true);

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

