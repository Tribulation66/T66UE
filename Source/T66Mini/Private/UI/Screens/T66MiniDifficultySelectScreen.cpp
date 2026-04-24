// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniDifficultySelectScreen.h"

#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Engine/Texture2D.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "Styling/SlateBrush.h"
#include "UI/Screens/T66MiniGeneratedScreenChrome.h"
#include "UI/T66MiniUIStyle.h"
#include "UI/T66UIManager.h"
#include "UI/T66UITypes.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FSlateBrush* T66MiniSceneBackgroundBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(TEXT("SourceAssets/UI/MainMenuReference/scene_background_purple_imagegen_1920x1080.png")),
			FMargin(0.f),
			TEXT("MiniSceneBackground"));
	}

	TSharedRef<SWidget> T66MiniMakeSceneBackground(const FLinearColor& FallbackColor)
	{
		if (const FSlateBrush* Brush = T66MiniSceneBackgroundBrush())
		{
			return SNew(SImage)
				.Image(Brush)
				.ColorAndOpacity(FLinearColor(0.82f, 0.82f, 0.88f, 1.0f));
		}

		return SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(FallbackColor);
	}

	FName T66MiniDifficultyResolveUnlockedCompanionID(const UT66MiniDataSubsystem* DataSubsystem, const UT66MiniSaveSubsystem* SaveSubsystem)
	{
		return SaveSubsystem ? SaveSubsystem->GetFirstUnlockedCompanionID(DataSubsystem) : NAME_None;
	}
}

UT66MiniDifficultySelectScreen::UT66MiniDifficultySelectScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MiniDifficultySelect;
	bIsModal = false;
}

void UT66MiniDifficultySelectScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionStateChangedHandle = SessionSubsystem->OnSessionStateChanged().AddUObject(this, &UT66MiniDifficultySelectScreen::HandleSessionStateChanged);
			SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::MiniDifficultySelect, true);
			LastSessionUiStateKey = BuildSessionUiStateKey();
		}
	}

	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	const bool bDifficultyLockedToHost = SessionSubsystem
		&& SessionSubsystem->IsPartySessionActive()
		&& SessionSubsystem->GetMaxPartyMembers() > 1
		&& !SessionSubsystem->IsLocalPlayerPartyHost();
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
		if (const FName DefaultCompanionID = T66MiniDifficultyResolveUnlockedCompanionID(DataSubsystem, SaveSubsystem); DefaultCompanionID != NAME_None)
		{
			FrontendState->SelectCompanion(DefaultCompanionID);
		}
	}

	if (!FrontendState->HasSelectedDifficulty() && DataSubsystem->GetDifficulties().Num() > 0)
	{
		FrontendState->SelectDifficulty(DataSubsystem->GetDifficulties()[0].DifficultyID);
	}
}

void UT66MiniDifficultySelectScreen::OnScreenDeactivated_Implementation()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->OnSessionStateChanged().Remove(SessionStateChangedHandle);
		}
	}

	SessionStateChangedHandle.Reset();
	Super::OnScreenDeactivated_Implementation();
}

void UT66MiniDifficultySelectScreen::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->OnSessionStateChanged().Remove(SessionStateChangedHandle);
		}
	}

	SessionStateChangedHandle.Reset();
	Super::NativeDestruct();
}

void UT66MiniDifficultySelectScreen::HandleSessionStateChanged()
{
	SyncToSharedPartyScreen();
	if (UIManager && UIManager->GetCurrentScreenType() != ScreenType)
	{
		return;
	}

	const FString NewSessionUiStateKey = BuildSessionUiStateKey();
	if (NewSessionUiStateKey == LastSessionUiStateKey)
	{
		return;
	}

	LastSessionUiStateKey = NewSessionUiStateKey;
	ForceRebuildSlate();
}

void UT66MiniDifficultySelectScreen::SyncToSharedPartyScreen()
{
	if (!UIManager)
	{
		return;
	}

	UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (!SessionSubsystem || !SessionSubsystem->IsPartyLobbyContextActive() || SessionSubsystem->IsLocalPlayerPartyHost())
	{
		return;
	}

	const ET66ScreenType DesiredScreen = SessionSubsystem->GetDesiredPartyFrontendScreen();
	switch (DesiredScreen)
	{
	case ET66ScreenType::MiniMainMenu:
	case ET66ScreenType::MiniSaveSlots:
	case ET66ScreenType::MiniCharacterSelect:
	case ET66ScreenType::MiniCompanionSelect:
	case ET66ScreenType::MiniDifficultySelect:
	case ET66ScreenType::MiniIdolSelect:
	case ET66ScreenType::MiniShop:
	case ET66ScreenType::MiniRunSummary:
		if (UIManager->GetCurrentScreenType() != DesiredScreen)
		{
			UIManager->ShowScreen(DesiredScreen);
		}
		break;
	default:
		break;
	}
}

TSharedRef<SWidget> UT66MiniDifficultySelectScreen::BuildSlateUI()
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	const bool bDifficultyLockedToHost = SessionSubsystem
		&& SessionSubsystem->IsPartySessionActive()
		&& SessionSubsystem->GetMaxPartyMembers() > 1
		&& !SessionSubsystem->IsLocalPlayerPartyHost();

	if (DataSubsystem && FrontendState && SaveSubsystem)
	{
		if (!FrontendState->HasSelectedHero() && DataSubsystem->GetHeroes().Num() > 0)
		{
			FrontendState->SelectHero(DataSubsystem->GetHeroes()[0].HeroID);
		}

		if (!FrontendState->HasSelectedCompanion() || !SaveSubsystem->IsCompanionUnlocked(FrontendState->GetSelectedCompanionID(), DataSubsystem))
		{
			if (const FName DefaultCompanionID = T66MiniDifficultyResolveUnlockedCompanionID(DataSubsystem, SaveSubsystem); DefaultCompanionID != NAME_None)
			{
				FrontendState->SelectCompanion(DefaultCompanionID);
			}
		}

		if (!FrontendState->HasSelectedDifficulty() && DataSubsystem->GetDifficulties().Num() > 0)
		{
			FrontendState->SelectDifficulty(DataSubsystem->GetDifficulties()[0].DifficultyID);
		}
	}

	const FT66MiniHeroDefinition* SelectedHero = (DataSubsystem && FrontendState) ? DataSubsystem->FindHero(FrontendState->GetSelectedHeroID()) : nullptr;
	const FT66MiniCompanionDefinition* SelectedCompanion = (DataSubsystem && FrontendState) ? DataSubsystem->FindCompanion(FrontendState->GetSelectedCompanionID()) : nullptr;
	const FT66MiniDifficultyDefinition* SelectedDifficulty = (DataSubsystem && FrontendState) ? DataSubsystem->FindDifficulty(FrontendState->GetSelectedDifficultyID()) : nullptr;
	const TArray<FT66MiniDifficultyDefinition> Difficulties = DataSubsystem ? DataSubsystem->GetDifficulties() : TArray<FT66MiniDifficultyDefinition>();
	LastSessionUiStateKey = BuildSessionUiStateKey();
	RefreshSelectedHeroBrush(SelectedHero);
	const FSlateBrush* HeroBrush = SelectedHeroBrush.Get();

	const FLinearColor BackgroundFill = T66MiniUI::ScreenBackground();
	const FLinearColor ScreenTint = T66MiniUI::ScreenTint();
	const FLinearColor PanelFill = T66MiniUI::PanelFill();
	const FLinearColor PanelOutline = T66MiniUI::PanelOutline();
	const FLinearColor MutedText = T66MiniUI::MutedText();
	const FLinearColor BodyText = T66MiniUI::Text();

	auto MakeFramedPanel = [&](TSharedRef<SWidget> Content, const FLinearColor& OutlineColor, const FLinearColor& FillColor, const FMargin& InnerPadding) -> TSharedRef<SWidget>
	{
		const bool bUseOrnamentSafePadding = InnerPadding.Left >= 16.f || InnerPadding.Right >= 16.f;
		const FMargin SafePadding = bUseOrnamentSafePadding
			? FMargin(InnerPadding.Left, InnerPadding.Top, FMath::Max(InnerPadding.Right, 76.f), InnerPadding.Bottom)
			: InnerPadding;
		return T66MiniGeneratedChrome::MakePanel(Content, SafePadding, T66MiniGeneratedChrome::ESlice::PanelLarge);
	};

	auto MakeRowPanel = [&](TSharedRef<SWidget> Content, const FMargin& InnerPadding) -> TSharedRef<SWidget>
	{
		return T66MiniGeneratedChrome::MakeRowPanel(Content, InnerPadding);
	};

	auto MakeSpriteWidget = [&](const float Width, const float Height) -> TSharedRef<SWidget>
	{
		return HeroBrush && HeroBrush->GetResourceObject()
			? StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(Width)
				.HeightOverride(Height)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					[
						SNew(SImage)
						.Image(HeroBrush)
					]
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(Width)
				.HeightOverride(Height)
				[
					T66MiniGeneratedChrome::MakePanel(
						SNew(STextBlock)
						.Text(FText::FromString(SelectedHero ? SelectedHero->DisplayName.Left(1).ToUpper() : FString(TEXT("?"))))
						.Font(T66MiniUI::BoldFont(40))
						.ColorAndOpacity(FLinearColor::White)
						.Justification(ETextJustify::Center),
						FMargin(4.f),
						T66MiniGeneratedChrome::ESlice::PortraitFrame)
				]);
	};

	auto MakeMetricChip = [&](const FString& Label, const FString& Value, const FLinearColor& Accent) -> TSharedRef<SWidget>
	{
		return T66MiniGeneratedChrome::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Font(T66MiniUI::BodyFont(10))
				.ColorAndOpacity(MutedText)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Value))
				.Font(T66MiniUI::BoldFont(15))
				.ColorAndOpacity(Accent)
			],
			FMargin(10.f, 7.f, 18.f, 7.f),
			T66MiniGeneratedChrome::ESlice::PanelSmall);
	};

	TSharedRef<SUniformGridPanel> HeroStatWrap = SNew(SUniformGridPanel).SlotPadding(FMargin(7.f, 7.f, 7.f, 7.f));

	if (SelectedHero)
	{
		HeroStatWrap->AddSlot(0, 0)[MakeMetricChip(TEXT("Attack Family"), SelectedHero->PrimaryCategory, T66MiniUI::AccentGold())];
		HeroStatWrap->AddSlot(1, 0)[MakeMetricChip(TEXT("Damage"), FString::Printf(TEXT("%.0f"), SelectedHero->BaseDamage), FLinearColor::White)];
		HeroStatWrap->AddSlot(2, 0)[MakeMetricChip(TEXT("Attack Speed"), FString::Printf(TEXT("%.0f"), SelectedHero->BaseAttackSpeed), FLinearColor::White)];
		HeroStatWrap->AddSlot(0, 1)[MakeMetricChip(TEXT("Armor"), FString::Printf(TEXT("%.0f"), SelectedHero->BaseArmor), FLinearColor::White)];
		HeroStatWrap->AddSlot(1, 1)[MakeMetricChip(TEXT("Luck"), FString::Printf(TEXT("%.0f"), SelectedHero->BaseLuck), FLinearColor::White)];
		HeroStatWrap->AddSlot(2, 1)[MakeMetricChip(TEXT("Speed"), FString::Printf(TEXT("%.0f"), SelectedHero->BaseSpeed), FLinearColor::White)];
	}

	TSharedRef<SUniformGridPanel> DifficultyStatWrap = SNew(SUniformGridPanel).SlotPadding(FMargin(8.f, 8.f, 8.f, 8.f));

	if (SelectedDifficulty)
	{
		const FLinearColor Accent = SelectedDifficulty->AccentColor;
		DifficultyStatWrap->AddSlot(0, 0)[MakeMetricChip(TEXT("Health"), FString::Printf(TEXT("%.0f%%"), SelectedDifficulty->HealthScalar * 100.f), Accent)];
		DifficultyStatWrap->AddSlot(1, 0)[MakeMetricChip(TEXT("Damage"), FString::Printf(TEXT("%.0f%%"), SelectedDifficulty->DamageScalar * 100.f), Accent)];
		DifficultyStatWrap->AddSlot(2, 0)[MakeMetricChip(TEXT("Speed"), FString::Printf(TEXT("%.0f%%"), SelectedDifficulty->SpeedScalar * 100.f), Accent)];
		DifficultyStatWrap->AddSlot(0, 1)[MakeMetricChip(TEXT("Spawn Rate"), FString::Printf(TEXT("%.0f%%"), SelectedDifficulty->SpawnRateScalar * 100.f), Accent)];
		DifficultyStatWrap->AddSlot(1, 1)[MakeMetricChip(TEXT("Boss"), FString::Printf(TEXT("%.0f%%"), SelectedDifficulty->BossScalar * 100.f), Accent)];
	}

	TSharedRef<SUniformGridPanel> DifficultyGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(10.f));
	for (int32 Index = 0; Index < Difficulties.Num(); ++Index)
	{
		const FT66MiniDifficultyDefinition& Difficulty = Difficulties[Index];
		const bool bIsSelected = SelectedDifficulty && SelectedDifficulty->DifficultyID == Difficulty.DifficultyID;
		const FLinearColor TitleColor = bIsSelected ? Difficulty.AccentColor : FLinearColor::White;
		const FLinearColor SubtitleColor = bIsSelected ? FLinearColor::White : MutedText;

		DifficultyGrid->AddSlot(Index, 0)
		[
			SNew(SBox)
			.WidthOverride(242.f)
			.HeightOverride(118.f)
			[
				SNew(SButton)
				.IsEnabled(!bDifficultyLockedToHost)
				.OnClicked(FOnClicked::CreateLambda([this, DifficultyID = Difficulty.DifficultyID]()
				{
					return HandleDifficultyClicked(DifficultyID);
				}))
				.ButtonColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.01f))
				.ContentPadding(FMargin(2.f))
				[
					T66MiniGeneratedChrome::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(Difficulty.DisplayName))
								.Font(T66MiniUI::BoldFont(Difficulty.DisplayName.Len() > 8 ? 16 : (Difficulty.DisplayName.Len() > 6 ? 18 : 22)))
								.ColorAndOpacity(TitleColor)
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f).HAlign(HAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(FString::Printf(TEXT("Tier %d"), Index + 1)))
								.Font(T66MiniUI::BodyFont(15))
								.ColorAndOpacity(SubtitleColor)
								.Justification(ETextJustify::Center)
							]
						, FMargin(14.f, 12.f, 24.f, 12.f),
						bIsSelected ? T66MiniGeneratedChrome::ESlice::CardSelected : T66MiniGeneratedChrome::ESlice::CardNormal)
				]
			]
		];
	}

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			T66MiniMakeSceneBackground(BackgroundFill)
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(FLinearColor(0.018f, 0.014f, 0.024f, 0.48f))
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
							T66MiniGeneratedChrome::MakeTitlePlaque(
								NSLOCTEXT("T66Mini.DifficultySelect", "Title", "Difficulty Selection"),
								30,
								640.f,
								78.f)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.10f).Padding(0.f, 0.f, 18.f, 0.f)
							[
								MakeFramedPanel(
									SNew(SBox)
									.HeightOverride(326.f)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight()
										[
											SNew(STextBlock)
											.Text(FText::FromString(SelectedHero ? SelectedHero->DisplayName : FString(TEXT("No Hero Selected"))))
											.Font(T66MiniUI::TitleFont(24))
											.ColorAndOpacity(FLinearColor::White)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 14.f)
										[
											SNew(STextBlock)
											.Text(FText::FromString(SelectedHero
												? FString::Printf(TEXT("%s specialist paired with %s for the next ladder tier."), *SelectedHero->PrimaryCategory, SelectedCompanion ? *SelectedCompanion->DisplayName : TEXT("no companion"))
												: TEXT("Pick a hero first to review their mini run profile.")))
											.Font(T66MiniUI::BoldFont(16))
											.ColorAndOpacity(T66MiniUI::AccentGold())
											.AutoWrapText(true)
										]
										+ SVerticalBox::Slot().FillHeight(1.f)
										[
											SNew(SHorizontalBox)
											+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 18.f, 0.f)
											[
												MakeFramedPanel(
													SNew(SBox)
													.WidthOverride(246.f)
													.HeightOverride(228.f)
													[
														MakeSpriteWidget(222.f, 222.f)
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
													.Text(FText::FromString(SelectedHero
														? SelectedHero->Description
														: FString(TEXT("Pick a hero to see the mini run prep summary and base stats."))))
													.Font(T66MiniUI::BodyFont(15))
													.ColorAndOpacity(BodyText)
													.AutoWrapText(true)
												]
												+ SVerticalBox::Slot().AutoHeight()
												[
													HeroStatWrap
												]
											]
										]
									],
									PanelOutline,
									PanelFill,
									FMargin(18.f, 16.f))
							]
							+ SHorizontalBox::Slot().FillWidth(0.92f)
							[
								MakeFramedPanel(
									SNew(SBox)
									.HeightOverride(326.f)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight()
										[
											SNew(STextBlock)
											.Text(FText::FromString(SelectedDifficulty ? SelectedDifficulty->DisplayName : FString(TEXT("No Difficulty Selected"))))
											.Font(T66MiniUI::TitleFont(24))
											.ColorAndOpacity(SelectedDifficulty ? SelectedDifficulty->AccentColor : FLinearColor::White)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 12.f)
										[
											SNew(STextBlock)
											.Text(FText::FromString(SelectedDifficulty ? SelectedDifficulty->Description : FString(TEXT("Select a difficulty tier to continue."))))
											.Font(T66MiniUI::BodyFont(16))
											.ColorAndOpacity(BodyText)
											.AutoWrapText(true)
										]
										+ SVerticalBox::Slot().AutoHeight()
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("T66Mini.DifficultySelect", "LadderLabel", "Ladder Modifiers"))
											.Font(T66MiniUI::BoldFont(16))
											.ColorAndOpacity(MutedText)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 12.f)
										[
											DifficultyStatWrap
										]
										+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Bottom)
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("T66Mini.DifficultySelect", "Hint", "Next screen: idol loadout, rerolls, and your final pre-run kit." ))
											.Font(T66MiniUI::BodyFont(15))
											.ColorAndOpacity(MutedText)
											.AutoWrapText(true)
										]
									],
									PanelOutline,
									PanelFill,
									FMargin(18.f, 16.f))
							]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							MakeFramedPanel(
								DifficultyGrid,
								PanelOutline,
								PanelFill,
								FMargin(18.f, 16.f))
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
				T66MiniGeneratedChrome::MakeButton(
					NSLOCTEXT("T66Mini.DifficultySelect", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66MiniDifficultySelectScreen::HandleBackClicked),
					ET66ButtonType::Neutral,
					140.f,
					48.f,
					18)
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 28.f, 24.f)
		[
			SNew(SBox)
			.WidthOverride(226.f)
			.HeightOverride(58.f)
			[
				T66MiniGeneratedChrome::MakeButton(
					NSLOCTEXT("T66Mini.DifficultySelect", "Continue", "CONTINUE"),
					FOnClicked::CreateUObject(this, &UT66MiniDifficultySelectScreen::HandleContinueClicked),
					ET66ButtonType::Success,
					226.f,
					58.f,
					18)
			]
		];
}

FReply UT66MiniDifficultySelectScreen::HandleBackClicked()
{
	NavigateBack();
	return FReply::Handled();
}

FReply UT66MiniDifficultySelectScreen::HandleContinueClicked()
{
	NavigateTo(ET66ScreenType::MiniIdolSelect);
	return FReply::Handled();
}

FReply UT66MiniDifficultySelectScreen::HandleDifficultyClicked(const FName DifficultyID)
{
	if (UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr)
	{
		if (SessionSubsystem->IsPartySessionActive()
			&& SessionSubsystem->GetMaxPartyMembers() > 1
			&& !SessionSubsystem->IsLocalPlayerPartyHost())
		{
			return FReply::Handled();
		}
	}

	if (UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr)
	{
		if (FrontendState->GetSelectedDifficultyID() == DifficultyID)
		{
			return FReply::Handled();
		}

		FrontendState->SelectDifficulty(DifficultyID);
		ForceRebuildSlate();
	}

	return FReply::Handled();
}

void UT66MiniDifficultySelectScreen::RefreshSelectedHeroBrush(const FT66MiniHeroDefinition* Hero)
{
	if (!SelectedHeroBrush.IsValid())
	{
		SelectedHeroBrush = MakeShared<FSlateBrush>();
		SelectedHeroBrush->DrawAs = ESlateBrushDrawType::Image;
		SelectedHeroBrush->ImageSize = FVector2D(260.f, 260.f);
	}

	SelectedHeroBrush->SetResourceObject(nullptr);
	if (!Hero)
	{
		return;
	}

	if (UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr)
	{
		if (UTexture2D* HeroTexture = VisualSubsystem->LoadHeroTexture(Hero->DisplayName))
		{
			SelectedHeroBrush->SetResourceObject(HeroTexture);
		}
	}
}

FString UT66MiniDifficultySelectScreen::BuildSessionUiStateKey() const
{
	const UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (!SessionSubsystem)
	{
		return FString();
	}

	TArray<FT66LobbyPlayerInfo> LobbyProfiles;
	SessionSubsystem->GetCurrentLobbyProfiles(LobbyProfiles);
	return FString::Printf(
		TEXT("%d|%d|%d|%d|%d|%d"),
		static_cast<int32>(SessionSubsystem->GetDesiredPartyFrontendScreen()),
		SessionSubsystem->IsPartyLobbyContextActive() ? 1 : 0,
		SessionSubsystem->IsLocalPlayerPartyHost() ? 1 : 0,
		SessionSubsystem->IsLocalLobbyReady() ? 1 : 0,
		SessionSubsystem->IsPartySessionActive() ? 1 : 0,
		LobbyProfiles.Num());
}
