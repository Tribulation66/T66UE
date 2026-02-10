// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66CollectorOverlayWidget.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
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
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66AchievementsSubsystem* Achieve = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetWorld() ? Cast<UT66GameInstance>(GetWorld()->GetGameInstance()) : nullptr;
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
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState && RunState->HasInventorySpace()) RunState->AddItem(ItemID);
	RefreshContent();
}

void UT66CollectorOverlayWidget::OnSpawnNPC(FName NPCID)
{
	AT66GameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AT66GameMode>() : nullptr;
	if (!GM) return;
	if (NPCID == FName(TEXT("TreeOfLife"))) GM->SpawnLabTreeOfLife();
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
	if (UT66GameInstance* GI = GetWorld() ? Cast<UT66GameInstance>(GetWorld()->GetGameInstance()) : nullptr)
		GI->bIsLabLevel = false;
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
		PC->RestoreGameplayInputMode();
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("FrontendLevel")));
}

void UT66CollectorOverlayWidget::CloseOverlay()
{
	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
		PC->RestoreGameplayInputMode();
}

void UT66CollectorOverlayWidget::RefreshContent()
{
	if (GetCachedWidget().IsValid()) { ReleaseSlateResources(true); TakeWidget(); }
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

	UT66GameInstance* GI = GetWorld() ? Cast<UT66GameInstance>(GetWorld()->GetGameInstance()) : nullptr;
	UT66AchievementsSubsystem* Achieve = GetWorld() && GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;

	// Tab row
	auto MakeTab = [&](const FText& Label, int32 Index) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeButton(FT66ButtonParams(Label, FOnClicked::CreateLambda([this, Index]() { CollectorTabIndex = Index; ReleaseSlateResources(true); TakeWidget(); return FReply::Handled(); }))
			.SetMinWidth(0.f).SetFontSize(11).SetPadding(FMargin(12.f, 6.f)));
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
		FItemData ItemData;
		if (GI && GI->GetItemData(ItemID, ItemData) && !ItemData.Icon.IsNull())
		{
			UT66UITexturePoolSubsystem* Pool = GetWorld() && GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
			if (Pool) T66SlateTexture::BindSharedBrushAsync(Pool, ItemData.Icon, this, IconBrush, ItemID, true);
		}
		Scroll->AddSlot().Padding(6.f)
			[
				FT66Style::MakePanel(
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
							FT66Style::MakeButton(AddBtn, FOnClicked::CreateLambda([this, CapturedID]() { OnAddItem(CapturedID); return FReply::Handled(); }), ET66ButtonType::Primary)
						]
					],
					FT66PanelParams(ET66PanelType::Panel).SetPadding(8.f)
				)
			];
	};

	auto AddSpawnCard = [&](const FText& NameText, const FText& DescText, TFunction<void()> OnSpawn)
	{
		Scroll->AddSlot().Padding(6.f)
			[
				FT66Style::MakePanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(NameText).Font(FT66Style::Tokens::FontBold(12)).ColorAndOpacity(FT66Style::Tokens::Text)]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)[SNew(STextBlock).Text(DescText).Font(FT66Style::Tokens::FontRegular(10)).ColorAndOpacity(FT66Style::Tokens::TextMuted).AutoWrapText(true)]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
						[
							FT66Style::MakeButton(SpawnBtn, FOnClicked::CreateLambda([OnSpawn]() { OnSpawn(); return FReply::Handled(); }), ET66ButtonType::Neutral)
						]
					],
					FT66PanelParams(ET66PanelType::Panel).SetPadding(8.f)
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
				if (!ItemData.EffectLine1.IsEmpty()) DescText = ItemData.EffectLine1;
				if (!ItemData.EffectLine2.IsEmpty()) DescText = FText::Format(LOCTEXT("DescLine", "{0}\n{1}"), DescText, ItemData.EffectLine2);
				if (!ItemData.EffectLine3.IsEmpty()) DescText = FText::Format(LOCTEXT("DescLine2", "{0}\n{1}"), DescText, ItemData.EffectLine3);
			}
			AddItemCard(ItemID, NameText, DescText);
		}
	}
	else if (CollectorTabIndex == 1)
	{
		AddSpawnCard(
			LOCTEXT("TreeOfLifeName", "Tree of Life"),
			LOCTEXT("TreeOfLifeDesc", "NPC: Tree of Life."),
			[this]() { OnSpawnNPC(FName(TEXT("TreeOfLife"))); }
		);
	}
	else if (CollectorTabIndex == 2)
	{
		static const TArray<FName> MobIDs = { FName(TEXT("RegularEnemy")), FName(TEXT("Leprechaun")), FName(TEXT("GoblinThief")), FName(TEXT("UniqueEnemy")) };
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
		AddSpawnCard(LOCTEXT("TreeOfLifeName", "Tree of Life"), FText::FromString(TEXT("Interactable")), [this]() { OnSpawnInteractable(FName(TEXT("TreeOfLife"))); });
		AddSpawnCard(LOCTEXT("CashTruck", "Cash Truck"), FText::FromString(TEXT("Interactable")), [this]() { OnSpawnInteractable(FName(TEXT("CashTruck"))); });
		AddSpawnCard(LOCTEXT("WheelSpin", "Wheel Spin"), FText::FromString(TEXT("Interactable")), [this]() { OnSpawnInteractable(FName(TEXT("WheelSpin"))); });
		AddSpawnCard(LOCTEXT("IdolAltar", "Idol Altar"), FText::FromString(TEXT("Interactable")), [this]() { OnSpawnInteractable(FName(TEXT("IdolAltar"))); });
	}

	TSharedRef<SVerticalBox> MainPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
		[SNew(STextBlock).Text(TitleCollector).Font(FT66Style::Tokens::FontBold(20)).ColorAndOpacity(FT66Style::Tokens::Text)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[TabRow]
		+ SVerticalBox::Slot().FillHeight(1.f)[SNew(SBox).HeightOverride(400.f)[Scroll]]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[FT66Style::MakeButton(CloseText, FOnClicked::CreateLambda([this]() { CloseOverlay(); return FReply::Handled(); }), ET66ButtonType::Neutral)]
			+ SHorizontalBox::Slot().AutoWidth()
			[FT66Style::MakeButton(ExitLab, FOnClicked::CreateLambda([this]() { OnExitLab(); return FReply::Handled(); }), ET66ButtonType::Danger)]
		];

	return SNew(SBox).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.85f))
			]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(40.f)
			[
				SNew(SBox).WidthOverride(520.f)
				[
					FT66Style::MakePanel(MainPanel, FT66PanelParams(ET66PanelType::Panel).SetPadding(24.f))
				]
			]
		];
}

#undef LOCTEXT_NAMESPACE
