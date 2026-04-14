// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniIdolSelectScreen.h"

#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Core/T66MiniRuntimeSubsystem.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Engine/Texture2D.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "Styling/SlateBrush.h"
#include "UI/T66MiniUIStyle.h"
#include "UI/T66UIManager.h"
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
	FName T66MiniResolveUnlockedCompanionID(const UT66MiniDataSubsystem* DataSubsystem, const UT66MiniSaveSubsystem* SaveSubsystem)
	{
		return SaveSubsystem ? SaveSubsystem->GetFirstUnlockedCompanionID(DataSubsystem) : NAME_None;
	}

	void T66MiniEnsureIdolOffers(UT66MiniDataSubsystem* DataSubsystem, UT66MiniFrontendStateSubsystem* FrontendState)
	{
		if (!DataSubsystem || !FrontendState)
		{
			return;
		}

		FrontendState->PrimeIdolOffers(DataSubsystem);
		if (FrontendState->GetCurrentIdolOfferIDs().Num() > 0)
		{
			return;
		}

		if (DataSubsystem->GetIdols().Num() == 0)
		{
			DataSubsystem->ReloadData();
		}

		if (DataSubsystem->GetIdols().Num() > 0)
		{
			FrontendState->RefreshIdolOffers(DataSubsystem);
		}

		if (FrontendState->GetCurrentIdolOfferIDs().Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("T66MiniIdolSelectScreen: no idol offers were generated. LoadedIdols=%d SelectedIdols=%d"),
				DataSubsystem->GetIdols().Num(),
				FrontendState->GetSelectedIdolIDs().Num());
		}
	}

	FString T66MiniFormatIdolName(const FT66MiniIdolDefinition& Idol)
	{
		FString Name = Idol.IdolID.ToString();
		Name.RemoveFromStart(TEXT("Idol_"));
		return Name.Replace(TEXT("_"), TEXT(" "));
	}
}

UT66MiniIdolSelectScreen::UT66MiniIdolSelectScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MiniIdolSelect;
	bIsModal = false;
}

void UT66MiniIdolSelectScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;

	if (DataSubsystem && FrontendState && SaveSubsystem)
	{
		if (!FrontendState->HasSelectedHero() && DataSubsystem->GetHeroes().Num() > 0)
		{
			FrontendState->SelectHero(DataSubsystem->GetHeroes()[0].HeroID);
		}

		if (!FrontendState->HasSelectedCompanion() || !SaveSubsystem->IsCompanionUnlocked(FrontendState->GetSelectedCompanionID(), DataSubsystem))
		{
			if (const FName DefaultCompanionID = T66MiniResolveUnlockedCompanionID(DataSubsystem, SaveSubsystem); DefaultCompanionID != NAME_None)
			{
				FrontendState->SelectCompanion(DefaultCompanionID);
			}
		}

		if (!FrontendState->HasSelectedDifficulty() && DataSubsystem->GetDifficulties().Num() > 0)
		{
			FrontendState->SelectDifficulty(DataSubsystem->GetDifficulties()[0].DifficultyID);
		}
	}

	T66MiniEnsureIdolOffers(DataSubsystem, FrontendState);
}

TSharedRef<SWidget> UT66MiniIdolSelectScreen::BuildSlateUI()
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;

	if (DataSubsystem && FrontendState && SaveSubsystem)
	{
		if (!FrontendState->HasSelectedHero() && DataSubsystem->GetHeroes().Num() > 0)
		{
			FrontendState->SelectHero(DataSubsystem->GetHeroes()[0].HeroID);
		}

		if (!FrontendState->HasSelectedCompanion() || !SaveSubsystem->IsCompanionUnlocked(FrontendState->GetSelectedCompanionID(), DataSubsystem))
		{
			if (const FName DefaultCompanionID = T66MiniResolveUnlockedCompanionID(DataSubsystem, SaveSubsystem); DefaultCompanionID != NAME_None)
			{
				FrontendState->SelectCompanion(DefaultCompanionID);
			}
		}

		if (!FrontendState->HasSelectedDifficulty() && DataSubsystem->GetDifficulties().Num() > 0)
		{
			FrontendState->SelectDifficulty(DataSubsystem->GetDifficulties()[0].DifficultyID);
		}
	}

	T66MiniEnsureIdolOffers(DataSubsystem, FrontendState);

	const TArray<FT66MiniIdolDefinition> Idols = DataSubsystem ? DataSubsystem->GetIdols() : TArray<FT66MiniIdolDefinition>();
	const FT66MiniHeroDefinition* SelectedHero = (DataSubsystem && FrontendState) ? DataSubsystem->FindHero(FrontendState->GetSelectedHeroID()) : nullptr;
	const FT66MiniCompanionDefinition* SelectedCompanion = (DataSubsystem && FrontendState) ? DataSubsystem->FindCompanion(FrontendState->GetSelectedCompanionID()) : nullptr;
	const FT66MiniDifficultyDefinition* SelectedDifficulty = (DataSubsystem && FrontendState) ? DataSubsystem->FindDifficulty(FrontendState->GetSelectedDifficultyID()) : nullptr;
	const int32 CompanionUnityStages = (SaveSubsystem && DataSubsystem && SelectedCompanion)
		? SaveSubsystem->GetCompanionUnityStagesCleared(SelectedCompanion->CompanionID, DataSubsystem)
		: 0;
	const float CompanionHealingPerSecond = (SaveSubsystem && DataSubsystem && SelectedCompanion)
		? SaveSubsystem->GetCompanionHealingPerSecond(SelectedCompanion->CompanionID, DataSubsystem)
		: 0.0f;
	RebuildIdolBrushes(Idols);

	if (CurrentStatusText.IsEmpty())
	{
		CurrentStatusText = NSLOCTEXT("T66Mini.IdolSelect", "DefaultStatus", "Select up to four idols, then continue into the run with the hero and difficulty you already locked in.");
	}

	const FLinearColor BackgroundFill(0.11f, 0.12f, 0.16f, 1.0f);
	const FLinearColor ScreenTint(0.02f, 0.03f, 0.05f, 0.20f);
	const FLinearColor PanelFill(0.07f, 0.08f, 0.11f, 0.97f);
	const FLinearColor PanelOutline(0.55f, 0.56f, 0.62f, 0.95f);
	const FLinearColor MutedText(0.79f, 0.82f, 0.88f, 1.0f);
	const FLinearColor BodyText(0.93f, 0.94f, 0.96f, 1.0f);

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

	auto MakeIconWidget = [&](const FSlateBrush* Brush, const FString& FallbackLabel, const float Width, const float Height, const FLinearColor& PlaceholderColor) -> TSharedRef<SWidget>
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
				.Padding(FMargin(6.f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(FallbackLabel))
					.Font(T66MiniUI::BoldFont(22))
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
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Value))
				.Font(T66MiniUI::BoldFont(16))
				.ColorAndOpacity(Accent)
			],
			FLinearColor(0.20f, 0.21f, 0.26f, 1.0f),
			FLinearColor(0.05f, 0.06f, 0.08f, 1.0f),
			FMargin(10.f, 8.f));
	};

	TSharedRef<SHorizontalBox> EquippedRow = SNew(SHorizontalBox);
	if (FrontendState)
	{
		for (int32 SlotIndex = 0; SlotIndex < UT66MiniFrontendStateSubsystem::MaxIdolSlots; ++SlotIndex)
		{
			const FName IdolID = FrontendState->GetSelectedIdolIDs().IsValidIndex(SlotIndex)
				? FrontendState->GetSelectedIdolIDs()[SlotIndex]
				: NAME_None;
			const FT66MiniIdolDefinition* Idol = DataSubsystem && IdolID != NAME_None ? DataSubsystem->FindIdol(IdolID) : nullptr;
			const FSlateBrush* IdolBrush = Idol ? FindIdolBrush(Idol->IdolID) : nullptr;
			const FString IdolName = Idol ? T66MiniFormatIdolName(*Idol) : FString::Printf(TEXT("EMPTY SLOT %d"), SlotIndex + 1);

			EquippedRow->AddSlot()
				.FillWidth(1.f)
				.Padding(SlotIndex > 0 ? FMargin(10.f, 0.f, 0.f, 0.f) : FMargin(0.f))
				[
					MakeFramedPanel(
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
						[
							MakeIconWidget(IdolBrush, Idol ? IdolName.Left(1).ToUpper() : FString::FromInt(SlotIndex + 1), 62.f, 62.f, Idol ? T66MiniUI::AccentGold() : T66MiniUI::RaisedFill())
						]
						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(FText::FromString(IdolName))
								.Font(T66MiniUI::BoldFont(Idol ? 17 : 16))
								.ColorAndOpacity(Idol ? FLinearColor::White : MutedText)
								.AutoWrapText(true)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(Idol ? FString::Printf(TEXT("%s | Base %.0f"), *Idol->Category, Idol->BaseDamage) : TEXT("Click an offer below to fill this slot.")))
								.Font(T66MiniUI::BodyFont(12))
								.ColorAndOpacity(Idol ? MutedText : BodyText)
								.AutoWrapText(true)
							]
						],
						Idol ? T66MiniUI::AccentGold() : PanelOutline,
						Idol ? FLinearColor(0.17f, 0.15f, 0.07f, 0.98f) : PanelFill,
						FMargin(14.f, 12.f))
				];
		}
	}

	TSharedRef<SVerticalBox> OfferColumn = SNew(SVerticalBox);
	if (FrontendState && DataSubsystem)
	{
		for (const FName IdolID : FrontendState->GetCurrentIdolOfferIDs())
		{
			const FT66MiniIdolDefinition* Idol = DataSubsystem->FindIdol(IdolID);
			if (!Idol)
			{
				continue;
			}

			const FSlateBrush* IdolBrush = FindIdolBrush(Idol->IdolID);
			const FString IdolName = T66MiniFormatIdolName(*Idol);

			OfferColumn->AddSlot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
						SNew(SButton)
						.OnClicked(FOnClicked::CreateLambda([this, IdolID]()
						{
							return HandleTakeIdolClicked(IdolID);
						}))
						.ButtonColorAndOpacity(T66MiniUI::AccentBlue())
						.ContentPadding(FMargin(6.f))
						[
							SNew(SBorder)
							.BorderImage(T66MiniUI::WhiteBrush())
							.BorderBackgroundColor(FLinearColor(0.05f, 0.06f, 0.08f, 1.0f))
							.Padding(FMargin(10.f, 8.f))
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 16.f, 0.f)
								[
									MakeFramedPanel(
										MakeIconWidget(IdolBrush, IdolName.Left(1).ToUpper(), 56.f, 56.f, T66MiniUI::RaisedFill()),
										FLinearColor(0.20f, 0.21f, 0.26f, 1.0f),
										FLinearColor(0.06f, 0.06f, 0.06f, 1.0f),
										FMargin(6.f))
							]
							+ SHorizontalBox::Slot().FillWidth(1.f)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SNew(STextBlock)
									.Text(FText::FromString(IdolName))
									.Font(T66MiniUI::BoldFont(17))
									.ColorAndOpacity(FLinearColor::White)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString::Printf(TEXT("%s idol | Base %.0f | Property %.0f"), *Idol->Category, Idol->BaseDamage, Idol->BaseProperty)))
									.Font(T66MiniUI::BodyFont(12))
									.ColorAndOpacity(BodyText)
									.AutoWrapText(true)
								]
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								MakeFramedPanel(
									SNew(STextBlock)
									.Text(NSLOCTEXT("T66Mini.IdolSelect", "Take", "TAKE"))
									.Font(T66MiniUI::BoldFont(18))
									.ColorAndOpacity(T66MiniUI::AccentGreen()),
									FLinearColor(0.22f, 0.24f, 0.19f, 1.0f),
									FLinearColor(0.10f, 0.12f, 0.08f, 1.0f),
									FMargin(14.f, 10.f))
							]
						]
					]
				];
		}
	}

	TSharedRef<SUniformGridPanel> StatusMetrics = SNew(SUniformGridPanel).SlotPadding(FMargin(10.f, 10.f, 10.f, 10.f));

	if (FrontendState)
	{
		StatusMetrics->AddSlot(0, 0)[MakeMetricChip(TEXT("Rerolls"), FString::Printf(TEXT("%d"), FrontendState->GetIdolRerollCount()), T66MiniUI::AccentGold())];
		StatusMetrics->AddSlot(1, 0)[MakeMetricChip(TEXT("Equipped"), FString::Printf(TEXT("%d / %d"), FrontendState->GetSelectedIdolIDs().Num(), UT66MiniFrontendStateSubsystem::MaxIdolSlots), FLinearColor::White)];
		StatusMetrics->AddSlot(0, 1)[MakeMetricChip(TEXT("Hero"), SelectedHero ? SelectedHero->DisplayName : FString(TEXT("--")), T66MiniUI::AccentGreen())];
		StatusMetrics->AddSlot(1, 1)[MakeMetricChip(TEXT("Difficulty"), SelectedDifficulty ? SelectedDifficulty->DisplayName : FString(TEXT("--")), SelectedDifficulty ? SelectedDifficulty->AccentColor : FLinearColor::White)];
		StatusMetrics->AddSlot(0, 2)[MakeMetricChip(TEXT("Companion"), SelectedCompanion ? SelectedCompanion->DisplayName : FString(TEXT("--")), T66MiniUI::AccentGold())];
		StatusMetrics->AddSlot(1, 2)[MakeMetricChip(TEXT("Heal / Sec"), FString::Printf(TEXT("%.1f"), CompanionHealingPerSecond), FLinearColor::White)];
		StatusMetrics->AddSlot(0, 3)[MakeMetricChip(TEXT("Unity"), FString::Printf(TEXT("%d / %d"), CompanionUnityStages, UT66MiniSaveSubsystem::CompanionUnityTierHyperStages), FLinearColor::White)];
	}

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
							.Text(NSLOCTEXT("T66Mini.IdolSelect", "Title", "Idol Selection"))
							.Font(T66MiniUI::TitleFont(30))
							.ColorAndOpacity(FLinearColor::White)
							.ShadowOffset(FVector2D(2.f, 2.f))
							.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.95f))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
						[
							EquippedRow
						]
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.06f).Padding(0.f, 0.f, 18.f, 0.f)
							[
								MakeFramedPanel(
									SNew(SScrollBox)
									+ SScrollBox::Slot()
									[
										OfferColumn
									],
									PanelOutline,
									PanelFill,
									FMargin(18.f, 16.f, 18.f, 60.f))
							]
							+ SHorizontalBox::Slot().FillWidth(0.82f)
							[
								MakeFramedPanel(
									SNew(SVerticalBox)
									+ SVerticalBox::Slot().AutoHeight()
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("T66Mini.IdolSelect", "LoadoutTitle", "Loadout Summary"))
										.Font(T66MiniUI::TitleFont(22))
										.ColorAndOpacity(FLinearColor::White)
									]
									+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 14.f)
									[
										StatusMetrics
									]
									+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
									[
										SNew(STextBlock)
										.Text(FText::FromString(SelectedDifficulty
											? FString::Printf(TEXT("%s pressure with %s on point and %s on support."), *SelectedDifficulty->DisplayName, SelectedHero ? *SelectedHero->DisplayName : TEXT("your hero"), SelectedCompanion ? *SelectedCompanion->DisplayName : TEXT("no companion"))
											: FString(TEXT("Lock in your idols before heading into the run."))))
										.Font(T66MiniUI::BodyFont(15))
										.ColorAndOpacity(BodyText)
										.AutoWrapText(true)
									]
									+ SVerticalBox::Slot().FillHeight(1.f)
									[
										SAssignNew(StatusTextBlock, STextBlock)
										.Text(CurrentStatusText)
										.Font(T66MiniUI::BodyFont(16))
										.ColorAndOpacity(MutedText)
										.AutoWrapText(true)
									],
									PanelOutline,
									PanelFill,
									FMargin(18.f, 16.f, 18.f, 60.f))
							]
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
				.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniIdolSelectScreen::HandleBackClicked))
				.ButtonColorAndOpacity(T66MiniUI::AccentBlue())
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Mini.IdolSelect", "Back", "BACK"))
					.Font(T66MiniUI::BoldFont(18))
					.ColorAndOpacity(FLinearColor::White)
					.Justification(ETextJustify::Center)
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 0.f, 24.f)
		[
			SNew(SBox)
			.WidthOverride(170.f)
			.HeightOverride(48.f)
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniIdolSelectScreen::HandleRerollClicked))
				.ButtonColorAndOpacity(T66MiniUI::AccentGold())
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Mini.IdolSelect", "Reroll", "REROLL"))
					.Font(T66MiniUI::BoldFont(18))
					.ColorAndOpacity(T66MiniUI::ButtonTextDark())
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
				.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniIdolSelectScreen::HandleContinueClicked))
				.ButtonColorAndOpacity(T66MiniUI::AccentGreen())
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Mini.IdolSelect", "Continue", "CONTINUE"))
					.Font(T66MiniUI::BoldFont(20))
					.ColorAndOpacity(T66MiniUI::ButtonTextDark())
					.Justification(ETextJustify::Center)
				]
			]
		];
}

FReply UT66MiniIdolSelectScreen::HandleBackClicked()
{
	NavigateBack();
	return FReply::Handled();
}

FReply UT66MiniIdolSelectScreen::HandleRerollClicked()
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	if (DataSubsystem && FrontendState)
	{
		FrontendState->RerollIdolOffers(DataSubsystem);
		T66MiniEnsureIdolOffers(DataSubsystem, FrontendState);
		CurrentStatusText = NSLOCTEXT("T66Mini.IdolSelect", "RerolledStatus", "Idol offers rerolled. Pick the best fit for your run.");
		ForceRebuildSlate();
	}

	return FReply::Handled();
}

FReply UT66MiniIdolSelectScreen::HandleTakeIdolClicked(const FName IdolID)
{
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	if (!FrontendState)
	{
		return FReply::Handled();
	}

	if (FrontendState->AddIdolToLoadout(IdolID))
	{
		CurrentStatusText = FText::FromString(FString::Printf(TEXT("%s added to the mini loadout."), *IdolID.ToString()));
		ForceRebuildSlate();
	}
	else
	{
		SetStatus(NSLOCTEXT("T66Mini.IdolSelect", "IdolRejected", "That idol is already equipped or all four slots are full."));
	}

	return FReply::Handled();
}

FReply UT66MiniIdolSelectScreen::HandleContinueClicked()
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniRuntimeSubsystem* RuntimeSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRuntimeSubsystem>() : nullptr;
	if (!FrontendState || !RunState || !SaveSubsystem || !RuntimeSubsystem)
	{
		SetStatus(NSLOCTEXT("T66Mini.IdolSelect", "MissingSystems", "Mini launch failed because one or more mini subsystems are missing."));
		return FReply::Handled();
	}

	if (FrontendState->GetSelectedIdolIDs().Num() <= 0)
	{
		SetStatus(NSLOCTEXT("T66Mini.IdolSelect", "NoIdols", "Pick at least one idol before continuing."));
		return FReply::Handled();
	}

	if (FrontendState->IsIntermissionFlow())
	{
		UT66MiniRunSaveGame* ActiveRun = RunState->GetActiveRun();
		if (!ActiveRun)
		{
			SetStatus(NSLOCTEXT("T66Mini.IdolSelect", "MissingActiveRun", "No active mini run is available for the next wave."));
			return FReply::Handled();
		}

		ActiveRun->EquippedIdolIDs = FrontendState->GetSelectedIdolIDs();
		ActiveRun->WaveIndex = FMath::Clamp(ActiveRun->WaveIndex + 1, 1, 5);
		ActiveRun->WaveSecondsRemaining = 180.f;
		ActiveRun->bHasMidWaveSnapshot = false;
		ActiveRun->bPendingShopIntermission = false;
		ActiveRun->bHasPlayerLocation = false;
		ActiveRun->PlayerLocation = FVector::ZeroVector;
		ActiveRun->bBossSpawnedForWave = false;
		ActiveRun->BossTelegraphRemaining = 0.f;
		ActiveRun->PendingBossID = NAME_None;
		ActiveRun->PendingBossSpawnLocation = FVector::ZeroVector;
		ActiveRun->EnemySpawnAccumulator = 0.f;
		ActiveRun->InteractableSpawnAccumulator = 0.f;
		ActiveRun->PostBossDelayRemaining = 0.f;
		ActiveRun->EnemySnapshots.Reset();
		ActiveRun->PickupSnapshots.Reset();
		ActiveRun->InteractableSnapshots.Reset();
		FrontendState->ExitIntermissionFlow();
		FrontendState->WriteIntermissionStateToRunSave(ActiveRun);
		SaveSubsystem->SaveRunToSlot(RunState->GetActiveSaveSlot(), ActiveRun);
		if (DataSubsystem)
		{
			FrontendState->RefreshIdolOffers(DataSubsystem);
		}
	}
	else if (!RunState->BootstrapRunFromFrontend(FrontendState, SaveSubsystem))
	{
		SetStatus(FText::FromString(RunState->GetLastBootstrapStatus()));
		return FReply::Handled();
	}

	FString FailureReason;
	if (!RuntimeSubsystem->LaunchMiniBattle(this, &FailureReason))
	{
		SetStatus(FText::FromString(FailureReason));
		return FReply::Handled();
	}

	if (UIManager)
	{
		UIManager->HideAllUI();
	}

	return FReply::Handled();
}

void UT66MiniIdolSelectScreen::SetStatus(const FText& InText)
{
	CurrentStatusText = InText;
	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(InText);
	}
}

void UT66MiniIdolSelectScreen::RebuildIdolBrushes(const TArray<FT66MiniIdolDefinition>& Idols)
{
	IdolBrushes.Reset();
	UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;

	for (const FT66MiniIdolDefinition& Idol : Idols)
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->ImageSize = FVector2D(128.f, 128.f);
		Brush->SetResourceObject(nullptr);

		if (!Idol.IconPath.IsEmpty())
		{
			if (UTexture2D* IconTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *Idol.IconPath)))
			{
				Brush->SetResourceObject(IconTexture);
			}
		}

		if (!Brush->GetResourceObject() && VisualSubsystem)
		{
			if (UTexture2D* IdolTexture = VisualSubsystem->LoadIdolEffectTexture(Idol.IdolID))
			{
				Brush->SetResourceObject(IdolTexture);
			}
		}

		IdolBrushes.Add(Idol.IdolID, Brush);
	}
}

const FSlateBrush* UT66MiniIdolSelectScreen::FindIdolBrush(const FName IdolID) const
{
	if (const TSharedPtr<FSlateBrush>* Found = IdolBrushes.Find(IdolID))
	{
		return Found->Get();
	}

	return nullptr;
}
