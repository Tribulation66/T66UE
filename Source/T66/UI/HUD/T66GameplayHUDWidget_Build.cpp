// Copyright Tribulation 66. All Rights Reserved.

#include "UI/HUD/T66GameplayHUDWidget_Private.h"

namespace
{
	static const FLinearColor T66HudBorderRed = FT66FlatStyle::DefaultBorder();
	static const FLinearColor T66HudDeepRed = FT66FlatStyle::DefaultFill();
	static const FLinearColor T66HudPanelRed = FT66FlatStyle::DefaultFill();
	static const FLinearColor T66HudDividerRed = FT66FlatStyle::DisabledBorder();
	static const FLinearColor T66HudTextRed = FT66FlatStyle::PrimaryText();

	static const FSlateBrush* GetGameplayHudSlotBrush(const bool bRed)
	{
		return FCoreStyle::Get().GetBrush("WhiteBrush");
	}

	static TSharedRef<SWidget> MakeGameplayHudSquarePanel(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, Padding, Content);
	}

	static TSharedRef<SWidget> MakeGameplayHudSquareSlot(const TSharedRef<SWidget>& Content, const FMargin& Padding, const bool bRed = false)
	{
		return FT66FlatStyle::MakeFlatPanel(bRed ? ET66FlatState::Selected : ET66FlatState::Default, Padding, Content);
	}
}

TSharedRef<SWidget> UT66GameplayHUDWidget::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	const UT66DifficultyTuningSubsystem* DifficultyTuning = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66DifficultyTuningSubsystem>() : nullptr;
	const ET66Difficulty SelectedDifficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const int32 InitialStage = GetRunState() ? GetRunState()->GetCurrentStage() : 1;
	const FText DifficultyAreaNameInit = BuildDifficultyAreaNameText(SelectedDifficulty);
	const FText StageInit = BuildDisplayedStageText(
		Loc,
		DifficultyTuning,
		SelectedDifficulty,
		InitialStage);
	const FText GoldInit = FText::AsNumber(0);
	const FText OweInit = FText::AsNumber(0);
	const FText NetWorthInit = FText::AsNumber(0);
	const FText ScoreLabelText = Loc ? Loc->GetText_ScoreLabel() : NSLOCTEXT("T66.GameplayHUD", "ScoreLabel", "Score:");
	const FText PortraitLabel = Loc ? Loc->GetText_PortraitPlaceholder() : NSLOCTEXT("T66.GameplayHUD", "PortraitLabel", "PORTRAIT");
	NetWorthText.Reset();
	StatDamageText.Reset();
	StatAttackSpeedText.Reset();
	StatAttackScaleText.Reset();
	StatArmorText.Reset();
	StatEvasionText.Reset();
	StatLuckText.Reset();
	PortraitStatPanelBox.Reset();
	PortraitPlaceholderText.Reset();
	DifficultyRowBox.Reset();
	CowardiceRowBox.Reset();
	BossPartBarRows.Reset();
	constexpr bool bUseAlternateHudChrome = false;
	const FLinearColor SlotOuterColor = bUseAlternateHudChrome ? FLinearColor(0.018f, 0.014f, 0.012f, 0.98f) : T66HudBorderRed;
	const FLinearColor SlotFrameColor = bUseAlternateHudChrome ? FLinearColor(0.56f, 0.42f, 0.23f, 0.88f) : T66HudBorderRed;
	const FLinearColor SlotFillColor = bUseAlternateHudChrome ? FLinearColor(0.025f, 0.026f, 0.032f, 0.96f) : T66HudDeepRed;
	const FLinearColor BossBarBackgroundColor = bUseAlternateHudChrome ? FT66FlatStyle::BossBarBackground() : FLinearColor(0.08f, 0.08f, 0.08f, 0.9f);
	const FLinearColor BossBarFillColor = bUseAlternateHudChrome ? FT66FlatStyle::BossBarFill() : FLinearColor(0.9f, 0.1f, 0.1f, 0.95f);
	const FLinearColor PromptBackgroundColor = bUseAlternateHudChrome ? FLinearColor(0.026f, 0.022f, 0.020f, 0.90f) : T66HudDeepRed;
	const FLinearColor DialogueBackgroundColor = bUseAlternateHudChrome ? FLinearColor(0.030f, 0.026f, 0.024f, 0.94f) : T66HudPanelRed;
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
	InventorySlotCountTexts.SetNum(InventorySlotWidgetCount);
	InventorySlotBrushes.SetNum(InventorySlotWidgetCount);
	CachedInventorySlotIDs.SetNum(InventorySlotWidgetCount);
	CachedInventorySlotCounts.Init(0, InventorySlotWidgetCount);
	ChestRewardCoinBoxes.SetNum(ChestRewardCoinCount);
	ChestRewardCoinImages.SetNum(ChestRewardCoinCount);
	ChestRewardBeamBoxes.SetNum(ChestRewardBeamCount);
	ChestRewardBeamBorders.SetNum(ChestRewardBeamCount);
	ChestRewardSparkleBoxes.SetNum(ChestRewardSparkleCount);
	ChestRewardSparkleBorders.SetNum(ChestRewardSparkleCount);
	LootBagRevealSparkleBoxes.SetNum(LootBagRevealSparkleCount);
	LootBagRevealSparkleBorders.SetNum(LootBagRevealSparkleCount);
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
	if (!ChestRewardCoinBrush.IsValid())
	{
		ChestRewardCoinBrush = MakeShared<FSlateBrush>();
		ChestRewardCoinBrush->DrawAs = ESlateBrushDrawType::Image;
		ChestRewardCoinBrush->ImageSize = FVector2D(36.f, 36.f);
		ChestRewardCoinBrush->Tiling = ESlateBrushTileType::NoTile;
		ChestRewardCoinBrush->SetResourceObject(nullptr);
	}
	BindRuntimeHudBrush(ChestRewardCoinBrush, GetChestRewardCoinRelativePath(), FVector2D(36.f, 36.f));
	if (!LootBagRevealClosedBrush.IsValid())
	{
		LootBagRevealClosedBrush = MakeShared<FSlateBrush>();
		LootBagRevealClosedBrush->DrawAs = ESlateBrushDrawType::Image;
		LootBagRevealClosedBrush->ImageSize = FVector2D(260.f, 246.f);
		LootBagRevealClosedBrush->Tiling = ESlateBrushTileType::NoTile;
		LootBagRevealClosedBrush->SetResourceObject(nullptr);
	}
	if (!LootBagRevealOpenBrush.IsValid())
	{
		LootBagRevealOpenBrush = MakeShared<FSlateBrush>();
		LootBagRevealOpenBrush->DrawAs = ESlateBrushDrawType::Image;
		LootBagRevealOpenBrush->ImageSize = FVector2D(286.f, 270.f);
		LootBagRevealOpenBrush->Tiling = ESlateBrushTileType::NoTile;
		LootBagRevealOpenBrush->SetResourceObject(nullptr);
	}
	if (!LootBagRevealCardIconBrush.IsValid())
	{
		LootBagRevealCardIconBrush = MakeShared<FSlateBrush>();
		LootBagRevealCardIconBrush->DrawAs = ESlateBrushDrawType::Image;
		LootBagRevealCardIconBrush->ImageSize = FVector2D(84.f, 84.f);
		LootBagRevealCardIconBrush->Tiling = ESlateBrushTileType::NoTile;
		LootBagRevealCardIconBrush->SetResourceObject(nullptr);
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
	// Load the fallback ultimate texture via the texture pool. The lower slot is
	// the equipped weapon icon and is populated from weapon data during refresh.
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> UltSoft = ResolveGameplayUltimateIcon(NAME_None, ET66UltimateType::None);
			T66SlateTexture::BindSharedBrushAsync(TexPool, UltSoft, this, UltimateBrush, FName(TEXT("HUDUltimate")), false);
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
	if (!BackroomsReviveBrush.IsValid())
	{
		BackroomsReviveBrush = MakeShared<FSlateBrush>();
		BackroomsReviveBrush->DrawAs = ESlateBrushDrawType::Image;
		BackroomsReviveBrush->ImageSize = FVector2D(26.f, 26.f);
		BackroomsReviveBrush->Tiling = ESlateBrushTileType::NoTile;
		BackroomsReviveBrush->SetResourceObject(nullptr);
	}
	{
		UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			const TSoftObjectPtr<UTexture2D> BackroomsQuickReviveSoft(FSoftObjectPath(TEXT("/Game/Items/Sprites/Item_BackroomsQuickRevive.Item_BackroomsQuickRevive")));
			T66SlateTexture::BindSharedBrushAsync(TexPool, BackroomsQuickReviveSoft, this, BackroomsReviveBrush, FName(TEXT("HUDBackroomsRevive")), false);
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
	static constexpr float MinimapWidth = 200.f;
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

	// Build legacy heart widgets off-screen for compatibility with refresh code; the visible
	// health surface is the percent readout below.
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
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.055f, 0.92f))
				.Padding(2.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					[
						SAssignNew(HeroDamagePercentFillBox, SBox)
						.WidthOverride(0.f)
						.HeightOverride(GT66DisplayedHeartAreaHeight - 4.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.92f, 0.12f, 0.08f, 0.82f))
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(HeroDamagePercentText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "HeroDamagePercentInitial", "0%"))
						.Font(FT66FlatStyle::Tokens::FontBold(22))
						.ColorAndOpacity(FLinearColor::White)
						.Justification(ETextJustify::Center)
					]
				]
			]
		];

	TSharedRef<SWidget> BackroomsReviveIconRowRef =
		SAssignNew(BackroomsReviveIconRowBox, SBox)
		.Visibility(EVisibility::Collapsed)
		.WidthOverride(GT66BottomLeftAbilityBoxSize)
		.HeightOverride(GT66BottomLeftAbilityBoxSize)
		[
			FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
				SAssignNew(BackroomsReviveIconImage, SImage)
				.Image(BackroomsReviveBrush.Get())
				.ColorAndOpacity(FLinearColor::White)),
				TEXT("GameplayHUD.BackroomsQuickRevive.Icon"),
				TEXT("Icon"))
		];

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
	const float BaseStatsPanelWidth = GT66BottomLeftBaseStatsWidth;
	const float AbilityInputBadgeWidth = 28.f;
	const float AbilityInputBadgeHeight = 18.f;
	const float AbilityIconInset = 6.f;
	const float BottomLeftColumnGap = 0.f;
	const FLinearColor BottomLeftPanelOuterColor = bUseAlternateHudChrome ? WithAlpha(FT66FlatStyle::DefaultBorder(), 0.96f) : T66HudBorderRed;
	const FLinearColor BottomLeftPanelInnerColor = bUseAlternateHudChrome ? WithAlpha(FT66FlatStyle::DefaultFill(), 0.98f) : T66HudPanelRed;
	const FLinearColor BottomLeftPanelTitleColor = bUseAlternateHudChrome ? FT66FlatStyle::PrimaryText() : T66HudTextRed;
	const FLinearColor BottomLeftPanelDividerColor = bUseAlternateHudChrome ? WithAlpha(FT66FlatStyle::DisabledBorder(), 0.66f) : T66HudDividerRed;
	const FLinearColor IdolSectionBorderColor = bUseAlternateHudChrome ? WithAlpha(FT66FlatStyle::DefaultBorder(), 0.95f) : T66HudBorderRed;
	const FLinearColor PortraitSectionBorderColor = bUseAlternateHudChrome ? WithAlpha(FT66FlatStyle::DefaultBorder(), 0.98f) : T66HudBorderRed;
	const FLinearColor AbilitySectionBorderColor = bUseAlternateHudChrome ? WithAlpha(FT66FlatStyle::DefaultBorder(), 0.96f) : T66HudBorderRed;
	const FLinearColor BaseStatsSectionBorderColor = bUseAlternateHudChrome ? WithAlpha(FT66FlatStyle::DefaultBorder(), 0.96f) : T66HudBorderRed;
	const FLinearColor SharedSectionFillColor = bUseAlternateHudChrome ? WithAlpha(FT66FlatStyle::DefaultFill(), 0.98f) : T66HudDeepRed;
	const FLinearColor LevelTextColor = bUseAlternateHudChrome ? FT66FlatStyle::PrimaryText() : T66HudTextRed;
	TSharedRef<SWidget> LevelBadgeRef =
		SNew(SBox)
		.WidthOverride(GT66BottomLeftLevelBadgeSize)
		.HeightOverride(GT66BottomLeftLevelBadgeSize)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				FT66AnimatedStyle::AttachMetadata(
					SAssignNew(LevelRingWidget, ST66RingWidget),
					TEXT("GameplayHUD.LevelRing"),
					TEXT("ProgressRing"))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(LevelText, STextBlock)
				.Text(FText::AsNumber(1))
				.Font(FT66FlatStyle::Tokens::FontBold(11))
				.ColorAndOpacity(LevelTextColor)
				.Justification(ETextJustify::Center)
			]
		];
	auto MakeBottomLeftBlackPanel = [&](const FText& Title, const TSharedRef<SWidget>& Content, const FMargin& InnerPadding) -> TSharedRef<SWidget>
	{
		return bUseAlternateHudChrome
			? StaticCastSharedRef<SWidget>(
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
							.Font(FT66FlatStyle::Tokens::FontBold(13))
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
				])
			: MakeGameplayHudSquarePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 2.f)
				[
					SNew(STextBlock)
					.Text(Title)
					.Font(FT66FlatStyle::Tokens::FontBold(13))
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
				],
				InnerPadding);
	};
	auto MakeBottomLeftBlackPanelNoTitle = [&](const TSharedRef<SWidget>& Content, const FMargin& InnerPadding) -> TSharedRef<SWidget>
	{
		return bUseAlternateHudChrome
			? StaticCastSharedRef<SWidget>(
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
				])
			: MakeGameplayHudSquarePanel(Content, InnerPadding);
	};
	auto MakeBottomLeftSectionPanel = [&](const TSharedRef<SWidget>& Content, const FMargin& InnerPadding, const FLinearColor& BorderColor) -> TSharedRef<SWidget>
	{
		const bool bUseRedSlot = BorderColor.R > BorderColor.B;
		return bUseAlternateHudChrome
			? StaticCastSharedRef<SWidget>(
				SNew(SBorder)
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
				])
			: MakeGameplayHudSquareSlot(Content, InnerPadding, bUseRedSlot);
	};
	auto MakeBaseStatLine = [&](TSharedPtr<STextBlock>& OutText) -> TSharedRef<SWidget>
	{
		return SAssignNew(OutText, STextBlock)
			.Text(NSLOCTEXT("T66.GameplayHUD", "BaseStatPending", "--"))
			.Font(FT66FlatStyle::Tokens::FontBold(13))
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			.Justification(ETextJustify::Left)
			.AutoWrapText(false);
	};
	auto MakeCurrencyReadout = [&](const FText& Label, TSharedPtr<STextBlock>& OutValueText, const FText& InitialValue, const FLinearColor& ValueColor) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FT66FlatStyle::Tokens::FontBold(13))
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SAssignNew(OutValueText, STextBlock)
				.Text(InitialValue)
				.Font(FT66FlatStyle::Tokens::FontBold(13))
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
					FT66FlatStyle::AttachMetadata(
						SAssignNew(IdolBorder, SBorder)
						.BorderImage(bUseAlternateHudChrome ? FCoreStyle::Get().GetBrush("WhiteBrush") : GetGameplayHudSlotBrush(false))
						.BorderBackgroundColor(SlotOuterColor)
						.Padding(1.f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SBorder)
								.BorderImage(bUseAlternateHudChrome ? FCoreStyle::Get().GetBrush("WhiteBrush") : GetGameplayHudSlotBrush(true))
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
									.BorderBackgroundColor(FLinearColor(0.95f, 0.97f, 1.0f, bUseAlternateHudChrome ? 0.12f : 0.08f))
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
						],
						FName(*FString::Printf(TEXT("GameplayHUD.IdolSlot.%02d.Border"), i + 1)),
						TEXT("HUDChromeSlot"),
						ET66FlatState::Default)
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

	// Inventory: two-row grid matching the combined idol + portrait + abilities HUD footprint.
	static constexpr int32 InvCols = 10;
	const int32 InvRows = FMath::Max(1, FMath::DivideAndRoundUp(InventorySlotBorders.Num(), InvCols));
	const float InvSlotSize = BottomRightInventorySlotSize;
	static constexpr float InvSlotPad = 0.f;
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
			TSharedPtr<STextBlock> SlotCountText;
			const int32 ThisSlotIndex = SlotIndex;
			RowBox->AddSlot()
				.AutoWidth()
				.Padding(InvSlotPad, InvSlotPad)
				[
					SAssignNew(InventorySlotContainers[ThisSlotIndex], SBox)
					.WidthOverride(InvSlotSize)
					.HeightOverride(InvSlotSize)
					[
						SNew(SOverlay)
						// Transparent slot bg with thin border outline
						+ SOverlay::Slot()
						[
							FT66FlatStyle::AttachMetadata(
								SAssignNew(SlotBorder, SBorder)
								.BorderImage(bUseAlternateHudChrome ? FCoreStyle::Get().GetBrush("WhiteBrush") : GetGameplayHudSlotBrush(false))
								.BorderBackgroundColor(SlotOuterColor)
								.Padding(1.f)
								[
									SNew(SOverlay)
									+ SOverlay::Slot()
									[
										SNew(SBorder)
										.BorderImage(bUseAlternateHudChrome ? FCoreStyle::Get().GetBrush("WhiteBrush") : GetGameplayHudSlotBrush(true))
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
											.BorderBackgroundColor(FLinearColor(0.95f, 0.97f, 1.0f, bUseAlternateHudChrome ? 0.12f : 0.08f))
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
								],
								FName(*FString::Printf(TEXT("GameplayHUD.InventorySlot.%02d.Border"), ThisSlotIndex + 1)),
								TEXT("HUDChromeSlot"),
								ET66FlatState::Default)
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
						+ SOverlay::Slot()
						.HAlign(HAlign_Right)
						.VAlign(VAlign_Bottom)
						.Padding(0.f, 0.f, 3.f, 2.f)
						[
							SAssignNew(SlotCountText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66FlatStyle::Tokens::FontBold(10))
							.ColorAndOpacity(FLinearColor::White)
							.ShadowOffset(FVector2D(1.f, 1.f))
							.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
							.Visibility(EVisibility::Collapsed)
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
			if (InventorySlotCountTexts.IsValidIndex(ThisSlotIndex))
			{
				InventorySlotCountTexts[ThisSlotIndex] = SlotCountText;
			}
			SlotIndex++;
		}
		InvGridRows->AddSlot().AutoHeight().HAlign(HAlign_Fill)[ RowBox ];
	}
	TSharedRef<SWidget> InvGridRef =
		SNew(SScrollBox)
		.Orientation(Orient_Horizontal)
		.ScrollBarVisibility(EVisibility::Collapsed)
		+ SScrollBox::Slot()
		[
			SNew(SBox)
			.WidthOverride(InvCols * InvSlotSize)
			[
				InvGridRows
			]
		];
	auto MakeInventoryEconomySection = [&](const FLinearColor& GoldValueColor, const FLinearColor& DebtValueColor, const FLinearColor& DividerColor) -> TSharedRef<SWidget>
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
				[
					MakeCurrencyReadout(NSLOCTEXT("T66.GameplayHUD", "GoldLabel", "Gold"), GoldText, GoldInit, GoldValueColor)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
				[
					MakeCurrencyReadout(NSLOCTEXT("T66.GameplayHUD", "DebtLabel", "Debt"), DebtText, OweInit, DebtValueColor)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					MakeCurrencyReadout(NSLOCTEXT("T66.GameplayHUD", "NetWorthLabel", "Net Worth"), NetWorthText, NetWorthInit, FT66FlatStyle::Tokens::Text)
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
			+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Bottom)
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
		return FMargin(0.f, 0.f, UT66GameplayHUDWidget::BottomRightRewardLaneRightPadding, 12.f);
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
			UT66GameplayHUDWidget::BottomRightRewardLaneRightPadding,
			UT66GameplayHUDWidget::BottomRightRewardLaneBottomPadding);
	});

	const TAttribute<FOptionalSize> FullMapWidthAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66FlatStyle::GetSafeFrameSize();
		const float Width = FMath::Clamp(SafeFrame.X - 72.f, 720.f, 1100.f);
		return FOptionalSize(Width);
	});

	const TAttribute<FOptionalSize> FullMapHeightAttr = TAttribute<FOptionalSize>::CreateLambda([]() -> FOptionalSize
	{
		const FVector2D SafeFrame = FT66FlatStyle::GetSafeFrameSize();
		const float Height = FMath::Clamp(SafeFrame.Y - 88.f, 420.f, 680.f);
		return FOptionalSize(Height);
	});

	TSharedRef<SOverlay> ChestRewardArtOverlay = SNew(SOverlay);
	for (int32 BeamIndex = 0; BeamIndex < ChestRewardBeamCount; ++BeamIndex)
	{
		ChestRewardArtOverlay->AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 0.f, 48.f)
		[
			SAssignNew(ChestRewardBeamBoxes[BeamIndex], SBox)
			.WidthOverride(46.f)
			.HeightOverride(360.f)
			.Visibility(EVisibility::Collapsed)
			.RenderTransformPivot(FVector2D(0.5f, 1.f))
			[
				SAssignNew(ChestRewardBeamBorders[BeamIndex], SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(1.f, 0.78f, 0.22f, 0.30f))
			]
		];
	}
	for (int32 SparkleIndex = 0; SparkleIndex < ChestRewardSparkleCount; ++SparkleIndex)
	{
		ChestRewardArtOverlay->AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(ChestRewardSparkleBoxes[SparkleIndex], SBox)
			.WidthOverride(8.f)
			.HeightOverride(8.f)
			.Visibility(EVisibility::Collapsed)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
			[
				SAssignNew(ChestRewardSparkleBorders[SparkleIndex], SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(1.f, 0.92f, 0.52f, 0.78f))
			]
		];
	}
	ChestRewardArtOverlay->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	.Padding(0.f, 0.f, 0.f, 32.f)
	[
		SAssignNew(ChestRewardClosedBox, SBox)
		.WidthOverride(312.f)
		.HeightOverride(240.f)
		.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		[
			SAssignNew(ChestRewardClosedImage, SImage)
			.Image(ChestRewardClosedBrush.Get())
			.ColorAndOpacity(FLinearColor::White)
		]
	];
	ChestRewardArtOverlay->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	.Padding(0.f, 0.f, 0.f, 22.f)
	[
		SAssignNew(ChestRewardOpenBox, SBox)
		.WidthOverride(360.f)
		.HeightOverride(308.f)
		.RenderTransformPivot(FVector2D(0.5f, 0.5f))
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
			.WidthOverride(36.f)
			.HeightOverride(36.f)
			.Visibility(EVisibility::Collapsed)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
			[
				SAssignNew(ChestRewardCoinImages[CoinIndex], SImage)
				.Image(ChestRewardCoinBrush.Get())
				.ColorAndOpacity(FLinearColor::White)
			]
		];
	}

	TSharedRef<SOverlay> LootBagRevealOverlay = SNew(SOverlay);
	for (int32 SparkleIndex = 0; SparkleIndex < LootBagRevealSparkleCount; ++SparkleIndex)
	{
		LootBagRevealOverlay->AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(LootBagRevealSparkleBoxes[SparkleIndex], SBox)
			.WidthOverride(8.f)
			.HeightOverride(8.f)
			.Visibility(EVisibility::Collapsed)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
			[
				SAssignNew(LootBagRevealSparkleBorders[SparkleIndex], SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(1.f, 0.88f, 0.36f, 0.82f))
			]
		];
	}
	LootBagRevealOverlay->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	[
		SAssignNew(LootBagRevealCardBox, SBox)
		.WidthOverride(PickupCardWidth)
		.HeightOverride(PickupCardHeight)
		.Visibility(EVisibility::Collapsed)
		.RenderTransformPivot(FVector2D(0.5f, 1.f))
		[
			SAssignNew(LootBagRevealCardTileBorder, SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.58f, 0.42f, 0.22f, 0.96f))
			.Padding(2.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.030f, 0.025f, 0.021f, 0.98f))
				.Padding(0.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						SAssignNew(LootBagRevealCardIconBorder, SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.028f, 0.026f, 0.032f, 0.98f))
						.Padding(FMargin(8.f))
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFit)
							.StretchDirection(EStretchDirection::Both)
							[
								SAssignNew(LootBagRevealCardIconImage, SImage)
								.Image(LootBagRevealCardIconBrush.Get())
								.ColorAndOpacity(FLinearColor::White)
								.Visibility(EVisibility::Collapsed)
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.055f, 0.043f, 0.032f, 0.98f))
						.Padding(FMargin(10.f, 8.f))
						[
							SAssignNew(LootBagRevealCardNameText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66FlatStyle::Tokens::FontBold(12))
							.ColorAndOpacity(FLinearColor::White)
							.Justification(ETextJustify::Center)
							.AutoWrapText(true)
							.WrapTextAt(PickupCardWidth - 20.f)
						]
					]
				]
			]
		]
	];
	LootBagRevealOverlay->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	.Padding(0.f, 0.f, 0.f, -8.f)
	[
		SAssignNew(LootBagRevealClosedBox, SBox)
		.WidthOverride(260.f)
		.HeightOverride(246.f)
		.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		[
			SAssignNew(LootBagRevealClosedImage, SImage)
			.Image(LootBagRevealClosedBrush.Get())
			.ColorAndOpacity(FLinearColor::White)
		]
	];
	LootBagRevealOverlay->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	.Padding(0.f, 0.f, 0.f, -8.f)
	[
		SAssignNew(LootBagRevealOpenBox, SBox)
		.WidthOverride(286.f)
		.HeightOverride(270.f)
		.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		[
			SAssignNew(LootBagRevealOpenImage, SImage)
			.Image(LootBagRevealOpenBrush.Get())
			.ColorAndOpacity(FLinearColor::White)
		]
	];

	const FText JumpKeycapText = ResolveGameplayJumpKeycapText();

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
						.Font(FT66FlatStyle::Tokens::FontBold(16))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
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
							FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
								SAssignNew(LootPromptIconImage, SImage)
								.Image(LootPromptIconBrush.Get())
								.ColorAndOpacity(FLinearColor::White)
								.Visibility(EVisibility::Collapsed)),
								TEXT("GameplayHUD.LootPrompt.Icon"),
								TEXT("Icon"))
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SAssignNew(LootPromptText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66FlatStyle::Tokens::FontBold(14))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
					]
				]
			]
		]
		// (Central countdown timer removed - stage timer info available in top-left stats)
		// In-world NPC dialogue (positioned via RenderTransform; hidden by default)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		[
			SAssignNew(WorldDialogueBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.RenderTransform(FSlateRenderTransform(FVector2D(0.f, 0.f)))
			[
				MakeGameplayHudSquarePanel(
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
							.Font(FT66FlatStyle::Tokens::FontBold(18))
							.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
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
							.Font(FT66FlatStyle::Tokens::FontBold(18))
							.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
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
							.Font(FT66FlatStyle::Tokens::FontBold(18))
							.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
						]
					]
				,
				FMargin(12.f)
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
			.Visibility(EVisibility::Collapsed)
			.WidthOverride(GT66MediaPanelW)
			.HeightOverride(GT66MediaPanelH)
			[
				MakeGameplayHudSquarePanel(
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
				FMargin(6.f)
			)
			]
		]
		// Top-left stats (score + speedrun time) - themed panel and text
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(TopLeftHudPadding)
		[
			bUseAlternateHudChrome
				? FT66FlatStyle::MakeHudPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(ScoreLabelText)
							.Font(FT66FlatStyle::Tokens::FontBold(10))
							.ColorAndOpacity(FSlateColor(FT66FlatStyle::Text()))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
						[
							SAssignNew(ScoreText, STextBlock)
							.Text(FText::AsNumber(0))
							.Font(FT66FlatStyle::Tokens::FontBold(10))
							.ColorAndOpacity(FSlateColor(FT66FlatStyle::Text()))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(ScorePacingText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScorePacingDefault", "Score Pace --"))
						.Font(FT66FlatStyle::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66FlatStyle::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(ScoreTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScoreTargetDefault", "Score to Beat --"))
						.Font(FT66FlatStyle::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66FlatStyle::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunDefault", "Time 0:00.00"))
						.Font(FT66FlatStyle::Tokens::FontBold(10))
						.ColorAndOpacity(FSlateColor(FT66FlatStyle::Text()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunPacingText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "TimePacingDefault", "Time Pace --:--.--"))
						.Font(FT66FlatStyle::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66FlatStyle::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetDefault", "Time to Beat --:--.--"))
						.Font(FT66FlatStyle::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66FlatStyle::TextMuted()))
						.Visibility(EVisibility::Collapsed)
					],
					FMargin(6.f, 4.f))
				: MakeGameplayHudSquarePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(ScoreLabelText)
							.Font(FT66FlatStyle::Tokens::FontBold(10))
							.ColorAndOpacity(FSlateColor(FT66FlatStyle::Tokens::Text))
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SAssignNew(ScoreText, STextBlock)
							.Text(FText::AsNumber(0))
							.Font(FT66FlatStyle::Tokens::FontBold(10))
							.ColorAndOpacity(FSlateColor(FT66FlatStyle::Tokens::Text))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
					[
						SAssignNew(ScorePacingText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScorePacingDefault", "Score Pace --"))
						.Font(FT66FlatStyle::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66FlatStyle::Tokens::TextMuted))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(ScoreTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "ScoreTargetDefault", "Score to Beat --"))
						.Font(FT66FlatStyle::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66FlatStyle::Tokens::TextMuted))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunDefault", "Time 0:00.00"))
						.Font(FT66FlatStyle::Tokens::FontBold(10))
						.ColorAndOpacity(FSlateColor(FT66FlatStyle::Tokens::Text))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunPacingText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "TimePacingDefault", "Time Pace --:--.--"))
						.Font(FT66FlatStyle::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66FlatStyle::Tokens::TextMuted))
						.Visibility(EVisibility::Collapsed)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SAssignNew(SpeedRunTargetText, STextBlock)
						.Text(NSLOCTEXT("T66.GameplayHUD", "SpeedRunTargetDefault", "Time to Beat --:--.--"))
						.Font(FT66FlatStyle::Tokens::FontBold(9))
						.ColorAndOpacity(FSlateColor(FT66FlatStyle::Tokens::TextMuted))
						.Visibility(EVisibility::Collapsed)
					],
					FMargin(8.f, 6.f)
				)
		]

		// Bottom-left portrait stack with a tighter, uniform panel footprint.
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(BottomLeftHudPadding)
		[
			SAssignNew(BottomLeftHUDBox, SBox)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(0.f, 0.f, 0.f, 2.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(IdolPanelMinWidth)
						.HeightOverride(TopStripPanelHeight)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(PortraitPanelSize)
						.HeightOverride(TopStripPanelHeight)
						[
							HeartsRowRef
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(PortraitStatPanelBox, SBox)
					[
						MakeBottomLeftBlackPanelNoTitle(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SBox)
								.WidthOverride(IdolPanelMinWidth)
								.HeightOverride(PortraitPanelSize)
								[
									SAssignNew(IdolSlotsPanelBox, SBox)
									.WidthOverride(IdolPanelMinWidth)
									.HeightOverride(PortraitPanelSize)
									[
										MakeBottomLeftSectionPanel(
											SNew(SBox)
											.HAlign(HAlign_Center)
											.VAlign(VAlign_Center)
											[
												IdolSlotsRef
											],
											FMargin(3.f),
											IdolSectionBorderColor)
									]
								]
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SBox)
								.WidthOverride(PortraitPanelSize)
								.HeightOverride(PortraitPanelSize)
								[
									MakeBottomLeftSectionPanel(
										bUseAlternateHudChrome
											? StaticCastSharedRef<SWidget>(
												SNew(SOverlay)
												+ SOverlay::Slot()
												[
													SNew(SBorder)
													.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
													.BorderBackgroundColor(FT66FlatStyle::PanelOuter())
													.Padding(1.f)
													[
														SNew(SBorder)
														.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
														.BorderBackgroundColor(FT66FlatStyle::Border())
														.Padding(1.f)
														[
															SAssignNew(PortraitBorder, SBorder)
															.BorderImage(bUseAlternateHudChrome ? FCoreStyle::Get().GetBrush("WhiteBrush") : GetGameplayHudSlotBrush(true))
															.BorderBackgroundColor(bUseAlternateHudChrome ? FT66FlatStyle::PanelInner() : T66HudPanelRed)
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
													.Font(FT66FlatStyle::Tokens::FontBold(11))
													.ColorAndOpacity(FT66FlatStyle::TextMuted())
													.Justification(ETextJustify::Center)
													.Visibility(EVisibility::Visible)
												]
												)
											: StaticCastSharedRef<SWidget>(
												SNew(SOverlay)
												+ SOverlay::Slot()
												[
													SAssignNew(PortraitBorder, SBorder)
													.BorderImage(bUseAlternateHudChrome ? FCoreStyle::Get().GetBrush("WhiteBrush") : GetGameplayHudSlotBrush(true))
													.BorderBackgroundColor(bUseAlternateHudChrome ? FLinearColor(0.12f, 0.12f, 0.14f, 1.f) : T66HudPanelRed)
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
													.Font(FT66FlatStyle::Tokens::FontBold(11))
													.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
													.Justification(ETextJustify::Center)
													.Visibility(EVisibility::Visible)
												]
												),
										FMargin(2.f),
										PortraitSectionBorderColor)
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
										.HeightOverride(0.f)
										[
											SNew(SBox)
											.HAlign(HAlign_Center)
											.VAlign(VAlign_Center)
											.Visibility(EVisibility::HitTestInvisible)
											[
												BackroomsReviveIconRowRef
											]
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
											.HeightOverride(0.f)
											.Visibility(EVisibility::Collapsed)
											[
												SAssignNew(UltimateBorder, SBorder)
														.BorderImage(bUseAlternateHudChrome ? FCoreStyle::Get().GetBrush("WhiteBrush") : GetGameplayHudSlotBrush(false))
														.BorderBackgroundColor(bUseAlternateHudChrome ? FLinearColor(0.03f, 0.03f, 0.05f, 1.f) : T66HudBorderRed)
														.Padding(0.f)
														[
															SNew(SOverlay)
															+ SOverlay::Slot()
															.Padding(AbilityIconInset)
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
																	.Font(FT66FlatStyle::Tokens::FontBold(16))
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
																		.Font(FT66FlatStyle::Tokens::FontBold(8))
																		.ColorAndOpacity(FLinearColor::White)
																		.Justification(ETextJustify::Center)
																	]
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
															.BorderImage(bUseAlternateHudChrome ? FCoreStyle::Get().GetBrush("WhiteBrush") : GetGameplayHudSlotBrush(false))
															.BorderBackgroundColor(bUseAlternateHudChrome ? FLinearColor(0.03f, 0.03f, 0.05f, 1.f) : T66HudBorderRed)
															.Padding(0.f)
															[
																SNew(SOverlay)
																+ SOverlay::Slot()
																.Padding(AbilityIconInset)
																[
																	SNew(SScaleBox)
																	.Stretch(EStretch::ScaleToFit)
																	[
																		SAssignNew(PassiveImage, SImage)
																		.Image(PassiveBrush.Get())
																		.ColorAndOpacity(FLinearColor::White)
																	]
																]
															]
														]
														+ SOverlay::Slot()
														.HAlign(HAlign_Right)
														.VAlign(VAlign_Bottom)
														.Padding(0.f, 0.f, 4.f, 4.f)
														[
															SAssignNew(PassiveStackBadgeBox, SBox)
															.WidthOverride(20.f)
															.HeightOverride(20.f)
															[
																SNew(SOverlay)
																+ SOverlay::Slot()
																[
																	SNew(SBorder)
																	.BorderImage(FT66FlatStyle::GetBrush(TEXT("T66.Brush.Circle")))
																	.BorderBackgroundColor(FLinearColor(0.12f, 0.10f, 0.08f, 0.95f))
																	.Padding(0.f)
																	.HAlign(HAlign_Center)
																	.VAlign(VAlign_Center)
																	[
																		SAssignNew(PassiveStackText, STextBlock)
																		.Text(FText::AsNumber(0))
																		.Font(FT66FlatStyle::Tokens::FontBold(9))
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
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(BottomLeftColumnGap, 0.f, 0.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(BaseStatsPanelWidth)
								.HeightOverride(PortraitPanelSize)
								[
									MakeBottomLeftSectionPanel(
										SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
										[
											MakeBaseStatLine(StatDamageText)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
										[
											MakeBaseStatLine(StatAttackSpeedText)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
										[
											MakeBaseStatLine(StatAttackScaleText)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
										[
											MakeBaseStatLine(StatArmorText)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
										[
											MakeBaseStatLine(StatEvasionText)
										]
										+ SVerticalBox::Slot().AutoHeight()
										[
											MakeBaseStatLine(StatLuckText)
										],
										FMargin(8.f, 6.f),
										BaseStatsSectionBorderColor)
								]
							]
							,
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
						FT66AnimatedStyle::AttachMetadata(
							SAssignNew(MinimapWidget, ST66WorldMapWidget)
							.bMinimap(true)
							.bShowLabels(false),
							TEXT("GameplayHUD.Minimap.Map"),
							TEXT("Minimap"))
					]
				]
				// Stage number + skulls beneath minimap, grouped in one compact black panel.
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)
				[
					SNew(SBox)
					.WidthOverride(MinimapWidth)
					[
						bUseAlternateHudChrome
							? FT66FlatStyle::MakeHudPanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SAssignNew(DifficultyAreaNameText, STextBlock)
								.Text(DifficultyAreaNameInit)
								.Font(FT66FlatStyle::Tokens::FontBold(10))
								.ColorAndOpacity(FT66FlatStyle::Tokens::Accent2)
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 2.f, 0.f, 0.f)
							[
								SAssignNew(StageText, STextBlock)
								.Text(StageInit)
								.Font(FT66FlatStyle::Tokens::FontBold(11))
								.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
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
								FT66FlatStyle::MakeBareButton(
									FT66BareButtonParams(
										FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnToggleImmortality),
										SAssignNew(ImmortalityButtonText, STextBlock)
										.Text(NSLOCTEXT("T66.Dev", "ImmortalityOff", "IMMORTALITY: OFF"))
										.Font(FT66FlatStyle::Tokens::FontBold(7))
										.ColorAndOpacity(FT66FlatStyle::Tokens::Text))
									.SetButtonStyle(&FT66FlatStyle::GetButtonStyle(TEXT("T66.Button.Neutral")))
									.SetColor(FT66FlatStyle::Tokens::Panel2)
									.SetPadding(FMargin(5.f, 2.f))
									.SetVisibility(EVisibility::Collapsed),
									&ImmortalityButton)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								FT66FlatStyle::MakeBareButton(
									FT66BareButtonParams(
										FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnTogglePower),
										SAssignNew(PowerButtonText, STextBlock)
										.Text(NSLOCTEXT("T66.Dev", "PowerOff", "POWER: OFF"))
										.Font(FT66FlatStyle::Tokens::FontBold(7))
										.ColorAndOpacity(FT66FlatStyle::Tokens::Text))
									.SetButtonStyle(&FT66FlatStyle::GetButtonStyle(TEXT("T66.Button.Neutral")))
									.SetColor(FT66FlatStyle::Tokens::Panel2)
									.SetPadding(FMargin(5.f, 2.f))
									.SetVisibility(EVisibility::Collapsed),
									&PowerButton)
							],
							FMargin(6.f, 4.f))
							: MakeGameplayHudSquarePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
							[
								SAssignNew(DifficultyAreaNameText, STextBlock)
								.Text(DifficultyAreaNameInit)
								.Font(FT66FlatStyle::Tokens::FontBold(10))
								.ColorAndOpacity(FT66FlatStyle::Tokens::Accent2)
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 2.f, 0.f, 0.f)
							[
								SAssignNew(StageText, STextBlock)
								.Text(StageInit)
								.Font(FT66FlatStyle::Tokens::FontBold(11))
								.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
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
								FT66FlatStyle::MakeBareButton(
									FT66BareButtonParams(
										FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnToggleImmortality),
										SAssignNew(ImmortalityButtonText, STextBlock)
										.Text(NSLOCTEXT("T66.Dev", "ImmortalityOff", "IMMORTALITY: OFF"))
										.Font(FT66FlatStyle::Tokens::FontBold(7))
										.ColorAndOpacity(FT66FlatStyle::Tokens::Text))
									.SetButtonStyle(&FT66FlatStyle::GetButtonStyle(TEXT("T66.Button.Neutral")))
									.SetColor(FT66FlatStyle::Tokens::Panel2)
									.SetPadding(FMargin(5.f, 2.f))
									.SetVisibility(EVisibility::Collapsed),
									&ImmortalityButton)
							]
							// Dev toggles are hidden in the current HUD pass.
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
							[
								FT66FlatStyle::MakeBareButton(
									FT66BareButtonParams(
										FOnClicked::CreateUObject(this, &UT66GameplayHUDWidget::OnTogglePower),
										SAssignNew(PowerButtonText, STextBlock)
										.Text(NSLOCTEXT("T66.Dev", "PowerOff", "POWER: OFF"))
										.Font(FT66FlatStyle::Tokens::FontBold(7))
										.ColorAndOpacity(FT66FlatStyle::Tokens::Text))
									.SetButtonStyle(&FT66FlatStyle::GetButtonStyle(TEXT("T66.Button.Neutral")))
									.SetColor(FT66FlatStyle::Tokens::Panel2)
									.SetPadding(FMargin(5.f, 2.f))
									.SetVisibility(EVisibility::Collapsed),
									&PowerButton)
							],
							FMargin(8.f, 6.f))
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
					bUseAlternateHudChrome
						? FT66FlatStyle::MakeHudPanel(
							MakeInventoryEconomySection(
								FT66FlatStyle::Accent2(),
								FT66FlatStyle::Danger(),
								WithAlpha(FT66FlatStyle::Border(), 0.65f)),
							FMargin(6.f, 5.f))
						: MakeGameplayHudSquarePanel(
							MakeInventoryEconomySection(
								FLinearColor(1.0f, 0.82f, 0.24f, 1.f),
								T66HudTextRed,
								T66HudDividerRed),
							FMargin(8.f, 6.f))
				]
			]
		]
		// Bottom-center interaction prompt, aligned with the bottom HUD band.
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 0.f, 12.f)
		[
			FT66FlatStyle::AttachMetadata(
				SAssignNew(InteractionPromptBox, SBox)
				.Visibility(EVisibility::Collapsed)
				.WidthOverride(440.f)
				.HeightOverride(54.f)
				[
					FT66FlatStyle::MakeFlatPanel(
						ET66FlatState::Default,
						FMargin(14.f, 10.f),
						FT66FlatStyle::AttachMetadata(
							SAssignNew(InteractionPromptTargetText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66FlatStyle::MakeBoldFont(18))
							.ColorAndOpacity(FT66FlatStyle::PrimaryText())
							.Justification(ETextJustify::Center)
							.AutoWrapText(false),
							TEXT("WorldInteractablePrompt.Text"),
							TEXT("Label.Body"),
							ET66FlatState::Default,
							TOptional<FLinearColor>(),
							false,
							NAME_None,
							true),
						nullptr,
						TEXT("WorldInteractablePrompt.Panel"))
				],
				TEXT("WorldInteractablePrompt.Root"),
				TEXT("InWorldPrompt"),
				ET66FlatState::Default,
				FT66FlatStyle::DefaultBorder())
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
				.BorderBackgroundColor(FLinearColor(0.58f, 0.42f, 0.22f, 0.96f))
				.Padding(3.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.58f, 0.42f, 0.22f, 0.96f))
					.Padding(FMargin(10.f, 8.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SAssignNew(AchievementNotificationTitleText, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66FlatStyle::Tokens::FontBold(16))
							.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
							.AutoWrapText(true)
							.WrapTextAt(256.f)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.GameplayHUD", "AchievementUnlocked", "Unlocked!"))
							.Font(FT66FlatStyle::Tokens::FontRegular(14))
							.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
						]
					]
				]
			]
		]
		// Chest reward presentation (target-owned post-interaction UI, non-pausing)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(BottomRightPickupPadding)
		[
			SAssignNew(ChestRewardBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.WidthOverride(BottomRightRewardLaneWidth)
			.HeightOverride(BottomRightRewardLaneHeight)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::DownOnly)
				[
					SNew(SBox)
					.WidthOverride(ChestRewardPanelWidth)
					.HeightOverride(ChestRewardPanelHeight)
					[
						SAssignNew(ChestRewardTileBorder, SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.58f, 0.42f, 0.22f, 0.72f))
						.Padding(3.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.018f, 0.014f, 0.012f, 0.82f))
							.Padding(FMargin(14.f, 10.f))
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								.HAlign(HAlign_Center)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.055f, 0.043f, 0.032f, 0.86f))
									.Padding(FMargin(18.f, 7.f))
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(SBox)
											.WidthOverride(28.f)
											.HeightOverride(28.f)
											[
												SNew(SImage)
												.Image(ChestRewardCoinBrush.Get())
												.ColorAndOpacity(FLinearColor::White)
											]
										]
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
										[
											SAssignNew(ChestRewardCounterText, STextBlock)
											.Text(FText::GetEmpty())
											.Font(FT66FlatStyle::Tokens::FontBold(32))
											.ColorAndOpacity(FLinearColor(0.98f, 0.83f, 0.24f, 1.f))
										]
									]
								]
								+ SVerticalBox::Slot().FillHeight(1.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.020f, 0.015f, 0.010f, 0.58f))
									.Padding(FMargin(0.f))
									[
										ChestRewardArtOverlay
									]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
								.HAlign(HAlign_Center)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.070f, 0.055f, 0.032f, 0.78f))
									.Padding(FMargin(14.f, 5.f))
									[
										SAssignNew(ChestRewardSkipText, STextBlock)
										.Text(FText::GetEmpty())
										.Font(FT66FlatStyle::Tokens::FontBold(11))
										.ColorAndOpacity(FLinearColor::White)
										.Justification(ETextJustify::Center)
									]
								]
							]
						]
					]
				]
			]
		]
		// Loot bag reveal: bag opens, card emerges, then hands off to the real pickup card.
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(BottomRightPickupPadding)
		[
			SAssignNew(LootBagRevealBox, SBox)
			.Visibility(EVisibility::Collapsed)
			.WidthOverride(PickupCardWidth)
			.HeightOverride(LootBagRevealPanelHeight)
			[
				LootBagRevealOverlay
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
				.BorderBackgroundColor(FLinearColor(0.58f, 0.42f, 0.22f, 0.96f))
				.Padding(2.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.030f, 0.025f, 0.021f, 0.98f))
					.Padding(0.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SAssignNew(PickupCardIconBorder, SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.028f, 0.026f, 0.032f, 0.98f))
							.Padding(FMargin(6.f))
							[
								SNew(SBox)
								.HeightOverride(104.f)
								[
									SNew(SScaleBox)
									.Stretch(EStretch::ScaleToFit)
									.StretchDirection(EStretchDirection::Both)
									[
										FT66FlatStyle::AttachMetadata(StaticCastSharedRef<SWidget>(
											SAssignNew(PickupCardIconImage, SImage)
											.Image(PickupCardIconBrush.Get())
											.ColorAndOpacity(FLinearColor::White)
											.Visibility(EVisibility::Collapsed)),
											TEXT("GameplayHUD.PickupCard.Icon"),
											TEXT("Icon"))
									]
								]
							]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.055f, 0.043f, 0.032f, 0.98f))
							.Padding(FMargin(10.f, 8.f))
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SAssignNew(PickupCardNameText, STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66FlatStyle::Tokens::FontBold(12))
									.ColorAndOpacity(FLinearColor::White)
									.AutoWrapText(true)
									.WrapTextAt(PickupCardWidth - 20.f)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
								[
									SAssignNew(PickupCardDescText, STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66FlatStyle::Tokens::FontRegular(10))
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
							.BorderBackgroundColor(FLinearColor(0.070f, 0.055f, 0.032f, 0.95f))
							.Padding(FMargin(8.f, 4.f))
							[
								SAssignNew(PickupCardSkipText, STextBlock)
								.Text(FText::GetEmpty())
								.Font(FT66FlatStyle::Tokens::FontBold(11))
								.ColorAndOpacity(FLinearColor::White)
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
			]
		]
		// Ragdoll recovery: center-bottom mash prompt and recovery meter.
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 0.f, 162.f)
		[
			SAssignNew(RagdollRecoveryPromptBox, SBox)
			.WidthOverride(RagdollRecoveryPromptWidth)
			.Visibility(EVisibility::Collapsed)
			[
				MakeGameplayHudSquarePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.GameplayHUD", "RagdollRecoveryPress", "PRESS"))
							.Font(FT66FlatStyle::Tokens::FontBold(12))
							.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.02f, 0.018f, 0.014f, 0.98f))
							.Padding(FMargin(10.f, 3.f))
							[
								SNew(STextBlock)
								.Text(JumpKeycapText)
								.Font(FT66FlatStyle::Tokens::FontBold(15))
								.ColorAndOpacity(FLinearColor(1.0f, 0.88f, 0.38f, 1.0f))
								.Justification(ETextJustify::Center)
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.GameplayHUD", "RagdollRecoveryMashJump", "MASH JUMP"))
							.Font(FT66FlatStyle::Tokens::FontBold(17))
							.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(RagdollRecoveryBarWidth)
						.HeightOverride(26.f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.025f, 0.025f, 0.028f, 0.94f))
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Fill)
							[
								SAssignNew(RagdollRecoveryFillBox, SBox)
								.WidthOverride(0.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.88f, 0.18f, 0.12f, 0.96f))
								]
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SAssignNew(RagdollRecoveryProgressText, STextBlock)
								.Text(NSLOCTEXT("T66.GameplayHUD", "RagdollRecoveryProgressInit", "RECOVERY 0%"))
								.Font(FT66FlatStyle::Tokens::FontBold(12))
								.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
								.Justification(ETextJustify::Center)
							]
						]
					],
					FMargin(14.f, 10.f))
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
				FT66FlatStyle::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(TutorialSubtitleSpeakerText, STextBlock)
						.Visibility(EVisibility::Collapsed)
						.Text(FText::GetEmpty())
						.Font(FT66FlatStyle::Tokens::FontBold(18))
						.ColorAndOpacity(FLinearColor(0.95f, 0.72f, 0.38f, 1.f))
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(TutorialSubtitleBodyText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66FlatStyle::Tokens::FontRegular(18))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
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
			FT66FlatStyle::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SAssignNew(TutorialHintLine1Text, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66FlatStyle::Tokens::FontBold(18))
					.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)
				[
					SAssignNew(TutorialHintLine2Text, STextBlock)
					.Visibility(EVisibility::Collapsed)
					.Text(FText::GetEmpty())
					.Font(FT66FlatStyle::Tokens::FontRegular(14))
					.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
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
				FT66AnimatedStyle::AttachMetadata(
					SAssignNew(CenterCrosshairWidget, ST66CrosshairWidget)
					.Locked(false),
					TEXT("GameplayHUD.Crosshair"),
					TEXT("Crosshair"))
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
					FT66AnimatedStyle::AttachMetadata(
						SNew(ST66ScopedSniperWidget),
						TEXT("GameplayHUD.ScopedSniperOverlay.Scope"),
						TEXT("ScopeOverlay"))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(0.f, 24.f, 0.f, 0.f)
				[
					MakeGameplayHudSquarePanel(
						SAssignNew(ScopedUltTimerText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66FlatStyle::Tokens::FontBold(20))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text),
						FMargin(12.f, 8.f))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Bottom)
				.Padding(0.f, 0.f, 0.f, 42.f)
				[
					MakeGameplayHudSquarePanel(
						SAssignNew(ScopedShotCooldownText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66FlatStyle::Tokens::FontBold(18))
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text),
						FMargin(14.f, 8.f))
				]
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
			.BorderBackgroundColor(WithAlpha(FT66FlatStyle::BackgroundColor(), 0.40f))
		]
		// Full-screen map overlay (OpenFullMap / M)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(FullMapOverlayBorder, SBorder)
			.Visibility(EVisibility::Collapsed)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.010f, 0.008f, 0.008f, 0.84f))
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					FT66FlatStyle::MakeFlatPanel(
						ET66FlatState::Default,
						FMargin(18.f, 14.f),
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.Map", "Title", "MAP"))
								.Font(FT66FlatStyle::MakeBoldFont(18))
								.ColorAndOpacity(FT66FlatStyle::PrimaryText())
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.Map", "CloseHint", "[M] Close"))
								.Font(FT66FlatStyle::MakeBoldFont(12))
								.ColorAndOpacity(FT66FlatStyle::SecondaryText())
							]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBox)
							.WidthOverride(FullMapWidthAttr)
							.HeightOverride(FullMapHeightAttr)
							[
								FT66FlatStyle::MakeFlatPanel(
									ET66FlatState::Default,
									FMargin(10.f),
									FT66AnimatedStyle::AttachMetadata(
										SAssignNew(FullMapWidget, ST66WorldMapWidget)
										.bMinimap(false)
										.bShowLabels(true),
										TEXT("GameplayHUD.FullMap.Map"),
										TEXT("FullMap")),
									nullptr,
									TEXT("GameplayHUD.FullMap.MapFrame"))
							]
						]
					,
					nullptr,
					TEXT("GameplayHUD.FullMap.Panel"))
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
				? WithAlpha(FT66FlatStyle::SelectedBorder(), 0.95f)
				: WithAlpha(FT66FlatStyle::DefaultFill(), 0.90f));
		}
		if (OptionTexts.IsValidIndex(i) && OptionTexts[i].IsValid())
		{
			OptionTexts[i]->SetColorAndOpacity(bSelected ? FT66FlatStyle::Tokens::Text : FT66FlatStyle::Tokens::TextMuted);
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
			SlotContainer->SetWidthOverride(SlotSize);
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
