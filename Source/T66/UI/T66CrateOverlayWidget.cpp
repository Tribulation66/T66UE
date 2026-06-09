// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CrateOverlayWidget.h"
#include "Core/T66GameInstance.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/Animation/T66LootUIAnimation.h"
#include "UI/T66GameplayHUDWidget.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66Style.h"

#include "Engine/World.h"
#include "Styling/CoreStyle.h"
#include "Types/WidgetActiveTimerDelegate.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	static const FName CrateTickMarker(TEXT("Interact.Crate.Tick"));
	static const FName CrateSlowdownTickMarker(TEXT("Interact.Crate.SlowdownTick"));
	static const FName CrateLandingMarker(TEXT("Interact.Crate.Landing"));
	static const FName CrateRarityRevealMarker(TEXT("Interact.Crate.RarityReveal"));
	static const FName CrateInventoryCommitMarker(TEXT("Crate.InventoryCommit"));

	static ET66ItemRarity LootRarityToItemRarity(ET66Rarity Rarity)
	{
		switch (Rarity)
		{
		case ET66Rarity::Black:  return ET66ItemRarity::Black;
		case ET66Rarity::Red:    return ET66ItemRarity::Red;
		case ET66Rarity::Yellow: return ET66ItemRarity::Yellow;
		case ET66Rarity::White:  return ET66ItemRarity::White;
		default:                 return ET66ItemRarity::Black;
		}
	}

	static FT66RarityWeights ApplyLootCrateBias(const FT66RarityWeights& BaseWeights, float LootCrateMultiplier)
	{
		const float Bias = FMath::Clamp(LootCrateMultiplier, 1.f, 3.f);

		FT66RarityWeights Result;
		Result.Black = BaseWeights.Black / Bias;
		Result.Red = BaseWeights.Red * FMath::Sqrt(Bias);
		Result.Yellow = BaseWeights.Yellow * Bias;
		Result.White = BaseWeights.White * Bias * Bias;
		return Result;
	}

	static FT66RarityWeights ApplyInteractableTierBias(const FT66RarityWeights& BaseWeights, ET66Rarity SourceRarity)
	{
		FT66RarityWeights Result = BaseWeights;
		switch (SourceRarity)
		{
		case ET66Rarity::Red:
			Result.Black *= 0.85f;
			Result.Red *= 1.15f;
			Result.Yellow *= 1.25f;
			Result.White *= 1.35f;
			break;
		case ET66Rarity::Yellow:
			Result.Black *= 0.65f;
			Result.Red *= 1.20f;
			Result.Yellow *= 1.65f;
			Result.White *= 2.10f;
			break;
		case ET66Rarity::White:
			Result.Black *= 0.45f;
			Result.Red *= 1.10f;
			Result.Yellow *= 2.20f;
			Result.White *= 3.50f;
			break;
		case ET66Rarity::Black:
		default:
			break;
		}
		return Result;
	}

	static FText BuildSkipCountdownText(float RemainingSeconds)
	{
		return FText::Format(
			NSLOCTEXT("T66.Crate", "SkipCountdown", "Skip {0}s"),
			FText::FromString(FString::Printf(TEXT("%.1f"), FMath::Max(0.f, RemainingSeconds))));
	}

}

UT66CrateOverlayWidget::UT66CrateOverlayWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT66CrateOverlayWidget::SetPresentationHost(UT66GameplayHUDWidget* InPresentationHost)
{
	PresentationHost = InPresentationHost;
}

void UT66CrateOverlayWidget::SetSourceCrateRarity(const ET66Rarity InSourceCrateRarity)
{
	SourceCrateRarity = InSourceCrateRarity;
}

bool UT66CrateOverlayWidget::CommitImmediateCrateReward(UWorld* World, const ET66Rarity SourceCrateRarity)
{
	if (!World)
	{
		return false;
	}

	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	if (!T66GI || !RunState)
	{
		return false;
	}

	if (RngSub)
	{
		RngSub->UpdateLuckStat(RunState->GetEffectiveLuckBiasStat());
	}

	FRandomStream LocalRng(static_cast<int32>(FPlatformTime::Cycles()));
	FRandomStream& Rng = RngSub ? RngSub->GetRunStream() : LocalRng;
	const ET66Difficulty Difficulty = T66GI->SelectedDifficulty;
	FT66RarityWeights CrateWeights = PlayerExperience
		? PlayerExperience->GetDifficultyCrateRarityWeights(Difficulty)
		: FT66RarityWeights{};
	CrateWeights = ApplyInteractableTierBias(CrateWeights, SourceCrateRarity);
	CrateWeights = ApplyLootCrateBias(CrateWeights, RunState->GetLootCrateRewardMultiplier());

	const ET66Rarity WinRarity = RngSub ? RngSub->RollRarityWeighted(CrateWeights, Rng) : ET66Rarity::Black;
	const int32 RarityDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
	const int32 RarityPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
	const FName ItemID = T66GI->GetRandomItemIDForLootRarityFromStream(WinRarity, Rng);
	if (ItemID.IsNone())
	{
		return false;
	}

	const int32 InventoryCountBefore = RunState->GetInventorySlots().Num();
	RunState->RecordLuckQualityRarity(
		FName(TEXT("CrateRewardRarity")),
		WinRarity,
		RarityDrawIndex,
		RarityPreDrawSeed,
		&CrateWeights);
	RunState->AddItemWithRarity(ItemID, LootRarityToItemRarity(WinRarity));
	return RunState->GetInventorySlots().Num() > InventoryCountBefore;
}

void UT66CrateOverlayWidget::RequestSkip()
{
	if (bCompletionSignaled || !RewardResult.bLocked)
	{
		return;
	}

	if (bSkipSequenceActive)
	{
		SignalAnimationComplete();
		return;
	}

	switch (ActivePhase)
	{
	case ECrateAnimationPhase::Reveal:
	case ECrateAnimationPhase::Hold:
		CommitRewardIfNeeded();
		BuildDismissSequence();
		break;
	case ECrateAnimationPhase::Overshoot:
	case ECrateAnimationPhase::Settle:
		BuildSettleRevealSequence(SkipSettleDuration);
		break;
	case ECrateAnimationPhase::Dismiss:
		SignalAnimationComplete();
		break;
	case ECrateAnimationPhase::Complete:
		break;
	case ECrateAnimationPhase::Idle:
	case ECrateAnimationPhase::Anticipation:
	case ECrateAnimationPhase::Spin:
	case ECrateAnimationPhase::Deceleration:
	default:
		BuildSkipToSettleSequence();
		break;
	}
}

void UT66CrateOverlayWidget::GenerateStrip()
{
	StripItems.Empty();
	RewardResult = FT66CrateRewardResult();
	StripState = FT66CrateStripPresentationState();
	WinnerIndex = WinnerPosition;

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	if (RngSub && RunState)
	{
		RngSub->UpdateLuckStat(RunState->GetEffectiveLuckBiasStat());
	}

	FRandomStream LocalRng(static_cast<int32>(FPlatformTime::Cycles()));
	FRandomStream& Rng = RngSub ? RngSub->GetRunStream() : LocalRng;

	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	FT66RarityWeights CrateWeights = PlayerExperience
		? PlayerExperience->GetDifficultyCrateRarityWeights(Difficulty)
		: FT66RarityWeights{};
	CrateWeights = ApplyInteractableTierBias(CrateWeights, SourceCrateRarity);
	if (RunState)
	{
		CrateWeights = ApplyLootCrateBias(CrateWeights, RunState->GetLootCrateRewardMultiplier());
	}

	const ET66Rarity WinRarity = RngSub ? RngSub->RollRarityWeighted(CrateWeights, Rng) : ET66Rarity::Black;
	RewardResult.Rarity = WinRarity;
	RewardResult.RarityDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
	RewardResult.RarityPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
	RewardResult.bRarityHasReplayWeights = true;
	RewardResult.RarityReplayWeights = CrateWeights;
	RewardResult.ItemID = T66GI ? T66GI->GetRandomItemIDForLootRarityFromStream(WinRarity, Rng) : FName(TEXT("Unknown"));
	RewardResult.bLocked = true;

	for (int32 i = 0; i < StripItemCount; ++i)
	{
		FCrateItemEntry Entry;
		if (i == WinnerPosition)
		{
			Entry.ItemID = RewardResult.ItemID;
			Entry.Rarity = WinRarity;
			WinnerIndex = i;
		}
		else
		{
			ET66Rarity DecoyRarity = ET66Rarity::Black;
			const float Total = CrateWeights.Black + CrateWeights.Red + CrateWeights.Yellow + CrateWeights.White;
			if (Total > UE_SMALL_NUMBER)
			{
				const float Roll = Rng.GetFraction();
				float Acc = CrateWeights.Black / Total;
				if (Roll < Acc)
				{
					DecoyRarity = ET66Rarity::Black;
				}
				else
				{
					Acc += CrateWeights.Red / Total;
					if (Roll < Acc)
					{
						DecoyRarity = ET66Rarity::Red;
					}
					else
					{
						Acc += CrateWeights.Yellow / Total;
						DecoyRarity = (Roll < Acc) ? ET66Rarity::Yellow : ET66Rarity::White;
					}
				}
			}

			Entry.ItemID = T66GI ? T66GI->GetRandomItemIDForLootRarityFromStream(DecoyRarity, Rng) : FName(TEXT("Decoy"));
			Entry.Rarity = DecoyRarity;
		}

		Entry.Color = FT66RarityUtil::GetRarityColor(Entry.Rarity);
		Entry.LayoutCenterX = (static_cast<float>(i) * ItemTileStride) + (ItemTileGap * 0.5f) + (ItemTileWidth * 0.5f);
		Entry.IconBrush = MakeShared<FSlateBrush>();
		Entry.IconBrush->DrawAs = ESlateBrushDrawType::Image;
		Entry.IconBrush->ImageSize = FVector2D(ItemPreviewSize, ItemPreviewSize);
		if (T66GI && TexPool)
		{
			FItemData ItemData;
			if (T66GI->GetItemData(Entry.ItemID, ItemData))
			{
				const TSoftObjectPtr<UTexture2D> IconSoft = ItemData.GetIconForRarity(LootRarityToItemRarity(Entry.Rarity));
				if (!IconSoft.IsNull())
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, IconSoft, this, Entry.IconBrush, FName(TEXT("CrateStrip"), i + 1), true);
				}
			}
		}
		StripItems.Add(Entry);
	}

	StripState.StripTotalWidth = static_cast<float>(StripItemCount) * ItemTileStride;
	const float WinnerCenterX = StripItems.IsValidIndex(WinnerIndex) ? StripItems[WinnerIndex].LayoutCenterX : 0.f;

	FT66ScrollingStripMotionInput MotionInput;
	MotionInput.ViewportWidth = CrateStripViewportWidth;
	MotionInput.StripTotalWidth = StripState.StripTotalWidth;
	MotionInput.WinnerCenterX = WinnerCenterX;
	MotionInput.ItemStride = ItemTileStride;
	MotionInput.WinnerIndex = WinnerIndex;
	MotionInput.Tuning.FastTravelTileCount = FastTravelTileCount;
	MotionInput.Tuning.OvershootTileFraction = OvershootTileFraction;
	MotionInput.Tuning.SlowdownTickTileCount = SlowdownTickTileCount;
	MotionInput.Tuning.SkipTravelTileCount = SkipTravelTileCount;

	const FT66ScrollingStripMotionPlan MotionPlan = T66LootUIAnimation::BuildScrollingStripMotionPlan(MotionInput);
	StripState.SelectorPositionX = MotionPlan.SelectorPositionX;
	StripState.FinalStripOffset = MotionPlan.FinalStripOffset;
	StripState.SpinEndOffset = MotionPlan.FastTravelEndOffset;
	StripState.OvershootOffset = MotionPlan.OvershootOffset;
	StripState.MaxVisibleOffset = MotionPlan.MaxVisibleOffset;
	StripState.RightContentAfterWinner = MotionPlan.RightContentAfterWinner;
	StripState.SlowdownStartIndex = MotionPlan.SlowdownStartIndex;
	StripState.bHasRightContentForFinalView = MotionPlan.bHasRightContentForFinalView;
	StripState.CurrentStripOffset = 0.f;
}

void UT66CrateOverlayWidget::NativeDestruct()
{
	StopAnimationActiveTimer();
	bAnimationActive = false;
	MarkerDispatcher.ClearHandlers();

	if (!bCompletionSignaled)
	{
		CommitRewardIfNeeded();
		if (UT66GameplayHUDWidget* Host = PresentationHost.Get())
		{
			Host->ClearActiveCratePresentation(this);
		}
	}

	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66CrateOverlayWidget::RebuildWidget()
{
	GenerateStrip();
	ResetVisualState();

	TSharedPtr<SHorizontalBox> StripRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < StripItems.Num(); ++i)
	{
		FCrateItemEntry& Entry = StripItems[i];
		const FLinearColor FrameColor = Entry.Color * 0.48f + FLinearColor(0.18f, 0.13f, 0.07f, 0.52f);

		TSharedPtr<SBox> TileBox;
		TSharedPtr<SBorder> GlowBorder;
		TSharedPtr<SBorder> FrameBorder;
		TSharedPtr<SImage> IconImage;

		StripRow->AddSlot()
			.AutoWidth()
			.Padding(ItemTileGap * 0.5f, 0.f)
			[
				SAssignNew(TileBox, SBox)
				.WidthOverride(ItemTileWidth)
				.HeightOverride(ItemTileHeight)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(GlowBorder, SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor::Transparent)
						.Padding(0.f)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Padding(2.f)
					[
						FT66FlatStyle::MakeFlatPanel(
							i == WinnerPosition ? ET66FlatState::Selected : ET66FlatState::Default,
							FMargin(6.f),
							SAssignNew(FrameBorder, SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
							.BorderBackgroundColor(FrameColor)
							.Padding(FMargin(3.f))
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SScaleBox)
									.Stretch(EStretch::ScaleToFit)
									.StretchDirection(EStretchDirection::Both)
									[
										SNew(SBox)
										.WidthOverride(ItemPreviewSize)
										.HeightOverride(ItemPreviewSize)
										[
											FT66FlatStyle::AttachMetadata(
												SAssignNew(IconImage, SImage)
												.Image(Entry.IconBrush.IsValid() ? Entry.IconBrush.Get() : nullptr)
												.ColorAndOpacity(FLinearColor::White),
												FName(*FString::Printf(TEXT("CrateOverlay.ItemIcon.%02d"), i + 1)),
												TEXT("Icon"),
												ET66FlatState::Default)
										]
									]
								]
								+ SOverlay::Slot()
								.HAlign(HAlign_Left)
								.VAlign(VAlign_Top)
								[
									SNew(SBox)
									.WidthOverride(10.f)
									.HeightOverride(10.f)
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(Entry.Color)
									]
								]
							],
							nullptr,
							FName(*FString::Printf(TEXT("CrateOverlay.ItemSlot.%02d"), i + 1)))
					]
				]
			];

		Entry.TileBox = TileBox;
		Entry.GlowBorder = GlowBorder;
		Entry.FrameBorder = FrameBorder;
		Entry.IconImage = IconImage;
	}

	TSharedRef<SWidget> Root = FT66FlatStyle::AttachMetadata(
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(
			0.f,
			0.f,
			UT66GameplayHUDWidget::BottomRightRewardLaneRightPadding,
			UT66GameplayHUDWidget::BottomRightRewardLaneBottomPadding))
		[
			SNew(SBox)
			.WidthOverride(UT66GameplayHUDWidget::BottomRightRewardLaneWidth)
			.HeightOverride(UT66GameplayHUDWidget::BottomRightCrateRewardLaneHeight)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::DownOnly)
				[
					SAssignNew(RootAnimationBox, SBox)
					.WidthOverride(CratePanelWidth)
					.HeightOverride(CratePanelHeight)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(CrateStripViewportWidth)
								.HeightOverride(ItemTileHeight + 4.f)
								.Clipping(EWidgetClipping::ClipToBounds)
								[
									FT66FlatStyle::AttachMetadata(
										SAssignNew(StripContainer, SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
										.Padding(0.f)
										[
											StripRow.ToSharedRef()
										],
										FName(TEXT("CrateOverlay.StripContainer")),
										TEXT("ScrollingStrip"),
										ET66FlatState::Default)
								]
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SAssignNew(SelectorFlareBox, SBox)
								.WidthOverride(9.f)
								.HeightOverride(ItemTileHeight + 24.f)
								[
									SAssignNew(SelectorFlareBorder, SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor::Transparent)
								]
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Top)
							[
								SNew(SBox)
								.WidthOverride(3.f)
								.HeightOverride(ItemTileHeight + 8.f)
								[
									FT66FlatStyle::AttachMetadata(
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(FT66FlatStyle::SelectedBorder()),
										FName(TEXT("CrateOverlay.WinnerNeedle")),
										TEXT("SelectionNeedle"),
										ET66FlatState::Selected,
										FT66FlatStyle::SelectedBorder())
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 2.f, 0.f, 0.f)
						[
							FT66FlatStyle::AttachMetadata(
								SAssignNew(SkipText, STextBlock)
								.Text(BuildSkipCountdownText(AnticipationDuration + SpinDuration + DecelerationDuration + OvershootDuration + SettleDuration + RevealDuration + HoldDuration + DismissDuration))
								.Font(FT66FlatStyle::MakeFont(8))
								.ColorAndOpacity(FT66FlatStyle::SecondaryText())
								.Justification(ETextJustify::Center),
								FName(TEXT("CrateOverlay.SkipText")),
								TEXT("Label.Caption"),
								ET66FlatState::Default,
								TOptional<FLinearColor>(),
								false,
								NAME_None,
								true)
						]
					]
				]
			]
		],
		FName(TEXT("CrateOverlay.Root")),
		TEXT("Overlay"),
		ET66FlatState::Default);

	BuildFullAnimationSequence();
	if (RootAnimationBox.IsValid())
	{
		StartAnimationActiveTimer(RootAnimationBox.ToSharedRef());
	}
	ApplyStripOffset(0.f);
	ApplySelectorVisuals();
	ApplyRevealVisuals();
	ApplyRootOpacity();
	UpdateSkipText();

	return FT66FlatStyle::MakeResponsiveRoot(Root);
}

FT66AnimationTimeline UT66CrateOverlayWidget::MakeTimeline(
	const FName TimelineName,
	const float Duration,
	const FT66AnimationCurveSpec& Curve,
	TFunction<void(float)> ProgressCallback) const
{
	FT66AnimationTimeline Timeline(TimelineName);
	Timeline.SetDuration(Duration);
	Timeline.SetCurve(Curve);
	Timeline.SetMaxMarkerDispatchesPerTick(24);
	Timeline.SetProgressCallback(MoveTemp(ProgressCallback));
	return Timeline;
}

void UT66CrateOverlayWidget::BuildFullAnimationSequence()
{
	RegisterMarkerHandlers();
	AnimationSequence = FT66AnimationSequence();
	bSkipSequenceActive = false;
	ActivePhase = ECrateAnimationPhase::Anticipation;
	ActiveSequenceDuration = 0.f;

	const float FinalOffset = StripState.FinalStripOffset;
	const float SpinEndOffset = StripState.SpinEndOffset;
	const float OvershootOffset = StripState.OvershootOffset;
	TWeakObjectPtr<UT66CrateOverlayWidget> WeakThis(this);

	FT66AnimationTimeline Anticipation = MakeTimeline(
		FName(TEXT("Crate.Anticipation")),
		AnticipationDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::Anticipation, 0.85f),
		[WeakThis](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Anticipation;
				Self->SelectorGlowAlpha = 0.10f + (FMath::Abs(CurveValue) * 0.12f);
				Self->ApplyStripOffset(FMath::Sin(CurveValue * PI * 2.f) * 2.5f);
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Anticipation));

	FT66AnimationTimeline Spin = MakeTimeline(
		FName(TEXT("Crate.Spin")),
		SpinDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis, SpinEndOffset](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Spin;
				Self->SelectorGlowAlpha = 0.20f;
				Self->NonWinnerOpacity = 1.f;
				Self->ApplyStripOffset(FMath::Lerp(0.f, SpinEndOffset, CurveValue));
			}
		});
	AddStripTickMarkers(Spin, 0.f, SpinEndOffset, false);
	AnimationSequence.AddTimeline(MoveTemp(Spin));

	FT66AnimationTimeline Deceleration = MakeTimeline(
		FName(TEXT("Crate.Deceleration")),
		DecelerationDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::WeightedDecel),
		[WeakThis, SpinEndOffset, FinalOffset](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Deceleration;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->SelectorGlowAlpha = 0.24f + (Clamped * 0.48f);
				Self->ApplyStripOffset(FMath::Lerp(SpinEndOffset, FinalOffset, Clamped));
			}
		});
	AddStripTickMarkers(Deceleration, SpinEndOffset, FinalOffset, true);
	AnimationSequence.AddTimeline(MoveTemp(Deceleration));

	FT66AnimationTimeline Overshoot = MakeTimeline(
		FName(TEXT("Crate.Overshoot")),
		OvershootDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::Overshoot, 1.15f),
		[WeakThis, FinalOffset, OvershootOffset](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Overshoot;
				Self->SelectorGlowAlpha = 0.70f;
				Self->ApplyStripOffset(FMath::Lerp(FinalOffset, OvershootOffset, FMath::Clamp(CurveValue, 0.f, 1.12f)));
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Overshoot));

	FT66AnimationTimeline Settle = MakeTimeline(
		FName(TEXT("Crate.Settle")),
		SettleDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::Settle, 1.10f),
		[WeakThis, FinalOffset, OvershootOffset](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Settle;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->SelectorGlowAlpha = 1.f - (Clamped * 0.45f);
				Self->ApplyStripOffset(FMath::Lerp(OvershootOffset, FinalOffset, Clamped));
			}
		});
	Settle.AddMarker({ CrateLandingMarker, ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, CrateLandingMarker });
	Settle.AddMarker({ CrateInventoryCommitMarker, ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, CrateInventoryCommitMarker });
	AnimationSequence.AddTimeline(MoveTemp(Settle));

	FT66AnimationTimeline Reveal = MakeTimeline(
		FName(TEXT("Crate.Reveal")),
		RevealDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
		[WeakThis](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Reveal;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->WinnerLiftOffset = FMath::Lerp(0.f, -16.f, Clamped);
				Self->WinnerGlowAlpha = Clamped;
				Self->NonWinnerOpacity = FMath::Lerp(1.f, 0.60f, Clamped);
				Self->SelectorGlowAlpha = FMath::Lerp(0.62f, 0.42f, Clamped);
			}
		});
	Reveal.AddMarker({ CrateRarityRevealMarker, ET66AnimationMarkerType::ProgressBased, 0.05f, 0.f, ET66AnimationMarkerFirePolicy::Once, CrateRarityRevealMarker });
	AnimationSequence.AddTimeline(MoveTemp(Reveal));

	FT66AnimationTimeline Hold = MakeTimeline(
		FName(TEXT("Crate.Hold")),
		HoldDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis](float)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Hold;
				Self->WinnerLiftOffset = -16.f;
				Self->WinnerGlowAlpha = 1.f;
				Self->NonWinnerOpacity = 0.60f;
				Self->SelectorGlowAlpha = 0.40f;
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Hold));

	FT66AnimationTimeline Dismiss = MakeTimeline(
		FName(TEXT("Crate.Dismiss")),
		DismissDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseInCubic),
		[WeakThis](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Dismiss;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->RootOpacity = 1.f - Clamped;
				Self->WinnerGlowAlpha = 1.f - (Clamped * 0.65f);
				Self->SelectorGlowAlpha = 0.40f * (1.f - Clamped);
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Dismiss));

	ActiveSequenceDuration = AnimationSequence.GetDuration();
	bAnimationActive = true;
	AnimationSequence.Play();
}

void UT66CrateOverlayWidget::BuildSkipToSettleSequence()
{
	RegisterMarkerHandlers();
	AnimationSequence = FT66AnimationSequence();
	bSkipSequenceActive = true;

	const float FinalOffset = StripState.FinalStripOffset;
	const float SkipStartOffset = T66LootUIAnimation::ComputeSkipStartOffset(
		StripState.CurrentStripOffset,
		FinalOffset,
		ItemTileStride,
		SkipTravelTileCount);
	const float OvershootOffset = StripState.OvershootOffset;
	ApplyStripOffset(SkipStartOffset);

	TWeakObjectPtr<UT66CrateOverlayWidget> WeakThis(this);
	FT66AnimationTimeline SkipDecel = MakeTimeline(
		FName(TEXT("Crate.SkipDeceleration")),
		SkipTravelDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::WeightedDecel),
		[WeakThis, SkipStartOffset, FinalOffset](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Deceleration;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->SelectorGlowAlpha = 0.40f + (Clamped * 0.35f);
				Self->ApplyStripOffset(FMath::Lerp(SkipStartOffset, FinalOffset, Clamped));
			}
		});
	AddStripTickMarkers(SkipDecel, SkipStartOffset, FinalOffset, true);
	AnimationSequence.AddTimeline(MoveTemp(SkipDecel));

	FT66AnimationTimeline Overshoot = MakeTimeline(
		FName(TEXT("Crate.SkipOvershoot")),
		OvershootDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::Overshoot, 1.10f),
		[WeakThis, FinalOffset, OvershootOffset](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Overshoot;
				Self->SelectorGlowAlpha = 0.78f;
				Self->ApplyStripOffset(FMath::Lerp(FinalOffset, OvershootOffset, FMath::Clamp(CurveValue, 0.f, 1.10f)));
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Overshoot));

	BuildSettleRevealSequence(SkipSettleDuration);
}

void UT66CrateOverlayWidget::BuildSettleRevealSequence(const float InSettleDuration)
{
	const bool bKeepExistingPrefix = AnimationSequence.GetChildCount() > 0 && bSkipSequenceActive;
	if (!bKeepExistingPrefix)
	{
		RegisterMarkerHandlers();
		AnimationSequence = FT66AnimationSequence();
		bSkipSequenceActive = true;
	}

	const float SettleStartOffset = bKeepExistingPrefix ? StripState.OvershootOffset : StripState.CurrentStripOffset;
	const float FinalOffset = StripState.FinalStripOffset;
	TWeakObjectPtr<UT66CrateOverlayWidget> WeakThis(this);

	FT66AnimationTimeline Settle = MakeTimeline(
		FName(TEXT("Crate.SkipSettle")),
		InSettleDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::Settle, 1.05f),
		[WeakThis, SettleStartOffset, FinalOffset](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Settle;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->SelectorGlowAlpha = 0.95f - (Clamped * 0.35f);
				Self->ApplyStripOffset(FMath::Lerp(SettleStartOffset, FinalOffset, Clamped));
			}
		});
	Settle.AddMarker({ CrateLandingMarker, ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, CrateLandingMarker });
	Settle.AddMarker({ CrateInventoryCommitMarker, ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, CrateInventoryCommitMarker });
	AnimationSequence.AddTimeline(MoveTemp(Settle));

	FT66AnimationTimeline Reveal = MakeTimeline(
		FName(TEXT("Crate.SkipReveal")),
		FMath::Min(0.48f, RevealDuration),
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
		[WeakThis](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Reveal;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->WinnerLiftOffset = FMath::Lerp(0.f, -16.f, Clamped);
				Self->WinnerGlowAlpha = Clamped;
				Self->NonWinnerOpacity = FMath::Lerp(1.f, 0.60f, Clamped);
				Self->SelectorGlowAlpha = FMath::Lerp(0.58f, 0.38f, Clamped);
			}
		});
	Reveal.AddMarker({ CrateRarityRevealMarker, ET66AnimationMarkerType::ProgressBased, 0.05f, 0.f, ET66AnimationMarkerFirePolicy::Once, CrateRarityRevealMarker });
	AnimationSequence.AddTimeline(MoveTemp(Reveal));

	FT66AnimationTimeline Hold = MakeTimeline(
		FName(TEXT("Crate.SkipHold")),
		0.25f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis](float)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Hold;
				Self->WinnerLiftOffset = -16.f;
				Self->WinnerGlowAlpha = 1.f;
				Self->NonWinnerOpacity = 0.60f;
				Self->SelectorGlowAlpha = 0.35f;
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Hold));

	FT66AnimationTimeline Dismiss = MakeTimeline(
		FName(TEXT("Crate.SkipDismiss")),
		0.25f,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseInCubic),
		[WeakThis](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Dismiss;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->RootOpacity = 1.f - Clamped;
				Self->WinnerGlowAlpha = 1.f - (Clamped * 0.65f);
				Self->SelectorGlowAlpha = 0.35f * (1.f - Clamped);
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Dismiss));

	ActiveSequenceDuration = AnimationSequence.GetDuration();
	bAnimationActive = true;
	AnimationSequence.Play();
}

void UT66CrateOverlayWidget::BuildDismissSequence()
{
	RegisterMarkerHandlers();
	AnimationSequence = FT66AnimationSequence();
	bSkipSequenceActive = true;
	CommitRewardIfNeeded();

	TWeakObjectPtr<UT66CrateOverlayWidget> WeakThis(this);
	FT66AnimationTimeline Dismiss = MakeTimeline(
		FName(TEXT("Crate.DismissFromReveal")),
		0.22f,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseInCubic),
		[WeakThis](float CurveValue)
		{
			if (UT66CrateOverlayWidget* Self = WeakThis.Get())
			{
				Self->ActivePhase = ECrateAnimationPhase::Dismiss;
				const float Clamped = FMath::Clamp(CurveValue, 0.f, 1.f);
				Self->RootOpacity = 1.f - Clamped;
				Self->WinnerGlowAlpha = 1.f - Clamped;
				Self->SelectorGlowAlpha = 0.25f * (1.f - Clamped);
			}
		});
	AnimationSequence.AddTimeline(MoveTemp(Dismiss));

	ActiveSequenceDuration = AnimationSequence.GetDuration();
	bAnimationActive = true;
	AnimationSequence.Play();
}

void UT66CrateOverlayWidget::AddStripTickMarkers(FT66AnimationTimeline& Timeline, const float StartOffset, const float EndOffset, const bool bWeightedDecel) const
{
	if (EndOffset <= StartOffset + UE_SMALL_NUMBER)
	{
		return;
	}

	for (int32 Index = 1; Index < WinnerIndex; ++Index)
	{
		if (!StripItems.IsValidIndex(Index))
		{
			continue;
		}

		const float BoundaryOffset = StripItems[Index].LayoutCenterX - StripState.SelectorPositionX;
		if (BoundaryOffset <= StartOffset || BoundaryOffset > EndOffset)
		{
			continue;
		}

		const float MarkerProgress = T66LootUIAnimation::ComputeStripBoundaryMarkerProgress(
			BoundaryOffset,
			StartOffset,
			EndOffset,
			bWeightedDecel);

		const bool bSlowdownTick = StripState.SlowdownStartIndex != INDEX_NONE && Index >= StripState.SlowdownStartIndex;
		const FName MarkerID = bSlowdownTick ? CrateSlowdownTickMarker : CrateTickMarker;
		Timeline.AddMarker({ MarkerID, ET66AnimationMarkerType::ProgressBased, MarkerProgress, 0.f, ET66AnimationMarkerFirePolicy::Once, MarkerID });
	}
}

void UT66CrateOverlayWidget::RegisterMarkerHandlers()
{
	MarkerDispatcher.ClearHandlers();
	MarkerDispatcher.RegisterAudioMarker(CrateTickMarker, CrateTickMarker, this);
	MarkerDispatcher.RegisterAudioMarker(CrateSlowdownTickMarker, CrateSlowdownTickMarker, this);
	MarkerDispatcher.RegisterAudioMarker(CrateLandingMarker, CrateLandingMarker, this);
	MarkerDispatcher.RegisterAudioMarker(CrateRarityRevealMarker, CrateRarityRevealMarker, this);

	TWeakObjectPtr<UT66CrateOverlayWidget> WeakThis(this);
	MarkerDispatcher.RegisterHandler(CrateTickMarker, [WeakThis](const FT66AnimationMarkerEvent&)
	{
		if (UT66CrateOverlayWidget* Self = WeakThis.Get())
		{
			Self->TriggerSelectorTickPulse();
		}
	});
	MarkerDispatcher.RegisterHandler(CrateSlowdownTickMarker, [WeakThis](const FT66AnimationMarkerEvent&)
	{
		if (UT66CrateOverlayWidget* Self = WeakThis.Get())
		{
			Self->TriggerSelectorTickPulse();
			Self->SelectorGlowAlpha = FMath::Max(Self->SelectorGlowAlpha, 0.64f);
		}
	});
	MarkerDispatcher.RegisterHandler(CrateLandingMarker, [WeakThis](const FT66AnimationMarkerEvent&)
	{
		if (UT66CrateOverlayWidget* Self = WeakThis.Get())
		{
			Self->SelectorGlowAlpha = 1.f;
			Self->ApplySelectorVisuals();
		}
	});
	MarkerDispatcher.RegisterHandler(CrateRarityRevealMarker, [WeakThis](const FT66AnimationMarkerEvent&)
	{
		if (UT66CrateOverlayWidget* Self = WeakThis.Get())
		{
			Self->WinnerGlowAlpha = FMath::Max(Self->WinnerGlowAlpha, 0.85f);
			Self->ApplyRevealVisuals();
		}
	});
	MarkerDispatcher.RegisterHandler(CrateInventoryCommitMarker, [WeakThis](const FT66AnimationMarkerEvent&)
	{
		if (UT66CrateOverlayWidget* Self = WeakThis.Get())
		{
			Self->CommitRewardIfNeeded();
		}
	});
}

void UT66CrateOverlayWidget::HandleMarkerEvents(const TArray<FT66AnimationMarkerEvent>& MarkerEvents)
{
	if (MarkerEvents.Num() == 0)
	{
		return;
	}

	MarkerDispatcher.Dispatch(MarkerEvents);
}

void UT66CrateOverlayWidget::StartAnimationActiveTimer(const TSharedRef<SWidget>& OwningWidget)
{
	StopAnimationActiveTimer();
	AnimationActiveTimerWidget = OwningWidget;
	AnimationActiveTimerHandle = OwningWidget->RegisterActiveTimer(
		0.f,
		FWidgetActiveTimerDelegate::CreateUObject(this, &UT66CrateOverlayWidget::HandleAnimationActiveTimer));
}

void UT66CrateOverlayWidget::StopAnimationActiveTimer()
{
	TSharedPtr<SWidget> Widget = AnimationActiveTimerWidget.Pin();
	if (Widget.IsValid() && AnimationActiveTimerHandle.IsValid())
	{
		Widget->UnRegisterActiveTimer(AnimationActiveTimerHandle.ToSharedRef());
	}

	AnimationActiveTimerHandle.Reset();
	AnimationActiveTimerWidget.Reset();
}

EActiveTimerReturnType UT66CrateOverlayWidget::HandleAnimationActiveTimer(double CurrentTime, const float DeltaTime)
{
	(void)CurrentTime;

	if (!bAnimationActive || !AnimationActiveTimerWidget.IsValid())
	{
		StopAnimationActiveTimer();
		return EActiveTimerReturnType::Stop;
	}

	TickAnimation(DeltaTime);

	if (TSharedPtr<SWidget> Widget = AnimationActiveTimerWidget.Pin())
	{
		Widget->Invalidate(EInvalidateWidgetReason::Paint);
	}

	return bAnimationActive ? EActiveTimerReturnType::Continue : EActiveTimerReturnType::Stop;
}

void UT66CrateOverlayWidget::TickAnimation(const float DeltaSeconds)
{
	if (!bAnimationActive)
	{
		return;
	}

	SelectorTickPulseAlpha = FMath::Max(0.f, SelectorTickPulseAlpha - (FMath::Max(0.f, DeltaSeconds) * 5.f));

	TArray<FT66AnimationMarkerEvent> MarkerEvents;
	AnimationSequence.Tick(FMath::Clamp(DeltaSeconds, 0.f, 0.05f), MarkerEvents);
	HandleMarkerEvents(MarkerEvents);

	ApplySelectorVisuals();
	ApplyRevealVisuals();
	ApplyRootOpacity();
	UpdateSkipText();

	if (IsAnimationTerminal())
	{
		bAnimationActive = false;
		ActivePhase = ECrateAnimationPhase::Complete;
		SignalAnimationComplete();
	}
}

void UT66CrateOverlayWidget::ApplyStripOffset(const float NewOffset)
{
	StripState.CurrentStripOffset = NewOffset;
	if (StripContainer.IsValid())
	{
		StripContainer->SetRenderTransform(FSlateRenderTransform(FVector2D(-StripState.CurrentStripOffset, 0.f)));
	}
}

void UT66CrateOverlayWidget::ApplySelectorVisuals()
{
	const float FlareAlpha = FMath::Clamp(SelectorGlowAlpha + (SelectorTickPulseAlpha * 0.30f), 0.f, 1.f);
	if (SelectorFlareBox.IsValid())
	{
		SelectorFlareBox->SetRenderOpacity(FlareAlpha);
	}
	if (SelectorFlareBorder.IsValid())
	{
		FLinearColor FlareColor = RewardResult.bLocked ? FT66RarityUtil::GetRarityColor(RewardResult.Rarity) : FT66FlatStyle::SelectedBorder();
		FlareColor.A = 0.10f + (FlareAlpha * 0.34f);
		SelectorFlareBorder->SetBorderBackgroundColor(FlareColor);
	}
}

void UT66CrateOverlayWidget::ApplyRevealVisuals()
{
	const FLinearColor WinnerColor = RewardResult.bLocked ? FT66RarityUtil::GetRarityColor(RewardResult.Rarity) : FLinearColor::White;

	for (int32 Index = 0; Index < StripItems.Num(); ++Index)
	{
		FCrateItemEntry& Entry = StripItems[Index];
		const bool bWinner = Index == WinnerIndex;
		if (Entry.TileBox.IsValid())
		{
			Entry.TileBox->SetRenderOpacity(bWinner ? 1.f : NonWinnerOpacity);
			Entry.TileBox->SetRenderTransform(FSlateRenderTransform(bWinner ? FVector2D(0.f, WinnerLiftOffset) : FVector2D::ZeroVector));
		}
		if (Entry.GlowBorder.IsValid())
		{
			FLinearColor GlowColor = bWinner ? WinnerColor : Entry.Color;
			GlowColor.A = bWinner ? (WinnerGlowAlpha * 0.36f) : 0.f;
			Entry.GlowBorder->SetRenderOpacity(bWinner ? WinnerGlowAlpha : 0.f);
			Entry.GlowBorder->SetBorderBackgroundColor(GlowColor);
		}
		if (Entry.IconImage.IsValid())
		{
			Entry.IconImage->SetColorAndOpacity(bWinner ? FLinearColor::White : FLinearColor(0.82f, 0.82f, 0.82f, NonWinnerOpacity));
		}
	}
}

void UT66CrateOverlayWidget::ApplyRootOpacity()
{
	if (RootAnimationBox.IsValid())
	{
		RootAnimationBox->SetRenderOpacity(FMath::Clamp(RootOpacity, 0.f, 1.f));
	}
	if (BackdropBorder.IsValid())
	{
		BackdropBorder->SetRenderOpacity(FMath::Clamp(RootOpacity, 0.f, 1.f));
	}
}

void UT66CrateOverlayWidget::TriggerSelectorTickPulse()
{
	SelectorTickPulseAlpha = 1.f;
}

void UT66CrateOverlayWidget::CommitRewardIfNeeded()
{
	if (!RewardResult.bLocked || RewardResult.bCommitAttempted)
	{
		return;
	}

	RewardResult.bCommitAttempted = true;
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState && !RewardResult.ItemID.IsNone())
	{
		const int32 InventoryCountBefore = RunState->GetInventorySlots().Num();
		RunState->RecordLuckQualityRarity(
			FName(TEXT("CrateRewardRarity")),
			RewardResult.Rarity,
			RewardResult.RarityDrawIndex,
			RewardResult.RarityPreDrawSeed,
			RewardResult.bRarityHasReplayWeights ? &RewardResult.RarityReplayWeights : nullptr);
		RunState->AddItemWithRarity(RewardResult.ItemID, LootRarityToItemRarity(RewardResult.Rarity));
		RewardResult.bCommitted = true;
		RewardResult.bAddedToInventory = RunState->GetInventorySlots().Num() > InventoryCountBefore;
	}
}

void UT66CrateOverlayWidget::SignalAnimationComplete()
{
	if (bCompletionSignaled)
	{
		return;
	}

	CommitRewardIfNeeded();
	bCompletionSignaled = true;

	if (UT66GameplayHUDWidget* Host = PresentationHost.Get())
	{
		Host->ClearActiveCratePresentation(this);
		if (RewardResult.bAddedToInventory)
		{
			Host->ShowPickupItemCard(RewardResult.ItemID, LootRarityToItemRarity(RewardResult.Rarity));
		}
	}

	RemoveFromParent();
}

void UT66CrateOverlayWidget::ResetVisualState()
{
	ActivePhase = ECrateAnimationPhase::Idle;
	bAnimationActive = false;
	bCompletionSignaled = false;
	bSkipSequenceActive = false;
	ActiveSequenceDuration = 0.f;
	SelectorGlowAlpha = 0.f;
	SelectorTickPulseAlpha = 0.f;
	WinnerGlowAlpha = 0.f;
	WinnerLiftOffset = 0.f;
	NonWinnerOpacity = 1.f;
	RootOpacity = 1.f;
}

void UT66CrateOverlayWidget::UpdateSkipText()
{
	if (!SkipText.IsValid())
	{
		return;
	}

	const float RemainingSeconds = bAnimationActive
		? ActiveSequenceDuration * (1.f - FMath::Clamp(AnimationSequence.GetTotalProgress(), 0.f, 1.f))
		: 0.f;
	SkipText->SetText(BuildSkipCountdownText(RemainingSeconds));
}

bool UT66CrateOverlayWidget::IsAnimationTerminal() const
{
	return T66Animation::IsTerminalState(AnimationSequence.GetState());
}
