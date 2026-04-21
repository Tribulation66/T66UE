// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66TDDifficultySelectScreen.h"

#include "Core/T66TDDataSubsystem.h"
#include "Core/T66TDFrontendStateSubsystem.h"
#include "Data/T66TDDataTypes.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UITypes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FVector2D TDMapPreviewSize(640.f, 360.f);

	void EnsureBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
	}

	TSharedRef<SWidget> MakeOptionalImage(const TSharedPtr<FSlateBrush>& Brush)
	{
		if (!Brush.IsValid() || !Brush->GetResourceObject())
		{
			return SNew(SBox);
		}

		return SNew(SImage)
			.Image(Brush.Get())
			.ColorAndOpacity(FLinearColor::White);
	}
}

UT66TDDifficultySelectScreen::UT66TDDifficultySelectScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::TDDifficultySelect;
	bIsModal = false;
}

void UT66TDDifficultySelectScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	EnsureSelectionState();
}

void UT66TDDifficultySelectScreen::OnScreenDeactivated_Implementation()
{
	ReleaseRetainedSlateState();
	Super::OnScreenDeactivated_Implementation();
}

void UT66TDDifficultySelectScreen::NativeDestruct()
{
	ReleaseRetainedSlateState();
	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66TDDifficultySelectScreen::BuildSlateUI()
{
	EnsureSelectionState();

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66TDDataSubsystem* TDDataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66TDDataSubsystem>() : nullptr;
	UT66TDFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr;
	const FT66TDDifficultyDefinition* SelectedDifficulty = (TDDataSubsystem && FrontendState) ? TDDataSubsystem->FindDifficulty(FrontendState->GetSelectedDifficultyID()) : nullptr;
	const FT66TDMapDefinition* SelectedMap = (TDDataSubsystem && FrontendState) ? TDDataSubsystem->FindMap(FrontendState->GetSelectedMapID()) : nullptr;
	const TArray<const FT66TDMapDefinition*> DifficultyMaps = (TDDataSubsystem && FrontendState) ? TDDataSubsystem->GetMapsForDifficulty(FrontendState->GetSelectedDifficultyID()) : TArray<const FT66TDMapDefinition*>();

	const FLinearColor ShellFill(0.012f, 0.010f, 0.014f, 0.985f);
	const FLinearColor PanelFill(0.050f, 0.032f, 0.040f, 0.98f);
	const FLinearColor CardFill(0.080f, 0.044f, 0.054f, 1.0f);
	const FLinearColor MutedText(0.78f, 0.74f, 0.72f, 1.0f);
	const FLinearColor BrightText(0.95f, 0.93f, 0.92f, 1.0f);

	TSharedRef<SVerticalBox> DifficultyButtons = SNew(SVerticalBox);
	if (TDDataSubsystem)
	{
		for (const FT66TDDifficultyDefinition& Difficulty : TDDataSubsystem->GetDifficulties())
		{
			const bool bIsSelected = SelectedDifficulty && SelectedDifficulty->DifficultyID == Difficulty.DifficultyID;
			DifficultyButtons->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(bIsSelected ? Difficulty.AccentColor : CardFill)
				.Padding(FMargin(1.f))
				[
					SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.OnClicked(FOnClicked::CreateUObject(this, &UT66TDDifficultySelectScreen::HandleDifficultyClicked, Difficulty.DifficultyID))
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(bIsSelected ? FLinearColor(0.10f, 0.05f, 0.06f, 1.0f) : FLinearColor(0.05f, 0.04f, 0.05f, 1.0f))
						.Padding(FMargin(14.f, 12.f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(FText::FromString(Difficulty.DisplayName))
								.Font(FT66Style::MakeFont(TEXT("Bold"), 18))
								.ColorAndOpacity(BrightText)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(Difficulty.Description))
								.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
								.ColorAndOpacity(MutedText)
								.AutoWrapText(true)
							]
						]
					]
				]
			];
		}
	}

	TSharedRef<SUniformGridPanel> MapGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(10.f));
	for (int32 Index = 0; Index < DifficultyMaps.Num(); ++Index)
	{
		const FT66TDMapDefinition* Map = DifficultyMaps[Index];
		if (!Map)
		{
			continue;
		}

		const bool bIsSelected = SelectedMap && SelectedMap->MapID == Map->MapID;
		const TSharedPtr<FSlateBrush>& PreviewBrush = FindOrLoadMapBrush(Map->BackgroundRelativePath);

		MapGrid->AddSlot(Index % 2, Index / 2)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(bIsSelected && SelectedDifficulty ? SelectedDifficulty->AccentColor : CardFill)
			.Padding(FMargin(1.f))
			[
				SNew(SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.OnClicked(FOnClicked::CreateUObject(this, &UT66TDDifficultySelectScreen::HandleMapClicked, Map->MapID))
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.04f, 1.0f))
					.Padding(FMargin(10.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBox)
							.WidthOverride(286.f)
							.HeightOverride(160.f)
							[
								SNew(SOverlay)
								+ SOverlay::Slot()[MakeOptionalImage(PreviewBrush)]
								+ SOverlay::Slot()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, PreviewBrush.IsValid() && PreviewBrush->GetResourceObject() ? 0.18f : 0.72f))]
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Map->DisplayName))
							.Font(FT66Style::MakeFont(TEXT("Bold"), 15))
							.ColorAndOpacity(BrightText)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::Format(
								NSLOCTEXT("T66TD.Difficulty", "MapMetaFmt", "{0} lanes | {1} pads | Boss wave {2}"),
								FText::AsNumber(Map->LaneCount),
								FText::AsNumber(Map->BuildPadCount),
								FText::AsNumber(Map->BossWave)))
							.Font(FT66Style::MakeFont(TEXT("Regular"), 10))
							.ColorAndOpacity(MutedText)
						]
					]
				]
			]
		];
	}

	FString FeaturedHeroes;
	if (TDDataSubsystem && SelectedMap)
	{
		for (int32 Index = 0; Index < SelectedMap->FeaturedHeroIDs.Num(); ++Index)
		{
			if (const FT66TDHeroDefinition* Hero = TDDataSubsystem->FindHero(SelectedMap->FeaturedHeroIDs[Index]))
			{
				if (!FeaturedHeroes.IsEmpty())
				{
					FeaturedHeroes += TEXT(", ");
				}
				FeaturedHeroes += Hero->DisplayName;
			}
		}
	}

	const TSharedRef<SWidget> SummaryPanel =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(PanelFill)
		.Padding(FMargin(16.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66TD.Difficulty", "SummaryTitle", "REGULAR ROTATION"))
				.Font(FT66Style::MakeFont(TEXT("Bold"), 16))
				.ColorAndOpacity(FLinearColor(0.94f, 0.74f, 0.32f, 1.0f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(SelectedMap ? SelectedMap->DisplayName : FString(TEXT("No map selected"))))
				.Font(FT66Style::MakeFont(TEXT("Bold"), 22))
				.ColorAndOpacity(BrightText)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(SelectedMap ? SelectedMap->ThemeLabel : FString()))
				.Font(FT66Style::MakeFont(TEXT("Regular"), 12))
				.ColorAndOpacity(MutedText)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(SelectedMap ? SelectedMap->Description : FString(TEXT("Select a map to inspect routing, pads, and enemy pressure."))))
				.Font(FT66Style::MakeFont(TEXT("Regular"), 13))
				.ColorAndOpacity(BrightText)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::Format(
					NSLOCTEXT("T66TD.Difficulty", "ScalarFmt", "Enemy Health x{0}  |  Enemy Speed x{1}  |  Boss x{2}  |  Reward x{3}"),
					FText::AsNumber(SelectedDifficulty ? SelectedDifficulty->EnemyHealthScalar : 1.0f),
					FText::AsNumber(SelectedDifficulty ? SelectedDifficulty->EnemySpeedScalar : 1.0f),
					FText::AsNumber(SelectedDifficulty ? SelectedDifficulty->BossScalar : 1.0f),
					FText::AsNumber(SelectedDifficulty ? SelectedDifficulty->RewardScalar : 1.0f)))
				.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
				.ColorAndOpacity(MutedText)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::Format(
					NSLOCTEXT("T66TD.Difficulty", "PressureFmt", "Pressure Notes: {0}"),
					FText::FromString(SelectedMap ? SelectedMap->EnemyNotes : FString(TEXT("Pending.")))))
				.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
				.ColorAndOpacity(BrightText)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::Format(
					NSLOCTEXT("T66TD.Difficulty", "HeroesFmt", "Featured Heroes: {0}"),
					FText::FromString(FeaturedHeroes.IsEmpty() ? FString(TEXT("Open roster")) : FeaturedHeroes)))
				.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
				.ColorAndOpacity(BrightText)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(
						NSLOCTEXT("T66TD.Difficulty", "StartMatch", "START MATCH"),
						FOnClicked::CreateUObject(this, &UT66TDDifficultySelectScreen::HandleStartMatchClicked),
						ET66ButtonType::Success)
					.SetMinWidth(308.f)
					.SetHeight(48.f)
					.SetFontSize(14)
					.SetPadding(FMargin(12.f, 8.f, 12.f, 6.f))
					.SetUseGlow(false))
			]
		];

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(ShellFill)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.40f))
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(FMargin(20.f, 20.f, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(304.f)
				.HeightOverride(40.f)
				[
					FT66Style::MakeButton(
						FT66ButtonParams(
							NSLOCTEXT("T66TD.Difficulty", "BackToTDMenu", "BACK TO TD"),
							FOnClicked::CreateUObject(this, &UT66TDDifficultySelectScreen::HandleBackClicked),
							ET66ButtonType::Neutral)
						.SetMinWidth(304.f)
						.SetHeight(40.f)
						.SetFontSize(12)
						.SetPadding(FMargin(14.f, 8.f, 14.f, 6.f))
						.SetUseGlow(false)
						.SetTextColor(FLinearColor(0.94f, 0.95f, 0.97f, 1.0f)))
				]
			]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 32.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66TD.Difficulty", "Title", "REGULAR DIFFICULTY AND MAP ROTATION"))
				.Font(FT66Style::MakeFont(TEXT("Black"), 28))
				.ColorAndOpacity(FLinearColor(0.96f, 0.80f, 0.38f, 1.0f))
			]
			+ SOverlay::Slot().Padding(FMargin(32.f, 100.f, 32.f, 24.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
				[
					SNew(SBox).WidthOverride(340.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(PanelFill)
						.Padding(FMargin(16.f))
						[
							SNew(SScrollBox)
							+ SScrollBox::Slot()[DifficultyButtons]
						]
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 16.f, 0.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(PanelFill)
					.Padding(FMargin(16.f))
					[
						MapGrid
					]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).WidthOverride(360.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()[SummaryPanel]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66TD.Difficulty", "RuntimeNote", "This pass locks the TD shell, 20-map rotation, and TD-owned data/assets. Gameplay runtime and placement/combat screens land next."))
							.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
							.ColorAndOpacity(MutedText)
							.AutoWrapText(true)
						]
					]
				]
			]
		];
}

FReply UT66TDDifficultySelectScreen::HandleBackClicked()
{
	NavigateTo(ET66ScreenType::TDMainMenu);
	return FReply::Handled();
}

FReply UT66TDDifficultySelectScreen::HandleDifficultyClicked(const FName DifficultyID)
{
	if (UT66TDFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->SelectDifficulty(DifficultyID);
	}

	EnsureSelectionState();
	FT66Style::DeferRebuild(this);
	return FReply::Handled();
}

FReply UT66TDDifficultySelectScreen::HandleMapClicked(const FName MapID)
{
	if (UT66TDFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->SelectMap(MapID);
	}

	FT66Style::DeferRebuild(this);
	return FReply::Handled();
}

FReply UT66TDDifficultySelectScreen::HandleStartMatchClicked()
{
	NavigateTo(ET66ScreenType::TDBattle);
	return FReply::Handled();
}

void UT66TDDifficultySelectScreen::EnsureSelectionState()
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66TDDataSubsystem* TDDataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66TDDataSubsystem>() : nullptr;
	UT66TDFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr;
	if (!TDDataSubsystem || !FrontendState)
	{
		return;
	}

	if (!FrontendState->HasSelectedDifficulty() && TDDataSubsystem->GetDifficulties().Num() > 0)
	{
		FrontendState->SelectDifficulty(TDDataSubsystem->GetDifficulties()[0].DifficultyID);
	}

	const TArray<const FT66TDMapDefinition*> MapsForDifficulty = TDDataSubsystem->GetMapsForDifficulty(FrontendState->GetSelectedDifficultyID());
	const FT66TDMapDefinition* SelectedMap = TDDataSubsystem->FindMap(FrontendState->GetSelectedMapID());
	const bool bSelectedMapMatchesDifficulty = SelectedMap && SelectedMap->DifficultyID == FrontendState->GetSelectedDifficultyID();
	if (!bSelectedMapMatchesDifficulty && MapsForDifficulty.Num() > 0)
	{
		FrontendState->SelectMap(MapsForDifficulty[0]->MapID);
	}
}

const TSharedPtr<FSlateBrush>& UT66TDDifficultySelectScreen::FindOrLoadMapBrush(const FString& RelativePath)
{
	for (const FMapBrushEntry& Entry : MapBrushEntries)
	{
		if (Entry.Key == RelativePath)
		{
			return Entry.Brush;
		}
	}

	FMapBrushEntry& NewEntry = MapBrushEntries.AddDefaulted_GetRef();
	NewEntry.Key = RelativePath;
	NewEntry.Brush = MakeShared<FSlateBrush>();
	EnsureBrush(NewEntry.Brush, TDMapPreviewSize);

	for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
	{
		if (!FPaths::FileExists(CandidatePath))
		{
			continue;
		}

		if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Trilinear, TEXT("TDMapPreview")))
		{
			RetainedTextures.Emplace(Texture);
			NewEntry.Brush->SetResourceObject(Texture);
			NewEntry.Brush->ImageSize = FVector2D(FMath::Max(1, Texture->GetSizeX()), FMath::Max(1, Texture->GetSizeY()));
			break;
		}
	}

	return NewEntry.Brush;
}

void UT66TDDifficultySelectScreen::ReleaseRetainedSlateState()
{
	MapBrushEntries.Reset();
	RetainedTextures.Reset();
}
