// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66SkinSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66TemporaryBuffUIUtils.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66FrontendGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SToolTip.h"
#include "Brushes/SlateImageBrush.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "FileMediaSource.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66HeroSelection, Log, All);

namespace
{
	constexpr int32 HeroSelectionCarouselVisibleSlots = 7;
	constexpr int32 HeroSelectionCarouselCenterIndex = HeroSelectionCarouselVisibleSlots / 2;

	float GetHeroSelectionCarouselBoxSize(const int32 Offset)
	{
		switch (FMath::Abs(Offset))
		{
		case 0: return 48.f;
		case 1: return 40.f;
		case 2: return 36.f;
		default: return 32.f;
		}
	}

	float GetHeroSelectionCarouselOpacity(const int32 Offset)
	{
		switch (FMath::Abs(Offset))
		{
		case 0: return 1.0f;
		case 1: return 0.82f;
		case 2: return 0.66f;
		default: return 0.52f;
		}
	}

	TSoftObjectPtr<UTexture2D> GetHeroSelectionBalanceIcon()
	{
		return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Assets/TopBar/frontend_topbar_power_coupons_icon.frontend_topbar_power_coupons_icon")));
	}

	FString GetHeroSelectionMedalImagePath(const ET66AccountMedalTier Tier)
	{
		switch (Tier)
		{
		case ET66AccountMedalTier::Bronze:
			return FPaths::ProjectContentDir() / TEXT("UI/Assets/Medals/medal_bronze.png");
		case ET66AccountMedalTier::Silver:
			return FPaths::ProjectContentDir() / TEXT("UI/Assets/Medals/medal_silver.png");
		case ET66AccountMedalTier::Gold:
			return FPaths::ProjectContentDir() / TEXT("UI/Assets/Medals/medal_gold.png");
		case ET66AccountMedalTier::Platinum:
			return FPaths::ProjectContentDir() / TEXT("UI/Assets/Medals/medal_platinum.png");
		case ET66AccountMedalTier::Diamond:
			return FPaths::ProjectContentDir() / TEXT("UI/Assets/Medals/medal_diamond.png");
		case ET66AccountMedalTier::None:
		default:
			return FPaths::ProjectContentDir() / TEXT("UI/Assets/Medals/medal_unproven.png");
		}
	}

	FString ResolveArthurPreviewMoviePath()
	{
		const TArray<FString> CandidatePaths = {
			FPaths::ProjectContentDir() / TEXT("Movies/ArthurPreview.mp4"),
			FPaths::ProjectDir() / TEXT("SourceAssets/Preview Videos/Arthur Preview.mp4"),
			FPaths::ProjectContentDir() / TEXT("Movies/KnightClip.mp4"),
			FPaths::ProjectDir() / TEXT("SourceAssets/Movies/KnightClip.mp4")
		};

		for (const FString& CandidatePath : CandidatePaths)
		{
			if (IFileManager::Get().FileExists(*CandidatePath))
			{
				return FPaths::ConvertRelativePathToFull(CandidatePath);
			}
		}

		return FString();
	}

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

	FText HeroRecordThirdMetricLabel(const bool bShowingCompanionInfo)
	{
		return bShowingCompanionInfo
			? NSLOCTEXT("T66.HeroSelection", "CompanionRecordUnionLabel", "Union")
			: NSLOCTEXT("T66.HeroSelection", "HeroRecordScoreLabel", "Cumulative Score");
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
	const FName SkinEntityID = GetCurrentSkinEntityID();
	const ET66SkinEntityType SkinEntityType = IsShowingCompanionSkins()
		? ET66SkinEntityType::Companion
		: ET66SkinEntityType::Hero;
	if (Skin && !SkinEntityID.IsNone())
	{
		PlaceholderSkins = Skin->GetSkinsForEntity(SkinEntityType, SkinEntityID);
	}
	else if (!IsShowingCompanionSkins())
	{
		// No subsystem or no preview hero: keep the hero list populated with defaults.
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
		ACBalanceTextBlock->SetText(FText::AsNumber(ACBalance));
	}
	UpdateHeroDisplay();
}

void UT66HeroSelectionScreen::AddSkinRowsToBox(const TSharedPtr<SVerticalBox>& Box)
{
	if (!Box.IsValid()) return;
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const bool bCompanionSkins = IsShowingCompanionSkins();
	const FName SkinEntityID = GetCurrentSkinEntityID();
	const FText EquipText = Loc ? Loc->GetText_Equip() : NSLOCTEXT("T66.Common", "Equip", "EQUIP");
	const FText EquippedText = NSLOCTEXT("T66.HeroSelection", "Equipped", "EQUIPPED");
	const FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");
	const FText RefundText = NSLOCTEXT("T66.Common", "Refund", "REFUND");
	const FText SelectCompanionForSkinsText = NSLOCTEXT("T66.HeroSelection", "SelectCompanionForSkins", "Select a companion to manage companion skins.");
	const FButtonStyle& PrimaryButtonStyle = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary");
	const float ActionMinHeight = 34.f;
	const float ActionMinWidth = 104.f;
	const float EquippedMinWidth = 112.f;
	const float BuyButtonMinWidth = 112.f;
	const float BuyButtonHeight = 34.f;
	const int32 SkinActionFontSize = 7;
	const int32 SkinPriceFontSize = 8;
	const int32 SkinTitleFontSize = 9;

	if (PlaceholderSkins.Num() == 0)
	{
		Box->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 6.0f)
			[
				FT66Style::MakePanel(
					SNew(STextBlock)
					.Text(bCompanionSkins && SkinEntityID.IsNone()
						? SelectCompanionForSkinsText
						: NSLOCTEXT("T66.HeroSelection", "NoSkinsAvailable", "No skins available."))
					.Font(FT66Style::Tokens::FontRegular(SkinTitleFontSize))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.AutoWrapText(true),
					FT66PanelParams(ET66PanelType::Panel2)
						.SetColor(FT66Style::IsDotaTheme()
							? FSlateColor(FLinearColor(0.028f, 0.028f, 0.031f, 1.0f))
							: FT66Style::Tokens::Panel2)
						.SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3)))
			];
		return;
	}

	auto ClearSkinPreviewOverride = [this, bCompanionSkins]()
	{
		if (bCompanionSkins)
		{
			PreviewedCompanionSkinIDOverride = NAME_None;
		}
		else
		{
			PreviewSkinIDOverride = NAME_None;
		}
	};

	auto ApplySkinSelection = [this, bCompanionSkins, SkinEntityID, &ClearSkinPreviewOverride](FName SkinID) -> FReply
	{
		if (SkinEntityID.IsNone())
		{
			return FReply::Handled();
		}

		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
			{
				if (bCompanionSkins)
				{
					SkinSub->SetEquippedCompanionSkinID(SkinEntityID, SkinID);
				}
				else
				{
					SkinSub->SetEquippedHeroSkinID(SkinEntityID, SkinID);
					if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
					{
						T66GI->SelectedHeroSkinID = SkinID;
					}
				}
				CommitLocalSelectionsToLobby(true);
				ClearSkinPreviewOverride();
				RefreshSkinsList();
			}
		}

		return FReply::Handled();
	};

	auto PurchaseAndEquipSkin = [this, bCompanionSkins, SkinEntityID, &ApplySkinSelection](FName SkinID, int32 Price) -> FReply
	{
		if (SkinEntityID.IsNone())
		{
			return FReply::Handled();
		}

		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
			{
				const bool bPurchased = bCompanionSkins
					? SkinSub->PurchaseCompanionSkin(SkinEntityID, SkinID, Price)
					: SkinSub->PurchaseHeroSkin(SkinEntityID, SkinID, Price);
				if (bPurchased)
				{
					return ApplySkinSelection(SkinID);
				}
			}
		}

		return FReply::Handled();
	};

	auto RefundOwnedSkin = [this, bCompanionSkins, SkinEntityID, &ClearSkinPreviewOverride](FName SkinID, int32 RefundAmount) -> FReply
	{
		if (SkinEntityID.IsNone())
		{
			return FReply::Handled();
		}

		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
			{
				const bool bRefunded = bCompanionSkins
					? SkinSub->RefundCompanionSkin(SkinEntityID, SkinID, RefundAmount)
					: SkinSub->RefundHeroSkin(SkinEntityID, SkinID, RefundAmount);
				if (bRefunded)
				{
					if (!bCompanionSkins)
					{
						if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
						{
							T66GI->SelectedHeroSkinID = UT66SkinSubsystem::DefaultSkinID;
						}
					}

					ClearSkinPreviewOverride();
					CommitLocalSelectionsToLobby(true);
					RefreshSkinsList();
				}
			}
		}

		return FReply::Handled();
	};

	auto TogglePreviewOverride = [this, bCompanionSkins](FName SkinID) -> FReply
	{
		if (bCompanionSkins)
		{
			PreviewedCompanionSkinIDOverride = (PreviewedCompanionSkinIDOverride == SkinID) ? NAME_None : SkinID;
		}
		else
		{
			PreviewSkinIDOverride = (PreviewSkinIDOverride == SkinID) ? NAME_None : SkinID;
		}
		UpdateHeroDisplay();
		return FReply::Handled();
	};

	for (const FSkinData& Skin : PlaceholderSkins)
	{
		FText SkinDisplayName = Loc ? Loc->GetText_SkinName(Skin.SkinID) : FText::FromName(Skin.SkinID);
		const int32 Price = FMath::Max(0, Skin.CoinCost);
		const FText PriceText = FText::AsNumber(Price);
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
							FOnClicked::CreateLambda([ApplySkinSelection, SkinIDCopy]() { return ApplySkinSelection(SkinIDCopy); }),
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
					SNew(SBox).MinDesiredWidth(ActionMinWidth).MinDesiredHeight(ActionMinHeight)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(!bIsOwned ? 0 : (bIsEquipped ? 2 : 1))
						+ SWidgetSwitcher::Slot()
						[
							FT66Style::MakeButton(FT66ButtonParams(PreviewText,
								FOnClicked::CreateLambda([TogglePreviewOverride, SkinIDCopy]() { return TogglePreviewOverride(SkinIDCopy); }),
								ET66ButtonType::Neutral)
								.SetMinWidth(ActionMinWidth)
								.SetHeight(ActionMinHeight)
								.SetPadding(FMargin(8.f, 3.f, 8.f, 2.f))
								.SetContent(SNew(STextBlock).Text(PreviewText).Font(FT66Style::Tokens::FontBold(SkinActionFontSize)).ColorAndOpacity(FT66Style::Tokens::Text).Justification(ETextJustify::Center)))
						]
						+ SWidgetSwitcher::Slot()
						[
							FT66Style::MakeButton(FT66ButtonParams(EquipText,
								FOnClicked::CreateLambda([ApplySkinSelection, SkinIDCopy]() { return ApplySkinSelection(SkinIDCopy); }),
								ET66ButtonType::Primary)
								.SetMinWidth(ActionMinWidth)
								.SetHeight(ActionMinHeight)
								.SetPadding(FMargin(8.f, 3.f, 8.f, 2.f))
								.SetContent(SNew(STextBlock).Text(EquipText).Font(FT66Style::Tokens::FontBold(SkinActionFontSize)).ColorAndOpacity(FT66Style::Tokens::Text).Justification(ETextJustify::Center)))
						]
						+ SWidgetSwitcher::Slot()
						[
							SNew(SBorder)
							.BorderImage(&PrimaryButtonStyle.Normal)
							.BorderBackgroundColor(FT66Style::IsDotaTheme()
								? FSlateColor(FLinearColor(0.075f, 0.075f, 0.08f, 1.0f))
								: FT66Style::Tokens::Accent2)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Padding(FMargin(8.f, 3.f, 8.f, 2.f))
							[
								SNew(STextBlock)
								.Text(EquippedText)
								.Font(FT66Style::Tokens::FontBold(SkinActionFontSize))
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.Justification(ETextJustify::Center)
							]
						]
					]
				];
			ButtonRow->AddSlot().AutoWidth().Padding(4.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(EquippedMinWidth).MinDesiredHeight(BuyButtonHeight)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(bIsOwned ? 1 : 0)
						+ SWidgetSwitcher::Slot()
						[
						FT66Style::MakeButton(FT66ButtonParams(PriceText,
							FOnClicked::CreateLambda([PurchaseAndEquipSkin, SkinIDCopy, Price]() { return PurchaseAndEquipSkin(SkinIDCopy, Price); }),
							ET66ButtonType::Primary)
							.SetMinWidth(BuyButtonMinWidth)
							.SetHeight(BuyButtonHeight)
							.SetColor(FT66Style::Tokens::Accent)
							.SetPadding(FMargin(8.f, 3.f, 8.f, 2.f))
							.SetContent(
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(PriceText)
									.Font(FT66Style::Tokens::FontBold(SkinPriceFontSize))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center)
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(6.f, 0.f, 0.f, 0.f)
								[
									SNew(SBox)
									.WidthOverride(24.f)
									.HeightOverride(16.f)
									[
										SNew(SOverlay)
										+ SOverlay::Slot()
										[
											SNew(SImage)
											.Image_Lambda([this]() -> const FSlateBrush*
											{
												return ACBalanceIconBrush.IsValid() && ::IsValid(ACBalanceIconBrush->GetResourceObject())
													? ACBalanceIconBrush.Get()
													: nullptr;
											})
										]
										+ SOverlay::Slot()
										.HAlign(HAlign_Center)
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
											.Visibility_Lambda([this]() -> EVisibility
											{
												return ACBalanceIconBrush.IsValid() && ::IsValid(ACBalanceIconBrush->GetResourceObject())
													? EVisibility::Collapsed
													: EVisibility::Visible;
											})
											.Text(FText::FromString(TEXT("CC")))
											.Font(FT66Style::Tokens::FontBold(SkinActionFontSize))
											.ColorAndOpacity(FT66Style::Tokens::Text)
										]
									]
								]
							)
						)
						]
						+ SWidgetSwitcher::Slot()
						[
						FT66Style::MakeButton(FT66ButtonParams(RefundText,
							FOnClicked::CreateLambda([RefundOwnedSkin, SkinIDCopy, Price]() { return RefundOwnedSkin(SkinIDCopy, Price); }),
							ET66ButtonType::Neutral)
							.SetMinWidth(ActionMinWidth)
							.SetHeight(ActionMinHeight)
							.SetPadding(FMargin(8.f, 3.f, 8.f, 2.f))
							.SetContent(SNew(STextBlock).Text(RefundText).Font(FT66Style::Tokens::FontBold(SkinActionFontSize)).ColorAndOpacity(FT66Style::Tokens::Text).Justification(ETextJustify::Center))
						)
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

bool UT66HeroSelectionScreen::IsShowingCompanionSkins() const
{
	return bShowingCompanionSkins;
}

void UT66HeroSelectionScreen::SetShowingCompanionSkins(bool bShowCompanionSkins)
{
	bShowingCompanionSkins = bShowCompanionSkins;
	if (SkinTargetOptions.Num() >= 2)
	{
		CurrentSkinTargetOption = SkinTargetOptions[bShowingCompanionSkins ? 1 : 0];
	}
	RefreshSkinsList();
}

bool UT66HeroSelectionScreen::IsShowingCompanionInfo() const
{
	return bShowingCompanionInfo;
}

void UT66HeroSelectionScreen::SetShowingCompanionInfo(bool bShowCompanionInfo)
{
	bShowingCompanionInfo = bShowCompanionInfo;
	if (InfoTargetOptions.Num() >= 2)
	{
		CurrentInfoTargetOption = InfoTargetOptions[bShowingCompanionInfo ? 1 : 0];
	}

	if (bShowingCompanionInfo)
	{
		bShowingStatsPanel = false;
	}
}

FName UT66HeroSelectionScreen::GetCurrentSkinEntityID() const
{
	if (bShowingCompanionSkins)
	{
		return PreviewedCompanionID;
	}

	return PreviewedHeroID.IsNone() && AllHeroIDs.Num() > 0 ? AllHeroIDs[0] : PreviewedHeroID;
}

FName UT66HeroSelectionScreen::GetEffectivePreviewedCompanionSkinID() const
{
	if (PreviewedCompanionID.IsNone())
	{
		return NAME_None;
	}

	if (!PreviewedCompanionSkinIDOverride.IsNone())
	{
		return PreviewedCompanionSkinIDOverride;
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
		{
			return SkinSub->GetEquippedCompanionSkinID(PreviewedCompanionID);
		}
	}

	return FName(TEXT("Default"));
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
			CommitLocalSelectionsToLobby(true);
			if (AT66HeroPreviewStage* PreviewStage = GetHeroPreviewStage())
			{
				PreviewStage->SetPreviewDifficulty(SelectedDifficulty);
			}
		}
	}
}

void UT66HeroSelectionScreen::OnSkinTargetChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (!NewValue.IsValid())
	{
		return;
	}

	const int32 Index = SkinTargetOptions.IndexOfByKey(NewValue);
	if (Index != INDEX_NONE)
	{
		CurrentSkinTargetOption = NewValue;
		SetShowingCompanionSkins(Index == 1);
	}
}

void UT66HeroSelectionScreen::OnInfoTargetChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (!NewValue.IsValid())
	{
		return;
	}

	const int32 Index = InfoTargetOptions.IndexOfByKey(NewValue);
	if (Index != INDEX_NONE)
	{
		CurrentInfoTargetOption = NewValue;
		SetShowingCompanionInfo(Index == 1);
		FT66Style::DeferRebuild(this);
	}
}

void UT66HeroSelectionScreen::CommitLocalSelectionsToLobby(bool bResetReady)
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedHeroID = PreviewedHeroID;
		GI->SelectedCompanionID = PreviewedCompanionID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->SelectedHeroBodyType = SelectedBodyType;

		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (bResetReady)
			{
				SessionSubsystem->SetLocalLobbyReady(false);
			}
			else
			{
				SessionSubsystem->SyncLocalLobbyProfile();
			}
		}
	}
}

void UT66HeroSelectionScreen::HandlePartyStateChanged()
{
	SyncToSharedPartyScreen();
	ForceRebuildSlate();
}

void UT66HeroSelectionScreen::HandleSessionStateChanged()
{
	SyncToSharedPartyScreen();
	ForceRebuildSlate();
}

void UT66HeroSelectionScreen::SyncToSharedPartyScreen()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI)
	{
		return;
	}

	UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>();
	if (!SessionSubsystem)
	{
		return;
	}

	SelectedDifficulty = SessionSubsystem->GetSharedLobbyDifficulty();
	GI->SelectedDifficulty = SelectedDifficulty;

	if (!SessionSubsystem->IsPartySessionActive() || SessionSubsystem->IsLocalPlayerPartyHost() || !UIManager)
	{
		return;
	}

	const ET66ScreenType DesiredScreen = SessionSubsystem->GetDesiredPartyFrontendScreen();
	if (DesiredScreen == ET66ScreenType::MainMenu && UIManager->GetCurrentScreenType() != ET66ScreenType::MainMenu)
	{
		UIManager->ShowScreen(ET66ScreenType::MainMenu);
	}
}

TSharedRef<SWidget> UT66HeroSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	// Ensure hero list and skin state so skin list and 3D preview match (BuildSlateUI can run before OnScreenActivated).
	RefreshHeroList();
	RefreshCompanionList();
	if (AllHeroIDs.Num() > 0 && PreviewedHeroID.IsNone())
	{
		PreviewedHeroID = AllHeroIDs[0];
	}
	if (PreviewedCompanionID.IsNone())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			PreviewedCompanionID = GI->SelectedCompanionID;
		}
	}
	TArray<FName> CompanionWheelIDs;
	CompanionWheelIDs.Add(NAME_None);
	CompanionWheelIDs.Append(AllCompanionIDs);
	CurrentCompanionIndex = CompanionWheelIDs.IndexOfByKey(PreviewedCompanionID);
	if (CurrentCompanionIndex == INDEX_NONE)
	{
		CurrentCompanionIndex = 0;
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
	FText SkinsText = Loc ? Loc->GetText_Skins() : NSLOCTEXT("T66.HeroSelection", "Skins", "SKINS");
	FText StatsText = Loc ? Loc->GetText_BaseStatsHeader() : NSLOCTEXT("T66.HeroSelection", "BaseStatsHeader", "STATS");
	FText FullStatsText = NSLOCTEXT("T66.HeroSelection", "FullStats", "FULL STATS");
	FText EnterText = Loc ? Loc->GetText_EnterTheTribulation() : NSLOCTEXT("T66.HeroSelection", "EnterTheTribulation", "ENTER THE TRIBULATION");
	FText ReadyText = NSLOCTEXT("T66.HeroSelection", "Ready", "READY");
	FText UnreadyText = NSLOCTEXT("T66.HeroSelection", "Unready", "UNREADY");
	FText WaitingForPartyText = NSLOCTEXT("T66.HeroSelection", "WaitingForParty", "WAITING FOR PARTY");
	FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	FText BuyText = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	FText EquipText = Loc ? Loc->GetText_Equip() : NSLOCTEXT("T66.Common", "Equip", "EQUIP");
	FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");

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
	const FText ACBalanceText = FText::AsNumber(ACBalance);

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66PartySubsystem* PartySubsystem = T66GI ? T66GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = T66GI ? T66GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	const FText NoCompanionText = Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.HeroSelection", "NoCompanion", "No Companion");
	FText CurrentHeroDisplayName = NSLOCTEXT("T66.HeroSelection", "HeroFallbackName", "Hero");
	FText CurrentCompanionDisplayName = NoCompanionText;
	if (T66GI)
	{
		FHeroData HeroNameData;
		const FName HeroNameID = !PreviewedHeroID.IsNone() ? PreviewedHeroID : T66GI->SelectedHeroID;
		if (!HeroNameID.IsNone() && T66GI->GetHeroData(HeroNameID, HeroNameData))
		{
			CurrentHeroDisplayName = Loc ? Loc->GetHeroDisplayName(HeroNameData) : HeroNameData.DisplayName;
		}

		FCompanionData CompanionNameData;
		const FName CompanionNameID = !PreviewedCompanionID.IsNone() ? PreviewedCompanionID : T66GI->SelectedCompanionID;
		if (!CompanionNameID.IsNone() && T66GI->GetCompanionData(CompanionNameID, CompanionNameData))
		{
			CurrentCompanionDisplayName = Loc ? Loc->GetCompanionDisplayName(CompanionNameData) : CompanionNameData.DisplayName;
		}
	}
	if (SessionSubsystem)
	{
		SelectedDifficulty = SessionSubsystem->GetSharedLobbyDifficulty();
	}
	const int32 ActivePartySlots = PartySubsystem ? FMath::Clamp(PartySubsystem->GetPartyMemberCount(), 1, 4) : 1;
	const bool bPartySessionActive = SessionSubsystem && SessionSubsystem->IsPartySessionActive();
	const bool bHasRemotePartyMembers = PartySubsystem && PartySubsystem->HasRemotePartyMembers();
	const bool bUsePartyReadyFlow = bPartySessionActive && bHasRemotePartyMembers;
	const bool bIsLocalPartyHost = !SessionSubsystem || SessionSubsystem->IsLocalPlayerPartyHost();
	const bool bCanEditDifficulty = !bUsePartyReadyFlow || bIsLocalPartyHost;
	const bool bLocalReady = SessionSubsystem && SessionSubsystem->IsLocalLobbyReady();
	const bool bCanStartPartyRun = !bUsePartyReadyFlow || !SessionSubsystem || SessionSubsystem->AreAllPartyMembersReadyForGameplay();
	const FText PrimaryActionText = bUsePartyReadyFlow && !bIsLocalPartyHost
		? (bLocalReady ? UnreadyText : ReadyText)
		: (bCanStartPartyRun ? EnterText : WaitingForPartyText);

	SkinTargetOptions.Empty();
	SkinTargetOptions.Add(MakeShared<FString>(CurrentHeroDisplayName.ToString()));
	SkinTargetOptions.Add(MakeShared<FString>(CurrentCompanionDisplayName.ToString()));
	if (SkinTargetOptions.Num() >= 2)
	{
		CurrentSkinTargetOption = SkinTargetOptions[bShowingCompanionSkins ? 1 : 0];
	}

	InfoTargetOptions.Empty();
	InfoTargetOptions.Add(MakeShared<FString>(CurrentHeroDisplayName.ToString()));
	InfoTargetOptions.Add(MakeShared<FString>(CurrentCompanionDisplayName.ToString()));
	if (InfoTargetOptions.Num() >= 2)
	{
		CurrentInfoTargetOption = InfoTargetOptions[bShowingCompanionInfo ? 1 : 0];
	}
	UT66BuffSubsystem* TempBuffSubsystem = T66GI ? T66GI->GetSubsystem<UT66BuffSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* SelectionTexPool = T66GI ? T66GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	if (!ACBalanceIconBrush.IsValid())
	{
		ACBalanceIconBrush = MakeShared<FSlateBrush>();
		ACBalanceIconBrush->DrawAs = ESlateBrushDrawType::Image;
		ACBalanceIconBrush->Tiling = ESlateBrushTileType::NoTile;
		ACBalanceIconBrush->ImageSize = FVector2D(40.f, 26.f);
		ACBalanceIconBrush->SetResourceObject(nullptr);
	}
	if (SelectionTexPool)
	{
		const TSoftObjectPtr<UTexture2D> BalanceIconSoft = GetHeroSelectionBalanceIcon();
		if (!BalanceIconSoft.IsNull())
		{
			TArray<FSoftObjectPath> BalanceIconPaths;
			BalanceIconPaths.Add(BalanceIconSoft.ToSoftObjectPath());
			SelectionTexPool->EnsureTexturesLoadedSync(BalanceIconPaths);
			ACBalanceIconBrush->SetResourceObject(T66SlateTexture::GetLoaded(SelectionTexPool, BalanceIconSoft));
		}
	}
	const TArray<ET66SecondaryStatType> ActiveTempBuffSlots = TempBuffSubsystem ? TempBuffSubsystem->GetSelectedSingleUseBuffSlots() : TArray<ET66SecondaryStatType>{};
	SelectedTemporaryBuffBrushes.Reset();
	SelectedTemporaryBuffBrushes.SetNum(UT66BuffSubsystem::MaxSelectedSingleUseBuffs);
	for (int32 SlotIndex = 0; SlotIndex < UT66BuffSubsystem::MaxSelectedSingleUseBuffs; ++SlotIndex)
	{
		const ET66SecondaryStatType SlotStat = ActiveTempBuffSlots.IsValidIndex(SlotIndex) ? ActiveTempBuffSlots[SlotIndex] : ET66SecondaryStatType::None;
		SelectedTemporaryBuffBrushes[SlotIndex] = T66IsLiveSecondaryStatType(SlotStat)
			? T66TemporaryBuffUI::CreateSecondaryBuffBrush(SelectionTexPool, this, SlotStat, FVector2D(30.f, 30.f))
			: nullptr;
	}

	if (!PartyHeroPortraitBrush.IsValid())
	{
		PartyHeroPortraitBrush = MakeShared<FSlateBrush>();
	}

	auto MakeSelectedTemporaryBuffSlot = [&](int32 SlotIndex) -> TSharedRef<SWidget>
	{
		const bool bFilled = SelectedTemporaryBuffBrushes.IsValidIndex(SlotIndex) && SelectedTemporaryBuffBrushes[SlotIndex].IsValid();
		const bool bOwnedForSlot = TempBuffSubsystem ? TempBuffSubsystem->IsSelectedSingleUseBuffSlotOwned(SlotIndex) : true;
		return FT66Style::MakeButton(
			FT66ButtonParams(
				FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffSlotClicked, SlotIndex),
				ET66ButtonType::Neutral)
				.SetMinWidth(56.f)
				.SetHeight(56.f)
				.SetPadding(FMargin(0.f))
				.SetColor(bOwnedForSlot ? FT66Style::Tokens::Panel : FLinearColor(0.14f, 0.07f, 0.07f, 1.0f))
				.SetContent(
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(bOwnedForSlot ? FT66Style::Tokens::Panel2 : FLinearColor(0.22f, 0.10f, 0.10f, 1.0f))
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						bFilled
						? StaticCastSharedRef<SWidget>(
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFit)
							[
								SNew(SImage)
								.Image_Lambda([this, SlotIndex]() -> const FSlateBrush*
								{
									return SelectedTemporaryBuffBrushes.IsValidIndex(SlotIndex) && SelectedTemporaryBuffBrushes[SlotIndex].IsValid()
										? SelectedTemporaryBuffBrushes[SlotIndex].Get()
										: nullptr;
								})
								.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, bOwnedForSlot ? 1.0f : 0.55f))
							])
						: StaticCastSharedRef<SWidget>(
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.HeroSelection", "TempBuffEmptySlot", "+"))
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted))
					]
				));
	};

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

	// Ensure we have stable brushes for the visible slots.
	HeroCarouselPortraitBrushes.SetNum(HeroSelectionCarouselVisibleSlots);
	HeroCarouselSlotColors.SetNum(HeroSelectionCarouselVisibleSlots);
	HeroCarouselSlotVisibility.SetNum(HeroSelectionCarouselVisibleSlots);
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
	
	// Show 7 heroes centered on current (prev3..next3).
	for (int32 Offset = -HeroSelectionCarouselCenterIndex; Offset <= HeroSelectionCarouselCenterIndex; Offset++)
	{
		if (AllHeroIDs.Num() == 0) break;

		const float BoxSize = GetHeroSelectionCarouselBoxSize(Offset);
		const float Opacity = GetHeroSelectionCarouselOpacity(Offset);
		const int32 SlotIdx = Offset + HeroSelectionCarouselCenterIndex;
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

		TSharedRef<SWidget> InteractiveCarouselSlot = CarouselSlotWidget;
		if (Offset == 0)
		{
			InteractiveCarouselSlot =
				SNew(SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.ContentPadding(0.f)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleHeroGridClicked))
				[
					CarouselSlotWidget
				];
		}

		HeroCarousel->AddSlot()
		.AutoWidth()
		.Padding(3.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(BoxSize)
			.HeightOverride(BoxSize)
			[
				InteractiveCarouselSlot
			]
			];
	}

	TSharedRef<SHorizontalBox> CompanionCarousel = SNew(SHorizontalBox);
	CompanionCarouselPortraitBrushes.SetNum(HeroSelectionCarouselVisibleSlots);
	CompanionCarouselSlotColors.SetNum(HeroSelectionCarouselVisibleSlots);
	CompanionCarouselSlotVisibility.SetNum(HeroSelectionCarouselVisibleSlots);
	CompanionCarouselSlotLabels.SetNum(HeroSelectionCarouselVisibleSlots);
	for (int32 i = 0; i < CompanionCarouselPortraitBrushes.Num(); ++i)
	{
		if (!CompanionCarouselPortraitBrushes[i].IsValid())
		{
			CompanionCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			CompanionCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(48.f, 48.f);
		}
	}
	RefreshCompanionCarouselPortraits();

	for (int32 Offset = -HeroSelectionCarouselCenterIndex; Offset <= HeroSelectionCarouselCenterIndex; Offset++)
	{
		const float BoxSize = GetHeroSelectionCarouselBoxSize(Offset);
		const float Opacity = GetHeroSelectionCarouselOpacity(Offset);
		const int32 SlotIdx = Offset + HeroSelectionCarouselCenterIndex;
		const TSharedRef<SWidget> CompanionSlotWidget = bDotaTheme
			? StaticCastSharedRef<SWidget>(FT66Style::MakeSlotFrame(
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image_Lambda([this, SlotIdx]() -> const FSlateBrush*
					{
						return CompanionCarouselPortraitBrushes.IsValidIndex(SlotIdx) ? CompanionCarouselPortraitBrushes[SlotIdx].Get() : nullptr;
					})
					.Visibility_Lambda([this, SlotIdx]() -> EVisibility
					{
						return CompanionCarouselSlotVisibility.IsValidIndex(SlotIdx)
							? CompanionCarouselSlotVisibility[SlotIdx]
							: EVisibility::Collapsed;
					})
					.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([this, SlotIdx]() -> FText
					{
						return CompanionCarouselSlotLabels.IsValidIndex(SlotIdx) ? CompanionCarouselSlotLabels[SlotIdx] : FText::GetEmpty();
					})
					.Font(FT66Style::Tokens::FontBold(9))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Visibility_Lambda([this, SlotIdx]() -> EVisibility
					{
						return CompanionCarouselSlotLabels.IsValidIndex(SlotIdx) && !CompanionCarouselSlotLabels[SlotIdx].IsEmpty()
							? EVisibility::Visible
							: EVisibility::Collapsed;
					})
				],
				TAttribute<FSlateColor>::CreateLambda([this, SlotIdx]() -> FSlateColor
				{
					return CompanionCarouselSlotColors.IsValidIndex(SlotIdx)
						? FSlateColor(CompanionCarouselSlotColors[SlotIdx])
						: FSlateColor(FT66Style::SlotInner());
				}),
				FMargin(2.f)))
			: StaticCastSharedRef<SWidget>(SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderBackgroundColor_Lambda([this, SlotIdx]() -> FSlateColor
					{
						return CompanionCarouselSlotColors.IsValidIndex(SlotIdx)
							? FSlateColor(CompanionCarouselSlotColors[SlotIdx])
							: FSlateColor(FLinearColor(0.2f, 0.2f, 0.25f, 1.0f));
					})
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				]
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image_Lambda([this, SlotIdx]() -> const FSlateBrush*
					{
						return CompanionCarouselPortraitBrushes.IsValidIndex(SlotIdx) ? CompanionCarouselPortraitBrushes[SlotIdx].Get() : nullptr;
					})
					.Visibility_Lambda([this, SlotIdx]() -> EVisibility
					{
						return CompanionCarouselSlotVisibility.IsValidIndex(SlotIdx)
							? CompanionCarouselSlotVisibility[SlotIdx]
							: EVisibility::Collapsed;
					})
					.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([this, SlotIdx]() -> FText
					{
						return CompanionCarouselSlotLabels.IsValidIndex(SlotIdx) ? CompanionCarouselSlotLabels[SlotIdx] : FText::GetEmpty();
					})
					.Font(FT66Style::Tokens::FontBold(9))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Visibility_Lambda([this, SlotIdx]() -> EVisibility
					{
						return CompanionCarouselSlotLabels.IsValidIndex(SlotIdx) && !CompanionCarouselSlotLabels[SlotIdx].IsEmpty()
							? EVisibility::Visible
							: EVisibility::Collapsed;
					})
				]);

		TSharedRef<SWidget> InteractiveCompanionSlot = CompanionSlotWidget;
		if (Offset == 0)
		{
			InteractiveCompanionSlot =
				SNew(SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.ContentPadding(0.f)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleCompanionGridClicked))
				[
					CompanionSlotWidget
				];
		}

		CompanionCarousel->AddSlot()
		.AutoWidth()
		.Padding(3.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(BoxSize)
			.HeightOverride(BoxSize)
			[
				InteractiveCompanionSlot
			]
		];
	}

	const FTextBlockStyle& TxtButton = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button");
	const float CenterPanelFill = 0.40f;
	const float TopBarBottomGap = 0.f;
	const float PanelBottomInset = 0.f;
	const float PanelGap = 0.f;
	const auto ShrinkFont = [](int32 BaseSize, int32 Minimum = 7) -> int32 { return FMath::Max(Minimum, BaseSize - 4); };
	const float SidePanelMinWidth = 380.f;
	const float BackButtonWidth = 84.f;
	const float FooterToggleWidth = 70.f;
	const float FooterToggleHeight = 28.f;
	const float FooterActionHeight = 46.f;
	const float BalanceBadgeIconWidth = 40.f;
	const float BalanceBadgeIconHeight = 24.f;
	const int32 ScreenHeaderFontSize = ShrinkFont(15, 10);
	const int32 BodyToggleFontSize = ShrinkFont(9, 7);
	const int32 PrimaryCtaFontSize = ShrinkFont(11, 7);
	const int32 HeroArrowFontSize = ShrinkFont(12, 8);
	const int32 ACBalanceFontSize = ShrinkFont(14, 10);
	const int32 HeroNameFontSize = ShrinkFont(13, 9);
	const int32 SecondaryButtonFontSize = ShrinkFont(9, 7);
	const int32 EntityDropdownFontSize = SecondaryButtonFontSize + 3;
	const int32 BodyTextFontSize = ShrinkFont(10, 7);
	const int32 BackButtonFontSize = ShrinkFont(14, 10);
	const int32 DifficultyMenuFontSize = PrimaryCtaFontSize;
	const float HeroArrowButtonWidth = bDotaTheme ? 30.f : 28.f;
	const float HeroArrowButtonHeight = bDotaTheme ? 26.f : 24.f;
	const TAttribute<FMargin> ScreenSafePadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 0.f, 0.f, 0.f);
	});

	auto MakeTemporaryBuffLoadoutPanel = [this,
		bDotaTheme,
		SelectionInsetFill,
		TempBuffSubsystem,
		SecondaryButtonFontSize,
		&MakeSelectedTemporaryBuffSlot]() -> TSharedRef<SWidget>
	{
		return FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)[MakeSelectedTemporaryBuffSlot(0)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)[MakeSelectedTemporaryBuffSlot(1)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)[MakeSelectedTemporaryBuffSlot(2)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)[MakeSelectedTemporaryBuffSlot(3)]
				+ SHorizontalBox::Slot().AutoWidth()[MakeSelectedTemporaryBuffSlot(4)]
			],
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(bDotaTheme ? SelectionInsetFill : FT66Style::Tokens::Panel)
				.SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3)));
	};

	auto MakeBalanceBadge = [this,
		bDotaTheme,
		SelectionInsetFill,
		BalanceBadgeIconWidth,
		BalanceBadgeIconHeight,
		ACBalanceText,
		ACBalanceFontSize,
		SecondaryButtonFontSize]() -> TSharedRef<SWidget>
	{
		return FT66Style::MakePanel(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(BalanceBadgeIconWidth)
				.HeightOverride(BalanceBadgeIconHeight)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFit)
						[
							SNew(SImage)
							.Image_Lambda([this]() -> const FSlateBrush*
							{
								return ACBalanceIconBrush.IsValid() && ::IsValid(ACBalanceIconBrush->GetResourceObject())
									? ACBalanceIconBrush.Get()
									: nullptr;
							})
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "CurrencyBadgeFallback", "CC"))
						.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Visibility_Lambda([this]() -> EVisibility
						{
							return ACBalanceIconBrush.IsValid() && ::IsValid(ACBalanceIconBrush->GetResourceObject())
								? EVisibility::Collapsed
								: EVisibility::Visible;
						})
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				SAssignNew(ACBalanceTextBlock, STextBlock)
				.Text(ACBalanceText)
				.Font(FT66Style::Tokens::FontBold(ACBalanceFontSize))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			],
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(bDotaTheme ? SelectionInsetFill : FT66Style::Tokens::Panel)
				.SetPadding(FMargin(10.f, 5.f)));
	};

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
				.SetPadding(FMargin(6.f, 3.f, 6.f, 2.f))
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
		HeroArrowButtonWidth,
		HeroArrowButtonHeight,
		HeroArrowFontSize,
		HeroCarousel]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 10.0f, 0.0f)
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
			.Padding(10.0f, 0.0f, 0.0f, 0.0f)
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

	auto MakeBodyToggleStrip = [MakeBodyTogglePanel]() -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			[
				MakeBodyTogglePanel()
			];
	};

	auto MakeCompanionStripControls = [this,
		HeroArrowButtonWidth,
		HeroArrowButtonHeight,
		HeroArrowFontSize,
		CompanionCarousel]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 10.0f, 0.0f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					NSLOCTEXT("T66.Common", "CompanionPrev", "<"),
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleCompanionPrevClicked),
					ET66ButtonType::Neutral)
					.SetMinWidth(HeroArrowButtonWidth)
					.SetHeight(HeroArrowButtonHeight)
					.SetFontSize(HeroArrowFontSize))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				CompanionCarousel
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.0f, 0.0f, 0.0f, 0.0f)
			[
				FT66Style::MakeButton(FT66ButtonParams(
					NSLOCTEXT("T66.Common", "CompanionNext", ">"),
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleCompanionNextClicked),
					ET66ButtonType::Neutral)
					.SetMinWidth(HeroArrowButtonWidth)
					.SetHeight(HeroArrowButtonHeight)
					.SetFontSize(HeroArrowFontSize))
			];
	};

	auto MakeSkinTargetDropdown = [this, EntityDropdownFontSize]() -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(152.f)
			[
				FT66Style::MakeDropdown(FT66DropdownParams(
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() -> FText
						{
							return CurrentSkinTargetOption.IsValid()
								? FText::FromString(*CurrentSkinTargetOption)
								: NSLOCTEXT("T66.HeroSelection", "SkinTargetHeroFallback", "HERO");
						})
						.Font(FT66Style::Tokens::FontBold(EntityDropdownFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
					],
					[this, EntityDropdownFontSize]()
					{
						TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
						for (const TSharedPtr<FString>& Opt : SkinTargetOptions)
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
											OnSkinTargetChanged(Captured, ESelectInfo::Direct);
											FSlateApplication::Get().DismissAllMenus();
											return FReply::Handled();
										}),
										ET66ButtonType::Neutral)
										.SetMinWidth(0.f)
										.SetHeight(36.f)
										.SetFontSize(EntityDropdownFontSize)
										.SetPadding(FMargin(10.f, 8.f, 10.f, 6.f))
										.SetContent(
											SNew(SOverlay)
											+ SOverlay::Slot()
											.HAlign(HAlign_Center)
											.VAlign(VAlign_Center)
											[
												SNew(STextBlock)
												.Text(FText::FromString(*Opt))
												.Font(FT66Style::Tokens::FontBold(EntityDropdownFontSize))
												.ColorAndOpacity(FT66Style::Tokens::Text)
												.Justification(ETextJustify::Center)
											]))
								];
						}
						return Box;
					})
					.SetMinWidth(0.f)
					.SetHeight(36.f)
					.SetPadding(FMargin(10.f, 7.f)))
			];
	};

	auto MakeInfoTargetDropdown = [this, EntityDropdownFontSize]() -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(152.f)
			[
				FT66Style::MakeDropdown(FT66DropdownParams(
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() -> FText
						{
							return CurrentInfoTargetOption.IsValid()
								? FText::FromString(*CurrentInfoTargetOption)
								: NSLOCTEXT("T66.HeroSelection", "InfoTargetHeroFallback", "Hero");
						})
						.Font(FT66Style::Tokens::FontBold(EntityDropdownFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
					],
					[this, EntityDropdownFontSize]()
					{
						TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
						for (const TSharedPtr<FString>& Opt : InfoTargetOptions)
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
											OnInfoTargetChanged(Captured, ESelectInfo::Direct);
											FSlateApplication::Get().DismissAllMenus();
											return FReply::Handled();
										}),
										ET66ButtonType::Neutral)
										.SetMinWidth(0.f)
										.SetHeight(36.f)
										.SetFontSize(EntityDropdownFontSize)
										.SetPadding(FMargin(10.f, 8.f, 10.f, 6.f))
										.SetContent(
											SNew(SOverlay)
											+ SOverlay::Slot()
											.HAlign(HAlign_Center)
											.VAlign(VAlign_Center)
											[
												SNew(STextBlock)
												.Text(FText::FromString(*Opt))
												.Font(FT66Style::Tokens::FontBold(EntityDropdownFontSize))
												.ColorAndOpacity(FT66Style::Tokens::Text)
												.Justification(ETextJustify::Center)
											]))
								];
						}
						return Box;
					})
					.SetMinWidth(0.f)
					.SetHeight(36.f)
					.SetPadding(FMargin(10.f, 7.f)))
			];
	};

	auto MakePartyBox = [this,
		ActivePartySlots,
		PartySubsystem,
		T66GI,
		bUsePartyReadyFlow,
		bCanStartPartyRun]() -> TSharedRef<SWidget>
	{
		const FLinearColor ShellFill(0.f, 0.f, 0.f, 0.985f);
		const FLinearColor LeaderSlotAccent(0.29f, 0.24f, 0.13f, 1.0f);
		const FLinearColor PartySlotAccent(0.15f, 0.17f, 0.19f, 1.0f);
		const FLinearColor PartySlotAccentInactive(0.08f, 0.09f, 0.10f, 1.0f);
		const FLinearColor PlaceholderTint(0.20f, 0.22f, 0.24f, 0.55f);
		const FLinearColor ReadyFill(0.16f, 0.44f, 0.21f, 1.0f);
		const FLinearColor ReadyStroke(0.55f, 0.84f, 0.60f, 1.0f);
		const FLinearColor NotReadyFill(0.48f, 0.14f, 0.14f, 1.0f);
		const FLinearColor NotReadyStroke(0.92f, 0.48f, 0.48f, 1.0f);
		const TArray<FT66PartyMemberEntry> PartyMembers = PartySubsystem ? PartySubsystem->GetPartyMembers() : TArray<FT66PartyMemberEntry>();
		UT66SteamHelper* SteamHelper = T66GI ? T66GI->GetSubsystem<UT66SteamHelper>() : nullptr;
		PartyAvatarBrushes.SetNum(4);
		const bool bTreatPartyAsReadyByDefault = !bUsePartyReadyFlow || PartyMembers.Num() <= 1;

		auto MakeReadyIndicator = [&](const bool bReady, const float Size, const int32 FontSize) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.WidthOverride(Size)
				.HeightOverride(Size)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(bReady ? ReadyStroke : NotReadyStroke)
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(bReady ? ReadyFill : NotReadyFill)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(bReady ? TEXT("✓") : TEXT("X")))
							.Font(FT66Style::Tokens::FontBold(FontSize))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
						]
					]
				];
		};

		auto ResolvePartyAvatarTexture = [PartySubsystem, SteamHelper](const FT66PartyMemberEntry& Member) -> UTexture2D*
		{
			if (!SteamHelper)
			{
				return nullptr;
			}

			if (UTexture2D* ExactAvatar = SteamHelper->GetAvatarTextureForSteamId(Member.PlayerId))
			{
				return ExactAvatar;
			}

			if (Member.bIsLocal || Member.DisplayName.Equals(SteamHelper->GetLocalDisplayName(), ESearchCase::IgnoreCase))
			{
				if (UTexture2D* LocalAvatar = SteamHelper->GetLocalAvatarTexture())
				{
					return LocalAvatar;
				}
			}

			const FT66PartyFriendEntry* MatchingFriend = PartySubsystem ? PartySubsystem->GetFriends().FindByPredicate([&Member](const FT66PartyFriendEntry& Friend)
			{
				return Friend.DisplayName.Equals(Member.DisplayName, ESearchCase::IgnoreCase);
			}) : nullptr;
			return MatchingFriend ? SteamHelper->GetAvatarTextureForSteamId(MatchingFriend->PlayerId) : nullptr;
		};

		TSharedRef<SHorizontalBox> PartySlots = SNew(SHorizontalBox);
		for (int32 SlotIndex = 0; SlotIndex < 4; ++SlotIndex)
		{
			const FT66PartyMemberEntry* PartyMember = PartyMembers.IsValidIndex(SlotIndex) ? &PartyMembers[SlotIndex] : nullptr;
			UTexture2D* AvatarTexture = nullptr;
			if (PartyMember)
			{
				AvatarTexture = ResolvePartyAvatarTexture(*PartyMember);
			}

			if (!PartyAvatarBrushes[SlotIndex].IsValid())
			{
				PartyAvatarBrushes[SlotIndex] = MakeShared<FSlateBrush>();
				PartyAvatarBrushes[SlotIndex]->DrawAs = ESlateBrushDrawType::Image;
				PartyAvatarBrushes[SlotIndex]->Tiling = ESlateBrushTileType::NoTile;
			}
			PartyAvatarBrushes[SlotIndex]->ImageSize = FVector2D(52.f, 52.f);
			PartyAvatarBrushes[SlotIndex]->SetResourceObject(AvatarTexture);

			const bool bHasAvatar = AvatarTexture != nullptr;
			const bool bPartyEnabledSlot = SlotIndex < ActivePartySlots;
			const bool bOccupiedSlot = PartyMember != nullptr;
			const bool bLeaderSlot = SlotIndex == 0;
			const bool bMemberReady = bOccupiedSlot && (bTreatPartyAsReadyByDefault || PartyMember->bReady);
			const FLinearColor SlotAccent = bLeaderSlot
				? LeaderSlotAccent
				: (bOccupiedSlot ? PartySlotAccent : PartySlotAccentInactive);
			const float PlaceholderOpacity = bPartyEnabledSlot ? 0.55f : 0.28f;

			const TSharedRef<SWidget> SlotBaseContent =
				bHasAvatar
				? StaticCastSharedRef<SWidget>(
					SNew(SImage)
					.Image_Lambda([this, SlotIndex]() -> const FSlateBrush*
					{
						return PartyAvatarBrushes.IsValidIndex(SlotIndex) && PartyAvatarBrushes[SlotIndex].IsValid()
							? PartyAvatarBrushes[SlotIndex].Get()
							: nullptr;
					}))
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

			const TSharedRef<SWidget> SlotContent =
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SlotBaseContent
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				.Padding(FMargin(0.f, 0.f, -9.f, 0.f))
				[
					bOccupiedSlot
						? StaticCastSharedRef<SWidget>(MakeReadyIndicator(bMemberReady, 18.f, 9))
						: StaticCastSharedRef<SWidget>(SNew(SSpacer))
				];

			PartySlots->AddSlot()
				.AutoWidth()
				.Padding(SlotIndex > 0 ? FMargin(10.f, 0.f, 0.f, 0.f) : FMargin(0.f))
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
			.MinDesiredHeight(84.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(ShellFill)
				.Padding(FMargin(12.f, 12.f, 12.f, 12.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						PartySlots
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(SSpacer)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						MakeReadyIndicator(bCanStartPartyRun, 52.f, 18)
					]
				]
			];
	};

	auto MakeRunControls = [this,
		bUsePartyReadyFlow,
		bIsLocalPartyHost,
		bCanEditDifficulty,
		bCanStartPartyRun,
		PrimaryActionText,
		PrimaryCtaFontSize,
		DifficultyMenuFontSize,
		FooterActionHeight,
		Loc]() -> TSharedRef<SWidget>
	{
		const FLinearColor ShellFill(0.f, 0.f, 0.f, 0.985f);
		auto WrapRunControls = [ShellFill](const TSharedRef<SWidget>& Content) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.MinDesiredHeight(84.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(ShellFill)
					.Padding(FMargin(12.f))
					[
						Content
					]
				];
		};

		if (bUsePartyReadyFlow && !bIsLocalPartyHost)
		{
			return WrapRunControls(
				FT66Style::MakeButton(FT66ButtonParams(
					PrimaryActionText,
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleEnterClicked),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetHeight(FooterActionHeight)
					.SetPadding(FMargin(12.f, 8.f))
					.SetFontSize(PrimaryCtaFontSize)));
		}

		return WrapRunControls(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.34f)
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox)
				.IsEnabled(bCanEditDifficulty)
				[
					FT66Style::MakeDropdown(FT66DropdownParams(
						SNew(SOverlay)
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text_Lambda([this, Loc]() -> FText
							{
								return CurrentDifficultyOption.IsValid()
									? FText::FromString(*CurrentDifficultyOption)
									: (Loc ? Loc->GetText_Easy() : NSLOCTEXT("T66.Difficulty", "Easy", "Easy"));
							})
							.Font(FT66Style::Tokens::FontBold(PrimaryCtaFontSize))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
						],
						[this, FooterActionHeight, DifficultyMenuFontSize]()
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
											.SetMinWidth(0.f)
											.SetHeight(FooterActionHeight)
											.SetFontSize(DifficultyMenuFontSize)
											.SetPadding(FMargin(10.f, 8.f, 10.f, 6.f))
											.SetContent(
												SNew(SOverlay)
												+ SOverlay::Slot()
												.HAlign(HAlign_Center)
												.VAlign(VAlign_Center)
												[
													SNew(STextBlock)
													.Text(FText::FromString(*Opt))
													.Font(FT66Style::Tokens::FontBold(DifficultyMenuFontSize))
													.ColorAndOpacity(FT66Style::Tokens::Text)
													.Justification(ETextJustify::Center)
												]))
									];
							}
							return Box;
						})
						.SetMinWidth(0.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(10.f, 8.f)))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.66f)
			[
				SNew(SBox)
				.IsEnabled(bCanStartPartyRun)
				[
					FT66Style::MakeButton(FT66ButtonParams(
						PrimaryActionText,
						FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleEnterClicked),
						bCanStartPartyRun ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
						.SetMinWidth(0.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(12.f, 8.f))
						.SetFontSize(PrimaryCtaFontSize)
						.SetContent(SNew(STextBlock)
							.Text(PrimaryActionText)
							.Font(FT66Style::Tokens::FontBold(PrimaryCtaFontSize))
							.ColorAndOpacity(FT66Style::Tokens::Text)))
				]
			]);
	};

	TSharedRef<SWidget> LeftPanelSwitcher =
		SNew(SWidgetSwitcher)
		.WidgetIndex_Lambda([this]() -> int32 { return bShowingStatsPanel ? 1 : 0; })
		+ SWidgetSwitcher::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSkinTargetDropdown()
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SkinsListBoxWidget.ToSharedRef()
				]
			]
		]
		+ SWidgetSwitcher::Slot()
		[
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
					.Text(FullStatsText)
					.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					FT66Style::MakeButton(
						FT66ButtonParams(
							SkinsText,
							FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleStatsClicked),
							ET66ButtonType::Neutral)
						.SetMinWidth(88.f)
						.SetFontSize(SecondaryButtonFontSize)
						.SetColor(TAttribute<FSlateColor>::CreateLambda([this]() -> FSlateColor
						{
							return bShowingStatsPanel ? FT66Style::ButtonPrimary() : FT66Style::ButtonNeutral();
						})))
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(HeroFullStatsWidget, STextBlock)
					.Text(NSLOCTEXT("T66.HeroSelection", "SelectHeroStatsHint", "Select a hero to view their full stats."))
					.Font(FT66Style::Tokens::FontRegular(BodyTextFontSize))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.AutoWrapText(true)
				]
			]
		];

	TSharedRef<SWidget> LeftSidePanel = FT66Style::MakePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(BackButtonWidth)
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
			[
				SNew(SBox)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				MakeBalanceBadge()
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			LeftPanelSwitcher
		],
		FT66PanelParams(ET66PanelType::Panel)
			.SetColor(bDotaTheme ? SelectionPanelFill : FT66Style::Tokens::Panel)
			.SetPadding(FMargin(FT66Style::Tokens::Space3)));

	TSharedRef<SWidget> CenterColumnWidget =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeSelectionBar(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					MakeHeroStripControls()
				])
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f)
		[
			MakeBodyToggleStrip()
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(0.0f, 0.0f, 0.0f, 0.0f)
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				CreateHeroPreviewWidget(HeroPreviewColor)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, TopBarBottomGap, 0.0f, 0.0f)
		[
			MakeSelectionBar(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					MakeCompanionStripControls()
				])
		];

	auto MakeMedalInfoTooltip = [SecondaryButtonFontSize]() -> TSharedRef<SToolTip>
	{
		auto MakeTierLine = [SecondaryButtonFontSize](const FText& LeftText, const FText& RightText) -> TSharedRef<SWidget>
		{
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.f, 0.f, 12.f, 0.f)
				[
					SNew(STextBlock)
					.Text(LeftText)
					.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNew(STextBlock)
					.Text(RightText)
					.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				];
		};

		return SNew(SToolTip)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "MedalTooltipHeader", "MEDALS"))
						.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 1))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[MakeTierLine(NSLOCTEXT("T66.HeroSelection", "MedalTierNone", "Unproven"), NSLOCTEXT("T66.HeroSelection", "MedalTierNoneDesc", "No tribulation clear yet."))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[MakeTierLine(NSLOCTEXT("T66.HeroSelection", "MedalTierBronze", "Bronze"), NSLOCTEXT("T66.HeroSelection", "MedalTierBronzeDesc", "Cleared Easy."))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[MakeTierLine(NSLOCTEXT("T66.HeroSelection", "MedalTierSilver", "Silver"), NSLOCTEXT("T66.HeroSelection", "MedalTierSilverDesc", "Cleared Medium."))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[MakeTierLine(NSLOCTEXT("T66.HeroSelection", "MedalTierGold", "Gold"), NSLOCTEXT("T66.HeroSelection", "MedalTierGoldDesc", "Cleared Hard."))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[MakeTierLine(NSLOCTEXT("T66.HeroSelection", "MedalTierPlatinum", "Platinum"), NSLOCTEXT("T66.HeroSelection", "MedalTierPlatinumDesc", "Cleared Very Hard."))]
					+ SVerticalBox::Slot().AutoHeight()[MakeTierLine(NSLOCTEXT("T66.HeroSelection", "MedalTierDiamond", "Diamond"), NSLOCTEXT("T66.HeroSelection", "MedalTierDiamondDesc", "Cleared Impossible."))],
					FT66PanelParams(ET66PanelType::Panel)
						.SetColor(FT66Style::Tokens::Panel2)
						.SetPadding(FMargin(12.f, 10.f)))
			];
	};

	TSharedRef<SWidget> RightInfoBody =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SBox)
			.HeightOverride(44.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.HeroSelection", "HeroSelectionGameTitle", "CHADPOCALYPSE"))
				.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize + 1))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			MakeInfoTargetDropdown()
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
						.Image_Lambda([this]() -> const FSlateBrush*
						{
							return HeroPreviewVideoBrush.IsValid() && ::IsValid(HeroPreviewMediaTexture)
								? HeroPreviewVideoBrush.Get()
								: nullptr;
						})
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
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.15f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordMedalLabel", "Medal"))
								.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(6.f, 0.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordMedalInfo", "?"))
								.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								.ToolTip(MakeMedalInfoTooltip())
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(26.f)
								.HeightOverride(26.f)
								[
									SAssignNew(HeroRecordMedalImageWidget, SImage)
									.Image_Lambda([this]() -> const FSlateBrush*
									{
										return HeroRecordMedalBrush.IsValid() ? HeroRecordMedalBrush.Get() : nullptr;
									})
								]
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							.VAlign(VAlign_Center)
							.Padding(8.f, 0.f, 0.f, 0.f)
							[
								SAssignNew(HeroRecordMedalWidget, STextBlock)
								.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordMedalDefault", "Unproven"))
								.Font(FT66Style::Tokens::FontBold(BodyTextFontSize + 2))
								.ColorAndOpacity(HeroRecordMedalColor(ET66AccountMedalTier::None))
							]
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
					+ SHorizontalBox::Slot()
					.FillWidth(0.95f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(HeroRecordThirdLabelWidget, STextBlock)
							.Text(HeroRecordThirdMetricLabel(false))
							.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f, 0.0f, 0.0f)
						[
							SAssignNew(HeroRecordThirdValueWidget, STextBlock)
							.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordThirdDefault", "0"))
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
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f, 0.0f, 8.0f)
				.HAlign(HAlign_Left)
				[
					SNew(SBox)
					.WidthOverride(118.f)
					.Visibility_Lambda([this]() -> EVisibility
					{
						return bShowingCompanionInfo ? EVisibility::Collapsed : EVisibility::Visible;
					})
					[
						FT66Style::MakeButton(
							FT66ButtonParams(
								StatsText,
								FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleStatsClicked),
								ET66ButtonType::Neutral)
							.SetMinWidth(0.f)
							.SetHeight(34.f)
							.SetFontSize(SecondaryButtonFontSize)
							.SetPadding(FMargin(6.f, 3.f, 6.f, 2.f))
							.SetColor(TAttribute<FSlateColor>::CreateLambda([this]() -> FSlateColor
							{
								return bShowingStatsPanel ? FT66Style::ButtonPrimary() : FT66Style::ButtonNeutral();
							})))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(HeroDescWidget, STextBlock)
					.Text(NSLOCTEXT("T66.HeroSelection", "SelectHeroDescriptionHint", "Select a hero to view their stats."))
					.Font(FT66Style::Tokens::FontRegular(BodyTextFontSize + 2))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.AutoWrapText(true)
				]
			]
		];

	TSharedRef<SWidget> RightSidePanel = FT66Style::MakePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			RightInfoBody
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 8.0f, 0.0f, 0.0f)
		[
			MakeTemporaryBuffLoadoutPanel()
		],
		FT66PanelParams(ET66PanelType::Panel)
			.SetColor(bDotaTheme ? SelectionPanelFill : FT66Style::Tokens::Panel)
			.SetPadding(FMargin(FT66Style::Tokens::Space4)));

	TSharedRef<SWidget> LeftFooterPanel =
		SNew(SBox)
		.WidthOverride(SidePanelMinWidth)
		[
			MakePartyBox()
		];

	TSharedRef<SWidget> RightFooterPanel =
		SNew(SBox)
		.WidthOverride(SidePanelMinWidth)
		[
			MakeRunControls()
		];

	TSharedRef<SWidget> Root = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
		.BorderBackgroundColor(FLinearColor::Transparent)
		.Padding(ScreenSafePadding)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SBox)
					.WidthOverride(SidePanelMinWidth)
					[
						LeftSidePanel
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f)
				[
					LeftFooterPanel
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(PanelGap, 0.0f, PanelGap, 0.0f)
			[
				CenterColumnWidget
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SBox)
					.WidthOverride(SidePanelMinWidth)
					[
						RightSidePanel
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f)
				[
					RightFooterPanel
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
FReply UT66HeroSelectionScreen::HandleCompanionPrevClicked()
{
	PreviewPreviousCompanion();
	FT66Style::DeferRebuild(this);
	return FReply::Handled();
}
FReply UT66HeroSelectionScreen::HandleCompanionNextClicked()
{
	PreviewNextCompanion();
	FT66Style::DeferRebuild(this);
	return FReply::Handled();
}
FReply UT66HeroSelectionScreen::HandleHeroGridClicked() { OnHeroGridClicked(); return FReply::Handled(); }
FReply UT66HeroSelectionScreen::HandleCompanionGridClicked() { OnCompanionGridClicked(); return FReply::Handled(); }
FReply UT66HeroSelectionScreen::HandleCompanionClicked() { OnChooseCompanionClicked(); return FReply::Handled(); }
FReply UT66HeroSelectionScreen::HandleTemporaryBuffSlotClicked(int32 SlotIndex)
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66BuffSubsystem* Buffs = GI->GetSubsystem<UT66BuffSubsystem>())
		{
			Buffs->SetSelectedSingleUseBuffEditSlotIndex(SlotIndex);
		}
	}

	ShowModal(ET66ScreenType::TemporaryBuffSelection);
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleSelectBuffsClicked() { ShowModal(ET66ScreenType::TemporaryBuffSelection); return FReply::Handled(); }
FReply UT66HeroSelectionScreen::HandleLoreClicked()
{
	bShowingLore = !bShowingLore;
	UpdateHeroDisplay();
	return FReply::Handled();
}
FReply UT66HeroSelectionScreen::HandleStatsClicked()
{
	if (bShowingCompanionInfo)
	{
		return FReply::Handled();
	}

	bShowingStatsPanel = !bShowingStatsPanel;
	return FReply::Handled();
}
FReply UT66HeroSelectionScreen::HandleEnterClicked() { OnEnterTribulationClicked(); return FReply::Handled(); }
FReply UT66HeroSelectionScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }
FReply UT66HeroSelectionScreen::HandleBackToPartyClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleBodyTypeAClicked()
{
	SelectedBodyType = ET66BodyType::TypeA;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedHeroBodyType = SelectedBodyType;
	}
	CommitLocalSelectionsToLobby(true);
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
	CommitLocalSelectionsToLobby(true);
	UpdateHeroDisplay(); // Update 3D preview immediately for this hero
	FT66Style::DeferRebuild(this);
	return FReply::Handled();
}

FText UT66HeroSelectionScreen::BuildHeroSummaryStatsText(const FHeroData& HeroData, const FT66HeroStatBlock& BaseStats, const FT66HeroStatBonuses& PermanentBuffBonuses) const
{
	(void)HeroData;
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText NewLine = NSLOCTEXT("T66.Common", "NewLine", "\n");
	const FText StatLineFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");
	const FText StatLineWithBonusFmt = NSLOCTEXT("T66.Stats", "StatLineWithBonus", "{0}: {1} + {2}");

	TArray<FText> Lines;
	Lines.Reserve(7);

	auto AddStatLine = [&](const FText& Label, int32 BaseValue, int32 BonusValue)
	{
		const FText ValueText = (BonusValue > 0)
			? FText::Format(StatLineWithBonusFmt, Label, FText::AsNumber(BaseValue), FText::AsNumber(BonusValue))
			: FText::Format(StatLineFmt, Label, FText::AsNumber(BaseValue));
		Lines.Add(ValueText);
	};

	AddStatLine(Loc ? Loc->GetText_Stat_Damage() : NSLOCTEXT("T66.Stats", "Damage", "Damage"), BaseStats.Damage, PermanentBuffBonuses.Damage);
	AddStatLine(Loc ? Loc->GetText_Stat_AttackSpeed() : NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed"), BaseStats.AttackSpeed, PermanentBuffBonuses.AttackSpeed);
	AddStatLine(Loc ? Loc->GetText_Stat_AttackScale() : NSLOCTEXT("T66.Stats", "AttackScale", "Attack Scale"), BaseStats.AttackScale, PermanentBuffBonuses.AttackScale);
	AddStatLine(Loc ? Loc->GetText_Stat_Accuracy() : NSLOCTEXT("T66.Stats", "Accuracy", "Accuracy"), BaseStats.Accuracy, PermanentBuffBonuses.Accuracy);
	AddStatLine(Loc ? Loc->GetText_Stat_Armor() : NSLOCTEXT("T66.Stats", "Armor", "Armor"), BaseStats.Armor, PermanentBuffBonuses.Armor);
	AddStatLine(Loc ? Loc->GetText_Stat_Evasion() : NSLOCTEXT("T66.Stats", "Evasion", "Evasion"), BaseStats.Evasion, PermanentBuffBonuses.Evasion);
	AddStatLine(Loc ? Loc->GetText_Stat_Luck() : NSLOCTEXT("T66.Stats", "Luck", "Luck"), BaseStats.Luck, PermanentBuffBonuses.Luck);

	return FText::Join(NewLine, Lines);
}

FText UT66HeroSelectionScreen::BuildHeroFullStatsText(const FHeroData& HeroData, const FT66HeroStatBlock& BaseStats, const FT66HeroPerLevelStatGains& PerLevelGains, const FT66HeroStatBonuses& PermanentBuffBonuses) const
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText NewLine = NSLOCTEXT("T66.Common", "NewLine", "\n");
	const FText StatLineFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");
	const FText StatLineWithBonusFmt = NSLOCTEXT("T66.Stats", "StatLineWithBonus", "{0}: {1} + {2}");
	const FText RangeFmt = NSLOCTEXT("T66.Stats", "StatRangeFormat", "{0}: {1}-{2}");

	TArray<FText> Lines;
	Lines.Reserve(48);

	const FText Quote = Loc ? Loc->GetText_HeroQuote(HeroData.HeroID) : FText::GetEmpty();
	const FText Description = Loc ? Loc->GetText_HeroDescription(HeroData.HeroID) : HeroData.Description;
	if (!Quote.IsEmpty())
	{
		Lines.Add(Quote);
	}
	if (!Description.IsEmpty())
	{
		if (!Lines.IsEmpty())
		{
			Lines.Add(FText::GetEmpty());
		}
		Lines.Add(Description);
	}

	auto AddHeader = [&](const FText& Header)
	{
		if (!Lines.IsEmpty())
		{
			Lines.Add(FText::GetEmpty());
		}
		Lines.Add(Header);
	};

	auto AddSimpleLine = [&](const FText& Label, const FText& Value)
	{
		Lines.Add(FText::Format(StatLineFmt, Label, Value));
	};

	auto AddStatLine = [&](const FText& Label, int32 BaseValue, int32 BonusValue)
	{
		const FText ValueText = (BonusValue > 0)
			? FText::Format(StatLineWithBonusFmt, Label, FText::AsNumber(BaseValue), FText::AsNumber(BonusValue))
			: FText::Format(StatLineFmt, Label, FText::AsNumber(BaseValue));
		Lines.Add(ValueText);
	};

	auto AddRangeLine = [&](const FText& Label, const FT66HeroStatGainRange& Range)
	{
		Lines.Add(FText::Format(RangeFmt, Label, FText::AsNumber(Range.Min), FText::AsNumber(Range.Max)));
	};

	AddHeader(NSLOCTEXT("T66.HeroSelection", "FullStatsOverview", "OVERVIEW"));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "PrimaryCategoryLabel", "Primary Category"), Loc ? Loc->GetText_IdolCategoryName(HeroData.PrimaryCategory) : StaticEnum<ET66AttackCategory>()->GetDisplayNameTextByValue(static_cast<int64>(HeroData.PrimaryCategory)));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "PassiveLabel", "Passive"), Loc ? Loc->GetText_PassiveName(HeroData.PassiveType) : StaticEnum<ET66PassiveType>()->GetDisplayNameTextByValue(static_cast<int64>(HeroData.PassiveType)));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "UltimateLabel", "Ultimate"), Loc ? Loc->GetText_UltimateName(HeroData.UltimateType) : StaticEnum<ET66UltimateType>()->GetDisplayNameTextByValue(static_cast<int64>(HeroData.UltimateType)));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "MaxSpeedLabel", "Max Speed"), FText::AsNumber(FMath::RoundToInt(HeroData.MaxSpeed)));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "AccelerationLabel", "Acceleration"), FText::Format(NSLOCTEXT("T66.HeroSelection", "AccelerationValue", "{0}% / sec"), FText::AsNumber(FMath::RoundToInt(HeroData.AccelerationPercentPerSecond))));

	AddHeader(NSLOCTEXT("T66.HeroSelection", "FullStatsBaseHeader", "BASE STATS"));
	AddStatLine(Loc ? Loc->GetText_Stat_Damage() : NSLOCTEXT("T66.Stats", "Damage", "Damage"), BaseStats.Damage, PermanentBuffBonuses.Damage);
	AddStatLine(Loc ? Loc->GetText_Stat_AttackSpeed() : NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed"), BaseStats.AttackSpeed, PermanentBuffBonuses.AttackSpeed);
	AddStatLine(Loc ? Loc->GetText_Stat_AttackScale() : NSLOCTEXT("T66.Stats", "AttackScale", "Attack Scale"), BaseStats.AttackScale, PermanentBuffBonuses.AttackScale);
	AddStatLine(Loc ? Loc->GetText_Stat_Accuracy() : NSLOCTEXT("T66.Stats", "Accuracy", "Accuracy"), BaseStats.Accuracy, PermanentBuffBonuses.Accuracy);
	AddStatLine(Loc ? Loc->GetText_Stat_Armor() : NSLOCTEXT("T66.Stats", "Armor", "Armor"), BaseStats.Armor, PermanentBuffBonuses.Armor);
	AddStatLine(Loc ? Loc->GetText_Stat_Evasion() : NSLOCTEXT("T66.Stats", "Evasion", "Evasion"), BaseStats.Evasion, PermanentBuffBonuses.Evasion);
	AddStatLine(Loc ? Loc->GetText_Stat_Luck() : NSLOCTEXT("T66.Stats", "Luck", "Luck"), BaseStats.Luck, PermanentBuffBonuses.Luck);
	AddSimpleLine(Loc ? Loc->GetText_Stat_Speed() : NSLOCTEXT("T66.Stats", "Speed", "Speed"), FText::AsNumber(BaseStats.Speed));

	AddHeader(NSLOCTEXT("T66.HeroSelection", "FullStatsPerLevelHeader", "PER LEVEL"));
	AddRangeLine(Loc ? Loc->GetText_Stat_Damage() : NSLOCTEXT("T66.Stats", "Damage", "Damage"), PerLevelGains.Damage);
	AddRangeLine(Loc ? Loc->GetText_Stat_AttackSpeed() : NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed"), PerLevelGains.AttackSpeed);
	AddRangeLine(Loc ? Loc->GetText_Stat_AttackScale() : NSLOCTEXT("T66.Stats", "AttackScale", "Attack Scale"), PerLevelGains.AttackScale);
	AddRangeLine(Loc ? Loc->GetText_Stat_Accuracy() : NSLOCTEXT("T66.Stats", "Accuracy", "Accuracy"), PerLevelGains.Accuracy);
	AddRangeLine(Loc ? Loc->GetText_Stat_Armor() : NSLOCTEXT("T66.Stats", "Armor", "Armor"), PerLevelGains.Armor);
	AddRangeLine(Loc ? Loc->GetText_Stat_Evasion() : NSLOCTEXT("T66.Stats", "Evasion", "Evasion"), PerLevelGains.Evasion);
	AddRangeLine(Loc ? Loc->GetText_Stat_Luck() : NSLOCTEXT("T66.Stats", "Luck", "Luck"), PerLevelGains.Luck);
	AddSimpleLine(Loc ? Loc->GetText_Stat_Speed() : NSLOCTEXT("T66.Stats", "Speed", "Speed"), NSLOCTEXT("T66.HeroSelection", "SpeedPerLevel", "+1 per level"));

	AddHeader(NSLOCTEXT("T66.HeroSelection", "FullStatsCategoryHeader", "CATEGORY STATS"));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "PierceDamageLabel", "Pierce Damage"), FText::AsNumber(HeroData.BasePierceDmg));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "PierceAttackSpeedLabel", "Pierce Attack Speed"), FText::AsNumber(HeroData.BasePierceAtkSpd));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "PierceAttackScaleLabel", "Pierce Attack Scale"), FText::AsNumber(HeroData.BasePierceAtkScale));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "BounceDamageLabel", "Bounce Damage"), FText::AsNumber(HeroData.BaseBounceDmg));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "BounceAttackSpeedLabel", "Bounce Attack Speed"), FText::AsNumber(HeroData.BaseBounceAtkSpd));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "BounceAttackScaleLabel", "Bounce Attack Scale"), FText::AsNumber(HeroData.BaseBounceAtkScale));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "AoeDamageLabel", "AOE Damage"), FText::AsNumber(HeroData.BaseAoeDmg));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "AoeAttackSpeedLabel", "AOE Attack Speed"), FText::AsNumber(HeroData.BaseAoeAtkSpd));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "AoeAttackScaleLabel", "AOE Attack Scale"), FText::AsNumber(HeroData.BaseAoeAtkScale));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "DotDamageLabel", "DOT Damage"), FText::AsNumber(HeroData.BaseDotDmg));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "DotAttackSpeedLabel", "DOT Attack Speed"), FText::AsNumber(HeroData.BaseDotAtkSpd));
	AddSimpleLine(NSLOCTEXT("T66.HeroSelection", "DotAttackScaleLabel", "DOT Attack Scale"), FText::AsNumber(HeroData.BaseDotAtkScale));

	AddHeader(NSLOCTEXT("T66.HeroSelection", "FullStatsPassiveHeader", "PASSIVE"));
	Lines.Add(Loc ? Loc->GetText_PassiveName(HeroData.PassiveType) : StaticEnum<ET66PassiveType>()->GetDisplayNameTextByValue(static_cast<int64>(HeroData.PassiveType)));
	const FText PassiveDescription = Loc ? Loc->GetText_PassiveDescription(HeroData.PassiveType) : FText::GetEmpty();
	if (!PassiveDescription.IsEmpty())
	{
		Lines.Add(PassiveDescription);
	}

	AddHeader(NSLOCTEXT("T66.HeroSelection", "FullStatsUltimateHeader", "ULTIMATE"));
	Lines.Add(Loc ? Loc->GetText_UltimateName(HeroData.UltimateType) : StaticEnum<ET66UltimateType>()->GetDisplayNameTextByValue(static_cast<int64>(HeroData.UltimateType)));
	const FText UltimateDescription = Loc ? Loc->GetText_UltimateDescription(HeroData.UltimateType) : FText::GetEmpty();
	if (!UltimateDescription.IsEmpty())
	{
		Lines.Add(UltimateDescription);
	}

	return FText::Join(NewLine, Lines);
}

void UT66HeroSelectionScreen::UpdateHeroDisplay()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;

	auto SetMedalBrushForTier = [this](const ET66AccountMedalTier MedalTier)
	{
		const FString MedalPath = GetHeroSelectionMedalImagePath(MedalTier);
		if (IFileManager::Get().FileExists(*MedalPath))
		{
			HeroRecordMedalBrush = MakeShared<FSlateImageBrush>(*MedalPath, FVector2D(26.f, 26.f));
		}
		else
		{
			HeroRecordMedalBrush.Reset();
		}
	};

	auto SetRecordValues = [this, &SetMedalBrushForTier](const ET66AccountMedalTier MedalTier, const int32 GamesPlayed, const FText& ThirdLabel, const FText& ThirdValue)
	{
		SetMedalBrushForTier(MedalTier);

		if (HeroRecordMedalWidget.IsValid())
		{
			HeroRecordMedalWidget->SetText(HeroRecordMedalText(MedalTier));
			HeroRecordMedalWidget->SetColorAndOpacity(HeroRecordMedalColor(MedalTier));
		}
		if (HeroRecordGamesWidget.IsValid())
		{
			HeroRecordGamesWidget->SetText(FText::AsNumber(GamesPlayed));
		}
		if (HeroRecordThirdLabelWidget.IsValid())
		{
			HeroRecordThirdLabelWidget->SetText(ThirdLabel);
		}
		if (HeroRecordThirdValueWidget.IsValid())
		{
			HeroRecordThirdValueWidget->SetText(ThirdValue);
		}
	};

	auto UpdateTargetOptions = [this, Loc, T66GI]()
	{
		const FText NoCompanionText = Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.HeroSelection", "NoCompanionOption", "No Companion");
		FText CurrentHeroDisplayName = NSLOCTEXT("T66.HeroSelection", "HeroTargetFallback", "Hero");
		FText CurrentCompanionDisplayName = NoCompanionText;

		FHeroData HeroTargetData;
		if (GetPreviewedHeroData(HeroTargetData))
		{
			CurrentHeroDisplayName = Loc ? Loc->GetHeroDisplayName(HeroTargetData) : HeroTargetData.DisplayName;
		}
		else if (T66GI)
		{
			FHeroData SelectedHeroData;
			if (T66GI->GetSelectedHeroData(SelectedHeroData))
			{
				CurrentHeroDisplayName = Loc ? Loc->GetHeroDisplayName(SelectedHeroData) : SelectedHeroData.DisplayName;
			}
		}

		FCompanionData CompanionTargetData;
		if (GetPreviewedCompanionData(CompanionTargetData))
		{
			CurrentCompanionDisplayName = Loc ? Loc->GetCompanionDisplayName(CompanionTargetData) : CompanionTargetData.DisplayName;
		}

		if (SkinTargetOptions.Num() >= 2)
		{
			if (SkinTargetOptions[0].IsValid())
			{
				*SkinTargetOptions[0] = CurrentHeroDisplayName.ToString();
			}
			if (SkinTargetOptions[1].IsValid())
			{
				*SkinTargetOptions[1] = CurrentCompanionDisplayName.ToString();
			}
			CurrentSkinTargetOption = SkinTargetOptions[bShowingCompanionSkins ? 1 : 0];
		}

		if (InfoTargetOptions.Num() >= 2)
		{
			if (InfoTargetOptions[0].IsValid())
			{
				*InfoTargetOptions[0] = CurrentHeroDisplayName.ToString();
			}
			if (InfoTargetOptions[1].IsValid())
			{
				*InfoTargetOptions[1] = CurrentCompanionDisplayName.ToString();
			}
			CurrentInfoTargetOption = InfoTargetOptions[bShowingCompanionInfo ? 1 : 0];
		}
	};

	UpdateTargetOptions();

	FHeroData HeroData;
	if (GetPreviewedHeroData(HeroData))
	{
		if (HeroLoreWidget.IsValid())
		{
			const FText Desc = Loc ? Loc->GetText_HeroDescription(PreviewedHeroID) : HeroData.Description;
			HeroLoreWidget->SetText(Desc);
		}

		FT66HeroStatBlock BaseStats;
		FT66HeroPerLevelStatGains PerLevelGains;
		if (T66GI)
		{
			T66GI->GetHeroStatTuning(PreviewedHeroID, BaseStats, PerLevelGains);
		}

		FT66HeroStatBonuses PermanentBuffBonuses;
		if (UT66BuffSubsystem* Buffs = GI ? GI->GetSubsystem<UT66BuffSubsystem>() : nullptr)
		{
			PermanentBuffBonuses = Buffs->GetPermanentBuffStatBonuses();
		}

		if (HeroDescWidget.IsValid())
		{
			HeroDescWidget->SetText(BuildHeroSummaryStatsText(HeroData, BaseStats, PermanentBuffBonuses));
		}
		if (HeroFullStatsWidget.IsValid())
		{
			HeroFullStatsWidget->SetText(BuildHeroFullStatsText(HeroData, BaseStats, PerLevelGains, PermanentBuffBonuses));
		}

		if (bShowingCompanionInfo)
		{
			FCompanionData CompanionData;
			const bool bHasCompanion = GetPreviewedCompanionData(CompanionData);
			const FText CompanionLoreText = bHasCompanion
				? (Loc ? Loc->GetText_CompanionLore(PreviewedCompanionID) : CompanionData.LoreText)
				: (Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.HeroSelection", "NoCompanionInfo", "No companion selected."));
			if (HeroDescWidget.IsValid())
			{
				HeroDescWidget->SetText(CompanionLoreText);
			}
			const ET66AccountMedalTier MedalTier = (Achievements && bHasCompanion)
				? Achievements->GetCompanionHighestMedal(PreviewedCompanionID)
				: ET66AccountMedalTier::None;
			const int32 GamesPlayed = (Achievements && bHasCompanion)
				? Achievements->GetCompanionGamesPlayed(PreviewedCompanionID)
				: 0;
			const int32 UnionStages = (Achievements && bHasCompanion)
				? Achievements->GetCompanionUnionStagesCleared(PreviewedCompanionID)
				: 0;
			SetRecordValues(
				MedalTier,
				GamesPlayed,
				HeroRecordThirdMetricLabel(true),
				FText::AsNumber(UnionStages));
		}
		else
		{
			if (HeroDescWidget.IsValid())
			{
				HeroDescWidget->SetText(BuildHeroSummaryStatsText(HeroData, BaseStats, PermanentBuffBonuses));
			}
			const ET66AccountMedalTier MedalTier = Achievements ? Achievements->GetHeroHighestMedal(PreviewedHeroID) : ET66AccountMedalTier::None;
			const int32 GamesPlayed = Achievements ? Achievements->GetHeroGamesPlayed(PreviewedHeroID) : 0;
			const int32 CumulativeScore = Achievements ? Achievements->GetHeroCumulativeScore(PreviewedHeroID) : 0;
			SetRecordValues(
				MedalTier,
				GamesPlayed,
				HeroRecordThirdMetricLabel(false),
				FText::AsNumber(CumulativeScore));
		}

		// Update 3D preview stage or fallback colored box (use preview skin override or selected skin)
		if (AT66HeroPreviewStage* PreviewStage = GetHeroPreviewStage())
		{
			FName EffectiveSkinID = PreviewSkinIDOverride.IsNone()
				? (T66GI && !T66GI->SelectedHeroSkinID.IsNone() ? T66GI->SelectedHeroSkinID : FName(TEXT("Default")))
				: PreviewSkinIDOverride;
			if (EffectiveSkinID.IsNone()) EffectiveSkinID = FName(TEXT("Default"));
			const FName EffectiveCompanionSkinID = GetEffectivePreviewedCompanionSkinID();
			PreviewStage->SetPreviewDifficulty(SelectedDifficulty);
			UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] UpdateHeroDisplay: PreviewSkinIDOverride=%s, GI->SelectedHeroSkinID=%s, EffectiveSkinID=%s"),
				*PreviewSkinIDOverride.ToString(), T66GI ? *T66GI->SelectedHeroSkinID.ToString() : TEXT("(null GI)"), *EffectiveSkinID.ToString());
			UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] UpdateHeroDisplay: calling SetPreviewHero HeroID=%s BodyType=%d SkinID=%s"),
				*PreviewedHeroID.ToString(), (int32)SelectedBodyType, *EffectiveSkinID.ToString());
			PreviewStage->SetPreviewHero(PreviewedHeroID, SelectedBodyType, EffectiveSkinID, PreviewedCompanionID, EffectiveCompanionSkinID);
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
		if (HeroFullStatsWidget.IsValid())
		{
			HeroFullStatsWidget->SetText(FText::GetEmpty());
		}
		if (HeroDescWidget.IsValid())
		{
			HeroDescWidget->SetText(FText::GetEmpty());
		}
		SetRecordValues(
			ET66AccountMedalTier::None,
			0,
			HeroRecordThirdMetricLabel(bShowingCompanionInfo),
			NSLOCTEXT("T66.HeroSelection", "HeroRecordThirdEmpty", "0"));
		if (AT66HeroPreviewStage* PreviewStage = GetHeroPreviewStage())
		{
			FName EffectiveSkinID = PreviewSkinIDOverride.IsNone()
				? (T66GI && !T66GI->SelectedHeroSkinID.IsNone() ? T66GI->SelectedHeroSkinID : FName(TEXT("Default")))
				: PreviewSkinIDOverride;
			if (EffectiveSkinID.IsNone()) EffectiveSkinID = FName(TEXT("Default"));
			const FName EffectiveCompanionSkinID = GetEffectivePreviewedCompanionSkinID();
			PreviewStage->SetPreviewHero(PreviewedHeroID, SelectedBodyType, EffectiveSkinID, PreviewedCompanionID, EffectiveCompanionSkinID);
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
	RefreshCompanionCarouselPortraits();
}

void UT66HeroSelectionScreen::RefreshHeroCarouselPortraits()
{
	if (AllHeroIDs.Num() <= 0)
	{
		HeroCarouselSlotColors.Init(FLinearColor(0.2f, 0.2f, 0.25f, 1.0f), HeroSelectionCarouselVisibleSlots);
		HeroCarouselSlotVisibility.Init(EVisibility::Collapsed, HeroSelectionCarouselVisibleSlots);
		for (TSharedPtr<FSlateBrush>& B : HeroCarouselPortraitBrushes)
		{
			if (B.IsValid()) B->SetResourceObject(nullptr);
		}
		return;
	}

	HeroCarouselPortraitBrushes.SetNum(HeroSelectionCarouselVisibleSlots);
	HeroCarouselSlotColors.SetNum(HeroSelectionCarouselVisibleSlots);
	HeroCarouselSlotVisibility.SetNum(HeroSelectionCarouselVisibleSlots);
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
		for (int32 Offset = -HeroSelectionCarouselCenterIndex; Offset <= HeroSelectionCarouselCenterIndex; ++Offset)
		{
			const int32 SlotIdx = Offset + HeroSelectionCarouselCenterIndex;
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

			const float BoxSize = GetHeroSelectionCarouselBoxSize(Offset);
			HeroCarouselSlotColors[SlotIdx] = SlotColor * GetHeroSelectionCarouselOpacity(Offset);
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

void UT66HeroSelectionScreen::RefreshCompanionCarouselPortraits()
{
	TArray<FName> CompanionWheelIDs;
	CompanionWheelIDs.Add(NAME_None);
	CompanionWheelIDs.Append(AllCompanionIDs);

	CompanionCarouselPortraitBrushes.SetNum(HeroSelectionCarouselVisibleSlots);
	CompanionCarouselSlotColors.SetNum(HeroSelectionCarouselVisibleSlots);
	CompanionCarouselSlotVisibility.SetNum(HeroSelectionCarouselVisibleSlots);
	CompanionCarouselSlotLabels.SetNum(HeroSelectionCarouselVisibleSlots);
	for (int32 Index = 0; Index < CompanionCarouselPortraitBrushes.Num(); ++Index)
	{
		if (!CompanionCarouselPortraitBrushes[Index].IsValid())
		{
			CompanionCarouselPortraitBrushes[Index] = MakeShared<FSlateBrush>();
			CompanionCarouselPortraitBrushes[Index]->DrawAs = ESlateBrushDrawType::Image;
			CompanionCarouselPortraitBrushes[Index]->ImageSize = FVector2D(60.f, 60.f);
		}
	}

	if (CompanionWheelIDs.Num() == 0)
	{
		for (int32 Index = 0; Index < HeroSelectionCarouselVisibleSlots; ++Index)
		{
			CompanionCarouselSlotColors[Index] = FLinearColor(0.2f, 0.2f, 0.25f, 1.0f);
			CompanionCarouselSlotVisibility[Index] = EVisibility::Collapsed;
			CompanionCarouselSlotLabels[Index] = FText::GetEmpty();
			if (CompanionCarouselPortraitBrushes[Index].IsValid())
			{
				CompanionCarouselPortraitBrushes[Index]->SetResourceObject(nullptr);
			}
		}
		return;
	}

	const int32 ClampedCenterIndex = FMath::Clamp(CurrentCompanionIndex, 0, CompanionWheelIDs.Num() - 1);
	const FText NoneLabel = NSLOCTEXT("T66.HeroSelection", "CompanionNoneShort", "NONE");

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		UT66UITexturePoolSubsystem* TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
		for (int32 Offset = -HeroSelectionCarouselCenterIndex; Offset <= HeroSelectionCarouselCenterIndex; ++Offset)
		{
			const int32 SlotIdx = Offset + HeroSelectionCarouselCenterIndex;
			const int32 CompanionIdx = (ClampedCenterIndex + Offset + CompanionWheelIDs.Num() * 2) % CompanionWheelIDs.Num();
			const FName CompanionID = CompanionWheelIDs[CompanionIdx];
			const float BoxSize = GetHeroSelectionCarouselBoxSize(Offset);
			FLinearColor SlotColor(0.18f, 0.18f, 0.22f, 1.0f);
			TSoftObjectPtr<UTexture2D> PortraitSoft;
			FText SlotLabel = FText::GetEmpty();

			if (CompanionID.IsNone())
			{
				SlotColor = FLinearColor(0.10f, 0.10f, 0.12f, 1.0f);
				SlotLabel = NoneLabel;
			}
			else
			{
				FCompanionData Data;
				if (GI->GetCompanionData(CompanionID, Data))
				{
					SlotColor = Data.PlaceholderColor;
					PortraitSoft = !Data.SelectionPortrait.IsNull() ? Data.SelectionPortrait : Data.Portrait;
				}
			}

			CompanionCarouselSlotColors[SlotIdx] = SlotColor * GetHeroSelectionCarouselOpacity(Offset);
			CompanionCarouselSlotVisibility[SlotIdx] = EVisibility::Visible;
			CompanionCarouselSlotLabels[SlotIdx] = SlotLabel;

			if (!CompanionCarouselPortraitBrushes[SlotIdx].IsValid())
			{
				continue;
			}

			if (PortraitSoft.IsNull() || !TexPool)
			{
				CompanionCarouselPortraitBrushes[SlotIdx]->SetResourceObject(nullptr);
			}
			else
			{
				T66SlateTexture::BindSharedBrushAsync(
					TexPool,
					PortraitSoft,
					this,
					CompanionCarouselPortraitBrushes[SlotIdx],
					FName(TEXT("HeroSelectionCompanionCarousel"), SlotIdx + 1),
					/*bClearWhileLoading*/ true);
			}
			CompanionCarouselPortraitBrushes[SlotIdx]->ImageSize = FVector2D(BoxSize, BoxSize);
		}
	}
}

void UT66HeroSelectionScreen::OnScreenDeactivated_Implementation()
{
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66HeroSelectionScreen::OnLanguageChanged);
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->OnPartyStateChanged().Remove(PartyStateChangedHandle);
			PartyStateChangedHandle.Reset();
		}

		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->OnSessionStateChanged().Remove(SessionStateChangedHandle);
			SessionStateChangedHandle.Reset();
		}
	}

	Super::OnScreenDeactivated_Implementation();
	if (HeroPreviewMediaPlayer)
	{
		HeroPreviewMediaPlayer->Close();
	}
}

void UT66HeroSelectionScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	// Rebuild Slate so the current standalone/frontend flow is reflected correctly.
	FT66Style::DeferRebuild(this);
	PreviewSkinIDOverride = NAME_None;
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66HeroSelectionScreen::OnLanguageChanged);
	}
	RefreshHeroList();
	bShowingStatsPanel = false;

	if (UT66GameInstance* GIPreload = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		// Keep startup light, then finish warming the full preview library after the screen is already visible.
		GIPreload->PrimeHeroSelectionAssetsAsync();
		GIPreload->PrimeHeroSelectionPreviewVisualsAsync();
	}
	RequestKnightPreviewMediaSource();

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>();
		RefreshCompanionList();
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartyStateChangedHandle = PartySubsystem->OnPartyStateChanged().AddUObject(this, &UT66HeroSelectionScreen::HandlePartyStateChanged);
		}

		if (SessionSubsystem)
		{
			SessionStateChangedHandle = SessionSubsystem->OnSessionStateChanged().AddUObject(this, &UT66HeroSelectionScreen::HandleSessionStateChanged);
			SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::HeroSelection);
		}

		SelectedDifficulty = SessionSubsystem ? SessionSubsystem->GetSharedLobbyDifficulty() : GI->SelectedDifficulty;
		SelectedBodyType = GI->SelectedHeroBodyType;
		PreviewedCompanionID = GI->SelectedCompanionID;
		TArray<FName> CompanionWheelIDs;
		CompanionWheelIDs.Add(NAME_None);
		CompanionWheelIDs.Append(AllCompanionIDs);
		CurrentCompanionIndex = CompanionWheelIDs.IndexOfByKey(PreviewedCompanionID);
		if (CurrentCompanionIndex == INDEX_NONE)
		{
			CurrentCompanionIndex = 0;
		}
		bShowingCompanionSkins = false;
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

	CommitLocalSelectionsToLobby(false);
	SyncToSharedPartyScreen();
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

void UT66HeroSelectionScreen::RefreshCompanionList()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		AllCompanionIDs = GI->GetAllCompanionIDs();
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

bool UT66HeroSelectionScreen::GetPreviewedCompanionData(FCompanionData& OutCompanionData)
{
	if (PreviewedCompanionID.IsNone())
	{
		return false;
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->GetCompanionData(PreviewedCompanionID, OutCompanionData);
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
	CommitLocalSelectionsToLobby(true);
	UpdateHeroDisplay();
	UpdateHeroPreviewVideo();
	UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero END"));
}

void UT66HeroSelectionScreen::PreviewCompanion(FName CompanionID)
{
	PreviewedCompanionID = CompanionID;
	PreviewedCompanionSkinIDOverride = NAME_None;

	TArray<FName> CompanionWheelIDs;
	CompanionWheelIDs.Add(NAME_None);
	CompanionWheelIDs.Append(AllCompanionIDs);
	CurrentCompanionIndex = CompanionWheelIDs.IndexOfByKey(CompanionID);
	if (CurrentCompanionIndex == INDEX_NONE)
	{
		CurrentCompanionIndex = 0;
	}

	CommitLocalSelectionsToLobby(true);
	RefreshSkinsList();
	UpdateHeroDisplay();
}

void UT66HeroSelectionScreen::SelectNoCompanion()
{
	PreviewCompanion(NAME_None);
}

void UT66HeroSelectionScreen::PreviewNextHero()
{
	if (AllHeroIDs.Num() == 0) return;
	CurrentHeroIndex = (CurrentHeroIndex + 1) % AllHeroIDs.Num();
	PreviewHero(AllHeroIDs[CurrentHeroIndex]);
}

void UT66HeroSelectionScreen::PreviewNextCompanion()
{
	TArray<FName> CompanionWheelIDs;
	CompanionWheelIDs.Add(NAME_None);
	CompanionWheelIDs.Append(AllCompanionIDs);
	if (CompanionWheelIDs.Num() == 0)
	{
		return;
	}

	CurrentCompanionIndex = (CurrentCompanionIndex + 1 + CompanionWheelIDs.Num()) % CompanionWheelIDs.Num();
	PreviewCompanion(CompanionWheelIDs[CurrentCompanionIndex]);
}

void UT66HeroSelectionScreen::PreviewPreviousHero()
{
	if (AllHeroIDs.Num() == 0) return;
	CurrentHeroIndex = (CurrentHeroIndex - 1 + AllHeroIDs.Num()) % AllHeroIDs.Num();
	PreviewHero(AllHeroIDs[CurrentHeroIndex]);
}

void UT66HeroSelectionScreen::PreviewPreviousCompanion()
{
	TArray<FName> CompanionWheelIDs;
	CompanionWheelIDs.Add(NAME_None);
	CompanionWheelIDs.Append(AllCompanionIDs);
	if (CompanionWheelIDs.Num() == 0)
	{
		return;
	}

	CurrentCompanionIndex = (CurrentCompanionIndex - 1 + CompanionWheelIDs.Num()) % CompanionWheelIDs.Num();
	PreviewCompanion(CompanionWheelIDs[CurrentCompanionIndex]);
}

void UT66HeroSelectionScreen::SelectDifficulty(ET66Difficulty Difficulty) { SelectedDifficulty = Difficulty; }
void UT66HeroSelectionScreen::ToggleBodyType() { SelectedBodyType = (SelectedBodyType == ET66BodyType::TypeA) ? ET66BodyType::TypeB : ET66BodyType::TypeA; }
void UT66HeroSelectionScreen::OnHeroGridClicked() { ShowModal(ET66ScreenType::HeroGrid); }
void UT66HeroSelectionScreen::OnCompanionGridClicked() { ShowModal(ET66ScreenType::CompanionGrid); }
void UT66HeroSelectionScreen::OnChooseCompanionClicked()
{
	OnCompanionGridClicked();
}
void UT66HeroSelectionScreen::OnHeroLoreClicked() { ShowModal(ET66ScreenType::HeroLore); }
void UT66HeroSelectionScreen::OnEnterTribulationClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (GI)
	{
		GI->SelectedHeroID = PreviewedHeroID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->SelectedHeroBodyType = SelectedBodyType;
		GI->ApplyConfiguredMainMapLayoutVariant();
		GI->bStageCatchUpPending = false;
		GI->PendingLoadedTransform = FTransform();
		GI->bApplyLoadedTransform = false;
		// New seed each time so procedural terrain layout differs per run
		GI->RunSeed = FMath::Rand();
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->ApplyCurrentPartyToGameInstanceRunContext();
		}
	}

	if (SessionSubsystem && SessionSubsystem->IsPartySessionActive() && GI)
	{
		GI->SelectedDifficulty = SessionSubsystem->GetSharedLobbyDifficulty();

		if (!SessionSubsystem->IsLocalPlayerPartyHost() && GI->GetSubsystem<UT66PartySubsystem>() && GI->GetSubsystem<UT66PartySubsystem>()->HasRemotePartyMembers())
		{
			SessionSubsystem->SetLocalLobbyReady(!SessionSubsystem->IsLocalLobbyReady());
			ForceRebuildSlate();
			return;
		}

		FString FailureReason;
		if (!SessionSubsystem->AreAllPartyMembersReadyForGameplay(&FailureReason))
		{
			UE_LOG(LogT66HeroSelection, Log, TEXT("%s"), *FailureReason);
			ForceRebuildSlate();
			return;
		}

		if (UIManager) UIManager->HideAllUI();
		SessionSubsystem->StartGameplayTravel();
		return;
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
void UT66HeroSelectionScreen::OnBackClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (SessionSubsystem->IsPartySessionActive())
			{
				SessionSubsystem->SetLocalLobbyReady(false);
				if (SessionSubsystem->IsLocalPlayerPartyHost())
				{
					SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::MainMenu, true);
					NavigateTo(ET66ScreenType::MainMenu);
				}
				return;
			}
		}
	}

	NavigateBack();
}

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
	const FName ArthurHeroID(TEXT("Hero_1"));
	const bool bIsArthur = (PreviewedHeroID == ArthurHeroID);
	const FString ArthurPreviewMoviePath = ResolveArthurPreviewMoviePath();
	const bool bHasArthurPreview = !ArthurPreviewMoviePath.IsEmpty();

	if (HeroPreviewVideoImage.IsValid())
	{
		HeroPreviewVideoImage->SetVisibility(bIsArthur && bHasArthurPreview ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (HeroPreviewPlaceholderText.IsValid())
	{
		HeroPreviewPlaceholderText->SetVisibility(bIsArthur && bHasArthurPreview ? EVisibility::Collapsed : EVisibility::Visible);
	}

	if (!HeroPreviewMediaPlayer)
	{
		return;
	}
	if (bIsArthur)
	{
		if (!bHasArthurPreview)
		{
			HeroPreviewMediaPlayer->Close();
			return;
		}
		const FString CurrentUrl = HeroPreviewMediaPlayer->GetUrl();
		if (!HeroPreviewMediaPlayer->IsPlaying() || !CurrentUrl.Contains(TEXT("ArthurPreview.mp4")))
		{
			HeroPreviewMediaPlayer->Close();
			if (!HeroPreviewMediaPlayer->OpenFile(ArthurPreviewMoviePath))
			{
				UE_LOG(LogT66HeroSelection, Warning, TEXT("Failed to open Arthur preview video from '%s'"), *ArthurPreviewMoviePath);
			}
		}
	}
	else
	{
		HeroPreviewMediaPlayer->Close();
	}
}

void UT66HeroSelectionScreen::RequestKnightPreviewMediaSource()
{
	if (KnightPreviewMediaSource)
	{
		return;
	}

	const FString MoviePath = ResolveArthurPreviewMoviePath();
	if (MoviePath.IsEmpty())
	{
		return;
	}

	KnightPreviewMediaSource = NewObject<UFileMediaSource>(this, NAME_None, RF_Transient);
	if (KnightPreviewMediaSource)
	{
		KnightPreviewMediaSource->SetFilePath(MoviePath);
		HandleKnightPreviewMediaSourceLoaded();
	}
}

void UT66HeroSelectionScreen::HandleKnightPreviewMediaSourceLoaded()
{
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

