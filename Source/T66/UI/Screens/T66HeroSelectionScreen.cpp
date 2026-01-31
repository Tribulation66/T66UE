// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
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
				// Dota-style orbit: horizontal rotates yaw, vertical tilts camera.
				Stage->AddPreviewOrbit(Delta.X * DegreesPerPixel, -Delta.Y * DegreesPerPixel);
			}
			return FReply::Handled();
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

	// Create placeholder skins - DisplayName will be retrieved from localization via SkinID
	FSkinData DefaultSkin;
	DefaultSkin.SkinID = FName(TEXT("Default"));
	DefaultSkin.bIsDefault = true;
	DefaultSkin.bIsOwned = true;
	DefaultSkin.bIsEquipped = true;
	DefaultSkin.CoinCost = 0;
	PlaceholderSkins.Add(DefaultSkin);

	FSkinData GoldenSkin;
	GoldenSkin.SkinID = FName(TEXT("Golden"));
	GoldenSkin.bIsDefault = false;
	GoldenSkin.bIsOwned = true;
	GoldenSkin.bIsEquipped = false;
	GoldenSkin.CoinCost = 5000;
	PlaceholderSkins.Add(GoldenSkin);

	FSkinData ShadowSkin;
	ShadowSkin.SkinID = FName(TEXT("Shadow"));
	ShadowSkin.bIsDefault = false;
	ShadowSkin.bIsOwned = false;
	ShadowSkin.bIsEquipped = false;
	ShadowSkin.CoinCost = 7500;
	PlaceholderSkins.Add(ShadowSkin);

	FSkinData InfernalSkin;
	InfernalSkin.SkinID = FName(TEXT("Infernal"));
	InfernalSkin.bIsDefault = false;
	InfernalSkin.bIsOwned = false;
	InfernalSkin.bIsEquipped = false;
	InfernalSkin.CoinCost = 10000;
	PlaceholderSkins.Add(InfernalSkin);

	FSkinData FrostSkin;
	FrostSkin.SkinID = FName(TEXT("Frost"));
	FrostSkin.bIsDefault = false;
	FrostSkin.bIsOwned = false;
	FrostSkin.bIsEquipped = false;
	FrostSkin.CoinCost = 8000;
	PlaceholderSkins.Add(FrostSkin);

	FSkinData VoidSkin;
	VoidSkin.SkinID = FName(TEXT("Void"));
	VoidSkin.bIsDefault = false;
	VoidSkin.bIsOwned = false;
	VoidSkin.bIsEquipped = false;
	VoidSkin.CoinCost = 9000;
	PlaceholderSkins.Add(VoidSkin);

	FSkinData PhoenixSkin;
	PhoenixSkin.SkinID = FName(TEXT("Phoenix"));
	PhoenixSkin.bIsDefault = false;
	PhoenixSkin.bIsOwned = true;
	PhoenixSkin.bIsEquipped = false;
	PhoenixSkin.CoinCost = 12000;
	PlaceholderSkins.Add(PhoenixSkin);

	FSkinData CosmicSkin;
	CosmicSkin.SkinID = FName(TEXT("Cosmic"));
	CosmicSkin.bIsDefault = false;
	CosmicSkin.bIsOwned = false;
	CosmicSkin.bIsEquipped = false;
	CosmicSkin.CoinCost = 15000;
	PlaceholderSkins.Add(CosmicSkin);

	FSkinData NeonSkin;
	NeonSkin.SkinID = FName(TEXT("Neon"));
	NeonSkin.bIsDefault = false;
	NeonSkin.bIsOwned = false;
	NeonSkin.bIsEquipped = false;
	NeonSkin.CoinCost = 6000;
	PlaceholderSkins.Add(NeonSkin);

	FSkinData PrimalSkin;
	PrimalSkin.SkinID = FName(TEXT("Primal"));
	PrimalSkin.bIsDefault = false;
	PrimalSkin.bIsOwned = false;
	PrimalSkin.bIsEquipped = false;
	PrimalSkin.CoinCost = 11000;
	PlaceholderSkins.Add(PrimalSkin);

	FSkinData CelestialSkin;
	CelestialSkin.SkinID = FName(TEXT("Celestial"));
	CelestialSkin.bIsDefault = false;
	CelestialSkin.bIsOwned = false;
	CelestialSkin.bIsEquipped = false;
	CelestialSkin.CoinCost = 20000;
	PlaceholderSkins.Add(CelestialSkin);

	FSkinData ObsidianSkin;
	ObsidianSkin.SkinID = FName(TEXT("Obsidian"));
	ObsidianSkin.bIsDefault = false;
	ObsidianSkin.bIsOwned = false;
	ObsidianSkin.bIsEquipped = false;
	ObsidianSkin.CoinCost = 18000;
	PlaceholderSkins.Add(ObsidianSkin);
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
	FText EquipText = Loc ? Loc->GetText_Equip() : NSLOCTEXT("T66.Common", "Equip", "EQUIP");

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

	// Build skins list with localized names
	TSharedRef<SVerticalBox> SkinsListBox = SNew(SVerticalBox);
	for (const FSkinData& Skin : PlaceholderSkins)
	{
		FLinearColor RowBg = Skin.bIsEquipped ? FT66Style::Tokens::Accent2 : FT66Style::Tokens::Panel2;
		
		// Get localized skin name
		FText SkinDisplayName = Loc ? Loc->GetText_SkinName(Skin.SkinID) : FText::FromName(Skin.SkinID);

		SkinsListBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 3.0f)
		[
			SNew(SBorder)
			.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
			.BorderBackgroundColor(RowBg)
			.Padding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(SkinDisplayName)
					.Font(FT66Style::Tokens::FontRegular(13))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(5.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(90.0f).HeightOverride(28.0f)
					[
						SNew(SButton)
						.HAlign(HAlign_Center).VAlign(VAlign_Center)
						.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
						.ButtonColorAndOpacity(Skin.bIsOwned ? FT66Style::Tokens::Success : FT66Style::Tokens::Accent)
						[
							SNew(STextBlock)
							.Text(Skin.bIsOwned ? EquipText : BuyText)
							.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip"))
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
		int32 HeroIdx = (CurrentHeroIndex + Offset + AllHeroIDs.Num()) % AllHeroIDs.Num();
		if (AllHeroIDs.Num() == 0) break;
		
		FHeroData HeroData;
		FLinearColor SpriteColor = FLinearColor(0.2f, 0.2f, 0.25f, 1.0f);
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (GI->GetHeroData(AllHeroIDs[HeroIdx], HeroData))
			{
				SpriteColor = HeroData.PlaceholderColor;
			}
		}
		
		float BoxSize = (Offset == 0) ? 60.0f : 45.0f; // Current hero is larger
		float Opacity = (Offset == 0) ? 1.0f : 0.6f;
		
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
				.BorderBackgroundColor(SpriteColor * Opacity)
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
					// Hero Grid Button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 20.0f, 0.0f)
					[
						SNew(SBox).MinDesiredWidth(150.0f).HeightOverride(40.0f)
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
					// Hero Sprite Carousel with navigation
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f)
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
					.Padding(0.0f, 0.0f, 10.0f, 0.0f)
					[
						SNew(SBorder)
						.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
						.Padding(FMargin(FT66Style::Tokens::Space3))
						[
							SNew(SVerticalBox)
							// Skins header
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.0f, 0.0f, 0.0f, 10.0f)
							[
								SNew(STextBlock).Text(SkinsText)
								.TextStyle(&TxtHeading)
							]
							// Skins list
							+ SVerticalBox::Slot()
							.FillHeight(1.0f)
							[
								SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									SkinsListBox
								]
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
							// Choose Companion button below preview (wide enough for full text)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.0f, 20.0f, 0.0f, 0.0f)
							[
								SNew(SBox).MinDesiredWidth(300.0f).HeightOverride(45.0f)
								[
									SNew(SButton)
									.HAlign(HAlign_Center).VAlign(VAlign_Center)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleCompanionClicked))
									.ButtonStyle(&BtnPrimary)
									.ButtonColorAndOpacity(FT66Style::Tokens::Accent2)
									.ContentPadding(FMargin(16.f, 10.f))
									[
										SNew(STextBlock).Text(ChooseCompanionText)
										.TextStyle(&TxtButton)
									]
								]
							]
						]
						// Body Type: TYPE A and TYPE B buttons (only one selected)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(0.0f, 10.0f, 0.0f, 0.0f)
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
						]
					]
					// RIGHT PANEL: Hero Info (same width as left)
					+ SHorizontalBox::Slot()
					.FillWidth(0.28f)
					.Padding(10.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SBorder)
						.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
						.Padding(FMargin(FT66Style::Tokens::Space4))
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
											SNew(STextBlock).Text(LoreText)
											.TextStyle(&TxtChip)
										]
									]
								]
							]
							// Video placeholder
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 0.0f, 0.0f, 12.0f)
							[
								SNew(SBorder)
								.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
								.Padding(FMargin(5.0f))
								[
									SNew(SBox).HeightOverride(140.0f)
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
							// Hero Description (localized blurb below medals)
							+ SVerticalBox::Slot()
							.FillHeight(1.0f)
							.Padding(0.0f, 0.0f, 0.0f, 0.0f)
							[
								SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									SAssignNew(HeroDescWidget, STextBlock)
									.Text(NSLOCTEXT("T66.HeroSelection", "SelectHeroDescriptionHint", "Select a hero to view their description."))
									.Font(FT66Style::Tokens::FontRegular(12))
									.ColorAndOpacity(FT66Style::Tokens::TextMuted)
									.AutoWrapText(true)
								]
							]
						]
					]
				]
				// === BOTTOM BAR: The Lab + Difficulty Dropdown + Enter Button ===
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(20.0f, 10.0f, 20.0f, 20.0f)
				[
					SNew(SHorizontalBox)
					// The Lab button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(10.0f, 0.0f)
					[
						SNew(SBox).MinDesiredWidth(140.0f).HeightOverride(50.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center).VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTheLabClicked))
							.ButtonStyle(&BtnNeutral)
							.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
							.ContentPadding(FMargin(16.f, 10.f))
							[
								SNew(STextBlock).Text(TheLabText)
								.TextStyle(&TxtButton)
							]
						]
					]
					// Difficulty Dropdown
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(10.0f, 0.0f)
					[
						SNew(SBox).MinDesiredWidth(170.0f).HeightOverride(40.0f)
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
					// Enter the Tribulation button (wide enough for full text)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(10.0f, 0.0f)
					[
						SNew(SBox).WidthOverride(340.0f).HeightOverride(50.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center).VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleEnterClicked))
							.ButtonStyle(&BtnDanger)
							.ButtonColorAndOpacity(FT66Style::Tokens::Danger)
							.ContentPadding(FMargin(18.f, 10.f))
							[
								SNew(STextBlock).Text(EnterText)
								.TextStyle(&TxtButton)
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
FReply UT66HeroSelectionScreen::HandleLoreClicked() { OnHeroLoreClicked(); return FReply::Handled(); }
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
		if (HeroDescWidget.IsValid())
		{
			HeroDescWidget->SetText(Loc ? Loc->GetText_HeroDescription(PreviewedHeroID) : HeroData.Description);
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
