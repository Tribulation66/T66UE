// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Widgets/Images/SImage.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/CoreStyle.h"

UT66RunSummaryScreen::UT66RunSummaryScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::RunSummary;
	bIsModal = true;
}

void UT66RunSummaryScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	// If we were opened from a leaderboard row click, load the saved snapshot first.
	LoadSavedRunSummaryIfRequested();

	// Default: banners are only relevant for a freshly-finished run (not for viewing saved leaderboard snapshots).
	bNewPersonalHighScore = false;
	bNewPersonalBestTime = false;

	// Foundation: submit score at run end if allowed by settings (Practice Mode blocks).
	// Important: do NOT submit when we're just viewing a saved run from the leaderboard.
	if (!bViewingSavedLeaderboardRunSummary)
	{
		UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
		UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
		if (RunState && LB)
		{
			// High Score submission: subsystem only updates if it's a new personal best (and Practice Mode blocks).
			LB->SubmitRunBounty(RunState->GetCurrentScore());
			bNewPersonalHighScore = LB->WasLastHighScoreNewPersonalBest();
		}

		// Speed Run banner: if any completed stage submission during this run set a new personal best time.
		if (RunState)
		{
			bNewPersonalBestTime = RunState->DidThisRunSetNewPersonalBestTime();
		}
	}

	EnsurePreviewCaptures();
}

void UT66RunSummaryScreen::OnScreenDeactivated_Implementation()
{
	DestroyPreviewCaptures();
	Super::OnScreenDeactivated_Implementation();
}

namespace
{
	template <typename TStage>
	static TStage* FindStage(UWorld* World)
	{
		if (!World) return nullptr;
		for (TActorIterator<TStage> It(World); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}
}

void UT66RunSummaryScreen::EnsurePreviewCaptures()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UT66GameInstance* GI = Cast<UT66GameInstance>(World->GetGameInstance());
	if (!GI) return;

	// Reuse the same preview stages used in the frontend screens (spawn them far away so players never see them).
	if (!HeroPreviewStage)
	{
		HeroPreviewStage = FindStage<AT66HeroPreviewStage>(World);
		if (!HeroPreviewStage)
		{
			FActorSpawnParameters P;
			P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			HeroPreviewStage = World->SpawnActor<AT66HeroPreviewStage>(AT66HeroPreviewStage::StaticClass(), FVector(1200000.f, 0.f, 200.f), FRotator::ZeroRotator, P);
		}
	}

	// Drive them from the saved selection so it matches the hero/companion selection UI.
	if (HeroPreviewStage)
	{
		const FName HeroID =
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroID :
			(!GI->SelectedHeroID.IsNone() ? GI->SelectedHeroID :
				(GI->GetAllHeroIDs().Num() > 0 ? GI->GetAllHeroIDs()[0] : NAME_None));

		const ET66BodyType BodyType =
			(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroBodyType : GI->SelectedHeroBodyType;
		if (!HeroID.IsNone())
		{
			HeroPreviewStage->SetPreviewHero(HeroID, BodyType);
		}
		if (UTextureRenderTarget2D* RT = HeroPreviewStage->GetRenderTarget())
		{
			if (!HeroPreviewBrush.IsValid())
			{
				HeroPreviewBrush = MakeShared<FSlateBrush>();
				HeroPreviewBrush->DrawAs = ESlateBrushDrawType::Image;
			}
			HeroPreviewBrush->SetResourceObject(RT);
			HeroPreviewBrush->ImageSize = FVector2D(640.f, 640.f);
		}
	}
}

void UT66RunSummaryScreen::DestroyPreviewCaptures()
{
	// Keep preview stages around; they are harmless and reused by other screens if present.
	HeroPreviewBrush.Reset();
}

void UT66RunSummaryScreen::LoadSavedRunSummaryIfRequested()
{
	bViewingSavedLeaderboardRunSummary = false;
	LoadedSavedSummary = nullptr;

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	if (!LB)
	{
		return;
	}

	FString SlotName;
	if (!LB->ConsumePendingRunSummaryRequest(SlotName))
	{
		return;
	}

	if (SlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	LoadedSavedSummary = Cast<UT66LeaderboardRunSummarySaveGame>(Loaded);
	if (LoadedSavedSummary)
	{
		bViewingSavedLeaderboardRunSummary = true;
	}
}

void UT66RunSummaryScreen::RebuildLogItems()
{
	LogItems.Reset();

	const TArray<FString>* LogPtr = nullptr;
	if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
	{
		LogPtr = &LoadedSavedSummary->EventLog;
	}
	else
	{
		UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		if (RunState)
		{
			LogPtr = &RunState->GetEventLog();
		}
	}

	const TArray<FString>& Log = LogPtr ? *LogPtr : TArray<FString>();

	LogItems.Reserve(Log.Num());
	for (const FString& Entry : Log)
	{
		LogItems.Add(MakeShared<FString>(Entry));
	}
}

TSharedRef<ITableRow> UT66RunSummaryScreen::GenerateLogRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FString Line = (Item.IsValid()) ? *Item : FString();

	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		.Padding(FMargin(4.f, 2.f))
		[
			SNew(STextBlock)
			.Text(FText::FromString(Line))
			.Font(FT66Style::Tokens::FontRegular(12))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		];
}

TSharedRef<SWidget> UT66RunSummaryScreen::BuildSlateUI()
{
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const int32 StageReached =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->StageReached :
		(RunState ? RunState->GetCurrentStage() : 1);

	const int32 HighScore =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->Bounty :
		(RunState ? RunState->GetCurrentScore() : 0);

	const int32 HeroLevel =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->HeroLevel :
		(RunState ? RunState->GetHeroLevel() : 1);

	const int32 DamageStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->DamageStat :
		(RunState ? RunState->GetDamageStat() : 1);

	const int32 AttackSpeedStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->AttackSpeedStat :
		(RunState ? RunState->GetAttackSpeedStat() : 1);

	const int32 AttackSizeStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->AttackSizeStat :
		(RunState ? RunState->GetScaleStat() : 1);

	const int32 ArmorStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->ArmorStat :
		(RunState ? RunState->GetArmorStat() : 1);

	const int32 EvasionStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->EvasionStat :
		(RunState ? RunState->GetEvasionStat() : 1);

	const int32 LuckStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->LuckStat :
		(RunState ? RunState->GetLuckStat() : 1);

	const int32 SpeedStat =
		(bViewingSavedLeaderboardRunSummary && LoadedSavedSummary) ? LoadedSavedSummary->SpeedStat :
		(RunState ? RunState->GetSpeedStat() : 1);
	UT66LocalizationSubsystem* Loc = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66GameInstance* GI = GetWorld() ? Cast<UT66GameInstance>(GetWorld()->GetGameInstance()) : nullptr;

	EnsurePreviewCaptures();
	InventoryItemIconBrushes.Reset();
	IdolIconBrushes.Reset();

	RebuildLogItems();
	TSharedRef<SListView<TSharedPtr<FString>>> LogList =
		SNew(SListView<TSharedPtr<FString>>)
		.ListItemsSource(&LogItems)
		.OnGenerateRow(SListView<TSharedPtr<FString>>::FOnGenerateRow::CreateUObject(this, &UT66RunSummaryScreen::GenerateLogRow))
		.SelectionMode(ESelectionMode::None);

	const FText TitleText = Loc ? Loc->GetText_RunSummaryTitle() : NSLOCTEXT("T66.RunSummary", "Title", "RUN SUMMARY");
	const FText StageHighScoreText = Loc
		? FText::Format(Loc->GetText_RunSummaryStageReachedBountyFormat(), FText::AsNumber(StageReached), FText::AsNumber(HighScore))
		: FText::FromString(FString::Printf(TEXT("Stage Reached: %d  |  High Score: %d"), StageReached, HighScore));

	auto MakePanel = [](const FText& Header, const TSharedRef<SWidget>& Body) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
			.Padding(FMargin(FT66Style::Tokens::Space4))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(Header)
					.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
				]
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					Body
				]
			];
	};

	auto MakeHeroPreview = [](const TSharedPtr<FSlateBrush>& Brush) -> TSharedRef<SWidget>
	{
		return Brush.IsValid()
			? StaticCastSharedRef<SWidget>(SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
				.Padding(0.f)
				[
					SNew(SBox)
					.WidthOverride(420.f)
					.HeightOverride(420.f)
					[
						SNew(SImage).Image(Brush.Get())
					]
				])
			: StaticCastSharedRef<SWidget>(SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
				.Padding(0.f)
				[
					SNew(SBox)
					.WidthOverride(420.f)
					.HeightOverride(420.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.RunSummary", "NoPreview", "No Preview"))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
						.Justification(ETextJustify::Center)
					]
				]);
	};

	// Event log (hidden by default; opened via "EVENT LOG" button).
	TSharedRef<SWidget> EventLogPanel =
		MakePanel(
			NSLOCTEXT("T66.RunSummary", "EventLogTitle", "EVENT LOG"),
			SNew(SBorder)
			.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
			.Padding(FMargin(FT66Style::Tokens::Space2))
			[
				LogList
			]);

	auto MakeEventLogButton = [&](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.MinDesiredWidth(160.f)
			.HeightOverride(44.f)
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleViewLogClicked))
				.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
				.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
				.ContentPadding(FMargin(14.f, 8.f))
				[
					SNew(STextBlock)
					.Text(Text)
					.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
				]
			];
	};

	// Level + stats (requested for Run Summary and saved leaderboard snapshots).
	TSharedRef<SVerticalBox> StatsBox = SNew(SVerticalBox);
	{
		const FText StatFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");

		auto AddStatLine = [&](const FText& Label, int32 Value)
		{
			StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(FText::Format(StatFmt, Label, FText::AsNumber(Value)))
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
		};

		AddStatLine(Loc ? Loc->GetText_Level() : NSLOCTEXT("T66.Common", "Level", "LEVEL"), HeroLevel);
		AddStatLine(Loc ? Loc->GetText_Stat_Damage() : NSLOCTEXT("T66.Stats", "Damage", "Damage"), DamageStat);
		AddStatLine(Loc ? Loc->GetText_Stat_AttackSpeed() : NSLOCTEXT("T66.Stats", "AttackSpeed", "Attack Speed"), AttackSpeedStat);
		AddStatLine(Loc ? Loc->GetText_Stat_AttackSize() : NSLOCTEXT("T66.Stats", "AttackSize", "Attack Size"), AttackSizeStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Armor() : NSLOCTEXT("T66.Stats", "Armor", "Armor"), ArmorStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Evasion() : NSLOCTEXT("T66.Stats", "Evasion", "Evasion"), EvasionStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Luck() : NSLOCTEXT("T66.Stats", "Luck", "Luck"), LuckStat);
		AddStatLine(Loc ? Loc->GetText_Stat_Speed() : NSLOCTEXT("T66.Stats", "Speed", "Speed"), SpeedStat);
	}

	// Build idols + inventory lists.
	TSharedRef<SVerticalBox> IdolsBox = SNew(SVerticalBox);
	TSharedRef<SVerticalBox> InvBox = SNew(SVerticalBox);
	{
		const TArray<FName>* IdolsPtr = nullptr;
		const TArray<FName>* InventoryPtr = nullptr;

		if (bViewingSavedLeaderboardRunSummary && LoadedSavedSummary)
		{
			IdolsPtr = &LoadedSavedSummary->EquippedIdols;
			InventoryPtr = &LoadedSavedSummary->Inventory;
		}
		else if (RunState)
		{
			IdolsPtr = &RunState->GetEquippedIdols();
			InventoryPtr = &RunState->GetInventory();
		}

		const TArray<FName> Empty;
		const TArray<FName>& Idols = IdolsPtr ? *IdolsPtr : Empty;
		const TArray<FName>& Inventory = InventoryPtr ? *InventoryPtr : Empty;

		for (int32 IdolSlotIndex = 0; IdolSlotIndex < UT66RunStateSubsystem::MaxEquippedIdolSlots; ++IdolSlotIndex)
		{
			const FName IdolID = (Idols.IsValidIndex(IdolSlotIndex)) ? Idols[IdolSlotIndex] : NAME_None;
			const FLinearColor IdolColor = !IdolID.IsNone() ? UT66RunStateSubsystem::GetIdolColor(IdolID) : FT66Style::Tokens::Stroke;
			FIdolData IdolData;
			const bool bHasIdolData = GI && !IdolID.IsNone() && GI->GetIdolData(IdolID, IdolData);
			UTexture2D* IdolTex = nullptr;
			if (bHasIdolData && !IdolData.Icon.IsNull())
			{
				IdolTex = IdolData.Icon.Get();
				if (!IdolTex)
				{
					IdolTex = IdolData.Icon.LoadSynchronous();
				}
			}
			TSharedPtr<FSlateBrush> IdolBrush;
			if (IdolTex)
			{
				IdolBrush = MakeShared<FSlateBrush>();
				IdolBrush->DrawAs = ESlateBrushDrawType::Image;
				IdolBrush->SetResourceObject(IdolTex);
				IdolBrush->ImageSize = FVector2D(18.f, 18.f);
				IdolIconBrushes.Add(IdolBrush);
			}

			IdolsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
				.Padding(FMargin(12.f, 10.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(SBox).WidthOverride(18.f).HeightOverride(18.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(IdolColor)
							.Padding(0.f)
							[
								IdolBrush.IsValid()
								? StaticCastSharedRef<SWidget>(SNew(SImage).Image(IdolBrush.Get()))
								: StaticCastSharedRef<SWidget>(SNew(SSpacer))
							]
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(10.f, 0.f)
					[
						SNew(STextBlock)
						.Text(IdolID.IsNone()
							? NSLOCTEXT("T66.Common", "Empty", "EMPTY")
							: FText::FromName(IdolID))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				]
			];
		}

		if (Inventory.Num() <= 0)
		{
			InvBox->AddSlot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.RunSummary", "NoItems", "No items."))
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
			];
		}
		else
		{
			for (const FName& ItemID : Inventory)
			{
				if (ItemID.IsNone()) continue;
				FItemData ItemData;
				const bool bHasItemData = GI && GI->GetItemData(ItemID, ItemData);
				UTexture2D* ItemTex = nullptr;
				if (bHasItemData && !ItemData.Icon.IsNull())
				{
					ItemTex = ItemData.Icon.Get();
					if (!ItemTex)
					{
						ItemTex = ItemData.Icon.LoadSynchronous();
					}
				}
				TSharedPtr<FSlateBrush> ItemBrush;
				if (ItemTex)
				{
					ItemBrush = MakeShared<FSlateBrush>();
					ItemBrush->DrawAs = ESlateBrushDrawType::Image;
					ItemBrush->SetResourceObject(ItemTex);
					ItemBrush->ImageSize = FVector2D(18.f, 18.f);
					InventoryItemIconBrushes.Add(ItemBrush);
				}

				InvBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(SBorder)
					.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel2"))
					.Padding(FMargin(12.f, 10.f))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SBox).WidthOverride(18.f).HeightOverride(18.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(bHasItemData ? ItemData.PlaceholderColor : FT66Style::Tokens::Stroke)
								.Padding(0.f)
								[
									ItemBrush.IsValid()
									? StaticCastSharedRef<SWidget>(SNew(SImage).Image(ItemBrush.Get()))
									: StaticCastSharedRef<SWidget>(SNew(SSpacer))
								]
							]
						]
						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(10.f, 0.f)
						[
							SNew(STextBlock)
							.Text(Loc ? Loc->GetText_ItemDisplayName(ItemID) : FText::FromName(ItemID))
							.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
					]
				];
			}
		}
	}

	// Buttons row (must be inside the main panel).
	TSharedRef<SWidget> ButtonsRow =
		bViewingSavedLeaderboardRunSummary
		? StaticCastSharedRef<SWidget>(
			SNew(SBox).MinDesiredWidth(200.f).HeightOverride(54.f)
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked))
				.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
				.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
				.ContentPadding(FMargin(18.f, 10.f))
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK"))
					.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
				]
			])
		: StaticCastSharedRef<SWidget>(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SNew(SBox).MinDesiredWidth(180.f).HeightOverride(54.f)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleRestartClicked))
					.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary"))
					.ButtonColorAndOpacity(FT66Style::Tokens::Success)
					.ContentPadding(FMargin(18.f, 10.f))
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_Restart() : NSLOCTEXT("T66.RunSummary", "Restart", "RESTART"))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
					]
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				SNew(SBox).MinDesiredWidth(200.f).HeightOverride(54.f)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66RunSummaryScreen::HandleMainMenuClicked))
					.ButtonStyle(&FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral"))
					.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
					.ContentPadding(FMargin(18.f, 10.f))
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_MainMenu() : NSLOCTEXT("T66.RunSummary", "MainMenu", "MAIN MENU"))
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button"))
					]
				]
			]);

	// Panels in the requested layout.
	TSharedRef<SWidget> BaseStatsPanel = MakePanel(
		Loc ? Loc->GetText_BaseStatsHeader() : NSLOCTEXT("T66.HeroSelection", "BaseStatsHeader", "Base Stats"),
		StatsBox
	);

	TSharedRef<SWidget> InventoryPanel = MakePanel(
		NSLOCTEXT("T66.RunSummary", "InventoryPanel", "INVENTORY"),
		SNew(SScrollBox) + SScrollBox::Slot()[InvBox]
	);

	TSharedRef<SWidget> IdolsPanel = MakePanel(
		NSLOCTEXT("T66.RunSummary", "IdolsPanel", "IDOLS"),
		SNew(SScrollBox) + SScrollBox::Slot()[IdolsBox]
	);

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::Tokens::Bg)
		[
			SNew(SOverlay)
			// Main full-screen panel
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
				.Padding(FMargin(FT66Style::Tokens::Space6))
				[
					SNew(SVerticalBox)
					// Header row: title left, Event Log button right
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(TitleText)
							.Font(FT66Style::Tokens::FontBold(40))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							MakeEventLogButton(NSLOCTEXT("T66.RunSummary", "EventLogTitle", "EVENT LOG"))
						]
					]
					// Stage reached / high score line
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f)
					[
						SNew(STextBlock)
						.Text(StageHighScoreText)
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
					// Personal best banners (only for newly-finished runs)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, -10.f, 0.f, 6.f)
					[
						SNew(STextBlock)
						.Visibility_Lambda([this]() { return (bNewPersonalHighScore && !bViewingSavedLeaderboardRunSummary) ? EVisibility::Visible : EVisibility::Collapsed; })
						.Text_Lambda([this, Loc]()
						{
							return Loc ? Loc->GetText_NewPersonalHighScore() : NSLOCTEXT("T66.RunSummary", "NewPersonalHighScore", "New Personal High Score");
						})
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Success)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f)
					[
						SNew(STextBlock)
						.Visibility_Lambda([this]() { return (bNewPersonalBestTime && !bViewingSavedLeaderboardRunSummary) ? EVisibility::Visible : EVisibility::Collapsed; })
						.Text_Lambda([this, Loc]()
						{
							return Loc ? Loc->GetText_NewPersonalBestTime() : NSLOCTEXT("T66.RunSummary", "NewPersonalBestTime", "New Personal Best Time");
						})
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Success)
					]
					// Main content: Hero preview top-left, stats to the right, items to the right of stats
					+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 0.f, 0.f, 18.f)
					[
						SNew(SHorizontalBox)
						// Left column: Hero preview + Idols below
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 18.f, 0.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								MakeHeroPreview(HeroPreviewBrush)
							]
							+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 18.f, 0.f, 0.f)
							[
								IdolsPanel
							]
						]
						// Middle: Base stats
						+ SHorizontalBox::Slot().FillWidth(0.42f).Padding(0.f, 0.f, 18.f, 0.f)
						[
							BaseStatsPanel
						]
						// Right: Inventory / items
						+ SHorizontalBox::Slot().FillWidth(0.58f)
						[
							InventoryPanel
						]
					]
					// Buttons row (inside panel)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
					[
						ButtonsRow
					]
				]
			]
			// Event log dropdown/popup (top-right)
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(0.f, 70.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(520.f)
				.HeightOverride(620.f)
				.Visibility(bLogVisible ? EVisibility::Visible : EVisibility::Collapsed)
				[
					EventLogPanel
				]
			]
		];
}

FReply UT66RunSummaryScreen::HandleRestartClicked() { OnRestartClicked(); return FReply::Handled(); }
FReply UT66RunSummaryScreen::HandleMainMenuClicked() { OnMainMenuClicked(); return FReply::Handled(); }
FReply UT66RunSummaryScreen::HandleViewLogClicked() { OnViewLogClicked(); return FReply::Handled(); }

void UT66RunSummaryScreen::OnRestartClicked()
{
	DestroyPreviewCaptures();
	if (bViewingSavedLeaderboardRunSummary)
	{
		// Viewer-mode (opened from leaderboard): just close the modal.
		if (UIManager)
		{
			UIManager->CloseModal();
			return;
		}
		UGameplayStatics::OpenLevel(this, FName(TEXT("FrontendLevel")));
		return;
	}
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState) RunState->ResetForNewRun();
	APlayerController* PC = GetOwningPlayer();
	if (PC) PC->SetPause(false);
	UGameplayStatics::OpenLevel(this, FName(TEXT("GameplayLevel")));
}

void UT66RunSummaryScreen::OnMainMenuClicked()
{
	DestroyPreviewCaptures();
	if (bViewingSavedLeaderboardRunSummary)
	{
		// Viewer-mode (opened from leaderboard): the main menu is already underneath.
		if (UIManager)
		{
			UIManager->CloseModal();
			return;
		}
		UGameplayStatics::OpenLevel(this, FName(TEXT("FrontendLevel")));
		return;
	}
	UT66RunStateSubsystem* RunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState) RunState->ResetForNewRun();
	APlayerController* PC = GetOwningPlayer();
	if (PC) PC->SetPause(false);
	// Use full map path for robustness in packaged builds.
	UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/FrontendLevel")));
}

void UT66RunSummaryScreen::OnViewLogClicked()
{
	bLogVisible = !bLogVisible;
	RefreshScreen();
}
