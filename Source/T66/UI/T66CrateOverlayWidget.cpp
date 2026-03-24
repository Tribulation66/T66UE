// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CrateOverlayWidget.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/T66GameplayHUDWidget.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
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

	static FText BuildSkipCountdownText(float RemainingSeconds)
	{
		return FText::Format(
			NSLOCTEXT("T66.Crate", "SkipCountdown", "Skip {0}s"),
			FText::FromString(FString::Printf(TEXT("%.1f"), FMath::Max(0.f, RemainingSeconds))));
	}
}

void UT66CrateOverlayWidget::SetPresentationHost(UT66GameplayHUDWidget* InPresentationHost)
{
	PresentationHost = InPresentationHost;
}

void UT66CrateOverlayWidget::RequestSkip()
{
	if (bResolved)
	{
		return;
	}

	if (!bScrolling)
	{
		StartScrolling();
	}

	ScrollElapsed = ScrollDuration;
	CurrentScrollOffset = TotalScrollDistance;
	if (StripContainer.IsValid())
	{
		StripContainer->SetPadding(FMargin(-CurrentScrollOffset, 0.f, 0.f, 0.f));
	}
	ResolveOpen();
}

void UT66CrateOverlayWidget::GenerateStrip()
{
	StripItems.Empty();

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	if (RngSub && RunState)
	{
		RngSub->UpdateLuckStat(RunState->GetLuckStat());
	}

	FRandomStream Rng(static_cast<int32>(FPlatformTime::Cycles()));

	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	FT66RarityWeights CrateWeights = PlayerExperience
		? PlayerExperience->GetDifficultyCrateRarityWeights(Difficulty)
		: FT66RarityWeights{};
	if (RunState)
	{
		CrateWeights = ApplyLootCrateBias(CrateWeights, RunState->GetLootCrateRewardMultiplier());
	}

	const ET66Rarity WinRarity = (RngSub) ? RngSub->RollRarityWeighted(CrateWeights, Rng) : ET66Rarity::Black;
	WinnerItemID = T66GI ? T66GI->GetRandomItemIDForLootRarity(WinRarity) : FName(TEXT("Unknown"));

	for (int32 i = 0; i < StripItemCount; ++i)
	{
		FCrateItemEntry Entry;
		if (i == WinnerPosition)
		{
			Entry.ItemID = WinnerItemID;
			Entry.Rarity = WinRarity;
			WinnerRarity = WinRarity;
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

	const float VisibleWidth = static_cast<float>(VisibleTileCount) * ItemTileStride;
	const float WinnerCenterX = (static_cast<float>(WinnerPosition) * ItemTileStride) + (ItemTileGap * 0.5f) + (ItemTileWidth * 0.5f);
	TotalScrollDistance = FMath::Max(0.f, WinnerCenterX - (VisibleWidth * 0.5f));
}

void UT66CrateOverlayWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScrollTickHandle);
		World->GetTimerManager().ClearTimer(StartHandle);
	}

	if (UT66GameplayHUDWidget* Host = PresentationHost.Get())
	{
		Host->ClearActiveCratePresentation(this);
	}
	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66CrateOverlayWidget::RebuildWidget()
{
	GenerateStrip();

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	TSharedPtr<SHorizontalBox> StripRow = SNew(SHorizontalBox);
	for (int32 i = 0; i < StripItems.Num(); ++i)
	{
		const FCrateItemEntry& Entry = StripItems[i];
		const ET66ItemRarity ItemRarity = LootRarityToItemRarity(Entry.Rarity);
		const FText ItemName = Loc ? Loc->GetText_ItemDisplayName(Entry.ItemID) : FText::FromName(Entry.ItemID);
		const FText RarityName = Loc ? Loc->GetText_ItemRarityName(ItemRarity) : FText::GetEmpty();
		const FLinearColor FrameColor = Entry.Color * 0.45f + FLinearColor(0.10f, 0.10f, 0.12f, 0.55f);
		const FLinearColor NameplateColor = Entry.Color * 0.82f + FLinearColor(0.08f, 0.08f, 0.09f, 0.18f);

		StripRow->AddSlot()
			.AutoWidth()
			.Padding(ItemTileGap * 0.5f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(ItemTileWidth)
				.HeightOverride(ItemTileHeight)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FrameColor)
					.Padding(FMargin(2.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.76f, 0.76f, 0.78f, 1.f))
							.Padding(FMargin(10.f, 8.f))
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.18f, 0.18f, 0.20f, 0.22f))
									.Padding(FMargin(0.f))
								]
								+ SOverlay::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SBox)
									.WidthOverride(ItemPreviewSize)
									.HeightOverride(ItemPreviewSize)
									[
										SNew(SImage)
										.Image(Entry.IconBrush.IsValid() ? Entry.IconBrush.Get() : nullptr)
										.ColorAndOpacity(FLinearColor::White)
									]
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(NameplateColor)
							.Padding(FMargin(8.f, 6.f))
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SNew(STextBlock)
									.Text(ItemName)
									.Font(FT66Style::Tokens::FontBold(11))
									.ColorAndOpacity(FLinearColor::White)
									.Justification(ETextJustify::Left)
									.AutoWrapText(true)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(RarityName)
									.Font(FT66Style::Tokens::FontRegular(9))
									.ColorAndOpacity(FLinearColor(0.96f, 0.96f, 0.96f, 0.92f))
									.Justification(ETextJustify::Left)
								]
							]
						]
					]
				]
			];
	}

	TSharedRef<SWidget> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(24.f, 0.f, 24.f, 520.f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Crate", "Title", "OPEN CRATE"))
					.Font(FT66Style::Tokens::FontBold(22))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBox)
						.WidthOverride(static_cast<float>(VisibleTileCount) * ItemTileStride)
						.HeightOverride(ItemTileHeight + 8.f)
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
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(ItemTileWidth + 4.f)
						.HeightOverride(ItemTileHeight + 8.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(1.f, 0.83f, 0.24f, 0.16f))
							.Padding(FMargin(0.f))
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
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(1.f, 0.82f, 0.22f, 0.92f))
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.12f, 0.16f, 0.14f, 0.92f))
					.Padding(FMargin(8.f, 4.f))
					[
						SAssignNew(SkipText, STextBlock)
						.Text(BuildSkipCountdownText(ScrollDuration))
						.Font(FT66Style::Tokens::FontRegular(10))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.Justification(ETextJustify::Right)
					]
				],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(14.f, 14.f, 14.f, 10.f))
			)
		];

	if (SkipText.IsValid())
	{
		SkipText->SetText(BuildSkipCountdownText(ScrollDuration));
	}

	// Start almost immediately so the reveal feels attached to the interact press.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(StartHandle, this, &UT66CrateOverlayWidget::StartScrolling, 0.05f, false);
	}

	return Root;
}

void UT66CrateOverlayWidget::UpdateSkipText()
{
	if (SkipText.IsValid())
	{
		SkipText->SetText(BuildSkipCountdownText(ScrollDuration - ScrollElapsed));
	}
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
	UpdateSkipText();

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
	if (bResolved)
	{
		return;
	}

	bResolved = true;
	bScrolling = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScrollTickHandle);
		World->GetTimerManager().ClearTimer(StartHandle);
	}

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	bool bAddedToInventory = false;
	if (RunState && !WinnerItemID.IsNone())
	{
		const int32 InventoryCountBefore = RunState->GetInventorySlots().Num();
		RunState->RecordLuckQualityRarity(FName(TEXT("CrateRewardRarity")), WinnerRarity);
		RunState->AddItemWithRarity(WinnerItemID, LootRarityToItemRarity(WinnerRarity));
		bAddedToInventory = RunState->GetInventorySlots().Num() > InventoryCountBefore;
	}

	if (UT66GameplayHUDWidget* Host = PresentationHost.Get())
	{
		Host->ClearActiveCratePresentation(this);
		if (bAddedToInventory)
		{
			Host->ShowPickupItemCard(WinnerItemID, LootRarityToItemRarity(WinnerRarity));
		}
	}

	RemoveFromParent();
}

void UT66CrateOverlayWidget::StartScrolling()
{
	if (bScrolling || bResolved) return;
	UWorld* World = GetWorld();
	if (!World) return;

	bScrolling = true;
	ScrollElapsed = 0.f;
	CurrentScrollOffset = 0.f;
	LastTickTimeSeconds = static_cast<float>(World->GetTimeSeconds());
	UpdateSkipText();

	World->GetTimerManager().ClearTimer(ScrollTickHandle);
	World->GetTimerManager().SetTimer(ScrollTickHandle, this, &UT66CrateOverlayWidget::TickScroll, 0.033f, true);
}
