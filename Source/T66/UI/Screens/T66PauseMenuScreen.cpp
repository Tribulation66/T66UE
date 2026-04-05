// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PauseMenuScreen.h"

#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"

#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/DataTable.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

UT66PauseMenuScreen::UT66PauseMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PauseMenu;
	bIsModal = true;
}

AT66PlayerController* UT66PauseMenuScreen::GetT66PlayerController() const
{
	return Cast<AT66PlayerController>(GetOwningPlayer());
}

TSharedRef<SWidget> UT66PauseMenuScreen::BuildSlateUI()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* GI = Cast<UT66GameInstance>(GameInstance);
	UT66LocalizationSubsystem* Loc = GameInstance ? GameInstance->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GameInstance ? GameInstance->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	const bool bDotaTheme = FT66Style::IsDotaTheme();
	const float SideColumnWidth = bDotaTheme ? 300.f : 340.f;
	const float ButtonColumnWidth = bDotaTheme ? 320.f : 360.f;
	const float ButtonMinWidth = bDotaTheme ? 280.f : 312.f;
	const float PanelGap = bDotaTheme ? 18.f : 24.f;
	const float PortraitSize = bDotaTheme ? 176.f : 208.f;
	const float HeartSize = bDotaTheme ? 34.f : 40.f;
	const float IdolSlotSize = bDotaTheme ? 40.f : 46.f;
	const float InventorySlotSize = bDotaTheme ? 34.f : 40.f;
	const float StatsPanelWidth = bDotaTheme ? 300.f : 340.f;
	const int32 InventoryColumns = 5;
	const int32 InventoryRows = FMath::DivideAndRoundUp(UT66RunStateSubsystem::MaxInventorySlots, InventoryColumns);

	const FText ResumeText = Loc ? Loc->GetText_Resume() : NSLOCTEXT("T66.PauseMenu", "Resume", "RESUME GAME");
	const FText SaveAndQuitText = Loc ? Loc->GetText_SaveAndQuit() : NSLOCTEXT("T66.PauseMenu", "SaveAndQuit", "SAVE AND QUIT");
	const FText RestartText = Loc ? Loc->GetText_Restart() : NSLOCTEXT("T66.PauseMenu", "Restart", "RESTART");
	const FText SettingsText = Loc ? Loc->GetText_Settings() : NSLOCTEXT("T66.PauseMenu", "Settings", "SETTINGS");
	const FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.Achievements", "Title", "ACHIEVEMENTS");
	const FText LeaderboardText = Loc ? Loc->GetText_Leaderboard() : NSLOCTEXT("T66.Leaderboard", "Title", "LEADERBOARD");
	const FText ScoreLabelText = Loc ? Loc->GetText_ScoreLabel() : NSLOCTEXT("T66.GameplayHUD", "ScoreLabel", "Score:");
	const FText InventoryTitleText = NSLOCTEXT("T66.GameplayHUD", "InventoryTitle", "Inventory");
	const FText IdolsTitleText = NSLOCTEXT("T66.GameplayHUD", "IdolsTitle", "IDOLS");
	const FText PortraitLabel = Loc ? Loc->GetText_PortraitPlaceholder() : NSLOCTEXT("T66.GameplayHUD", "PortraitLabel", "PORTRAIT");

	auto MakePauseButton = [this, bDotaTheme, ButtonMinWidth](const FText& Text, FReply (UT66PauseMenuScreen::*ClickFunc)(), ET66ButtonType Type) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HAlign(HAlign_Fill)
			.Padding(FMargin(0.f, bDotaTheme ? 4.f : 6.f))
			[
				FT66Style::MakeButton(
					FT66ButtonParams(Text, FOnClicked::CreateUObject(this, ClickFunc), Type)
					.SetFontSize(bDotaTheme ? 20 : 28)
					.SetFontWeight(bDotaTheme ? TEXT("Regular") : TEXT("Bold"))
					.SetPadding(bDotaTheme ? FMargin(18.f, 10.f, 18.f, 8.f) : FMargin(18.f, 12.f))
					.SetMinWidth(ButtonMinWidth)
					.SetHeight(bDotaTheme ? 60.f : 66.f))
			];
	};

	auto MakeIconSlot = [bDotaTheme](const TSharedPtr<FSlateBrush>& IconBrush, const FLinearColor& SlotColor, float SlotSize) -> TSharedRef<SWidget>
	{
		if (bDotaTheme)
		{
			return SNew(SBox)
				.WidthOverride(SlotSize)
				.HeightOverride(SlotSize)
				[
					FT66Style::MakeSlotFrame(
						SNew(SImage)
						.Image(IconBrush.IsValid() ? IconBrush.Get() : nullptr)
						.ColorAndOpacity(FLinearColor::White),
						SlotColor,
						FMargin(2.f))
				];
		}

		const FLinearColor OuterColor = FLinearColor(0.f, 0.f, 0.f, 0.25f);
		const FLinearColor InnerFrameColor = SlotColor;
		const FLinearColor FillColor = FLinearColor(0.f, 0.f, 0.f, 0.f);

		return SNew(SBox)
			.WidthOverride(SlotSize)
			.HeightOverride(SlotSize)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(OuterColor)
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(InnerFrameColor)
						.Padding(0.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FillColor)
						]
					]
				]
				+ SOverlay::Slot()
				.Padding(2.f)
				[
					SNew(SImage)
					.Image(IconBrush.IsValid() ? IconBrush.Get() : nullptr)
					.ColorAndOpacity(FLinearColor::White)
				]
			];
	};

	if (!PortraitBrush.IsValid())
	{
		PortraitBrush = MakeShared<FSlateBrush>();
		PortraitBrush->DrawAs = ESlateBrushDrawType::Image;
		PortraitBrush->ImageSize = FVector2D(1.f, 1.f);
	}

	if (!HeartBrush.IsValid())
	{
		HeartBrush = MakeShared<FSlateBrush>();
		HeartBrush->DrawAs = ESlateBrushDrawType::Image;
		HeartBrush->ImageSize = FVector2D(40.f, 40.f);
		HeartBrush->Tiling = ESlateBrushTileType::NoTile;
	}

	if (TexPool)
	{
		const TSoftObjectPtr<UTexture2D> HeartSoft(FSoftObjectPath(TEXT("/Game/UI/Sprites/UI/HEARTS.HEARTS")));
		T66SlateTexture::BindSharedBrushAsync(TexPool, HeartSoft, this, HeartBrush, FName(TEXT("PauseHeart")), false);
	}

	IdolSlotBrushes.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
	for (int32 Index = 0; Index < IdolSlotBrushes.Num(); ++Index)
	{
		if (!IdolSlotBrushes[Index].IsValid())
		{
			IdolSlotBrushes[Index] = MakeShared<FSlateBrush>();
			IdolSlotBrushes[Index]->DrawAs = ESlateBrushDrawType::Image;
			IdolSlotBrushes[Index]->ImageSize = FVector2D(IdolSlotSize, IdolSlotSize);
		}
	}

	InventorySlotBrushes.SetNum(UT66RunStateSubsystem::MaxInventorySlots);
	for (int32 Index = 0; Index < InventorySlotBrushes.Num(); ++Index)
	{
		if (!InventorySlotBrushes[Index].IsValid())
		{
			InventorySlotBrushes[Index] = MakeShared<FSlateBrush>();
			InventorySlotBrushes[Index]->DrawAs = ESlateBrushDrawType::Image;
			InventorySlotBrushes[Index]->ImageSize = FVector2D(InventorySlotSize, InventorySlotSize);
		}
	}

	const int32 HeartTier = RunState ? RunState->GetHeartDisplayTier() : 0;
	const FLinearColor FilledHeartColor = (RunState && RunState->IsSaintBlessingActive())
		? FLinearColor(0.95f, 0.95f, 1.00f, 1.f)
		: FT66RarityUtil::GetTierColor(HeartTier);
	const FLinearColor EmptyHeartColor(0.25f, 0.25f, 0.28f, 0.35f);
	const FLinearColor PortraitBorderColor = RunState
		? (RunState->IsSaintBlessingActive()
			? FLinearColor(0.92f, 0.92f, 0.98f, 1.f)
			: (RunState->GetMaxHP() > 0.f ? FT66RarityUtil::GetTierColor(HeartTier) : (FT66Style::IsDotaTheme() ? FT66Style::Border() : FLinearColor(0.12f, 0.12f, 0.14f, 1.f))))
		: (FT66Style::IsDotaTheme() ? FT66Style::Border() : FLinearColor(0.12f, 0.12f, 0.14f, 1.f));

	bool bHasPortrait = false;
	if (GI && RunState)
	{
		ET66HeroPortraitVariant PortraitVariant = RunState->IsSaintBlessingActive()
			? ET66HeroPortraitVariant::Invincible
			: ET66HeroPortraitVariant::Half;
		if (!RunState->IsSaintBlessingActive() && RunState->GetCurrentHearts() <= 1)
		{
			PortraitVariant = ET66HeroPortraitVariant::Low;
		}
		else if (!RunState->IsSaintBlessingActive() && RunState->GetCurrentHearts() >= 5)
		{
			PortraitVariant = ET66HeroPortraitVariant::Full;
		}

		FHeroData HeroData;
		if (!GI->SelectedHeroID.IsNone() && GI->GetHeroData(GI->SelectedHeroID, HeroData))
		{
			const TSoftObjectPtr<UTexture2D> PortraitSoft = GI->ResolveHeroPortrait(HeroData, GI->SelectedHeroBodyType, PortraitVariant);
			bHasPortrait = !PortraitSoft.IsNull();
			if (bHasPortrait && TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, PortraitBrush, FName(TEXT("PausePortrait")), true);
			}
		}
	}

	if (!bHasPortrait && PortraitBrush.IsValid())
	{
		PortraitBrush->SetResourceObject(nullptr);
	}

	TSharedRef<SHorizontalBox> HeartsRow = SNew(SHorizontalBox);
	for (int32 HeartIndex = 0; HeartIndex < UT66RunStateSubsystem::DefaultMaxHearts; ++HeartIndex)
	{
		const float Fill = RunState ? RunState->GetHeartSlotFill(HeartIndex) : 0.f;
		const bool bFilled = Fill > 0.01f;
		const FLinearColor HeartColor = (RunState && RunState->IsSaintBlessingActive())
			? FLinearColor(0.95f, 0.95f, 1.00f, 1.f)
			: (RunState ? FT66RarityUtil::GetTierColor(RunState->GetHeartSlotTier(HeartIndex)) : FilledHeartColor);
		HeartsRow->AddSlot()
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(HeartSize)
				.HeightOverride(HeartSize)
				[
					SNew(SImage)
					.Image(HeartBrush.Get())
					.ColorAndOpacity(bFilled ? HeartColor : EmptyHeartColor)
				]
			];
	}

	TSharedRef<SWidget> ScorePanel = FT66Style::MakePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(ScoreLabelText)
			.Font(FT66Style::Tokens::FontBold(18))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(RunState ? FText::AsNumber(RunState->GetCurrentScore()) : FText::AsNumber(0))
			.Font(FT66Style::Tokens::FontBold(34))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text_Lambda([RunState]() -> FText
			{
				if (!RunState)
				{
					return NSLOCTEXT("T66.PauseMenu", "ScoreMultiplierFallback", "x1.0");
				}

				FNumberFormattingOptions Options;
				Options.MinimumFractionalDigits = 1;
				Options.MaximumFractionalDigits = 1;
				return FText::Format(
					NSLOCTEXT("T66.GameplayHUD", "ScoreMultiplierFormat", "x{0}"),
					FText::AsNumber(RunState->GetDifficultyScalar(), &Options));
			})
			.Font(FT66Style::Tokens::FontBold(18))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		],
		FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(16.f, 14.f)));

	TSharedRef<SWidget> PortraitPanel = FT66Style::MakePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
		[
			HeartsRow
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBox)
			.WidthOverride(PortraitSize)
			.HeightOverride(PortraitSize)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(PortraitBorderColor)
				]
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image(PortraitBrush.Get())
					.ColorAndOpacity(FLinearColor::White)
					.Visibility(bHasPortrait ? EVisibility::Visible : EVisibility::Collapsed)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(PortraitLabel)
					.Font(FT66Style::Tokens::FontBold(11))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.Justification(ETextJustify::Center)
					.Visibility(bHasPortrait ? EVisibility::Collapsed : EVisibility::Visible)
				]
			]
		],
		FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f)));

	TSharedRef<SUniformGridPanel> IdolGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(3.f));

	UT66IdolManagerSubsystem* IdolManager = GI ? GI->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
	static const TArray<FName> EmptyIdolIDs;
	static const TArray<uint8> EmptyIdolTierValues;
	const TArray<FName>& EquippedIdols = IdolManager ? IdolManager->GetEquippedIdols() : (RunState ? RunState->GetEquippedIdols() : EmptyIdolIDs);
	const TArray<uint8>& EquippedIdolTierValues = IdolManager ? IdolManager->GetEquippedIdolTierValues() : (RunState ? RunState->GetEquippedIdolTierValues() : EmptyIdolTierValues);
	for (int32 SlotIndex = 0; SlotIndex < UT66IdolManagerSubsystem::MaxEquippedIdolSlots; ++SlotIndex)
	{
		FLinearColor IdolSlotColor(0.10f, 0.16f, 0.14f, 0.92f);
		TSoftObjectPtr<UTexture2D> IdolIconSoft;

		if (GI && EquippedIdols.IsValidIndex(SlotIndex) && !EquippedIdols[SlotIndex].IsNone())
		{
			FIdolData IdolData;
			if (GI->GetIdolData(EquippedIdols[SlotIndex], IdolData))
			{
				const int32 IdolTierValue = EquippedIdolTierValues.IsValidIndex(SlotIndex)
					? FMath::Clamp(static_cast<int32>(EquippedIdolTierValues[SlotIndex]), 1, UT66IdolManagerSubsystem::MaxIdolLevel)
					: 1;
				const ET66ItemRarity IdolRarity = UT66IdolManagerSubsystem::IdolTierValueToRarity(IdolTierValue);
				IdolSlotColor = FItemData::GetItemRarityColor(IdolRarity);
				IdolIconSoft = IdolData.GetIconForRarity(IdolRarity);
			}
		}

		if (IdolSlotBrushes.IsValidIndex(SlotIndex) && IdolSlotBrushes[SlotIndex].IsValid())
		{
			if (!IdolIconSoft.IsNull() && TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, IdolIconSoft, this, IdolSlotBrushes[SlotIndex], FName(TEXT("PauseIdol"), SlotIndex + 1), true);
			}
			else
			{
				IdolSlotBrushes[SlotIndex]->SetResourceObject(nullptr);
			}
		}

		IdolGrid->AddSlot(SlotIndex, 0)
		[
			MakeIconSlot(
				IdolSlotBrushes.IsValidIndex(SlotIndex) ? IdolSlotBrushes[SlotIndex] : nullptr,
				IdolSlotColor,
				IdolSlotSize)
		];
	}

	TSharedRef<SWidget> IdolsPanel = FT66Style::MakePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(IdolsTitleText)
			.Font(FT66Style::Tokens::FontBold(16))
			.ColorAndOpacity(bDotaTheme ? FT66Style::Text() : FLinearColor(0.75f, 0.82f, 0.78f, 1.f))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
		[
			IdolGrid
		],
		FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 10.f)));

	static constexpr float InventorySlotPad = 2.f;
	TSharedRef<SVerticalBox> InventoryGrid = SNew(SVerticalBox);
	const TArray<FName> InventoryIDs = RunState ? RunState->GetInventory() : TArray<FName>();
	const TArray<FT66InventorySlot> InventorySlots = RunState ? RunState->GetInventorySlots() : TArray<FT66InventorySlot>();
	const TArray<FName>* DisplayInventoryIDs = &InventoryIDs;
	const TArray<FT66InventorySlot>* DisplayInventorySlots = &InventorySlots;

	for (int32 Row = 0; Row < InventoryRows; ++Row)
	{
		TSharedRef<SHorizontalBox> RowBox = SNew(SHorizontalBox);
		for (int32 Column = 0; Column < InventoryColumns; ++Column)
		{
			const int32 SlotIndex = (Row * InventoryColumns) + Column;
			FLinearColor SlotColor = FLinearColor(0.f, 0.f, 0.f, 0.25f);
			TSoftObjectPtr<UTexture2D> SlotIconSoft;

			if (GI && DisplayInventoryIDs->IsValidIndex(SlotIndex) && !(*DisplayInventoryIDs)[SlotIndex].IsNone())
			{
				FItemData ItemData;
				if (GI->GetItemData((*DisplayInventoryIDs)[SlotIndex], ItemData))
				{
					const ET66ItemRarity SlotRarity = DisplayInventorySlots->IsValidIndex(SlotIndex) ? (*DisplayInventorySlots)[SlotIndex].Rarity : ET66ItemRarity::Black;
					SlotColor = FItemData::GetItemRarityColor(SlotRarity);
					SlotIconSoft = ItemData.GetIconForRarity(SlotRarity);
				}
			}

			if (InventorySlotBrushes.IsValidIndex(SlotIndex) && InventorySlotBrushes[SlotIndex].IsValid())
			{
				if (!SlotIconSoft.IsNull() && TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, SlotIconSoft, this, InventorySlotBrushes[SlotIndex], FName(TEXT("PauseInv"), SlotIndex + 1), true);
				}
				else
				{
					InventorySlotBrushes[SlotIndex]->SetResourceObject(nullptr);
				}
			}

			RowBox->AddSlot()
				.AutoWidth()
				.Padding(InventorySlotPad)
				[
					MakeIconSlot(
						InventorySlotBrushes.IsValidIndex(SlotIndex) ? InventorySlotBrushes[SlotIndex] : nullptr,
						SlotColor,
						InventorySlotSize)
				];
		}

		InventoryGrid->AddSlot().AutoHeight()[ RowBox ];
	}

	TSharedRef<SWidget> InventoryPanel = FT66Style::MakePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(InventoryTitleText)
			.Font(FT66Style::Tokens::FontBold(16))
			.ColorAndOpacity(bDotaTheme ? FT66Style::Text() : FLinearColor(0.75f, 0.82f, 0.78f, 1.f))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 6.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(RunState ? FText::Format(Loc ? Loc->GetText_NetWorthFormat() : NSLOCTEXT("T66.GameplayHUD", "NetWorthInit", "Net Worth: 0"), FText::AsNumber(RunState->GetNetWorth())) : FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(14))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(RunState ? FText::Format(Loc ? Loc->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldInit", "Gold: 0"), FText::AsNumber(RunState->GetCurrentGold())) : FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(14))
				.ColorAndOpacity(FLinearColor(0.85f, 0.75f, 0.20f, 1.f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(RunState ? FText::Format(Loc ? Loc->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweInit", "Debt: 0"), FText::AsNumber(RunState->GetCurrentDebt())) : FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(14))
				.ColorAndOpacity(FT66Style::Tokens::Danger)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
		[
			InventoryGrid
		],
		FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 10.f)));

	TSharedRef<SWidget> StatsPanel = RunState
		? T66StatsPanelSlate::MakeEssentialStatsPanel(RunState, Loc, StatsPanelWidth, false)
		: FT66Style::MakePanel(
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.PauseMenu", "StatsUnavailable", "Stats unavailable"))
			.Font(FT66Style::Tokens::FontRegular(14))
			.ColorAndOpacity(FT66Style::Tokens::Text),
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(16.f)));

	FSlateFontInfo PauseTitleFont = FT66Style::Tokens::FontBold(bDotaTheme ? 48 : 44);
	if (bDotaTheme)
	{
		PauseTitleFont.LetterSpacing = 120;
	}

	TSharedRef<SWidget> ButtonsPanel = FT66Style::MakePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 18.f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.PauseMenu", "PausedTitle", "PAUSED"))
			.Font(PauseTitleFont)
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
		[ MakePauseButton(ResumeText, &UT66PauseMenuScreen::HandleResumeClicked, ET66ButtonType::Success) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
		[ MakePauseButton(SaveAndQuitText, &UT66PauseMenuScreen::HandleSaveAndQuitClicked, ET66ButtonType::Neutral) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
		[ MakePauseButton(RestartText, &UT66PauseMenuScreen::HandleRestartClicked, ET66ButtonType::Danger) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
		[ MakePauseButton(SettingsText, &UT66PauseMenuScreen::HandleSettingsClicked, ET66ButtonType::Neutral) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
		[ MakePauseButton(AchievementsText, &UT66PauseMenuScreen::HandleAchievementsClicked, ET66ButtonType::Neutral) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
		[ MakePauseButton(LeaderboardText, &UT66PauseMenuScreen::HandleLeaderboardClicked, ET66ButtonType::Neutral) ],
		FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(FT66Style::Tokens::Space5, FT66Style::Tokens::Space4)));

	TSharedRef<SWidget> LeftColumn = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			PortraitPanel
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
		[
			IdolsPanel
		];

	TSharedRef<SWidget> RightColumn = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			StatsPanel
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
		[
			InventoryPanel
		];

	const FLinearColor ScrimColor = FT66Style::IsDotaTheme()
		? FT66Style::Scrim()
		: FLinearColor(FT66Style::Tokens::Scrim.R, FT66Style::Tokens::Scrim.G, FT66Style::Tokens::Scrim.B, 0.88f);
	const FMargin ScorePadding = FT66Style::GetSafePadding(FMargin(20.f, 18.f, 0.f, 0.f));

	TSharedRef<SWidget> MainRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(0.f, 0.f, PanelGap, 0.f)
		[
			SNew(SBox)
			.WidthOverride(SideColumnWidth)
			[
				LeftColumn
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Top)
		.Padding(0.f, 0.f, PanelGap, 0.f)
		[
			SNew(SBox)
			.WidthOverride(ButtonColumnWidth)
			[
				ButtonsPanel
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		[
			SNew(SBox)
			.WidthOverride(SideColumnWidth)
			[
				RightColumn
			]
		];

	TSharedRef<SWidget> PauseSurface = bDotaTheme
		? StaticCastSharedRef<SWidget>(FT66Style::MakeViewportFrame(MainRow, FMargin(22.f, 20.f)))
		: StaticCastSharedRef<SWidget>(FT66Style::MakePanel(
			MainRow,
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(22.f, 20.f))));
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(ScrimColor)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(ScorePadding)
			[
				ScorePanel
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				FT66Style::MakeSafeFrame(PauseSurface, FMargin(18.f, 18.f))
			]
		];
}

FReply UT66PauseMenuScreen::HandleResumeClicked() { OnResumeClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleSaveAndQuitClicked() { OnSaveAndQuitClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleRestartClicked() { OnRestartClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleSettingsClicked() { OnSettingsClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleAchievementsClicked() { OnAchievementsClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleLeaderboardClicked() { OnLeaderboardClicked(); return FReply::Handled(); }

void UT66PauseMenuScreen::OnResumeClicked()
{
	CloseModal();
	AT66PlayerController* PC = GetT66PlayerController();
	if (PC)
	{
		PC->SetPause(false);
		PC->RestoreGameplayInputMode();
	}
}

void UT66PauseMenuScreen::OnSaveAndQuitClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	AT66PlayerController* PC = GetT66PlayerController();
	if (!GI || !PC) return;

	UT66SaveSubsystem* SaveSub = GI->GetSubsystem<UT66SaveSubsystem>();
	if (!SaveSub) return;

	int32 SlotIndex = GI->CurrentSaveSlotIndex;
	if (SlotIndex < 0 || SlotIndex >= UT66SaveSubsystem::MaxSlots)
	{
		SlotIndex = SaveSub->FindFirstEmptySlot();
		if (SlotIndex == INDEX_NONE)
		{
			SlotIndex = SaveSub->FindOldestOccupiedSlot();
			if (SlotIndex == INDEX_NONE) return;
		}
		GI->CurrentSaveSlotIndex = SlotIndex;
	}

	UT66RunSaveGame* SaveObj = NewObject<UT66RunSaveGame>(this);
	if (!SaveObj) return;

	SaveObj->HeroID = GI->SelectedHeroID;
	SaveObj->HeroBodyType = GI->SelectedHeroBodyType;
	SaveObj->CompanionID = GI->SelectedCompanionID;
	SaveObj->Difficulty = GI->SelectedDifficulty;
	SaveObj->PartySize = GI->SelectedPartySize;
	SaveObj->MapName = GetWorld() ? UWorld::RemovePIEPrefix(GetWorld()->GetMapName()) : FString();
	SaveObj->LastPlayedUtc = FDateTime::UtcNow().ToIso8601();
	SaveObj->RunSeed = GI->RunSeed;
	SaveObj->bRunIneligibleForLeaderboard = GI->bRunIneligibleForLeaderboard;

	if (UT66RunStateSubsystem* LocalRunState = GI->GetSubsystem<UT66RunStateSubsystem>())
	{
		SaveObj->StageReached = LocalRunState->GetCurrentStage();
		if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
		{
			SaveObj->EquippedIdols = IdolManager->GetEquippedIdols();
			SaveObj->EquippedIdolTiers = IdolManager->GetEquippedIdolTierValues();
		}
		else
		{
			SaveObj->EquippedIdols = LocalRunState->GetEquippedIdols();
			SaveObj->EquippedIdolTiers = LocalRunState->GetEquippedIdolTierValues();
		}
	}

	if (APawn* Pawn = PC->GetPawn())
	{
		SaveObj->PlayerTransform = Pawn->GetActorTransform();
	}

	const UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>();
	SaveObj->OwnerPlayerId = !GI->CurrentRunOwnerPlayerId.IsEmpty()
		? GI->CurrentRunOwnerPlayerId
		: (PartySubsystem ? PartySubsystem->GetLocalPlayerId() : TEXT("local_player"));
	SaveObj->OwnerDisplayName = !GI->CurrentRunOwnerDisplayName.IsEmpty()
		? GI->CurrentRunOwnerDisplayName
		: (PartySubsystem ? PartySubsystem->GetLocalDisplayName() : TEXT("You"));
	SaveObj->PartyMemberIds = GI->CurrentRunPartyMemberIds.Num() > 0
		? GI->CurrentRunPartyMemberIds
		: (PartySubsystem ? PartySubsystem->GetCurrentPartyMemberIds() : TArray<FString>());
	SaveObj->PartyMemberDisplayNames = GI->CurrentRunPartyMemberDisplayNames.Num() > 0
		? GI->CurrentRunPartyMemberDisplayNames
		: (PartySubsystem ? PartySubsystem->GetCurrentPartyMemberDisplayNames() : TArray<FString>());
	if (SaveObj->PartyMemberIds.Num() == 0 && !SaveObj->OwnerPlayerId.IsEmpty())
	{
		SaveObj->PartyMemberIds.Add(SaveObj->OwnerPlayerId);
	}
	if (SaveObj->PartyMemberDisplayNames.Num() == 0 && !SaveObj->OwnerDisplayName.IsEmpty())
	{
		SaveObj->PartyMemberDisplayNames.Add(SaveObj->OwnerDisplayName);
	}

	SaveObj->SavedPartyPlayers.Reset();
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			for (APlayerState* PlayerState : GameState->PlayerArray)
			{
				const AT66SessionPlayerState* SessionPlayerState = Cast<AT66SessionPlayerState>(PlayerState);
				if (!SessionPlayerState)
				{
					continue;
				}

				const FT66LobbyPlayerInfo& LobbyInfo = SessionPlayerState->GetLobbyInfo();
				if (LobbyInfo.SteamId.IsEmpty())
				{
					continue;
				}

				FT66SavedPartyPlayerState& SavedPlayer = SaveObj->SavedPartyPlayers.AddDefaulted_GetRef();
				SavedPlayer.PlayerId = LobbyInfo.SteamId;
				SavedPlayer.DisplayName = LobbyInfo.DisplayName;
				SavedPlayer.HeroID = LobbyInfo.SelectedHeroID;
				SavedPlayer.HeroBodyType = LobbyInfo.SelectedHeroBodyType;
				SavedPlayer.HeroSkinID = LobbyInfo.SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : LobbyInfo.SelectedHeroSkinID;
				SavedPlayer.CompanionID = LobbyInfo.SelectedCompanionID;
				SavedPlayer.CompanionBodyType = LobbyInfo.SelectedCompanionBodyType;
				SavedPlayer.bIsPartyHost = LobbyInfo.bPartyHost;
			}
		}
	}

	if (SaveObj->SavedPartyPlayers.Num() == 0)
	{
		FT66SavedPartyPlayerState& LocalSavedPlayer = SaveObj->SavedPartyPlayers.AddDefaulted_GetRef();
		LocalSavedPlayer.PlayerId = SaveObj->OwnerPlayerId;
		LocalSavedPlayer.DisplayName = SaveObj->OwnerDisplayName;
		LocalSavedPlayer.HeroID = SaveObj->HeroID;
		LocalSavedPlayer.HeroBodyType = SaveObj->HeroBodyType;
		LocalSavedPlayer.HeroSkinID = GI->SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : GI->SelectedHeroSkinID;
		LocalSavedPlayer.CompanionID = SaveObj->CompanionID;
		LocalSavedPlayer.PlayerTransform = SaveObj->PlayerTransform;
		LocalSavedPlayer.bIsPartyHost = true;
	}
	else
	{
		for (FT66SavedPartyPlayerState& SavedPlayer : SaveObj->SavedPartyPlayers)
		{
			if (SavedPlayer.PlayerId == SaveObj->OwnerPlayerId)
			{
				SavedPlayer.PlayerTransform = SaveObj->PlayerTransform;
				SavedPlayer.bIsPartyHost = true;
				break;
			}
		}
	}

	SaveObj->PartyMemberIds.Reset();
	SaveObj->PartyMemberDisplayNames.Reset();
	for (const FT66SavedPartyPlayerState& SavedPlayer : SaveObj->SavedPartyPlayers)
	{
		if (!SavedPlayer.PlayerId.IsEmpty())
		{
			SaveObj->PartyMemberIds.AddUnique(SavedPlayer.PlayerId);
		}
		if (!SavedPlayer.DisplayName.IsEmpty())
		{
			SaveObj->PartyMemberDisplayNames.AddUnique(SavedPlayer.DisplayName);
		}
	}

	switch (SaveObj->SavedPartyPlayers.Num())
	{
	case 4:
		SaveObj->PartySize = ET66PartySize::Quad;
		break;
	case 3:
		SaveObj->PartySize = ET66PartySize::Trio;
		break;
	case 2:
		SaveObj->PartySize = ET66PartySize::Duo;
		break;
	case 1:
	default:
		SaveObj->PartySize = ET66PartySize::Solo;
		break;
	}

	if (!SaveSub->SaveToSlot(SlotIndex, SaveObj)) return;

	PC->SetPause(false);
	UGameplayStatics::OpenLevel(this, FName(TEXT("FrontendLevel")));
}

void UT66PauseMenuScreen::OnRestartClicked()
{
	AT66PlayerController* PC = GetT66PlayerController();
	if (PC) PC->SetPause(false);
	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetTribulationEntryLevelName());
}

void UT66PauseMenuScreen::OnSettingsClicked()
{
	ShowModal(ET66ScreenType::Settings);
}

void UT66PauseMenuScreen::OnAchievementsClicked()
{
	ShowModal(ET66ScreenType::Achievements);
}

void UT66PauseMenuScreen::OnLeaderboardClicked()
{
	ShowModal(ET66ScreenType::Leaderboard);
}

