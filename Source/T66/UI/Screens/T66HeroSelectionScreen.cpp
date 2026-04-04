// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66PowerUpSubsystem.h"
#include "Core/T66SkinSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66FrontendGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Images/SImage.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "FileMediaSource.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66HeroSelection, Log, All);

namespace
{
	FText HeroRecordMedalText(ET66AccountMedalTier Tier)
	{
		switch (Tier)
		{
		case ET66AccountMedalTier::Bronze:
			return NSLOCTEXT("T66.HeroSelection", "MedalBronze", "Bronze");
		case ET66AccountMedalTier::Silver:
			return NSLOCTEXT("T66.HeroSelection", "MedalSilver", "Silver");
		case ET66AccountMedalTier::Gold:
			return NSLOCTEXT("T66.HeroSelection", "MedalGold", "Gold");
		case ET66AccountMedalTier::Platinum:
			return NSLOCTEXT("T66.HeroSelection", "MedalPlatinum", "Platinum");
		case ET66AccountMedalTier::Diamond:
			return NSLOCTEXT("T66.HeroSelection", "MedalDiamond", "Diamond");
		case ET66AccountMedalTier::None:
		default:
			return NSLOCTEXT("T66.HeroSelection", "MedalNone", "Unproven");
		}
	}

	FLinearColor HeroRecordMedalColor(ET66AccountMedalTier Tier)
	{
		switch (Tier)
		{
		case ET66AccountMedalTier::Bronze:
			return FLinearColor(0.67f, 0.43f, 0.26f, 1.0f);
		case ET66AccountMedalTier::Silver:
			return FLinearColor(0.76f, 0.79f, 0.84f, 1.0f);
		case ET66AccountMedalTier::Gold:
			return FLinearColor(0.89f, 0.74f, 0.27f, 1.0f);
		case ET66AccountMedalTier::Platinum:
			return FLinearColor(0.56f, 0.77f, 0.88f, 1.0f);
		case ET66AccountMedalTier::Diamond:
			return FLinearColor(0.45f, 0.86f, 0.99f, 1.0f);
		case ET66AccountMedalTier::None:
		default:
			return FLinearColor(0.74f, 0.74f, 0.74f, 1.0f);
		}
	}

	class ST66DragRotatePreview : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66DragRotatePreview) {}
			SLATE_ARGUMENT(TWeakObjectPtr<AT66HeroPreviewStage>, Stage)
			SLATE_ARGUMENT(float, DegreesPerPixel)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			Stage = InArgs._Stage;
			DegreesPerPixel = InArgs._DegreesPerPixel;
			bDragging = false;
			// No child content — transparent overlay for drag-rotate/zoom.
			// The viewport renders the 3D character directly behind this widget.
		}

		virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				bDragging = true;
				LastPos = MouseEvent.GetScreenSpacePosition();
				return FReply::Handled().CaptureMouse(SharedThis(this));
			}
			return FReply::Unhandled();
		}

		virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (bDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				bDragging = false;
				return FReply::Handled().ReleaseMouseCapture();
			}
			return FReply::Unhandled();
		}

		virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (!bDragging)
			{
				return FReply::Unhandled();
			}

			const FVector2D Pos = MouseEvent.GetScreenSpacePosition();
			const FVector2D Delta = Pos - LastPos;
			LastPos = Pos;

			if (Stage.IsValid())
			{
				// Rotate the hero without orbiting the camera (keeps the platform visually stable).
				Stage->AddPreviewYaw(Delta.X * DegreesPerPixel);
			}
			return FReply::Handled();
		}

		virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (Stage.IsValid())
			{
				Stage->AddPreviewZoom(MouseEvent.GetWheelDelta());
				if (UWorld* World = Stage->GetWorld())
				{
					if (AT66FrontendGameMode* GM = Cast<AT66FrontendGameMode>(World->GetAuthGameMode()))
					{
						GM->PositionCameraForHeroPreview();
					}
				}
				return FReply::Handled();
			}
			return FReply::Unhandled();
		}

	private:
		TWeakObjectPtr<AT66HeroPreviewStage> Stage;
		bool bDragging = false;
		FVector2D LastPos = FVector2D::ZeroVector;
		float DegreesPerPixel = 0.25f;
	};
}

UT66HeroSelectionScreen::UT66HeroSelectionScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::HeroSelection;
	bIsModal = false;
	KnightPreviewMediaSourceAsset = TSoftObjectPtr<UFileMediaSource>(FSoftObjectPath(TEXT("/Game/Characters/Heroes/Knight/KnightClip.KnightClip")));
}

UT66LocalizationSubsystem* UT66HeroSelectionScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

void UT66HeroSelectionScreen::GeneratePlaceholderSkins()
{
	PlaceholderSkins.Empty();
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66SkinSubsystem* Skin = GI ? GI->GetSubsystem<UT66SkinSubsystem>() : nullptr;
	if (Skin && !PreviewedHeroID.IsNone())
	{
		PlaceholderSkins = Skin->GetSkinsForEntity(ET66SkinEntityType::Hero, PreviewedHeroID);
	}
	else
	{
		// No subsystem or no hero: minimal list
		for (FName SkinID : UT66SkinSubsystem::GetAllSkinIDs())
		{
			FSkinData S;
			S.SkinID = SkinID;
			S.bIsDefault = (SkinID == UT66SkinSubsystem::DefaultSkinID);
			S.bIsOwned = S.bIsDefault;
			S.bIsEquipped = S.bIsDefault;
			S.CoinCost = S.bIsDefault ? 0 : UT66SkinSubsystem::DefaultSkinPriceAC;
			PlaceholderSkins.Add(S);
		}
	}
}

void UT66HeroSelectionScreen::RefreshSkinsList()
{
	GeneratePlaceholderSkins();
	if (SkinsListBoxWidget.IsValid())
	{
		SkinsListBoxWidget->ClearChildren();
		AddSkinRowsToBox(SkinsListBoxWidget);
	}
	// Update AC balance display
	if (ACBalanceTextBlock.IsValid())
	{
		int32 ACBalance = 0;
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66SkinSubsystem* Skin = GI->GetSubsystem<UT66SkinSubsystem>())
			{
				ACBalance = Skin->GetAchievementCoinsBalance();
			}
		}
		UT66LocalizationSubsystem* Loc = GetLocSubsystem();
		const FText ACBalanceText = Loc
			? FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(ACBalance))
			: FText::Format(NSLOCTEXT("T66.Achievements", "AchievementCoinsFormatFallback", "{0} AC"), FText::AsNumber(ACBalance));
		ACBalanceTextBlock->SetText(ACBalanceText);
	}
	UpdateHeroDisplay();
}

void UT66HeroSelectionScreen::AddSkinRowsToBox(const TSharedPtr<SVerticalBox>& Box)
{
	if (!Box.IsValid()) return;
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText EquipText = Loc ? Loc->GetText_Equip() : NSLOCTEXT("T66.Common", "Equip", "EQUIP");
	const FText EquippedText = NSLOCTEXT("T66.HeroSelection", "Equipped", "EQUIPPED");
	const FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");
	const FText BuyText = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	const FButtonStyle& PrimaryButtonStyle = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary");
	const float ActionMinHeight = 34.f;
	const float ActionMinWidth = 132.f;
	const float EquippedMinWidth = 138.f;
	const float BuyButtonMinWidth = 138.f;
	const float BuyButtonHeight = 42.f;
	const int32 SkinActionFontSize = 7;
	const int32 SkinPriceFontSize = 7;
	const int32 SkinTitleFontSize = 9;
	for (const FSkinData& Skin : PlaceholderSkins)
	{
		FText SkinDisplayName = Loc ? Loc->GetText_SkinName(Skin.SkinID) : FText::FromName(Skin.SkinID);
		const int32 Price = FMath::Max(0, Skin.CoinCost);
		const FText PriceText = Loc
			? FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(Price))
			: FText::Format(NSLOCTEXT("T66.Achievements", "AchievementCoinsFormatFallback", "{0} AC"), FText::AsNumber(Price));
		const FName SkinIDCopy = Skin.SkinID;
		const bool bIsDefault = Skin.bIsDefault;
		const bool bIsOwned = Skin.bIsOwned;
		const bool bIsEquipped = Skin.bIsEquipped;
		const bool bIsBeachgoer = (SkinIDCopy == FName(TEXT("Beachgoer")));

		TSharedRef<SHorizontalBox> ButtonRow = SNew(SHorizontalBox);
		if (bIsDefault)
		{
			ButtonRow->AddSlot().AutoWidth().Padding(4.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(EquippedMinWidth).MinDesiredHeight(ActionMinHeight)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(bIsEquipped ? 1 : 0)
						+ SWidgetSwitcher::Slot()
						[
							FT66Style::MakeButton(FT66ButtonParams(EquipText,
							FOnClicked::CreateLambda([this, SkinIDCopy]()
							{
								if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
								{
									if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
									{
										FName HeroID = PreviewedHeroID.IsNone() && AllHeroIDs.Num() > 0 ? AllHeroIDs[0] : PreviewedHeroID;
										if (!HeroID.IsNone())
										{
											SkinSub->SetEquippedHeroSkinID(HeroID, SkinIDCopy);
											if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
											{
												T66GI->SelectedHeroSkinID = SkinIDCopy;
											}
											PreviewSkinIDOverride = NAME_None;
											RefreshSkinsList();
										}
									}
								}
								return FReply::Handled();
							}),
							ET66ButtonType::Primary)
							.SetMinWidth(ActionMinWidth)
							.SetHeight(ActionMinHeight)
							.SetPadding(FMargin(8.f, 4.f))
							.SetContent(SNew(STextBlock).Text(EquipText).Font(FT66Style::Tokens::FontBold(SkinActionFontSize)).ColorAndOpacity(FT66Style::Tokens::Text).Justification(ETextJustify::Center))
						)
						]
						+ SWidgetSwitcher::Slot()
						[
							SNew(SBox).MinDesiredWidth(EquippedMinWidth).HeightOverride(ActionMinHeight)
							[
								SNew(SBorder)
								.BorderImage(&PrimaryButtonStyle.Normal)
								.BorderBackgroundColor(FT66Style::IsDotaTheme()
									? FSlateColor(FLinearColor(0.075f, 0.075f, 0.08f, 1.0f))
									: FT66Style::Tokens::Accent2)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.Padding(FMargin(10.0f, 4.0f))
								[
									SNew(STextBlock)
									.Text(EquippedText)
									.Font(FT66Style::Tokens::FontBold(SkinActionFontSize))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center)
								]
							]
						]
					]
				];
		}
		if (bIsBeachgoer)
		{
			ButtonRow->AddSlot().AutoWidth().Padding(4.0f, 0.0f)
				[
					FT66Style::MakeButton(FT66ButtonParams(PreviewText,
					FOnClicked::CreateLambda([this, SkinIDCopy]()
					{
						if (PreviewSkinIDOverride == SkinIDCopy)
						{
							PreviewSkinIDOverride = NAME_None;
						}
						else
						{
							PreviewSkinIDOverride = SkinIDCopy;
						}
						UpdateHeroDisplay();
						return FReply::Handled();
					}),
					ET66ButtonType::Neutral)
					.SetMinWidth(ActionMinWidth)
					.SetHeight(ActionMinHeight)
					.SetPadding(FMargin(8.f, 4.f))
					.SetContent(SNew(STextBlock).Text(PreviewText).Font(FT66Style::Tokens::FontBold(SkinActionFontSize)).ColorAndOpacity(FT66Style::Tokens::Text).Justification(ETextJustify::Center))
				)
				];
			ButtonRow->AddSlot().AutoWidth().Padding(4.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(EquippedMinWidth).MinDesiredHeight(BuyButtonHeight)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(!bIsOwned ? 0 : (bIsEquipped ? 2 : 1))
						+ SWidgetSwitcher::Slot()
						[
						FT66Style::MakeButton(FT66ButtonParams(BuyText,
							FOnClicked::CreateLambda([this, SkinIDCopy, Price]()
							{
								FName HeroID = PreviewedHeroID.IsNone() && AllHeroIDs.Num() > 0 ? AllHeroIDs[0] : PreviewedHeroID;
								if (HeroID.IsNone()) return FReply::Handled();
								UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
								if (!GI) return FReply::Handled();
								UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>();
								if (!SkinSub) return FReply::Handled();
								if (!SkinSub->PurchaseHeroSkin(HeroID, SkinIDCopy, Price)) return FReply::Handled();
								SkinSub->SetEquippedHeroSkinID(HeroID, SkinIDCopy);
								if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
								{
									T66GI->SelectedHeroSkinID = SkinIDCopy;
								}
								PreviewSkinIDOverride = NAME_None;
								RefreshSkinsList();
								return FReply::Handled();
							}),
							ET66ButtonType::Primary)
							.SetMinWidth(BuyButtonMinWidth)
							.SetHeight(BuyButtonHeight)
							.SetColor(FT66Style::Tokens::Accent)
							.SetPadding(FMargin(12.f, 6.f, 12.f, 4.f))
							.SetContent(
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ SNew(STextBlock).Text(BuyText).Font(FT66Style::Tokens::FontBold(SkinActionFontSize)).ColorAndOpacity(FT66Style::Tokens::Text).Justification(ETextJustify::Center) ]
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ SNew(STextBlock).Text(PriceText).Font(FT66Style::Tokens::FontRegular(SkinPriceFontSize)).ColorAndOpacity(FT66Style::Tokens::Text).Justification(ETextJustify::Center) ]
							)
						)
						]
						+ SWidgetSwitcher::Slot()
						[
						FT66Style::MakeButton(FT66ButtonParams(EquipText,
							FOnClicked::CreateLambda([this, SkinIDCopy]()
							{
								FName HeroID = PreviewedHeroID.IsNone() && AllHeroIDs.Num() > 0 ? AllHeroIDs[0] : PreviewedHeroID;
								if (HeroID.IsNone()) return FReply::Handled();
								if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
								{
									if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
									{
										SkinSub->SetEquippedHeroSkinID(HeroID, SkinIDCopy);
										if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
										{
											T66GI->SelectedHeroSkinID = SkinIDCopy;
										}
										PreviewSkinIDOverride = NAME_None;
										RefreshSkinsList();
									}
								}
								return FReply::Handled();
							}),
							ET66ButtonType::Primary)
							.SetMinWidth(ActionMinWidth)
							.SetHeight(ActionMinHeight)
							.SetPadding(FMargin(8.f, 4.f))
							.SetContent(SNew(STextBlock).Text(EquipText).Font(FT66Style::Tokens::FontBold(SkinActionFontSize)).ColorAndOpacity(FT66Style::Tokens::Text).Justification(ETextJustify::Center))
						)
						]
						+ SWidgetSwitcher::Slot()
						[
							SNew(SBox).MinDesiredWidth(EquippedMinWidth).HeightOverride(ActionMinHeight)
							[
								SNew(SBorder)
								.BorderImage(&PrimaryButtonStyle.Normal)
								.BorderBackgroundColor(FT66Style::IsDotaTheme()
									? FSlateColor(FLinearColor(0.075f, 0.075f, 0.08f, 1.0f))
									: FT66Style::Tokens::Accent2)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.Padding(FMargin(10.0f, 4.0f))
								[
									SNew(STextBlock)
									.Text(EquippedText)
									.Font(FT66Style::Tokens::FontBold(SkinActionFontSize))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center)
								]
							]
						]
					]
				];
		}

		Box->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 6.0f)
			[
				FT66Style::MakePanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(SkinDisplayName)
						.Font(FT66Style::Tokens::FontRegular(SkinTitleFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(5.0f, 0.0f, 0.0f, 0.0f)
					[
						ButtonRow
					],
					FT66PanelParams(ET66PanelType::Panel2)
						.SetColor(FT66Style::IsDotaTheme()
							? FSlateColor(FLinearColor(0.028f, 0.028f, 0.031f, 1.0f))
							: FT66Style::Tokens::Panel2)
						.SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3)))
			];
	}
}

void UT66HeroSelectionScreen::OnDifficultyChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (!NewValue.IsValid()) return;
	
	// Find which difficulty this corresponds to
	int32 Index = DifficultyOptions.IndexOfByKey(NewValue);
	if (Index != INDEX_NONE)
	{
		TArray<ET66Difficulty> Difficulties = {
			ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard,
			ET66Difficulty::VeryHard, ET66Difficulty::Impossible
		};
		if (Index < Difficulties.Num())
		{
			SelectedDifficulty = Difficulties[Index];
			CurrentDifficultyOption = NewValue;
			if (AT66HeroPreviewStage* PreviewStage = GetHeroPreviewStage())
			{
				PreviewStage->SetPreviewDifficulty(SelectedDifficulty);
			}
		}
	}
}

TSharedRef<SWidget> UT66HeroSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	// Ensure hero list and skin state so skin list and 3D preview match (BuildSlateUI can run before OnScreenActivated).
	RefreshHeroList();
	if (AllHeroIDs.Num() > 0 && PreviewedHeroID.IsNone())
	{
		PreviewedHeroID = AllHeroIDs[0];
	}
	UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] BuildSlateUI: PreviewedHeroID=%s"), *PreviewedHeroID.ToString());
	if (!PreviewedHeroID.IsNone())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
			{
				// Check if the current SelectedHeroSkinID is owned by this hero.
				// If not, reset to this hero's equipped skin (or Default).
				const FName CurrentSkin = GI->SelectedHeroSkinID;
				UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] BuildSlateUI: CurrentSkin (GI->SelectedHeroSkinID) = %s"), *CurrentSkin.ToString());
				
				const bool bIsNone = CurrentSkin.IsNone();
				const bool bIsDefault = CurrentSkin == FName(TEXT("Default"));
				const bool bIsOwned = SkinSub->IsHeroSkinOwned(PreviewedHeroID, CurrentSkin);
				UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] BuildSlateUI: bIsNone=%d, bIsDefault=%d, bIsOwned=%d"),
					bIsNone ? 1 : 0, bIsDefault ? 1 : 0, bIsOwned ? 1 : 0);
				
				const bool bCurrentSkinOwned = bIsNone || bIsDefault || bIsOwned;
				if (!bCurrentSkinOwned)
				{
					const FName NewSkin = SkinSub->GetEquippedHeroSkinID(PreviewedHeroID);
					UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] BuildSlateUI: %s does NOT own %s, switching to equipped: %s"),
						*PreviewedHeroID.ToString(), *CurrentSkin.ToString(), *NewSkin.ToString());
					GI->SelectedHeroSkinID = NewSkin;
				}
				else
				{
					UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] BuildSlateUI: %s OWNS %s (or is Default/None), keeping it"),
						*PreviewedHeroID.ToString(), *CurrentSkin.ToString());
				}
				if (GI->SelectedHeroSkinID.IsNone())
				{
					GI->SelectedHeroSkinID = FName(TEXT("Default"));
				}
				UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] BuildSlateUI: final GI->SelectedHeroSkinID = %s"), *GI->SelectedHeroSkinID.ToString());
			}
		}
	}
	GeneratePlaceholderSkins();
	EnsureHeroPreviewVideoResources();

	// Get localized text
	FText HeroGridText = Loc ? Loc->GetText_HeroGrid() : NSLOCTEXT("T66.HeroSelection", "HeroGrid", "HERO GRID");
	FText ChooseCompanionText = Loc ? Loc->GetText_ChooseCompanion() : NSLOCTEXT("T66.HeroSelection", "ChooseCompanion", "CHOOSE COMPANION");
	FText SkinsText = Loc ? Loc->GetText_Skins() : NSLOCTEXT("T66.HeroSelection", "Skins", "SKINS");
	FText HeroInfoText = Loc ? Loc->GetText_HeroInfo() : NSLOCTEXT("T66.HeroSelection", "HeroInfo", "HERO INFO");
	FText LoreText = Loc ? Loc->GetText_Lore() : NSLOCTEXT("T66.HeroSelection", "Lore", "LORE");
	FText DemoText = Loc ? Loc->GetText_Demo() : NSLOCTEXT("T66.HeroSelection", "Demo", "Demo");
	FText BackToLobbyText = Loc ? Loc->GetText_BackToLobby() : NSLOCTEXT("T66.Lobby", "BackToLobby", "BACK TO LOBBY");
	FText EnterText = Loc ? Loc->GetText_EnterTheTribulation() : NSLOCTEXT("T66.HeroSelection", "EnterTheTribulation", "ENTER THE TRIBULATION");
	FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	FText BuyText = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	FText EquipText = Loc ? Loc->GetText_Equip() : NSLOCTEXT("T66.Common", "Equip", "EQUIP");
	FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");

	bool bHideEnterFromLobby = false;
	if (UT66GameInstance* GIFlow = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		bHideEnterFromLobby = GIFlow->bHeroSelectionFromLobby;
	}

	// Initialize difficulty dropdown options
	DifficultyOptions.Empty();
	TArray<ET66Difficulty> Difficulties = {
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard,
		ET66Difficulty::VeryHard, ET66Difficulty::Impossible
	};
	for (ET66Difficulty Diff : Difficulties)
	{
		FText DiffText = Loc ? Loc->GetText_Difficulty(Diff) : NSLOCTEXT("T66.Difficulty", "Unknown", "???");
		DifficultyOptions.Add(MakeShared<FString>(DiffText.ToString()));
	}
	// Set current selection
	int32 CurrentDiffIndex = Difficulties.IndexOfByKey(SelectedDifficulty);
	if (CurrentDiffIndex != INDEX_NONE && CurrentDiffIndex < DifficultyOptions.Num())
	{
		CurrentDifficultyOption = DifficultyOptions[CurrentDiffIndex];
	}
	else if (DifficultyOptions.Num() > 0)
	{
		CurrentDifficultyOption = DifficultyOptions[0];
	}

	// AC balance (shown in the skins panel)
	int32 ACBalance = 0;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
		{
			ACBalance = SkinSub->GetAchievementCoinsBalance();
		}
	}
	const FText ACBalanceText = Loc
		? FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(ACBalance))
		: FText::Format(NSLOCTEXT("T66.Achievements", "AchievementCoinsFormatFallback", "{0} AC"), FText::AsNumber(ACBalance));

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66PartySubsystem* PartySubsystem = T66GI ? T66GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	const int32 ActivePartySlots = PartySubsystem ? FMath::Clamp(PartySubsystem->GetPartyMemberCount(), 1, 4) : 1;

	if (!PartyHeroPortraitBrush.IsValid())
	{
		PartyHeroPortraitBrush = MakeShared<FSlateBrush>();
	}
	PartyHeroPortraitBrush->DrawAs = ESlateBrushDrawType::Image;
	PartyHeroPortraitBrush->Tiling = ESlateBrushTileType::NoTile;
	PartyHeroPortraitBrush->ImageSize = FVector2D(76.f, 76.f);
	PartyHeroPortraitBrush->SetResourceObject(nullptr);

	if (T66GI && T66GI->GetSubsystem<UT66UITexturePoolSubsystem>())
	{
		FHeroData HeroData;
		TSoftObjectPtr<UTexture2D> HeroPortraitSoft;
		if (T66GI->GetHeroData(T66GI->SelectedHeroID, HeroData))
		{
			HeroPortraitSoft = T66GI->ResolveHeroPortrait(HeroData, T66GI->SelectedHeroBodyType, ET66HeroPortraitVariant::Half);
		}
		if (!HeroPortraitSoft.IsNull())
		{
			TArray<FSoftObjectPath> PortraitPaths;
			PortraitPaths.AddUnique(HeroPortraitSoft.ToSoftObjectPath());
			T66GI->GetSubsystem<UT66UITexturePoolSubsystem>()->EnsureTexturesLoadedSync(PortraitPaths);
		}
		PartyHeroPortraitBrush->SetResourceObject(T66SlateTexture::GetLoaded(T66GI->GetSubsystem<UT66UITexturePoolSubsystem>(), HeroPortraitSoft));
	}

	// Build skins list (Default + Beachgoer only). Refreshed in place via RefreshSkinsList() when Equip/Buy.
	SAssignNew(SkinsListBoxWidget, SVerticalBox);
	AddSkinRowsToBox(SkinsListBoxWidget);

	// Get current hero color for preview
	FLinearColor HeroPreviewColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f); // Default gray
	FHeroData CurrentHeroData;
	if (GetPreviewedHeroData(CurrentHeroData))
	{
		HeroPreviewColor = CurrentHeroData.PlaceholderColor;
	}

	// Build hero sprite carousel (colored boxes for each hero)
	TSharedRef<SHorizontalBox> HeroCarousel = SNew(SHorizontalBox);
	const bool bDotaTheme = FT66Style::IsDotaTheme();
	const FLinearColor SelectionShellFill = FLinearColor::Black;
	const FSlateColor SelectionPanelFill = FLinearColor(0.022f, 0.022f, 0.024f, 1.0f);
	const FSlateColor SelectionInsetFill = FLinearColor(0.032f, 0.032f, 0.036f, 1.0f);
	const FSlateColor SelectionToggleFill = FLinearColor(0.055f, 0.055f, 0.06f, 1.0f);
	RefreshHeroList(); // Ensure hero list is populated

	// Ensure we have stable brushes for the 5 visible slots.
	HeroCarouselPortraitBrushes.SetNum(5);
	HeroCarouselSlotColors.SetNum(5);
	HeroCarouselSlotVisibility.SetNum(5);
	for (int32 i = 0; i < HeroCarouselPortraitBrushes.Num(); ++i)
	{
		if (!HeroCarouselPortraitBrushes[i].IsValid())
		{
			HeroCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			HeroCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			HeroCarouselPortraitBrushes[i]->ImageSize = FVector2D(48.f, 48.f);
		}
	}
	RefreshHeroCarouselPortraits();
	
	// Show 5 heroes centered on current (prev2, prev1, current, next1, next2)
	for (int32 Offset = -2; Offset <= 2; Offset++)
	{
		if (AllHeroIDs.Num() == 0) break;

		const float BoxSize = (Offset == 0) ? 48.0f : 36.0f; // Current hero is larger
		const float Opacity = (Offset == 0) ? 1.0f : 0.6f;
		const int32 SlotIdx = Offset + 2;
		const TSharedRef<SWidget> CarouselSlotWidget = bDotaTheme
			? StaticCastSharedRef<SWidget>(FT66Style::MakeSlotFrame(
				SNew(SImage)
				.Image_Lambda([this, SlotIdx]() -> const FSlateBrush*
				{
					return HeroCarouselPortraitBrushes.IsValidIndex(SlotIdx) ? HeroCarouselPortraitBrushes[SlotIdx].Get() : nullptr;
				})
				.Visibility_Lambda([this, SlotIdx]() -> EVisibility
				{
					return HeroCarouselSlotVisibility.IsValidIndex(SlotIdx)
						? HeroCarouselSlotVisibility[SlotIdx]
						: EVisibility::Collapsed;
				})
				.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity)),
				TAttribute<FSlateColor>::CreateLambda([this, SlotIdx]() -> FSlateColor
				{
					return HeroCarouselSlotColors.IsValidIndex(SlotIdx)
						? FSlateColor(HeroCarouselSlotColors[SlotIdx])
						: FSlateColor(FT66Style::SlotInner());
				}),
				FMargin(2.f)))
			: StaticCastSharedRef<SWidget>(SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderBackgroundColor_Lambda([this, SlotIdx]() -> FSlateColor
					{
						return HeroCarouselSlotColors.IsValidIndex(SlotIdx)
							? FSlateColor(HeroCarouselSlotColors[SlotIdx])
							: FSlateColor(FLinearColor(0.2f, 0.2f, 0.25f, 1.0f));
					})
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				]
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image_Lambda([this, SlotIdx]() -> const FSlateBrush*
					{
						return HeroCarouselPortraitBrushes.IsValidIndex(SlotIdx) ? HeroCarouselPortraitBrushes[SlotIdx].Get() : nullptr;
					})
					.Visibility_Lambda([this, SlotIdx]() -> EVisibility
					{
						return HeroCarouselSlotVisibility.IsValidIndex(SlotIdx)
							? HeroCarouselSlotVisibility[SlotIdx]
							: EVisibility::Collapsed;
					})
					.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity))
				]);

		HeroCarousel->AddSlot()
		.AutoWidth()
		.Padding(4.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(BoxSize)
			.HeightOverride(BoxSize)
			[
				CarouselSlotWidget
			]
		];
	}

	const FTextBlockStyle& TxtButton = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button");
	const float CenterPanelFill = 0.40f;
	const float TopBarBottomGap = 0.f;
	const float ContentTopGap = 0.f;
	const float PanelBottomInset = 0.f;
	const float PanelGap = 0.f;
	const auto ShrinkFont = [](int32 BaseSize, int32 Minimum = 7) -> int32 { return FMath::Max(Minimum, BaseSize - 4); };
	const float SidePanelMinWidth = 380.f;
	const float TopBarSideWidth = 112.f;
	const float FooterToggleWidth = 98.f;
	const float FooterToggleHeight = 40.f;
	const float FooterPanelWidth = 248.f;
	const float FooterActionHeight = 46.f;
	const float PartyPanelWidth = 300.f;
	const float RunControlsWidth = 424.f;
	const float PreviewToggleBottomPadding = 14.f;
	const int32 ScreenHeaderFontSize = ShrinkFont(15, 10);
	const int32 BodyToggleFontSize = ShrinkFont(9, 7);
	const int32 PrimaryCtaFontSize = ShrinkFont(11, 7);
	const int32 HeroGridLabelFontSize = ShrinkFont(10, 7);
	const int32 HeroArrowFontSize = ShrinkFont(12, 8);
	const int32 ACBalanceFontSize = ShrinkFont(14, 10);
	const int32 HeroNameFontSize = ShrinkFont(13, 9);
	const int32 SecondaryButtonFontSize = ShrinkFont(9, 7);
	const int32 BodyTextFontSize = ShrinkFont(10, 7);
	const int32 BackToLobbyFontSize = ShrinkFont(12, 8);
	const int32 BackButtonFontSize = ShrinkFont(14, 10);
	const float HeroGridButtonWidth = bDotaTheme ? 110.f : 86.f;
	const FMargin HeroGridButtonPadding = bDotaTheme ? FMargin(7.f, 4.f) : FMargin(6.f, 4.f);
	const float HeroArrowButtonWidth = bDotaTheme ? 30.f : 28.f;
	const float HeroArrowButtonHeight = bDotaTheme ? 26.f : 24.f;
	const TAttribute<FMargin> ScreenSafePadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 0.f, 0.f, 0.f);
	});

	auto MakeFocusMaskFill = [SelectionShellFill]() -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(SelectionShellFill);
	};

	auto MakePreviewFocusMask = [bDotaTheme, &MakeFocusMaskFill, SidePanelMinWidth, TopBarBottomGap, PanelGap]() -> TSharedRef<SWidget>
	{
		if (!bDotaTheme)
		{
			return SNew(SBox).Visibility(EVisibility::Collapsed);
		}

		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, TopBarBottomGap)
			[
				SNew(SBox)
				.HeightOverride(40.f)
				[
					MakeFocusMaskFill()
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, PanelGap, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(SidePanelMinWidth)
					[
						MakeFocusMaskFill()
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(PanelGap, 0.0f)
				[
					SNew(SBox)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(PanelGap, 0.0f, 0.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(SidePanelMinWidth)
					[
						MakeFocusMaskFill()
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, TopBarBottomGap, 0.0f, 0.0f)
			[
				SNew(SBox)
				.HeightOverride(64.f)
				[
					MakeFocusMaskFill()
				]
			];
	};

	auto MakeBodyTogglePanel = [this,
		bDotaTheme,
		FooterToggleWidth,
		FooterToggleHeight,
		BodyToggleFontSize,
		SelectionToggleFill,
		SelectionPanelFill]() -> TSharedRef<SWidget>
	{
		const FSlateColor TogglePanelColor = bDotaTheme ? SelectionPanelFill : FT66Style::Tokens::Panel2;
		const FSlateColor ToggleActiveColor = bDotaTheme ? SelectionToggleFill : FT66Style::Tokens::Accent2;

		auto MakeBodyToggleButton = [this, FooterToggleWidth, FooterToggleHeight, BodyToggleFontSize, ToggleActiveColor, TogglePanelColor](const TCHAR* Label, ET66BodyType BodyType) -> TSharedRef<SWidget>
		{
			const FOnClicked OnClicked = (BodyType == ET66BodyType::TypeA)
				? FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBodyTypeAClicked)
				: FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBodyTypeBClicked);

			return FT66Style::MakeButton(FT66ButtonParams(
				FText::AsCultureInvariant(Label),
				OnClicked,
				ET66ButtonType::Neutral)
				.SetMinWidth(FooterToggleWidth)
				.SetHeight(FooterToggleHeight)
				.SetFontSize(BodyToggleFontSize)
				.SetPadding(FMargin(10.f, 7.f, 10.f, 6.f))
				.SetColor(TAttribute<FSlateColor>::CreateLambda([this, BodyType, ToggleActiveColor, TogglePanelColor]() -> FSlateColor
				{
					return SelectedBodyType == BodyType ? ToggleActiveColor : TogglePanelColor;
				}))
				.SetContent(
					SNew(STextBlock)
					.Text(FText::AsCultureInvariant(Label))
					.Font(FT66Style::Tokens::FontBold(BodyToggleFontSize))
					.ColorAndOpacity(FT66Style::Tokens::Text)));
		};

		return FT66Style::MakePanel(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f)
			[
				MakeBodyToggleButton(TEXT("CHAD"), ET66BodyType::TypeA)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f)
			[
				MakeBodyToggleButton(TEXT("STACY"), ET66BodyType::TypeB)
			],
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(TogglePanelColor)
				.SetPadding(FMargin(4.f, 4.f, 4.f, 3.f)));
	};

	auto MakeChooseCompanionPanel = [this,
		bDotaTheme,
		ChooseCompanionText,
		FooterPanelWidth,
		FooterActionHeight,
		PrimaryCtaFontSize,
		SelectionPanelFill]() -> TSharedRef<SWidget>
	{
		const FSlateColor CompanionPanelColor = bDotaTheme ? SelectionPanelFill : FT66Style::Tokens::Panel;
		const FSlateColor CompanionButtonColor = bDotaTheme ? SelectionPanelFill : FT66Style::Tokens::Panel2;

		return FT66Style::MakePanel(
			SNew(SBox)
			.WidthOverride(FooterPanelWidth)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					ChooseCompanionText,
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleCompanionClicked),
					ET66ButtonType::Neutral)
					.SetMinWidth(0.f)
					.SetHeight(FooterActionHeight)
					.SetFontSize(PrimaryCtaFontSize)
					.SetPadding(FMargin(12.f, 8.f))
					.SetColor(CompanionButtonColor))
			],
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(CompanionPanelColor)
				.SetPadding(FMargin(6.f, 6.f)));
	};

	auto MakeWorldScrim = []() -> TSharedRef<SWidget>
	{
		return SNew(SBox).Visibility(EVisibility::Collapsed);
	};

	auto MakeSelectionBar = [bDotaTheme, SelectionPanelFill](TSharedRef<SWidget> Content) -> TSharedRef<SWidget>
	{
		if (bDotaTheme)
		{
			return FT66Style::MakeViewportFrame(Content, FMargin(6.f, 6.f));
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(SelectionPanelFill)
				.SetPadding(FMargin(6.f, 6.f)));
	};

	auto MakeHeroStripControls = [this,
		HeroGridText,
		HeroGridButtonWidth,
		HeroGridButtonPadding,
		HeroGridLabelFontSize,
		HeroArrowButtonWidth,
		HeroArrowButtonHeight,
		HeroArrowFontSize,
		HeroCarousel]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(6.0f, 0.0f, 6.0f, 0.0f)
			[
				FT66Style::MakeButton(FT66ButtonParams(HeroGridText,
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleHeroGridClicked),
					ET66ButtonType::Neutral)
					.SetMinWidth(HeroGridButtonWidth)
					.SetPadding(HeroGridButtonPadding)
					.SetContent(SNew(STextBlock)
						.Text(HeroGridText)
						.Font(FT66Style::Tokens::FontBold(HeroGridLabelFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					NSLOCTEXT("T66.Common", "Prev", "<"),
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandlePrevClicked),
					ET66ButtonType::Neutral)
					.SetMinWidth(HeroArrowButtonWidth)
					.SetHeight(HeroArrowButtonHeight)
					.SetFontSize(HeroArrowFontSize))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				HeroCarousel
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					NSLOCTEXT("T66.Common", "Next", ">"),
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleNextClicked),
					ET66ButtonType::Neutral)
					.SetMinWidth(HeroArrowButtonWidth)
					.SetHeight(HeroArrowButtonHeight)
					.SetFontSize(HeroArrowFontSize))
			];
	};

	auto MakePartyBox = [this,
		PartyPanelWidth,
		ActivePartySlots]() -> TSharedRef<SWidget>
	{
		const FLinearColor ShellFill(0.004f, 0.005f, 0.010f, 0.985f);
		const FLinearColor LeaderSlotAccent(0.29f, 0.24f, 0.13f, 1.0f);
		const FLinearColor PartySlotAccent(0.15f, 0.17f, 0.19f, 1.0f);
		const FLinearColor PartySlotAccentInactive(0.08f, 0.09f, 0.10f, 1.0f);
		const FLinearColor PlaceholderTint(0.20f, 0.22f, 0.24f, 0.55f);

		TSharedRef<SHorizontalBox> PartySlots = SNew(SHorizontalBox);
		for (int32 SlotIndex = 0; SlotIndex < 4; ++SlotIndex)
		{
			const bool bPartyEnabledSlot = SlotIndex < ActivePartySlots;
			const bool bLeaderSlot = SlotIndex == 0;
			const FLinearColor SlotAccent = bLeaderSlot
				? LeaderSlotAccent
				: (bPartyEnabledSlot ? PartySlotAccent : PartySlotAccentInactive);
			const float PlaceholderOpacity = bPartyEnabledSlot ? 0.55f : 0.28f;

			const TSharedRef<SWidget> SlotContent =
				bLeaderSlot
				? StaticCastSharedRef<SWidget>(
					SNew(SImage)
					.Image(PartyHeroPortraitBrush.Get()))
				: StaticCastSharedRef<SWidget>(
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Top)
					.Padding(0.f, 10.f, 0.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(12.f)
						.HeightOverride(12.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(PlaceholderTint.R, PlaceholderTint.G, PlaceholderTint.B, PlaceholderOpacity))
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Bottom)
					.Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(SBox)
						.WidthOverride(20.f)
						.HeightOverride(14.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(PlaceholderTint.R, PlaceholderTint.G, PlaceholderTint.B, PlaceholderOpacity))
						]
					]);

			PartySlots->AddSlot()
				.AutoWidth()
				.Padding(SlotIndex > 0 ? FMargin(5.f, 0.f, 0.f, 0.f) : FMargin(0.f))
				[
					SNew(SBox)
					.WidthOverride(52.f)
					.HeightOverride(52.f)
					[
						FT66Style::MakeSlotFrame(SlotContent, SlotAccent, bLeaderSlot ? FMargin(1.f) : FMargin(6.f))
					]
				];
		}

		return SNew(SBox)
			.WidthOverride(PartyPanelWidth)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(ShellFill)
				.Padding(FMargin(12.f, 10.f, 12.f, 8.f))
				[
					PartySlots
				]
			];
	};

	auto MakeRunControls = [this,
		bHideEnterFromLobby,
		BackToLobbyText,
		BackToLobbyFontSize,
		EnterText,
		PrimaryCtaFontSize,
		FooterActionHeight,
		Loc,
		RunControlsWidth]() -> TSharedRef<SWidget>
	{
		if (bHideEnterFromLobby)
		{
			return SNew(SBox)
				.WidthOverride(RunControlsWidth)
				[
					FT66Style::MakeButton(FT66ButtonParams(
						BackToLobbyText,
						FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBackToLobbyClicked),
						ET66ButtonType::Danger)
						.SetMinWidth(0.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(12.f, 8.f))
						.SetFontSize(BackToLobbyFontSize))
				];
		}

		return SNew(SBox)
			.WidthOverride(RunControlsWidth)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.34f)
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					FT66Style::MakeDropdown(FT66DropdownParams(
						SNew(STextBlock)
						.Text_Lambda([this, Loc]() -> FText
						{
							return CurrentDifficultyOption.IsValid()
								? FText::FromString(*CurrentDifficultyOption)
								: (Loc ? Loc->GetText_Easy() : NSLOCTEXT("T66.Difficulty", "Easy", "Easy"));
						})
						.Font(FT66Style::Tokens::FontBold(PrimaryCtaFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text),
						[this]()
						{
							TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
							for (const TSharedPtr<FString>& Opt : DifficultyOptions)
							{
								if (!Opt.IsValid())
								{
									continue;
								}

								TSharedPtr<FString> Captured = Opt;
								Box->AddSlot()
									.AutoHeight()
									[
										FT66Style::MakeButton(FT66ButtonParams(
											FText::FromString(*Opt),
											FOnClicked::CreateLambda([this, Captured]()
											{
												OnDifficultyChanged(Captured, ESelectInfo::Direct);
												FSlateApplication::Get().DismissAllMenus();
												return FReply::Handled();
											}),
											ET66ButtonType::Neutral)
											.SetMinWidth(0.f))
									];
							}
							return Box;
						})
						.SetMinWidth(0.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(10.f, 8.f)))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.66f)
				[
					FT66Style::MakeButton(FT66ButtonParams(
						EnterText,
						FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleEnterClicked),
						ET66ButtonType::Primary)
						.SetMinWidth(0.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(12.f, 8.f))
						.SetFontSize(PrimaryCtaFontSize)
						.SetContent(SNew(STextBlock)
							.Text(EnterText)
							.Font(FT66Style::Tokens::FontBold(PrimaryCtaFontSize))
							.ColorAndOpacity(FT66Style::Tokens::Text)))
				]
			];
	};

	TSharedRef<SWidget> Root = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
		.BorderBackgroundColor(FLinearColor::Transparent)
		.Padding(0)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				MakePreviewFocusMask()
			]
			+ SOverlay::Slot()
			[
				MakeWorldScrim()
			]
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(ScreenSafePadding)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, TopBarBottomGap)
					[
						MakeSelectionBar(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(TopBarSideWidth)
								[
									FT66Style::MakeButton(FT66ButtonParams(
										BackText,
										FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBackClicked),
										ET66ButtonType::Neutral)
										.SetMinWidth(0.f)
										.SetFontSize(BackButtonFontSize))
								]
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								MakeHeroStripControls()
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(TopBarSideWidth)
							]
						)
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(0.0f, ContentTopGap, 0.0f, 0.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Fill)
						[
							SNew(SBox)
							.WidthOverride(SidePanelMinWidth)
							[
								FT66Style::MakePanel(
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.0f, 0.0f, 0.0f, 8.0f)
									[
										SNew(SOverlay)
										+ SOverlay::Slot()
										.HAlign(HAlign_Center)
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
											.Text(SkinsText)
											.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize))
											.ColorAndOpacity(FT66Style::Tokens::Text)
										]
										+ SOverlay::Slot()
										.HAlign(HAlign_Right)
										.VAlign(VAlign_Center)
										[
											FT66Style::MakePanel(
												SAssignNew(ACBalanceTextBlock, STextBlock)
												.Text(ACBalanceText)
												.Font(FT66Style::Tokens::FontBold(ACBalanceFontSize))
												.ColorAndOpacity(FT66Style::Tokens::Text),
												FT66PanelParams(ET66PanelType::Panel)
													.SetColor(bDotaTheme ? SelectionInsetFill : FT66Style::Tokens::Panel)
													.SetPadding(FMargin(10.0f, 5.0f)))
										]
									]
									+ SVerticalBox::Slot()
									.FillHeight(1.0f)
									[
										SNew(SScrollBox)
										+ SScrollBox::Slot()
										[
											SkinsListBoxWidget.ToSharedRef()
										]
									],
									FT66PanelParams(ET66PanelType::Panel)
										.SetColor(bDotaTheme ? SelectionPanelFill : FT66Style::Tokens::Panel)
										.SetPadding(FMargin(FT66Style::Tokens::Space3)))
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(CenterPanelFill)
						.Padding(PanelGap, 0.0f, PanelGap, PanelBottomInset)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SBox)
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									CreateHeroPreviewWidget(HeroPreviewColor)
								]
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Bottom)
							.Padding(0.0f, 0.0f, 0.0f, PreviewToggleBottomPadding)
							[
								MakeBodyTogglePanel()
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Fill)
						.Padding(PanelGap, 0.0f, 0.0f, PanelBottomInset)
						[
							SNew(SBox)
							.WidthOverride(SidePanelMinWidth)
							[
								FT66Style::MakePanel(
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.FillHeight(1.0f)
									[
										SNew(SWidgetSwitcher)
										.WidgetIndex_Lambda([this]() -> int32 { return bShowingLore ? 1 : 0; })
										+ SWidgetSwitcher::Slot()
										[
											SNew(SVerticalBox)
											+ SVerticalBox::Slot()
											.AutoHeight()
											.HAlign(HAlign_Center)
											.Padding(0.0f, 0.0f, 0.0f, 6.0f)
											[
												SNew(STextBlock)
												.Text(HeroInfoText)
												.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize))
												.ColorAndOpacity(FT66Style::Tokens::Text)
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(0.0f, 0.0f, 0.0f, 10.0f)
											[
												SNew(SHorizontalBox)
												+ SHorizontalBox::Slot()
												.FillWidth(1.0f)
												.VAlign(VAlign_Center)
												[
													FT66Style::MakeButton(FT66ButtonParams(
														Loc ? Loc->GetText_SelectYourHero() : NSLOCTEXT("T66.HeroSelection", "SelectHero", "Select a Hero"),
														FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleLoreClicked),
														ET66ButtonType::Neutral)
														.SetContent(SAssignNew(HeroNameWidget, STextBlock)
															.Text(Loc ? Loc->GetText_SelectYourHero() : NSLOCTEXT("T66.HeroSelection", "SelectHero", "Select a Hero"))
															.Font(FT66Style::Tokens::FontBold(HeroNameFontSize))
															.ColorAndOpacity(FT66Style::Tokens::Text)
															.Justification(ETextJustify::Center)))
												]
												+ SHorizontalBox::Slot()
												.AutoWidth()
												.VAlign(VAlign_Center)
												.Padding(8.0f, 0.0f, 0.0f, 0.0f)
												[
													FT66Style::MakeButton(FT66ButtonParams(
														DemoText,
														FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTheLabClicked),
														ET66ButtonType::Neutral)
														.SetMinWidth(64.f)
														.SetContent(SNew(STextBlock)
															.Text(DemoText)
															.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize))
															.ColorAndOpacity(FT66Style::Tokens::Text)))
												]
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(0.0f, 0.0f, 0.0f, 10.0f)
											[
												FT66Style::MakePanel(
													SNew(SBox)
													.HeightOverride(204.0f)
													[
														SNew(SOverlay)
														+ SOverlay::Slot()
														[
															SAssignNew(HeroPreviewVideoImage, SImage)
															.Image(HeroPreviewVideoBrush.IsValid() ? HeroPreviewVideoBrush.Get() : nullptr)
														]
														+ SOverlay::Slot()
														.HAlign(HAlign_Center)
														.VAlign(VAlign_Center)
														[
															SAssignNew(HeroPreviewPlaceholderText, STextBlock)
															.Text(NSLOCTEXT("T66.HeroSelection", "VideoPreview", "[VIDEO PREVIEW]"))
															.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
															.ColorAndOpacity(FT66Style::Tokens::TextMuted)
															.Justification(ETextJustify::Center)
														]
													],
													FT66PanelParams(ET66PanelType::Panel)
														.SetColor(bDotaTheme ? SelectionInsetFill : FT66Style::Tokens::Panel)
														.SetPadding(FMargin(5.0f)))
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(0.0f, 0.0f, 0.0f, 10.0f)
											[
												FT66Style::MakePanel(
													SNew(SVerticalBox)
													+ SVerticalBox::Slot()
													.AutoHeight()
													.Padding(0.0f, 0.0f, 0.0f, 6.0f)
													[
														SNew(STextBlock)
														.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordTitle", "HERO RECORD"))
														.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize))
														.ColorAndOpacity(FT66Style::Tokens::TextMuted)
													]
													+ SVerticalBox::Slot()
													.AutoHeight()
													[
														SNew(SHorizontalBox)
														+ SHorizontalBox::Slot()
														.FillWidth(1.15f)
														[
															SNew(SVerticalBox)
															+ SVerticalBox::Slot()
															.AutoHeight()
															[
																SNew(STextBlock)
																.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordMedalLabel", "Highest Medal"))
																.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
																.ColorAndOpacity(FT66Style::Tokens::TextMuted)
															]
															+ SVerticalBox::Slot()
															.AutoHeight()
															.Padding(0.0f, 2.0f, 0.0f, 0.0f)
															[
																SAssignNew(HeroRecordMedalWidget, STextBlock)
																.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordMedalDefault", "Unproven"))
																.Font(FT66Style::Tokens::FontBold(BodyTextFontSize + 2))
																.ColorAndOpacity(FLinearColor(0.89f, 0.74f, 0.27f, 1.0f))
															]
														]
														+ SHorizontalBox::Slot()
														.FillWidth(0.85f)
														[
															SNew(SVerticalBox)
															+ SVerticalBox::Slot()
															.AutoHeight()
															[
																SNew(STextBlock)
																.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordUnityLabel", "Unity"))
																.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
																.ColorAndOpacity(FT66Style::Tokens::TextMuted)
															]
															+ SVerticalBox::Slot()
															.AutoHeight()
															.Padding(0.0f, 2.0f, 0.0f, 0.0f)
															[
																SAssignNew(HeroRecordUnityWidget, STextBlock)
																.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordUnityDefault", "0"))
																.Font(FT66Style::Tokens::FontBold(BodyTextFontSize + 2))
																.ColorAndOpacity(FT66Style::Tokens::Text)
															]
														]
														+ SHorizontalBox::Slot()
														.FillWidth(0.95f)
														[
															SNew(SVerticalBox)
															+ SVerticalBox::Slot()
															.AutoHeight()
															[
																SNew(STextBlock)
																.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordGamesLabel", "Games Played"))
																.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
																.ColorAndOpacity(FT66Style::Tokens::TextMuted)
															]
															+ SVerticalBox::Slot()
															.AutoHeight()
															.Padding(0.0f, 2.0f, 0.0f, 0.0f)
															[
																SAssignNew(HeroRecordGamesWidget, STextBlock)
																.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordGamesDefault", "0"))
																.Font(FT66Style::Tokens::FontBold(BodyTextFontSize + 2))
																.ColorAndOpacity(FT66Style::Tokens::Text)
															]
														]
													],
													FT66PanelParams(ET66PanelType::Panel)
														.SetColor(bDotaTheme ? SelectionInsetFill : FT66Style::Tokens::Panel)
														.SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2)))
											]
											+ SVerticalBox::Slot()
											.FillHeight(1.0f)
											[
												SNew(SScrollBox)
												+ SScrollBox::Slot()
												[
													SAssignNew(HeroDescWidget, STextBlock)
													.Text(NSLOCTEXT("T66.HeroSelection", "SelectHeroDescriptionHint", "Select a hero to view their description."))
													.Font(FT66Style::Tokens::FontRegular(BodyTextFontSize))
													.ColorAndOpacity(FT66Style::Tokens::TextMuted)
													.AutoWrapText(true)
												]
											]
										]
										+ SWidgetSwitcher::Slot()
										[
											SNew(SVerticalBox)
											+ SVerticalBox::Slot()
											.AutoHeight()
											.HAlign(HAlign_Center)
											.Padding(0.0f, 0.0f, 0.0f, 6.0f)
											[
												SNew(STextBlock)
												.Text(LoreText)
												.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize))
												.ColorAndOpacity(FT66Style::Tokens::Text)
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(0.0f, 0.0f, 0.0f, 12.0f)
											[
												SNew(SHorizontalBox)
												+ SHorizontalBox::Slot()
												.FillWidth(1.0f)
												.VAlign(VAlign_Center)
												[
													FT66Style::MakePanel(
														SNew(STextBlock)
														.Text_Lambda([this, Loc]() -> FText
														{
															FHeroData HeroData;
															if (GetPreviewedHeroData(HeroData))
															{
																return Loc ? Loc->GetHeroDisplayName(HeroData) : HeroData.DisplayName;
															}
															return Loc ? Loc->GetText_SelectYourHero() : NSLOCTEXT("T66.HeroSelection", "SelectHero", "Select a Hero");
														})
														.Font(FT66Style::Tokens::FontBold(HeroNameFontSize))
														.ColorAndOpacity(FT66Style::Tokens::Text)
														.Justification(ETextJustify::Center),
														FT66PanelParams(ET66PanelType::Panel)
															.SetColor(bDotaTheme ? SelectionInsetFill : FT66Style::Tokens::Panel)
															.SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2)))
												]
												+ SHorizontalBox::Slot()
												.AutoWidth()
												.VAlign(VAlign_Center)
												.Padding(8.0f, 0.0f, 0.0f, 0.0f)
												[
													FT66Style::MakeButton(FT66ButtonParams(
														BackText,
														FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleLoreClicked),
														ET66ButtonType::Neutral)
														.SetMinWidth(64.f)
														.SetContent(SNew(STextBlock)
															.Text(BackText)
															.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize))
															.ColorAndOpacity(FT66Style::Tokens::Text)))
												]
											]
											+ SVerticalBox::Slot()
											.FillHeight(1.0f)
											[
												SNew(SScrollBox)
												+ SScrollBox::Slot()
												[
													SAssignNew(HeroLoreWidget, STextBlock)
													.Text(FText::GetEmpty())
													.Font(FT66Style::Tokens::FontRegular(BodyTextFontSize))
													.ColorAndOpacity(FT66Style::Tokens::TextMuted)
													.AutoWrapText(true)
												]
											]
										]
									],
									FT66PanelParams(ET66PanelType::Panel)
										.SetColor(bDotaTheme ? SelectionPanelFill : FT66Style::Tokens::Panel)
										.SetPadding(FMargin(FT66Style::Tokens::Space4)))
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, TopBarBottomGap, 0.0f, 0.0f)
					[
						MakeSelectionBar(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(0.34f)
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Bottom)
							.Padding(0.0f, 0.0f, 8.0f, 0.0f)
							[
								MakePartyBox()
							]
							+ SHorizontalBox::Slot()
							.FillWidth(0.32f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Bottom)
							[
								MakeChooseCompanionPanel()
							]
							+ SHorizontalBox::Slot()
							.FillWidth(0.34f)
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Bottom)
							.Padding(8.0f, 0.0f, 0.0f, 0.0f)
							[
								MakeRunControls()
							]
						)
					]
				]
			]
		];
	UpdateHeroDisplay();
	UpdateHeroPreviewVideo();
	return Root;
}

FReply UT66HeroSelectionScreen::HandlePrevClicked()
{
	PreviewPreviousHero();
	FT66Style::DeferRebuild(this);
	return FReply::Handled();
}
FReply UT66HeroSelectionScreen::HandleNextClicked()
{
	PreviewNextHero();
	FT66Style::DeferRebuild(this);
	return FReply::Handled();
}
FReply UT66HeroSelectionScreen::HandleHeroGridClicked() { OnHeroGridClicked(); return FReply::Handled(); }
FReply UT66HeroSelectionScreen::HandleCompanionClicked() { OnChooseCompanionClicked(); return FReply::Handled(); }
FReply UT66HeroSelectionScreen::HandleLoreClicked()
{
	bShowingLore = !bShowingLore;
	UpdateHeroDisplay();
	return FReply::Handled();
}
FReply UT66HeroSelectionScreen::HandleTheLabClicked() { OnTheLabClicked(); return FReply::Handled(); }
FReply UT66HeroSelectionScreen::HandleEnterClicked() { OnEnterTribulationClicked(); return FReply::Handled(); }
FReply UT66HeroSelectionScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }
FReply UT66HeroSelectionScreen::HandleBackToLobbyClicked()
{
	NavigateTo(ET66ScreenType::Lobby);
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleBodyTypeAClicked()
{
	SelectedBodyType = ET66BodyType::TypeA;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedHeroBodyType = SelectedBodyType;
	}
	UpdateHeroDisplay(); // Update 3D preview immediately for this hero
	FT66Style::DeferRebuild(this);
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleBodyTypeBClicked()
{
	SelectedBodyType = ET66BodyType::TypeB;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedHeroBodyType = SelectedBodyType;
	}
	UpdateHeroDisplay(); // Update 3D preview immediately for this hero
	FT66Style::DeferRebuild(this);
	return FReply::Handled();
}

void UT66HeroSelectionScreen::UpdateHeroDisplay()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	FHeroData HeroData;
	if (GetPreviewedHeroData(HeroData))
	{
		if (HeroNameWidget.IsValid())
		{
			HeroNameWidget->SetText(Loc ? Loc->GetHeroDisplayName(HeroData) : HeroData.DisplayName);
		}
		if (HeroLoreWidget.IsValid())
		{
			const FText Desc = Loc ? Loc->GetText_HeroDescription(PreviewedHeroID) : HeroData.Description;
			HeroLoreWidget->SetText(Desc);
		}
		if (HeroDescWidget.IsValid())
		{
			// Main panel: quote + base stats (Speed excluded). Lore panel shows the full description.
			const FText Quote = Loc ? Loc->GetText_HeroQuote(PreviewedHeroID) : FText::GetEmpty();

			FT66HeroStatBlock BaseStats;
			FT66HeroPerLevelStatGains PerLevelGains;
			UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
			{
				T66GI->GetHeroStatTuning(PreviewedHeroID, BaseStats, PerLevelGains);
			}
			(void)PerLevelGains;

			FT66HeroStatBonuses PowerUpBonuses;
			if (UT66PowerUpSubsystem* PowerUp = GI ? GI->GetSubsystem<UT66PowerUpSubsystem>() : nullptr)
			{
				PowerUpBonuses = PowerUp->GetPowerupStatBonuses();
			}

			const FText NewLine = NSLOCTEXT("T66.Common", "NewLine", "\n");
			const FText StatLineFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");
			const FText StatLineWithBonusFmt = NSLOCTEXT("T66.Stats", "StatLineWithBonus", "{0}: {1} + {2}");
			TArray<FText> Lines;
			Lines.Reserve(12);
			if (!Quote.IsEmpty())
			{
				Lines.Add(Quote);
				Lines.Add(FText::GetEmpty());
			}
			Lines.Add(Loc ? Loc->GetText_BaseStatsHeader() : NSLOCTEXT("T66.HeroSelection", "BaseStatsHeader", "Base Stats"));

			auto AddStatLine = [&](const FText& Label, int32 BaseValue, int32 BonusValue)
			{
				FText ValueText = (BonusValue > 0)
					? FText::Format(StatLineWithBonusFmt, Label, FText::AsNumber(BaseValue), FText::AsNumber(BonusValue))
					: FText::Format(StatLineFmt, Label, FText::AsNumber(BaseValue));
				Lines.Add(ValueText);
			};

			AddStatLine(Loc ? Loc->GetText_Stat_Damage() : NSLOCTEXT("T66.Stats", "Damage", "Damage"), BaseStats.Damage, PowerUpBonuses.Damage);
			AddStatLine(Loc ? Loc->GetText_Stat_AttackSpeed() : NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed"), BaseStats.AttackSpeed, PowerUpBonuses.AttackSpeed);
			AddStatLine(Loc ? Loc->GetText_Stat_AttackScale() : NSLOCTEXT("T66.Stats", "AttackScale", "Attack Scale"), BaseStats.AttackScale, PowerUpBonuses.AttackScale);
			AddStatLine(Loc ? Loc->GetText_Stat_Armor() : NSLOCTEXT("T66.Stats", "Armor", "Armor"), BaseStats.Armor, PowerUpBonuses.Armor);
			AddStatLine(Loc ? Loc->GetText_Stat_Evasion() : NSLOCTEXT("T66.Stats", "Evasion", "Evasion"), BaseStats.Evasion, PowerUpBonuses.Evasion);
			AddStatLine(Loc ? Loc->GetText_Stat_Luck() : NSLOCTEXT("T66.Stats", "Luck", "Luck"), BaseStats.Luck, PowerUpBonuses.Luck);

			HeroDescWidget->SetText(FText::Join(NewLine, Lines));
		}
		if (HeroRecordMedalWidget.IsValid() || HeroRecordUnityWidget.IsValid() || HeroRecordGamesWidget.IsValid())
		{
			UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
			UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
			const ET66AccountMedalTier MedalTier = Achievements ? Achievements->GetHeroHighestMedal(PreviewedHeroID) : ET66AccountMedalTier::None;
			const int32 UnityStages = Achievements ? Achievements->GetHeroUnityStagesCleared(PreviewedHeroID) : 0;
			const int32 GamesPlayed = Achievements ? Achievements->GetHeroGamesPlayed(PreviewedHeroID) : 0;

			if (HeroRecordMedalWidget.IsValid())
			{
				HeroRecordMedalWidget->SetText(HeroRecordMedalText(MedalTier));
				HeroRecordMedalWidget->SetColorAndOpacity(HeroRecordMedalColor(MedalTier));
			}
			if (HeroRecordUnityWidget.IsValid())
			{
				HeroRecordUnityWidget->SetText(FText::AsNumber(UnityStages));
			}
			if (HeroRecordGamesWidget.IsValid())
			{
				HeroRecordGamesWidget->SetText(FText::AsNumber(GamesPlayed));
			}
		}
		// Update 3D preview stage or fallback colored box (use preview skin override or selected skin)
		if (AT66HeroPreviewStage* PreviewStage = GetHeroPreviewStage())
		{
			UT66GameInstance* GIPreview = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
			FName EffectiveSkinID = PreviewSkinIDOverride.IsNone()
				? (GIPreview && !GIPreview->SelectedHeroSkinID.IsNone() ? GIPreview->SelectedHeroSkinID : FName(TEXT("Default")))
				: PreviewSkinIDOverride;
			if (EffectiveSkinID.IsNone()) EffectiveSkinID = FName(TEXT("Default"));
			PreviewStage->SetPreviewDifficulty(SelectedDifficulty);
			UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] UpdateHeroDisplay: PreviewSkinIDOverride=%s, GI->SelectedHeroSkinID=%s, EffectiveSkinID=%s"),
				*PreviewSkinIDOverride.ToString(), GIPreview ? *GIPreview->SelectedHeroSkinID.ToString() : TEXT("(null GI)"), *EffectiveSkinID.ToString());
			UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] UpdateHeroDisplay: calling SetPreviewHero HeroID=%s BodyType=%d SkinID=%s"),
				*PreviewedHeroID.ToString(), (int32)SelectedBodyType, *EffectiveSkinID.ToString());
			FName CompanionID = GIPreview ? GIPreview->SelectedCompanionID : NAME_None;
			PreviewStage->SetPreviewHero(PreviewedHeroID, SelectedBodyType, EffectiveSkinID, CompanionID);
			if (UWorld* World = GetWorld())
			{
				if (AT66FrontendGameMode* GM = Cast<AT66FrontendGameMode>(World->GetAuthGameMode()))
				{
					GM->PositionCameraForHeroPreview();
				}
			}
		}
		else if (HeroPreviewColorBox.IsValid())
		{
			HeroPreviewColorBox->SetBorderBackgroundColor(FT66Style::IsDotaTheme() ? FLinearColor::Transparent : HeroData.PlaceholderColor);
		}
	}
	else
	{
		// No preview hero (should not happen; there is always a hero in the preview). Leave widgets as-is.
		if (HeroLoreWidget.IsValid())
		{
			HeroLoreWidget->SetText(FText::GetEmpty());
		}
		if (HeroRecordMedalWidget.IsValid())
		{
			HeroRecordMedalWidget->SetText(NSLOCTEXT("T66.HeroSelection", "HeroRecordMedalEmpty", "Unproven"));
			HeroRecordMedalWidget->SetColorAndOpacity(HeroRecordMedalColor(ET66AccountMedalTier::None));
		}
		if (HeroRecordUnityWidget.IsValid())
		{
			HeroRecordUnityWidget->SetText(NSLOCTEXT("T66.HeroSelection", "HeroRecordUnityEmpty", "0"));
		}
		if (HeroRecordGamesWidget.IsValid())
		{
			HeroRecordGamesWidget->SetText(NSLOCTEXT("T66.HeroSelection", "HeroRecordGamesEmpty", "0"));
		}
		if (AT66HeroPreviewStage* PreviewStage = GetHeroPreviewStage())
		{
			UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
			FName EffectiveSkinID = PreviewSkinIDOverride.IsNone()
				? (GI && !GI->SelectedHeroSkinID.IsNone() ? GI->SelectedHeroSkinID : FName(TEXT("Default")))
				: PreviewSkinIDOverride;
			if (EffectiveSkinID.IsNone()) EffectiveSkinID = FName(TEXT("Default"));
			FName CompanionID = GI ? GI->SelectedCompanionID : NAME_None;
			PreviewStage->SetPreviewHero(PreviewedHeroID, SelectedBodyType, EffectiveSkinID, CompanionID);
			if (UWorld* World = GetWorld())
			{
				if (AT66FrontendGameMode* GM = Cast<AT66FrontendGameMode>(World->GetAuthGameMode()))
				{
					GM->PositionCameraForHeroPreview();
				}
			}
		}
		else if (HeroPreviewColorBox.IsValid())
		{
			HeroPreviewColorBox->SetBorderBackgroundColor(FT66Style::IsDotaTheme() ? FLinearColor::Transparent : FLinearColor(0.3f, 0.3f, 0.4f, 1.0f));
		}
	}

	// Keep the top carousel portraits in sync with the preview index.
	RefreshHeroCarouselPortraits();
}

void UT66HeroSelectionScreen::RefreshHeroCarouselPortraits()
{
	if (AllHeroIDs.Num() <= 0)
	{
		HeroCarouselSlotColors.Init(FLinearColor(0.2f, 0.2f, 0.25f, 1.0f), 5);
		HeroCarouselSlotVisibility.Init(EVisibility::Collapsed, 5);
		for (TSharedPtr<FSlateBrush>& B : HeroCarouselPortraitBrushes)
		{
			if (B.IsValid()) B->SetResourceObject(nullptr);
		}
		return;
	}

	// Ensure 5 stable slots.
	HeroCarouselPortraitBrushes.SetNum(5);
	HeroCarouselSlotColors.SetNum(5);
	HeroCarouselSlotVisibility.SetNum(5);
	for (int32 i = 0; i < HeroCarouselPortraitBrushes.Num(); ++i)
	{
		if (!HeroCarouselPortraitBrushes[i].IsValid())
		{
			HeroCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			HeroCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			HeroCarouselPortraitBrushes[i]->ImageSize = FVector2D(60.f, 60.f);
		}
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		UT66UITexturePoolSubsystem* TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
		for (int32 Offset = -2; Offset <= 2; ++Offset)
		{
			const int32 SlotIdx = Offset + 2;
			if (!HeroCarouselPortraitBrushes.IsValidIndex(SlotIdx) || !HeroCarouselPortraitBrushes[SlotIdx].IsValid())
			{
				continue;
			}

			const int32 HeroIdx = (CurrentHeroIndex + Offset + AllHeroIDs.Num()) % AllHeroIDs.Num();
			const FName HeroID = AllHeroIDs.IsValidIndex(HeroIdx) ? AllHeroIDs[HeroIdx] : NAME_None;
			FLinearColor SlotColor(0.2f, 0.2f, 0.25f, 1.0f);

			TSoftObjectPtr<UTexture2D> PortraitSoft;
			if (!HeroID.IsNone())
			{
				FHeroData D;
				if (GI->GetHeroData(HeroID, D))
				{
					SlotColor = D.PlaceholderColor;
					PortraitSoft = GI->ResolveHeroPortrait(D, SelectedBodyType, ET66HeroPortraitVariant::Half);
				}
			}

			const float BoxSize = (Offset == 0) ? 60.f : 45.f;
			HeroCarouselSlotColors[SlotIdx] = SlotColor * ((Offset == 0) ? 1.0f : 0.6f);
			HeroCarouselSlotVisibility[SlotIdx] = PortraitSoft.IsNull() ? EVisibility::Hidden : EVisibility::Visible;
			if (PortraitSoft.IsNull() || !TexPool)
			{
				HeroCarouselPortraitBrushes[SlotIdx]->SetResourceObject(nullptr);
			}
			else
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, HeroCarouselPortraitBrushes[SlotIdx], FName(TEXT("HeroCarousel"), SlotIdx + 1), /*bClearWhileLoading*/ true);
			}
			HeroCarouselPortraitBrushes[SlotIdx]->ImageSize = FVector2D(BoxSize, BoxSize);
		}
	}
}

void UT66HeroSelectionScreen::OnScreenDeactivated_Implementation()
{
	Super::OnScreenDeactivated_Implementation();
	if (HeroPreviewMediaPlayer)
	{
		HeroPreviewMediaPlayer->Close();
	}
	KnightPreviewMediaSourceHandle.Reset();
	// When opened from Lobby, persist current selection so Lobby can show hero portrait (and so returning from Companion Selection still shows co-op layout).
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (GI->bHeroSelectionFromLobby)
		{
			GI->SelectedHeroID = PreviewedHeroID;
			GI->SelectedDifficulty = SelectedDifficulty;
			GI->SelectedHeroBodyType = SelectedBodyType;
			// Do not clear bHeroSelectionFromLobby here: we may be navigating to Companion Selection and will return to this screen still in co-op flow. Cleared when leaving Lobby (Lobby::OnScreenDeactivated).
		}
	}
}

void UT66HeroSelectionScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	// Rebuild Slate so co-op vs solo layout is correct (Lab + Back to Lobby only when from Lobby; difficulty + Enter when solo). Reuse is not enough—cached tree may be from the other flow.
	FT66Style::DeferRebuild(this);
	PreviewSkinIDOverride = NAME_None;
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66HeroSelectionScreen::OnLanguageChanged);
	}
	RefreshHeroList();

	if (UT66GameInstance* GIPreload = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		// Keep the first-open path responsive; the full hero-selection warmup runs asynchronously.
		GIPreload->PrimeHeroSelectionAssetsAsync();
	}
	RequestKnightPreviewMediaSource();

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		SelectedDifficulty = GI->SelectedDifficulty;
		SelectedBodyType = GI->SelectedHeroBodyType;
		// Prefer current preview when valid (e.g. after theme rebuild) so display doesn't jump back to Hero 1.
		if (!PreviewedHeroID.IsNone() && AllHeroIDs.Contains(PreviewedHeroID))
		{
			PreviewHero(PreviewedHeroID);
		}
		else if (!GI->SelectedHeroID.IsNone() && AllHeroIDs.Contains(GI->SelectedHeroID))
		{
			PreviewHero(GI->SelectedHeroID);
		}
		else if (AllHeroIDs.Num() > 0)
		{
			PreviewHero(AllHeroIDs[0]);
		}
		// Sync selected skin to this hero's equipped skin (per-hero).
		if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
		{
			if (!PreviewedHeroID.IsNone())
			{
				GI->SelectedHeroSkinID = SkinSub->GetEquippedHeroSkinID(PreviewedHeroID);
				if (GI->SelectedHeroSkinID.IsNone())
				{
					GI->SelectedHeroSkinID = FName(TEXT("Default"));
				}
			}
		}
	}
	if (UWorld* World = GetWorld())
	{
		if (AT66FrontendGameMode* GM = Cast<AT66FrontendGameMode>(World->GetAuthGameMode()))
			GM->PositionCameraForHeroPreview();
	}
}

void UT66HeroSelectionScreen::RefreshScreen_Implementation()
{
	FHeroData HeroData;
	if (GetPreviewedHeroData(HeroData))
	{
		OnPreviewedHeroChanged(HeroData);
	}
	UpdateHeroDisplay();
}

void UT66HeroSelectionScreen::RefreshHeroList()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		AllHeroIDs = GI->GetAllHeroIDs();
	}
}

TArray<FHeroData> UT66HeroSelectionScreen::GetAllHeroes()
{
	TArray<FHeroData> Heroes;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		for (const FName& HeroID : AllHeroIDs)
		{
			FHeroData HeroData;
			if (GI->GetHeroData(HeroID, HeroData)) Heroes.Add(HeroData);
		}
	}
	return Heroes;
}

bool UT66HeroSelectionScreen::GetPreviewedHeroData(FHeroData& OutHeroData)
{
	if (PreviewedHeroID.IsNone()) return false;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->GetHeroData(PreviewedHeroID, OutHeroData);
	}
	return false;
}

bool UT66HeroSelectionScreen::GetSelectedCompanionData(FCompanionData& OutCompanionData)
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->GetSelectedCompanionData(OutCompanionData);
	}
	return false;
}

void UT66HeroSelectionScreen::PreviewHero(FName HeroID)
{
	UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero START: switching to HeroID=%s"), *HeroID.ToString());
	
	PreviewedHeroID = HeroID;
	CurrentHeroIndex = AllHeroIDs.IndexOfByKey(HeroID);
	if (CurrentHeroIndex == INDEX_NONE) CurrentHeroIndex = 0;
	
	// Clear any preview override when switching heroes (preview is hero-specific).
	PreviewSkinIDOverride = NAME_None;
	UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero: cleared PreviewSkinIDOverride"));
	
	// Sync selected skin to this hero's equipped skin so 3D preview and skin list match.
	// If the previously selected skin is not owned by this hero, fall back to Default.
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
		{
			// First check if the current SelectedHeroSkinID is owned by this hero.
			// If not, reset to this hero's equipped skin (or Default).
			const FName CurrentSkin = GI->SelectedHeroSkinID;
			UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero: CurrentSkin (GI->SelectedHeroSkinID) = %s"), *CurrentSkin.ToString());
			
			const bool bIsDefault = CurrentSkin == FName(TEXT("Default"));
			const bool bIsOwned = SkinSub->IsHeroSkinOwned(HeroID, CurrentSkin);
			UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero: bIsDefault=%d, bIsOwned=%d"), bIsDefault ? 1 : 0, bIsOwned ? 1 : 0);
			
			const bool bCurrentSkinOwned = bIsDefault || bIsOwned;
			if (!bCurrentSkinOwned)
			{
				// Current skin not owned by this hero; switch to this hero's equipped skin.
				const FName NewSkin = SkinSub->GetEquippedHeroSkinID(HeroID);
				UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero: %s does NOT own %s, switching to equipped: %s"),
					*HeroID.ToString(), *CurrentSkin.ToString(), *NewSkin.ToString());
				GI->SelectedHeroSkinID = NewSkin;
			}
			else
			{
				UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero: %s OWNS %s, keeping it"),
					*HeroID.ToString(), *CurrentSkin.ToString());
			}
			if (GI->SelectedHeroSkinID.IsNone())
			{
				GI->SelectedHeroSkinID = FName(TEXT("Default"));
			}
			UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero: final GI->SelectedHeroSkinID = %s"), *GI->SelectedHeroSkinID.ToString());
		}
		else
		{
			UE_LOG(LogT66HeroSelection, Warning, TEXT("[BEACH] PreviewHero: SkinSubsystem is NULL!"));
		}
	}
	else
	{
		UE_LOG(LogT66HeroSelection, Warning, TEXT("[BEACH] PreviewHero: GI is NULL!"));
	}
	
	FHeroData HeroData;
	if (GetPreviewedHeroData(HeroData)) OnPreviewedHeroChanged(HeroData);
	UpdateHeroDisplay();
	UpdateHeroPreviewVideo();
	UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero END"));
}

void UT66HeroSelectionScreen::PreviewNextHero()
{
	if (AllHeroIDs.Num() == 0) return;
	CurrentHeroIndex = (CurrentHeroIndex + 1) % AllHeroIDs.Num();
	PreviewHero(AllHeroIDs[CurrentHeroIndex]);
}

void UT66HeroSelectionScreen::PreviewPreviousHero()
{
	if (AllHeroIDs.Num() == 0) return;
	CurrentHeroIndex = (CurrentHeroIndex - 1 + AllHeroIDs.Num()) % AllHeroIDs.Num();
	PreviewHero(AllHeroIDs[CurrentHeroIndex]);
}

void UT66HeroSelectionScreen::SelectDifficulty(ET66Difficulty Difficulty) { SelectedDifficulty = Difficulty; }
void UT66HeroSelectionScreen::ToggleBodyType() { SelectedBodyType = (SelectedBodyType == ET66BodyType::TypeA) ? ET66BodyType::TypeB : ET66BodyType::TypeA; }
void UT66HeroSelectionScreen::OnHeroGridClicked() { ShowModal(ET66ScreenType::HeroGrid); }
void UT66HeroSelectionScreen::OnChooseCompanionClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedHeroID = PreviewedHeroID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->SelectedHeroBodyType = SelectedBodyType;
		GI->bStageCatchUpPending = false;
	}
	NavigateTo(ET66ScreenType::CompanionSelection);
}
void UT66HeroSelectionScreen::OnHeroLoreClicked() { ShowModal(ET66ScreenType::HeroLore); }
void UT66HeroSelectionScreen::OnTheLabClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		GI->SelectedHeroID = PreviewedHeroID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->SelectedHeroBodyType = SelectedBodyType;
		GI->bStageCatchUpPending = false;
		GI->bIsLabLevel = true;
	}
	if (UIManager) UIManager->HideAllUI();
	UGameplayStatics::OpenLevel(this, FName(TEXT("LabLevel")));
}
void UT66HeroSelectionScreen::OnEnterTribulationClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		GI->SelectedHeroID = PreviewedHeroID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->SelectedHeroBodyType = SelectedBodyType;
		GI->bStageCatchUpPending = false;
		GI->PendingLoadedTransform = FTransform();
		GI->bApplyLoadedTransform = false;
		GI->bLoadAsPreview = false;
		// New seed each time so procedural terrain layout differs per run
		GI->RunSeed = FMath::Rand();
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->ApplyCurrentPartyToGameInstanceRunContext();
		}
	}
	if (UIManager) UIManager->HideAllUI();
	if (GI)
	{
		GI->TransitionToGameplayLevel();
	}
	else
	{
		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetTribulationEntryLevelName());
	}
}
void UT66HeroSelectionScreen::OnBackClicked() { NavigateBack(); }

AT66HeroPreviewStage* UT66HeroSelectionScreen::GetHeroPreviewStage() const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

TSharedRef<SWidget> UT66HeroSelectionScreen::CreateHeroPreviewWidget(const FLinearColor& FallbackColor)
{
	AT66HeroPreviewStage* PreviewStage = GetHeroPreviewStage();

	if (PreviewStage)
	{
		// In-world preview: transparent overlay for drag-rotate/zoom.
		// The main viewport renders the character with full Lumen GI behind the UI.
		return SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(ST66DragRotatePreview)
				.Stage(PreviewStage)
				.DegreesPerPixel(0.28f)
			];
	}

	// Fallback: colored box when no preview stage in level
	return SAssignNew(HeroPreviewColorBox, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::IsDotaTheme() ? FLinearColor::Transparent : FallbackColor)
		[
			SNew(SBox)
		];
}

void UT66HeroSelectionScreen::EnsureHeroPreviewVideoResources()
{
	if (HeroPreviewMediaPlayer && HeroPreviewMediaTexture && HeroPreviewVideoBrush.IsValid())
	{
		return;
	}
	HeroPreviewMediaPlayer = NewObject<UMediaPlayer>(this, UMediaPlayer::StaticClass(), NAME_None, RF_Transient);
	if (HeroPreviewMediaPlayer)
	{
		HeroPreviewMediaPlayer->SetLooping(true);
		HeroPreviewMediaPlayer->PlayOnOpen = true;
	}
	HeroPreviewMediaTexture = NewObject<UMediaTexture>(this, UMediaTexture::StaticClass(), NAME_None, RF_Transient);
	if (HeroPreviewMediaTexture && HeroPreviewMediaPlayer)
	{
		HeroPreviewMediaTexture->SetMediaPlayer(HeroPreviewMediaPlayer);
		HeroPreviewMediaTexture->UpdateResource();
	}
	HeroPreviewVideoBrush = MakeShared<FSlateBrush>();
	HeroPreviewVideoBrush->DrawAs = ESlateBrushDrawType::Image;
	HeroPreviewVideoBrush->Tiling = ESlateBrushTileType::NoTile;
	HeroPreviewVideoBrush->ImageSize = FVector2D(640.0f, 360.0f);
	if (HeroPreviewMediaTexture)
	{
		HeroPreviewVideoBrush->SetResourceObject(HeroPreviewMediaTexture);
	}
}

void UT66HeroSelectionScreen::UpdateHeroPreviewVideo()
{
	const FName KnightHeroID(TEXT("Hero_1"));
	const bool bIsKnight = (PreviewedHeroID == KnightHeroID);
	const bool bHasKnightPreview = (KnightPreviewMediaSource != nullptr);

	if (HeroPreviewVideoImage.IsValid())
	{
		HeroPreviewVideoImage->SetVisibility(bIsKnight && bHasKnightPreview ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (HeroPreviewPlaceholderText.IsValid())
	{
		HeroPreviewPlaceholderText->SetVisibility(bIsKnight && bHasKnightPreview ? EVisibility::Collapsed : EVisibility::Visible);
	}

	if (!HeroPreviewMediaPlayer)
	{
		return;
	}
	if (bIsKnight)
	{
		if (!KnightPreviewMediaSource)
		{
			RequestKnightPreviewMediaSource();
			HeroPreviewMediaPlayer->Close();
			return;
		}
		if (!HeroPreviewMediaPlayer->IsPlaying())
		{
			HeroPreviewMediaPlayer->OpenSource(KnightPreviewMediaSource);
		}
	}
	else
	{
		HeroPreviewMediaPlayer->Close();
	}
}

void UT66HeroSelectionScreen::RequestKnightPreviewMediaSource()
{
	if (KnightPreviewMediaSource || KnightPreviewMediaSourceHandle.IsValid() || KnightPreviewMediaSourceAsset.IsNull())
	{
		return;
	}

	KnightPreviewMediaSource = KnightPreviewMediaSourceAsset.Get();
	if (KnightPreviewMediaSource)
	{
		HandleKnightPreviewMediaSourceLoaded();
		return;
	}

	KnightPreviewMediaSourceHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		KnightPreviewMediaSourceAsset.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UT66HeroSelectionScreen::HandleKnightPreviewMediaSourceLoaded));

	if (!KnightPreviewMediaSourceHandle.IsValid())
	{
		HandleKnightPreviewMediaSourceLoaded();
	}
}

void UT66HeroSelectionScreen::HandleKnightPreviewMediaSourceLoaded()
{
	KnightPreviewMediaSourceHandle.Reset();
	KnightPreviewMediaSource = KnightPreviewMediaSourceAsset.Get();
	UpdateHeroPreviewVideo();
}

void UT66HeroSelectionScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}

FReply UT66HeroSelectionScreen::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// Key B switches to Type B (return to Type A via the Type A button only)
	if (InKeyEvent.GetKey() == EKeys::B)
	{
		SelectedBodyType = ET66BodyType::TypeB;
		FT66Style::DeferRebuild(this);
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

