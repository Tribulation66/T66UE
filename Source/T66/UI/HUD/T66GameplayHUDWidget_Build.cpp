// Copyright Tribulation 66. All Rights Reserved.

#include "UI/HUD/T66GameplayHUDWidget_Private.h"

TSharedRef<SWidget> UT66GameplayHUDWidget::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	const UT66PlayerExperienceSubSystem* PlayerExperience = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	const ET66Difficulty SelectedDifficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const int32 InitialStage = GetRunState() ? GetRunState()->GetCurrentStage() : 1;
	const FText StageInit = BuildDisplayedStageText(
		Loc,
		PlayerExperience,
		SelectedDifficulty,
		InitialStage,
		false);
	const FText NetWorthInit = FText::AsNumber(0);
	const FText GoldInit = FText::AsNumber(0);
	const FText OweInit = FText::AsNumber(0);
	const FText ScoreLabelText = Loc ? Loc->GetText_ScoreLabel() : NSLOCTEXT("T66.GameplayHUD", "ScoreLabel", "Score:");
	const FText PortraitLabel = Loc ? Loc->GetText_PortraitPlaceholder() : NSLOCTEXT("T66.GameplayHUD", "PortraitLabel", "PORTRAIT");
	NetWorthText.Reset();
	PortraitStatPanelBox.Reset();
	PortraitPlaceholderText.Reset();
	DifficultyRowBox.Reset();
	CowardiceRowBox.Reset();
	BossPartBarRows.Reset();
	const bool bDotaTheme = FT66Style::IsDotaTheme();
	const FLinearColor SlotOuterColor = bDotaTheme ? FT66Style::SlotOuter() : FLinearColor(0.f, 0.f, 0.f, 0.25f);
	const FLinearColor SlotFrameColor = bDotaTheme ? FT66Style::SlotInner() : FLinearColor(0.45f, 0.55f, 0.50f, 0.5f);
	const FLinearColor SlotFillColor = bDotaTheme ? FT66Style::SlotFill() : FLinearColor(0.f, 0.f, 0.f, 0.f);
	const FLinearColor BossBarBackgroundColor = bDotaTheme ? FT66Style::BossBarBackground() : FLinearColor(0.08f, 0.08f, 0.08f, 0.9f);
	const FLinearColor BossBarFillColor = bDotaTheme ? FT66Style::BossBarFill() : FLinearColor(0.9f, 0.1f, 0.1f, 0.95f);
	const FLinearColor PromptBackgroundColor = bDotaTheme ? FT66Style::PromptBackground() : FLinearColor(0.02f, 0.02f, 0.03f, 0.65f);
	const FLinearColor DialogueBackgroundColor = bDotaTheme ? FT66Style::PromptBackground() : FLinearColor(0.06f, 0.06f, 0.08f, 0.85f);
	const int32 InventorySlotWidgetCount = UT66RunStateSubsystem::MaxInventorySlots;

	HeartBorders.SetNum(GT66DisplayedHeartCount);
	HeartFillBoxes.SetNum(GT66DisplayedHeartCount);
	HeartImages.SetNum(GT66DisplayedHeartCount);
	HeartTierBrushes.SetNum(5);
	DifficultyBorders.SetNum(5);
	DifficultyImages.SetNum(5);
	ClownImages.SetNum(5);
	IdolSlotBorders.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	IdolSlotContainers.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	IdolSlotImages.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	IdolSlotBrushes.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	IdolLevelDotBorders.Empty();
	CachedIdolSlotIDs.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	InventorySlotBorders.SetNum(InventorySlotWidgetCount);
	InventorySlotContainers.SetNum(InventorySlotWidgetCount);
	InventorySlotImages.SetNum(InventorySlotWidgetCount);
	InventorySlotBrushes.SetNum(InventorySlotWidgetCount);
	CachedInventorySlotIDs.SetNum(InventorySlotWidgetCount);
	ChestRewardCoinBoxes.SetNum(ChestRewardCoinCount);
	ChestRewardCoinImages.SetNum(ChestRewardCoinCount);
	StatusEffectDots.SetNum(3);
	StatusEffectDotBoxes.SetNum(3);
	WorldDialogueOptionBorders.SetNum(3);
	WorldDialogueOptionTexts.SetNum(3);
	static constexpr float BossBarWidth = 560.f;

	// Brushes for icons (kept alive by shared pointers).
	if (!LootPromptIconBrush.IsValid())
	{
		LootPromptIconBrush = MakeShared<FSlateBrush>();
		LootPromptIconBrush->DrawAs = ESlateBrushDrawType::Image;
		LootPromptIconBrush->ImageSize = FVector2D(28.f, 28.f);
	}
	if (!GoldCurrencyBrush.IsValid())
	{
		GoldCurrencyBrush = MakeShared<FSlateBrush>();
		GoldCurrencyBrush->DrawAs = ESlateBrushDrawType::Image;
		GoldCurrencyBrush->ImageSize = FVector2D(20.f, 20.f);
		GoldCurrencyBrush->Tiling = ESlateBrushTileType::NoTile;
		GoldCurrencyBrush->SetResourceObject(nullptr);
	}
	if (!DebtCurrencyBrush.IsValid())
	{
		DebtCurrencyBrush = MakeShared<FSlateBrush>();
		DebtCurrencyBrush->DrawAs = ESlateBrushDrawType::Image;
		DebtCurrencyBrush->ImageSize = FVector2D(20.f, 20.f);
		DebtCurrencyBrush->Tiling = ESlateBrushTileType::NoTile;
		DebtCurrencyBrush->SetResourceObject(nullptr);
	}
	BindRuntimeHudBrush(GoldCurrencyBrush, GetGoldCurrencyRelativePath(), FVector2D(20.f, 20.f));
	BindRuntimeHudBrush(DebtCurrencyBrush, GetDebtCurrencyRelativePath(), FVector2D(20.f, 20.f));
	if (!ChestRewardClosedBrush.IsValid())
	{
		ChestRewardClosedBrush = MakeShared<FSlateBrush>();
		ChestRewardClosedBrush->DrawAs = ESlateBrushDrawType::Image;
		ChestRewardClosedBrush->ImageSize = FVector2D(108.f, 108.f);
		ChestRewardClosedBrush->Tiling = ESlateBrushTileType::NoTile;
		ChestRewardClosedBrush->SetResourceObject(nullptr);
	}
	if (!ChestRewardOpenBrush.IsValid())
	{
		ChestRewardOpenBrush = MakeShared<FSlateBrush>();
		ChestRewardOpenBrush->DrawAs = ESlateBrushDrawType::Image;
		ChestRewardOpenBrush->ImageSize = FVector2D(108.f, 108.f);
		ChestRewardOpenBrush->Tiling = ESlateBrushTileType::NoTile;
		ChestRewardOpenBrush->SetResourceObject(nullptr);
	}
	if (!PortraitBrush.IsValid())
	{
		PortraitBrush = MakeShared<FSlateBrush>();
		PortraitBrush->DrawAs = ESlateBrushDrawType::Image;
		PortraitBrush->ImageSize = FVector2D(GT66BottomLeftPortraitPanelSize, GT66BottomLeftPortraitPanelSize);
	}
	if (!UltimateBrush.IsValid())
	{
		UltimateBrush = MakeShared<FSlateBrush>();
		UltimateBrush->DrawAs = ESlateBrushDrawType::Image;
		UltimateBrush->ImageSize = FVector2D(GT66BottomLeftAbilityBoxSize, GT66BottomLeftAbilityBoxSize);
		UltimateBrush->Tiling = ESlateBrushTileType::NoTile;
		UltimateBrush->SetResourceObject(nullptr);
	}
	if (!PassiveBrush.IsValid())
	{
		PassiveBrush = MakeShared<FSlateBrush>();
		PassiveBrush->DrawAs = ESlateBrushDrawType::Image;
		PassiveBrush->ImageSize = FVector2D(GT66BottomLeftAbilityBoxSize, GT66BottomLeftAbilityBoxSize);
		PassiveBrush->Tiling = ESlateBrushTileType::NoTile;
		PassiveBrush->SetResourceObject(nullptr);
	}
	// Load fallback ability textures via the texture pool.
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> UltSoft = ResolveGameplayUltimateIcon(NAME_None, ET66UltimateType::None);
			T66SlateTexture::BindSharedBrushAsync(TexPool, UltSoft, this, UltimateBrush, FName(TEXT("HUDUltimate")), false);
			const TSoftObjectPtr<UTexture2D> PassiveSoft = ResolveGameplayPassiveIcon(NAME_None, ET66PassiveType::None);
			T66SlateTexture::BindSharedBrushAsync(TexPool, PassiveSoft, this, PassiveBrush, FName(TEXT("HUDPassive")), false);
		}
	}
	// Wheel spin texture
	{
		WheelTextureBrush = FSlateBrush();
		WheelTextureBrush.ImageSize = FVector2D(124.f, 124.f);
		WheelTextureBrush.DrawAs = ESlateBrushDrawType::Image;
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> WheelSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Wheel/Firefly_Gemini_Flash_Remove_background_286654.Firefly_Gemini_Flash_Remove_background_286654")));
			T66SlateTexture::BindBrushAsync(TexPool, WheelSoft, this, WheelTextureBrush, FName(TEXT("HUDWheel")), /*bClearWhileLoading*/ true);
		}
	}
	// Heart sprite brush
	if (!HeartBrush.IsValid())
	{
		HeartBrush = MakeShared<FSlateBrush>();
		HeartBrush->DrawAs = ESlateBrushDrawType::Image;
		HeartBrush->ImageSize = FVector2D(GT66DisplayedHeartWidth, GT66DisplayedHeartHeight);
		HeartBrush->Tiling = ESlateBrushTileType::NoTile;
		HeartBrush->SetResourceObject(nullptr);
	}
	if (!HeartBlessingBrush.IsValid())
	{
		HeartBlessingBrush = MakeShared<FSlateBrush>();
		HeartBlessingBrush->DrawAs = ESlateBrushDrawType::Image;
		HeartBlessingBrush->ImageSize = FVector2D(GT66DisplayedHeartWidth, GT66DisplayedHeartHeight);
		HeartBlessingBrush->Tiling = ESlateBrushTileType::NoTile;
		HeartBlessingBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> HeartSoft = ResolveGameplayHeartIcon(0);
			T66SlateTexture::BindSharedBrushAsync(TexPool, HeartSoft, this, HeartBrush, FName(TEXT("HUDHeartFallback")), false);
			T66SlateTexture::BindSharedBrushAsync(TexPool, ResolveGameplayBlessingHeartIcon(), this, HeartBlessingBrush, FName(TEXT("HUDHeartBlessing")), false);
			for (int32 TierIndex = 0; TierIndex < HeartTierBrushes.Num(); ++TierIndex)
			{
				if (!HeartTierBrushes[TierIndex].IsValid())
				{
					HeartTierBrushes[TierIndex] = MakeShared<FSlateBrush>();
					HeartTierBrushes[TierIndex]->DrawAs = ESlateBrushDrawType::Image;
					HeartTierBrushes[TierIndex]->ImageSize = FVector2D(GT66DisplayedHeartWidth, GT66DisplayedHeartHeight);
					HeartTierBrushes[TierIndex]->Tiling = ESlateBrushTileType::NoTile;
					HeartTierBrushes[TierIndex]->SetResourceObject(nullptr);
				}

				T66SlateTexture::BindSharedBrushAsync(
					TexPool,
					ResolveGameplayHeartIcon(TierIndex),
					this,
					HeartTierBrushes[TierIndex],
					FName(*FString::Printf(TEXT("HUDHeart_%d"), TierIndex)),
					false);
			}
		}
	}
	if (!QuickReviveBrush.IsValid())
	{
		QuickReviveBrush = MakeShared<FSlateBrush>();
		QuickReviveBrush->DrawAs = ESlateBrushDrawType::Image;
		QuickReviveBrush->ImageSize = FVector2D(26.f, 26.f);
		QuickReviveBrush->Tiling = ESlateBrushTileType::NoTile;
		QuickReviveBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> QuickReviveSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/Interactables/QuickReviveIcon.QuickReviveIcon")));
			T66SlateTexture::BindSharedBrushAsync(TexPool, QuickReviveSoft, this, QuickReviveBrush, FName(TEXT("HUDQuickRevive")), false);
		}
	}

	auto ConfigureCrispUISprite = [](UTexture2D* Tex)
	{
		if (!Tex)
		{
			return;
		}

		Tex->bForceMiplevelsToBeResident = true;
		Tex->NeverStream = true;
		Tex->Filter = TextureFilter::TF_Nearest;
		Tex->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		Tex->UpdateResource();
	};
	// Skull sprite brush
	if (!SkullBrush.IsValid())
	{
		SkullBrush = MakeShared<FSlateBrush>();
		SkullBrush->DrawAs = ESlateBrushDrawType::Image;
		SkullBrush->ImageSize = FVector2D(38.f, 38.f);
		SkullBrush->Tiling = ESlateBrushTileType::NoTile;
		SkullBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> SkullSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/SKULL.SKULL")));
			T66SlateTexture::BindSharedBrushAsync(TexPool, SkullSoft, this, SkullBrush, FName(TEXT("HUDSkull")), false);
			if (UTexture2D* LoadedSkull = TexPool->GetLoadedTexture(SkullSoft))
			{
				ConfigureCrispUISprite(LoadedSkull);
			}
			TexPool->RequestTexture(SkullSoft, this, FName(TEXT("HUDSkullConfig")), [ConfigureCrispUISprite](UTexture2D* LoadedSkull)
			{
				ConfigureCrispUISprite(LoadedSkull);
			});
		}
	}
	// Clown sprite brush (cowardice gates taken; same size as skull).
	if (!ClownBrush.IsValid())
	{
		ClownBrush = MakeShared<FSlateBrush>();
		ClownBrush->DrawAs = ESlateBrushDrawType::Image;
		ClownBrush->ImageSize = FVector2D(38.f, 38.f);
		ClownBrush->Tiling = ESlateBrushTileType::NoTile;
		ClownBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> ClownSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/CLOWN.CLOWN")));
			T66SlateTexture::BindSharedBrushAsync(TexPool, ClownSoft, this, ClownBrush, FName(TEXT("HUDClown")), false);
			if (UTexture2D* LoadedClown = TexPool->GetLoadedTexture(ClownSoft))
			{
				ConfigureCrispUISprite(LoadedClown);
			}
			TexPool->RequestTexture(ClownSoft, this, FName(TEXT("HUDClownConfig")), [ConfigureCrispUISprite](UTexture2D* LoadedClown)
			{
				ConfigureCrispUISprite(LoadedClown);
			});
		}
	}
	for (int32 i = 0; i < IdolSlotBrushes.Num(); ++i)
	{
		IdolSlotBrushes[i] = MakeShared<FSlateBrush>();
		IdolSlotBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		IdolSlotBrushes[i]->ImageSize = FVector2D(GT66BottomLeftIdolSlotSize, GT66BottomLeftIdolSlotSize);
	}
	for (int32 i = 0; i < InventorySlotBrushes.Num(); ++i)
	{
		InventorySlotBrushes[i] = MakeShared<FSlateBrush>();
		InventorySlotBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
		// Size is assigned below where InvSlotSize is known; keep a safe default now.
		InventorySlotBrushes[i]->ImageSize = FVector2D(36.f, 36.f);
	}

	// Difficulty row (5-slot skull sprites).
	static constexpr float MinimapWidth = MinimapPanelWidth;
	static constexpr float DiffGap = 1.f;
	static constexpr float DiffSize = 30.f;
	TSharedRef<SHorizontalBox> DifficultyRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < DifficultyBorders.Num(); ++i)
	{
		TSharedPtr<SImage> DiffImg;
		DifficultyRowRef->AddSlot()
			.AutoWidth()
			.Padding(i == 0 ? 0.f : DiffGap, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(DiffSize)
				.HeightOverride(DiffSize)
				[
					SAssignNew(DiffImg, SImage)
					.Image(SkullBrush.Get())
					.ColorAndOpacity(FLinearColor::White)
					.Visibility(EVisibility::Collapsed) // Start hidden; skulls appear one-by-one
				]
			];
		DifficultyImages[i] = DiffImg;
	}

	// Cowardice row (5-slot clown sprites, below skulls; one per cowardice gate taken).
	TSharedRef<SHorizontalBox> CowardiceRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < ClownImages.Num(); ++i)
	{
		TSharedPtr<SImage> ClownImg;
		CowardiceRowRef->AddSlot()
			.AutoWidth()
			.Padding(i == 0 ? 0.f : DiffGap, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(DiffSize)
				.HeightOverride(DiffSize)
				[
					SAssignNew(ClownImg, SImage)
					.Image(ClownBrush.Get())
					.ColorAndOpacity(FLinearColor::White)
					.Visibility(EVisibility::Collapsed)
				]
			];
		ClownImages[i] = ClownImg;
	}

	// Build a 10-heart display as two rows of five segments.
	static constexpr float HeartWidth = GT66DisplayedHeartWidth;
	static constexpr float HeartHeight = GT66DisplayedHeartHeight;
	static constexpr float HeartPad = GT66DisplayedHeartColumnGap;
	static constexpr float HeartRowGap = GT66DisplayedHeartRowGap;
	static constexpr float TopStripPanelHeight = GT66BottomLeftAbilityBoxSize;
	TSharedRef<SHorizontalBox> TopHeartsRowRef = SNew(SHorizontalBox);
	TSharedRef<SHorizontalBox> BottomHeartsRowRef = SNew(SHorizontalBox);
	for (int32 i = 0; i < GT66DisplayedHeartCount; ++i)
	{
		TSharedPtr<SBox> HeartFillBox;
		TSharedPtr<SImage> HeartImg;
		TSharedRef<SHorizontalBox> TargetRow = (i < 5) ? TopHeartsRowRef : BottomHeartsRowRef;
		TargetRow->AddSlot()
			.FillWidth(1.f)
			.Padding((i % 5) > 0 ? HeartPad : 0.f, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(HeartWidth)
				.HeightOverride(HeartHeight)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Image(HeartBrush.Get())
						.ColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.28f, 0.35f))
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					[
						SAssignNew(HeartFillBox, SBox)
						.WidthOverride(HeartWidth)
						.HeightOverride(HeartHeight)
						.Clipping(EWidgetClipping::ClipToBounds)
						[
							SNew(SBox)
							.WidthOverride(HeartWidth)
							.HeightOverride(HeartHeight)
							[
								SAssignNew(HeartImg, SImage)
								.Image(HeartBrush.Get())
								.ColorAndOpacity(FLinearColor::White)
							]
						]
					]
				]
			];
		HeartFillBoxes[i] = HeartFillBox;
		HeartImages[i] = HeartImg;
	}
	TSharedRef<SWidget> HeartsRowRef =
		SNew(SBox)
		.WidthOverride(GT66DisplayedHeartAreaWidth)
		.HeightOverride(GT66DisplayedHeartAreaHeight)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Fill)
			[
				TopHeartsRowRef
			]
			+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Fill).Padding(0.f, HeartRowGap, 0.f, 0.f)
			[
				BottomHeartsRowRef
			]
		];

	TSharedRef<SWidget> QuickReviveIconRowRef =
		SAssignNew(QuickReviveIconRowBox, SBox)
		.Visibility(EVisibility::Collapsed)
		.WidthOverride(GT66BottomLeftAbilityBoxSize)
		.HeightOverride(GT66BottomLeftAbilityBoxSize)
		[
			SAssignNew(QuickReviveIconImage, SImage)
			.Image(QuickReviveBrush.Get())
			.ColorAndOpacity(FLinearColor::White)
		];

	// Status effect dots row (above hearts): burn / chill / curse
	TSharedRef<SHorizontalBox> StatusDotsRowRef =
		SNew(SHorizontalBox)
		.Visibility(EVisibility::Collapsed);
	for (int32 i = 0; i < 3; ++i)
	{
		TSharedPtr<SBox> DotBox;
		TSharedPtr<ST66DotWidget> Dot;
		StatusDotsRowRef->AddSlot()
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SAssignNew(DotBox, SBox)
				.WidthOverride(8.f)
				.HeightOverride(8.f)
				.Visibility(EVisibility::Collapsed)
				[
					SAssignNew(Dot, ST66DotWidget)
				]
			];
		StatusEffectDotBoxes[i] = DotBox;
		StatusEffectDots[i] = Dot;
	}

	// Idol slots: 2x2 grid sized to match the stats panel footprint.
	TSharedRef<SGridPanel> IdolSlotsRef = SNew(SGridPanel);
	static constexpr int32 IdolColumns = 2;
	static constexpr float IdolSlotPad = GT66BottomLeftIdolSlotPad;
	static constexpr float IdolSlotSize = GT66BottomLeftIdolSlotSize;
	const float IdolPanelMinWidth = GT66BottomLeftSidePanelWidth;
	const float PortraitPanelSize = GT66BottomLeftPortraitPanelSize;
	const float InventoryPanelVisibleWidth = BottomRightInventoryPanelWidth;
	const float InventoryPanelVisibleHeight = BottomRightInventoryPanelHeight;
	const float AbilityColumnWidth = GT66BottomLeftAbilityBoxSize;
	const float AbilityIconSize = GT66BottomLeftAbilityBoxSize;
	const float AbilityLabelBarHeight = 16.f;
	const float AbilityInputBadgeWidth = 28.f;
	const float AbilityInputBadgeHeight = 18.f;
	const float AbilityIconInset = 6.f;
	const float BottomLeftColumnGap = 0.f;
	const FLinearColor BottomLeftPanelOuterColor(0.f, 0.f, 0.f, 0.95f);
	const FLinearColor BottomLeftPanelInnerColor(0.f, 0.f, 0.f, 1.f);
	const FLinearColor BottomLeftPanelTitleColor = FT66Style::TextMuted();
	const FLinearColor BottomLeftPanelDividerColor = FT66Style::Border() * FLinearColor(1.f, 1.f, 1.f, 0.6f);
	const FLinearColor IdolSectionBorderColor(0.45f, 0.63f, 0.78f, 0.95f);
	const FLinearColor PortraitSectionBorderColor(0.72f, 0.58f, 0.32f, 0.95f);
	const FLinearColor AbilitySectionBorderColor(0.54f, 0.74f, 0.94f, 0.95f);
	const FLinearColor HeartsSectionBorderColor(0.72f, 0.34f, 0.34f, 0.95f);
	const FLinearColor SharedSectionFillColor(0.03f, 0.03f, 0.04f, 1.f);
	const FLinearColor LevelTextColor = bDotaTheme ? FT66Style::Accent2() : FLinearColor(0.90f, 0.75f, 0.20f, 1.f);
	TSharedRef<SWidget> LevelBadgeRef =
		SNew(SBox)
		.WidthOverride(GT66BottomLeftLevelBadgeSize)
		.HeightOverride(GT66BottomLeftLevelBadgeSize)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SAssignNew(LevelRingWidget, ST66RingWidget)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(LevelText, STextBlock)
				.Text(FText::AsNumber(1))
				.Font(FT66Style::Tokens::FontBold(11))
				.ColorAndOpacity(LevelTextColor)
				.Justification(ETextJustify::Center)
			]
		];
	auto MakeBottomLeftBlackPanel = [&](const FText& Title, const TSharedRef<SWidget>& Content, const FMargin& InnerPadding) -> TSharedRef<SWidget>
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BottomLeftPanelOuterColor)
				.Padding(3.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BottomLeftPanelInnerColor)
					.Padding(InnerPadding)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 2.f)
						[
							SNew(STextBlock)
							.Text(Title)
							.Font(FT66Style::Tokens::FontBold(13))
							.ColorAndOpacity(BottomLeftPanelTitleColor)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(4.f, 0.f, 4.f, 3.f)
						[
							SNew(SBox)
							.HeightOverride(1.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(BottomLeftPanelDividerColor)
							]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							Content
						]
					]
				]
			];
	};
	auto MakeBottomLeftBlackPanelNoTitle = [&](const TSharedRef<SWidget>& Content, const FMargin& InnerPadding) -> TSharedRef<SWidget>
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BottomLeftPanelOuterColor)
				.Padding(3.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BottomLeftPanelInnerColor)
					.Padding(InnerPadding)
					[
						Content
					]
				]
			];
	};
	auto MakeBottomLeftSectionPanel = [&](const TSharedRef<SWidget>& Content, const FMargin& InnerPadding, const FLinearColor& BorderColor) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(BorderColor)
			.Padding(2.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(SharedSectionFillColor)
				.Padding(InnerPadding)
				[
					Content
				]
			];
	};
	auto MakeCurrencyReadout = [&](const TSharedPtr<FSlateBrush>& IconBrush, TSharedPtr<STextBlock>& OutValueText, const FText& InitialValue, const FLinearColor& ValueColor) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(18.f)
				.HeightOverride(18.f)
				[
					SNew(SImage)
					.Image(IconBrush.Get())
					.ColorAndOpacity(FLinearColor::White)
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SAssignNew(OutValueText, STextBlock)
				.Text(InitialValue)
				.Font(FT66Style::Tokens::FontBold(13))
				.ColorAndOpacity(ValueColor)
			];
	};
	for (int32 i = 0; i < UT66IdolManagerSubsystem::MaxEquippedIdolSlots; ++i)
	{
		TSharedPtr<SBorder> IdolBorder;
		const int32 Row = i / IdolColumns;
		const int32 Col = i % IdolColumns;
		IdolSlotsRef->AddSlot(Col, Row)
			.Padding(IdolSlotPad)
			[
				SAssignNew(IdolSlotContainers[i], SBox)
				.WidthOverride(IdolSlotSize)
				.HeightOverride(IdolSlotSize)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
					SAssignNew(IdolBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(SlotOuterColor)
					.Padding(1.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(SlotFrameColor)
							.Padding(1.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(SlotFillColor)
							]
						]
						+ SOverlay::Slot()
						.VAlign(VAlign_Top)
						.Padding(1.f, 1.f, 1.f, 0.f)
						[
							SNew(SBox)
							.HeightOverride(2.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.97f, 1.0f, bDotaTheme ? 0.12f : 0.08f))
							]
						]
						+ SOverlay::Slot()
						.VAlign(VAlign_Bottom)
						.Padding(1.f, 0.f, 1.f, 1.f)
						[
							SNew(SBox)
							.HeightOverride(2.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.42f))
							]
						]
					]
					]
					+ SOverlay::Slot()
					[
						SAssignNew(IdolSlotImages[i], SImage)
						.Image(IdolSlotBrushes.IsValidIndex(i) && IdolSlotBrushes[i].IsValid() ? IdolSlotBrushes[i].Get() : nullptr)
						.ColorAndOpacity(FLinearColor::White)
						.Visibility(EVisibility::Collapsed)
					]
				]
			];
		IdolSlotBorders[i] = IdolBorder;
	}

	// Inventory: fixed two-row strip, stretched edge-to-edge so the full inventory fits without scrolling.
	static constexpr int32 InvCols = 10;
	const int32 InvRows = FMath::Max(1, FMath::DivideAndRoundUp(InventorySlotBorders.Num(), InvCols));
	const float InvSlotSize = BottomRightInventorySlotSize;
	static constexpr float InvSlotPad = 2.f;
	const FLinearColor InvSlotBorderColor = SlotFrameColor;
	for (int32 i = 0; i < InventorySlotBrushes.Num(); ++i)
	{
		if (InventorySlotBrushes[i].IsValid())
		{
			InventorySlotBrushes[i]->ImageSize = FVector2D(InvSlotSize - 4.f, InvSlotSize - 4.f);
		}
	}
	TSharedRef<SVerticalBox> InvGridRows = SNew(SVerticalBox);
	int32 SlotIndex = 0;
	for (int32 Row = 0; Row < InvRows; ++Row)
	{
		TSharedRef<SHorizontalBox> RowBox = SNew(SHorizontalBox);
		for (int32 Col = 0; Col < InvCols; ++Col)
		{
			if (!InventorySlotContainers.IsValidIndex(SlotIndex))
			{
				break;
			}
			TSharedPtr<SBorder> SlotBorder;
			TSharedPtr<SImage> SlotImage;
			const int32 ThisSlotIndex = SlotIndex;
			RowBox->AddSlot()
				.FillWidth(1.f)
				.Padding(InvSlotPad, InvSlotPad)
				[
					SAssignNew(InventorySlotContainers[ThisSlotIndex], SBox)
					.HeightOverride(InvSlotSize)
					[
						SNew(SOverlay)
						// Transparent slot bg with thin border outline
						+ SOverlay::Slot()
						[
							SAssignNew(SlotBorder, SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(SlotOuterColor)
							.Padding(1.f)
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(InvSlotBorderColor)
									.Padding(1.f)
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(SlotFillColor)
									]
								]
								+ SOverlay::Slot()
								.VAlign(VAlign_Top)
								.Padding(1.f, 1.f, 1.f, 0.f)
								[
									SNew(SBox)
									.HeightOverride(2.f)
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(FLinearColor(0.95f, 0.97f, 1.0f, bDotaTheme ? 0.12f : 0.08f))
									]
								]
								+ SOverlay::Slot()
								.VAlign(VAlign_Bottom)
								.Padding(1.f, 0.f, 1.f, 1.f)
								[
									SNew(SBox)
									.HeightOverride(2.f)
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.42f))
									]
								]
							]
						]
						// Item icon on top
						+ SOverlay::Slot()
						.Padding(2.f)
						[
							SAssignNew(SlotImage, SImage)
							.Image(InventorySlotBrushes.IsValidIndex(ThisSlotIndex) && InventorySlotBrushes[ThisSlotIndex].IsValid()
								? InventorySlotBrushes[ThisSlotIndex].Get()
								: nullptr)
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				];

			if (InventorySlotBorders.IsValidIndex(ThisSlotIndex))
			{
				InventorySlotBorders[ThisSlotIndex] = SlotBorder;
			}
			if (InventorySlotImages.IsValidIndex(ThisSlotIndex))
			{
				InventorySlotImages[ThisSlotIndex] = SlotImage;
			}
			SlotIndex++;
		}
		InvGridRows->AddSlot().AutoHeight().HAlign(HAlign_Fill)[ RowBox ];
	}
	TSharedRef<SWidget> InvGridRef =
		SNew(SBox)
		.HAlign(HAlign_Fill)
		[
			InvGridRows
		];
	auto MakeInventoryEconomySection = [&](const FLinearColor& GoldValueColor, const FLinearColor& DebtValueColor, const FLinearColor& NetWorthValueColor, const FLinearColor& DividerColor) -> TSharedRef<SWidget>
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(2.f, 0.f, 2.f, 4.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 14.f, 0.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.GameplayHUD", "InventoryHeader", "Inventory"))
					.Font(FT66Style::Tokens::FontBold(12))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
				[
					MakeCurrencyReadout(GoldCurrencyBrush, GoldText, GoldInit, GoldValueColor)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
				[
					MakeCurrencyReadout(DebtCurrencyBrush, DebtText, OweInit, DebtValueColor)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					MakeCurrencyReadout(DebtCurrencyBrush, NetWorthText, NetWorthInit, NetWorthValueColor)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SSpacer)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				SNew(SBox)
				.HeightOverride(1.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(DividerColor)
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				InvGridRef
			];
	};

	const TAttribute<FMargin> TopCenterBossPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 12.f, 0.f, 0.f);
	});

	const TAttribute<FMargin> TopCenterLootPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 48.f, 0.f, 0.f);
	});

	const TAttribute<FMargin> TopLeftHudPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(12.f, 12.f, 0.f, 0.f);
	});

	const TAttribute<FMargin> TopRightHudPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 12.f, 12.f, 0.f);
	});

	const TAttribute<FMargin> PauseLeftStatsPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(12.f, 112.f, 0.f, 0.f);
	});

	const TAttribute<FMargin> PauseRightAchievementsPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 286.f, 12.f, 0.f);
	});

	const TAttribute<FMargin> RightCenterHudPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 0.f, 12.f, 0.f);
	});

	const TAttribute<FMargin> BottomLeftHudPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(12.f, 0.f, 0.f, 12.f);
	});

	const TAttribute<FMargin> BottomLeftMediaPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(12.f, 0.f, 0.f, GT66MediaBottomOffset);
	});

	const TAttribute<FMargin> BottomRightInventoryPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 0.f, 12.f, 0.f);
	});

	const TAttribute<FMargin> BottomCenterAchievementPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(0.f, 0.f, 0.f, 36.f);
	});

	const TAttribute<FMargin> BottomRightPickupPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FMargin(
			0.f,
			0.f,
			12.f,
			UT66GameplayHUDWidget::BottomRightInventoryPanelHeight + UT66GameplayHUDWidget::BottomRightPresentationGap);
	});

	const TAttribute<FOptionalSize> FullMapWidthAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66Style::GetSafeFrameSize();
		const float Width = FMath::Clamp(SafeFrame.X - 72.f, 720.f, 1100.f);
		return FOptionalSize(Width);
	});

	const TAttribute<FOptionalSize> FullMapHeightAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66Style::GetSafeFrameSize();
		const float Height = FMath::Clamp(SafeFrame.Y - 88.f, 420.f, 680.f);
		return FOptionalSize(Height);
	});

	TSharedRef<SOverlay> ChestRewardArtOverlay = SNew(SOverlay);
	ChestRewardArtOverlay->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SAssignNew(ChestRewardClosedBox, SBox)
		.WidthOverride(104.f)
		.HeightOverride(104.f)
		[
			SAssignNew(ChestRewardClosedImage, SImage)
			.Image(ChestRewardClosedBrush.Get())
			.ColorAndOpacity(FLinearColor::White)
		]
	];
	ChestRewardArtOverlay->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
		[
			SAssignNew(ChestRewardOpenBox, SBox)
			.WidthOverride(112.f)
			.HeightOverride(112.f)
			[
				SAssignNew(ChestRewardOpenImage, SImage)
				.Image(ChestRewardOpenBrush.Get())
				.ColorAndOpacity(FLinearColor::White)
			]
		];
	for (int32 CoinIndex = 0; CoinIndex < ChestRewardCoinCount; ++CoinIndex)
	{
		ChestRewardArtOverlay->AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(ChestRewardCoinBoxes[CoinIndex], SBox)
			.WidthOverride(18.f)
			.HeightOverride(18.f)
			.Visibility(EVisibility::Collapsed)
			[
				SAssignNew(ChestRewardCoinImages[CoinIndex], SImage)
				.Image(GoldCurrencyBrush.Get())
				.ColorAndOpacity(FLinearColor::White)
			]
		];
	}

	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(PauseBackdropBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.96f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(TopCenterBossPadding)
		[
			SAssignNew(BossBarContainerBox, SBox)
			.WidthOverride(BossBarWidth)
			.Visibility(EVisibility::Collapsed)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(BossBarBackgroundColor)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(BossBarFillBox, SBox)
						.HeightOverride(28.f)
						.WidthOverride(BossBarWidth)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(BossBarFillColor)
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(BossBarText, STextBlock)
						.Text(FText::Format(
							NSLOCTEXT("T66.Common", "Fraction", "{0}/{1}"),
							FText::AsNumber(100),
							FText::AsNumber(100)))
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 4.f, 0.f, 0.f)
				[
					SAssignNew(BossPartBarsBox, SVerticalBox)
					.Visibility(EVisibility::Collapsed)
				]
			]
		]
		// Top-center loot prompt (non-blocking)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(TopCenterLootPadding)
		[
			SAssignNew(LootPromptBox, SBox)
			.WidthOverride(760.f)
			.HeightOverride(40.f)
			.Visibility(EVisibility::Collapsed)
			[
				SAssignNew(LootPromptBorder, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(PromptBackgroundColor)
				.Padding(10.f, 6.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(28.f)
						.HeightOverride(28.f)
						[
							SAssignNew(LootPromptIconImage, SImage)
							.Image(LootPromptIconBrush.Get())
							.ColorAndOpacity(FLinearColor::White)
							.Visibility(EVisibility::Collapsed)
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SAssignNew(LootPromptText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(14))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				]
			]
		]
		// (Central countdown timer removed — stage timer info available in top-left stats)
		// In-world NPC dialogue (positioned via RenderTransform; hidden by default)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		[
			SAssignNew(WorldDialogueBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.RenderTransform(FSlateRenderTransform(FVector2D(0.f, 0.f)))
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(WorldDialogueOptionBorders[0], SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(DialogueBackgroundColor)
						.Padding(10.f, 6.f)
						[
							SAssignNew(WorldDialogueOptionTexts[0], STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
					[
						SAssignNew(WorldDialogueOptionBorders[1], SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(DialogueBackgroundColor)
						.Padding(10.f, 6.f)
						[
							SAssignNew(WorldDialogueOptionTexts[1], STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
					[
						SAssignNew(WorldDialogueOptionBorders[2], SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(DialogueBackgroundColor)
						.Padding(10.f, 6.f)
						[
							SAssignNew(WorldDialogueOptionTexts[2], STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
				,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(12.f)
			)
			]
		]
		// Media viewer panel. Source is selected from Settings.
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(PauseLeftStatsPadding)
		[
			SAssignNew(PauseStatsPanelBox, SBox)
			.Visibility(EVisibility::Collapsed)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(PauseRightAchievementsPadding)
		[
			SAssignNew(PauseAchievementsPanelBox, SBox)
			.Visibility(EVisibility::Collapsed)
		]
		// Media viewer panel. Source is selected from Settings.
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(BottomLeftMediaPadding)
		[
			SAssignNew(TikTokPlaceholderBox, SBox)
			.WidthOverride(GT66MediaPanelW)
			.HeightOverride(GT66MediaPanelH)
			[
				FT66Style::MakePanel(
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 0.55f))
					.Padding(2.f)
					[
						SAssignNew(MediaViewerVideoBox, SBox)
						[
							SAssignNew(TikTokContentBox, SBox)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 1.f))
							]
						]
					]
				,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(6.f)
				)
			]
		]
		// Top-left stats (score + speedrun time) — themed panel and text
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(TopLeftHudPadding)
		[
			bDotaTheme
				? FT66Style::MakeHudPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(ScoreLabelText)
							.Font(FT66Style::Tokens::FontBold(10))
							.ColorAndOpacity(FSlateColor(FT66Style::Text()))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
						[
							SAssignNew(ScoreText, STextBlock)
							.Text(FText::AsNumber(0))
							.Font(FT66Style::Tokens::FontBold(10))
							.ColorAndOpacity(FSlateColor(FT66Style::Text()))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(ScorePacingText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScorePacingDefault", "Score Pace --"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(ScoreTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScoreTargetDefault", "Score to Beat --"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunDefault", "Time 0:00.00"))
						.Font(FT66Style::Tokens::FontBold(10))
						.ColorAndOpacity(FSlateColor(FT66Style::Text()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunPacingText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "TimePacingDefault", "Time Pace --:--.--"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetDefault", "Time to Beat --:--.--"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					],
					FMargin(6.f, 4.f))
				: FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(ScoreLabelText)
							.Font(FT66Style::Tokens::FontBold(10))
							.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SAssignNew(ScoreText, STextBlock)
							.Text(FText::AsNumber(0))
							.Font(FT66Style::Tokens::FontBold(10))
							.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
					[
						SAssignNew(ScorePacingText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScorePacingDefault", "Score Pace --"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::TextMuted))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(ScoreTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScoreTargetDefault", "Score to Beat --"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::TextMuted))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunDefault", "Time 0:00.00"))
						.Font(FT66Style::Tokens::FontBold(10))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::Text))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunPacingText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "TimePacingDefault", "Time Pace --:--.--"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::TextMuted))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetDefault", "Time to Beat --:--.--"))
						.Font(FT66Style::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66Style::Tokens::TextMuted))
						.Visibility(EVisibility::Collapsed)
					],
					FT66PanelParams(ET66PanelType::Panel).SetPadding(6.f)
				)
		]

		// Wheel spin HUD animation (same lane/layout as chest reward)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(BottomRightPickupPadding)
		[
			SAssignNew(WheelSpinBox, SBox)
			.WidthOverride(PickupCardWidth)
			.HeightOverride(PickupCardHeight)
			.Visibility(EVisibility::Collapsed)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.24f, 0.24f, 0.28f, 1.f))
				.Padding(2.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.f))
					.Padding(0.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor::Black)
							.Padding(FMargin(8.f, 6.f))
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
									[
										SNew(SBox)
										.WidthOverride(18.f)
										.HeightOverride(18.f)
										[
											SNew(SImage)
											.Image(GoldCurrencyBrush.Get())
											.ColorAndOpacity(FLinearColor::White)
										]
									]
									+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
									[
										SAssignNew(WheelSpinText, STextBlock)
										.Text(FText::GetEmpty())
										.Font(FT66Style::Tokens::FontBold(18))
										.ColorAndOpacity(FLinearColor(0.98f, 0.83f, 0.24f, 1.f))
										.Justification(ETextJustify::Center)
									]
								]
							]
						]
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.05f, 1.f))
							.Padding(FMargin(6.f))
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SBox)
									.WidthOverride(124.f)
									.HeightOverride(124.f)
									[
										SAssignNew(WheelSpinDisk, SImage)
										.Image(&WheelTextureBrush)
									]
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 1.f))
							.Padding(FMargin(8.f, 4.f))
							[
								SAssignNew(WheelSpinSkipText, STextBlock)
								.Text(FText::GetEmpty())
								.Font(FT66Style::Tokens::FontBold(11))
								.ColorAndOpacity(FLinearColor::White)
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
			]
		]

		// Bottom-left portrait stack with a tighter, uniform panel footprint.
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(BottomLeftHudPadding)
		[
			SAssignNew(BottomLeftHUDBox, SBox)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Bottom)
				[
					SAssignNew(IdolSlotsPanelBox, SBox)
					[
						MakeBottomLeftBlackPanel(
							NSLOCTEXT("T66.GameplayHUD", "IdolsHeader", "IDOLS"),
							SNew(SBox)
							.WidthOverride(IdolPanelMinWidth)
							[
								MakeBottomLeftSectionPanel(
									SNew(SBox)
									.HAlign(HAlign_Center)
									[
										IdolSlotsRef
									],
									FMargin(3.f),
									IdolSectionBorderColor)
							],
							FMargin(2.f, 2.f))
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(BottomLeftColumnGap, 0.f, 0.f, 0.f)
				.VAlign(VAlign_Bottom)
				[
					SAssignNew(PortraitStatPanelBox, SBox)
					[
						MakeBottomLeftBlackPanelNoTitle(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SBox)
								.WidthOverride(PortraitPanelSize)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot().AutoHeight()
									[
										SNew(SBox)
										.WidthOverride(PortraitPanelSize)
										.HeightOverride(TopStripPanelHeight)
										[
											MakeBottomLeftSectionPanel(
												SNew(SOverlay)
												+ SOverlay::Slot()
												[
													HeartsRowRef
												]
												+ SOverlay::Slot()
												.HAlign(HAlign_Center)
												.VAlign(VAlign_Top)
												.Padding(0.f, 2.f, 0.f, 0.f)
												[
													StatusDotsRowRef
												],
												FMargin(0.f),
												HeartsSectionBorderColor)
										]
									]
									+ SVerticalBox::Slot().AutoHeight()
									[
										SNew(SBox)
										.WidthOverride(PortraitPanelSize)
										.HeightOverride(PortraitPanelSize)
										[
											MakeBottomLeftSectionPanel(
												bDotaTheme
													? StaticCastSharedRef<SWidget>(
														SNew(SOverlay)
														+ SOverlay::Slot()
														[
															SNew(SBorder)
															.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
															.BorderBackgroundColor(FT66Style::PanelOuter())
															.Padding(1.f)
															[
																SNew(SBorder)
																.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
																.BorderBackgroundColor(FT66Style::Border())
																.Padding(1.f)
																[
																	SAssignNew(PortraitBorder, SBorder)
																	.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
																	.BorderBackgroundColor(FT66Style::PanelInner())
																]
															]
														]
														+ SOverlay::Slot()
														[
															SAssignNew(PortraitImage, SImage)
															.Image(PortraitBrush.Get())
															.ColorAndOpacity(FLinearColor::White)
															.Visibility(EVisibility::Collapsed)
														]
														+ SOverlay::Slot()
														.HAlign(HAlign_Right)
														.VAlign(VAlign_Top)
														.Padding(4.f)
														[
															LevelBadgeRef
														]
														+ SOverlay::Slot()
														.HAlign(HAlign_Center)
														.VAlign(VAlign_Center)
														[
															SAssignNew(PortraitPlaceholderText, STextBlock)
															.Text(PortraitLabel)
															.Font(FT66Style::Tokens::FontBold(11))
															.ColorAndOpacity(FT66Style::TextMuted())
															.Justification(ETextJustify::Center)
															.Visibility(EVisibility::Visible)
														]
														)
													: StaticCastSharedRef<SWidget>(
														SNew(SOverlay)
														+ SOverlay::Slot()
														[
															SAssignNew(PortraitBorder, SBorder)
															.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
															.BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.14f, 1.f))
														]
														+ SOverlay::Slot()
														[
															SAssignNew(PortraitImage, SImage)
															.Image(PortraitBrush.Get())
															.ColorAndOpacity(FLinearColor::White)
															.Visibility(EVisibility::Collapsed)
														]
														+ SOverlay::Slot()
														.HAlign(HAlign_Right)
														.VAlign(VAlign_Top)
														.Padding(4.f)
														[
															LevelBadgeRef
														]
														+ SOverlay::Slot()
														.HAlign(HAlign_Center)
														.VAlign(VAlign_Center)
														[
															SAssignNew(PortraitPlaceholderText, STextBlock)
															.Text(PortraitLabel)
															.Font(FT66Style::Tokens::FontBold(11))
															.ColorAndOpacity(FT66Style::Tokens::TextMuted)
															.Justification(ETextJustify::Center)
															.Visibility(EVisibility::Visible)
														]
														),
												FMargin(2.f),
												PortraitSectionBorderColor)
										]
									]
								]
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(BottomLeftColumnGap, 0.f, 0.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(AbilityColumnWidth)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot().AutoHeight()
									[
										SNew(SBox)
										.WidthOverride(AbilityColumnWidth)
										.HeightOverride(TopStripPanelHeight)
										[
											MakeBottomLeftSectionPanel(
												SNew(SBox)
												.HAlign(HAlign_Center)
												.VAlign(VAlign_Center)
												[
													QuickReviveIconRowRef
												],
												FMargin(3.f),
												AbilitySectionBorderColor)
										]
									]
									+ SVerticalBox::Slot().AutoHeight()
									[
										SNew(SBox)
										.WidthOverride(AbilityColumnWidth)
										.HeightOverride(PortraitPanelSize)
										[
											MakeBottomLeftSectionPanel(
												SNew(SVerticalBox)
												+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, GT66BottomLeftAbilityGap)
												[
													SNew(SBox)
													.WidthOverride(AbilityIconSize)
													.HeightOverride(AbilityIconSize)
													[
														SAssignNew(UltimateBorder, SBorder)
														.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
														.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.05f, 1.f))
														.Padding(0.f)
														[
															SNew(SOverlay)
															+ SOverlay::Slot()
															.Padding(AbilityIconInset, AbilityIconInset, AbilityIconInset, AbilityLabelBarHeight + 2.f)
															[
																SNew(SScaleBox)
																.Stretch(EStretch::ScaleToFit)
																[
																	SAssignNew(UltimateImage, SImage)
																	.Image(UltimateBrush.Get())
																	.ColorAndOpacity(FLinearColor::White)
																]
															]
															+ SOverlay::Slot()
															[
																SAssignNew(UltimateCooldownOverlay, SBorder)
																.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
																.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.65f))
																.HAlign(HAlign_Center)
																.VAlign(VAlign_Center)
																.Visibility(EVisibility::Collapsed)
																[
																	SAssignNew(UltimateText, STextBlock)
																	.Text(FText::GetEmpty())
																	.Font(FT66Style::Tokens::FontBold(16))
																	.ColorAndOpacity(FLinearColor::White)
																	.Justification(ETextJustify::Center)
																]
															]
															+ SOverlay::Slot()
															.HAlign(HAlign_Right)
															.VAlign(VAlign_Top)
															[
																SNew(SBox)
																.WidthOverride(AbilityInputBadgeWidth)
																.HeightOverride(AbilityInputBadgeHeight)
																[
																	SNew(SBorder)
																	.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
																	.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.75f))
																	.HAlign(HAlign_Center)
																	.VAlign(VAlign_Center)
																	[
																		SAssignNew(UltimateInputHintText, STextBlock)
																		.Text(NSLOCTEXT("T66.GameplayHUD", "UltKeybindDefault", "R"))
																		.Font(FT66Style::Tokens::FontBold(8))
																		.ColorAndOpacity(FLinearColor::White)
																		.Justification(ETextJustify::Center)
																	]
																]
															]
															+ SOverlay::Slot()
															.VAlign(VAlign_Bottom)
															[
																SNew(SBorder)
																.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
																.BorderBackgroundColor(FLinearColor::Black)
																.Padding(FMargin(0.f, 1.f))
																[
																	SNew(STextBlock)
																	.Text(NSLOCTEXT("T66.GameplayHUD", "UltLabelBar", "ULT"))
																	.Font(FT66Style::Tokens::FontBold(8))
																	.ColorAndOpacity(FLinearColor::White)
																	.Justification(ETextJustify::Center)
																]
															]
														]
													]
												]
												+ SVerticalBox::Slot().AutoHeight()
												[
													SNew(SBox)
													.WidthOverride(AbilityIconSize)
													.HeightOverride(AbilityIconSize)
													[
														SNew(SOverlay)
														+ SOverlay::Slot()
														[
															SAssignNew(PassiveBorder, SBorder)
															.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
															.BorderBackgroundColor(FLinearColor(0.03f, 0.03f, 0.05f, 1.f))
															.Padding(0.f)
															[
																SNew(SOverlay)
																+ SOverlay::Slot()
																.Padding(AbilityIconInset, AbilityIconInset, AbilityIconInset, AbilityLabelBarHeight + 2.f)
																[
																	SNew(SScaleBox)
																	.Stretch(EStretch::ScaleToFit)
																	[
																		SAssignNew(PassiveImage, SImage)
																		.Image(PassiveBrush.Get())
																		.ColorAndOpacity(FLinearColor::White)
																	]
																]
																+ SOverlay::Slot()
																.VAlign(VAlign_Bottom)
																[
																	SNew(SBorder)
																	.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
																	.BorderBackgroundColor(FLinearColor::Black)
																	.Padding(FMargin(0.f, 1.f))
																	[
																		SNew(STextBlock)
																		.Text(NSLOCTEXT("T66.GameplayHUD", "PassiveLabelBar", "PASSIVE"))
																		.Font(FT66Style::Tokens::FontBold(7))
																		.ColorAndOpacity(FLinearColor::White)
																		.Justification(ETextJustify::Center)
																	]
																]
															]
														]
														+ SOverlay::Slot()
														.HAlign(HAlign_Right)
														.VAlign(VAlign_Bottom)
														.Padding(0.f, 0.f, 4.f, AbilityLabelBarHeight + 4.f)
														[
															SAssignNew(PassiveStackBadgeBox, SBox)
															.WidthOverride(20.f)
															.HeightOverride(20.f)
															[
																SNew(SOverlay)
																+ SOverlay::Slot()
																[
																	SNew(SBorder)
																	.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Circle"))
																	.BorderBackgroundColor(FLinearColor(0.12f, 0.10f, 0.08f, 0.95f))
																	.Padding(0.f)
																	.HAlign(HAlign_Center)
																	.VAlign(VAlign_Center)
																	[
																		SAssignNew(PassiveStackText, STextBlock)
																		.Text(FText::AsNumber(0))
																		.Font(FT66Style::Tokens::FontBold(9))
																		.ColorAndOpacity(FLinearColor(0.95f, 0.75f, 0.25f, 1.f))
																		.Justification(ETextJustify::Center)
																	]
																]
															]
														]
													]
												],
												FMargin(2.f),
												AbilitySectionBorderColor)
										]
									]
								]
							],
							FMargin(2.f, 2.f))
					]
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(TopRightHudPadding)
		[
			SAssignNew(MinimapPanelBox, SBox)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBox)
					.WidthOverride(MinimapWidth)
					.HeightOverride(MinimapWidth)
					[
						bDotaTheme
							? FT66Style::MakeMinimapFrame(
								SAssignNew(MinimapWidget, ST66WorldMapWidget)
								.bMinimap(true)
								.bShowLabels(false),
								FMargin(8.f))
							: StaticCastSharedRef<SWidget>(
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									FT66Style::MakePanel(
										SAssignNew(MinimapWidget, ST66WorldMapWidget)
										.bMinimap(true)
										.bShowLabels(false),
										FT66PanelParams(ET66PanelType::Panel2).SetPadding(8.f)
									)
								])
					]
				]
				// Stage number + skulls beneath minimap, grouped in one compact black panel.
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)
				[
					SNew(SBox)
					.WidthOverride(MinimapWidth)
					[
						bDotaTheme
							? FT66Style::MakeHudPanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SAssignNew(StageText, STextBlock)
								.Text(StageInit)
								.Font(FT66Style::Tokens::FontBold(11))
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.Justification(ETextJustify::Center)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(DifficultyRowBox, SBox)
								.Visibility(EVisibility::Collapsed)
								[
									DifficultyRowRef
								]
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)
							[
								SAssignNew(CowardiceRowBox, SBox)
								.Visibility(EVisibility::Collapsed)
								[
									CowardiceRowRef
								]
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(ImmortalityButton, SButton)
								.Visibility(EVisibility::Collapsed)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnToggleImmortality))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								.ContentPadding(FMargin(5.f, 2.f))
								[
									SAssignNew(ImmortalityButtonText, STextBlock)
									.Text(NSLOCTEXT("T66.Dev", "ImmortalityOff", "IMMORTALITY: OFF"))
									.Font(FT66Style::Tokens::FontBold(7))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(PowerButton, SButton)
								.Visibility(EVisibility::Collapsed)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnTogglePower))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								.ContentPadding(FMargin(5.f, 2.f))
								[
									SAssignNew(PowerButtonText, STextBlock)
									.Text(NSLOCTEXT("T66.Dev", "PowerOff", "POWER: OFF"))
									.Font(FT66Style::Tokens::FontBold(7))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							],
							FMargin(6.f, 4.f))
							: FT66Style::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SAssignNew(StageText, STextBlock)
								.Text(StageInit)
								.Font(FT66Style::Tokens::FontBold(11))
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.Justification(ETextJustify::Center)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(DifficultyRowBox, SBox)
								.Visibility(EVisibility::Collapsed)
								[
									DifficultyRowRef
								]
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)
							[
								SAssignNew(CowardiceRowBox, SBox)
								.Visibility(EVisibility::Collapsed)
								[
									CowardiceRowRef
								]
							]
							// Dev toggles are hidden in the current HUD pass.
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(ImmortalityButton, SButton)
								.Visibility(EVisibility::Collapsed)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnToggleImmortality))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								.ContentPadding(FMargin(5.f, 2.f))
								[
									SAssignNew(ImmortalityButtonText, STextBlock)
									.Text(NSLOCTEXT("T66.Dev", "ImmortalityOff", "IMMORTALITY: OFF"))
									.Font(FT66Style::Tokens::FontBold(7))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
							// Dev toggles are hidden in the current HUD pass.
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								SAssignNew(PowerButton, SButton)
								.Visibility(EVisibility::Collapsed)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnTogglePower))
								.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								.ContentPadding(FMargin(5.f, 2.f))
								[
									SAssignNew(PowerButtonText, STextBlock)
									.Text(NSLOCTEXT("T66.Dev", "PowerOff", "POWER: OFF"))
									.Font(FT66Style::Tokens::FontBold(7))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							],
							FT66PanelParams(ET66PanelType::Panel).SetPadding(6.f))
					]
				]
			]
		]
		// Inventory panel bottom-right (Gold/Owe and grid); FPS is above idol panel on the left
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(BottomRightInventoryPadding)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(InventoryPanelBox, SBox)
				.WidthOverride(InventoryPanelVisibleWidth)
				.HeightOverride(InventoryPanelVisibleHeight)
				[
					bDotaTheme
						? FT66Style::MakeHudPanel(
							MakeInventoryEconomySection(
								FT66Style::Accent2(),
								FT66Style::Danger(),
								FT66Style::Tokens::Success,
								WithAlpha(FT66Style::Border(), 0.65f)),
							FMargin(8.f, 6.f))
						: StaticCastSharedRef<SWidget>(
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.30f, 0.38f, 0.35f, 0.85f))
								.Padding(3.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.08f, 0.14f, 0.12f, 0.92f))
									.Padding(FMargin(10.f, 8.f))
									[
										MakeInventoryEconomySection(
											FLinearColor(0.85f, 0.75f, 0.20f, 1.f),
											FT66Style::Tokens::Danger,
											FT66Style::Tokens::Success,
											FLinearColor(0.55f, 0.65f, 0.58f, 0.55f))
									]
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
							[
								SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top)
							[
								SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom)
							[
								SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
								]
							]
							+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom)
							[
								SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.55f, 0.65f, 0.58f, 0.9f))
								]
							])
				]
			]
		]
		// Achievement unlock notification (lower-center lane, clear of inventory and idol panels)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(BottomCenterAchievementPadding)
		[
			SAssignNew(AchievementNotificationBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.WidthOverride(280.f)
			[
				SAssignNew(AchievementNotificationBorder, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
				.Padding(3.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FT66Style::Tokens::Panel)
					.Padding(FMargin(10.f, 8.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SAssignNew(AchievementNotificationTitleText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.AutoWrapText(true)
							.WrapTextAt(256.f)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.GameplayHUD", "AchievementUnlocked", "Unlocked!"))
							.Font(FT66Style::Tokens::FontRegular(14))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
				]
			]
		]
		// Chest reward presentation (same lane as pickup card, non-pausing)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(BottomRightPickupPadding)
		[
			SAssignNew(ChestRewardBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.WidthOverride(PickupCardWidth)
			.HeightOverride(PickupCardHeight)
			[
				SAssignNew(ChestRewardTileBorder, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Tokens::Panel)
				.Padding(2.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.f))
					.Padding(0.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor::Black)
							.Padding(FMargin(8.f, 6.f))
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(SBox)
									.WidthOverride(18.f)
									.HeightOverride(18.f)
									[
										SNew(SImage)
										.Image(GoldCurrencyBrush.Get())
										.ColorAndOpacity(FLinearColor::White)
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
								[
									SAssignNew(ChestRewardCounterText, STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66Style::Tokens::FontBold(18))
									.ColorAndOpacity(FLinearColor(0.98f, 0.83f, 0.24f, 1.f))
								]
							]
						]
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.05f, 0.04f, 0.02f, 1.f))
							.Padding(FMargin(4.f, 6.f))
							[
								ChestRewardArtOverlay
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 1.f))
							.Padding(FMargin(8.f, 4.f))
							[
								SAssignNew(ChestRewardSkipText, STextBlock)
								.Text(FText::GetEmpty())
								.Font(FT66Style::Tokens::FontBold(11))
								.ColorAndOpacity(FLinearColor::White)
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
			]
		]
		// Pickup item card (right side, bottom of card just above inventory)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(BottomRightPickupPadding)
		[
			SAssignNew(PickupCardBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.WidthOverride(PickupCardWidth)
			.HeightOverride(PickupCardHeight)
			[
				SAssignNew(PickupCardTileBorder, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Tokens::Panel)
				.Padding(2.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.f))
					.Padding(0.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SAssignNew(PickupCardIconBorder, SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.04f, 0.04f, 0.05f, 1.f))
							.Padding(FMargin(6.f))
							[
								SNew(SBox)
								.HeightOverride(104.f)
								[
									SNew(SScaleBox)
									.Stretch(EStretch::ScaleToFit)
									.StretchDirection(EStretchDirection::Both)
									[
										SAssignNew(PickupCardIconImage, SImage)
										.Image(PickupCardIconBrush.Get())
										.ColorAndOpacity(FLinearColor::White)
										.Visibility(EVisibility::Collapsed)
									]
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor::Black)
							.Padding(FMargin(10.f, 8.f))
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SAssignNew(PickupCardNameText, STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66Style::Tokens::FontBold(12))
									.ColorAndOpacity(FLinearColor::White)
									.AutoWrapText(true)
									.WrapTextAt(PickupCardWidth - 20.f)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
								[
									SAssignNew(PickupCardDescText, STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66Style::Tokens::FontRegular(10))
									.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.92f))
									.AutoWrapText(true)
									.WrapTextAt(PickupCardWidth - 20.f)
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 1.f))
							.Padding(FMargin(8.f, 4.f))
							[
								SAssignNew(PickupCardSkipText, STextBlock)
								.Text(FText::GetEmpty())
								.Font(FT66Style::Tokens::FontBold(11))
								.ColorAndOpacity(FLinearColor::White)
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
			]
		]
		// Tutorial subtitle (guide dialogue)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 0.f, 92.f)
		[
			SNew(SBox)
			.MinDesiredWidth(860.f)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(TutorialSubtitleSpeakerText, STextBlock)
						.Visibility(EVisibility::Collapsed)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FLinearColor(0.95f, 0.72f, 0.38f, 1.f))
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(TutorialSubtitleBodyText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontRegular(18))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
						.WrapTextAt(820.f)
					]
				,
				FT66PanelParams(ET66PanelType::Panel)
					.SetPadding(FMargin(18.f, 12.f))
					.SetVisibility(EVisibility::Collapsed),
				&TutorialSubtitleBorder
			)
			]
		]
		// Tutorial hint (above crosshair)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0.f, -220.f, 0.f, 0.f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SAssignNew(TutorialHintLine1Text, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)
				[
					SAssignNew(TutorialHintLine2Text, STextBlock)
					.Visibility(EVisibility::Collapsed)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontRegular(14))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.Justification(ETextJustify::Center)
				]
			,
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 8.f)).SetVisibility(EVisibility::Collapsed),
			&TutorialHintBorder
		)
		]
		// Center crosshair (screen center; camera unchanged)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0.f)
		[
			SAssignNew(CenterCrosshairBox, SBox)
			.WidthOverride(28.f)
			.HeightOverride(28.f)
			[
				SAssignNew(CenterCrosshairWidget, ST66CrosshairWidget)
				.Locked(false)
			]
		]
		// Hero 1 scoped sniper overlay (first-person aim view + ult timers)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(ScopedSniperOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
			.Padding(0.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(ST66ScopedSniperWidget)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(0.f, 24.f, 0.f, 0.f)
				[
					FT66Style::MakePanel(
						SAssignNew(ScopedUltTimerText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(20))
						.ColorAndOpacity(FT66Style::Tokens::Text),
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(12.f, 8.f)))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Bottom)
				.Padding(0.f, 0.f, 0.f, 42.f)
				[
					FT66Style::MakePanel(
						SAssignNew(ScopedShotCooldownText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FT66Style::Tokens::Text),
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f, 8.f)))
				]
			]
		]
		// Quick Revive downed overlay
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(QuickReviveDownedOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.42f, 0.42f, 0.42f, 0.58f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(QuickReviveDownedText, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(28))
				.ColorAndOpacity(FLinearColor::White)
				.Justification(ETextJustify::Center)
			]
		]
		// Curse (visibility) overlay (always on top)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(CurseOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.05f, 0.0f, 0.08f, 0.40f))
		]
		// Full-screen map overlay (OpenFullMap / M)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(FullMapOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.78f))
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					FT66Style::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.Map", "Title", "MAP"))
								.Font(FT66Style::Tokens::FontBold(18))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.Map", "CloseHint", "[M] Close"))
								.Font(FT66Style::Tokens::FontBold(12))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBox)
							.WidthOverride(FullMapWidthAttr)
							.HeightOverride(FullMapHeightAttr)
							[
								FT66Style::MakePanel(
									SAssignNew(FullMapWidget, ST66WorldMapWidget)
									.bMinimap(false)
									.bShowLabels(true),
									FT66PanelParams(ET66PanelType::Panel2).SetPadding(10.f)
								)
							]
						]
					,
					FT66PanelParams(ET66PanelType::Panel)
				)
				]
			]
		];

	ApplyInventoryInspectMode();
	return Root;
}

static void T66_ApplyWorldDialogueSelection(
	const TArray<TSharedPtr<SBorder>>& OptionBorders,
	const TArray<TSharedPtr<STextBlock>>& OptionTexts,
	int32 SelectedIndex)
{
	for (int32 i = 0; i < OptionBorders.Num(); ++i)
	{
		const bool bSelected = (i == SelectedIndex);
		if (OptionBorders[i].IsValid())
		{
			OptionBorders[i]->SetBorderBackgroundColor(bSelected
				? (FT66Style::IsDotaTheme() ? FT66Style::SelectionFill() : FLinearColor(0.18f, 0.18f, 0.26f, 0.95f))
				: (FT66Style::IsDotaTheme() ? FT66Style::PromptBackground() : FLinearColor(0.06f, 0.06f, 0.08f, 0.85f)));
		}
		if (OptionTexts.IsValidIndex(i) && OptionTexts[i].IsValid())
		{
			OptionTexts[i]->SetColorAndOpacity(bSelected ? FT66Style::Tokens::Text : FT66Style::Tokens::TextMuted);
		}
	}
}


FReply UT66GameplayHUDWidget::OnToggleImmortality()
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->ToggleDevImmortality();
	}
	return FReply::Handled();
}


FReply UT66GameplayHUDWidget::OnTogglePower()
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->ToggleDevPower();
	}
	return FReply::Handled();
}


void UT66GameplayHUDWidget::SetInventoryInspectMode(bool bEnabled)
{
	if (bInventoryInspectMode == bEnabled)
	{
		return;
	}

	bInventoryInspectMode = bEnabled;
	ApplyInventoryInspectMode();
}


void UT66GameplayHUDWidget::ApplyInventoryInspectMode()
{
	const float Scale = bInventoryInspectMode ? BottomRightInventoryInspectScale : 1.f;

	if (InventoryPanelBox.IsValid())
	{
		InventoryPanelBox->SetWidthOverride(BottomRightInventoryPanelWidth * Scale);
		InventoryPanelBox->SetHeightOverride(BottomRightInventoryPanelHeight * Scale);
	}

	const float SlotSize = BottomRightInventorySlotSize * Scale;
	const float IconSize = FMath::Max(8.f, SlotSize - 4.f);
	for (const TSharedPtr<SBox>& SlotContainer : InventorySlotContainers)
	{
		if (SlotContainer.IsValid())
		{
			SlotContainer->SetHeightOverride(SlotSize);
		}
	}

	for (const TSharedPtr<FSlateBrush>& SlotBrush : InventorySlotBrushes)
	{
		if (SlotBrush.IsValid())
		{
			SlotBrush->ImageSize = FVector2D(IconSize, IconSize);
		}
	}
}
