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
	const FText SelectCompanionForSkinsText = NSLOCTEXT("T66.HeroSelection", "SelectCompanionForSkins", "Select a girlfriend to manage girlfriend skins.");
	const float ActionMinHeight = 30.f;
	const float ActionMinWidth = 84.f;
	const float EquippedMinWidth = 100.f;
	const float BuyButtonMinWidth = 84.f;
	const float BuyButtonHeight = 30.f;
	const int32 SkinActionFontSize = 13;
	const int32 SkinPriceFontSize = 14;
	const int32 SkinTitleFontSize = 24;
	const FVector2D SkinPortraitSize(68.f, 68.f);

	auto MakeSkinListRow = [](const TSharedRef<SWidget>& Content, const FMargin& RowPadding, const ET66FlatState RowState) -> TSharedRef<SWidget>
	{
		static_cast<void>(RowState);
		return SNew(SBox)
			.HeightOverride(76.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(FLinearColor::Transparent)
				.Padding(RowPadding)
				[
					Content
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
					FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3),
					ET66FlatState::Default)
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

	for (int32 SkinIndex = 0; SkinIndex < PlaceholderSkins.Num(); ++SkinIndex)
	{
		const FSkinData& Skin = PlaceholderSkins[SkinIndex];
		FText SkinDisplayName = !Skin.DisplayName.IsEmpty()
			? Skin.DisplayName
			: (Loc ? Loc->GetText_SkinName(Skin.SkinID) : FText::FromName(Skin.SkinID));
		const int32 Price = FMath::Max(0, Skin.CoinCost);
		const FText PriceText = FText::AsNumber(Price);
		const FName SkinIDCopy = Skin.SkinID;
		const bool bIsDefault = Skin.bIsDefault;
		const bool bIsOwned = Skin.bIsOwned;
		const bool bIsEquipped = Skin.bIsEquipped;
		const bool bIsDemoSkin = (SkinIDCopy == UT66SkinSubsystem::DemoSkinID);

		const FString SkinInitialText = SkinDisplayName.ToString().Left(2).ToUpper();
		const FLinearColor SkinThumbnailFill = bIsDefault
			? FLinearColor(0.16f, 0.08f, 0.05f, 1.0f)
			: (bIsDemoSkin
				? FLinearColor(0.07f, 0.18f, 0.25f, 1.0f)
				: FLinearColor(0.09f, 0.08f, 0.12f, 1.0f));

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
							.SetContent(MakeHeroSelectionFittedLabel(EquipText, SkinActionFontSize, FT66Style::Tokens::Text))
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
		if (bIsDemoSkin)
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
								.SetContent(MakeHeroSelectionFittedLabel(PreviewText, SkinActionFontSize, FT66Style::Tokens::Text)))
						]
						+ SWidgetSwitcher::Slot()
						[
							MakeHeroSelectionButton(FT66ButtonParams(EquipText,
								FOnClicked::CreateLambda([ApplySkinSelection, SkinIDCopy]() { return ApplySkinSelection(SkinIDCopy); }),
								ET66ButtonType::Primary)
								.SetMinWidth(ActionMinWidth)
								.SetHeight(ActionMinHeight)
								.SetPadding(FMargin(7.f, 3.f, 7.f, 2.f))
								.SetContent(MakeHeroSelectionFittedLabel(EquipText, SkinActionFontSize, FT66Style::Tokens::Text)))
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
							.SetColor(HeroSelectionChromeTokenAccent())
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
										FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
											SNew(SImage)
											.Image_Lambda([this]() -> const FSlateBrush*
											{
												return ACBalanceIconBrush.IsValid() && ::IsValid(ACBalanceIconBrush->GetResourceObject())
													? ACBalanceIconBrush.Get()
													: nullptr;
											})),
											FName(*FString::Printf(TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.%s.PriceIcon"), *SkinIDCopy.ToString())),
											TEXT("Icon"))
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
							.SetContent(MakeHeroSelectionFittedLabel(RefundText, SkinActionFontSize, FT66Style::Tokens::Text))
						)
						]
					]
				];
		}
		else if (!bIsDefault)
		{
			ButtonRow->AddSlot().AutoWidth().Padding(3.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(EquippedMinWidth).MinDesiredHeight(BuyButtonHeight)
					[
						MakeHeroSelectionButton(FT66ButtonParams(PriceText,
							FOnClicked::CreateLambda([]() { return FReply::Handled(); }),
							ET66ButtonType::Neutral)
							.SetMinWidth(BuyButtonMinWidth)
							.SetHeight(BuyButtonHeight)
							.SetPadding(FMargin(7.f, 3.f, 7.f, 2.f))
							.SetContent(
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(PriceText)
									.Font(FT66FlatStyle::MakeBoldFont(SkinPriceFontSize))
									.ColorAndOpacity(GetHeroSelectionParchmentText())
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
											FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
												SNew(SImage)
												.Image_Lambda([this]() -> const FSlateBrush*
												{
													return ACBalanceIconBrush.IsValid() && ::IsValid(ACBalanceIconBrush->GetResourceObject())
														? ACBalanceIconBrush.Get()
														: nullptr;
												})),
												FName(*FString::Printf(TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.%s.PriceIcon"), *SkinIDCopy.ToString())),
												TEXT("Icon"))
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
											.Font(FT66FlatStyle::MakeBoldFont(SkinActionFontSize))
											.ColorAndOpacity(GetHeroSelectionParchmentText())
										]
									]
								])
						)
					]
				];
		}

		Box->AddSlot()
			.AutoHeight()
			.Padding(0.0f, SkinIndex > 0 ? 6.0f : 0.0f, 0.0f, 0.0f)
			[
				MakeSkinListRow(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 14.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(SkinPortraitSize.X)
						.HeightOverride(SkinPortraitSize.Y)
						[
							FT66FlatStyle::MakeFlatSubPanel(
								bIsEquipped ? ET66FlatState::Selected : ET66FlatState::Default,
								FMargin(0.f),
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
								.BorderBackgroundColor(SkinThumbnailFill)
								.Padding(0.f)
								[
									SNew(SOverlay)
									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(STextBlock)
										.Text(FText::FromString(SkinInitialText))
										.Font(FT66FlatStyle::MakeBoldFont(24))
										.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
										.Justification(ETextJustify::Center)
									]
								]
							)
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(SkinDisplayName)
						.Font(FT66FlatStyle::MakeBoldFont(SkinTitleFontSize))
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
					FMargin(8.f, 4.f),
					bIsEquipped ? ET66FlatState::Selected : ET66FlatState::Default)
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
