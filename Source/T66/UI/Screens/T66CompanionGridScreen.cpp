// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66CompanionGridScreen.h"
#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Screens/T66CompanionSelectionScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Engine/Texture2D.h"
#include "UI/T66DemoModeUIUtils.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

namespace
{
	FName CompanionGridTag(const TCHAR* Name)
	{
		return FName(Name);
	}

	FName CompanionGridSlotTag(const int32 Index)
	{
		return FName(*FString::Printf(TEXT("CompanionGrid.Slot%02d"), Index + 1));
	}

	TSharedRef<SWidget> MakeCompanionGridScrim(const FName Tag)
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.56f))
			.Padding(0.f),
			Tag,
			TEXT("Scrim"),
			ET66FlatState::Default);
	}

	TSharedRef<SWidget> MakeCompanionGridTile(
		const ET66FlatState State,
		FOnClicked OnClicked,
		const TSharedRef<SWidget>& Content,
		const bool bEnabled,
		const FName Tag)
	{
		TSharedRef<SOverlay> TileContent = SNew(SOverlay)
			+ SOverlay::Slot()
			.Padding(28.f)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					Content
				]
			];

		return FT66FlatStyle::MakeFlatToggleGroupButton(
			State,
			TileContent,
			MoveTemp(OnClicked),
			FMargin(0.f),
			138.f,
			138.f,
			bEnabled,
			Tag,
			CompanionGridTag(TEXT("CompanionGridSelection")));
	}
}

UT66CompanionGridScreen::UT66CompanionGridScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::CompanionGrid;
	bIsModal = true;
}

UT66LocalizationSubsystem* UT66CompanionGridScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

void UT66CompanionGridScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	AllCompanionIDs.Empty();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		AllCompanionIDs = GI->GetPlayableCompanionIDs();
	}
}

TSharedRef<SWidget> UT66CompanionGridScreen::BuildSlateUI()
{
	if (AllCompanionIDs.Num() == 0)
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			AllCompanionIDs = GI->GetPlayableCompanionIDs();
		}
	}

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	FText TitleText = Loc ? Loc->GetText_CompanionGrid() : NSLOCTEXT("T66.CompanionGrid", "Title", "GIRLFRIEND GRID");

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	TArray<FName> IDsWithNone;
	IDsWithNone.Add(NAME_None);
	IDsWithNone.Append(AllCompanionIDs);

	constexpr int32 CompanionGridVisibleSlotCount = 25;
	constexpr float TileSize = 138.f;
	const float SlotXs[5] = { 597.f, 744.f, 891.f, 1038.f, 1185.f };
	const float SlotYs[5] = { 218.f, 365.f, 512.f, 659.f, 806.f };

	CompanionPortraitBrushes.Reset();
	CompanionPortraitBrushes.Reserve(FMath::Min(IDsWithNone.Num(), CompanionGridVisibleSlotCount));

	const TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddSlot = [Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X, Y, W, H))
			[
				Widget
			];
	};

	AddSlot(0.f, 0.f, 1920.f, 1080.f, MakeCompanionGridScrim(CompanionGridTag(TEXT("CompanionGrid.Scrim"))));
	AddSlot(406.f, 36.f, 1109.f, 1008.f,
		FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(0.f),
			SNullWidget::NullWidget,
			nullptr,
			CompanionGridTag(TEXT("CompanionGrid.ModalPanel"))));
	AddSlot(812.f, 76.f, 296.f, 59.f,
		FT66FlatStyle::MakeFlatLabel(
			TitleText,
			ET66FlatLabelRole::Header,
			ETextJustify::Center,
			CompanionGridTag(TEXT("CompanionGrid.Title"))));

	for (int32 Index = 0; Index < CompanionGridVisibleSlotCount; Index++)
	{
		const int32 Row = Index / 5;
		const int32 Col = Index % 5;
		const bool bHasEntry = IDsWithNone.IsValidIndex(Index);
		FName CompanionID = bHasEntry ? IDsWithNone[Index] : NAME_None;
		FCompanionData Data;
		bool bUnlocked = bHasEntry;
		if (bHasEntry && !CompanionID.IsNone() && GI)
		{
			GI->GetCompanionData(CompanionID, Data);
			if (UT66CompanionUnlockSubsystem* Unlocks = GI->GetSubsystem<UT66CompanionUnlockSubsystem>())
			{
				bUnlocked = Unlocks->IsCompanionUnlocked(CompanionID);
			}
		}

		TSharedPtr<FSlateBrush> PortraitBrush;
		if (bHasEntry && !CompanionID.IsNone() && GI && GI->GetCompanionData(CompanionID, Data))
		{
			TSoftObjectPtr<UTexture2D> PortraitSoft = !Data.SelectionPortrait.IsNull() ? Data.SelectionPortrait : Data.Portrait;
			if (PortraitSoft.IsNull())
			{
				CompanionPortraitBrushes.Add(nullptr);
			}
			else
			{
				PortraitBrush = T66ScreenSlateHelpers::MakeSlateBrush(FVector2D(256.0f, 256.0f));
				CompanionPortraitBrushes.Add(PortraitBrush);
				if (TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, PortraitBrush, CompanionID, /*bClearWhileLoading*/ true);
				}
			}
		}
		else
		{
			CompanionPortraitBrushes.Add(nullptr);
		}

		FName CompanionIDCopy = CompanionID;
		TSharedPtr<FSlateBrush> PortraitBrushCopy = PortraitBrush;
		const bool bSelected = bHasEntry && GI && GI->SelectedCompanionID == CompanionIDCopy;
		const bool bCompanionPlayable = !bHasEntry || CompanionIDCopy.IsNone() || !GI || GI->IsCompanionPlayable(CompanionIDCopy);
		const TSharedRef<SWidget> PortraitContent = PortraitBrushCopy.IsValid()
			? StaticCastSharedRef<SWidget>(SNew(SImage).Image(PortraitBrushCopy.Get()))
			: SNullWidget::NullWidget;
		const bool bEnabled = bHasEntry && (CompanionIDCopy.IsNone() || (bUnlocked && bCompanionPlayable));
		const FOnClicked TileClicked = bEnabled
			? FOnClicked::CreateLambda([this, CompanionIDCopy]() { return HandleCompanionClicked(CompanionIDCopy); })
			: FOnClicked();
		const TSharedRef<SWidget> Tile = MakeCompanionGridTile(
			bEnabled ? (bSelected ? ET66FlatState::Selected : ET66FlatState::Default) : ET66FlatState::Disabled,
			TileClicked,
			PortraitContent,
			bEnabled,
			CompanionGridSlotTag(Index));
		AddSlot(
			SlotXs[Col],
			SlotYs[Row],
			TileSize,
			TileSize,
			Tile);
	}

	const TSharedRef<SWidget> RootContent = SNew(SBox)
		.WidthOverride(1920.f)
		.HeightOverride(1080.f)
		[
			Canvas
		];

	return FT66FlatStyle::AttachMetadata(
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				RootContent
			]
		],
		CompanionGridTag(TEXT("CompanionGrid.Root")),
		TEXT("Root"),
		ET66FlatState::Default);
}

FReply UT66CompanionGridScreen::HandleCompanionClicked(FName CompanionID)
{
	// Ignore locked companions (grid buttons should already be disabled, but keep it robust).
	if (!CompanionID.IsNone())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (!GI->IsCompanionPlayable(CompanionID))
			{
				return FReply::Handled();
			}

			if (UT66CompanionUnlockSubsystem* Unlocks = GI->GetSubsystem<UT66CompanionUnlockSubsystem>())
			{
				if (!Unlocks->IsCompanionUnlocked(CompanionID))
				{
					return FReply::Handled();
				}
			}
		}
	}

	if (UIManager)
	{
		UT66ScreenBase* Underlying = UIManager->GetCurrentScreen();
		if (UT66CompanionSelectionScreen* CompScreen = Cast<UT66CompanionSelectionScreen>(Underlying))
		{
			if (CompanionID.IsNone())
			{
				CompScreen->SelectNoCompanion();
			}
			else
			{
				CompScreen->PreviewCompanion(CompanionID);
			}
		}
		else if (UT66HeroSelectionScreen* HeroScreen = Cast<UT66HeroSelectionScreen>(Underlying))
		{
			if (CompanionID.IsNone())
			{
				HeroScreen->SelectNoCompanion();
			}
			else
			{
				HeroScreen->PreviewCompanion(CompanionID);
			}
		}
	}
	CloseModal();
	return FReply::Handled();
}

FReply UT66CompanionGridScreen::HandleCloseClicked()
{
	CloseModal();
	return FReply::Handled();
}
