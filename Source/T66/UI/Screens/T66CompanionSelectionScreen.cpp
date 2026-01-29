// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66CompanionSelectionScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"

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

void UT66CompanionSelectionScreen::GeneratePlaceholderSkins()
{
	PlaceholderSkins.Empty();

	FSkinData DefaultSkin;
	DefaultSkin.SkinID = FName(TEXT("Default"));
	DefaultSkin.DisplayName = FText::FromString(TEXT("Default"));
	DefaultSkin.bIsDefault = true;
	DefaultSkin.bIsOwned = true;
	DefaultSkin.bIsEquipped = true;
	DefaultSkin.CoinCost = 0;
	PlaceholderSkins.Add(DefaultSkin);

	FSkinData AngelicSkin;
	AngelicSkin.SkinID = FName(TEXT("Angelic"));
	AngelicSkin.DisplayName = FText::FromString(TEXT("Angelic Grace"));
	AngelicSkin.bIsDefault = false;
	AngelicSkin.bIsOwned = true;
	AngelicSkin.bIsEquipped = false;
	AngelicSkin.CoinCost = 3000;
	PlaceholderSkins.Add(AngelicSkin);

	FSkinData DemonicSkin;
	DemonicSkin.SkinID = FName(TEXT("Demonic"));
	DemonicSkin.DisplayName = FText::FromString(TEXT("True Form"));
	DemonicSkin.bIsDefault = false;
	DemonicSkin.bIsOwned = false;
	DemonicSkin.bIsEquipped = false;
	DemonicSkin.CoinCost = 6000;
	PlaceholderSkins.Add(DemonicSkin);
}

TSharedRef<SWidget> UT66CompanionSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	GeneratePlaceholderSkins();

	// Get localized text
	FText TitleText = Loc ? Loc->GetText_SelectCompanion() : FText::FromString(TEXT("SELECT COMPANION"));
	FText CompanionGridText = Loc ? Loc->GetText_CompanionGrid() : FText::FromString(TEXT("COMPANION GRID"));
	FText NoCompanionText = Loc ? Loc->GetText_NoCompanion() : FText::FromString(TEXT("NO COMPANION"));
	FText SkinsText = Loc ? Loc->GetText_Skins() : FText::FromString(TEXT("SKINS"));
	FText LoreText = Loc ? Loc->GetText_Lore() : FText::FromString(TEXT("LORE"));
	FText ConfirmText = Loc ? Loc->GetText_ConfirmCompanion() : FText::FromString(TEXT("CONFIRM COMPANION"));
	FText BackText = Loc ? Loc->GetText_Back() : FText::FromString(TEXT("BACK"));
	FText BuyText = Loc ? Loc->GetText_Buy() : FText::FromString(TEXT("BUY"));
	FText EquipText = Loc ? Loc->GetText_Equip() : FText::FromString(TEXT("EQUIP"));

	// Build skins list
	TSharedRef<SVerticalBox> SkinsListBox = SNew(SVerticalBox);
	for (const FSkinData& Skin : PlaceholderSkins)
	{
		FLinearColor RowBg = Skin.bIsEquipped 
			? FLinearColor(0.2f, 0.4f, 0.3f, 1.0f) 
			: FLinearColor(0.1f, 0.1f, 0.15f, 1.0f);

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
					.Text(Skin.DisplayName)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(5.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SBox).WidthOverride(60.0f).HeightOverride(28.0f)
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

	// Determine current display name
	FText CurrentCompanionName = NoCompanionText;
	FText CurrentCompanionLore = FText::FromString(TEXT("Selecting no companion means you face the tribulation alone."));
	if (!PreviewedCompanionID.IsNone())
	{
		FCompanionData Data;
		if (GetPreviewedCompanionData(Data))
		{
			CurrentCompanionName = Loc ? Loc->GetCompanionDisplayName(Data) : Data.DisplayName;
			CurrentCompanionLore = Data.LoreText;
		}
	}

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.0f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				// === TOP BAR: Companion Belt + Companion Grid Button ===
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(20.0f, 15.0f, 20.0f, 10.0f)
				[
					SNew(SHorizontalBox)
					// Companion Grid Button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 20.0f, 0.0f)
					[
						SNew(SBox).WidthOverride(160.0f).HeightOverride(40.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center).VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleCompanionGridClicked))
							.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
							[
								SNew(STextBlock).Text(CompanionGridText)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
					// Companion Belt (navigation arrows + current companion)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f)
						[
							SNew(SBox).WidthOverride(50.0f).HeightOverride(50.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandlePrevClicked))
								.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
								[
									SNew(STextBlock).Text(FText::FromString(TEXT("<")))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SBorder)
							.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.25f, 1.0f))
							.Padding(FMargin(20.0f, 10.0f))
							[
								SAssignNew(CompanionNameWidget, STextBlock)
								.Text(CurrentCompanionName)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
								.ColorAndOpacity(FLinearColor::White)
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f)
						[
							SNew(SBox).WidthOverride(50.0f).HeightOverride(50.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleNextClicked))
								.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
								[
									SNew(STextBlock).Text(FText::FromString(TEXT(">")))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
					]
					// No Companion Button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(20.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SBox).WidthOverride(150.0f).HeightOverride(40.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center).VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleNoCompanionClicked))
							.ButtonColorAndOpacity(PreviewedCompanionID.IsNone() ? FLinearColor(0.5f, 0.3f, 0.3f, 1.0f) : FLinearColor(0.3f, 0.2f, 0.2f, 1.0f))
							[
								SNew(STextBlock).Text(NoCompanionText)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
				]
				// === MAIN CONTENT: Left Panel | Center Preview | Right Panel ===
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(20.0f, 10.0f)
				[
					SNew(SHorizontalBox)
					// LEFT PANEL: Skins
					+ SHorizontalBox::Slot()
					.FillWidth(0.22f)
					.Padding(0.0f, 0.0f, 10.0f, 0.0f)
					[
						SNew(SBorder)
						.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.08f, 1.0f))
						.Padding(FMargin(10.0f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.0f, 0.0f, 0.0f, 10.0f)
							[
								SNew(STextBlock).Text(SkinsText)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
								.ColorAndOpacity(FLinearColor::White)
							]
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
					// CENTER: Companion Preview
					+ SHorizontalBox::Slot()
					.FillWidth(0.46f)
					.Padding(10.0f, 0.0f)
					[
						SNew(SVerticalBox)
						// Preview Area (placeholder)
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SBorder)
							.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.1f, 1.0f))
							.Padding(FMargin(40.0f))
							[
								SNew(STextBlock)
								.Text(PreviewedCompanionID.IsNone() 
									? FText::FromString(TEXT("[NO COMPANION SELECTED]"))
									: FText::FromString(TEXT("[3D COMPANION PREVIEW]")))
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
								.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f))
							]
						]
						// Body Type Toggle
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(0.0f, 10.0f, 0.0f, 0.0f)
						[
							SNew(SBox).WidthOverride(150.0f).HeightOverride(40.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleBodyTypeClicked))
								.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
								.IsEnabled(!PreviewedCompanionID.IsNone())
								[
									SAssignNew(BodyTypeWidget, STextBlock)
									.Text(Loc ? Loc->GetText_BodyTypeA() : FText::FromString(TEXT("TYPE A")))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
					]
					// RIGHT PANEL: Companion Info
					+ SHorizontalBox::Slot()
					.FillWidth(0.32f)
					.Padding(10.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SBorder)
						.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.08f, 1.0f))
						.Padding(FMargin(15.0f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.0f, 0.0f, 0.0f, 15.0f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("COMPANION INFO")))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
								.ColorAndOpacity(FLinearColor::White)
							]
							// Lore/Description
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 0.0f, 0.0f, 15.0f)
							[
								SAssignNew(CompanionLoreWidget, STextBlock)
								.Text(CurrentCompanionLore)
								.Font(FCoreStyle::GetDefaultFontStyle("Italic", 12))
								.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
								.AutoWrapText(true)
							]
							// Passive effect info
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 0.0f, 0.0f, 15.0f)
							[
								SNew(SBorder)
								.BorderBackgroundColor(FLinearColor(0.1f, 0.15f, 0.1f, 1.0f))
								.Padding(FMargin(10.0f))
								[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT("Passive: Heals the player during combat")))
									.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
									.ColorAndOpacity(FLinearColor(0.6f, 0.9f, 0.6f, 1.0f))
									.AutoWrapText(true)
								]
							]
							// Medals placeholder (3 medals for companions)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.0f, 0.0f, 0.0f, 15.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)
								[
									SNew(SBox).WidthOverride(35.0f).HeightOverride(35.0f)
									[
										SNew(SBorder).BorderBackgroundColor(FLinearColor(0.2f, 0.5f, 0.2f, 1.0f)) // Easy
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)
								[
									SNew(SBox).WidthOverride(35.0f).HeightOverride(35.0f)
									[
										SNew(SBorder).BorderBackgroundColor(FLinearColor(0.5f, 0.4f, 0.1f, 1.0f)) // Medium
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)
								[
									SNew(SBox).WidthOverride(35.0f).HeightOverride(35.0f)
									[
										SNew(SBorder).BorderBackgroundColor(FLinearColor(0.5f, 0.15f, 0.15f, 1.0f)) // Hard
									]
								]
							]
							// Lore button
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							[
								SNew(SBox).WidthOverride(100.0f).HeightOverride(35.0f)
								[
									SNew(SButton)
									.HAlign(HAlign_Center).VAlign(VAlign_Center)
									.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleLoreClicked))
									.ButtonColorAndOpacity(FLinearColor(0.2f, 0.15f, 0.1f, 1.0f))
									.IsEnabled(!PreviewedCompanionID.IsNone())
									[
										SNew(STextBlock).Text(LoreText)
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
										.ColorAndOpacity(FLinearColor::White)
									]
								]
							]
							+ SVerticalBox::Slot().FillHeight(1.0f)
						]
					]
				]
				// === BOTTOM BAR: Confirm Button ===
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(20.0f, 10.0f, 20.0f, 20.0f)
				[
					SNew(SBox).WidthOverride(250.0f).HeightOverride(55.0f)
					[
						SNew(SButton)
						.HAlign(HAlign_Center).VAlign(VAlign_Center)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleConfirmClicked))
						.ButtonColorAndOpacity(FLinearColor(0.2f, 0.5f, 0.2f, 1.0f))
						[
							SNew(STextBlock).Text(ConfirmText)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
							.ColorAndOpacity(FLinearColor::White)
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
					.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleBackClicked))
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

FReply UT66CompanionSelectionScreen::HandlePrevClicked() { PreviewPreviousCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleNextClicked() { PreviewNextCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleCompanionGridClicked() { OnCompanionGridClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleNoCompanionClicked() { SelectNoCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleLoreClicked() { OnCompanionLoreClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleConfirmClicked() { OnConfirmCompanionClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

FReply UT66CompanionSelectionScreen::HandleBodyTypeClicked()
{
	ToggleBodyType();
	if (BodyTypeWidget.IsValid())
	{
		UT66LocalizationSubsystem* Loc = GetLocSubsystem();
		FText TypeText = (SelectedBodyType == ET66BodyType::TypeA)
			? (Loc ? Loc->GetText_BodyTypeA() : FText::FromString(TEXT("TYPE A")))
			: (Loc ? Loc->GetText_BodyTypeB() : FText::FromString(TEXT("TYPE B")));
		BodyTypeWidget->SetText(TypeText);
	}
	return FReply::Handled();
}

void UT66CompanionSelectionScreen::UpdateCompanionDisplay()
{
	if (CompanionNameWidget.IsValid())
	{
		if (PreviewedCompanionID.IsNone())
		{
			UT66LocalizationSubsystem* Loc = GetLocSubsystem();
			CompanionNameWidget->SetText(Loc ? Loc->GetText_NoCompanion() : FText::FromString(TEXT("No Companion")));
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
			CompanionLoreWidget->SetText(FText::FromString(TEXT("Selecting no companion means you face the tribulation alone.")));
		}
		else
		{
			FCompanionData Data;
			if (GetPreviewedCompanionData(Data))
				CompanionLoreWidget->SetText(Data.LoreText);
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
		if (!GI->SelectedCompanionID.IsNone())
			PreviewCompanion(GI->SelectedCompanionID);
		else
			SelectNoCompanion();
		SelectedBodyType = GI->SelectedCompanionBodyType;
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
	CurrentCompanionIndex = ID.IsNone() ? -1 : AllCompanionIDs.IndexOfByKey(ID);
	if (CurrentCompanionIndex == INDEX_NONE) CurrentCompanionIndex = -1;
	FCompanionData Data;
	bool bIsNoCompanion = ID.IsNone();
	if (!bIsNoCompanion) GetPreviewedCompanionData(Data);
	OnPreviewedCompanionChanged(Data, bIsNoCompanion);
	UpdateCompanionDisplay();
	// Rebuild UI to update button states
	TakeWidget();
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

void UT66CompanionSelectionScreen::ToggleBodyType() { SelectedBodyType = (SelectedBodyType == ET66BodyType::TypeA) ? ET66BodyType::TypeB : ET66BodyType::TypeA; }
void UT66CompanionSelectionScreen::OnCompanionGridClicked() { ShowModal(ET66ScreenType::CompanionGrid); }
void UT66CompanionSelectionScreen::OnCompanionLoreClicked() { if (!PreviewedCompanionID.IsNone()) ShowModal(ET66ScreenType::CompanionLore); }
void UT66CompanionSelectionScreen::OnConfirmCompanionClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedCompanionID = PreviewedCompanionID;
		GI->SelectedCompanionBodyType = SelectedBodyType;
	}
	NavigateBack();
}
void UT66CompanionSelectionScreen::OnBackClicked() { NavigateBack(); }

void UT66CompanionSelectionScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	TakeWidget();
}
