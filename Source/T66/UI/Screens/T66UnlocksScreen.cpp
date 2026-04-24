// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66UnlocksScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Engine/Texture2D.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "UObject/StrongObjectPtr.h"
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
	const TCHAR* T66SettingsAssetRoot = TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/");

	TMap<FString, TStrongObjectPtr<UTexture2D>> GUnlocksGeneratedTextureCache;
	TMap<FString, TSharedPtr<FSlateBrush>> GUnlocksGeneratedBrushCache;
	TMap<FString, TSharedPtr<FButtonStyle>> GUnlocksGeneratedButtonStyleCache;

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

	bool T66TryResolveCompanionUnlockPresentation(FName CompanionID, ET66Difficulty& OutDifficulty, int32& OutLocalStage)
	{
		const int32 TargetIndex = T66ExtractNumericSuffix(CompanionID);
		if (TargetIndex <= 0)
		{
			return false;
		}

		if (TargetIndex <= 5)
		{
			OutDifficulty = ET66Difficulty::Easy;
			OutLocalStage = FMath::Min(TargetIndex, 4);
			return true;
		}

		if (TargetIndex <= 10)
		{
			OutDifficulty = ET66Difficulty::Medium;
			OutLocalStage = FMath::Min(TargetIndex - 5, 4);
			return true;
		}

		if (TargetIndex <= 15)
		{
			OutDifficulty = ET66Difficulty::Hard;
			OutLocalStage = FMath::Min(TargetIndex - 10, 4);
			return true;
		}

		if (TargetIndex <= 20)
		{
			OutDifficulty = ET66Difficulty::VeryHard;
			OutLocalStage = FMath::Min(TargetIndex - 15, 4);
			return true;
		}

		if (TargetIndex <= 24)
		{
			OutDifficulty = ET66Difficulty::Impossible;
			OutLocalStage = FMath::Min(TargetIndex - 20, 3);
			return true;
		}

		return false;
	}

	FText T66GetUnlockDifficultyLabel(UT66LocalizationSubsystem* Loc, const ET66Difficulty Difficulty)
	{
		if (Loc)
		{
			return Loc->GetText_Difficulty(Difficulty);
		}

		switch (Difficulty)
		{
		case ET66Difficulty::Easy:
			return NSLOCTEXT("T66.Unlocks", "CompanionUnlockDifficultyEasy", "Easy");
		case ET66Difficulty::Medium:
			return NSLOCTEXT("T66.Unlocks", "CompanionUnlockDifficultyMedium", "Medium");
		case ET66Difficulty::Hard:
			return NSLOCTEXT("T66.Unlocks", "CompanionUnlockDifficultyHard", "Hard");
		case ET66Difficulty::VeryHard:
			return NSLOCTEXT("T66.Unlocks", "CompanionUnlockDifficultyVeryHard", "Very Hard");
		case ET66Difficulty::Impossible:
		default:
			return NSLOCTEXT("T66.Unlocks", "CompanionUnlockDifficultyImpossible", "Impossible");
		}
	}

	FString MakeSettingsAssetPath(const TCHAR* FileName)
	{
		return FString(T66SettingsAssetRoot) / FileName;
	}

	UTexture2D* LoadUnlocksGeneratedTexture(const FString& SourceRelativePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GUnlocksGeneratedTextureCache.Find(SourceRelativePath))
		{
			return CachedTexture->Get();
		}

		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
				CandidatePath,
				TextureFilter::TF_Trilinear,
				true,
				TEXT("UnlocksGeneratedUI"));
			if (!Texture)
			{
				Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					TEXT("UnlocksGeneratedUI"));
			}

			if (Texture)
			{
				GUnlocksGeneratedTextureCache.Add(SourceRelativePath, TStrongObjectPtr<UTexture2D>(Texture));
				return Texture;
			}
		}

		return nullptr;
	}

	const FSlateBrush* ResolveUnlocksGeneratedBrush(const FString& SourceRelativePath, const FVector2D& ImageSize = FVector2D::ZeroVector)
	{
		const FString BrushKey = FString::Printf(TEXT("%s::%.0fx%.0f"), *SourceRelativePath, ImageSize.X, ImageSize.Y);
		if (const TSharedPtr<FSlateBrush>* CachedBrush = GUnlocksGeneratedBrushCache.Find(BrushKey))
		{
			return CachedBrush->Get();
		}

		UTexture2D* Texture = LoadUnlocksGeneratedTexture(SourceRelativePath);
		if (!Texture)
		{
			return nullptr;
		}

		const FVector2D ResolvedSize = ImageSize.X > 0.f && ImageSize.Y > 0.f
			? ImageSize
			: FVector2D(static_cast<float>(Texture->GetSizeX()), static_cast<float>(Texture->GetSizeY()));

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ResolvedSize;
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(Texture);

		GUnlocksGeneratedBrushCache.Add(BrushKey, Brush);
		return Brush.Get();
	}

	const FButtonStyle* ResolveUnlocksGeneratedButtonStyle(
		const FString& Key,
		const FString& NormalPath,
		const FString& HoverPath,
		const FString& PressedPath,
		const FString& DisabledPath)
	{
		if (const TSharedPtr<FButtonStyle>* CachedStyle = GUnlocksGeneratedButtonStyleCache.Find(Key))
		{
			return CachedStyle->Get();
		}

		const FButtonStyle& NoBorderStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		TSharedPtr<FButtonStyle> Style = MakeShared<FButtonStyle>(NoBorderStyle);
		if (const FSlateBrush* NormalBrush = ResolveUnlocksGeneratedBrush(NormalPath))
		{
			Style->SetNormal(*NormalBrush);
		}
		if (const FSlateBrush* HoverBrush = ResolveUnlocksGeneratedBrush(HoverPath))
		{
			Style->SetHovered(*HoverBrush);
		}
		if (const FSlateBrush* PressedBrush = ResolveUnlocksGeneratedBrush(PressedPath))
		{
			Style->SetPressed(*PressedBrush);
		}
		if (const FSlateBrush* DisabledBrush = ResolveUnlocksGeneratedBrush(DisabledPath))
		{
			Style->SetDisabled(*DisabledBrush);
		}
		Style->SetNormalPadding(FMargin(0.f));
		Style->SetPressedPadding(FMargin(0.f));

		GUnlocksGeneratedButtonStyleCache.Add(Key, Style);
		return Style.Get();
	}

	const FButtonStyle* ResolveUnlocksCompactButtonStyle()
	{
		return ResolveUnlocksGeneratedButtonStyle(
			TEXT("Unlocks.CompactButton"),
			MakeSettingsAssetPath(TEXT("settings_compact_neutral_normal.png")),
			MakeSettingsAssetPath(TEXT("settings_compact_neutral_hover.png")),
			MakeSettingsAssetPath(TEXT("settings_compact_neutral_pressed.png")),
			MakeSettingsAssetPath(TEXT("settings_toggle_inactive_normal.png")));
	}

	TSharedRef<SWidget> MakeUnlocksGeneratedPanel(
		const FString& SourceRelativePath,
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const FLinearColor& Tint = FLinearColor::White,
		const FLinearColor& FallbackFill = T66UnlocksInsetFill())
	{
		if (const FSlateBrush* Brush = ResolveUnlocksGeneratedBrush(SourceRelativePath))
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(Tint)
				.Padding(Padding)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FallbackFill)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeUnlocksGeneratedButton(
		const FT66ButtonParams& Params,
		const FButtonStyle* ButtonStyle,
		const FSlateFontInfo& Font,
		const FLinearColor& TextColor,
		const FMargin& ContentPadding)
	{
		TSharedRef<SButton> Button = SNew(SButton)
			.ButtonStyle(ButtonStyle ? ButtonStyle : &FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.ContentPadding(ContentPadding)
			.IsEnabled(Params.IsEnabled)
			.OnClicked(FT66Style::DebounceClick(Params.OnClicked))
			[
				SNew(STextBlock)
				.Text(Params.Label)
				.Font(Font)
				.ColorAndOpacity(TextColor)
				.Justification(ETextJustify::Center)
			];

		TSharedRef<SBox> Box = SNew(SBox)
			.MinDesiredWidth(Params.MinWidth)
			.Visibility(Params.Visibility)
			[
				Button
			];
		if (Params.Height > 0.f)
		{
			Box->SetHeightOverride(Params.Height);
		}
		return Box;
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

			ET66Difficulty UnlockDifficulty = ET66Difficulty::Easy;
			int32 UnlockStage = INDEX_NONE;
			const bool bHasUnlockPresentation = T66TryResolveCompanionUnlockPresentation(CompanionID, UnlockDifficulty, UnlockStage);
			const bool bUnlocked = CompanionUnlocks
				? CompanionUnlocks->IsCompanionUnlocked(CompanionID)
				: CompanionData.bUnlockedByDefault;

			FUnlockEntry& Entry = Entries.AddDefaulted_GetRef();
			Entry.Name = Loc ? Loc->GetText_CompanionName(CompanionID) : (!CompanionData.DisplayName.IsEmpty() ? CompanionData.DisplayName : FText::FromName(CompanionID));
			Entry.TypeText = NSLOCTEXT("T66.Unlocks", "CompanionType", "COMPANION");
			Entry.DetailText = bHasUnlockPresentation
				? FText::Format(
					NSLOCTEXT("T66.Unlocks", "CompanionStageCondition", "Clear {0} Stage {1} for the first time."),
					T66GetUnlockDifficultyLabel(Loc, UnlockDifficulty),
					FText::AsNumber(UnlockStage))
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
	const FText ActiveSliceTitle = NSLOCTEXT("T66.MiniGames", "SliceActiveTitle", "CHADPOCALYPSE MINI");
	const FText ActiveSliceBody = NSLOCTEXT("T66.MiniGames", "SliceActiveBody", "Launch the current 2D survivor mini-game shell with its own saves, heroes, idols, enemies, and progression.");
	const FText ActiveSliceTag = NSLOCTEXT("T66.MiniGames", "SliceActiveTag", "AVAILABLE NOW");
	const FText TDTitle = NSLOCTEXT("T66.MiniGames", "SliceTDTitle", "CHADPOCALYPSE TD");
	const FText TDBody = NSLOCTEXT("T66.MiniGames", "SliceTDBody", "Launch the tower defense mini-game with hero placement, enemy waves, upgrades, and a 20-map regular-mode rotation across all five tower themes.");
	const FText TDTag = NSLOCTEXT("T66.MiniGames", "SliceTDTag", "AVAILABLE NOW");
	const FText ComingSoonTitle = NSLOCTEXT("T66.MiniGames", "SliceComingSoonTitle", "COMING SOON");
	const FText ComingSoonBody = NSLOCTEXT("T66.MiniGames", "SliceComingSoonBody", "Reserved slot for future mini-game releases.");
	const FText ComingSoonTag = NSLOCTEXT("T66.MiniGames", "SliceComingSoonTag", "IN DEVELOPMENT");
	const float ResponsiveScale = FMath::Max(FT66Style::GetViewportResponsiveScale(), KINDA_SMALL_NUMBER);
	const float TopBarOverlapPx = 22.f;
	const float TopInset = UIManager
		? FMath::Max(0.f, (UIManager->GetFrontendTopBarContentHeight() - TopBarOverlapPx) / ResponsiveScale)
		: 0.f;

	const TAttribute<FMargin> SafeContentInsets = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66Style::GetSafeFrameInsets();
	});

	const auto MakeSlicePanel = [&](const FText& Title, const FText& Body, const FText& Tag, const FLinearColor& Accent, const bool bClickable, FOnClicked ClickDelegate = FOnClicked()) -> TSharedRef<SWidget>
	{
		TSharedRef<SWidget> SliceContent =
			SNew(SBox)
			.HeightOverride(132.f)
			[
				MakeUnlocksGeneratedPanel(
					MakeSettingsAssetPath(TEXT("settings_row_shell_split.png")),
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
						MakeUnlocksGeneratedPanel(
							MakeSettingsAssetPath(bClickable ? TEXT("settings_toggle_on_normal.png") : TEXT("settings_toggle_inactive_normal.png")),
							SNew(STextBlock)
							.Text(Tag)
							.Font(FT66Style::Tokens::FontBold(14))
							.ColorAndOpacity(bClickable ? FLinearColor(0.99f, 0.93f, 0.74f, 1.f) : FT66Style::Tokens::TextMuted),
							FMargin(18.f, 8.f),
							FLinearColor::White,
							bClickable ? Accent : FLinearColor(0.18f, 0.20f, 0.24f, 1.f))
					],
					FMargin(28.f, 20.f),
					bClickable ? FLinearColor::White : FLinearColor(0.72f, 0.76f, 0.82f, 1.f),
					FLinearColor(0.018f, 0.020f, 0.026f, 1.0f))
			];

		if (!bClickable)
		{
			return SliceContent;
		}

		return SNew(SButton)
			.ButtonStyle(FCoreStyle::Get(), "NoBorder")
			.OnClicked(ClickDelegate)
			[
				SliceContent
			];
	};

	return SNew(SBox)
		.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
		[
			MakeUnlocksGeneratedPanel(
				MakeSettingsAssetPath(TEXT("settings_content_shell_frame.png")),
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
								MakeUnlocksGeneratedPanel(
									MakeSettingsAssetPath(TEXT("settings_content_shell_frame.png")),
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.f, 0.f, 0.f, 18.f)
									[
										MakeSlicePanel(ActiveSliceTitle, ActiveSliceBody, ActiveSliceTag, FT66Style::Success(), true, FOnClicked::CreateUObject(this, &UT66UnlocksScreen::HandleOpenMiniChadpocalypseClicked))
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.f, 0.f, 0.f, 18.f)
									[
										MakeSlicePanel(TDTitle, TDBody, TDTag, FLinearColor(0.88f, 0.34f, 0.22f, 1.0f), true, FOnClicked::CreateUObject(this, &UT66UnlocksScreen::HandleOpenChadpocalypseTDClicked))
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
									FMargin(30.f),
									FLinearColor::White,
									T66UnlocksInsetFill())
							]
						]
					]
				],
				FMargin(18.f),
				FLinearColor::White,
				T66FrontendShellFill())
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
			MakeUnlocksGeneratedPanel(
				MakeSettingsAssetPath(TEXT("settings_row_shell_full.png")),
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
					MakeUnlocksGeneratedPanel(
						MakeSettingsAssetPath(Entry.bUnlocked ? TEXT("settings_toggle_on_normal.png") : TEXT("settings_toggle_inactive_normal.png")),
						SNew(STextBlock)
						.Text(Entry.bUnlocked
							? NSLOCTEXT("T66.Unlocks", "UnlockedStatus", "UNLOCKED")
							: NSLOCTEXT("T66.Unlocks", "LockedStatus", "LOCKED"))
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(Entry.bUnlocked ? FLinearColor(0.05f, 0.07f, 0.04f, 1.f) : FT66Style::Tokens::TextMuted)
						.Justification(ETextJustify::Center),
						FMargin(18.f, 8.f),
						FLinearColor::White,
						Entry.bUnlocked ? T66UnlocksUnlockedRowFill() : T66UnlocksRowFill())
				]
				,
				FMargin(22.f, 16.f),
				Entry.bUnlocked ? FLinearColor(0.92f, 1.0f, 0.88f, 1.0f) : FLinearColor::White,
				Entry.bUnlocked ? T66UnlocksUnlockedRowFill() : T66UnlocksRowFill())
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

FReply UT66UnlocksScreen::HandleOpenChadpocalypseTDClicked()
{
	NavigateTo(ET66ScreenType::TDMainMenu);
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
