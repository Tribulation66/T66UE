// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"
#include "Core/T66WeaponManagerSubsystem.h"
#include "UI/T66FrontendVideoPlayer.h"

using namespace T66HeroSelectionPrivate;

namespace
{
	struct FHeroKitPreviewDisplay
	{
		FName KitID = NAME_None;
		FText Title;
		FText Description;
		FLinearColor FallbackColor = FLinearColor(0.025f, 0.027f, 0.035f, 1.f);
	};

	FText FormatAttackCategory(ET66AttackCategory Category)
	{
		switch (Category)
		{
		case ET66AttackCategory::Bounce:
			return NSLOCTEXT("T66.HeroSelection", "AttackCategoryBounce", "Bounce");
		case ET66AttackCategory::AOE:
			return NSLOCTEXT("T66.HeroSelection", "AttackCategoryAoe", "AOE");
		case ET66AttackCategory::DOT:
			return NSLOCTEXT("T66.HeroSelection", "AttackCategoryDot", "DOT");
		case ET66AttackCategory::SingleTarget:
			return NSLOCTEXT("T66.HeroSelection", "AttackCategorySingleTarget", "Single Target");
		case ET66AttackCategory::Pierce:
		default:
			return NSLOCTEXT("T66.HeroSelection", "AttackCategoryPierce", "Pierce");
		}
	}

	FHeroKitPreviewDisplay ResolveHeroKitPreviewDisplay(
		UT66GameInstance* T66GI,
		UT66LocalizationSubsystem* Loc,
		const FHeroData& HeroData,
		const ET66HeroKitPreviewSlot KitSlot)
	{
		FHeroKitPreviewDisplay Display;
		Display.FallbackColor = HeroData.PlaceholderColor;
		if (KitSlot == ET66HeroKitPreviewSlot::Weapon)
		{
			Display.KitID = UT66WeaponManagerSubsystem::MakeWeaponID(
				HeroData.HeroID,
				ET66WeaponRarity::Black,
				HeroData.PrimaryCategory);

			FWeaponData WeaponData;
			if (T66GI && T66GI->GetWeaponData(Display.KitID, WeaponData))
			{
				Display.Title = WeaponData.DisplayName.IsEmpty()
					? FText::FromName(Display.KitID)
					: WeaponData.DisplayName;
				Display.Description = WeaponData.Description.IsEmpty()
					? FText::Format(
						NSLOCTEXT("T66.HeroSelection", "WeaponPreviewFallbackDescription", "Primary branch: {0}."),
						FormatAttackCategory(HeroData.PrimaryCategory))
					: WeaponData.Description;
				return Display;
			}

			Display.Title = FText::Format(
				NSLOCTEXT("T66.HeroSelection", "WeaponPreviewFallbackTitle", "{0} Weapon"),
				FormatAttackCategory(HeroData.PrimaryCategory));
			Display.Description = FText::Format(
				NSLOCTEXT("T66.HeroSelection", "WeaponPreviewFallbackDescriptionShort", "Primary {0} branch preview."),
				FormatAttackCategory(HeroData.PrimaryCategory));
			return Display;
		}

		Display.KitID = StaticEnum<ET66UltimateType>()
			? FName(*StaticEnum<ET66UltimateType>()->GetNameStringByValue(static_cast<int64>(HeroData.UltimateType)))
			: FName(TEXT("Ultimate"));
		Display.Title = Loc
			? Loc->GetText_UltimateName(HeroData.UltimateType)
			: FText::FromName(Display.KitID);
		Display.Description = Loc
			? Loc->GetText_UltimateDescription(HeroData.UltimateType)
			: NSLOCTEXT("T66.HeroSelection", "UltimatePreviewFallbackDescription", "Ultimate kit preview.");
		return Display;
	}

	FText FormatHeroRecordRankText(const int32 Rank)
	{
		if (Rank <= 0)
		{
			return NSLOCTEXT("T66.HeroSelection", "HeroRecordRankUnranked", "Unranked");
		}

		if (Rank <= 10000)
		{
			return FText::Format(
				NSLOCTEXT("T66.HeroSelection", "HeroRecordRankExactFormat", "#{0}"),
				FText::AsNumber(Rank));
		}

		if (Rank <= 25000)
		{
			return NSLOCTEXT("T66.HeroSelection", "HeroRecordRankTop25K", "Top 25K");
		}

		if (Rank <= 50000)
		{
			return NSLOCTEXT("T66.HeroSelection", "HeroRecordRankTop50K", "Top 50K");
		}

		if (Rank <= 100000)
		{
			return NSLOCTEXT("T66.HeroSelection", "HeroRecordRankTop100K", "Top 100K");
		}

		return NSLOCTEXT("T66.HeroSelection", "HeroRecordRankUnranked", "Unranked");
	}
}

FReply UT66HeroSelectionScreen::HandleWeaponPreviewClicked()
{
	ApplyKitPreviewVideo(ET66HeroKitPreviewSlot::Weapon);
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleUltimatePreviewClicked()
{
	ApplyKitPreviewVideo(ET66HeroKitPreviewSlot::Ultimate);
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandlePassivePreviewClicked()
{
	if (UT66HeroSelectionPreviewController* HeroPreviewController = GetOrCreatePreviewController())
	{
		HeroPreviewController->ToggleSelectedPreviewClip(ET66HeroSelectionPreviewClip::Passive, bShowingCompanionInfo);
	}
	return FReply::Handled();
}

void UT66HeroSelectionScreen::ApplyKitPreviewVideo(const ET66HeroKitPreviewSlot KitSlot)
{
	SelectedKitPreviewSlot = KitSlot;

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	FHeroData HeroData;
	if (!T66GI || !GetPreviewedHeroData(HeroData))
	{
		if (KitPreviewVideoPlayer)
		{
			KitPreviewVideoPlayer->CloseVideo();
		}
		if (KitPreviewColorBox.IsValid())
		{
			KitPreviewColorBox->SetBorderBackgroundColor(FLinearColor(0.025f, 0.027f, 0.035f, 1.f));
		}
		return;
	}

	const FHeroKitPreviewDisplay Display = ResolveHeroKitPreviewDisplay(T66GI, GetLocSubsystem(), HeroData, KitSlot);
	if (KitPreviewTitleWidget.IsValid())
	{
		KitPreviewTitleWidget->SetText(Display.Title);
	}
	if (KitPreviewDescriptionWidget.IsValid())
	{
		KitPreviewDescriptionWidget->SetText(Display.Description);
	}

	FName EffectiveSkinID = FName(TEXT("Default"));
	if (const UT66HeroSelectionPreviewController* HeroPreviewController = GetPreviewController())
	{
		EffectiveSkinID = HeroPreviewController->ResolveEffectiveHeroSkinID(T66GI);
	}
	else if (T66GI && !T66GI->SelectedHeroSkinID.IsNone())
	{
		EffectiveSkinID = T66GI->SelectedHeroSkinID;
	}

	FT66FrontendVideoAsset VideoAsset;
	const bool bResolved = T66FrontendVideoCatalog::ResolveHeroKitPreview(
		HeroData.HeroID,
		KitSlot,
		Display.KitID,
		EffectiveSkinID,
		SelectedBodyType,
		VideoAsset);
	bool bVideoOpened = false;
	if (bResolved)
	{
		if (!KitPreviewVideoPlayer)
		{
			KitPreviewVideoPlayer = NewObject<UT66FrontendVideoPlayer>(this);
		}
		if (KitPreviewVideoPlayer)
		{
			const FName DebugName = KitSlot == ET66HeroKitPreviewSlot::Weapon
				? FName(TEXT("HeroSelectionWeaponKitPreview"))
				: FName(TEXT("HeroSelectionUltimateKitPreview"));
			bVideoOpened = KitPreviewVideoPlayer->OpenVideo(
				VideoAsset,
				FVector2D(540.f, 96.f),
				DebugName);
		}
	}

	if (!bVideoOpened && KitPreviewVideoPlayer)
	{
		KitPreviewVideoPlayer->CloseVideo();
	}
	if (KitPreviewColorBox.IsValid())
	{
		KitPreviewColorBox->SetBorderBackgroundColor(bVideoOpened ? FLinearColor::Transparent : Display.FallbackColor);
	}
}

void UT66HeroSelectionScreen::RefreshKitPreviewPanelText()
{
	ApplyKitPreviewVideo(SelectedKitPreviewSlot);
}

const FSlateBrush* UT66HeroSelectionScreen::GetKitPreviewVideoBrush() const
{
	return KitPreviewVideoPlayer ? KitPreviewVideoPlayer->GetVideoBrush() : nullptr;
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
		static_cast<void>(TotalEntries);
		if (bRankSuccess && Rank > 0)
		{
			HeroRecordRankWidget->SetText(FormatHeroRecordRankText(Rank));
		}
		else
		{
			HeroRecordRankWidget->SetText(FormatHeroRecordRankText(0));
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

	SetImageBrushFromPath(
		GetHeroSelectionRankImagePath(),
		HeroRecordRankTexture,
		HeroRecordRankBrush,
		FVector2D(46.f, 46.f),
		TEXT("HeroSelectionRankIcon"));

	auto RefreshRecordValues = [this]()
	{
		RefreshHeroRecordRank();
	};

	auto UpdateTargetOptions = [this, Loc, T66GI]()
	{
		const FText NoCompanionText = Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.HeroSelection", "NoCompanionOption", "No Girlfriend");
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
		RefreshKitPreviewPanelText();

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
			const int32 UnionStages = (Achievements && bHasCompanion)
				? Achievements->GetCompanionUnionStagesCleared(PreviewedCompanionID)
				: 0;
			const float HealAmount = bHasCompanion
				? AT66CompanionBase::GetHealingAmountForDifficulty(SelectedDifficulty)
				: 0.f;
			const float HealIntervalSeconds = bHasCompanion
				? AT66CompanionBase::GetHealingIntervalSecondsForDifficulty(SelectedDifficulty)
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
						? FormatCompanionDifficultyHealText(HealAmount, HealIntervalSeconds)
						: (Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.HeroSelection", "NoCompanionInfo", "No girlfriend selected.")));
			}
			if (CompanionUnityTextWidget.IsValid())
			{
				CompanionUnityTextWidget->SetText(
					bHasCompanion
						? FText::Format(
							NSLOCTEXT("T66.HeroSelection", "CompanionUnityFormat", "Unity: {0} / {1}"),
							FText::AsNumber(CompanionUnityStagesCleared),
							FText::AsNumber(UT66AchievementsSubsystem::UnionTier_HyperStages))
					: NSLOCTEXT("T66.HeroSelection", "CompanionUnityNoSelection", "Select a girlfriend to view unity."));
			}
			RefreshRecordValues();
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
			RefreshRecordValues();
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
		RefreshRecordValues();
	}

	if (HeroPreviewController)
	{
		HeroPreviewController->ApplyHeroPreviewVideo(
			T66GI,
			PreviewedHeroID,
			PreviewedCompanionID,
			SelectedBodyType,
			SelectedDifficulty,
			bShowingCompanionInfo,
			HeroPreviewFallbackColor);
	}

	RefreshHeroCarouselPortraits();
	RefreshCompanionCarouselPortraits();
}

void UT66HeroSelectionScreen::RefreshHeroCarouselPortraits()
{
	if (AllHeroIDs.Num() <= 0)
	{
		HeroCarouselSlotColors.Init(FLinearColor(0.2f, 0.2f, 0.25f, 1.0f), HeroSelectionHeroCarouselVisibleSlots);
		HeroCarouselSlotVisibility.Init(EVisibility::Collapsed, HeroSelectionHeroCarouselVisibleSlots);
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

	HeroCarouselPortraitBrushes.SetNum(HeroSelectionHeroCarouselVisibleSlots);
	HeroCarouselSlotColors.SetNum(HeroSelectionHeroCarouselVisibleSlots);
	HeroCarouselSlotVisibility.SetNum(HeroSelectionHeroCarouselVisibleSlots);
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
		for (int32 Offset = -HeroSelectionHeroCarouselCenterIndex; Offset <= HeroSelectionHeroCarouselCenterIndex; ++Offset)
		{
			const int32 SlotIdx = Offset + HeroSelectionHeroCarouselCenterIndex;
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
