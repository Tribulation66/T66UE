// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66LabOverlayWidget.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Data/T66DataTypes.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SOverlay.h"

#define LOCTEXT_NAMESPACE "T66.Lab"

// Right-side strip: match inventory panel width (10 slots * 42 + 9*4 pad + 24 border = 480); top offset so panel sits between Power and inventory.
static constexpr float LabPanelWidth = 480.f;
static constexpr float LabPanelTopOffset = 340.f;
static constexpr float LabPanelMaxHeight = 220.f;

TArray<FName> UT66LabOverlayWidget::GetUnlockedItemIDs() const
{
	TArray<FName> Out;
	UT66AchievementsSubsystem* Achieve = GetAchievements();
	UT66GameInstance* GI = GetT66GameInstance();
	if (!Achieve || !GI || !GI->GetItemsDataTable()) return Out;
	for (const FName& ItemID : Achieve->GetProfile() ? Achieve->GetProfile()->LabUnlockedItemIDs : TArray<FName>())
	{
		if (ItemID.IsNone()) continue;
		FItemData Dummy;
		if (GI->GetItemData(ItemID, Dummy))
			Out.Add(ItemID);
	}
	return Out;
}

TArray<FName> UT66LabOverlayWidget::GetUnlockedEnemyIDs() const
{
	TArray<FName> Out;
	UT66AchievementsSubsystem* Achieve = GetAchievements();
	if (!Achieve || !Achieve->GetProfile()) return Out;
	Out = Achieve->GetProfile()->LabUnlockedEnemyIDs;
	return Out;
}

UT66GameInstance* UT66LabOverlayWidget::GetT66GameInstance() const
{
	return GetWorld() ? Cast<UT66GameInstance>(GetWorld()->GetGameInstance()) : nullptr;
}

AT66GameMode* UT66LabOverlayWidget::GetLabGameMode() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AT66GameMode>() : nullptr;
}

UT66RunStateSubsystem* UT66LabOverlayWidget::GetRunState() const
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}

UT66AchievementsSubsystem* UT66LabOverlayWidget::GetAchievements() const
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
}

void UT66LabOverlayWidget::OnGrantItem(FName ItemID)
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->AddItem(ItemID);
		RefreshLabUI();
	}
}

void UT66LabOverlayWidget::OnResetItems()
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->ClearInventory();
		RefreshLabUI();
	}
}

void UT66LabOverlayWidget::OnSpawnEnemy(FName EnemyID, int32 TabIndex)
{
	AT66GameMode* GM = GetLabGameMode();
	if (!GM) return;
	if (TabIndex == 1 && EnemyID == FName(TEXT("TreeOfLife")))
	{
		GM->SpawnLabTreeOfLife();
	}
	else if (TabIndex == 2)
	{
		GM->SpawnLabMob(EnemyID);
	}
	else if (TabIndex == 3)
	{
		GM->SpawnLabBoss(EnemyID);
	}
	RefreshLabUI();
}

void UT66LabOverlayWidget::OnResetEnemies()
{
	if (AT66GameMode* GM = GetLabGameMode())
	{
		GM->ResetLabSpawnedActors();
		RefreshLabUI();
	}
}

void UT66LabOverlayWidget::OnExitLab()
{
	if (UT66GameInstance* GI = GetT66GameInstance())
	{
		GI->bIsLabLevel = false;
	}
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("FrontendLevel")));
}

void UT66LabOverlayWidget::OnToggleLabPanel()
{
	bLabPanelExpanded = !bLabPanelExpanded;
	RefreshLabUI();
}

void UT66LabOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UT66LabOverlayWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UT66LabOverlayWidget::RefreshLabUI()
{
	if (GetCachedWidget().IsValid())
	{
		ReleaseSlateResources(true);
		TakeWidget();
	}
}

TSharedRef<SWidget> UT66LabOverlayWidget::RebuildWidget()
{
	const FText TitleItems = LOCTEXT("LabItems", "Items");
	const FText ResetItems = LOCTEXT("LabResetItems", "Reset Items");
	const FText ResetEnemies = LOCTEXT("LabResetEnemies", "Reset Enemies");
	const FText ExitLab = LOCTEXT("LabExit", "Exit The Lab");
	const FText TabItems = LOCTEXT("LabTabItems", "Items");
	const FText TabNPC = LOCTEXT("LabTabNPC", "NPC");
	const FText TabMobs = LOCTEXT("LabTabMobs", "Mobs");
	const FText TabBosses = LOCTEXT("LabTabBosses", "Stage Bosses");
	const FText Spawn = LOCTEXT("LabSpawn", "Spawn");
	const FText LabToggle = LOCTEXT("LabToggle", "Lab");

	TArray<FName> ItemIDs = GetUnlockedItemIDs();
	TArray<FName> EnemyIDs = GetUnlockedEnemyIDs();

	static const FName TreeOfLifeID(TEXT("TreeOfLife"));
	static const TArray<FName> MobIDs = { FName(TEXT("RegularEnemy")), FName(TEXT("Leprechaun")), FName(TEXT("GoblinThief")), FName(TEXT("UniqueEnemy")) };
	TArray<FName> NPCList;
	NPCList.Add(TreeOfLifeID);
	TArray<FName> MobsList;
	for (const FName& M : MobIDs) { if (EnemyIDs.Contains(M)) MobsList.Add(M); }
	TArray<FName> BossList;
	UT66GameInstance* GI = GetT66GameInstance();
	if (GI && GI->GetBossesDataTable())
	{
		TArray<FName> RowNames = GI->GetBossesDataTable()->GetRowNames();
		for (const FName& R : RowNames) { if (EnemyIDs.Contains(R)) BossList.Add(R); }
	}

	// Items tab content
	TSharedRef<SScrollBox> ItemsScroll = SNew(SScrollBox);
	for (const FName& ItemID : ItemIDs)
	{
		FName CapturedID = ItemID;
		ItemsScroll->AddSlot()
			.Padding(2.f)
			[
				FT66Style::MakeButton(
					FText::FromName(ItemID),
					FOnClicked::CreateLambda([this, CapturedID]() { OnGrantItem(CapturedID); return FReply::Handled(); }),
					ET66ButtonType::Neutral
				)
			];
	}

	auto BuildEnemyList = [&](const TArray<FName>& List, int32 TabIdx) -> TSharedRef<SWidget>
	{
		TSharedRef<SScrollBox> Box = SNew(SScrollBox);
		for (const FName& EID : List)
		{
			FName CapturedEID = EID;
			int32 CapturedTab = TabIdx;
			Box->AddSlot()
				.Padding(2.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(STextBlock).Text(FText::FromName(EID)).Font(FT66Style::Tokens::FontRegular(11)).ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						FT66Style::MakeButton(
							Spawn,
							FOnClicked::CreateLambda([this, CapturedEID, CapturedTab]() { OnSpawnEnemy(CapturedEID, CapturedTab); return FReply::Handled(); }),
							ET66ButtonType::Neutral,
							80.f,
							28.f
						)
					]
				];
		}
		return Box;
	};

	// Tab content: 0=Items, 1=NPC, 2=Mobs, 3=Bosses
	TSharedRef<SWidget> TabContent =
		(LabTabIndex == 0) ? StaticCastSharedRef<SWidget>(ItemsScroll) :
		(LabTabIndex == 1) ? BuildEnemyList(NPCList, 1) :
		(LabTabIndex == 2) ? BuildEnemyList(MobsList, 2) :
		BuildEnemyList(BossList, 3);

	// Tab row: Items | NPC | Mobs | Stage Bosses
	TSharedRef<SHorizontalBox> TabRow = SNew(SHorizontalBox);
	auto AddTab = [&](const FText& Label, int32 Index)
	{
		TabRow->AddSlot().AutoWidth().Padding(2.f)
			[
				SNew(SButton)
				.OnClicked_Lambda([this, Index]() { LabTabIndex = Index; ReleaseSlateResources(true); TakeWidget(); return FReply::Handled(); })
				.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
				.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
				.ContentPadding(FMargin(6.f, 2.f))
				[
					SNew(STextBlock).Text(Label).Font(FT66Style::Tokens::FontBold(9)).ColorAndOpacity(FT66Style::Tokens::Text)
				]
			];
	};
	AddTab(TabItems, 0);
	AddTab(TabNPC, 1);
	AddTab(TabMobs, 2);
	AddTab(TabBosses, 3);

	// Toggle button (like minimap): show "Lab" and expand/collapse state
	TSharedRef<SButton> ToggleButton = SNew(SButton)
		.OnClicked(FOnClicked::CreateLambda([this]() { OnToggleLabPanel(); return FReply::Handled(); }))
		.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
		.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
		.ContentPadding(FMargin(8.f, 4.f))
		[
			SNew(STextBlock)
			.Text(bLabPanelExpanded ? LOCTEXT("LabHide", "Lab \u25BC") : LOCTEXT("LabShow", "Lab \u25B6"))
			.Font(FT66Style::Tokens::FontBold(10))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		];

	// Single right-side panel: toggle + (when expanded) tabs + content + Reset + Exit
	TSharedRef<SVerticalBox> LabPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			ToggleButton
		];

	if (bLabPanelExpanded)
	{
		LabPanel->AddSlot().AutoHeight().Padding(0.f, 6.f)
			[TabRow];
		LabPanel->AddSlot()
			.FillHeight(1.f)
			[
				SNew(SBox).HeightOverride(LabPanelMaxHeight)[TabContent]
			];
		// One dynamic "Reset xxx" button: Reset Items when Items tab, Reset Enemies when NPC/Mobs/Bosses
		const FText ResetLabel = (LabTabIndex == 0) ? ResetItems : ResetEnemies;
		const bool bIsItemsTab = (LabTabIndex == 0);
		LabPanel->AddSlot().AutoHeight().Padding(0.f, 4.f)
			[
				FT66Style::MakeButton(
					ResetLabel,
					FOnClicked::CreateLambda([this, bIsItemsTab]() {
						if (bIsItemsTab) OnResetItems();
						else OnResetEnemies();
						return FReply::Handled();
					}),
					bIsItemsTab ? ET66ButtonType::Neutral : ET66ButtonType::Danger
				)
			];
		LabPanel->AddSlot().AutoHeight().Padding(0.f, 4.f)
			[
				FT66Style::MakeButton(ExitLab, FOnClicked::CreateLambda([this]() { OnExitLab(); return FReply::Handled(); }), ET66ButtonType::Danger)
			];
	}

	// Full-screen overlay: transparent, only the right-side panel has content (between Power and inventory)
	return SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(24.f, LabPanelTopOffset, 24.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(LabPanelWidth)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FT66Style::Tokens::Panel)
					.Padding(8.f)
					[LabPanel]
				]
			]
		];
}

#undef LOCTEXT_NAMESPACE
