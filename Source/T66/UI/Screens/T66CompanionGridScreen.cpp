// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66CompanionGridScreen.h"
#include "UI/Screens/T66CompanionSelectionScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"

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
		AllCompanionIDs = GI->GetAllCompanionIDs();
	}
}

TSharedRef<SWidget> UT66CompanionGridScreen::BuildSlateUI()
{
	if (AllCompanionIDs.Num() == 0)
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			AllCompanionIDs = GI->GetAllCompanionIDs();
		}
	}

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	FText TitleText = Loc ? Loc->GetText_CompanionGrid() : NSLOCTEXT("T66.CompanionGrid", "Title", "COMPANION GRID");
	FText CloseText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	FText NoCompanionText = Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.CompanionGrid", "NoCompanion", "NO COMPANION");

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));

	const int32 Columns = 6;
	TSharedRef<SVerticalBox> GridVertical = SNew(SVerticalBox);

	// First row: NO COMPANION tile, then companions
	TArray<FName> IDsWithNone;
	IDsWithNone.Add(NAME_None); // NO COMPANION
	IDsWithNone.Append(AllCompanionIDs);

	for (int32 Row = 0; Row * Columns < IDsWithNone.Num(); Row++)
	{
		TSharedRef<SHorizontalBox> RowBox = SNew(SHorizontalBox);
		for (int32 Col = 0; Col < Columns; Col++)
		{
			int32 Index = Row * Columns + Col;
			if (Index >= IDsWithNone.Num()) break;

			FName CompanionID = IDsWithNone[Index];
			FLinearColor SpriteColor = FLinearColor(0.35f, 0.25f, 0.25f, 1.0f); // Gray-red for "no companion"
			bool bUnlocked = true;
			if (!CompanionID.IsNone() && GI)
			{
				FCompanionData Data;
				if (GI->GetCompanionData(CompanionID, Data))
				{
					SpriteColor = Data.PlaceholderColor;
				}

				if (UT66CompanionUnlockSubsystem* Unlocks = GI->GetSubsystem<UT66CompanionUnlockSubsystem>())
				{
					bUnlocked = Unlocks->IsCompanionUnlocked(CompanionID);
				}
			}
			if (!CompanionID.IsNone() && !bUnlocked)
			{
				// Locked silhouette: black tile (still visible in the grid).
				SpriteColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
			}

			FName CompanionIDCopy = CompanionID;
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
						.OnClicked_Lambda([this, CompanionIDCopy]() { return HandleCompanionClicked(CompanionIDCopy); })
						.IsEnabled(CompanionIDCopy.IsNone() || bUnlocked)
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
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
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
						SNew(SBox).WidthOverride(120.0f).HeightOverride(44.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center).VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionGridScreen::HandleCloseClicked))
							.ButtonColorAndOpacity(FLinearColor(0.2f, 0.2f, 0.28f, 1.0f))
							[
								SNew(STextBlock).Text(CloseText)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
				]
			]
		];
}

FReply UT66CompanionGridScreen::HandleCompanionClicked(FName CompanionID)
{
	// Ignore locked companions (grid buttons should already be disabled, but keep it robust).
	if (!CompanionID.IsNone())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
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
	}
	CloseModal();
	return FReply::Handled();
}

FReply UT66CompanionGridScreen::HandleCloseClicked()
{
	CloseModal();
	return FReply::Handled();
}
