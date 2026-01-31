// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66HeroGridScreen.h"
#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"

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
	FText TitleText = Loc ? Loc->GetText_HeroGrid() : NSLOCTEXT("T66.HeroGrid", "Title", "HERO GRID");
	FText CloseText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));

	// Grid: 6 columns, rows as needed. Each cell is a colored box for a hero.
	const int32 Columns = 6;
	TSharedRef<SVerticalBox> GridVertical = SNew(SVerticalBox);

	for (int32 Row = 0; Row * Columns < AllHeroIDs.Num(); Row++)
	{
		TSharedRef<SHorizontalBox> RowBox = SNew(SHorizontalBox);
		for (int32 Col = 0; Col < Columns; Col++)
		{
			int32 Index = Row * Columns + Col;
			if (Index >= AllHeroIDs.Num()) break;

			FName HeroID = AllHeroIDs[Index];
			FHeroData HeroData;
			FLinearColor SpriteColor = FLinearColor(0.25f, 0.25f, 0.3f, 1.0f);
			if (GI && GI->GetHeroData(HeroID, HeroData))
			{
				SpriteColor = HeroData.PlaceholderColor;
			}

			FName HeroIDCopy = HeroID; // For lambda capture
			RowBox->AddSlot()
				.AutoWidth()
				.Padding(6.0f, 6.0f)
				[
					SNew(SBox)
					.WidthOverride(72.0f)
					.HeightOverride(72.0f)
					[
						SNew(SButton)
						.ButtonColorAndOpacity(SpriteColor)
						.OnClicked_Lambda([this, HeroIDCopy]() { return HandleHeroClicked(HeroIDCopy); })
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(SpriteColor)
						]
					]
				];
		}
		GridVertical->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				RowBox
			];
	}

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::Tokens::Scrim)
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
				.Padding(FMargin(FT66Style::Tokens::Space8, FT66Style::Tokens::Space6))
				[
					SNew(SVerticalBox)
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 20.0f)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
					]
					// Scrollable grid
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.MaxHeight(450.0f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							GridVertical
						]
					]
					// Close button
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 20.0f, 0.0f, 0.0f)
					[
						SNew(SBox).WidthOverride(120.0f).HeightOverride(44.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center).VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroGridScreen::HandleCloseClicked))
							.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
							.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
							.ContentPadding(FMargin(16.f, 10.f))
							[
								SNew(STextBlock).Text(CloseText)
								.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
							]
						]
					]
				]
			]
		];
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
