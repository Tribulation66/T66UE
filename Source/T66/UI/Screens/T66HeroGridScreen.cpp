// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66HeroGridScreen.h"
#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Styling/SlateBrush.h"
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

	const int32 Columns = FMath::Max(1, FMath::Min(3, AllHeroIDs.Num()));
	const int32 Rows = AllHeroIDs.Num() > 0 ? (AllHeroIDs.Num() + Columns - 1) / Columns : 0;

	HeroPortraitBrushes.Reset();
	HeroPortraitBrushes.Reserve(AllHeroIDs.Num());

	TSharedRef<SGridPanel> GridPanel = SNew(SGridPanel);
	for (int32 Index = 0; Index < AllHeroIDs.Num(); Index++)
	{
		const int32 Row = Index / Columns;
		const int32 Col = Index % Columns;
		FName HeroID = AllHeroIDs[Index];
		FHeroData HeroData;
		FLinearColor SpriteColor = FLinearColor(0.25f, 0.25f, 0.3f, 1.0f);
		TSharedPtr<FSlateBrush> PortraitBrush;
		if (GI && GI->GetHeroData(HeroID, HeroData))
		{
			SpriteColor = HeroData.PlaceholderColor;
			if (!HeroData.Portrait.IsNull())
			{
				PortraitBrush = MakeShared<FSlateBrush>();
				PortraitBrush->DrawAs = ESlateBrushDrawType::Image;
				PortraitBrush->ImageSize = FVector2D(256.f, 256.f);
				HeroPortraitBrushes.Add(PortraitBrush);
				if (TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, HeroData.Portrait, this, PortraitBrush, HeroID, /*bClearWhileLoading*/ true);
				}
			}
		}
		FName HeroIDCopy = HeroID;
		GridPanel->AddSlot(Col, Row)
			.Padding(8.0f)
			[
				SNew(SButton)
				.ButtonColorAndOpacity(FLinearColor::Transparent)
				.OnClicked_Lambda([this, HeroIDCopy]() { return HandleHeroClicked(HeroIDCopy); })
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(SpriteColor)
					]
					+ SOverlay::Slot()
					[
						PortraitBrush.IsValid()
						? StaticCastSharedRef<SWidget>(SNew(SImage).Image(PortraitBrush.Get()))
						: StaticCastSharedRef<SWidget>(SNew(SSpacer))
					]
				]
			];
	}

	// Fill remaining grid slots so layout is even (e.g. 4 heroes = 2x2)
	for (int32 Index = static_cast<int32>(AllHeroIDs.Num()); Index < Rows * Columns; Index++)
	{
		const int32 Row = Index / Columns;
		const int32 Col = Index % Columns;
		GridPanel->AddSlot(Col, Row)
			.Padding(8.0f)
			[
				SNew(SSpacer)
			];
	}
	// Make grid rows and columns share space equally so the grid fills the screen
	for (int32 Col = 0; Col < Columns; Col++)
	{
		GridPanel->SetColumnFill(Col, 1.0f);
	}
	for (int32 Row = 0; Row < Rows; Row++)
	{
		GridPanel->SetRowFill(Row, 1.0f);
	}

	TSharedRef<SVerticalBox> GridVertical = SNew(SVerticalBox);
	GridVertical->AddSlot()
		.FillHeight(1.0f)
		[
			GridPanel
		];

	const FButtonStyle& BtnNeutral = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral");
	const FTextBlockStyle& TxtButton = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button");

	// Centered modal dialog (same layout as companion grid)
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.15f, 1.0f))
				.Padding(FMargin(30.0f, 25.0f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 20.0f)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FT66Style::Tokens::FontBold(28))
						.ColorAndOpacity(FLinearColor::White)
					]
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
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 20.0f, 0.0f, 0.0f)
					[
						SNew(SBox).MinDesiredWidth(120.0f).HeightOverride(44.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center).VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroGridScreen::HandleCloseClicked))
							.ButtonStyle(&BtnNeutral)
							.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
							[
								SNew(STextBlock).Text(CloseText)
								.TextStyle(&TxtButton)
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
