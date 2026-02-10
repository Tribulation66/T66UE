// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PlayerSummaryPickerScreen.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66GameInstance.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66UIManager.h"
#include "UI/T66UITypes.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"

UT66PlayerSummaryPickerScreen::UT66PlayerSummaryPickerScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PlayerSummaryPicker;
	bIsModal = true;
}

TSharedRef<SWidget> UT66PlayerSummaryPickerScreen::BuildSlateUI()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66UIManager* Manager = UIManager.Get();

	if (!LB || !Manager)
	{
		return SNew(SBorder)
			.BorderBackgroundColor(FLinearColor::Black)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Picker", "Error", "Error loading picker."))
				.Font(FT66Style::Tokens::FontRegular(14))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
	}

	const TArray<TObjectPtr<UT66LeaderboardRunSummarySaveGame>>& Snapshots = LB->GetPendingPickerSnapshots();
	if (Snapshots.Num() == 0)
	{
		return SNew(SBorder)
			.BorderBackgroundColor(FLinearColor::Black)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Picker", "NoPlayers", "No players."))
				.Font(FT66Style::Tokens::FontRegular(14))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
	}

	HeroBrushes.Reset();
	HeroBrushes.SetNum(Snapshots.Num());
	UT66UITexturePoolSubsystem* TexPool = T66GI ? T66GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	for (int32 i = 0; i < Snapshots.Num(); ++i)
	{
		UT66LeaderboardRunSummarySaveGame* Snap = Snapshots[i];
		if (!Snap) continue;

		TSharedPtr<FSlateBrush>& Brush = HeroBrushes[i];
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
			Brush->DrawAs = ESlateBrushDrawType::Image;
			Brush->ImageSize = FVector2D(96.f, 96.f);
		}
		if (T66GI && TexPool && !Snap->HeroID.IsNone())
		{
			FHeroData HeroData;
			if (T66GI->GetHeroData(Snap->HeroID, HeroData))
			{
				TSoftObjectPtr<UTexture2D> PortraitSoft = (Snap->HeroBodyType == ET66BodyType::TypeB && !HeroData.PortraitTypeB.IsNull())
					? HeroData.PortraitTypeB : HeroData.Portrait;
				if (!PortraitSoft.IsNull())
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, Brush,
						FName(*FString::Printf(TEXT("PickerHero_%d"), i)), /*bClearWhileLoading*/ true);
				}
			}
		}
	}

	TSharedRef<SHorizontalBox> OptionsBox = SNew(SHorizontalBox);
	for (int32 i = 0; i < Snapshots.Num(); ++i)
	{
		UT66LeaderboardRunSummarySaveGame* Snap = Snapshots[i];
		if (!Snap) continue;

		const FString DisplayName = Snap->DisplayName.IsEmpty() ? TEXT("Player") : Snap->DisplayName;
		const int32 CapturedIndex = i;

		OptionsBox->AddSlot()
			.FillWidth(1.0f)
			.Padding(16.0f, 0.0f)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(DisplayName))
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 12.0f)
					[
						SNew(SBox)
						.WidthOverride(96.0f)
						.HeightOverride(96.0f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FT66Style::Tokens::Panel2)
							.Padding(0.0f)
							[
								HeroBrushes.IsValidIndex(CapturedIndex) && HeroBrushes[CapturedIndex].IsValid()
									? StaticCastSharedRef<SWidget>(SNew(SImage).Image(HeroBrushes[CapturedIndex].Get()))
									: StaticCastSharedRef<SWidget>(SNew(SSpacer))
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						FT66Style::MakeButton(
							FT66ButtonParams(NSLOCTEXT("T66.Picker", "Select", "SELECT"), FOnClicked::CreateUObject(this, &UT66PlayerSummaryPickerScreen::HandleSelectClicked, CapturedIndex), ET66ButtonType::Primary)
							.SetMinWidth(140.f))
					]
				,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(16.0f, 12.0f)))
			];
	}

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f))
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 24.0f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Picker", "Title", "Pick the Player"))
						.Font(FT66Style::Tokens::FontBold(28))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						OptionsBox
					]
				,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(32.0f, 24.0f)))
			]
		];
}

FReply UT66PlayerSummaryPickerScreen::HandleSelectClicked(int32 Index)
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	if (!LB || !UIManager) return FReply::Handled();

	const TArray<TObjectPtr<UT66LeaderboardRunSummarySaveGame>>& Snapshots = LB->GetPendingPickerSnapshots();
	if (!Snapshots.IsValidIndex(Index) || !Snapshots[Index])
	{
		CloseModal();
		return FReply::Handled();
	}

	UT66LeaderboardRunSummarySaveGame* Chosen = Snapshots[Index];
	LB->SetPendingFakeRunSummarySnapshot(Chosen);
	LB->ClearPendingPickerSnapshots();
	CloseModal();
	UIManager->ShowModal(ET66ScreenType::RunSummary);
	return FReply::Handled();
}
