// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66WeaponAltarOverlayWidget.h"

#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Core/T66WeaponManagerSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66WeaponAltar.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66FlatStyle.h"

#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	UT66WeaponManagerSubsystem* GetWeaponManager(UWorld* World)
	{
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UT66WeaponManagerSubsystem>() : nullptr;
	}

	const FSlateBrush* WhiteBrush()
	{
		return FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	}

	TSharedRef<SWidget> MakeWeaponAltarButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		TSharedPtr<SWidget>& OutButton,
		TSharedPtr<SBorder>& OutBackground,
		TSharedPtr<STextBlock>& OutText,
		const float MinWidth,
		const FName Tag)
	{
		OutBackground.Reset();
		TSharedPtr<STextBlock> TextWidget;
		TSharedRef<SWidget> Button = FT66FlatStyle::MakeFlatToggleGroupButton(
			ET66FlatState::Default,
			SAssignNew(TextWidget, STextBlock)
				.Text(Label)
				.Font(FT66FlatStyle::MakeBoldFont(16))
				.ColorAndOpacity(FT66FlatStyle::PrimaryText())
				.Justification(ETextJustify::Center),
			OnClicked,
			FMargin(16.f, 10.f),
			MinWidth,
			46.f,
			true,
			Tag);

		OutButton = Button;
		OutText = TextWidget;
		return Button;
	}

	void SetWeaponAltarButtonState(
		const TSharedPtr<SWidget>& Button,
		const TSharedPtr<SBorder>& Background,
		const TSharedPtr<STextBlock>& Text,
		const bool bEnabled,
		const bool bSelected = false)
	{
		if (Button.IsValid())
		{
			Button->SetEnabled(bEnabled);
		}
		if (Background.IsValid())
		{
			Background->SetBorderBackgroundColor(
				bEnabled
					? (bSelected ? FT66FlatStyle::SelectedBorder() : FT66FlatStyle::DefaultBorder())
					: FT66FlatStyle::DisabledBorder());
		}
		if (Text.IsValid())
		{
			Text->SetColorAndOpacity(
				bEnabled
					? (bSelected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText())
					: FT66FlatStyle::DisabledText());
		}
	}
}

void UT66WeaponAltarOverlayWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		if (UT66WeaponManagerSubsystem* WeaponManager = GetWeaponManager(World))
		{
			WeaponManager->OnWeaponStateChanged.RemoveAll(this);
		}
	}

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && !PC->IsPaused())
		{
			PC->RestoreGameplayInputMode();
		}
	}

	Super::NativeDestruct();
}

bool UT66WeaponAltarOverlayWidget::HasSelectionsRemaining() const
{
	const AT66WeaponAltar* Altar = SourceAltar.Get();
	return !Altar || Altar->RemainingSelections > 0;
}

void UT66WeaponAltarOverlayWidget::ConsumeSelectionBudget()
{
	if (AT66WeaponAltar* Altar = SourceAltar.Get())
	{
		Altar->RemainingSelections = FMath::Max(0, Altar->RemainingSelections - 1);
	}
}

void UT66WeaponAltarOverlayWidget::HandleWeaponsChanged()
{
	RefreshOffers();
}

TSharedRef<SWidget> UT66WeaponAltarOverlayWidget::RebuildWidget()
{
	UWorld* World = GetWorld();
	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66WeaponManagerSubsystem* WeaponManager = GetWeaponManager(World);
	const AT66WeaponAltar* Altar = SourceAltar.Get();

	if (WeaponManager && GI)
	{
		WeaponManager->BuildWeaponOffers(GI->SelectedHeroID, Altar ? Altar->WeaponOfferRarity : ET66WeaponRarity::Black);
		WeaponManager->OnWeaponStateChanged.RemoveAll(this);
		WeaponManager->OnWeaponStateChanged.AddUObject(this, &UT66WeaponAltarOverlayWidget::HandleWeaponsChanged);
	}

	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	constexpr float CardWidth = 286.f;
	constexpr float CardHeight = 510.f;
	constexpr float IconSize = 206.f;
	constexpr float CardGap = 18.f;
	constexpr float CardPadding = 12.f;

	OfferCardBoxes.SetNum(OfferSlotCount);
	OfferNameTexts.SetNum(OfferSlotCount);
	OfferDescriptionTexts.SetNum(OfferSlotCount);
	OfferIconImages.SetNum(OfferSlotCount);
	OfferIconBrushes.SetNum(OfferSlotCount);
	OfferTileBorders.SetNum(OfferSlotCount);
	OfferIconBorders.SetNum(OfferSlotCount);
	OfferButtons.SetNum(OfferSlotCount);
	OfferButtonBorders.SetNum(OfferSlotCount);
	OfferButtonTexts.SetNum(OfferSlotCount);

	for (int32 SlotIndex = 0; SlotIndex < OfferSlotCount; ++SlotIndex)
	{
		OfferIconBrushes[SlotIndex] = MakeShared<FSlateBrush>();
		OfferIconBrushes[SlotIndex]->DrawAs = ESlateBrushDrawType::Image;
		OfferIconBrushes[SlotIndex]->ImageSize = FVector2D(IconSize, IconSize);
	}

	TSharedRef<SHorizontalBox> CardRow = SNew(SHorizontalBox);
	for (int32 SlotIndex = 0; SlotIndex < OfferSlotCount; ++SlotIndex)
	{
		TSharedRef<SWidget> ChooseButton = MakeWeaponAltarButton(
			NSLOCTEXT("T66.WeaponAltar", "Choose", "CHOOSE"),
			FOnClicked::CreateUObject(this, &UT66WeaponAltarOverlayWidget::OnChooseSlot, SlotIndex),
			OfferButtons[SlotIndex],
			OfferButtonBorders[SlotIndex],
			OfferButtonTexts[SlotIndex],
			220.f,
			FName(*FString::Printf(TEXT("WeaponAltar.ChooseButton.%d"), SlotIndex)));

		CardRow->AddSlot()
		.AutoWidth()
		.Padding(SlotIndex > 0 ? FMargin(CardGap, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			SAssignNew(OfferCardBoxes[SlotIndex], SBox)
			.WidthOverride(CardWidth)
			.HeightOverride(CardHeight)
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(CardPadding),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(52.f)
						.VAlign(VAlign_Center)
						[
							FT66FlatStyle::AttachMetadata(
								SAssignNew(OfferNameTexts[SlotIndex], STextBlock)
								.Text(FText::GetEmpty())
								.Font(FT66FlatStyle::MakeBoldFont(18))
								.ColorAndOpacity(FT66FlatStyle::PrimaryText())
								.Justification(ETextJustify::Center)
								.AutoWrapText(true),
								FName(*FString::Printf(TEXT("WeaponAltar.CardTitle.%d"), SlotIndex)),
								TEXT("Label.Header"),
								ET66FlatState::Default)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 0.f)
					[
						SAssignNew(OfferIconBorders[SlotIndex], SBorder)
						.BorderImage(WhiteBrush())
						.BorderBackgroundColor(FT66FlatStyle::DefaultBorder())
						.Padding(4.f)
						[
							SNew(SBox)
							.WidthOverride(IconSize)
							.HeightOverride(IconSize)
							[
								SAssignNew(OfferIconImages[SlotIndex], SImage)
								.Image(OfferIconBrushes[SlotIndex].Get())
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
					[
						SNew(SBox)
						.HeightOverride(96.f)
						.VAlign(VAlign_Center)
						[
							FT66FlatStyle::AttachMetadata(
								SAssignNew(OfferDescriptionTexts[SlotIndex], STextBlock)
								.Text(FText::GetEmpty())
								.Font(FT66FlatStyle::MakeFont(15))
								.ColorAndOpacity(FT66FlatStyle::SecondaryText())
								.Justification(ETextJustify::Center)
								.AutoWrapText(true),
								FName(*FString::Printf(TEXT("WeaponAltar.CardDescription.%d"), SlotIndex)),
								TEXT("Label.Body"),
								ET66FlatState::Default)
						]
					]
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						SNew(SSpacer)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
					[
						ChooseButton
					],
					&OfferTileBorders[SlotIndex],
					FName(*FString::Printf(TEXT("WeaponAltar.Card.%d"), SlotIndex)))
			]
		];
	}

	TSharedPtr<SWidget> BackButton;
	TSharedPtr<SBorder> BackButtonBorder;
	TSharedPtr<STextBlock> BackButtonText;

	const TAttribute<FMargin> VerticalSafeInsets = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		const FMargin SafeInsets = FT66FlatStyle::GetSafeFrameInsets();
		return FMargin(0.f, SafeInsets.Top, 0.f, SafeInsets.Bottom);
	});
	const TAttribute<FOptionalSize> SurfaceWidthAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		return FOptionalSize(FMath::Max(1.f, FT66FlatStyle::GetViewportLogicalSize().X));
	});

	TSharedRef<SWidget> Root =
		FT66FlatStyle::AttachMetadata(
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(WhiteBrush())
			.BorderBackgroundColor(FLinearColor(
				FT66FlatStyle::BackgroundColor().R,
				FT66FlatStyle::BackgroundColor().G,
				FT66FlatStyle::BackgroundColor().B,
				0.97f)),
				FName(TEXT("WeaponAltar.Backdrop")),
				TEXT("Panel"),
				ET66FlatState::Default)
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			.BorderBackgroundColor(FLinearColor::Transparent)
			.Padding(VerticalSafeInsets)
			[
				SNew(SBox)
				.WidthOverride(SurfaceWidthAttr)
				[
					FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(28.f),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							MakeWeaponAltarButton(
								BackText,
								FOnClicked::CreateUObject(this, &UT66WeaponAltarOverlayWidget::OnBack),
								BackButton,
								BackButtonBorder,
								BackButtonText,
								110.f,
								FName(TEXT("WeaponAltar.BackButton")))
						]
						+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							FT66FlatStyle::AttachMetadata(
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.WeaponAltar", "Title", "WEAPON ALTAR"))
								.Font(FT66FlatStyle::MakeBoldFont(42))
								.ColorAndOpacity(FT66FlatStyle::PrimaryText()),
								FName(TEXT("WeaponAltar.Title")),
								TEXT("Label.Title"),
								ET66FlatState::Default)
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SBox).WidthOverride(110.f)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
					[
						FT66FlatStyle::AttachMetadata(
							SAssignNew(StatusText, STextBlock)
							.Text(NSLOCTEXT("T66.WeaponAltar", "Status", "Choose one auto-attack weapon for this run."))
							.Font(FT66FlatStyle::MakeFont(15))
							.ColorAndOpacity(FT66FlatStyle::SecondaryText()),
							FName(TEXT("WeaponAltar.StatusText")),
							TEXT("Label.Caption"),
							ET66FlatState::Default)
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[
						CardRow
					]
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						SNew(SSpacer)
					],
					nullptr,
					FName(TEXT("WeaponAltar.Panel")))
				]
			]
		],
		FName(TEXT("WeaponAltar.Root")),
		TEXT("Overlay"),
		ET66FlatState::Default);

	SetWeaponAltarButtonState(BackButton, BackButtonBorder, BackButtonText, true, false);
	RefreshOffers();
	return FT66FlatStyle::MakeResponsiveRoot(Root);
}

void UT66WeaponAltarOverlayWidget::RefreshOffers()
{
	UWorld* World = GetWorld();
	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66WeaponManagerSubsystem* WeaponManager = GetWeaponManager(World);
	if (!GI || !WeaponManager)
	{
		return;
	}

	UT66UITexturePoolSubsystem* TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
	const TArray<FName>& WeaponOffers = WeaponManager->GetWeaponOfferIDs();
	const bool bHasSelectionAllowance = HasSelectionsRemaining();

	for (int32 SlotIndex = 0; SlotIndex < OfferSlotCount; ++SlotIndex)
	{
		const FName WeaponID = WeaponOffers.IsValidIndex(SlotIndex) ? WeaponOffers[SlotIndex] : NAME_None;
		FWeaponData WeaponData;
		const bool bHasData = !WeaponID.IsNone() && GI->GetWeaponData(WeaponID, WeaponData);
		const bool bSelected = bHasData && WeaponManager->IsWeaponOfferSelected(WeaponID);
		const bool bCanChoose = bHasData && !bSelected && bHasSelectionAllowance;
		const FLinearColor RarityColor = bHasData ? UT66WeaponManagerSubsystem::GetWeaponRarityColor(WeaponData.Rarity) : FT66FlatStyle::Tokens::Panel2;

		if (OfferCardBoxes.IsValidIndex(SlotIndex) && OfferCardBoxes[SlotIndex].IsValid())
		{
			OfferCardBoxes[SlotIndex]->SetVisibility(bHasData ? EVisibility::Visible : EVisibility::Collapsed);
		}
		if (OfferNameTexts.IsValidIndex(SlotIndex) && OfferNameTexts[SlotIndex].IsValid())
		{
			OfferNameTexts[SlotIndex]->SetText(bHasData ? WeaponData.DisplayName : FText::GetEmpty());
		}
		if (OfferDescriptionTexts.IsValidIndex(SlotIndex) && OfferDescriptionTexts[SlotIndex].IsValid())
		{
			OfferDescriptionTexts[SlotIndex]->SetText(bHasData ? WeaponData.Description : FText::GetEmpty());
		}
		if (OfferTileBorders.IsValidIndex(SlotIndex) && OfferTileBorders[SlotIndex].IsValid())
		{
			OfferTileBorders[SlotIndex]->SetBorderBackgroundColor(bSelected ? FT66FlatStyle::SelectedBorder() : FT66FlatStyle::DefaultBorder());
		}
		if (OfferIconBorders.IsValidIndex(SlotIndex) && OfferIconBorders[SlotIndex].IsValid())
		{
			OfferIconBorders[SlotIndex]->SetBorderBackgroundColor(RarityColor);
		}
		if (OfferIconBrushes.IsValidIndex(SlotIndex) && OfferIconBrushes[SlotIndex].IsValid())
		{
			if (bHasData && !WeaponData.Icon.IsNull() && TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, WeaponData.Icon, this, OfferIconBrushes[SlotIndex], FName(TEXT("WeaponAltarOffer"), SlotIndex + 1), true);
			}
			else
			{
				OfferIconBrushes[SlotIndex]->SetResourceObject(nullptr);
			}
		}
		if (OfferIconImages.IsValidIndex(SlotIndex) && OfferIconImages[SlotIndex].IsValid())
		{
			OfferIconImages[SlotIndex]->SetVisibility(bHasData && !WeaponData.Icon.IsNull() ? EVisibility::Visible : EVisibility::Hidden);
		}
		if (OfferButtonTexts.IsValidIndex(SlotIndex) && OfferButtonTexts[SlotIndex].IsValid())
		{
			OfferButtonTexts[SlotIndex]->SetText(
				bSelected
					? NSLOCTEXT("T66.WeaponAltar", "Equipped", "EQUIPPED")
					: NSLOCTEXT("T66.WeaponAltar", "Choose", "CHOOSE"));
		}

		SetWeaponAltarButtonState(
			OfferButtons.IsValidIndex(SlotIndex) ? OfferButtons[SlotIndex] : TSharedPtr<SWidget>(),
			OfferButtonBorders.IsValidIndex(SlotIndex) ? OfferButtonBorders[SlotIndex] : TSharedPtr<SBorder>(),
			OfferButtonTexts.IsValidIndex(SlotIndex) ? OfferButtonTexts[SlotIndex] : TSharedPtr<STextBlock>(),
			bSelected || bCanChoose,
			bSelected);
	}
}

FReply UT66WeaponAltarOverlayWidget::OnChooseSlot(const int32 SlotIndex)
{
	UWorld* World = GetWorld();
	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66WeaponManagerSubsystem* WeaponManager = GetWeaponManager(World);
	if (!GI || !WeaponManager)
	{
		return FReply::Handled();
	}

	if (!HasSelectionsRemaining())
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(NSLOCTEXT("T66.WeaponAltar", "NoSelectionsRemaining", "The weapon choice is already locked for this run."));
		}
		return FReply::Handled();
	}

	const TArray<FName>& Offers = WeaponManager->GetWeaponOfferIDs();
	const FName WeaponID = Offers.IsValidIndex(SlotIndex) ? Offers[SlotIndex] : NAME_None;
	FWeaponData WeaponData;
	if (WeaponID.IsNone() || !GI->GetWeaponData(WeaponID, WeaponData) || !WeaponManager->SelectWeapon(WeaponID))
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(NSLOCTEXT("T66.WeaponAltar", "SelectionFailed", "Weapon selection failed."));
		}
		return FReply::Handled();
	}

	ConsumeSelectionBudget();
	if (StatusText.IsValid())
	{
		StatusText->SetText(FText::Format(NSLOCTEXT("T66.WeaponAltar", "SelectionApplied", "Equipped {0}."), WeaponData.DisplayName));
	}
	RefreshOffers();
	return FReply::Handled();
}

FReply UT66WeaponAltarOverlayWidget::OnBack()
{
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && !PC->IsPaused())
		{
			PC->RestoreGameplayInputMode();
		}
	}
	return FReply::Handled();
}
