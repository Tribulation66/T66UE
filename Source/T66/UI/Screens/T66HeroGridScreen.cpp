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
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"
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
	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const int32 HeroCount = AllHeroIDs.Num();

	const int32 Columns = FMath::Max(1, FMath::CeilToInt(FMath::Sqrt(static_cast<float>(FMath::Max(HeroCount, 1)))));
	const int32 Rows = FMath::Max(1, FMath::CeilToInt(static_cast<float>(FMath::Max(HeroCount, 1)) / static_cast<float>(Columns)));
	const float ModalWidth = FMath::Min(SafeFrameSize.X * 0.92f, 1180.0f);
	const float ModalHeight = FMath::Min(SafeFrameSize.Y * 0.94f, 700.0f);
	const float ModalPaddingX = 30.0f;
	const float ModalPaddingY = 25.0f;
	const float TitleSectionHeight = 58.0f;
	const float FooterSectionHeight = 78.0f;
	const float TileGap = 6.0f;
	const float GridWidthBudget = FMath::Max(1.0f, ModalWidth - (ModalPaddingX * 2.0f));
	const float GridHeightBudget = FMath::Max(1.0f, ModalHeight - (ModalPaddingY * 2.0f) - TitleSectionHeight - FooterSectionHeight);
	const float TileSize = FMath::Clamp(
		FMath::FloorToFloat(FMath::Min(
			(GridWidthBudget - TileGap * static_cast<float>(Columns)) / static_cast<float>(Columns),
			(GridHeightBudget - TileGap * static_cast<float>(Rows)) / static_cast<float>(Rows))),
		64.0f,
		256.0f);
	const float GridWidth = Columns * TileSize + Columns * TileGap;
	const float GridHeight = Rows * TileSize + Rows * TileGap;

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
			const TSoftObjectPtr<UTexture2D> PortraitSoft = GI->ResolveHeroPortrait(HeroData, GI->SelectedHeroBodyType, ET66HeroPortraitVariant::Half);
			if (!PortraitSoft.IsNull())
			{
				PortraitBrush = MakeShared<FSlateBrush>();
				PortraitBrush->DrawAs = ESlateBrushDrawType::Image;
				PortraitBrush->ImageSize = FVector2D(256.f, 256.f);
				HeroPortraitBrushes.Add(PortraitBrush);
				if (TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, PortraitBrush, HeroID, /*bClearWhileLoading*/ true);
				}
			}
		}
		FName HeroIDCopy = HeroID;
		GridPanel->AddSlot(Col, Row)
			.Padding(TileGap * 0.5f)
			[
				SNew(SBox)
				.WidthOverride(TileSize)
				.HeightOverride(TileSize)
				[
					FT66Style::MakeButton(FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateLambda([this, HeroIDCopy]() { return HandleHeroClicked(HeroIDCopy); }))
						.SetMinWidth(0.f)
						.SetPadding(FMargin(0.0f))
						.SetColor(FLinearColor::Transparent)
						.SetContent(
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(SpriteColor)
							]
							+ SOverlay::Slot()
							[
								SNew(SScaleBox)
								.Stretch(EStretch::ScaleToFit)
								[
									PortraitBrush.IsValid()
									? StaticCastSharedRef<SWidget>(SNew(SImage).Image(PortraitBrush.Get()))
									: StaticCastSharedRef<SWidget>(SNew(SSpacer))
								]
							]))
				]
			];
	}

	// Fill remaining grid slots so layout is even (e.g. 4 heroes = 2x2)
	for (int32 Index = static_cast<int32>(AllHeroIDs.Num()); Index < Rows * Columns; Index++)
	{
		const int32 Row = Index / Columns;
		const int32 Col = Index % Columns;
		GridPanel->AddSlot(Col, Row)
			.Padding(TileGap * 0.5f)
			[
				SNew(SBox)
				.WidthOverride(TileSize)
				.HeightOverride(TileSize)
				[
					SNew(SSpacer)
				]
			];
	}

	// Centered modal dialog (same layout as companion grid)
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::Scrim())
		[
			SNew(SBox)
			.WidthOverride(ModalWidth)
			.HeightOverride(ModalHeight)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Panel())
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
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(GridWidth)
						.HeightOverride(GridHeight)
						[
							GridPanel
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 20.0f, 0.0f, 0.0f)
					[
						FT66Style::MakeButton(FT66ButtonParams(CloseText, FOnClicked::CreateUObject(this, &UT66HeroGridScreen::HandleCloseClicked))
							.SetMinWidth(120.f))
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

