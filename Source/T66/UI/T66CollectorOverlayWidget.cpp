// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CollectorOverlayWidget.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "UI/Style/T66FlatStyle.h"
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
	if (NPCID == FName(TEXT("Fountain"))) GM->SpawnLabFountain();
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
	{
		if (GI->IsLabRun())
		{
			GI->SelectedRunCategory = ET66RunCategory::Tower;
		}
	}
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
		PC->RestoreGameplayInputMode();
	UT66GameInstance::TransitionToFrontendLevel(World);
}

void UT66CollectorOverlayWidget::CloseOverlay()
{
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
		PC->RestoreGameplayInputMode();
}

void UT66CollectorOverlayWidget::RefreshContent()
{
	FT66FlatStyle::DeferRebuild(this);
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

	auto MakeCollectorLabel = [](const FText& Text, int32 FontSize, bool bBold, FSlateColor Color, FName Tag, const FString& Role) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.AutoWrapText(true),
			Tag,
			Role,
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	};

	// Tab row
	auto MakeTab = [&](const FText& Label, int32 Index) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatButton(
			CollectorTabIndex == Index ? ET66FlatState::Selected : ET66FlatState::Default,
			Label,
			FOnClicked::CreateLambda([this, Index]() { CollectorTabIndex = Index; FT66FlatStyle::DeferRebuild(this); return FReply::Handled(); }),
			nullptr,
			nullptr,
			FMargin(12.f, 6.f),
			0.f,
			34.f,
			true,
			11,
			FName(*FString::Printf(TEXT("CollectorOverlay.Tab.%d"), Index)),
			FName(TEXT("CollectorOverlayTabs")));
	};

	TSharedRef<SHorizontalBox> TabRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)[MakeTab(TabItems, 0)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)[MakeTab(TabNPCs, 1)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)[MakeTab(TabEnemies, 2)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)[MakeTab(TabInteractables, 3)];

	// Content: scroll of cards (sprite, description, ADD/Spawn)
	TSharedRef<SScrollBox> Scroll = SNew(SScrollBox);
	int32 ItemCardIndex = 0;
	int32 SpawnCardIndex = 0;

	auto AddItemCard = [&](FName ItemID, const FText& NameText, const FText& DescText)
	{
		FName CapturedID = ItemID;
		++ItemCardIndex;
		const int32 CapturedCardIndex = ItemCardIndex;
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
				SNew(SBox)
				.HeightOverride(128.f)
				[
					FT66FlatStyle::MakeFlatPanel(
						ET66FlatState::Default,
						FMargin(8.f),
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f)
						[
							SNew(SBox).WidthOverride(64.f).HeightOverride(64.f)
							[
								FT66FlatStyle::AttachMetadata(
									SNew(SImage)
									.Image(IconBrush.Get())
									.ColorAndOpacity(FLinearColor::White),
									FName(*FString::Printf(TEXT("CollectorOverlay.ItemIcon.%02d"), CapturedCardIndex)),
									TEXT("Icon"),
									ET66FlatState::Default)
							]
						]
						+ SHorizontalBox::Slot().FillWidth(1.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								MakeCollectorLabel(
									NameText,
									12,
									true,
									FSlateColor(FT66FlatStyle::PrimaryText()),
									FName(*FString::Printf(TEXT("CollectorOverlay.ItemTitle.%02d"), CapturedCardIndex)),
									TEXT("Label.Body"))
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)
							[
								MakeCollectorLabel(
									DescText,
									10,
									false,
									FSlateColor(FT66FlatStyle::SecondaryText()),
									FName(*FString::Printf(TEXT("CollectorOverlay.ItemDescription.%02d"), CapturedCardIndex)),
									TEXT("Label.Caption"))
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
							[
								FT66FlatStyle::MakeFlatButton(
									ET66FlatState::Default,
									AddBtn,
									FOnClicked::CreateLambda([this, CapturedID]() { OnAddItem(CapturedID); return FReply::Handled(); }),
									nullptr,
									nullptr,
									FMargin(10.f, 4.f),
									76.f,
									30.f,
									true,
									10,
									FName(*FString::Printf(TEXT("CollectorOverlay.ItemAddButton.%02d"), CapturedCardIndex)))
							]
						],
						nullptr,
						FName(*FString::Printf(TEXT("CollectorOverlay.ItemCard.%02d"), CapturedCardIndex)))
				]
			];
	};

	auto AddSpawnCard = [&](const FText& NameText, const FText& DescText, TFunction<void()> OnSpawn)
	{
		++SpawnCardIndex;
		const int32 CapturedCardIndex = SpawnCardIndex;
		Scroll->AddSlot().Padding(6.f)
			[
				SNew(SBox)
				.HeightOverride(104.f)
				[
					FT66FlatStyle::MakeFlatPanel(
						ET66FlatState::Default,
						FMargin(8.f),
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							MakeCollectorLabel(
								NameText,
								12,
								true,
								FSlateColor(FT66FlatStyle::PrimaryText()),
								FName(*FString::Printf(TEXT("CollectorOverlay.SpawnTitle.%02d"), CapturedCardIndex)),
								TEXT("Label.Body"))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)
						[
							MakeCollectorLabel(
								DescText,
								10,
								false,
								FSlateColor(FT66FlatStyle::SecondaryText()),
								FName(*FString::Printf(TEXT("CollectorOverlay.SpawnDescription.%02d"), CapturedCardIndex)),
								TEXT("Label.Caption"))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
						[
							FT66FlatStyle::MakeFlatButton(
								ET66FlatState::Default,
								SpawnBtn,
								FOnClicked::CreateLambda([OnSpawn]() { OnSpawn(); return FReply::Handled(); }),
								nullptr,
								nullptr,
								FMargin(10.f, 4.f),
								86.f,
								30.f,
								true,
								10,
								FName(*FString::Printf(TEXT("CollectorOverlay.SpawnButton.%02d"), CapturedCardIndex)))
						],
						nullptr,
						FName(*FString::Printf(TEXT("CollectorOverlay.SpawnCard.%02d"), CapturedCardIndex)))
				]
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
				const FText BaseStat = StaticEnum<ET66HeroStatType>()->GetDisplayNameTextByValue(static_cast<int64>(ItemData.BaseStatType));
				const FText Stat = StaticEnum<ET66StatType>()->GetDisplayNameTextByValue(static_cast<int64>(ItemData.StatType));
				if (ItemData.BaseStatType == ET66HeroStatType::Special)
				{
					DescText = ItemData.StatType != ET66StatType::None
						? FText::Format(LOCTEXT("SpecialDescLineWithSecondary", "Primary: {0}\nLine 2: {1}"), BaseStat, Stat)
						: FText::Format(LOCTEXT("SpecialDescLine", "Primary: {0}"), BaseStat);
				}
				else
				{
					DescText = FText::Format(LOCTEXT("DescLine", "Line 1: +{0}\nLine 2: {1}"), BaseStat, Stat);
				}
			}
			AddItemCard(ItemID, NameText, DescText);
		}
	}
	else if (CollectorTabIndex == 1)
	{
		AddSpawnCard(
			LOCTEXT("FountainName", "Fountain"),
			LOCTEXT("FountainDesc", "NPC: Fountain."),
			[this]() { OnSpawnNPC(FName(TEXT("Fountain"))); }
		);
	}
	else if (CollectorTabIndex == 2)
	{
		static const TArray<FName> MobIDs =
		{
			FName(TEXT("Slime")),
			FName(TEXT("TombSpider")),
			FName(TEXT("TuskerBoar")),
			FName(TEXT("JellyHover")),
			FName(TEXT("Gargoyle"))
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
		AddSpawnCard(LOCTEXT("FountainInteractableName", "Fountain"), FText::FromString(TEXT("Interactable")), [this]() { OnSpawnInteractable(FName(TEXT("Fountain"))); });
		AddSpawnCard(LOCTEXT("Chest", "Chest"), FText::FromString(TEXT("Interactable")), [this]() { OnSpawnInteractable(FName(TEXT("Chest"))); });
		AddSpawnCard(LOCTEXT("TNTInteractableName", "TNT"), FText::FromString(TEXT("Interactable")), [this]() { OnSpawnInteractable(FName(TEXT("TNT"))); });
		AddSpawnCard(LOCTEXT("IdolAltar", "Idol Altar"), FText::FromString(TEXT("Interactable")), [this]() { OnSpawnInteractable(FName(TEXT("IdolAltar"))); });
	}

	TSharedRef<SVerticalBox> MainPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
		[
			MakeCollectorLabel(
				TitleCollector,
				20,
				true,
				FSlateColor(FT66FlatStyle::PrimaryText()),
				FName(TEXT("CollectorOverlay.Title")),
				TEXT("Label.Header"))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[TabRow]
		+ SVerticalBox::Slot().FillHeight(1.f)[SNew(SBox).HeightOverride(360.f)[Scroll]]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				FT66FlatStyle::MakeFlatButton(
					ET66FlatState::Default,
					CloseText,
					FOnClicked::CreateLambda([this]() { CloseOverlay(); return FReply::Handled(); }),
					nullptr,
					nullptr,
					FMargin(12.f, 6.f),
					92.f,
					34.f,
					true,
					11,
					FName(TEXT("CollectorOverlay.CloseButton")))
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				FT66FlatStyle::MakeFlatButton(
					ET66FlatState::Selected,
					ExitLab,
					FOnClicked::CreateLambda([this]() { OnExitLab(); return FReply::Handled(); }),
					nullptr,
					nullptr,
					FMargin(12.f, 6.f),
					128.f,
					34.f,
					true,
					11,
					FName(TEXT("CollectorOverlay.ExitButton")))
			]
		];

	const TAttribute<FMargin> SafePanelPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66FlatStyle::GetSafePadding(FMargin(40.f, 126.f, 40.f, 40.f));
	});

	const FLinearColor BackdropColor = FLinearColor(
		FT66FlatStyle::BackgroundColor().R,
		FT66FlatStyle::BackgroundColor().G,
		FT66FlatStyle::BackgroundColor().B,
		0.86f);

	TSharedRef<SWidget> Root = FT66FlatStyle::AttachMetadata(
		SNew(SBox).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				FT66FlatStyle::AttachMetadata(
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BackdropColor),
					FName(TEXT("CollectorOverlay.Backdrop")),
					TEXT("Panel"),
					ET66FlatState::Default)
			]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(SafePanelPadding)
			[
				SNew(SBox).WidthOverride(564.f).HeightOverride(568.5f)
				[
					FT66FlatStyle::MakeFlatPanel(
						ET66FlatState::Default,
						FMargin(24.f),
						MainPanel,
						nullptr,
						FName(TEXT("CollectorOverlay.Panel")))
				]
			]
		],
		FName(TEXT("CollectorOverlay.Root")),
		TEXT("Overlay"),
		ET66FlatState::Default);

	return FT66FlatStyle::MakeResponsiveRoot(Root);
}

#undef LOCTEXT_NAMESPACE
