// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CollectorOverlayWidget.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "UI/Style/T66OverlayChromeStyle.h"
#include "UI/Style/T66Style.h"
#include "UI/T66SlateTextureHelpers.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"

#define LOCTEXT_NAMESPACE "T66.Collector"

TArray<FName> UT66CollectorOverlayWidget::GetUnlockedItemIDs() const
{
	TArray<FName> Out;
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66AchievementsSubsystem* Achieve = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	if (!Achieve || !Achieve->GetProfile() || !T66GI || !T66GI->GetItemsDataTable()) return Out;
	for (const FName& ItemID : Achieve->GetProfile()->LabUnlockedItemIDs)
	{
		if (ItemID.IsNone()) continue;
		FItemData Dummy;
		if (T66GI->GetItemData(ItemID, Dummy)) Out.Add(ItemID);
	}
	return Out;
}

TArray<FName> UT66CollectorOverlayWidget::GetUnlockedEnemyIDs() const
{
	TArray<FName> Out;
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66AchievementsSubsystem* Achieve = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (Achieve && Achieve->GetProfile()) Out = Achieve->GetProfile()->LabUnlockedEnemyIDs;
	return Out;
}

void UT66CollectorOverlayWidget::OnAddItem(FName ItemID)
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState && RunState->HasInventorySpace()) RunState->AddItem(ItemID);
	RefreshContent();
}

void UT66CollectorOverlayWidget::OnSpawnNPC(FName NPCID)
{
	AT66GameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AT66GameMode>() : nullptr;
	if (!GM) return;
	if (NPCID == FName(TEXT("Fountain"))) GM->SpawnLabFountainOfLife();
	RefreshContent();
}

void UT66CollectorOverlayWidget::OnSpawnEnemy(FName EnemyID, bool bIsBoss)
{
	AT66GameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AT66GameMode>() : nullptr;
	if (!GM) return;
	if (bIsBoss) GM->SpawnLabBoss(EnemyID);
	else GM->SpawnLabMob(EnemyID);
	RefreshContent();
}

void UT66CollectorOverlayWidget::OnSpawnInteractable(FName InteractableID)
{
	AT66GameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AT66GameMode>() : nullptr;
	if (GM) GM->SpawnLabInteractable(InteractableID);
	RefreshContent();
}

void UT66CollectorOverlayWidget::OnExitLab()
{
	UWorld* World = GetWorld();
	UGameInstance* GIBase = World ? World->GetGameInstance() : nullptr;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GIBase))
		GI->bIsLabLevel = false;
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
		PC->RestoreGameplayInputMode();
	UGameplayStatics::OpenLevel(World, UT66GameInstance::GetFrontendLevelName());
}

void UT66CollectorOverlayWidget::CloseOverlay()
{
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
		PC->RestoreGameplayInputMode();
}

void UT66CollectorOverlayWidget::RefreshContent()
{
	FT66Style::DeferRebuild(this);
}

void UT66CollectorOverlayWidget::NativeDestruct()
{
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->IsGameplayLevel() && !PC->IsPaused())
			PC->RestoreGameplayInputMode();
	}
	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66CollectorOverlayWidget::RebuildWidget()
{
	const FText TitleCollector = LOCTEXT("CollectorTitle", "The Collector");
	const FText TabItems = LOCTEXT("TabItems", "Items");
	const FText TabNPCs = LOCTEXT("TabNPCs", "NPCs");
	const FText TabEnemies = LOCTEXT("TabEnemies", "Enemies");
	const FText TabInteractables = LOCTEXT("TabInteractables", "Interactables");
	const FText AddBtn = LOCTEXT("Add", "ADD");
	const FText SpawnBtn = LOCTEXT("Spawn", "Spawn");
	const FText ExitLab = LOCTEXT("ExitLab", "Exit The Lab");
	const FText CloseText = LOCTEXT("Close", "Close");

	UWorld* World = GetWorld();
	UGameInstance* GIBase = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* GI = Cast<UT66GameInstance>(GIBase);
	UT66AchievementsSubsystem* Achieve = GIBase ? GIBase->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;

	ItemIconBrushes.Empty();

	// Tab row
	auto MakeTab = [&](const FText& Label, int32 Index) -> TSharedRef<SWidget>
	{
		return T66OverlayChromeStyle::MakeButton(
			T66OverlayChromeStyle::MakeButtonParams(
				Label,
				FOnClicked::CreateLambda([this, Index]() { CollectorTabIndex = Index; FT66Style::DeferRebuild(this); return FReply::Handled(); }),
				ET66OverlayChromeButtonFamily::Tab)
			.SetMinWidth(0.f)
			.SetFontSize(11)
			.SetPadding(FMargin(12.f, 6.f))
			.SetSelected(TAttribute<bool>::CreateLambda([this, Index]() { return CollectorTabIndex == Index; })));
	};

	TSharedRef<SHorizontalBox> TabRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)[MakeTab(TabItems, 0)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)[MakeTab(TabNPCs, 1)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)[MakeTab(TabEnemies, 2)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)[MakeTab(TabInteractables, 3)];

	// Content: scroll of cards (sprite, description, ADD/Spawn)
	TSharedRef<SScrollBox> Scroll = SNew(SScrollBox);

	auto AddItemCard = [&](FName ItemID, const FText& NameText, const FText& DescText)
	{
		FName CapturedID = ItemID;
		TSharedPtr<FSlateBrush> IconBrush = MakeShared<FSlateBrush>();
		IconBrush->DrawAs = ESlateBrushDrawType::Image;
		IconBrush->ImageSize = FVector2D(64.f, 64.f);
		ItemIconBrushes.Add(IconBrush);
		FItemData ItemData;
		if (GI && GI->GetItemData(ItemID, ItemData))
		{
			const TSoftObjectPtr<UTexture2D> ItemIconSoft = ItemData.GetIconForRarity(ET66ItemRarity::Black);
			UT66UITexturePoolSubsystem* Pool = GIBase ? GIBase->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
			if (Pool && !ItemIconSoft.IsNull()) T66SlateTexture::BindSharedBrushAsync(Pool, ItemIconSoft, this, IconBrush, ItemID, true);
		}
		Scroll->AddSlot().Padding(6.f)
			[
				T66OverlayChromeStyle::MakePanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f)
					[
						SNew(SBox).WidthOverride(64.f).HeightOverride(64.f)
						[
							SNew(SImage).Image(IconBrush.Get()).ColorAndOpacity(FLinearColor::White)
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(NameText).Font(FT66Style::Tokens::FontBold(12)).ColorAndOpacity(FT66Style::Tokens::Text)]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)[SNew(STextBlock).Text(DescText).Font(FT66Style::Tokens::FontRegular(10)).ColorAndOpacity(FT66Style::Tokens::TextMuted).AutoWrapText(true)]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
						[
							T66OverlayChromeStyle::MakeButton(
								T66OverlayChromeStyle::MakeButtonParams(
									AddBtn,
									FOnClicked::CreateLambda([this, CapturedID]() { OnAddItem(CapturedID); return FReply::Handled(); }),
									ET66OverlayChromeButtonFamily::Primary))
						]
					],
					ET66OverlayChromeBrush::OfferCardNormal,
					FMargin(8.f)
				)
			];
	};

	auto AddSpawnCard = [&](const FText& NameText, const FText& DescText, TFunction<void()> OnSpawn)
	{
		Scroll->AddSlot().Padding(6.f)
			[
				T66OverlayChromeStyle::MakePanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(NameText).Font(FT66Style::Tokens::FontBold(12)).ColorAndOpacity(FT66Style::Tokens::Text)]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)[SNew(STextBlock).Text(DescText).Font(FT66Style::Tokens::FontRegular(10)).ColorAndOpacity(FT66Style::Tokens::TextMuted).AutoWrapText(true)]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
						[
							T66OverlayChromeStyle::MakeButton(
								T66OverlayChromeStyle::MakeButtonParams(
									SpawnBtn,
									FOnClicked::CreateLambda([OnSpawn]() { OnSpawn(); return FReply::Handled(); }),
									ET66OverlayChromeButtonFamily::Neutral))
						]
					],
					ET66OverlayChromeBrush::OfferCardNormal,
					FMargin(8.f)
				)
			];
	};

	if (CollectorTabIndex == 0)
	{
		for (const FName& ItemID : GetUnlockedItemIDs())
		{
			FItemData ItemData;
			FText NameText = FText::FromName(ItemID);
			FText DescText = FText::GetEmpty();
			if (GI && GI->GetItemData(ItemID, ItemData))
			{
				// Build description from primary + secondary stat types.
				const FText PrimaryStat = StaticEnum<ET66HeroStatType>()->GetDisplayNameTextByValue(static_cast<int64>(ItemData.PrimaryStatType));
				const FText SecondaryStat = StaticEnum<ET66SecondaryStatType>()->GetDisplayNameTextByValue(static_cast<int64>(ItemData.SecondaryStatType));
				DescText = FText::Format(LOCTEXT("DescLine", "Line 1: +{0}\nLine 2: {1}"), PrimaryStat, SecondaryStat);
			}
			AddItemCard(ItemID, NameText, DescText);
		}
	}
	else if (CollectorTabIndex == 1)
	{
		AddSpawnCard(
			LOCTEXT("FountainOfLifeName", "Fountain of Life"),
			LOCTEXT("FountainOfLifeDesc", "NPC: Fountain of Life."),
			[this]() { OnSpawnNPC(FName(TEXT("Fountain"))); }
		);
	}
	else if (CollectorTabIndex == 2)
	{
		static const TArray<FName> MobIDs =
		{
			FName(TEXT("Cow")),
			FName(TEXT("Pig")),
			FName(TEXT("Goat")),
			FName(TEXT("Roost")),
			FName(TEXT("GoblinThief")),
			FName(TEXT("UniqueEnemy"))
		};
		TArray<FName> EnemyIDs = GetUnlockedEnemyIDs();
		for (const FName& M : MobIDs)
			if (EnemyIDs.Contains(M))
				AddSpawnCard(FText::FromName(M), FText::FromString(TEXT("Mob")), [this, M]() { OnSpawnEnemy(M, false); });
		if (GI && GI->GetBossesDataTable())
			for (const FName& RowName : GI->GetBossesDataTable()->GetRowNames())
				if (EnemyIDs.Contains(RowName))
					AddSpawnCard(FText::FromName(RowName), FText::FromString(TEXT("Stage Boss")), [this, RowName]() { OnSpawnEnemy(RowName, true); });
	}
	else
	{
		AddSpawnCard(LOCTEXT("FountainOfLifeInteractableName", "Fountain of Life"), FText::FromString(TEXT("Interactable")), [this]() { OnSpawnInteractable(FName(TEXT("Fountain"))); });
		AddSpawnCard(LOCTEXT("Chest", "Chest"), FText::FromString(TEXT("Interactable")), [this]() { OnSpawnInteractable(FName(TEXT("Chest"))); });
		AddSpawnCard(LOCTEXT("WheelSpin", "Wheel Spin"), FText::FromString(TEXT("Interactable")), [this]() { OnSpawnInteractable(FName(TEXT("WheelSpin"))); });
		AddSpawnCard(LOCTEXT("IdolAltar", "Idol Altar"), FText::FromString(TEXT("Interactable")), [this]() { OnSpawnInteractable(FName(TEXT("IdolAltar"))); });
	}

	TSharedRef<SVerticalBox> MainPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
		[SNew(STextBlock).Text(TitleCollector).Font(FT66Style::Tokens::FontBold(20)).ColorAndOpacity(FT66Style::Tokens::Text)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[TabRow]
		+ SVerticalBox::Slot().FillHeight(1.f)[SNew(SBox).HeightOverride(360.f)[Scroll]]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				T66OverlayChromeStyle::MakeButton(
					T66OverlayChromeStyle::MakeButtonParams(
						CloseText,
						FOnClicked::CreateLambda([this]() { CloseOverlay(); return FReply::Handled(); }),
						ET66OverlayChromeButtonFamily::Neutral))
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				T66OverlayChromeStyle::MakeButton(
					T66OverlayChromeStyle::MakeButtonParams(
						ExitLab,
						FOnClicked::CreateLambda([this]() { OnExitLab(); return FReply::Handled(); }),
						ET66OverlayChromeButtonFamily::Danger))
			]
		];

	const TAttribute<FMargin> SafePanelPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66Style::GetSafePadding(FMargin(40.f, 126.f, 40.f, 40.f));
	});

	TSharedRef<SWidget> Root = SNew(SBox).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.010f, 0.008f, 0.008f, 0.86f))
			]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(SafePanelPadding)
			[
				SNew(SBox).WidthOverride(564.f)
				[
					T66OverlayChromeStyle::MakePanel(
						MainPanel,
						ET66OverlayChromeBrush::OverlayModalPanel,
						FMargin(24.f))
				]
			]
		];

	return FT66Style::MakeResponsiveRoot(Root);
}

#undef LOCTEXT_NAMESPACE
