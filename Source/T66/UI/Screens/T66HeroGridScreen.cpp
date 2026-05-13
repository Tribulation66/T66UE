// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66HeroGridScreen.h"
#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Engine/Texture2D.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

namespace
{
	FName HeroGridTag(const TCHAR* Name)
	{
		return FName(Name);
	}

	FName HeroGridSlotTag(const int32 Index)
	{
		return FName(*FString::Printf(TEXT("HeroGrid.Slot%02d"), Index + 1));
	}

	TSharedRef<SWidget> MakeHeroGridScrim(const FName Tag)
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

	TSharedRef<SWidget> MakeHeroGridTile(
		const ET66FlatState State,
		FOnClicked OnClicked,
		const TSharedRef<SWidget>& Content,
		const bool bEnabled,
		const FName Tag)
	{
		TSharedRef<SOverlay> TileContent = SNew(SOverlay)
			+ SOverlay::Slot()
			.Padding(40.f)
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
			176.f,
			176.f,
			bEnabled,
			Tag,
			HeroGridTag(TEXT("HeroGridSelection")));
	}
}

UT66HeroGridScreen::UT66HeroGridScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::HeroGrid;
	bIsModal = true;
}

UT66LocalizationSubsystem* UT66HeroGridScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

void UT66HeroGridScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	AllHeroIDs.Empty();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		AllHeroIDs = GI->GetPlayableHeroIDs();
	}
}

TSharedRef<SWidget> UT66HeroGridScreen::BuildSlateUI()
{
	// Ensure hero list is populated (BuildSlateUI may run before OnScreenActivated)
	if (AllHeroIDs.Num() == 0)
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			AllHeroIDs = GI->GetPlayableHeroIDs();
		}
	}

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66UITexturePoolSubsystem* TexPool = nullptr;
	if (UGameInstance* GI0 = UGameplayStatics::GetGameInstance(this))
	{
		TexPool = GI0->GetSubsystem<UT66UITexturePoolSubsystem>();
	}

	FText TitleText = Loc ? Loc->GetText_HeroGrid() : NSLOCTEXT("T66.HeroGrid", "Title", "HERO GRID");
	constexpr int32 HeroGridVisibleSlotCount = 16;
	constexpr float TileSize = 176.f;
	const float SlotXs[4] = { 596.f, 780.f, 964.f, 1149.f };
	const float SlotYs[4] = { 217.f, 401.f, 585.f, 770.f };

	HeroPortraitBrushes.Reset();
	HeroPortraitBrushes.Reserve(FMath::Min(AllHeroIDs.Num(), HeroGridVisibleSlotCount));

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

	AddSlot(0.f, 0.f, 1920.f, 1080.f, MakeHeroGridScrim(HeroGridTag(TEXT("HeroGrid.Scrim"))));
	AddSlot(411.f, 36.f, 1097.f, 1008.f,
		FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(0.f),
			SNullWidget::NullWidget,
			nullptr,
			HeroGridTag(TEXT("HeroGrid.ModalPanel"))));
	AddSlot(870.f, 76.f, 180.f, 59.f,
		FT66FlatStyle::MakeFlatLabel(
			TitleText,
			ET66FlatLabelRole::Title,
			ETextJustify::Center,
			HeroGridTag(TEXT("HeroGrid.Title"))));

	for (int32 Index = 0; Index < HeroGridVisibleSlotCount; Index++)
	{
		const int32 Row = Index / 4;
		const int32 Col = Index % 4;
		const bool bHasHero = AllHeroIDs.IsValidIndex(Index);
		FName HeroID = bHasHero ? AllHeroIDs[Index] : NAME_None;
		FHeroData HeroData;
		const bool bSelected = bHasHero && GI && GI->SelectedHeroID == HeroID;
		TSharedPtr<FSlateBrush> PortraitBrush;
		if (bHasHero && GI && GI->GetHeroData(HeroID, HeroData))
		{
			const TSoftObjectPtr<UTexture2D> PortraitSoft = GI->ResolveHeroPortrait(HeroData, GI->SelectedHeroBodyType, ET66HeroPortraitVariant::Half);
			if (!PortraitSoft.IsNull())
			{
				PortraitBrush = T66ScreenSlateHelpers::MakeSlateBrush(FVector2D(256.0f, 256.0f));
				HeroPortraitBrushes.Add(PortraitBrush);
				if (TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, PortraitBrush, HeroID, /*bClearWhileLoading*/ true);
				}
			}
		}
		FName HeroIDCopy = HeroID;
		const TSharedRef<SWidget> PortraitContent = PortraitBrush.IsValid()
			? StaticCastSharedRef<SWidget>(SNew(SImage).Image(PortraitBrush.Get()))
			: SNullWidget::NullWidget;
		const FOnClicked TileClicked = bHasHero
			? FOnClicked::CreateLambda([this, HeroIDCopy]() { return HandleHeroClicked(HeroIDCopy); })
			: FOnClicked();
		AddSlot(
			SlotXs[Col],
			SlotYs[Row],
			TileSize,
			TileSize,
			MakeHeroGridTile(
				bHasHero ? (bSelected ? ET66FlatState::Selected : ET66FlatState::Default) : ET66FlatState::Disabled,
				TileClicked,
				PortraitContent,
				bHasHero,
				HeroGridSlotTag(Index)));
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
		HeroGridTag(TEXT("HeroGrid.Root")),
		TEXT("Root"),
		ET66FlatState::Default);
}

FReply UT66HeroGridScreen::HandleHeroClicked(FName HeroID)
{
	// Tell the underlying Hero Selection screen to preview this hero
	if (UIManager)
	{
		UT66ScreenBase* Underlying = UIManager->GetCurrentScreen();
		if (UT66HeroSelectionScreen* HeroScreen = Cast<UT66HeroSelectionScreen>(Underlying))
		{
			HeroScreen->PreviewHero(HeroID);
		}
	}
	CloseModal();
	return FReply::Handled();
}

FReply UT66HeroGridScreen::HandleCloseClicked()
{
	CloseModal();
	return FReply::Handled();
}

