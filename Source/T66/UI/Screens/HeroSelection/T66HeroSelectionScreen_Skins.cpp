// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

using namespace T66HeroSelectionPrivate;

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
		T66SelectionScreenUtils::PopulateDefaultOwnedSkins(PlaceholderSkins);
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
	if (ACBalanceTextBlock.IsValid())
	{
		ACBalanceTextBlock->SetText(FText::AsNumber(T66SelectionScreenUtils::GetAchievementCoinBalance(this)));
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
	const float ActionMinHeight = 34.f;
	const float ActionMinWidth = 92.f;
	const float EquippedMinWidth = 108.f;
	const float BuyButtonMinWidth = 92.f;
	const float BuyButtonHeight = 34.f;
	const int32 SkinActionFontSize = 14;
	const int32 SkinPriceFontSize = 14;
	const int32 SkinTitleFontSize = 15;

	auto MakeSkinListRow = [](const TSharedRef<SWidget>& Content, const FMargin& RowPadding) -> TSharedRef<SWidget>
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(RowPadding)
				[
					Content
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.f, 0.f)
			[
				SNew(SBox)
				.HeightOverride(1.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.45f, 0.27f, 0.07f, 0.65f))
				]
			];
	};

	if (PlaceholderSkins.Num() == 0)
	{
		Box->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 6.0f)
			[
				MakeSkinListRow(
					SNew(STextBlock)
					.Text(bCompanionSkins && SkinEntityID.IsNone()
						? SelectCompanionForSkinsText
						: NSLOCTEXT("T66.HeroSelection", "NoSkinsAvailable", "No skins available."))
					.Font(FT66Style::Tokens::FontRegular(SkinTitleFontSize))
					.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
					.AutoWrapText(true),
					FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3))
			];
		return;
	}

	auto ClearSkinPreviewOverride = [this, bCompanionSkins]()
	{
		UT66HeroSelectionPreviewController* HeroPreviewController = GetOrCreatePreviewController();
		if (!HeroPreviewController)
		{
			return;
		}

		if (bCompanionSkins)
		{
			HeroPreviewController->ResetCompanionSkinPreviewOverride();
		}
		else
		{
			HeroPreviewController->ResetHeroSkinPreviewOverride();
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
		UT66HeroSelectionPreviewController* HeroPreviewController = GetOrCreatePreviewController();
		if (!HeroPreviewController)
		{
			return FReply::Handled();
		}

		if (bCompanionSkins)
		{
			HeroPreviewController->ToggleCompanionSkinPreviewOverride(SkinID);
		}
		else
		{
			HeroPreviewController->ToggleHeroSkinPreviewOverride(SkinID);
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
			ButtonRow->AddSlot().AutoWidth().Padding(3.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(EquippedMinWidth).MinDesiredHeight(ActionMinHeight)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(bIsEquipped ? 1 : 0)
						+ SWidgetSwitcher::Slot()
						[
							MakeHeroSelectionButton(FT66ButtonParams(EquipText,
							FOnClicked::CreateLambda([ApplySkinSelection, SkinIDCopy]() { return ApplySkinSelection(SkinIDCopy); }),
							ET66ButtonType::Primary)
							.SetMinWidth(ActionMinWidth)
							.SetHeight(ActionMinHeight)
							.SetPadding(FMargin(7.f, 3.f))
							.SetContent(SNew(STextBlock).Text(EquipText).Font(FT66Style::Tokens::FontBold(SkinActionFontSize)).ColorAndOpacity(FT66Style::Tokens::Text).Justification(ETextJustify::Center))
						)
						]
						+ SWidgetSwitcher::Slot()
						[
							MakeHeroSelectionButton(FT66ButtonParams(
								EquippedText,
								FOnClicked::CreateLambda([]() { return FReply::Handled(); }),
								ET66ButtonType::ToggleActive)
								.SetMinWidth(EquippedMinWidth)
								.SetHeight(ActionMinHeight)
								.SetPadding(FMargin(7.f, 3.f))
								.SetFontSize(SkinActionFontSize))
						]
					]
				];
		}
		if (bIsBeachgoer)
		{
			ButtonRow->AddSlot().AutoWidth().Padding(3.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(ActionMinWidth).MinDesiredHeight(ActionMinHeight)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(!bIsOwned ? 0 : (bIsEquipped ? 2 : 1))
						+ SWidgetSwitcher::Slot()
						[
							MakeHeroSelectionButton(FT66ButtonParams(PreviewText,
								FOnClicked::CreateLambda([TogglePreviewOverride, SkinIDCopy]() { return TogglePreviewOverride(SkinIDCopy); }),
								ET66ButtonType::Neutral)
								.SetMinWidth(ActionMinWidth)
								.SetHeight(ActionMinHeight)
								.SetPadding(FMargin(7.f, 3.f, 7.f, 2.f))
								.SetContent(SNew(STextBlock).Text(PreviewText).Font(FT66Style::Tokens::FontBold(SkinActionFontSize)).ColorAndOpacity(FT66Style::Tokens::Text).Justification(ETextJustify::Center)))
						]
						+ SWidgetSwitcher::Slot()
						[
							MakeHeroSelectionButton(FT66ButtonParams(EquipText,
								FOnClicked::CreateLambda([ApplySkinSelection, SkinIDCopy]() { return ApplySkinSelection(SkinIDCopy); }),
								ET66ButtonType::Primary)
								.SetMinWidth(ActionMinWidth)
								.SetHeight(ActionMinHeight)
								.SetPadding(FMargin(7.f, 3.f, 7.f, 2.f))
								.SetContent(SNew(STextBlock).Text(EquipText).Font(FT66Style::Tokens::FontBold(SkinActionFontSize)).ColorAndOpacity(FT66Style::Tokens::Text).Justification(ETextJustify::Center)))
						]
						+ SWidgetSwitcher::Slot()
						[
							MakeHeroSelectionButton(FT66ButtonParams(
								EquippedText,
								FOnClicked::CreateLambda([]() { return FReply::Handled(); }),
								ET66ButtonType::ToggleActive)
								.SetMinWidth(ActionMinWidth)
								.SetHeight(ActionMinHeight)
								.SetPadding(FMargin(7.f, 3.f, 7.f, 2.f))
								.SetFontSize(SkinActionFontSize))
						]
					]
				];
			ButtonRow->AddSlot().AutoWidth().Padding(3.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(EquippedMinWidth).MinDesiredHeight(BuyButtonHeight)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(bIsOwned ? 1 : 0)
						+ SWidgetSwitcher::Slot()
						[
						MakeHeroSelectionButton(FT66ButtonParams(PriceText,
							FOnClicked::CreateLambda([PurchaseAndEquipSkin, SkinIDCopy, Price]() { return PurchaseAndEquipSkin(SkinIDCopy, Price); }),
							ET66ButtonType::Primary)
							.SetMinWidth(BuyButtonMinWidth)
							.SetHeight(BuyButtonHeight)
							.SetColor(FT66Style::Tokens::Accent)
							.SetPadding(FMargin(7.f, 3.f, 7.f, 2.f))
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
								.Padding(4.f, 0.f, 0.f, 0.f)
								[
									SNew(SBox)
									.WidthOverride(20.f)
									.HeightOverride(14.f)
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
						MakeHeroSelectionButton(FT66ButtonParams(RefundText,
							FOnClicked::CreateLambda([RefundOwnedSkin, SkinIDCopy, Price]() { return RefundOwnedSkin(SkinIDCopy, Price); }),
							ET66ButtonType::Neutral)
							.SetMinWidth(ActionMinWidth)
							.SetHeight(ActionMinHeight)
							.SetPadding(FMargin(7.f, 3.f, 7.f, 2.f))
							.SetContent(SNew(STextBlock).Text(RefundText).Font(FT66Style::Tokens::FontBold(SkinActionFontSize)).ColorAndOpacity(FT66Style::Tokens::Text).Justification(ETextJustify::Center))
						)
						]
					]
				];
		}

		Box->AddSlot()
			.AutoHeight()
			.Padding(0.0f)
			[
				MakeSkinListRow(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 7.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(30.f)
						.HeightOverride(30.f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SImage)
								.Image(GetHeroSelectionPartySlotBrush())
							]
							+ SOverlay::Slot()
							.Padding(8.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(bIsBeachgoer
									? FLinearColor(0.02f, 0.42f, 0.58f, 1.0f)
									: FLinearColor(0.16f, 0.08f, 0.015f, 1.0f))
							]
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(SkinDisplayName)
						.Font(FT66Style::Tokens::FontRegular(SkinTitleFontSize))
						.ColorAndOpacity(GetHeroSelectionParchmentText())
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(3.0f, 0.0f, 0.0f, 0.0f)
					[
						ButtonRow
					],
					FMargin(10.f, 7.f))
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
	RefreshTargetDropdownTexts();
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

	RefreshPanelSwitchers();
	RefreshTargetDropdownTexts();
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

	if (const UT66HeroSelectionPreviewController* HeroPreviewController = GetPreviewController())
	{
		return HeroPreviewController->ResolveEffectiveCompanionSkinID(
			Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)),
			PreviewedCompanionID);
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
		UpdateHeroDisplay();
	}
}
