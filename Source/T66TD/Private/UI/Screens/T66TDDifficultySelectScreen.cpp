// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66TDDifficultySelectScreen.h"

#include "Core/T66TDDataSubsystem.h"
#include "Core/T66TDFrontendStateSubsystem.h"
#include "Data/T66TDDataTypes.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UI/T66TDUIStyle.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UITypes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
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

		return SNew(SScaleBox)
			.Stretch(EStretch::Fill)
			[
				SNew(SImage)
				.Image(Brush.Get())
				.ColorAndOpacity(FLinearColor::White)
			];
	}

	const FSlateBrush* ResolveScreenBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* RelativePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel);
	}

	const FSlateBrush* TDDifficultyLeftPanelBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveScreenBrush(
			Entry,
			T66TDUI::MasterBasicPanelPath(),
			T66TDUI::MasterPanelMargin(),
			TEXT("TDDifficultyMasterLeftPanel"));
	}

	const FSlateBrush* TDDifficultyCenterPanelBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveScreenBrush(
			Entry,
			T66TDUI::MasterBasicPanelPath(),
			T66TDUI::MasterPanelMargin(),
			TEXT("TDDifficultyMasterCenterPanel"));
	}

	const FSlateBrush* TDDifficultyRightPanelBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveScreenBrush(
			Entry,
			T66TDUI::MasterBasicPanelPath(),
			T66TDUI::MasterPanelMargin(),
			TEXT("TDDifficultyMasterRightPanel"));
	}

	const FSlateBrush* TDDifficultyItemBrush(const bool bIsSelected)
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush NormalEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush SelectedEntry;
		return ResolveScreenBrush(
			bIsSelected ? SelectedEntry : NormalEntry,
			T66TDUI::MasterInnerPanelPath(),
			T66TDUI::MasterPanelMargin(),
			bIsSelected ? TEXT("TDDifficultyMasterItemSelected") : TEXT("TDDifficultyMasterItem"));
	}

	const FSlateBrush* TDDifficultyMapCardBrush(const bool bIsSelected)
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush NormalEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush SelectedEntry;
		return ResolveScreenBrush(
			bIsSelected ? SelectedEntry : NormalEntry,
			T66TDUI::MasterInnerPanelPath(),
			T66TDUI::MasterPanelMargin(),
			bIsSelected ? TEXT("TDMasterMapCardSelected") : TEXT("TDMasterMapCard"));
	}

	const FSlateBrush* TDDifficultyButtonBrush(const ET66ButtonType Type)
	{
		return T66TDUI::ButtonPlateBrush(Type);
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

	const FLinearColor ShellFill = T66TDUI::ShellFill();
	const FLinearColor MutedText = T66TDUI::MutedText();
	const FLinearColor BrightText = T66TDUI::BrightText();

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
				.BorderBackgroundColor(T66TDUI::SelectionStroke(bIsSelected))
				.Padding(FMargin(1.f))
				[
					SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.OnClicked(FOnClicked::CreateUObject(this, &UT66TDDifficultySelectScreen::HandleDifficultyClicked, Difficulty.DifficultyID))
					[
						T66TDUI::MakeGeneratedPanel(
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
							,
							TDDifficultyItemBrush(bIsSelected),
							T66TDUI::CardFill(),
							FMargin(16.f, 12.f))
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
			.BorderBackgroundColor(T66TDUI::SelectionStroke(bIsSelected && SelectedDifficulty != nullptr))
			.Padding(FMargin(1.f))
				[
					SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.OnClicked(FOnClicked::CreateUObject(this, &UT66TDDifficultySelectScreen::HandleMapClicked, Map->MapID))
					[
					T66TDUI::MakeGeneratedPanel(
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
						,
						TDDifficultyMapCardBrush(bIsSelected),
						T66TDUI::CardFill(),
						FMargin(12.f))
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

	const TSharedPtr<FSlateBrush> SelectedMapBackgroundBrush = SelectedMap ? FindOrLoadMapBrush(SelectedMap->BackgroundRelativePath) : nullptr;
	FT66ButtonParams StartButtonParams = T66TDUI::MakePrimaryButtonParams(
		NSLOCTEXT("T66TD.Difficulty", "StartMatch", "START MATCH"),
		FOnClicked::CreateUObject(this, &UT66TDDifficultySelectScreen::HandleStartMatchClicked),
		308.f,
		48.f,
		14);
	StartButtonParams.SetDotaPlateOverrideBrush(TDDifficultyButtonBrush(ET66ButtonType::Success));

	const TSharedRef<SWidget> SummaryPanel =
		T66TDUI::MakeGeneratedPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66TD.Difficulty", "SummaryTitle", "REGULAR ROTATION"))
				.Font(FT66Style::MakeFont(TEXT("Bold"), 16))
				.ColorAndOpacity(T66TDUI::AccentGold())
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
				FT66Style::MakeButton(StartButtonParams)
			]
			,
			TDDifficultyRightPanelBrush(),
			ShellFill,
			FMargin(24.f, 22.f));

	FT66ButtonParams BackButtonParams = T66TDUI::MakeUtilityButtonParams(
		NSLOCTEXT("T66TD.Difficulty", "BackToTDMenu", "BACK TO TD"),
		FOnClicked::CreateUObject(this, &UT66TDDifficultySelectScreen::HandleBackClicked),
		304.f,
		44.f,
		10);
	BackButtonParams.SetDotaPlateOverrideBrush(TDDifficultyButtonBrush(ET66ButtonType::Neutral));

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(ShellFill)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				MakeOptionalImage(SelectedMapBackgroundBrush)
			]
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.70f))
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(FMargin(20.f, 20.f, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(304.f)
				.HeightOverride(44.f)
				[
					FT66Style::MakeButton(BackButtonParams)
				]
			]
			+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Top).Padding(430.f, 46.f, 120.f, 0.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66TD.Difficulty", "Title", "REGULAR DIFFICULTY AND MAP ROTATION"))
				.Font(FT66Style::MakeFont(TEXT("Black"), 18))
				.ColorAndOpacity(T66TDUI::AccentGold())
				.Justification(ETextJustify::Center)
			]
			+ SOverlay::Slot().Padding(FMargin(32.f, 100.f, 32.f, 24.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
				[
					SNew(SBox).WidthOverride(340.f)
					[
						T66TDUI::MakeGeneratedPanel(
							SNew(SScrollBox)
							+ SScrollBox::Slot()[DifficultyButtons]
							,
							TDDifficultyLeftPanelBrush(),
							ShellFill,
							FMargin(22.f, 20.f))
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 16.f, 0.f)
				[
					T66TDUI::MakeGeneratedPanel(
						MapGrid
						,
						TDDifficultyCenterPanelBrush(),
						T66TDUI::PanelFill(),
						FMargin(20.f))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).WidthOverride(360.f)
					[
						SummaryPanel
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
		if (const UT66TDDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66TDDataSubsystem>() : nullptr)
		{
			if (const FT66TDStageDefinition* StageDefinition = DataSubsystem->FindStageForMap(MapID))
			{
				FrontendState->SelectStage(StageDefinition->StageID);
			}
		}
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

	if (const FT66TDMapDefinition* ActiveMap = TDDataSubsystem->FindMap(FrontendState->GetSelectedMapID()))
	{
		if (const FT66TDStageDefinition* StageDefinition = TDDataSubsystem->FindStageForMap(ActiveMap->MapID))
		{
			FrontendState->SelectStage(StageDefinition->StageID);
		}
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
