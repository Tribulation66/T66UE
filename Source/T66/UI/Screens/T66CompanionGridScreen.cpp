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
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SSpacer.h"

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
	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	TArray<FName> IDsWithNone;
	IDsWithNone.Add(NAME_None);
	IDsWithNone.Append(AllCompanionIDs);

	const int32 CompanionCount = IDsWithNone.Num();
	const T66ScreenSlateHelpers::FResponsiveGridModalMetrics GridMetrics =
		T66ScreenSlateHelpers::MakeResponsiveGridModalMetrics(CompanionCount, SafeFrameSize);

	CompanionPortraitBrushes.Reset();
	CompanionPortraitBrushes.Reserve(IDsWithNone.Num());

	TSharedRef<SGridPanel> GridPanel = SNew(SGridPanel);
	for (int32 Index = 0; Index < IDsWithNone.Num(); Index++)
	{
		const int32 Row = Index / GridMetrics.Columns;
		const int32 Col = Index % GridMetrics.Columns;
		FName CompanionID = IDsWithNone[Index];
		FCompanionData Data;
		FLinearColor SpriteColor = FLinearColor(0.35f, 0.25f, 0.25f, 1.0f);
		bool bUnlocked = true;
		if (!CompanionID.IsNone() && GI)
		{
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
			SpriteColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
		}

		TSharedPtr<FSlateBrush> PortraitBrush;
		if (!CompanionID.IsNone() && GI && GI->GetCompanionData(CompanionID, Data))
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
		const TSharedRef<SWidget> PortraitContent = PortraitBrushCopy.IsValid()
			? StaticCastSharedRef<SWidget>(SNew(SImage).Image(PortraitBrushCopy.Get()))
			: (CompanionIDCopy.IsNone()
				? StaticCastSharedRef<SWidget>(
					SNew(STextBlock)
					.Text(NoCompanionText)
					.Font(FT66Style::Tokens::FontBold(FMath::Clamp(FMath::RoundToInt(GridMetrics.TileSize * 0.16f), 10, 18)))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center)
					.AutoWrapText(true))
				: StaticCastSharedRef<SWidget>(SNew(SSpacer)));
		GridPanel->AddSlot(Col, Row)
			.Padding(GridMetrics.TileGap * 0.5f)
			[
				T66ScreenSlateHelpers::MakeResponsiveGridTile(
					FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateLambda([this, CompanionIDCopy]() { return HandleCompanionClicked(CompanionIDCopy); }))
						.SetEnabled(CompanionIDCopy.IsNone() || bUnlocked),
					SpriteColor,
					PortraitContent,
					GridMetrics)
			];
	}

	T66ScreenSlateHelpers::AddUniformGridPaddingSlots(*GridPanel, IDsWithNone.Num(), GridMetrics);

	return T66ScreenSlateHelpers::MakeResponsiveGridModal(
		TitleText,
		GridPanel,
		FT66Style::MakeButton(
			FT66ButtonParams(CloseText, FOnClicked::CreateUObject(this, &UT66CompanionGridScreen::HandleCloseClicked))
			.SetMinWidth(120.0f)
			.SetColor(FT66Style::Tokens::Panel2)),
		GridMetrics);
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

