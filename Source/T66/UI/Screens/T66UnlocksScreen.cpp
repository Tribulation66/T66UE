// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66UnlocksScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	bool T66IsPausedGameplayWidget(const UUserWidget* Widget)
	{
		const APlayerController* PC = Widget ? Widget->GetOwningPlayer() : nullptr;
		return PC && PC->IsPaused();
	}

	FLinearColor T66FrontendShellFill()
	{
		return FLinearColor(0.004f, 0.005f, 0.010f, 0.985f);
	}

	FLinearColor T66UnlocksInsetFill()
	{
		return FLinearColor(0.024f, 0.025f, 0.030f, 1.0f);
	}

	FLinearColor T66UnlocksRowFill()
	{
		return FLinearColor(0.028f, 0.029f, 0.034f, 1.0f);
	}

	FLinearColor T66UnlocksUnlockedRowFill()
	{
		return FLinearColor(0.036f, 0.048f, 0.041f, 1.0f);
	}

	int32 T66ExtractNumericSuffix(FName ID)
	{
		const FString AsString = ID.ToString();
		int32 UnderscoreIndex = INDEX_NONE;
		if (!AsString.FindLastChar(TEXT('_'), UnderscoreIndex))
		{
			return MAX_int32;
		}

		const FString Suffix = AsString.Mid(UnderscoreIndex + 1);
		return FCString::Atoi(*Suffix);
	}

	int32 T66CompanionUnlockStageFromID(FName CompanionID)
	{
		const int32 TargetIndex = T66ExtractNumericSuffix(CompanionID);
		if (TargetIndex <= 0)
		{
			return INDEX_NONE;
		}

		if (TargetIndex <= 23)
		{
			return TargetIndex;
		}

		if (TargetIndex == 24)
		{
			return 23;
		}

		return INDEX_NONE;
	}
}

UT66UnlocksScreen::UT66UnlocksScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Unlocks;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66UnlocksScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

UT66AchievementsSubsystem* UT66UnlocksScreen::GetAchievementsSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66AchievementsSubsystem>();
	}
	return nullptr;
}

UT66CompanionUnlockSubsystem* UT66UnlocksScreen::GetCompanionUnlockSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66CompanionUnlockSubsystem>();
	}
	return nullptr;
}

UT66GameInstance* UT66UnlocksScreen::GetT66GameInstance() const
{
	return Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
}

TArray<UT66UnlocksScreen::FUnlockEntry> UT66UnlocksScreen::BuildEntriesForCategory(EUnlockCategory Category) const
{
	TArray<FUnlockEntry> Entries;

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem();
	UT66CompanionUnlockSubsystem* CompanionUnlocks = GetCompanionUnlockSubsystem();
	UT66GameInstance* GameInstance = GetT66GameInstance();
	if (!GameInstance)
	{
		return Entries;
	}

	if (Category == EUnlockCategory::Heroes)
	{
		TArray<FName> HeroIDs = GameInstance->GetAllHeroIDs();
		HeroIDs.Sort([](const FName& Left, const FName& Right)
		{
			return T66ExtractNumericSuffix(Left) < T66ExtractNumericSuffix(Right);
		});

		for (const FName HeroID : HeroIDs)
		{
			FHeroData HeroData;
			if (!GameInstance->GetHeroData(HeroID, HeroData))
			{
				continue;
			}

			FUnlockEntry& Entry = Entries.AddDefaulted_GetRef();
			Entry.Name = Loc ? Loc->GetText_HeroName(HeroID) : (!HeroData.DisplayName.IsEmpty() ? HeroData.DisplayName : FText::FromName(HeroID));
			Entry.TypeText = NSLOCTEXT("T66.Unlocks", "HeroType", "HERO");
			Entry.DetailText = HeroData.bUnlockedByDefault
				? NSLOCTEXT("T66.Unlocks", "HeroUnlockedDetail", "Available from the start.")
				: NSLOCTEXT("T66.Unlocks", "HeroLockedDetail", "Unlock condition is not exposed by the current hero data.");
			Entry.bUnlocked = HeroData.bUnlockedByDefault;
			Entry.Order = T66ExtractNumericSuffix(HeroID);
		}
	}
	else if (Category == EUnlockCategory::Companions)
	{
		TArray<FName> CompanionIDs = GameInstance->GetAllCompanionIDs();
		CompanionIDs.Sort([](const FName& Left, const FName& Right)
		{
			return T66ExtractNumericSuffix(Left) < T66ExtractNumericSuffix(Right);
		});

		for (const FName CompanionID : CompanionIDs)
		{
			FCompanionData CompanionData;
			if (!GameInstance->GetCompanionData(CompanionID, CompanionData))
			{
				continue;
			}

			const int32 UnlockStage = T66CompanionUnlockStageFromID(CompanionID);
			const bool bUnlocked = CompanionUnlocks
				? CompanionUnlocks->IsCompanionUnlocked(CompanionID)
				: CompanionData.bUnlockedByDefault;

			FUnlockEntry& Entry = Entries.AddDefaulted_GetRef();
			Entry.Name = Loc ? Loc->GetText_CompanionName(CompanionID) : (!CompanionData.DisplayName.IsEmpty() ? CompanionData.DisplayName : FText::FromName(CompanionID));
			Entry.TypeText = NSLOCTEXT("T66.Unlocks", "CompanionType", "COMPANION");
			Entry.DetailText = (UnlockStage != INDEX_NONE)
				? FText::Format(NSLOCTEXT("T66.Unlocks", "CompanionStageCondition", "Clear Stage {0} for the first time."), FText::AsNumber(UnlockStage))
				: NSLOCTEXT("T66.Unlocks", "CompanionFallbackCondition", "Unlock condition is not exposed by the current companion data.");
			Entry.bUnlocked = bUnlocked;
			Entry.Order = T66ExtractNumericSuffix(CompanionID);
		}
	}
	else
	{
		TArray<FName> ItemIDs;
		if (UDataTable* ItemsTable = GameInstance->GetItemsDataTable())
		{
			ItemIDs = ItemsTable->GetRowNames();
		}

		ItemIDs.AddUnique(FName(TEXT("Item_GamblersToken")));
		ItemIDs.Sort([](const FName& Left, const FName& Right)
		{
			return Left.LexicalLess(Right);
		});

		for (const FName ItemID : ItemIDs)
		{
			FItemData ItemData;
			if (!GameInstance->GetItemData(ItemID, ItemData))
			{
				continue;
			}

			FUnlockEntry& Entry = Entries.AddDefaulted_GetRef();
			Entry.Name = Loc ? Loc->GetText_ItemDisplayName(ItemID) : FText::FromName(ItemID);
			Entry.TypeText = NSLOCTEXT("T66.Unlocks", "ItemType", "ITEM");
			Entry.DetailText = NSLOCTEXT("T66.Unlocks", "ItemCondition", "Obtain this item in a run for the first time.");
			Entry.bUnlocked = Achievements ? Achievements->IsLabUnlockedItem(ItemID) : false;
			Entry.Order = Entries.Num();
		}
	}

	Entries.Sort([](const FUnlockEntry& Left, const FUnlockEntry& Right)
	{
		if (Left.bUnlocked != Right.bUnlocked)
		{
			return Left.bUnlocked > Right.bUnlocked;
		}
		if (Left.Order != Right.Order)
		{
			return Left.Order < Right.Order;
		}
		return Left.Name.ToString() < Right.Name.ToString();
	});

	return Entries;
}

int32 UT66UnlocksScreen::GetTotalUnlockCount() const
{
	return BuildEntriesForCategory(EUnlockCategory::Heroes).Num()
		+ BuildEntriesForCategory(EUnlockCategory::Companions).Num()
		+ BuildEntriesForCategory(EUnlockCategory::Items).Num();
}

int32 UT66UnlocksScreen::GetUnlockedCount() const
{
	int32 UnlockedCount = 0;
	const TArray<EUnlockCategory> Categories = {
		EUnlockCategory::Heroes,
		EUnlockCategory::Companions,
		EUnlockCategory::Items
	};

	for (const EUnlockCategory Category : Categories)
	{
		for (const FUnlockEntry& Entry : BuildEntriesForCategory(Category))
		{
			if (Entry.bUnlocked)
			{
				++UnlockedCount;
			}
		}
	}

	return UnlockedCount;
}

TSharedRef<SWidget> UT66UnlocksScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	const FText ActiveSliceTitle = NSLOCTEXT("T66.MiniGames", "SliceActiveTitle", "CHADPOCALYPSE MINI");
	const FText ActiveSliceBody = NSLOCTEXT("T66.MiniGames", "SliceActiveBody", "Launch the current 2D survivor mini-game shell with its own saves, heroes, idols, enemies, and progression.");
	const FText ActiveSliceTag = NSLOCTEXT("T66.MiniGames", "SliceActiveTag", "AVAILABLE NOW");
	const FText ComingSoonTitle = NSLOCTEXT("T66.MiniGames", "SliceComingSoonTitle", "COMING SOON");
	const FText ComingSoonBody = NSLOCTEXT("T66.MiniGames", "SliceComingSoonBody", "Reserved slot for future mini-game releases.");
	const FText ComingSoonTag = NSLOCTEXT("T66.MiniGames", "SliceComingSoonTag", "IN DEVELOPMENT");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const float ResponsiveScale = FMath::Max(FT66Style::GetViewportResponsiveScale(), KINDA_SMALL_NUMBER);
	const float TopBarOverlapPx = 22.f;
	const float TopInset = UIManager
		? FMath::Max(0.f, (UIManager->GetFrontendTopBarContentHeight() - TopBarOverlapPx) / ResponsiveScale)
		: 0.f;
	const bool bShowBackButton = !(UIManager && UIManager->IsFrontendTopBarVisible());

	const TAttribute<FMargin> SafeContentInsets = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66Style::GetSafeFrameInsets();
	});

	const TAttribute<FMargin> SafeBackPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66Style::GetSafePadding(FMargin(20.f, 0.f, 0.f, 20.f));
	});

	const auto MakeSlicePanel = [&](const FText& Title, const FText& Body, const FText& Tag, const FLinearColor& Accent, const bool bClickable) -> TSharedRef<SWidget>
	{
		TSharedRef<SWidget> SliceContent =
			SNew(SBox)
			.HeightOverride(132.f)
			[
				FT66Style::MakePanel(
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.01f, 0.015f, 0.022f, 0.90f))
						.Padding(FMargin(26.f, 18.f))
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							.VAlign(VAlign_Center)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
									.Text(Title)
									.Font(FT66Style::Tokens::FontBold(30))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.f, 8.f, 40.f, 0.f)
								[
									SNew(STextBlock)
									.Text(Body)
									.Font(FT66Style::Tokens::FontRegular(18))
									.ColorAndOpacity(FT66Style::Tokens::TextMuted)
									.AutoWrapText(true)
								]
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Right)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(bClickable ? Accent : FLinearColor(0.18f, 0.20f, 0.24f, 1.f))
								.Padding(FMargin(16.f, 8.f))
								[
									SNew(STextBlock)
									.Text(Tag)
									.Font(FT66Style::Tokens::FontBold(14))
									.ColorAndOpacity(bClickable ? FLinearColor(0.05f, 0.06f, 0.07f, 1.f) : FT66Style::Tokens::Text)
								]
							]
						]
					],
					FT66PanelParams(ET66PanelType::Panel)
						.SetColor(FLinearColor(0.018f, 0.020f, 0.026f, 1.0f))
						.SetPadding(FMargin(3.f)))
			];

		if (!bClickable)
		{
			return SliceContent;
		}

		return SNew(SButton)
			.ButtonStyle(FCoreStyle::Get(), "NoBorder")
			.OnClicked(FOnClicked::CreateUObject(this, &UT66UnlocksScreen::HandleOpenMiniChadpocalypseClicked))
			[
				SliceContent
			];
	};

	return SNew(SBox)
		.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
		[
			FT66Style::MakePanel(
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
					.Padding(SafeContentInsets)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(0.f, 18.f, 0.f, 18.f)
						[
							SNew(SBox)
							.WidthOverride(1120.f)
							[
								FT66Style::MakePanel(
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.f, 0.f, 0.f, 18.f)
									[
										MakeSlicePanel(ActiveSliceTitle, ActiveSliceBody, ActiveSliceTag, FT66Style::Success(), true)
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.f, 0.f, 0.f, 18.f)
									[
										MakeSlicePanel(ComingSoonTitle, ComingSoonBody, ComingSoonTag, FT66Style::Accent2(), false)
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.f, 0.f, 0.f, 18.f)
									[
										MakeSlicePanel(ComingSoonTitle, ComingSoonBody, ComingSoonTag, FT66Style::Accent2(), false)
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										MakeSlicePanel(ComingSoonTitle, ComingSoonBody, ComingSoonTag, FT66Style::Accent2(), false)
									],
									FT66PanelParams(ET66PanelType::Panel)
										.SetColor(T66UnlocksInsetFill())
										.SetPadding(FMargin(26.f)))
							]
						]
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Bottom)
				.Padding(SafeBackPadding)
				[
					SNew(SBox)
					.Visibility(bShowBackButton ? EVisibility::Visible : EVisibility::Collapsed)
					[
						FT66Style::MakeButton(
							FT66ButtonParams(
								BackText,
								FOnClicked::CreateUObject(this, &UT66UnlocksScreen::HandleBackClicked),
								ET66ButtonType::Neutral)
							.SetMinWidth(120.f)
							.SetFontSize(20))
					]
				],
				FT66PanelParams(ET66PanelType::Panel).SetColor(T66FrontendShellFill()))
		];
}

void UT66UnlocksScreen::RebuildUnlockList()
{
	if (!UnlockListBox.IsValid())
	{
		return;
	}

	UnlockListBox->ClearChildren();

	const TArray<FUnlockEntry> Entries = BuildEntriesForCategory(CurrentCategory);
	for (const FUnlockEntry& Entry : Entries)
	{
		UnlockListBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(Entry.bUnlocked ? T66UnlocksUnlockedRowFill() : T66UnlocksRowFill())
			.Padding(FMargin(18.f, 14.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Top)
				.Padding(0.f, 0.f, 14.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Entry.TypeText)
					.Font(FT66Style::Tokens::FontBold(20))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(Entry.Name)
						.Font(FT66Style::Tokens::FontBold(24))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 4.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(Entry.DetailText)
						.Font(FT66Style::Tokens::FontRegular(20))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				.Padding(16.f, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Entry.bUnlocked
						? NSLOCTEXT("T66.Unlocks", "UnlockedStatus", "UNLOCKED")
						: NSLOCTEXT("T66.Unlocks", "LockedStatus", "LOCKED"))
					.Font(FT66Style::Tokens::FontBold(20))
					.ColorAndOpacity(Entry.bUnlocked ? FT66Style::Tokens::Success : FT66Style::Tokens::TextMuted)
				]
			]
		];
	}
}

void UT66UnlocksScreen::SwitchToCategory(EUnlockCategory Category)
{
	CurrentCategory = Category;
	RebuildUnlockList();
}

void UT66UnlocksScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	CurrentCategory = EUnlockCategory::Heroes;
	RebuildUnlockList();

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66UnlocksScreen::HandleLanguageChanged);
	}

	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		Achievements->AchievementsStateChanged.AddUniqueDynamic(this, &UT66UnlocksScreen::HandleAchievementsStateChanged);
	}
}

void UT66UnlocksScreen::OnScreenDeactivated_Implementation()
{
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66UnlocksScreen::HandleLanguageChanged);
	}

	if (UT66AchievementsSubsystem* Achievements = GetAchievementsSubsystem())
	{
		Achievements->AchievementsStateChanged.RemoveDynamic(this, &UT66UnlocksScreen::HandleAchievementsStateChanged);
	}

	Super::OnScreenDeactivated_Implementation();
}

void UT66UnlocksScreen::OnBackClicked()
{
	if (T66IsPausedGameplayWidget(this) && UIManager)
	{
		ShowModal(ET66ScreenType::PauseMenu);
		return;
	}

	NavigateBack();
}

FReply UT66UnlocksScreen::HandleCategoryClicked(EUnlockCategory Category)
{
	SwitchToCategory(Category);
	return FReply::Handled();
}

FReply UT66UnlocksScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66UnlocksScreen::HandleOpenMiniChadpocalypseClicked()
{
	NavigateTo(ET66ScreenType::MiniMainMenu);
	return FReply::Handled();
}

void UT66UnlocksScreen::HandleLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}

void UT66UnlocksScreen::HandleAchievementsStateChanged()
{
	FT66Style::DeferRebuild(this);
}
