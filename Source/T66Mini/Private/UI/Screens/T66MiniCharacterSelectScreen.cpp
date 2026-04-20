// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniCharacterSelectScreen.h"

#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Engine/Texture2D.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "Save/T66MiniProfileSaveGame.h"
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
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

UT66MiniCharacterSelectScreen::UT66MiniCharacterSelectScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MiniCharacterSelect;
	bIsModal = false;
}

void UT66MiniCharacterSelectScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionStateChangedHandle = SessionSubsystem->OnSessionStateChanged().AddUObject(this, &UT66MiniCharacterSelectScreen::HandleSessionStateChanged);
			SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::MiniCharacterSelect, true);
			LastSessionUiStateKey = BuildSessionUiStateKey();
		}
	}

	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	if (!DataSubsystem || !FrontendState)
	{
		return;
	}

	if (!FrontendState->HasSelectedHero() && DataSubsystem->GetHeroes().Num() > 0)
	{
		FrontendState->SelectHero(DataSubsystem->GetHeroes()[0].HeroID);
	}
}

void UT66MiniCharacterSelectScreen::OnScreenDeactivated_Implementation()
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

void UT66MiniCharacterSelectScreen::NativeDestruct()
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

void UT66MiniCharacterSelectScreen::HandleSessionStateChanged()
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

void UT66MiniCharacterSelectScreen::SyncToSharedPartyScreen()
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

TSharedRef<SWidget> UT66MiniCharacterSelectScreen::BuildSlateUI()
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	const TArray<FT66MiniHeroDefinition> Heroes = DataSubsystem ? DataSubsystem->GetHeroes() : TArray<FT66MiniHeroDefinition>();
	const FT66MiniHeroDefinition* SelectedHero = (DataSubsystem && FrontendState) ? DataSubsystem->FindHero(FrontendState->GetSelectedHeroID()) : nullptr;
	if (!SelectedHero && Heroes.Num() > 0)
	{
		SelectedHero = &Heroes[0];
	}

	LastSessionUiStateKey = BuildSessionUiStateKey();

	RebuildHeroSpriteBrushes(Heroes);
	const FSlateBrush* SelectedSprite = SelectedHero ? FindHeroSpriteBrush(SelectedHero->HeroID) : nullptr;
	const UT66MiniProfileSaveGame* ProfileSave = (SaveSubsystem && DataSubsystem) ? SaveSubsystem->LoadOrCreateProfileSave(DataSubsystem) : nullptr;

	const FLinearColor BackgroundFill(0.11f, 0.12f, 0.16f, 1.0f);
	const FLinearColor ScreenTint(0.02f, 0.03f, 0.05f, 0.20f);
	const FLinearColor PanelFill(0.07f, 0.08f, 0.11f, 0.97f);
	const FLinearColor PanelOutline(0.55f, 0.56f, 0.62f, 0.95f);
	const FLinearColor SelectedFill(0.33f, 0.30f, 0.14f, 0.95f);
	const FLinearColor SelectedBorder(0.82f, 0.78f, 0.47f, 1.0f);
	const FLinearColor BodyText(0.91f, 0.92f, 0.95f, 1.0f);
	const FLinearColor MutedText(0.79f, 0.82f, 0.88f, 1.0f);
	const FLinearColor RecordText(0.94f, 0.90f, 0.72f, 1.0f);
	const FLinearColor ButtonText(0.94f, 0.94f, 0.92f, 1.0f);

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

	auto MakeSpriteWidget = [&](const FSlateBrush* SpriteBrush, const FString& FallbackText, const FLinearColor& PlaceholderColor, const float Width, const float Height, const int32 FontSize) -> TSharedRef<SWidget>
	{
		return SpriteBrush && SpriteBrush->GetResourceObject()
			? StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.WidthOverride(Width)
				.HeightOverride(Height)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					[
						SNew(SImage)
						.Image(SpriteBrush)
					]
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBorder)
				.BorderImage(T66MiniUI::WhiteBrush())
				.BorderBackgroundColor(PlaceholderColor)
				.Padding(FMargin(8.f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(FallbackText))
					.Font(T66MiniUI::BoldFont(FontSize))
					.ColorAndOpacity(FLinearColor::White)
					.Justification(ETextJustify::Center)
				]);
	};

	auto MakeStatChip = [&](const FString& Label, const FString& Value, const FLinearColor& AccentColor) -> TSharedRef<SWidget>
	{
		return MakeFramedPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Font(T66MiniUI::BodyFont(11))
				.ColorAndOpacity(MutedText)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Value))
				.Font(T66MiniUI::BoldFont(16))
				.ColorAndOpacity(AccentColor)
			],
			FLinearColor(0.20f, 0.21f, 0.26f, 1.0f),
			FLinearColor(0.05f, 0.06f, 0.08f, 1.0f),
			FMargin(10.f, 8.f));
	};

	TSharedRef<SUniformGridPanel> StatWrap = SNew(SUniformGridPanel).SlotPadding(FMargin(10.f, 10.f, 10.f, 10.f));

	if (SelectedHero)
	{
		StatWrap->AddSlot(0, 0)[MakeStatChip(TEXT("Attack Family"), SelectedHero->PrimaryCategory, RecordText)];
		StatWrap->AddSlot(1, 0)[MakeStatChip(TEXT("Damage"), FString::Printf(TEXT("%.0f"), SelectedHero->BaseDamage), FLinearColor::White)];
		StatWrap->AddSlot(2, 0)[MakeStatChip(TEXT("Attack Speed"), FString::Printf(TEXT("%.0f"), SelectedHero->BaseAttackSpeed), FLinearColor::White)];
		StatWrap->AddSlot(0, 1)[MakeStatChip(TEXT("Armor"), FString::Printf(TEXT("%.0f"), SelectedHero->BaseArmor), FLinearColor::White)];
		StatWrap->AddSlot(1, 1)[MakeStatChip(TEXT("Luck"), FString::Printf(TEXT("%.0f"), SelectedHero->BaseLuck), FLinearColor::White)];
		StatWrap->AddSlot(2, 1)[MakeStatChip(TEXT("Speed"), FString::Printf(TEXT("%.0f"), SelectedHero->BaseSpeed), FLinearColor::White)];
	}

	TSharedRef<SUniformGridPanel> HeroGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(9.f, 9.f, 9.f, 9.f));
	const int32 ColumnCount = 8;
	for (int32 Index = 0; Index < Heroes.Num(); ++Index)
	{
		const FT66MiniHeroDefinition& Hero = Heroes[Index];
		const bool bIsSelected = SelectedHero && SelectedHero->HeroID == Hero.HeroID;
		const FSlateBrush* HeroBrush = FindHeroSpriteBrush(Hero.HeroID);

		HeroGrid->AddSlot(Index % ColumnCount, Index / ColumnCount)
		[
			SNew(SBox)
			.WidthOverride(126.f)
			.HeightOverride(102.f)
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateLambda([this, HeroID = Hero.HeroID]() { return HandleHeroClicked(HeroID); }))
				.ButtonColorAndOpacity(bIsSelected ? SelectedFill : PanelFill)
				.ContentPadding(FMargin(8.f, 7.f))
				[
					SNew(SBorder)
					.BorderImage(T66MiniUI::WhiteBrush())
					.BorderBackgroundColor(bIsSelected ? SelectedBorder : PanelOutline)
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(T66MiniUI::WhiteBrush())
						.BorderBackgroundColor(FLinearColor(0.04f, 0.05f, 0.07f, 1.0f))
						.Padding(FMargin(8.f, 4.f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
							[
								MakeSpriteWidget(HeroBrush, Hero.DisplayName.Left(1).ToUpper(), Hero.PlaceholderColor, 60.f, 60.f, 22)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 3.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(Hero.DisplayName))
								.Font(T66MiniUI::BoldFont(11))
								.ColorAndOpacity(bIsSelected ? FLinearColor(0.10f, 0.11f, 0.06f, 1.0f) : BodyText)
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
							]
						]
					]
				]
			]
		];
	}

	const TSharedRef<SWidget> SelectedSpriteWidget = MakeSpriteWidget(
		SelectedSprite,
		SelectedHero ? SelectedHero->DisplayName.Left(1).ToUpper() : FString(TEXT("?")),
		SelectedHero ? SelectedHero->PlaceholderColor : T66MiniUI::RaisedFill(),
		214.f,
		214.f,
		42);

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
							.Text(NSLOCTEXT("T66Mini.CharacterSelect", "Title", "Character Selection"))
							.Font(T66MiniUI::TitleFont(30))
							.ColorAndOpacity(FLinearColor::White)
							.ShadowOffset(FVector2D(2.f, 2.f))
							.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.95f))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 18.f, 0.f)
							[
								MakeFramedPanel(
									SNew(SBox)
									.HeightOverride(294.f)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight()
										[
											SNew(STextBlock)
											.Text(FText::FromString(SelectedHero ? SelectedHero->DisplayName : FString(TEXT("No Hero Selected"))))
											.Font(T66MiniUI::TitleFont(24))
											.ColorAndOpacity(FLinearColor::White)
											.ShadowOffset(FVector2D(1.5f, 1.5f))
											.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 14.f)
										[
											SNew(STextBlock)
											.Text(FText::FromString(SelectedHero
												? FString::Printf(TEXT("%s specialist"), *SelectedHero->PrimaryCategory)
												: TEXT("Select a hero to continue.")))
											.Font(T66MiniUI::BoldFont(16))
											.ColorAndOpacity(RecordText)
										]
										+ SVerticalBox::Slot().FillHeight(1.f)
										[
											SNew(SHorizontalBox)
											+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(0.f, 0.f, 18.f, 0.f)
											[
												MakeFramedPanel(
													SNew(SBox)
													.WidthOverride(240.f)
													.HeightOverride(218.f)
													[
														SNew(SBorder)
														.BorderImage(T66MiniUI::WhiteBrush())
														.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.06f, 1.0f))
														.Padding(FMargin(12.f))
														[
															SNew(SBox)
															.WidthOverride(214.f)
															.HeightOverride(214.f)
															[
																SelectedSpriteWidget
															]
														]
													],
													FLinearColor(0.08f, 0.08f, 0.08f, 1.0f),
													FLinearColor(0.06f, 0.06f, 0.06f, 1.0f),
													FMargin(2.f))
											]
											+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Fill)
											[
												SNew(SVerticalBox)
												+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 12.f)
												[
													SNew(STextBlock)
													.Text(FText::FromString(SelectedHero ? SelectedHero->Description : FString(TEXT("Select a hero to inspect their run profile."))))
													.Font(T66MiniUI::BodyFont(15))
													.ColorAndOpacity(BodyText)
													.AutoWrapText(true)
												]
												+ SVerticalBox::Slot().AutoHeight()
												[
													StatWrap
												]
											]
										]
									],
									PanelOutline,
									PanelFill,
									FMargin(18.f, 16.f))
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeFramedPanel(
									SNew(SBox)
									.WidthOverride(278.f)
									.HeightOverride(294.f)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight()
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("T66Mini.CharacterSelect", "RecordsTitle", "Records"))
											.Font(T66MiniUI::TitleFont(20))
											.ColorAndOpacity(FLinearColor::White)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 10.f)
										[
											SNew(STextBlock)
											.Text(FText::FromString(FString::Printf(TEXT("Best wave reached\nWave %d"), ProfileSave ? ProfileSave->BestWaveReached : 1)))
											.Font(T66MiniUI::BodyFont(14))
											.ColorAndOpacity(RecordText)
											.Justification(ETextJustify::Center)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
										[
											SNew(STextBlock)
											.Text(FText::FromString(ProfileSave && ProfileSave->BestClearSeconds > 0.f ? FString::Printf(TEXT("Fastest clear\n%.1fs"), ProfileSave->BestClearSeconds) : TEXT("Fastest clear\nNo clear yet")))
											.Font(T66MiniUI::BodyFont(14))
											.ColorAndOpacity(RecordText)
											.Justification(ETextJustify::Center)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
										[
											SNew(STextBlock)
											.Text(FText::FromString(FString::Printf(TEXT("Completed runs\n%d"), ProfileSave ? ProfileSave->CompletedRunCount : 0)))
											.Font(T66MiniUI::BodyFont(14))
											.ColorAndOpacity(RecordText)
											.Justification(ETextJustify::Center)
										]
										+ SVerticalBox::Slot().AutoHeight()
										[
											SNew(STextBlock)
											.Text(FText::FromString(FString::Printf(TEXT("Failed runs\n%d"), ProfileSave ? ProfileSave->FailedRunCount : 0)))
											.Font(T66MiniUI::BodyFont(14))
											.ColorAndOpacity(RecordText)
											.Justification(ETextJustify::Center)
										]
										+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Bottom)
										[
											SNew(STextBlock)
											.Text(FText::FromString(FString::Printf(TEXT("Unlocked heroes\n%d / %d"), ProfileSave ? ProfileSave->UnlockedHeroIDs.Num() : Heroes.Num(), Heroes.Num())))
											.Font(T66MiniUI::BodyFont(13))
											.ColorAndOpacity(MutedText)
											.Justification(ETextJustify::Center)
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
								+ SScrollBox::Slot()[HeroGrid],
								PanelOutline,
								PanelFill,
								FMargin(18.f, 16.f, 18.f, 52.f))
						]
					]
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(20.f, 18.f, 0.f, 0.f)
		[
			SNew(SBox)
			.WidthOverride(122.f)
			.HeightOverride(40.f)
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniCharacterSelectScreen::HandleBackClicked))
				.ButtonColorAndOpacity(FLinearColor(0.12f, 0.12f, 0.12f, 0.95f))
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Mini.CharacterSelect", "Back", "Back"))
					.Font(T66MiniUI::BoldFont(17))
					.ColorAndOpacity(ButtonText)
					.Justification(ETextJustify::Center)
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 22.f, 18.f)
		[
			SNew(SBox)
			.WidthOverride(188.f)
			.HeightOverride(46.f)
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniCharacterSelectScreen::HandleContinueClicked))
				.ButtonColorAndOpacity(FLinearColor(0.27f, 0.30f, 0.14f, 0.94f))
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Mini.CharacterSelect", "Continue", "CONTINUE"))
					.Font(T66MiniUI::BoldFont(17))
					.ColorAndOpacity(FLinearColor(0.89f, 0.89f, 0.82f, 1.0f))
					.Justification(ETextJustify::Center)
				]
			]
		];
}

FReply UT66MiniCharacterSelectScreen::HandleBackClicked()
{
	NavigateBack();
	return FReply::Handled();
}

FReply UT66MiniCharacterSelectScreen::HandleContinueClicked()
{
	NavigateTo(ET66ScreenType::MiniCompanionSelect);
	return FReply::Handled();
}

FReply UT66MiniCharacterSelectScreen::HandleHeroClicked(const FName HeroID)
{
	if (UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr)
	{
		if (FrontendState->GetSelectedHeroID() == HeroID)
		{
			return FReply::Handled();
		}

		FrontendState->SelectHero(HeroID);
		ForceRebuildSlate();
	}

	return FReply::Handled();
}

void UT66MiniCharacterSelectScreen::RebuildHeroSpriteBrushes(const TArray<FT66MiniHeroDefinition>& Heroes)
{
	HeroSpriteBrushes.Reset();

	UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	for (const FT66MiniHeroDefinition& Hero : Heroes)
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->ImageSize = FVector2D(220.f, 220.f);
		Brush->SetResourceObject(nullptr);

		if (VisualSubsystem)
		{
			if (UTexture2D* HeroTexture = VisualSubsystem->LoadHeroTexture(Hero.DisplayName))
			{
				Brush->SetResourceObject(HeroTexture);
			}
		}

		HeroSpriteBrushes.Add(Hero.HeroID, Brush);
	}
}

FString UT66MiniCharacterSelectScreen::BuildSessionUiStateKey() const
{
	const UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (!SessionSubsystem)
	{
		return FString();
	}

	TArray<FT66LobbyPlayerInfo> LobbyProfiles;
	SessionSubsystem->GetCurrentLobbyProfiles(LobbyProfiles);
	return FString::Printf(
		TEXT("%d|%d|%d|%d|%d"),
		static_cast<int32>(SessionSubsystem->GetDesiredPartyFrontendScreen()),
		SessionSubsystem->IsPartyLobbyContextActive() ? 1 : 0,
		SessionSubsystem->IsLocalPlayerPartyHost() ? 1 : 0,
		SessionSubsystem->IsLocalLobbyReady() ? 1 : 0,
		LobbyProfiles.Num());
}

const FSlateBrush* UT66MiniCharacterSelectScreen::FindHeroSpriteBrush(const FName HeroID) const
{
	if (const TSharedPtr<FSlateBrush>* Found = HeroSpriteBrushes.Find(HeroID))
	{
		return Found->Get();
	}

	return nullptr;
}
