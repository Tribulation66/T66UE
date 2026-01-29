// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Images/SImage.h"
#include "Engine/TextureRenderTarget2D.h"

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
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
		.ColorAndOpacity(FLinearColor::White);
}

TSharedRef<SWidget> UT66HeroSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	GeneratePlaceholderSkins();

	// Get localized text
	FText HeroGridText = Loc ? Loc->GetText_HeroGrid() : FText::FromString(TEXT("HERO GRID"));
	FText ChooseCompanionText = Loc ? Loc->GetText_ChooseCompanion() : FText::FromString(TEXT("CHOOSE COMPANION"));
	FText SkinsText = Loc ? Loc->GetText_Skins() : FText::FromString(TEXT("SKINS"));
	FText HeroInfoText = Loc ? Loc->GetText_HeroInfo() : FText::FromString(TEXT("HERO INFO"));
	FText LoreText = Loc ? Loc->GetText_Lore() : FText::FromString(TEXT("LORE"));
	FText TheLabText = Loc ? Loc->GetText_TheLab() : FText::FromString(TEXT("THE LAB"));
	FText EnterText = Loc ? Loc->GetText_EnterTheTribulation() : FText::FromString(TEXT("ENTER THE TRIBULATION"));
	FText BackText = Loc ? Loc->GetText_Back() : FText::FromString(TEXT("BACK"));
	FText BuyText = Loc ? Loc->GetText_Buy() : FText::FromString(TEXT("BUY"));
	FText EquipText = Loc ? Loc->GetText_Equip() : FText::FromString(TEXT("EQUIP"));

	// Initialize difficulty dropdown options
	DifficultyOptions.Empty();
	TArray<ET66Difficulty> Difficulties = {
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard,
		ET66Difficulty::VeryHard, ET66Difficulty::Impossible, ET66Difficulty::Perdition, ET66Difficulty::Final
	};
	for (ET66Difficulty Diff : Difficulties)
	{
		FText DiffText = Loc ? Loc->GetText_Difficulty(Diff) : FText::FromString(TEXT("???"));
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
		FLinearColor RowBg = Skin.bIsEquipped 
			? FLinearColor(0.2f, 0.4f, 0.3f, 1.0f) 
			: FLinearColor(0.1f, 0.1f, 0.15f, 1.0f);
		
		// Get localized skin name
		FText SkinDisplayName = Loc ? Loc->GetText_SkinName(Skin.SkinID) : FText::FromName(Skin.SkinID);

		SkinsListBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 3.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(RowBg)
			.Padding(FMargin(10.0f, 8.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(SkinDisplayName)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(5.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SBox).WidthOverride(78.0f).HeightOverride(28.0f)
					[
						SNew(SButton)
						.HAlign(HAlign_Center).VAlign(VAlign_Center)
						.ButtonColorAndOpacity(Skin.bIsOwned ? FLinearColor(0.2f, 0.5f, 0.3f, 1.0f) : FLinearColor(0.5f, 0.4f, 0.1f, 1.0f))
						[
							SNew(STextBlock)
							.Text(Skin.bIsOwned ? EquipText : BuyText)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
							.ColorAndOpacity(FLinearColor::White)
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

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.0f))
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
						SNew(SBox).WidthOverride(130.0f).HeightOverride(40.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center).VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleHeroGridClicked))
							.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
							[
								SNew(STextBlock).Text(HeroGridText)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
								.ColorAndOpacity(FLinearColor::White)
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
								.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
								[
									SNew(STextBlock).Text(FText::FromString(TEXT("<")))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
									.ColorAndOpacity(FLinearColor::White)
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
								.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
								[
									SNew(STextBlock).Text(FText::FromString(TEXT(">")))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
									.ColorAndOpacity(FLinearColor::White)
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
						.BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.18f, 1.0f))
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.Padding(FMargin(10.0f))
						[
							SNew(SVerticalBox)
							// Skins header
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.0f, 0.0f, 0.0f, 10.0f)
							[
								SNew(STextBlock).Text(SkinsText)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
								.ColorAndOpacity(FLinearColor::White)
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
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.FillHeight(1.0f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								CreateHeroPreviewWidget(HeroPreviewColor)
							]
							// Choose Companion button below preview (wide enough for full text)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.0f, 20.0f, 0.0f, 0.0f)
							[
								SNew(SBox).WidthOverride(280.0f).HeightOverride(45.0f)
								[
									SNew(SButton)
									.HAlign(HAlign_Center).VAlign(VAlign_Center)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleCompanionClicked))
									.ButtonColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.5f, 1.0f))
									[
										SNew(STextBlock).Text(ChooseCompanionText)
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
										.ColorAndOpacity(FLinearColor::White)
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
											? FLinearColor(0.4f, 0.5f, 0.7f, 1.0f)
											: FLinearColor(0.15f, 0.15f, 0.2f, 1.0f);
									})
									[
										SNew(STextBlock)
										.Text(Loc ? Loc->GetText_BodyTypeA() : FText::FromString(TEXT("TYPE A")))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
										.ColorAndOpacity(FLinearColor::White)
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
											? FLinearColor(0.4f, 0.5f, 0.7f, 1.0f)
											: FLinearColor(0.15f, 0.15f, 0.2f, 1.0f);
									})
									[
										SNew(STextBlock)
										.Text(Loc ? Loc->GetText_BodyTypeB() : FText::FromString(TEXT("TYPE B")))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
										.ColorAndOpacity(FLinearColor::White)
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
						.BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.18f, 1.0f))
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.Padding(FMargin(15.0f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.0f, 0.0f, 0.0f, 8.0f)
							[
								SNew(STextBlock).Text(HeroInfoText)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
								.ColorAndOpacity(FLinearColor::White)
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
									.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.0f))
									.Padding(FMargin(12.0f, 8.0f))
									[
										SAssignNew(HeroNameWidget, STextBlock)
										.Text(FText::FromString(TEXT("Select a Hero")))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
										.ColorAndOpacity(FLinearColor::White)
										.Justification(ETextJustify::Center)
									]
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(8.0f, 0.0f, 0.0f, 0.0f)
								[
									SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f)
									[
										SNew(SButton)
										.HAlign(HAlign_Center).VAlign(VAlign_Center)
										.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleLoreClicked))
										.ButtonColorAndOpacity(FLinearColor(0.2f, 0.15f, 0.1f, 1.0f))
										[
											SNew(STextBlock).Text(LoreText)
											.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
											.ColorAndOpacity(FLinearColor::White)
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
								.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.05f, 1.0f))
								.Padding(FMargin(5.0f))
								[
									SNew(SBox).HeightOverride(140.0f)
									[
										SNew(STextBlock)
										.Text(FText::FromString(TEXT("[VIDEO PREVIEW]")))
										.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
										.ColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f))
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
									.Text(FText::FromString(TEXT("Select a hero to view their description.")))
									.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
									.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
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
						SNew(SBox).WidthOverride(120.0f).HeightOverride(50.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center).VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTheLabClicked))
							.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
							[
								SNew(STextBlock).Text(TheLabText)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
					// Difficulty Dropdown
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(10.0f, 0.0f)
					[
						SNew(SBox).WidthOverride(140.0f).HeightOverride(40.0f)
						[
							SNew(SComboBox<TSharedPtr<FString>>)
							.OptionsSource(&DifficultyOptions)
							.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo) {
								OnDifficultyChanged(NewValue, SelectInfo);
							})
							.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem) -> TSharedRef<SWidget> {
								return SNew(STextBlock)
									.Text(FText::FromString(*InItem))
									.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
									.ColorAndOpacity(FLinearColor::White);
							})
							.InitiallySelectedItem(CurrentDifficultyOption)
							[
								SNew(STextBlock)
								.Text_Lambda([this]() -> FText {
									return CurrentDifficultyOption.IsValid() ? FText::FromString(*CurrentDifficultyOption) : FText::FromString(TEXT("Easy"));
								})
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
								.ColorAndOpacity(FLinearColor::White)
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
							.ButtonColorAndOpacity(FLinearColor(0.6f, 0.2f, 0.1f, 1.0f))
							[
								SNew(STextBlock).Text(EnterText)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
								.ColorAndOpacity(FLinearColor::White)
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
				SNew(SBox).WidthOverride(100.0f).HeightOverride(40.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center).VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBackClicked))
					.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
					[
						SNew(STextBlock).Text(BackText)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FLinearColor::White)
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
			HeroNameWidget->SetText(FText::FromString(TEXT("Select a Hero")));
		}
		if (HeroDescWidget.IsValid())
		{
			HeroDescWidget->SetText(FText::FromString(TEXT("Select a hero to view their description.")));
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
		if (!GI->SelectedHeroID.IsNone())
		{
			PreviewHero(GI->SelectedHeroID);
		}
		else if (AllHeroIDs.Num() > 0)
		{
			PreviewHero(AllHeroIDs[0]);
		}
		SelectedDifficulty = GI->SelectedDifficulty;
		SelectedBodyType = GI->SelectedHeroBodyType;
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
		HeroPreviewBrush->ImageSize = FVector2D(250.f, 350.f);
		HeroPreviewBrush->DrawAs = ESlateBrushDrawType::Image;
		HeroPreviewBrush->Tiling = ESlateBrushTileType::NoTile;

		return SNew(SBox)
			.WidthOverride(250.0f)
			.HeightOverride(350.0f)
			[
				SNew(SImage)
				.Image(HeroPreviewBrush.Get())
			];
	}

	// Fallback: colored box when no preview stage in level
	return SAssignNew(HeroPreviewColorBox, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FallbackColor)
		[
			SNew(SBox)
			.WidthOverride(250.0f)
			.HeightOverride(350.0f)
		];
}

void UT66HeroSelectionScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	TakeWidget();
}
