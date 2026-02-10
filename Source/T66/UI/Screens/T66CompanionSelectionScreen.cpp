// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66CompanionSelectionScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66SkinSubsystem.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "Gameplay/T66FrontendGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Notifications/SProgressBar.h"
// Render target removed — in-world preview uses main viewport camera with full Lumen GI.

namespace
{
	class ST66DragRotateCompanionPreview : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66DragRotateCompanionPreview) {}
			SLATE_ARGUMENT(TWeakObjectPtr<AT66CompanionPreviewStage>, Stage)
			SLATE_ARGUMENT(float, DegreesPerPixel)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			Stage = InArgs._Stage;
			DegreesPerPixel = InArgs._DegreesPerPixel;
			bDragging = false;
			// No child content — transparent overlay for drag-rotate/zoom.
			// The viewport renders the 3D companion directly behind this widget.
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
				// Rotate the companion without orbiting the camera (no vertical pitch).
				Stage->AddPreviewYaw(Delta.X * DegreesPerPixel);
			}
			return FReply::Handled();
		}

		virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (Stage.IsValid())
			{
				Stage->AddPreviewZoom(MouseEvent.GetWheelDelta());
				return FReply::Handled();
			}
			return FReply::Unhandled();
		}

	private:
		TWeakObjectPtr<AT66CompanionPreviewStage> Stage;
		bool bDragging = false;
		FVector2D LastPos = FVector2D::ZeroVector;
		float DegreesPerPixel = 0.25f;
	};
}

UT66CompanionSelectionScreen::UT66CompanionSelectionScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::CompanionSelection;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66CompanionSelectionScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

bool UT66CompanionSelectionScreen::IsCompanionUnlocked(FName CompanionID) const
{
	if (CompanionID.IsNone())
	{
		return true;
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66CompanionUnlockSubsystem* Unlocks = GI->GetSubsystem<UT66CompanionUnlockSubsystem>())
		{
			return Unlocks->IsCompanionUnlocked(CompanionID);
		}
	}
	return true; // fail-open so we don't hard-lock the UI if subsystem is missing
}

void UT66CompanionSelectionScreen::GeneratePlaceholderSkins()
{
	PlaceholderSkins.Empty();
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66SkinSubsystem* Skin = GI ? GI->GetSubsystem<UT66SkinSubsystem>() : nullptr;
	FName CompanionForSkins = PreviewedCompanionID.IsNone() && AllCompanionIDs.Num() > 0 ? AllCompanionIDs[0] : PreviewedCompanionID;
	if (Skin && !CompanionForSkins.IsNone())
	{
		PlaceholderSkins = Skin->GetSkinsForEntity(ET66SkinEntityType::Companion, CompanionForSkins);
	}
	else
	{
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

void UT66CompanionSelectionScreen::RefreshSkinsList()
{
	GeneratePlaceholderSkins();
	if (SkinsListBoxWidget.IsValid())
	{
		SkinsListBoxWidget->ClearChildren();
		AddSkinRowsToBox(SkinsListBoxWidget);
	}
	if (ACBalanceTextBlock.IsValid())
	{
		int32 ACBalance = 0;
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66SkinSubsystem* Skin = GI->GetSubsystem<UT66SkinSubsystem>())
				ACBalance = Skin->GetAchievementCoinsBalance();
		}
		UT66LocalizationSubsystem* Loc = GetLocSubsystem();
		const FText ACBalanceText = Loc
			? FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(ACBalance))
			: FText::Format(NSLOCTEXT("T66.Achievements", "AchievementCoinsFormatFallback", "{0} AC"), FText::AsNumber(ACBalance));
		ACBalanceTextBlock->SetText(ACBalanceText);
	}
	UpdateCompanionDisplay();
}

void UT66CompanionSelectionScreen::AddSkinRowsToBox(const TSharedPtr<SVerticalBox>& Box)
{
	if (!Box.IsValid()) return;
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText EquipText = Loc ? Loc->GetText_Equip() : NSLOCTEXT("T66.Common", "Equip", "EQUIP");
	const FText EquippedText = NSLOCTEXT("T66.HeroSelection", "Equipped", "EQUIPPED");
	const FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");
	const FText BuyText = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	const FButtonStyle& PrimaryButtonStyle = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary");
	static constexpr int32 BeachgoerPriceAC = 250;
	const FText BeachgoerPriceText = Loc
		? FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(BeachgoerPriceAC))
		: FText::Format(NSLOCTEXT("T66.Achievements", "AchievementCoinsFormatFallback", "{0} AC"), FText::AsNumber(BeachgoerPriceAC));

	for (const FSkinData& Skin : PlaceholderSkins)
	{
		FName SkinIDCopy = Skin.SkinID;
		bool bIsDefault = Skin.bIsDefault;
		bool bIsOwned = Skin.bIsOwned;
		bool bIsEquipped = Skin.bIsEquipped;
		FName CID = PreviewedCompanionID.IsNone() && AllCompanionIDs.Num() > 0 ? AllCompanionIDs[0] : PreviewedCompanionID;

		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Loc ? Loc->GetText_SkinName(SkinIDCopy) : FText::FromName(SkinIDCopy))
				.Font(FT66Style::Tokens::FontRegular(16))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];

		if (bIsDefault)
		{
			Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(5.0f, 0.0f)
				[
					SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(bIsEquipped ? 1 : 0)
						+ SWidgetSwitcher::Slot()
						[
						FT66Style::MakeButton(FT66ButtonParams(EquipText,
							FOnClicked::CreateLambda([this, CID]()
							{
								if (CID.IsNone()) return FReply::Handled();
								if (UT66SkinSubsystem* SkinSub = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UT66SkinSubsystem>())
								{
									SkinSub->SetEquippedCompanionSkinID(CID, UT66SkinSubsystem::DefaultSkinID);
									PreviewedCompanionSkinIDOverride = NAME_None;
									RefreshSkinsList();
								}
								return FReply::Handled();
							}),
							ET66ButtonType::Primary).SetMinWidth(0.f))
						]
						+ SWidgetSwitcher::Slot()
						[
							SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f)
							[
								SNew(SBorder)
								.BorderImage(&PrimaryButtonStyle.Normal)
								.BorderBackgroundColor(FT66Style::Tokens::Accent2)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.Padding(FMargin(4.0f, 2.0f))
								[
									SNew(STextBlock)
									.Text(EquippedText)
									.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip"))
									.Justification(ETextJustify::Center)
								]
							]
						]
					]
				];
		}
		else
		{
			Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(5.0f, 0.0f)
				[
					FT66Style::MakeButton(FT66ButtonParams(PreviewText,
					FOnClicked::CreateLambda([this, SkinIDCopy]()
					{
						PreviewedCompanionSkinIDOverride = (PreviewedCompanionSkinIDOverride == SkinIDCopy) ? NAME_None : SkinIDCopy;
						UpdateCompanionDisplay();
						return FReply::Handled();
					}),
					ET66ButtonType::Neutral).SetMinWidth(80.f))
				];
			Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(4.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(90.0f).HeightOverride(36.0f)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(!bIsOwned ? 0 : (bIsEquipped ? 2 : 1))
						+ SWidgetSwitcher::Slot()
						[
						FT66Style::MakeButton(FT66ButtonParams(BuyText,
							FOnClicked::CreateLambda([this, CID, SkinIDCopy]()
							{
								if (CID.IsNone()) return FReply::Handled();
								UT66SkinSubsystem* SkinSub = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UT66SkinSubsystem>();
								if (!SkinSub || !SkinSub->PurchaseCompanionSkin(CID, SkinIDCopy, BeachgoerPriceAC)) return FReply::Handled();
								SkinSub->SetEquippedCompanionSkinID(CID, SkinIDCopy);
								PreviewedCompanionSkinIDOverride = NAME_None;
								RefreshSkinsList();
								return FReply::Handled();
							}),
							ET66ButtonType::Primary)
							.SetMinWidth(0.f)
							.SetColor(FT66Style::Tokens::Accent)
							.SetContent(
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ SNew(STextBlock).Text(BuyText).TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip")) ]
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ SNew(STextBlock).Text(BeachgoerPriceText).Font(FT66Style::Tokens::FontRegular(10)).ColorAndOpacity(FT66Style::Tokens::Text) ]
							))
						]
						+ SWidgetSwitcher::Slot()
						[
						FT66Style::MakeButton(FT66ButtonParams(EquipText,
							FOnClicked::CreateLambda([this, CID, SkinIDCopy]()
							{
								if (CID.IsNone()) return FReply::Handled();
								if (UT66SkinSubsystem* SkinSub = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UT66SkinSubsystem>())
								{
									SkinSub->SetEquippedCompanionSkinID(CID, SkinIDCopy);
									PreviewedCompanionSkinIDOverride = NAME_None;
									RefreshSkinsList();
								}
								return FReply::Handled();
							}),
							ET66ButtonType::Primary).SetMinWidth(0.f))
						]
						+ SWidgetSwitcher::Slot()
						[
							SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f)
							[
								SNew(SBorder)
								.BorderImage(&PrimaryButtonStyle.Normal)
								.BorderBackgroundColor(FT66Style::Tokens::Accent2)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.Padding(FMargin(4.0f, 2.0f))
								[
									SNew(STextBlock)
									.Text(EquippedText)
									.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip"))
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
					Row,
					FT66PanelParams(ET66PanelType::Panel2).SetColor(FT66Style::Tokens::Panel2).SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3)))
			];
	}
}

TSharedRef<SWidget> UT66CompanionSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	GeneratePlaceholderSkins();

	// Get localized text
	FText TitleText = Loc ? Loc->GetText_SelectCompanion() : NSLOCTEXT("T66.CompanionSelection", "Title", "SELECT COMPANION");
	FText CompanionGridText = Loc ? Loc->GetText_CompanionGrid() : NSLOCTEXT("T66.CompanionSelection", "Grid", "COMPANION GRID");
	FText NoCompanionText = Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.CompanionSelection", "NoCompanion", "NO COMPANION");
	FText SkinsText = Loc ? Loc->GetText_Skins() : NSLOCTEXT("T66.CompanionSelection", "Skins", "SKINS");
	FText LoreText = Loc ? Loc->GetText_Lore() : NSLOCTEXT("T66.CompanionSelection", "Lore", "LORE");
	FText ConfirmText = Loc ? Loc->GetText_ConfirmCompanion() : NSLOCTEXT("T66.CompanionSelection", "ConfirmCompanion", "CONFIRM COMPANION");
	FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	FText BuyText = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	FText EquipText = Loc ? Loc->GetText_Equip() : NSLOCTEXT("T66.Common", "Equip", "EQUIP");

	// AC balance and skins list (same pattern as hero selection)
	int32 ACBalance = 0;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
			ACBalance = SkinSub->GetAchievementCoinsBalance();
	}
	const FText ACBalanceText = Loc
		? FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(ACBalance))
		: FText::Format(NSLOCTEXT("T66.Achievements", "AchievementCoinsFormatFallback", "{0} AC"), FText::AsNumber(ACBalance));

	SAssignNew(SkinsListBoxWidget, SVerticalBox);
	AddSkinRowsToBox(SkinsListBoxWidget);

	// Carousel list: [NO COMPANION] + companions
	UT66GameInstance* GICheck = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	TArray<FName> CarouselIDs;
	CarouselIDs.Add(NAME_None);
	CarouselIDs.Append(AllCompanionIDs);
	const int32 NumCarousel = CarouselIDs.Num();
	const int32 CarouselIndex = FMath::Clamp(CurrentCompanionIndex + 1, 0, NumCarousel > 0 ? NumCarousel - 1 : 0);

	// Ensure we have stable brushes for the 5 visible slots (portraits).
	CompanionCarouselPortraitBrushes.SetNum(5);
	for (int32 i = 0; i < CompanionCarouselPortraitBrushes.Num(); ++i)
	{
		if (!CompanionCarouselPortraitBrushes[i].IsValid())
		{
			CompanionCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			CompanionCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(60.f, 60.f);
		}
	}
	RefreshCompanionCarouselPortraits();

	// Build carousel (5 tiles: prev2, prev1, current, next1, next2) — portrait or colored fallback
	TSharedRef<SHorizontalBox> CompanionCarousel = SNew(SHorizontalBox);
	for (int32 Offset = -2; Offset <= 2; Offset++)
	{
		const int32 SlotIdx = Offset + 2;
		int32 Idx = NumCarousel > 0 ? (CarouselIndex + Offset + NumCarousel * 2) % NumCarousel : 0;
		FName CompanionID = NumCarousel > 0 ? CarouselIDs[Idx] : NAME_None;
		FCompanionData Data;
		FLinearColor SpriteColor = FLinearColor(0.35f, 0.25f, 0.25f, 1.0f);
		if (!CompanionID.IsNone() && GICheck && GICheck->GetCompanionData(CompanionID, Data))
		{
			SpriteColor = Data.PlaceholderColor;
		}
		const bool bUnlocked = IsCompanionUnlocked(CompanionID);
		if (!CompanionID.IsNone() && !bUnlocked)
		{
			SpriteColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
		}
		const float BoxSize = (Offset == 0) ? 60.0f : 45.0f;
		const float Opacity = (Offset == 0) ? 1.0f : 0.6f;
		CompanionCarousel->AddSlot()
			.AutoWidth()
			.Padding(4.0f, 0.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(BoxSize).HeightOverride(BoxSize)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBorder)
						.BorderBackgroundColor(SpriteColor * Opacity)
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
							if (!CompanionCarouselPortraitBrushes.IsValidIndex(SlotIdx) || !CompanionCarouselPortraitBrushes[SlotIdx].IsValid())
							{
								return EVisibility::Collapsed;
							}
							return CompanionCarouselPortraitBrushes[SlotIdx]->GetResourceObject() ? EVisibility::Visible : EVisibility::Collapsed;
						})
						.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity))
					]
				]
			];
	}

	// Current display name/lore for right panel
	FText CurrentCompanionName = NoCompanionText;
	FText CurrentCompanionLore = NSLOCTEXT("T66.CompanionSelection", "NoCompanionLore", "Selecting no companion means you face the tribulation alone.");
	if (!PreviewedCompanionID.IsNone())
	{
		FCompanionData Data;
		if (GetPreviewedCompanionData(Data))
		{
			CurrentCompanionName = Loc ? Loc->GetCompanionDisplayName(Data) : Data.DisplayName;
			CurrentCompanionLore = Data.LoreText;
		}
	}

	// Preview color for center (fallback when no 3D stage)
	FLinearColor PreviewColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
	if (!PreviewedCompanionID.IsNone())
	{
		FCompanionData Data;
		if (GetPreviewedCompanionData(Data))
			PreviewColor = Data.PlaceholderColor;
	}
	if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
	{
		PreviewColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
	}

	const FTextBlockStyle& TxtHeading = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading");
	const FTextBlockStyle& TxtButton = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button");

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
		.Padding(0)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				// === TOP BAR: Companion Grid (next to left arrow) + Carousel + No Companion ===
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(20.0f, 15.0f, 20.0f, 10.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 10.0f, 0.0f)
						[
						FT66Style::MakeButton(FT66ButtonParams(CompanionGridText,
							FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleCompanionGridClicked),
							ET66ButtonType::Neutral).SetMinWidth(160.f).SetPadding(FMargin(12.f, 8.f)))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f)
						[
						FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66.Common", "Prev", "<"),
							FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandlePrevClicked),
							ET66ButtonType::Neutral).SetMinWidth(40.f).SetHeight(40.f).SetFontSize(20))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							CompanionCarousel
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f)
						[
						FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66.Common", "Next", ">"),
							FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleNextClicked),
							ET66ButtonType::Neutral).SetMinWidth(40.f).SetHeight(40.f).SetFontSize(20))
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(20.0f, 0.0f, 0.0f, 0.0f)
					[
					FT66Style::MakeButton(FT66ButtonParams(NoCompanionText,
						FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleNoCompanionClicked),
						ET66ButtonType::Neutral)
						.SetMinWidth(160.f)
						.SetPadding(FMargin(12.f, 8.f))
						.SetColor(PreviewedCompanionID.IsNone() ? FT66Style::Tokens::Accent : FT66Style::Tokens::Panel2))
					]
				]
				// === MAIN CONTENT: Left 0.28 | Center 0.44 | Right 0.28 (same size as hero selection) ===
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(20.0f, 10.0f, 20.0f, 10.0f)
				[
					SNew(SHorizontalBox)
					// LEFT PANEL: Skins (same width and padding as hero selection)
					+ SHorizontalBox::Slot()
					.FillWidth(0.28f)
					.VAlign(VAlign_Fill)
					.Padding(0.0f, 0.0f, 10.0f, 80.0f)
					[
						FT66Style::MakePanel(
							SNew(SVerticalBox)
							// Skins header + AC balance (match hero selection)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 0.0f, 0.0f, 10.0f)
							[
								SNew(SOverlay)
								+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center)
								[
									SNew(STextBlock).Text(SkinsText)
									.TextStyle(&TxtHeading)
								]
								+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Center)
								[
									FT66Style::MakePanel(
										SAssignNew(ACBalanceTextBlock, STextBlock)
										.Text(ACBalanceText)
										.Font(FT66Style::Tokens::FontBold(22))
										.ColorAndOpacity(FT66Style::Tokens::Text),
										FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(15.0f, 8.0f)))
								]
							]
							+ SVerticalBox::Slot()
							.FillHeight(1.0f)
							[
								SkinsListBoxWidget.ToSharedRef()
							],
							FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(FT66Style::Tokens::Space3)))
					]
					// CENTER: Companion Preview (3D or colored box)
					+ SHorizontalBox::Slot()
					.FillWidth(0.44f)
					.Padding(10.0f, 0.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
						[
						SNew(SBox)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							CreateCompanionPreviewWidget(PreviewColor)
						]
						]
					]
					// RIGHT PANEL: Companion Info / Lore (SWidgetSwitcher, same pattern as hero selection)
					+ SHorizontalBox::Slot()
					.FillWidth(0.28f)
					.Padding(10.0f, 0.0f, 0.0f, 0.0f)
					[
						FT66Style::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.FillHeight(1.0f)
							[
								SNew(SWidgetSwitcher)
								.WidgetIndex_Lambda([this]() -> int32 { return bShowingLore ? 1 : 0; })
								// 0) COMPANION INFO view
								+ SWidgetSwitcher::Slot()
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									.HAlign(HAlign_Center)
									.Padding(0.0f, 0.0f, 0.0f, 8.0f)
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("T66.CompanionSelection", "CompanionInfo", "COMPANION INFO"))
										.TextStyle(&TxtHeading)
									]
									// Name + LORE button same row
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
												SAssignNew(CompanionNameWidget, STextBlock)
												.Text(CurrentCompanionName)
												.TextStyle(&TxtButton)
												.Justification(ETextJustify::Center)
												.AutoWrapText(true),
												FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2)))
										]
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										.Padding(8.0f, 0.0f, 0.0f, 0.0f)
										[
										FT66Style::MakeButton(FT66ButtonParams(LoreText,
											FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleLoreClicked),
											ET66ButtonType::Neutral)
											.SetMinWidth(110.f)
											.SetDynamicLabel(TAttribute<FText>::CreateLambda([this, LoreText, BackText]() -> FText { return bShowingLore ? BackText : LoreText; })))
										]
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.0f, 0.0f, 0.0f, 10.0f)
									[
										SAssignNew(CompanionLoreWidget, STextBlock)
										.Text(CurrentCompanionLore)
										.Font(FT66Style::Tokens::FontRegular(12))
										.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
										.AutoWrapText(true)
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.0f, 0.0f, 0.0f, 10.0f)
									[
										SNew(SBorder)
										.BorderBackgroundColor(FLinearColor(0.1f, 0.15f, 0.1f, 1.0f))
										.Padding(FMargin(10.0f))
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("T66.CompanionSelection", "CompanionPassivePlaceholder", "Passive: Heals the player during combat"))
											.Font(FT66Style::Tokens::FontRegular(11))
											.ColorAndOpacity(FLinearColor(0.6f, 0.9f, 0.6f, 1.0f))
											.AutoWrapText(true)
										]
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.HAlign(HAlign_Center)
									.Padding(0.0f, 0.0f, 0.0f, 10.0f)
									[
										SAssignNew(CompanionUnionBox, SBox)
										[
											SNew(SVerticalBox)
											+ SVerticalBox::Slot()
											.AutoHeight()
											.HAlign(HAlign_Center)
											[
												SAssignNew(CompanionUnionText, STextBlock)
												.Text(FText::GetEmpty())
												.Font(FT66Style::Tokens::FontBold(12))
												.ColorAndOpacity(FT66Style::Tokens::TextMuted)
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.HAlign(HAlign_Center)
											.Padding(0.f, 6.f)
											[
												SNew(SBox)
												.WidthOverride(240.f)
												.HeightOverride(14.f)
												[
													SNew(SOverlay)
													+ SOverlay::Slot()
													[
														SAssignNew(CompanionUnionProgressBar, SProgressBar)
														.Percent_Lambda([this]() -> TOptional<float> { return FMath::Clamp(CompanionUnionProgress01, 0.f, 1.f); })
														.FillColorAndOpacity(FLinearColor(0.20f, 0.65f, 0.35f, 1.0f))
													]
													+ SOverlay::Slot()
													.HAlign(HAlign_Left)
													.Padding(FMargin(240.f * 0.25f - 1.f, 0.f, 0.f, 0.f))
													[
														SNew(SBox).WidthOverride(2.f).HeightOverride(14.f)
														[
															SNew(SBorder)
															.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
															.BorderBackgroundColor(FLinearColor(0.95f, 0.95f, 0.98f, 0.65f))
														]
													]
													+ SOverlay::Slot()
													.HAlign(HAlign_Left)
													.Padding(FMargin(240.f * 0.50f - 1.f, 0.f, 0.f, 0.f))
													[
														SNew(SBox).WidthOverride(2.f).HeightOverride(14.f)
														[
															SNew(SBorder)
															.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
															.BorderBackgroundColor(FLinearColor(0.95f, 0.95f, 0.98f, 0.65f))
														]
													]
													+ SOverlay::Slot()
													.HAlign(HAlign_Right)
													[
														SNew(SBox).WidthOverride(2.f).HeightOverride(14.f)
														[
															SNew(SBorder)
															.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
															.BorderBackgroundColor(FLinearColor(0.95f, 0.95f, 0.98f, 0.65f))
														]
													]
												]
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.HAlign(HAlign_Center)
											[
												SAssignNew(CompanionUnionHealingText, STextBlock)
												.Text(FText::GetEmpty())
												.Font(FT66Style::Tokens::FontBold(12))
												.ColorAndOpacity(FT66Style::Tokens::TextMuted)
											]
										]
									]
									+ SVerticalBox::Slot().FillHeight(1.0f)
									[
										SNew(SBox)
									]
								]
								// 1) LORE view (same panel size, same theme as hero selection)
								+ SWidgetSwitcher::Slot()
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									.HAlign(HAlign_Center)
									.Padding(0.0f, 0.0f, 0.0f, 8.0f)
									[
										SNew(STextBlock).Text(LoreText)
										.TextStyle(&TxtHeading)
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
											.Text_Lambda([this, Loc, NoCompanionText]() -> FText
											{
												if (PreviewedCompanionID.IsNone()) return NoCompanionText;
												FCompanionData Data;
												if (GetPreviewedCompanionData(Data))
													return Loc ? Loc->GetCompanionDisplayName(Data) : Data.DisplayName;
												return NoCompanionText;
											})
											.TextStyle(&TxtButton)
											.Justification(ETextJustify::Center),
											FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2)))
									]
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)
									.Padding(8.0f, 0.0f, 0.0f, 0.0f)
									[
									FT66Style::MakeButton(FT66ButtonParams(BackText,
											FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleLoreClicked),
											ET66ButtonType::Neutral).SetMinWidth(110.f))
										]
									]
									+ SVerticalBox::Slot()
									.FillHeight(1.0f)
									[
										SNew(SScrollBox)
										+ SScrollBox::Slot()
										[
											SAssignNew(CompanionLoreDetailWidget, STextBlock)
											.Text_Lambda([this, Loc]() -> FText
											{
												return Loc ? Loc->GetText_CompanionLore(PreviewedCompanionID) : NSLOCTEXT("T66.CompanionLore", "FallbackLore", "No lore available.");
											})
											.Font(FT66Style::Tokens::FontRegular(14))
											.ColorAndOpacity(FT66Style::Tokens::TextMuted)
											.AutoWrapText(true)
										]
									]
								]
							]
					,
					FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(FT66Style::Tokens::Space4)))
				]
			]
			// === BOTTOM BAR: Confirm (same font as Choose Companion) ===
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(20.0f, 10.0f, 20.0f, 20.0f)
				[
				FT66Style::MakeButton(FT66ButtonParams(ConfirmText,
					FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleConfirmClicked),
					ET66ButtonType::Primary)
					.SetMinWidth(240.f)
					.SetEnabled(TAttribute<bool>::CreateLambda([this]() -> bool
					{
						return PreviewedCompanionID.IsNone() || IsCompanionUnlocked(PreviewedCompanionID);
					})))
				]
			]
			// Back button (bottom-left overlay, same theme as hero selection)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(20.0f, 0.0f, 0.0f, 20.0f)
			[
			FT66Style::MakeButton(FT66ButtonParams(BackText,
				FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleBackClicked),
				ET66ButtonType::Neutral).SetMinWidth(120.f))
			]
		];
}

FReply UT66CompanionSelectionScreen::HandlePrevClicked() { PreviewPreviousCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleNextClicked() { PreviewNextCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleCompanionGridClicked() { OnCompanionGridClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleNoCompanionClicked() { SelectNoCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleLoreClicked() { bShowingLore = !bShowingLore; UpdateCompanionDisplay(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleConfirmClicked() { OnConfirmCompanionClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

AT66CompanionPreviewStage* UT66CompanionSelectionScreen::GetCompanionPreviewStage() const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It)
		return *It;
	return nullptr;
}

TSharedRef<SWidget> UT66CompanionSelectionScreen::CreateCompanionPreviewWidget(const FLinearColor& FallbackColor)
{
	AT66CompanionPreviewStage* Stage = GetCompanionPreviewStage();

	if (Stage)
	{
		// In-world preview: transparent overlay for drag-rotate/zoom.
		// The main viewport renders the companion with full Lumen GI behind the UI.
		return SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(ST66DragRotateCompanionPreview)
				.Stage(Stage)
				.DegreesPerPixel(0.28f)
			];
	}
	return SAssignNew(CompanionPreviewColorBox, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FallbackColor)
		[
			SNew(SBox)
		];
}

void UT66CompanionSelectionScreen::UpdateCompanionDisplay()
{
	FName EffectiveSkin = PreviewedCompanionSkinIDOverride;
	if (EffectiveSkin.IsNone() && !PreviewedCompanionID.IsNone())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
			{
				EffectiveSkin = SkinSub->GetEquippedCompanionSkinID(PreviewedCompanionID);
			}
		}
	}
	if (EffectiveSkin.IsNone()) EffectiveSkin = FName(TEXT("Default"));

	if (AT66CompanionPreviewStage* Stage = GetCompanionPreviewStage())
	{
		Stage->SetPreviewCompanion(PreviewedCompanionID, EffectiveSkin);
	}
	else if (CompanionPreviewColorBox.IsValid())
	{
		FCompanionData Data;
		if (GetPreviewedCompanionData(Data))
			CompanionPreviewColorBox->SetBorderBackgroundColor(Data.PlaceholderColor);
		else
			CompanionPreviewColorBox->SetBorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.4f, 1.0f));

		if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
		{
			CompanionPreviewColorBox->SetBorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f, 1.0f));
		}
	}

	if (CompanionNameWidget.IsValid())
	{
		if (PreviewedCompanionID.IsNone())
		{
			UT66LocalizationSubsystem* Loc = GetLocSubsystem();
			CompanionNameWidget->SetText(Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.CompanionSelection", "NoCompanionTitle", "No Companion"));
		}
		else
		{
			FCompanionData Data;
			if (GetPreviewedCompanionData(Data))
			{
				UT66LocalizationSubsystem* Loc = GetLocSubsystem();
				CompanionNameWidget->SetText(Loc ? Loc->GetCompanionDisplayName(Data) : Data.DisplayName);
			}
		}
	}

	if (CompanionLoreWidget.IsValid())
	{
		if (PreviewedCompanionID.IsNone())
		{
			CompanionLoreWidget->SetText(NSLOCTEXT("T66.CompanionSelection", "NoCompanionLore", "Selecting no companion means you face the tribulation alone."));
		}
		else
		{
			FCompanionData Data;
			if (GetPreviewedCompanionData(Data))
				CompanionLoreWidget->SetText(Data.LoreText);
		}
	}

	// Friendliness / Union UI (profile progression; only meaningful when a companion is selected)
	if (CompanionUnionBox.IsValid())
	{
		const bool bShowUnion = !PreviewedCompanionID.IsNone() && IsCompanionUnlocked(PreviewedCompanionID);
		CompanionUnionBox->SetVisibility(bShowUnion ? EVisibility::Visible : EVisibility::Collapsed);
	}

	CompanionUnionProgress01 = 0.f;
	CompanionUnionStagesCleared = 0;
	if (!PreviewedCompanionID.IsNone())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
			{
				CompanionUnionStagesCleared = FMath::Max(0, Ach->GetCompanionUnionStagesCleared(PreviewedCompanionID));
				CompanionUnionProgress01 = FMath::Clamp(Ach->GetCompanionUnionProgress01(PreviewedCompanionID), 0.f, 1.f);

				const int32 Needed = UT66AchievementsSubsystem::UnionTier_HyperStages;
				if (CompanionUnionText.IsValid())
				{
					CompanionUnionText->SetText(FText::Format(
						NSLOCTEXT("T66.CompanionSelection", "FriendlinessStagesFormat", "Stages: {0}/{1}"),
						FText::AsNumber(CompanionUnionStagesCleared),
						FText::AsNumber(Needed)));
				}

				int32 HealingType = 1; // Basic
				if (CompanionUnionStagesCleared >= UT66AchievementsSubsystem::UnionTier_HyperStages) HealingType = 4;
				else if (CompanionUnionStagesCleared >= UT66AchievementsSubsystem::UnionTier_MediumStages) HealingType = 3;
				else if (CompanionUnionStagesCleared >= UT66AchievementsSubsystem::UnionTier_GoodStages) HealingType = 2;

				if (CompanionUnionHealingText.IsValid())
				{
					CompanionUnionHealingText->SetText(FText::Format(
						NSLOCTEXT("T66.CompanionSelection", "HealingTypeFormat", "Healing Type: {0}"),
						FText::AsNumber(HealingType)));
				}
			}
		}
	}

	RefreshCompanionCarouselPortraits();
}

void UT66CompanionSelectionScreen::RefreshCompanionCarouselPortraits()
{
	TArray<FName> CarouselIDs;
	CarouselIDs.Add(NAME_None);
	CarouselIDs.Append(AllCompanionIDs);
	const int32 NumCarousel = CarouselIDs.Num();
	const int32 CarouselIndex = FMath::Clamp(CurrentCompanionIndex + 1, 0, NumCarousel > 0 ? NumCarousel - 1 : 0);

	CompanionCarouselPortraitBrushes.SetNum(5);
	for (int32 i = 0; i < CompanionCarouselPortraitBrushes.Num(); ++i)
	{
		if (!CompanionCarouselPortraitBrushes[i].IsValid())
		{
			CompanionCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			CompanionCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(60.f, 60.f);
		}
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		UT66UITexturePoolSubsystem* TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
		for (int32 Offset = -2; Offset <= 2; ++Offset)
		{
			const int32 SlotIdx = Offset + 2;
			if (!CompanionCarouselPortraitBrushes.IsValidIndex(SlotIdx) || !CompanionCarouselPortraitBrushes[SlotIdx].IsValid())
			{
				continue;
			}
			const int32 Idx = NumCarousel > 0 ? (CarouselIndex + Offset + NumCarousel * 2) % NumCarousel : 0;
			const FName CompanionID = NumCarousel > 0 ? CarouselIDs[Idx] : NAME_None;
			TSoftObjectPtr<UTexture2D> PortraitSoft;
			if (!CompanionID.IsNone())
			{
				FCompanionData D;
				if (GI->GetCompanionData(CompanionID, D) && !D.Portrait.IsNull())
				{
					PortraitSoft = D.Portrait;
				}
			}
			const float BoxSize = (Offset == 0) ? 60.f : 45.f;
			if (PortraitSoft.IsNull() || !TexPool)
			{
				CompanionCarouselPortraitBrushes[SlotIdx]->SetResourceObject(nullptr);
			}
			else
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, CompanionCarouselPortraitBrushes[SlotIdx], FName(TEXT("CompanionCarousel"), SlotIdx + 1), /*bClearWhileLoading*/ true);
			}
			CompanionCarouselPortraitBrushes[SlotIdx]->ImageSize = FVector2D(BoxSize, BoxSize);
		}
	}
}

void UT66CompanionSelectionScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66CompanionSelectionScreen::OnLanguageChanged);
	}
	RefreshCompanionList();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (!GI->SelectedCompanionID.IsNone() && AllCompanionIDs.Contains(GI->SelectedCompanionID))
			PreviewCompanion(GI->SelectedCompanionID);
		else
			SelectNoCompanion();
	}
	if (UWorld* World = GetWorld())
	{
		if (AT66FrontendGameMode* GM = Cast<AT66FrontendGameMode>(World->GetAuthGameMode()))
			GM->PositionCameraForCompanionPreview();
	}
}

void UT66CompanionSelectionScreen::RefreshScreen_Implementation()
{
	FCompanionData Data;
	bool bIsNoCompanion = PreviewedCompanionID.IsNone();
	if (!bIsNoCompanion) GetPreviewedCompanionData(Data);
	OnPreviewedCompanionChanged(Data, bIsNoCompanion);
	UpdateCompanionDisplay();
}

void UT66CompanionSelectionScreen::RefreshCompanionList()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		AllCompanionIDs = GI->GetAllCompanionIDs();
}

TArray<FCompanionData> UT66CompanionSelectionScreen::GetAllCompanions()
{
	TArray<FCompanionData> Companions;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		for (const FName& ID : AllCompanionIDs)
		{
			FCompanionData Data;
			if (GI->GetCompanionData(ID, Data)) Companions.Add(Data);
		}
	}
	return Companions;
}

bool UT66CompanionSelectionScreen::GetPreviewedCompanionData(FCompanionData& OutData)
{
	if (PreviewedCompanionID.IsNone()) return false;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		return GI->GetCompanionData(PreviewedCompanionID, OutData);
	return false;
}

void UT66CompanionSelectionScreen::PreviewCompanion(FName ID)
{
	PreviewedCompanionID = ID;
	PreviewedCompanionSkinIDOverride = NAME_None;
	CurrentCompanionIndex = ID.IsNone() ? -1 : AllCompanionIDs.IndexOfByKey(ID);
	if (CurrentCompanionIndex == INDEX_NONE) CurrentCompanionIndex = -1;
	FCompanionData Data;
	bool bIsNoCompanion = ID.IsNone();
	if (!bIsNoCompanion) GetPreviewedCompanionData(Data);
	OnPreviewedCompanionChanged(Data, bIsNoCompanion);
	RefreshSkinsList();
}

void UT66CompanionSelectionScreen::SelectNoCompanion() { PreviewCompanion(NAME_None); }

void UT66CompanionSelectionScreen::PreviewNextCompanion()
{
	if (AllCompanionIDs.Num() == 0) return;
	CurrentCompanionIndex++;
	if (CurrentCompanionIndex >= AllCompanionIDs.Num())
		SelectNoCompanion();
	else
		PreviewCompanion(AllCompanionIDs[CurrentCompanionIndex]);
}

void UT66CompanionSelectionScreen::PreviewPreviousCompanion()
{
	if (AllCompanionIDs.Num() == 0) return;
	CurrentCompanionIndex--;
	if (CurrentCompanionIndex < -1)
	{
		CurrentCompanionIndex = AllCompanionIDs.Num() - 1;
		PreviewCompanion(AllCompanionIDs[CurrentCompanionIndex]);
	}
	else if (CurrentCompanionIndex == -1)
		SelectNoCompanion();
	else
		PreviewCompanion(AllCompanionIDs[CurrentCompanionIndex]);
}

void UT66CompanionSelectionScreen::OnCompanionGridClicked() { ShowModal(ET66ScreenType::CompanionGrid); }
void UT66CompanionSelectionScreen::OnCompanionLoreClicked() { if (!PreviewedCompanionID.IsNone()) ShowModal(ET66ScreenType::CompanionLore); }
void UT66CompanionSelectionScreen::OnConfirmCompanionClicked()
{
	// Locked companions cannot be confirmed/selected.
	if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
	{
		return;
	}
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedCompanionID = PreviewedCompanionID;
	}
	NavigateBack();
}
void UT66CompanionSelectionScreen::OnBackClicked() { NavigateBack(); }

void UT66CompanionSelectionScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}
