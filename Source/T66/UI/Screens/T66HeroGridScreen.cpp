// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66HeroGridScreen.h"
#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SSpacer.h"

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
		AllHeroIDs = GI->GetAllHeroIDs();
	}
}

TSharedRef<SWidget> UT66HeroGridScreen::BuildSlateUI()
{
	// Ensure hero list is populated (BuildSlateUI may run before OnScreenActivated)
	if (AllHeroIDs.Num() == 0)
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			AllHeroIDs = GI->GetAllHeroIDs();
		}
	}

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	FText CloseText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66UITexturePoolSubsystem* TexPool = nullptr;
	if (UGameInstance* GI0 = UGameplayStatics::GetGameInstance(this))
	{
		TexPool = GI0->GetSubsystem<UT66UITexturePoolSubsystem>();
	}

	FText TitleText = Loc ? Loc->GetText_HeroGrid() : NSLOCTEXT("T66.HeroGrid", "Title", "HERO GRID");
	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const T66ScreenSlateHelpers::FResponsiveGridModalMetrics GridMetrics =
		T66ScreenSlateHelpers::MakeResponsiveGridModalMetrics(AllHeroIDs.Num(), SafeFrameSize);

	HeroPortraitBrushes.Reset();
	HeroPortraitBrushes.Reserve(AllHeroIDs.Num());

	TSharedRef<SGridPanel> GridPanel = SNew(SGridPanel);
	for (int32 Index = 0; Index < AllHeroIDs.Num(); Index++)
	{
		const int32 Row = Index / GridMetrics.Columns;
		const int32 Col = Index % GridMetrics.Columns;
		FName HeroID = AllHeroIDs[Index];
		FHeroData HeroData;
		FLinearColor SpriteColor = FLinearColor(0.25f, 0.25f, 0.3f, 1.0f);
		TSharedPtr<FSlateBrush> PortraitBrush;
		if (GI && GI->GetHeroData(HeroID, HeroData))
		{
			SpriteColor = HeroData.PlaceholderColor;
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
			: StaticCastSharedRef<SWidget>(SNew(SSpacer));
		GridPanel->AddSlot(Col, Row)
			.Padding(GridMetrics.TileGap * 0.5f)
			[
				T66ScreenSlateHelpers::MakeResponsiveGridTile(
					FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateLambda([this, HeroIDCopy]() { return HandleHeroClicked(HeroIDCopy); })),
					SpriteColor,
					PortraitContent,
					GridMetrics)
			];
	}

	T66ScreenSlateHelpers::AddUniformGridPaddingSlots(*GridPanel, AllHeroIDs.Num(), GridMetrics);

	return T66ScreenSlateHelpers::MakeResponsiveGridModal(
		TitleText,
		GridPanel,
		FT66Style::MakeButton(FT66ButtonParams(CloseText, FOnClicked::CreateUObject(this, &UT66HeroGridScreen::HandleCloseClicked))
			.SetMinWidth(120.0f)),
		GridMetrics);
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

