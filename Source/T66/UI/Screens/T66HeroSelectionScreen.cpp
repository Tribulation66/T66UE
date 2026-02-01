// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Images/SImage.h"
#include "Engine/TextureRenderTarget2D.h"

namespace
{
	class ST66DragRotatePreview : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66DragRotatePreview) {}
			SLATE_ARGUMENT(TWeakObjectPtr<AT66HeroPreviewStage>, Stage)
			SLATE_ARGUMENT(const FSlateBrush*, Brush)
			SLATE_ARGUMENT(float, DegreesPerPixel)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			Stage = InArgs._Stage;
			DegreesPerPixel = InArgs._DegreesPerPixel;
			bDragging = false;

			ChildSlot
			[
				SNew(SImage).Image(InArgs._Brush)
			];
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

	// Shared skins for heroes and companions (reuse localization in UT66LocalizationSubsystem::GetText_SkinName).
	static constexpr int32 SkinPriceAC = 250;
	const FName SkinIDs[5] = {
		FName(TEXT("Default")),
		FName(TEXT("Golden")),
		FName(TEXT("Shadow")),
		FName(TEXT("Infernal")),
		FName(TEXT("Frost")),
	};
	for (const FName& SkinID : SkinIDs)
	{
		FSkinData Skin;
		Skin.SkinID = SkinID;
		Skin.bIsDefault = false;
		Skin.bIsOwned = false;   // Buy-only stub
		Skin.bIsEquipped = false;
		Skin.CoinCost = (SkinID == FName(TEXT("Default"))) ? 0 : SkinPriceAC;
		PlaceholderSkins.Add(Skin);
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
			ET66Difficulty::VeryHard, ET66Difficulty::Impossible, ET66Difficulty::Perdition, ET66Difficulty::Final
		};
		if (Index < Difficulties.Num())
		{
			SelectedDifficulty = Difficulties[Index];
			CurrentDifficultyOption = NewValue;
		}
	}
}

TSharedRef<SWidget> UT66HeroSelectionScreen::GenerateDifficultyItem(TSharedPtr<FString> InItem)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InItem))
		.Font(FT66Style::Tokens::FontRegular(12))
		.ColorAndOpacity(FT66Style::Tokens::Text);
}

TSharedRef<SWidget> UT66HeroSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	GeneratePlaceholderSkins();

	// Get localized text
	FText HeroGridText = Loc ? Loc->GetText_HeroGrid() : NSLOCTEXT("T66.HeroSelection", "HeroGrid", "HERO GRID");
	FText ChooseCompanionText = Loc ? Loc->GetText_ChooseCompanion() : NSLOCTEXT("T66.HeroSelection", "ChooseCompanion", "CHOOSE COMPANION");
	FText SkinsText = Loc ? Loc->GetText_Skins() : NSLOCTEXT("T66.HeroSelection", "Skins", "SKINS");
	FText HeroInfoText = Loc ? Loc->GetText_HeroInfo() : NSLOCTEXT("T66.HeroSelection", "HeroInfo", "HERO INFO");
	FText LoreText = Loc ? Loc->GetText_Lore() : NSLOCTEXT("T66.HeroSelection", "Lore", "LORE");
	FText TheLabText = Loc ? Loc->GetText_TheLab() : NSLOCTEXT("T66.HeroSelection", "TheLab", "THE LAB");
	FText EnterText = Loc ? Loc->GetText_EnterTheTribulation() : NSLOCTEXT("T66.HeroSelection", "EnterTheTribulation", "ENTER THE TRIBULATION");
	FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	FText BuyText = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	(void)TheLabText; // (The Lab button removed from this screen)

	// Initialize difficulty dropdown options
	DifficultyOptions.Empty();
	TArray<ET66Difficulty> Difficulties = {
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard,
		ET66Difficulty::VeryHard, ET66Difficulty::Impossible, ET66Difficulty::Perdition, ET66Difficulty::Final
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
		if (const UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			ACBalance = Ach->GetAchievementCoinsBalance();
		}
	}
	const FText ACBalanceText = Loc
		? FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(ACBalance))
		: FText::Format(NSLOCTEXT("T66.Achievements", "AchievementCoinsFormatFallback", "{0} AC"), FText::AsNumber(ACBalance));

	// Build skins list (5 entries, fill panel)
	TSharedRef<SVerticalBox> SkinsListBox = SNew(SVerticalBox);
	for (const FSkinData& Skin : PlaceholderSkins)
	{
		FText SkinDisplayName = Loc ? Loc->GetText_SkinName(Skin.SkinID) : FText::FromName(Skin.SkinID);
		const int32 Price = FMath::Max(0, Skin.CoinCost);
		const FText PriceText = Loc
			? FText::Format(Loc->GetText_AchievementCoinsFormat(), FText::AsNumber(Price))
			: FText::Format(NSLOCTEXT("T66.Achievements", "AchievementCoinsFormatFallback", "{0} AC"), FText::AsNumber(Price));

		SkinsListBox->AddSlot()
		.FillHeight(1.0f)
		.Padding(0.0f, 6.0f)
		[
			SNew(SBorder)
			.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
			.BorderBackgroundColor(FT66Style::Tokens::Panel2)
			.Padding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(SkinDisplayName)
					.Font(FT66Style::Tokens::FontRegular(16))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(5.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(110.0f).HeightOverride(46.0f)
					[
						SNew(SButton)
						.HAlign(HAlign_Center).VAlign(VAlign_Center)
						.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
						.ButtonColorAndOpacity(FT66Style::Tokens::Accent)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SNew(STextBlock)
								.Text(BuyText)
								.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip"))
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SNew(STextBlock)
								.Text(PriceText)
								.Font(FT66Style::Tokens::FontRegular(12))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						]
					]
				]
			]
		];
	}

	// Get current hero color for preview
	FLinearColor HeroPreviewColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f); // Default gray
	FHeroData CurrentHeroData;
	if (GetPreviewedHeroData(CurrentHeroData))
	{
		HeroPreviewColor = CurrentHeroData.PlaceholderColor;
	}

	// Build hero sprite carousel (colored boxes for each hero)
	TSharedRef<SHorizontalBox> HeroCarousel = SNew(SHorizontalBox);
	RefreshHeroList(); // Ensure hero list is populated
	
	// Show 5 heroes centered on current (prev2, prev1, current, next1, next2)
	for (int32 Offset = -2; Offset <= 2; Offset++)
	{
		if (AllHeroIDs.Num() == 0) break;

		const float BoxSize = (Offset == 0) ? 60.0f : 45.0f; // Current hero is larger
		const float Opacity = (Offset == 0) ? 1.0f : 0.6f;
		
		HeroCarousel->AddSlot()
		.AutoWidth()
		.Padding(4.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(BoxSize)
			.HeightOverride(BoxSize)
			[
				SNew(SBorder)
				.BorderBackgroundColor_Lambda([this, Offset, Opacity]() -> FSlateColor
				{
					if (AllHeroIDs.Num() <= 0)
					{
						return FLinearColor(0.2f, 0.2f, 0.25f, 1.0f);
					}

					const int32 HeroIdx = (CurrentHeroIndex + Offset + AllHeroIDs.Num()) % AllHeroIDs.Num();
					FLinearColor SpriteColor(0.2f, 0.2f, 0.25f, 1.0f);
					if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
					{
						FHeroData HeroData;
						if (GI->GetHeroData(AllHeroIDs[HeroIdx], HeroData))
						{
							SpriteColor = HeroData.PlaceholderColor;
						}
					}
					return SpriteColor * Opacity;
				})
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			]
		];
	}

	const FButtonStyle& BtnNeutral = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral");
	const FButtonStyle& BtnPrimary = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary");
	const FButtonStyle& BtnDanger = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Danger");
	const FTextBlockStyle& TxtHeading = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading");
	const FTextBlockStyle& TxtBody = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");
	const FTextBlockStyle& TxtButton = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button");
	const FTextBlockStyle& TxtChip = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip");

	return SNew(SBorder)
		.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Bg"))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				// === TOP BAR: Hero Grid Button + Hero Sprite Carousel ===
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(20.0f, 15.0f, 20.0f, 10.0f)
				[
					SNew(SHorizontalBox)
					// Hero Sprite Carousel with navigation
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SHorizontalBox)
						// HERO GRID button to the left of the left carousel arrow
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.0f, 0.0f, 10.0f, 0.0f)
						[
							SNew(SBox).MinDesiredWidth(120.0f).HeightOverride(40.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleHeroGridClicked))
								.ButtonStyle(&BtnNeutral)
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								.ContentPadding(FMargin(12.f, 8.f))
								[
									SNew(STextBlock).Text(HeroGridText)
									.TextStyle(&TxtChip)
								]
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 12.0f, 0.0f)
						[
							SNew(SBox).WidthOverride(40.0f).HeightOverride(40.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandlePrevClicked))
								.ButtonStyle(&BtnNeutral)
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								[
									SNew(STextBlock).Text(NSLOCTEXT("T66.Common", "Prev", "<"))
									.Font(FT66Style::Tokens::FontBold(20))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							HeroCarousel
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f)
						[
							SNew(SBox).WidthOverride(40.0f).HeightOverride(40.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleNextClicked))
								.ButtonStyle(&BtnNeutral)
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								[
									SNew(STextBlock).Text(NSLOCTEXT("T66.Common", "Next", ">"))
									.Font(FT66Style::Tokens::FontBold(20))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
					]
				]
				// === MAIN CONTENT: Left Panel | Center Preview | Right Panel (equal side panels, center centered) ===
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(20.0f, 10.0f, 20.0f, 10.0f)
				[
					SNew(SHorizontalBox)
					// LEFT PANEL: Skins (same width as right)
					+ SHorizontalBox::Slot()
					.FillWidth(0.28f)
					.VAlign(VAlign_Fill)
					.Padding(0.0f, 0.0f, 10.0f, 80.0f)
					[
						SNew(SBorder)
						.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
						.Padding(FMargin(FT66Style::Tokens::Space3))
						[
							SNew(SVerticalBox)
							// Skins header
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 0.0f, 0.0f, 10.0f)
							[
								SNew(SOverlay)
								+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
								[
									SNew(STextBlock).Text(SkinsText)
									.TextStyle(&TxtHeading)
								]
								+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Center)
								[
									SNew(SBorder)
									.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
									.Padding(FMargin(15.0f, 8.0f))
									[
										SNew(STextBlock)
										.Text(ACBalanceText)
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
										.ColorAndOpacity(FLinearColor(1.0f, 0.9f, 0.5f, 1.0f))
									]
								]
							]
							// Skins list
							+ SVerticalBox::Slot()
							.FillHeight(1.0f)
							[
								SkinsListBox
							]
						]
					]
					// CENTER: Hero Preview (3D render target or colored box fallback)
					+ SHorizontalBox::Slot()
					.FillWidth(0.44f)
					.Padding(10.0f, 0.0f)
					[
						SNew(SVerticalBox)
						// Hero Preview Area - 3D preview image or colored box
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
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
									CreateHeroPreviewWidget(HeroPreviewColor)
								]
							]
							// Body Type: TYPE A and TYPE B buttons (only one selected) (right under the platform)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.0f, 12.0f, 0.0f, 0.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(4.0f, 0.0f)
								[
									SNew(SBox).WidthOverride(100.0f).HeightOverride(40.0f)
									[
										SNew(SButton)
										.HAlign(HAlign_Center).VAlign(VAlign_Center)
										.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBodyTypeAClicked))
										.ButtonColorAndOpacity_Lambda([this]() -> FSlateColor {
											return SelectedBodyType == ET66BodyType::TypeA
												? FT66Style::Tokens::Accent2
												: FT66Style::Tokens::Panel2;
										})
										.ButtonStyle(&BtnNeutral)
										[
											SNew(STextBlock)
											.Text(Loc ? Loc->GetText_BodyTypeA() : NSLOCTEXT("T66.HeroSelection", "BodyTypeA", "TYPE A"))
											.TextStyle(&TxtChip)
										]
									]
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(4.0f, 0.0f)
								[
									SNew(SBox).WidthOverride(100.0f).HeightOverride(40.0f)
									[
										SNew(SButton)
										.HAlign(HAlign_Center).VAlign(VAlign_Center)
										.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBodyTypeBClicked))
										.ButtonColorAndOpacity_Lambda([this]() -> FSlateColor {
											return SelectedBodyType == ET66BodyType::TypeB
												? FT66Style::Tokens::Accent2
												: FT66Style::Tokens::Panel2;
										})
										.ButtonStyle(&BtnNeutral)
										[
											SNew(STextBlock)
											.Text(Loc ? Loc->GetText_BodyTypeB() : NSLOCTEXT("T66.HeroSelection", "BodyTypeB", "TYPE B"))
											.TextStyle(&TxtChip)
										]
									]
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(12.0f, 0.0f, 0.0f, 0.0f)
								[
									SNew(SBox).WidthOverride(240.0f).HeightOverride(40.0f)
									[
										SNew(SButton)
										.HAlign(HAlign_Center).VAlign(VAlign_Center)
										.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleCompanionClicked))
										.ButtonStyle(&BtnPrimary)
										.ButtonColorAndOpacity(FT66Style::Tokens::Accent2)
										.ContentPadding(FMargin(14.f, 8.f))
										[
											SNew(STextBlock).Text(ChooseCompanionText)
											.TextStyle(&TxtButton)
										]
									]
								]
							]
						]
					]
					// RIGHT PANEL: Hero Info (same width as left)
					+ SHorizontalBox::Slot()
					.FillWidth(0.28f)
					.VAlign(VAlign_Fill)
					.Padding(10.0f, 0.0f, 0.0f, 80.0f)
					[
						SNew(SBorder)
						.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
						.Padding(FMargin(FT66Style::Tokens::Space4))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.FillHeight(1.0f)
							[
								SNew(SWidgetSwitcher)
								.WidgetIndex_Lambda([this]() -> int32 { return bShowingLore ? 1 : 0; })
								// 0) HERO INFO view
								+ SWidgetSwitcher::Slot()
								[
									SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								.HAlign(HAlign_Center)
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock).Text(HeroInfoText)
									.TextStyle(&TxtHeading)
								]
								// Hero name + Lore button on same row
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 12.0f)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.FillWidth(1.0f)
									.VAlign(VAlign_Center)
									[
										SNew(SBorder)
										.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
										.Padding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2))
										[
											SAssignNew(HeroNameWidget, STextBlock)
											.Text(Loc ? Loc->GetText_SelectYourHero() : NSLOCTEXT("T66.HeroSelection", "SelectHero", "Select a Hero"))
											.TextStyle(&TxtButton)
											.Justification(ETextJustify::Center)
										]
									]
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)
									.Padding(8.0f, 0.0f, 0.0f, 0.0f)
									[
										SNew(SBox).MinDesiredWidth(110.0f).HeightOverride(36.0f)
										[
											SNew(SButton)
											.HAlign(HAlign_Center).VAlign(VAlign_Center)
											.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleLoreClicked))
											.ButtonStyle(&BtnNeutral)
											.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
											[
												SNew(STextBlock)
												.Text_Lambda([this, LoreText, BackText]() -> FText { return bShowingLore ? BackText : LoreText; })
												.TextStyle(&TxtChip)
											]
										]
									]
								]
								// Video placeholder (square-ish)
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 12.0f)
								[
									SNew(SBorder)
									.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
									.Padding(FMargin(5.0f))
									[
										SNew(SBox).HeightOverride(360.0f)
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("T66.HeroSelection", "VideoPreview", "[VIDEO PREVIEW]"))
											.TextStyle(&TxtBody)
											.Justification(ETextJustify::Center)
										]
									]
								]
								// Medals placeholder (bigger, right below video)
								+ SVerticalBox::Slot()
								.AutoHeight()
								.HAlign(HAlign_Center)
								.Padding(0.0f, 0.0f, 0.0f, 12.0f)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f)
									[
										SNew(SBox).WidthOverride(52.0f).HeightOverride(52.0f)
										[
											SNew(SBorder).BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
										]
									]
									+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f)
									[
										SNew(SBox).WidthOverride(52.0f).HeightOverride(52.0f)
										[
											SNew(SBorder).BorderBackgroundColor(FLinearColor(0.3f, 0.1f, 0.1f, 1.0f))
										]
									]
									+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f)
									[
										SNew(SBox).WidthOverride(52.0f).HeightOverride(52.0f)
										[
											SNew(SBorder).BorderBackgroundColor(FLinearColor(0.4f, 0.35f, 0.1f, 1.0f))
										]
									]
									+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f)
									[
										SNew(SBox).WidthOverride(52.0f).HeightOverride(52.0f)
										[
											SNew(SBorder).BorderBackgroundColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
										]
									]
								]
								// Quote + Base Stats (replaces short description here)
								+ SVerticalBox::Slot()
								.FillHeight(1.0f)
								.Padding(0.0f, 0.0f, 0.0f, 0.0f)
								[
									SNew(SScrollBox)
									+ SScrollBox::Slot()
									[
										SAssignNew(HeroDescWidget, STextBlock)
										.Text(NSLOCTEXT("T66.HeroSelection", "SelectHeroDescriptionHint", "Select a hero to view their description."))
										.Font(FT66Style::Tokens::FontRegular(14))
										.ColorAndOpacity(FT66Style::Tokens::TextMuted)
										.AutoWrapText(true)
									]
								]
								]
								// 1) LORE view (same panel size)
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
										SNew(SBorder)
										.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
										.Padding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2))
										[
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
											.TextStyle(&TxtButton)
											.Justification(ETextJustify::Center)
										]
									]
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)
									.Padding(8.0f, 0.0f, 0.0f, 0.0f)
									[
										SNew(SBox).MinDesiredWidth(110.0f).HeightOverride(36.0f)
										[
											SNew(SButton)
											.HAlign(HAlign_Center).VAlign(VAlign_Center)
											.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleLoreClicked))
											.ButtonStyle(&BtnNeutral)
											.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
											[
												SNew(STextBlock)
												.Text(BackText)
												.TextStyle(&TxtChip)
											]
										]
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
										.Font(FT66Style::Tokens::FontRegular(14))
										.ColorAndOpacity(FT66Style::Tokens::TextMuted)
										.AutoWrapText(true)
									]
								]
								]
							]
							// Difficulty + Enter (in the right panel)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 10.0f, 0.0f, 0.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(0.42f)
								.VAlign(VAlign_Center)
								.Padding(0.0f, 0.0f, 8.0f, 0.0f)
								[
									SNew(SBox).HeightOverride(40.0f)
									[
										SNew(SComboBox<TSharedPtr<FString>>)
										.OptionsSource(&DifficultyOptions)
										.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo) {
											OnDifficultyChanged(NewValue, SelectInfo);
										})
										.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem) -> TSharedRef<SWidget> {
											return SNew(STextBlock)
												.Text(FText::FromString(*InItem))
												.Font(FT66Style::Tokens::FontRegular(12))
												.ColorAndOpacity(FT66Style::Tokens::Text);
										})
										.InitiallySelectedItem(CurrentDifficultyOption)
										[
											SNew(STextBlock)
											.Text_Lambda([this, Loc]() -> FText {
												return CurrentDifficultyOption.IsValid()
													? FText::FromString(*CurrentDifficultyOption)
													: (Loc ? Loc->GetText_Easy() : NSLOCTEXT("T66.Difficulty", "Easy", "Easy"));
											})
											.Font(FT66Style::Tokens::FontBold(12))
											.ColorAndOpacity(FT66Style::Tokens::Text)
										]
									]
								]
								+ SHorizontalBox::Slot()
								.FillWidth(0.58f)
								[
									SNew(SBox).HeightOverride(40.0f)
									[
										SNew(SButton)
										.HAlign(HAlign_Center).VAlign(VAlign_Center)
										.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleEnterClicked))
										.ButtonStyle(&BtnDanger)
										.ButtonColorAndOpacity(FT66Style::Tokens::Danger)
										.ContentPadding(FMargin(12.f, 8.f))
										[
											SNew(STextBlock).Text(EnterText)
											.Font(FT66Style::Tokens::FontBold(12))
											.ColorAndOpacity(FT66Style::Tokens::Text)
										]
									]
								]
							]
						]
					]
				]
			]
			// Back button (bottom-left overlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(20.0f, 0.0f, 0.0f, 20.0f)
			[
				SNew(SBox).MinDesiredWidth(120.0f).HeightOverride(40.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center).VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBackClicked))
					.ButtonStyle(&BtnNeutral)
					.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
					[
						SNew(STextBlock).Text(BackText)
						.TextStyle(&TxtButton)
					]
				]
			]
		];
}

FReply UT66HeroSelectionScreen::HandlePrevClicked()
{
	PreviewPreviousHero();
	ReleaseSlateResources(true);
	TakeWidget();
	return FReply::Handled();
}
FReply UT66HeroSelectionScreen::HandleNextClicked()
{
	PreviewNextHero();
	ReleaseSlateResources(true);
	TakeWidget();
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

FReply UT66HeroSelectionScreen::HandleBodyTypeAClicked()
{
	SelectedBodyType = ET66BodyType::TypeA;
	TakeWidget(); // Refresh so TYPE A button highlights
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleBodyTypeBClicked()
{
	SelectedBodyType = ET66BodyType::TypeB;
	TakeWidget(); // Refresh so TYPE B button highlights
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
			if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
			{
				GI->GetHeroStatTuning(PreviewedHeroID, BaseStats, PerLevelGains);
			}
			(void)PerLevelGains;

			const FText NewLine = NSLOCTEXT("T66.Common", "NewLine", "\n");
			const FText StatLineFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");
			TArray<FText> Lines;
			Lines.Reserve(12);
			if (!Quote.IsEmpty())
			{
				Lines.Add(Quote);
				Lines.Add(FText::GetEmpty());
			}
			Lines.Add(Loc ? Loc->GetText_BaseStatsHeader() : NSLOCTEXT("T66.HeroSelection", "BaseStatsHeader", "Base Stats"));

			auto AddLine = [&](const FText& Label, int32 Value)
			{
				Lines.Add(FText::Format(StatLineFmt, Label, FText::AsNumber(Value)));
			};

			AddLine(Loc ? Loc->GetText_Stat_Damage() : NSLOCTEXT("T66.Stats", "Damage", "Damage"), BaseStats.Damage);
			AddLine(Loc ? Loc->GetText_Stat_AttackSpeed() : NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed"), BaseStats.AttackSpeed);
			AddLine(Loc ? Loc->GetText_Stat_AttackSize() : NSLOCTEXT("T66.Stats", "AttackSize", "Attack Size"), BaseStats.AttackSize);
			AddLine(Loc ? Loc->GetText_Stat_Armor() : NSLOCTEXT("T66.Stats", "Armor", "Armor"), BaseStats.Armor);
			AddLine(Loc ? Loc->GetText_Stat_Evasion() : NSLOCTEXT("T66.Stats", "Evasion", "Evasion"), BaseStats.Evasion);
			AddLine(Loc ? Loc->GetText_Stat_Luck() : NSLOCTEXT("T66.Stats", "Luck", "Luck"), BaseStats.Luck);

			HeroDescWidget->SetText(FText::Join(NewLine, Lines));
		}
		// Update 3D preview stage or fallback colored box
		if (AT66HeroPreviewStage* PreviewStage = GetHeroPreviewStage())
		{
			PreviewStage->SetPreviewHero(PreviewedHeroID, SelectedBodyType);
		}
		else if (HeroPreviewColorBox.IsValid())
		{
			HeroPreviewColorBox->SetBorderBackgroundColor(HeroData.PlaceholderColor);
		}
	}
	else
	{
		if (HeroNameWidget.IsValid())
		{
			HeroNameWidget->SetText(Loc ? Loc->GetText_SelectYourHero() : NSLOCTEXT("T66.HeroSelection", "SelectHero", "Select a Hero"));
		}
		if (HeroDescWidget.IsValid())
		{
			HeroDescWidget->SetText(NSLOCTEXT("T66.HeroSelection", "SelectHeroDescriptionHint", "Select a hero to view their description."));
		}
		if (HeroLoreWidget.IsValid())
		{
			HeroLoreWidget->SetText(FText::GetEmpty());
		}
		if (AT66HeroPreviewStage* PreviewStage = GetHeroPreviewStage())
		{
			PreviewStage->SetPreviewHero(PreviewedHeroID, SelectedBodyType);
		}
		else if (HeroPreviewColorBox.IsValid())
		{
			HeroPreviewColorBox->SetBorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.4f, 1.0f));
		}
	}
}

void UT66HeroSelectionScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66HeroSelectionScreen::OnLanguageChanged);
	}
	RefreshHeroList();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		// Apply saved settings before we trigger the first preview render.
		SelectedDifficulty = GI->SelectedDifficulty;
		SelectedBodyType = GI->SelectedHeroBodyType;

		if (!GI->SelectedHeroID.IsNone())
		{
			PreviewHero(GI->SelectedHeroID);
		}
		else if (AllHeroIDs.Num() > 0)
		{
			PreviewHero(AllHeroIDs[0]);
		}
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
	PreviewedHeroID = HeroID;
	CurrentHeroIndex = AllHeroIDs.IndexOfByKey(HeroID);
	if (CurrentHeroIndex == INDEX_NONE) CurrentHeroIndex = 0;
	FHeroData HeroData;
	if (GetPreviewedHeroData(HeroData)) OnPreviewedHeroChanged(HeroData);
	UpdateHeroDisplay();
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
		GI->bStageBoostPending = (SelectedDifficulty != ET66Difficulty::Easy);
	}
	NavigateTo(ET66ScreenType::CompanionSelection);
}
void UT66HeroSelectionScreen::OnHeroLoreClicked() { ShowModal(ET66ScreenType::HeroLore); }
void UT66HeroSelectionScreen::OnTheLabClicked() { UE_LOG(LogTemp, Log, TEXT("The Lab - not implemented")); }
void UT66HeroSelectionScreen::OnEnterTribulationClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedHeroID = PreviewedHeroID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->SelectedHeroBodyType = SelectedBodyType;
		GI->bStageBoostPending = (SelectedDifficulty != ET66Difficulty::Easy);
	}
	if (UIManager) UIManager->HideAllUI();
	UGameplayStatics::OpenLevel(this, FName(TEXT("GameplayLevel")));
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
	UTextureRenderTarget2D* RenderTarget = PreviewStage ? PreviewStage->GetRenderTarget() : nullptr;

	if (RenderTarget)
	{
		// 3D preview: use render target as image
		HeroPreviewBrush = MakeShared<FSlateBrush>();
		HeroPreviewBrush->SetResourceObject(RenderTarget);
		// Keep a reasonable desired size; SScaleBox below handles actual layout.
		HeroPreviewBrush->ImageSize = FVector2D(512.f, 720.f);
		HeroPreviewBrush->DrawAs = ESlateBrushDrawType::Image;
		HeroPreviewBrush->Tiling = ESlateBrushTileType::NoTile;

		return SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(ST66DragRotatePreview)
				.Stage(PreviewStage)
				.Brush(HeroPreviewBrush.Get())
				.DegreesPerPixel(0.28f)
			];
	}

	// Fallback: colored box when no preview stage in level
	return SAssignNew(HeroPreviewColorBox, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FallbackColor)
		[
			SNew(SBox)
		];
}

void UT66HeroSelectionScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	TakeWidget();
}
