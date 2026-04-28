// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

using namespace T66HeroSelectionPrivate;

FReply UT66HeroSelectionScreen::HandleUltimatePreviewClicked()
{
	if (UT66HeroSelectionPreviewController* HeroPreviewController = GetOrCreatePreviewController())
	{
		HeroPreviewController->ToggleSelectedPreviewClip(ET66HeroSelectionPreviewClip::Ultimate, bShowingCompanionInfo);
		HeroPreviewController->UpdateHeroPreviewVideo(PreviewedHeroID, bShowingCompanionInfo);
	}
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandlePassivePreviewClicked()
{
	if (UT66HeroSelectionPreviewController* HeroPreviewController = GetOrCreatePreviewController())
	{
		HeroPreviewController->ToggleSelectedPreviewClip(ET66HeroSelectionPreviewClip::Passive, bShowingCompanionInfo);
		HeroPreviewController->UpdateHeroPreviewVideo(PreviewedHeroID, bShowingCompanionInfo);
	}
	return FReply::Handled();
}

void UT66HeroSelectionScreen::RefreshHeroRecordRank()
{
	if (!HeroRecordRankWidget.IsValid())
	{
		return;
	}

	HeroRecordRankRequestKey.Reset();
	if (bShowingCompanionInfo || PreviewedHeroID.IsNone())
	{
		HeroRecordRankWidget->SetText(NSLOCTEXT("T66.HeroSelection", "HeroRecordRankUnavailable", "--"));
		return;
	}

	UGameInstance* GIBase = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GIBase);
	UT66BackendSubsystem* Backend = GIBase ? GIBase->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend)
	{
		HeroRecordRankWidget->SetText(NSLOCTEXT("T66.HeroSelection", "HeroRecordRankUnavailableNoBackend", "--"));
		return;
	}

	int32 PartySize = 1;
	if (T66GI)
	{
		if (UT66SessionSubsystem* SessionSubsystem = T66GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (SessionSubsystem->IsPartyLobbyContextActive())
			{
				PartySize = FMath::Max(1, SessionSubsystem->GetCurrentLobbyPlayerCount());
			}
		}
		if (PartySize <= 1)
		{
			if (UT66PartySubsystem* PartySubsystem = T66GI->GetSubsystem<UT66PartySubsystem>())
			{
				PartySize = FMath::Max(1, PartySubsystem->GetPartyMemberCount());
			}
		}
	}

	const FString HeroID = PreviewedHeroID.ToString();
	const FString DifficultyKey = HeroSelectionDifficultyToApiString(SelectedDifficulty);
	const FString PartyKey = HeroSelectionPartySizeToApiString(PartySize);
	const FString RankKey = UT66BackendSubsystem::MakeMyRankCacheKey(
		TEXT("score"),
		TEXT("alltime"),
		PartyKey,
		DifficultyKey,
		TEXT("hero"),
		HeroID);
	HeroRecordRankRequestKey = RankKey;

	bool bRankSuccess = false;
	int32 Rank = 0;
	int32 TotalEntries = 0;
	if (Backend->GetCachedMyRank(RankKey, bRankSuccess, Rank, TotalEntries))
	{
		if (bRankSuccess && Rank > 0)
		{
			HeroRecordRankWidget->SetText(FText::Format(
				NSLOCTEXT("T66.HeroSelection", "HeroRecordRankFormat", "#{0} / {1}"),
				FText::AsNumber(Rank),
				FText::AsNumber(FMath::Max(0, TotalEntries))));
		}
		else
		{
			HeroRecordRankWidget->SetText(NSLOCTEXT("T66.HeroSelection", "HeroRecordRankUnavailableRank", "N/A"));
		}
		return;
	}

	if (!Backend->IsBackendConfigured() || !Backend->HasSteamTicket())
	{
		HeroRecordRankWidget->SetText(NSLOCTEXT("T66.HeroSelection", "HeroRecordRankOffline", "--"));
		return;
	}

	HeroRecordRankWidget->SetText(NSLOCTEXT("T66.HeroSelection", "HeroRecordRankPending", "..."));
	Backend->FetchMyRankFiltered(
		TEXT("score"),
		TEXT("alltime"),
		PartyKey,
		DifficultyKey,
		TEXT("hero"),
		HeroID);
}

void UT66HeroSelectionScreen::HandleBackendMyRankDataReady(const FString& Key, bool bSuccess, int32 Rank, int32 TotalEntries)
{
	static_cast<void>(bSuccess);
	static_cast<void>(Rank);
	static_cast<void>(TotalEntries);

	if (!HeroRecordRankRequestKey.Equals(Key) || !HasBuiltSlateUI() || !IsVisible())
	{
		return;
	}

	RefreshHeroRecordRank();
}

void UT66HeroSelectionScreen::UpdateHeroDisplay()
{
	RefreshPanelSwitchers();

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66HeroSelectionPreviewController* HeroPreviewController = GetOrCreatePreviewController();

	auto SetImageBrushFromPath = [this](const FString& ImagePath, TStrongObjectPtr<UTexture2D>& Texture, TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize, const TCHAR* DebugName)
	{
		if (IFileManager::Get().FileExists(*ImagePath))
		{
			Texture.Reset();
			UTexture2D* LoadedTexture = T66RuntimeUITextureAccess::ImportFileTexture(
				ImagePath,
				TextureFilter::TF_Trilinear,
				true,
				DebugName);
			if (!LoadedTexture)
			{
				LoadedTexture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
					ImagePath,
					TextureFilter::TF_Trilinear,
					DebugName);
			}

			if (LoadedTexture)
			{
				Texture.Reset(LoadedTexture);
				if (!Brush.IsValid())
				{
					Brush = MakeShared<FSlateBrush>();
					Brush->DrawAs = ESlateBrushDrawType::Image;
					Brush->Tiling = ESlateBrushTileType::NoTile;
					Brush->TintColor = FSlateColor(FLinearColor::White);
				}
				Brush->ImageSize = ImageSize;
				Brush->SetResourceObject(Texture.Get());
			}
			else
			{
				Brush = MakeShared<FSlateImageBrush>(*ImagePath, ImageSize);
			}
		}
		else
		{
			Texture.Reset();
			Brush.Reset();
		}
	};

	auto SetMedalBrushForTier = [this, &SetImageBrushFromPath](const ET66AccountMedalTier MedalTier)
	{
		SetImageBrushFromPath(
			GetHeroSelectionMedalImagePath(MedalTier),
			HeroRecordMedalTexture,
			HeroRecordMedalBrush,
			FVector2D(46.f, 46.f),
			TEXT("HeroSelectionMedalIcon"));
	};

	SetImageBrushFromPath(
		GetHeroSelectionRankImagePath(),
		HeroRecordRankTexture,
		HeroRecordRankBrush,
		FVector2D(46.f, 46.f),
		TEXT("HeroSelectionRankIcon"));

	auto SetRecordValues = [this, &SetMedalBrushForTier](const ET66AccountMedalTier MedalTier)
	{
		SetMedalBrushForTier(MedalTier);

		if (HeroRecordMedalWidget.IsValid())
		{
			HeroRecordMedalWidget->SetText(HeroRecordMedalText(MedalTier));
			HeroRecordMedalWidget->SetColorAndOpacity(HeroRecordMedalColor(MedalTier));
		}
		RefreshHeroRecordRank();
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
	RefreshTargetDropdownTexts();
	RefreshDifficultyDropdownText();

	if (HeroPreviewController)
	{
		HeroPreviewController->RefreshCompanionPreviewPanel(T66GI, PreviewedCompanionID, bShowingCompanionInfo);
	}

	FHeroData HeroData;
	FLinearColor HeroPreviewFallbackColor(0.3f, 0.3f, 0.4f, 1.0f);
	if (GetPreviewedHeroData(HeroData))
	{
		HeroPreviewFallbackColor = HeroData.PlaceholderColor;
		if (HeroLoreWidget.IsValid())
		{
			const FText Desc = Loc ? Loc->GetText_HeroDescription(PreviewedHeroID) : HeroData.Description;
			HeroLoreWidget->SetText(Desc);
		}

		FT66HeroStatBlock BaseStats;
		if (T66GI)
		{
			FT66HeroPerLevelStatGains UnusedPerLevelGains;
			T66GI->GetHeroStatTuning(PreviewedHeroID, BaseStats, UnusedPerLevelGains);
		}

		FT66HeroStatBonuses PermanentBuffBonuses;
		if (UT66BuffSubsystem* Buffs = GI ? GI->GetSubsystem<UT66BuffSubsystem>() : nullptr)
		{
			PermanentBuffBonuses = Buffs->GetPermanentBuffStatBonuses();
		}

		PopulateHeroStatsSnapshot(HeroData, BaseStats, PermanentBuffBonuses);
		RefreshHeroStatsPanels();

		if (HeroUltimateIconBrush.IsValid())
		{
			HeroUltimateIconBrush->SetResourceObject(nullptr);
		}
		if (HeroPassiveIconBrush.IsValid())
		{
			HeroPassiveIconBrush->SetResourceObject(nullptr);
		}
		if (T66GI)
		{
			if (UT66UITexturePoolSubsystem* TexPool = T66GI->GetSubsystem<UT66UITexturePoolSubsystem>())
			{
				T66SlateTexture::BindSharedBrushAsync(
					TexPool,
					ResolveHeroSelectionUltimateIcon(HeroData.HeroID, HeroData.UltimateType),
					this,
					HeroUltimateIconBrush,
					FName(TEXT("HeroSelectionUltimateIcon")),
					true);
				T66SlateTexture::BindSharedBrushAsync(
					TexPool,
					ResolveHeroSelectionPassiveIcon(HeroData.HeroID, HeroData.PassiveType),
					this,
					HeroPassiveIconBrush,
					FName(TEXT("HeroSelectionPassiveIcon")),
					true);
			}
		}

		if (bShowingCompanionInfo)
		{
			FCompanionData CompanionData;
			const bool bHasCompanion = GetPreviewedCompanionData(CompanionData);
			const ET66AccountMedalTier MedalTier = (Achievements && bHasCompanion)
				? Achievements->GetCompanionHighestMedal(PreviewedCompanionID)
				: ET66AccountMedalTier::None;
			const int32 UnionStages = (Achievements && bHasCompanion)
				? Achievements->GetCompanionUnionStagesCleared(PreviewedCompanionID)
				: 0;
			const float HealingPerSecond = bHasCompanion
				? AT66CompanionBase::GetHealingPerSecondForUnionStages(UnionStages)
				: 0.f;
			CompanionUnityStagesCleared = UnionStages;
			CompanionUnityProgress01 = (Achievements && bHasCompanion)
				? FMath::Clamp(Achievements->GetCompanionUnionProgress01(PreviewedCompanionID), 0.f, 1.f)
				: 0.f;
			if (CompanionUnityProgressBar.IsValid())
			{
				CompanionUnityProgressBar->SetPercent(CompanionUnityProgress01);
			}
			if (CompanionHealsPerSecondWidget.IsValid())
			{
				CompanionHealsPerSecondWidget->SetText(
					bHasCompanion
						? FormatCompanionHealingPerSecondText(HealingPerSecond)
						: (Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.HeroSelection", "NoCompanionInfo", "No companion selected.")));
			}
			if (CompanionUnityTextWidget.IsValid())
			{
				CompanionUnityTextWidget->SetText(
					bHasCompanion
						? FText::Format(
							NSLOCTEXT("T66.HeroSelection", "CompanionUnityFormat", "Unity: {0} / {1}"),
							FText::AsNumber(CompanionUnityStagesCleared),
							FText::AsNumber(UT66AchievementsSubsystem::UnionTier_HyperStages))
						: NSLOCTEXT("T66.HeroSelection", "CompanionUnityNoSelection", "Select a companion to view unity."));
			}
			SetRecordValues(MedalTier);
		}
		else
		{
			if (CompanionHealsPerSecondWidget.IsValid())
			{
				CompanionHealsPerSecondWidget->SetText(NSLOCTEXT("T66.HeroSelection", "CompanionHealsPerSecondDefault", "Heals / Second: 0"));
			}
			CompanionUnityStagesCleared = 0;
			CompanionUnityProgress01 = 0.f;
			if (CompanionUnityProgressBar.IsValid())
			{
				CompanionUnityProgressBar->SetPercent(CompanionUnityProgress01);
			}
			if (CompanionUnityTextWidget.IsValid())
			{
				CompanionUnityTextWidget->SetText(NSLOCTEXT("T66.HeroSelection", "CompanionUnityDefault", "Unity: 0 / 20"));
			}
			const ET66AccountMedalTier MedalTier = Achievements ? Achievements->GetHeroHighestMedal(PreviewedHeroID) : ET66AccountMedalTier::None;
			SetRecordValues(MedalTier);
		}
	}
	else
	{
		if (HeroLoreWidget.IsValid())
		{
			HeroLoreWidget->SetText(FText::GetEmpty());
		}
		HeroStatsSnapshot = nullptr;
		RefreshHeroStatsPanels();
		if (HeroUltimateIconBrush.IsValid())
		{
			HeroUltimateIconBrush->SetResourceObject(nullptr);
		}
		if (HeroPassiveIconBrush.IsValid())
		{
			HeroPassiveIconBrush->SetResourceObject(nullptr);
		}
		if (CompanionHealsPerSecondWidget.IsValid())
		{
			CompanionHealsPerSecondWidget->SetText(NSLOCTEXT("T66.HeroSelection", "CompanionHealsPerSecondDefault", "Heals / Second: 0"));
		}
		CompanionUnityStagesCleared = 0;
		CompanionUnityProgress01 = 0.f;
		if (CompanionUnityProgressBar.IsValid())
		{
			CompanionUnityProgressBar->SetPercent(CompanionUnityProgress01);
		}
		if (CompanionUnityTextWidget.IsValid())
		{
			CompanionUnityTextWidget->SetText(NSLOCTEXT("T66.HeroSelection", "CompanionUnityDefault", "Unity: 0 / 20"));
		}
		SetRecordValues(ET66AccountMedalTier::None);
	}

	if (HeroPreviewController)
	{
		HeroPreviewController->ApplyHeroPreviewStage(
			T66GI,
			PreviewedHeroID,
			PreviewedCompanionID,
			SelectedBodyType,
			SelectedDifficulty,
			HeroPreviewFallbackColor);
	}

	RefreshHeroCarouselPortraits();
	RefreshCompanionCarouselPortraits();
}

void UT66HeroSelectionScreen::RefreshHeroCarouselPortraits()
{
	if (AllHeroIDs.Num() <= 0)
	{
		HeroCarouselSlotColors.Init(FLinearColor(0.2f, 0.2f, 0.25f, 1.0f), HeroSelectionCarouselVisibleSlots);
		HeroCarouselSlotVisibility.Init(EVisibility::Collapsed, HeroSelectionCarouselVisibleSlots);
		for (int32 Index = 0; Index < HeroCarouselPortraitBrushes.Num(); ++Index)
		{
			if (HeroCarouselPortraitBrushes[Index].IsValid())
			{
				HeroCarouselPortraitBrushes[Index]->SetResourceObject(nullptr);
			}
			if (HeroCarouselImageWidgets.IsValidIndex(Index) && HeroCarouselImageWidgets[Index].IsValid())
			{
				HeroCarouselImageWidgets[Index]->SetVisibility(EVisibility::Collapsed);
			}
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
				T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, HeroCarouselPortraitBrushes[SlotIdx], FName(TEXT("HeroCarousel"), SlotIdx + 1), true);
			}
			HeroCarouselPortraitBrushes[SlotIdx]->ImageSize = FVector2D(BoxSize, BoxSize);
			if (HeroCarouselImageWidgets.IsValidIndex(SlotIdx) && HeroCarouselImageWidgets[SlotIdx].IsValid())
			{
				HeroCarouselImageWidgets[SlotIdx]->SetVisibility(HeroCarouselSlotVisibility[SlotIdx]);
			}
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
			if (CompanionCarouselImageWidgets.IsValidIndex(Index) && CompanionCarouselImageWidgets[Index].IsValid())
			{
				CompanionCarouselImageWidgets[Index]->SetVisibility(EVisibility::Collapsed);
			}
			if (CompanionCarouselLabelWidgets.IsValidIndex(Index) && CompanionCarouselLabelWidgets[Index].IsValid())
			{
				CompanionCarouselLabelWidgets[Index]->SetText(FText::GetEmpty());
				CompanionCarouselLabelWidgets[Index]->SetVisibility(EVisibility::Collapsed);
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
					true);
			}
			CompanionCarouselPortraitBrushes[SlotIdx]->ImageSize = FVector2D(BoxSize, BoxSize);
			if (CompanionCarouselImageWidgets.IsValidIndex(SlotIdx) && CompanionCarouselImageWidgets[SlotIdx].IsValid())
			{
				CompanionCarouselImageWidgets[SlotIdx]->SetVisibility(CompanionCarouselSlotVisibility[SlotIdx]);
			}
			if (CompanionCarouselLabelWidgets.IsValidIndex(SlotIdx) && CompanionCarouselLabelWidgets[SlotIdx].IsValid())
			{
				CompanionCarouselLabelWidgets[SlotIdx]->SetText(CompanionCarouselSlotLabels[SlotIdx]);
				CompanionCarouselLabelWidgets[SlotIdx]->SetVisibility(CompanionCarouselSlotLabels[SlotIdx].IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible);
			}
		}
	}
}
