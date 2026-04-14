// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniCompanionSelectScreen.h"

#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Engine/Texture2D.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "Styling/SlateBrush.h"
#include "UI/T66MiniUIStyle.h"
#include "UI/T66UITypes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FT66MiniCompanionDefinition* T66MiniResolveDefaultCompanion(
		const UT66MiniDataSubsystem* DataSubsystem,
		const UT66MiniSaveSubsystem* SaveSubsystem)
	{
		if (!DataSubsystem)
		{
			return nullptr;
		}

		for (const FT66MiniCompanionDefinition& Companion : DataSubsystem->GetCompanions())
		{
			if (SaveSubsystem ? SaveSubsystem->IsCompanionUnlocked(Companion.CompanionID, DataSubsystem) : Companion.bUnlockedByDefault)
			{
				return &Companion;
			}
		}

		return DataSubsystem->GetCompanions().Num() > 0 ? &DataSubsystem->GetCompanions()[0] : nullptr;
	}
}

UT66MiniCompanionSelectScreen::UT66MiniCompanionSelectScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MiniCompanionSelect;
	bIsModal = false;
}

void UT66MiniCompanionSelectScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	if (!DataSubsystem || !FrontendState || !SaveSubsystem)
	{
		return;
	}

	if (!FrontendState->HasSelectedHero() && DataSubsystem->GetHeroes().Num() > 0)
	{
		FrontendState->SelectHero(DataSubsystem->GetHeroes()[0].HeroID);
	}

	if (!FrontendState->HasSelectedCompanion() || !SaveSubsystem->IsCompanionUnlocked(FrontendState->GetSelectedCompanionID(), DataSubsystem))
	{
		if (const FT66MiniCompanionDefinition* DefaultCompanion = T66MiniResolveDefaultCompanion(DataSubsystem, SaveSubsystem))
		{
			FrontendState->SelectCompanion(DefaultCompanion->CompanionID);
		}
	}
}

TSharedRef<SWidget> UT66MiniCompanionSelectScreen::BuildSlateUI()
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	const TArray<FT66MiniCompanionDefinition> Companions = DataSubsystem ? DataSubsystem->GetCompanions() : TArray<FT66MiniCompanionDefinition>();

	if (DataSubsystem && FrontendState && SaveSubsystem)
	{
		if (!FrontendState->HasSelectedHero() && DataSubsystem->GetHeroes().Num() > 0)
		{
			FrontendState->SelectHero(DataSubsystem->GetHeroes()[0].HeroID);
		}

		if (!FrontendState->HasSelectedCompanion() || !SaveSubsystem->IsCompanionUnlocked(FrontendState->GetSelectedCompanionID(), DataSubsystem))
		{
			if (const FT66MiniCompanionDefinition* DefaultCompanion = T66MiniResolveDefaultCompanion(DataSubsystem, SaveSubsystem))
			{
				FrontendState->SelectCompanion(DefaultCompanion->CompanionID);
			}
		}
	}

	const FT66MiniHeroDefinition* SelectedHero = (DataSubsystem && FrontendState) ? DataSubsystem->FindHero(FrontendState->GetSelectedHeroID()) : nullptr;
	const FT66MiniCompanionDefinition* SelectedCompanion = (DataSubsystem && FrontendState) ? DataSubsystem->FindCompanion(FrontendState->GetSelectedCompanionID()) : nullptr;
	if ((!SelectedCompanion || (SaveSubsystem && !SaveSubsystem->IsCompanionUnlocked(SelectedCompanion->CompanionID, DataSubsystem))) && DataSubsystem && SaveSubsystem)
	{
		SelectedCompanion = T66MiniResolveDefaultCompanion(DataSubsystem, SaveSubsystem);
	}

	RebuildCompanionBrushes(Companions);
	const FSlateBrush* SelectedBrush = SelectedCompanion ? FindCompanionBrush(SelectedCompanion->CompanionID) : nullptr;
	const int32 TotalStagesCleared = (SaveSubsystem && DataSubsystem) ? SaveSubsystem->GetTotalStagesCleared(DataSubsystem) : 0;
	const int32 UnlockedCompanionCount = (SaveSubsystem && DataSubsystem) ? SaveSubsystem->GetUnlockedCompanionCount(DataSubsystem) : Companions.Num();
	const int32 SelectedUnityStages = (SaveSubsystem && DataSubsystem && SelectedCompanion)
		? SaveSubsystem->GetCompanionUnityStagesCleared(SelectedCompanion->CompanionID, DataSubsystem)
		: 0;
	const float SelectedHealingPerSecond = (SaveSubsystem && DataSubsystem && SelectedCompanion)
		? SaveSubsystem->GetCompanionHealingPerSecond(SelectedCompanion->CompanionID, DataSubsystem)
		: 0.0f;

	const FLinearColor BackgroundFill(0.11f, 0.12f, 0.16f, 1.0f);
	const FLinearColor ScreenTint(0.02f, 0.03f, 0.05f, 0.20f);
	const FLinearColor PanelFill(0.07f, 0.08f, 0.11f, 0.97f);
	const FLinearColor PanelOutline(0.55f, 0.56f, 0.62f, 0.95f);
	const FLinearColor SelectedFill(0.25f, 0.27f, 0.15f, 0.96f);
	const FLinearColor SelectedBorder(0.82f, 0.78f, 0.47f, 1.0f);
	const FLinearColor BodyText(0.91f, 0.92f, 0.95f, 1.0f);
	const FLinearColor MutedText(0.79f, 0.82f, 0.88f, 1.0f);
	const FLinearColor AccentText(0.92f, 0.88f, 0.68f, 1.0f);

	auto MakeFramedPanel = [&](TSharedRef<SWidget> Content, const FLinearColor& OutlineColor, const FLinearColor& FillColor, const FMargin& InnerPadding) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(OutlineColor)
			.Padding(1.f)
			[
				SNew(SBorder)
				.BorderImage(T66MiniUI::WhiteBrush())
				.BorderBackgroundColor(FillColor)
				.Padding(InnerPadding)
				[
					Content
				]
			];
	};

	auto MakeSpriteWidget = [&](const FSlateBrush* Brush, const FString& FallbackText, const FLinearColor& PlaceholderColor, const float Width, const float Height, const int32 FontSize) -> TSharedRef<SWidget>
	{
		return Brush && Brush->GetResourceObject()
			? StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(Width)
				.HeightOverride(Height)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					[
						SNew(SImage)
						.Image(Brush)
					]
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBorder)
				.BorderImage(T66MiniUI::WhiteBrush())
				.BorderBackgroundColor(PlaceholderColor)
				.Padding(FMargin(10.f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(FallbackText))
					.Font(T66MiniUI::BoldFont(FontSize))
					.ColorAndOpacity(FLinearColor::White)
					.Justification(ETextJustify::Center)
				]);
	};

	auto MakeMetricChip = [&](const FString& Label, const FString& Value, const FLinearColor& Accent) -> TSharedRef<SWidget>
	{
		return MakeFramedPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Font(T66MiniUI::BodyFont(12))
				.ColorAndOpacity(MutedText)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Value))
				.Font(T66MiniUI::BoldFont(17))
				.ColorAndOpacity(Accent)
			],
			FLinearColor(0.20f, 0.21f, 0.26f, 1.0f),
			FLinearColor(0.05f, 0.06f, 0.08f, 1.0f),
			FMargin(10.f, 8.f));
	};

	TSharedRef<SUniformGridPanel> CompanionMetrics = SNew(SUniformGridPanel).SlotPadding(FMargin(10.f, 10.f, 10.f, 10.f));
	if (SelectedCompanion)
	{
		CompanionMetrics->AddSlot(0, 0)[MakeMetricChip(TEXT("Healing / Sec"), FString::Printf(TEXT("%.1f"), SelectedHealingPerSecond), AccentText)];
		CompanionMetrics->AddSlot(1, 0)[MakeMetricChip(TEXT("Unity"), FString::Printf(TEXT("%d / %d"), SelectedUnityStages, UT66MiniSaveSubsystem::CompanionUnityTierHyperStages), FLinearColor::White)];
		CompanionMetrics->AddSlot(2, 0)[MakeMetricChip(TEXT("Unlock"), SelectedCompanion->UnlockStageRequirement <= 0 ? FString(TEXT("STARTER")) : FString::Printf(TEXT("Stage %d"), SelectedCompanion->UnlockStageRequirement), FLinearColor::White)];
	}

	TSharedRef<SUniformGridPanel> CompanionGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(9.f, 9.f, 9.f, 9.f));
	const int32 ColumnCount = 6;
	for (int32 Index = 0; Index < Companions.Num(); ++Index)
	{
		const FT66MiniCompanionDefinition& Companion = Companions[Index];
		const bool bIsUnlocked = SaveSubsystem ? SaveSubsystem->IsCompanionUnlocked(Companion.CompanionID, DataSubsystem) : Companion.bUnlockedByDefault;
		const bool bIsSelected = bIsUnlocked && SelectedCompanion && SelectedCompanion->CompanionID == Companion.CompanionID;
		const FSlateBrush* CompanionBrush = FindCompanionBrush(Companion.CompanionID);
		const FString StatusText = bIsUnlocked
			? FString(TEXT("READY"))
			: (Companion.UnlockStageRequirement <= 0
				? FString(TEXT("LOCKED"))
				: FString::Printf(TEXT("STAGE %d"), Companion.UnlockStageRequirement));

		CompanionGrid->AddSlot(Index % ColumnCount, Index / ColumnCount)
		[
			SNew(SBox)
			.WidthOverride(168.f)
			.HeightOverride(132.f)
			[
				SNew(SButton)
				.IsEnabled(bIsUnlocked)
				.OnClicked(FOnClicked::CreateLambda([this, CompanionID = Companion.CompanionID]()
				{
					return HandleCompanionClicked(CompanionID);
				}))
				.ButtonColorAndOpacity(bIsSelected ? SelectedFill : (bIsUnlocked ? PanelFill : FLinearColor(0.05f, 0.05f, 0.06f, 0.84f)))
				.ContentPadding(FMargin(8.f, 7.f))
				[
					SNew(SBorder)
					.BorderImage(T66MiniUI::WhiteBrush())
					.BorderBackgroundColor(bIsSelected ? SelectedBorder : (bIsUnlocked ? PanelOutline : FLinearColor(0.24f, 0.24f, 0.28f, 0.95f)))
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(T66MiniUI::WhiteBrush())
						.BorderBackgroundColor(FLinearColor(0.04f, 0.05f, 0.07f, 1.0f))
						.Padding(FMargin(8.f, 6.f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
							[
								MakeSpriteWidget(CompanionBrush, Companion.DisplayName.Left(1).ToUpper(), Companion.PlaceholderColor, 78.f, 78.f, 26)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 5.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(Companion.DisplayName))
								.Font(T66MiniUI::BoldFont(12))
								.ColorAndOpacity(!bIsUnlocked ? MutedText : (bIsSelected ? T66MiniUI::ButtonTextDark() : BodyText))
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 2.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(StatusText))
								.Font(T66MiniUI::BodyFont(10))
								.ColorAndOpacity(bIsUnlocked ? AccentText : FLinearColor(0.88f, 0.74f, 0.52f, 1.0f))
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
			]
		];
	}

	const FString PairingLine = (SelectedHero && SelectedCompanion)
		? FString::Printf(TEXT("%s enters the run with %s providing restorative support."), *SelectedHero->DisplayName, *SelectedCompanion->DisplayName)
		: FString(TEXT("Unlock or choose a companion before moving into difficulty selection."));
	const FString CompanionDescription = SelectedCompanion
		? (!SelectedCompanion->Description.IsEmpty()
			? SelectedCompanion->Description
			: FString::Printf(TEXT("%s is ready to trail the hero and keep the health bar topped up."), *SelectedCompanion->DisplayName))
		: FString(TEXT("Select a companion from the roster below."));

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(BackgroundFill)
			.Padding(FMargin(16.f, 14.f, 16.f, 14.f))
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(T66MiniUI::WhiteBrush())
					.BorderBackgroundColor(ScreenTint)
				]
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Fill)
				[
					SNew(SBox)
					.WidthOverride(1320.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 14.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66Mini.CompanionSelect", "Title", "Companion Selection"))
							.Font(T66MiniUI::TitleFont(30))
							.ColorAndOpacity(FLinearColor::White)
							.ShadowOffset(FVector2D(2.f, 2.f))
							.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.95f))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.08f).Padding(0.f, 0.f, 18.f, 0.f)
							[
								MakeFramedPanel(
									SNew(SBox)
									.HeightOverride(304.f)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight()
										[
											SNew(STextBlock)
											.Text(FText::FromString(SelectedCompanion ? SelectedCompanion->DisplayName : FString(TEXT("No Companion Selected"))))
											.Font(T66MiniUI::TitleFont(24))
											.ColorAndOpacity(FLinearColor::White)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 14.f)
										[
											SNew(STextBlock)
											.Text(FText::FromString(PairingLine))
											.Font(T66MiniUI::BoldFont(16))
											.ColorAndOpacity(AccentText)
											.AutoWrapText(true)
										]
										+ SVerticalBox::Slot().FillHeight(1.f)
										[
											SNew(SHorizontalBox)
											+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(0.f, 0.f, 18.f, 0.f)
											[
												MakeFramedPanel(
													SNew(SBox)
													.WidthOverride(240.f)
													.HeightOverride(224.f)
													[
														SNew(SBorder)
														.BorderImage(T66MiniUI::WhiteBrush())
														.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.06f, 1.0f))
														.Padding(FMargin(12.f))
														[
															MakeSpriteWidget(
																SelectedBrush,
																SelectedCompanion ? SelectedCompanion->DisplayName.Left(1).ToUpper() : FString(TEXT("?")),
																SelectedCompanion ? SelectedCompanion->PlaceholderColor : T66MiniUI::RaisedFill(),
																214.f,
																214.f,
																42)
														]
													],
													FLinearColor(0.08f, 0.08f, 0.08f, 1.0f),
													FLinearColor(0.06f, 0.06f, 0.06f, 1.0f),
													FMargin(2.f))
											]
											+ SHorizontalBox::Slot().FillWidth(1.f)
											[
												SNew(SVerticalBox)
												+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 12.f)
												[
													SNew(STextBlock)
													.Text(FText::FromString(CompanionDescription))
													.Font(T66MiniUI::BodyFont(15))
													.ColorAndOpacity(BodyText)
													.AutoWrapText(true)
												]
												+ SVerticalBox::Slot().AutoHeight()
												[
													CompanionMetrics
												]
											]
										]
									],
									PanelOutline,
									PanelFill,
									FMargin(18.f, 16.f))
							]
							+ SHorizontalBox::Slot().FillWidth(0.72f)
							[
								MakeFramedPanel(
									SNew(SBox)
									.HeightOverride(304.f)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight()
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("T66Mini.CompanionSelect", "StatusTitle", "Run Pairing"))
											.Font(T66MiniUI::TitleFont(20))
											.ColorAndOpacity(FLinearColor::White)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 10.f)
										[
											MakeMetricChip(TEXT("Hero"), SelectedHero ? SelectedHero->DisplayName : FString(TEXT("--")), T66MiniUI::AccentGreen())
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
										[
											MakeMetricChip(TEXT("Companion"), SelectedCompanion ? SelectedCompanion->DisplayName : FString(TEXT("--")), AccentText)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
										[
											MakeMetricChip(TEXT("Roster"), FString::Printf(TEXT("%d / %d unlocked"), UnlockedCompanionCount, Companions.Num()), FLinearColor::White)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
										[
											MakeMetricChip(TEXT("Stages Cleared"), FString::Printf(TEXT("%d"), TotalStagesCleared), FLinearColor::White)
										]
										+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Bottom)
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("T66Mini.CompanionSelect", "Hint", "Clear mini stages to unlock more companions and grow unity for stronger healing."))
											.Font(T66MiniUI::BodyFont(14))
											.ColorAndOpacity(MutedText)
											.AutoWrapText(true)
										]
									],
									PanelOutline,
									PanelFill,
									FMargin(16.f, 16.f))
							]
						]
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							MakeFramedPanel(
								SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									CompanionGrid
								],
								PanelOutline,
								PanelFill,
								FMargin(18.f, 16.f, 18.f, 88.f))
						]
					]
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(28.f, 0.f, 0.f, 24.f)
		[
			SNew(SBox)
			.WidthOverride(140.f)
			.HeightOverride(48.f)
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniCompanionSelectScreen::HandleBackClicked))
				.ButtonColorAndOpacity(T66MiniUI::AccentBlue())
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Mini.CompanionSelect", "Back", "BACK"))
					.Font(T66MiniUI::BoldFont(18))
					.ColorAndOpacity(FLinearColor::White)
					.Justification(ETextJustify::Center)
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 28.f, 24.f)
		[
			SNew(SBox)
			.WidthOverride(200.f)
			.HeightOverride(54.f)
			[
				SNew(SButton)
				.IsEnabled(SelectedCompanion != nullptr)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniCompanionSelectScreen::HandleContinueClicked))
				.ButtonColorAndOpacity(T66MiniUI::AccentGreen())
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Mini.CompanionSelect", "Continue", "CONTINUE"))
					.Font(T66MiniUI::BoldFont(20))
					.ColorAndOpacity(T66MiniUI::ButtonTextDark())
					.Justification(ETextJustify::Center)
				]
			]
		];
}

FReply UT66MiniCompanionSelectScreen::HandleBackClicked()
{
	NavigateBack();
	return FReply::Handled();
}

FReply UT66MiniCompanionSelectScreen::HandleContinueClicked()
{
	NavigateTo(ET66ScreenType::MiniDifficultySelect);
	return FReply::Handled();
}

FReply UT66MiniCompanionSelectScreen::HandleCompanionClicked(const FName CompanionID)
{
	if (UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->SelectCompanion(CompanionID);
		ForceRebuildSlate();
	}

	return FReply::Handled();
}

void UT66MiniCompanionSelectScreen::RebuildCompanionBrushes(const TArray<FT66MiniCompanionDefinition>& Companions)
{
	CompanionBrushes.Reset();

	UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	for (const FT66MiniCompanionDefinition& Companion : Companions)
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->ImageSize = FVector2D(220.f, 220.f);
		Brush->SetResourceObject(nullptr);

		if (VisualSubsystem)
		{
			if (UTexture2D* CompanionTexture = VisualSubsystem->LoadCompanionTexture(Companion.VisualID))
			{
				Brush->SetResourceObject(CompanionTexture);
			}
		}

		CompanionBrushes.Add(Companion.CompanionID, Brush);
	}
}

const FSlateBrush* UT66MiniCompanionSelectScreen::FindCompanionBrush(const FName CompanionID) const
{
	if (const TSharedPtr<FSlateBrush>* Found = CompanionBrushes.Find(CompanionID))
	{
		return Found->Get();
	}

	return nullptr;
}
