// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SaveSlotsScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Dota/T66DotaSlate.h"
#include "UI/Style/T66Style.h"
#include "UI/T66SlateTextureHelpers.h"
#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Styling/SlateBrush.h"
#include "Styling/CoreStyle.h"

UT66SaveSlotsScreen::UT66SaveSlotsScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::SaveSlots;
	bIsModal = false;
}

TSharedRef<SWidget> UT66SaveSlotsScreen::BuildSlateUI()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	FText TitleText = Loc ? Loc->GetText_SaveSlots() : NSLOCTEXT("T66.SaveSlots", "Title", "SAVE SLOTS");
	FText EmptyText = Loc ? Loc->GetText_EmptySlot() : NSLOCTEXT("T66.SaveSlots", "EmptySlot", "EMPTY SLOT");
	FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");
	FText LoadText = Loc ? Loc->GetText_LoadGame() : NSLOCTEXT("T66.MainMenu", "LoadGame", "LOAD GAME");
	const FText PrevText = NSLOCTEXT("T66.SaveSlots", "PrevPage", "PREV");
	const FText NextText = NSLOCTEXT("T66.SaveSlots", "NextPage", "NEXT");

	// Main Menu–style background
	if (TexPool)
	{
		static const TArray<FSoftObjectPath> MainMenuBgPaths = {
			FSoftObjectPath(TEXT("/Game/UI/MainMenu/MMRed.MMRed"))
		};
		TexPool->EnsureTexturesLoadedSync(MainMenuBgPaths);
	}
	if (!SaveSlotsBackgroundBrush.IsValid())
	{
		SaveSlotsBackgroundBrush = MakeShared<FSlateBrush>();
		SaveSlotsBackgroundBrush->DrawAs = ESlateBrushDrawType::Box;
		SaveSlotsBackgroundBrush->Tiling = ESlateBrushTileType::NoTile;
		SaveSlotsBackgroundBrush->SetResourceObject(nullptr);
	}
	if (TexPool)
	{
		const TSoftObjectPtr<UTexture2D> BgSoft(FSoftObjectPath(TEXT("/Game/UI/MainMenu/MMRed.MMRed")));
		if (UTexture2D* Cached = TexPool->GetLoadedTexture(BgSoft))
		{
			SaveSlotsBackgroundBrush->SetResourceObject(Cached);
		}
	}

	SlotHeroPortraitBrushes.SetNum(SlotsPerPage);
	SlotIdolBrushes.SetNum(SlotsPerPage);
	for (int32 i = 0; i < SlotsPerPage; ++i)
	{
		if (!SlotHeroPortraitBrushes[i].IsValid())
		{
			SlotHeroPortraitBrushes[i] = MakeShared<FSlateBrush>();
			SlotHeroPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Box;
			SlotHeroPortraitBrushes[i]->SetResourceObject(nullptr);
		}
		SlotIdolBrushes[i].SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
		for (int32 k = 0; k < UT66IdolManagerSubsystem::MaxEquippedIdolSlots; ++k)
		{
			if (!SlotIdolBrushes[i][k].IsValid())
			{
				SlotIdolBrushes[i][k] = MakeShared<FSlateBrush>();
				SlotIdolBrushes[i][k]->DrawAs = ESlateBrushDrawType::Box;
				SlotIdolBrushes[i][k]->ImageSize = FVector2D(24.f, 24.f);
				SlotIdolBrushes[i][k]->SetResourceObject(nullptr);
			}
		}
	}

	const float SlotPadding = 20.f;
	const float PortraitSize = 80.f;
	const bool bHasVisibleSaves = VisibleSlotIndices.Num() > 0;

	auto MakeSlotCard = [this, SaveSub, Loc, GI, TexPool, EmptyText, PreviewText, LoadText, PortraitSize](
		int32 LocalIndex) -> TSharedRef<SWidget>
	{
		const int32 SlotIndex = GetVisibleSlotIndexForPageEntry(LocalIndex);
		const bool bSlotVisible = SlotIndex != INDEX_NONE;
		bool bOccupied = bSlotVisible;
		FString LastPlayed, HeroDisplayName, MapName;
		if (bSlotVisible && SaveSub)
		{
			SaveSub->GetSlotMeta(SlotIndex, bOccupied, LastPlayed, HeroDisplayName, MapName);
		}

		UT66RunSaveGame* Loaded = nullptr;
		if (bOccupied && SaveSub) Loaded = SaveSub->LoadFromSlot(SlotIndex);

		FText HeroNameText = bOccupied && Loc ? Loc->GetText_HeroName(Loaded ? Loaded->HeroID : FName(*HeroDisplayName)) : EmptyText;
		int32 StageNum = Loaded ? Loaded->StageReached : 1;
		const TArray<FName>& Idols = Loaded ? Loaded->EquippedIdols : TArray<FName>();
		const TArray<uint8>& IdolTiers = Loaded ? Loaded->EquippedIdolTiers : TArray<uint8>();

		if (bOccupied && Loaded && TexPool && GI)
		{
			FHeroData HeroData;
			if (GI->GetHeroData(Loaded->HeroID, HeroData))
			{
				TSoftObjectPtr<UTexture2D> PortraitSoft = GI->ResolveHeroPortrait(HeroData, Loaded->HeroBodyType, ET66HeroPortraitVariant::Half);
				if (!PortraitSoft.IsNull())
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this,
						SlotHeroPortraitBrushes[LocalIndex], FName(TEXT("SaveSlotHero"), SlotIndex), true);
				}
			}
			for (int32 k = 0; k < FMath::Min(Idols.Num(), UT66IdolManagerSubsystem::MaxEquippedIdolSlots); ++k)
			{
				FIdolData IdolData;
				const int32 TierValue = IdolTiers.IsValidIndex(k)
					? FMath::Clamp(static_cast<int32>(IdolTiers[k]), 1, UT66IdolManagerSubsystem::MaxIdolLevel)
					: 1;
				const TSoftObjectPtr<UTexture2D> IdolIconSoft = GI->GetIdolData(Idols[k], IdolData)
					? IdolData.GetIconForRarity(UT66IdolManagerSubsystem::IdolTierValueToRarity(TierValue))
					: TSoftObjectPtr<UTexture2D>();
				if (!IdolIconSoft.IsNull())
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, IdolIconSoft, this,
						SlotIdolBrushes[LocalIndex][k], FName(TEXT("SaveSlotIdol"), SlotIndex * 10 + k), true);
				}
			}
		}

		TSharedRef<SWidget> PortraitWidget = SNew(SBox).WidthOverride(PortraitSize).HeightOverride(PortraitSize)
			[
				FT66Style::IsDotaTheme()
					? FT66DotaSlate::MakeViewportFrame(
						SNew(SImage)
						.Image(TAttribute<const FSlateBrush*>::Create([this, LocalIndex]()
							{ return SlotHeroPortraitBrushes.IsValidIndex(LocalIndex) && SlotHeroPortraitBrushes[LocalIndex].IsValid()
								? SlotHeroPortraitBrushes[LocalIndex].Get() : nullptr; })),
						FMargin(0.f))
					: StaticCastSharedRef<SWidget>(
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(bOccupied ? FT66Style::Tokens::Panel2 : FT66Style::Tokens::Panel)
						[
							SNew(SImage)
							.Image(TAttribute<const FSlateBrush*>::Create([this, LocalIndex]()
								{ return SlotHeroPortraitBrushes.IsValidIndex(LocalIndex) && SlotHeroPortraitBrushes[LocalIndex].IsValid()
									? SlotHeroPortraitBrushes[LocalIndex].Get() : nullptr; }))
						])
			];

		TSharedRef<SHorizontalBox> IdolRow = SNew(SHorizontalBox);
		for (int32 k = 0; k < UT66IdolManagerSubsystem::MaxEquippedIdolSlots; ++k)
		{
			IdolRow->AddSlot().AutoWidth().Padding(2.f)
				[
					SNew(SBox).WidthOverride(24.f).HeightOverride(24.f)
					[
						SNew(SImage)
						.Image(TAttribute<const FSlateBrush*>::Create([this, LocalIndex, k]() -> const FSlateBrush*
						{
							if (SlotIdolBrushes.IsValidIndex(LocalIndex) && SlotIdolBrushes[LocalIndex].IsValidIndex(k) && SlotIdolBrushes[LocalIndex][k].IsValid())
								return SlotIdolBrushes[LocalIndex][k].Get();
							return nullptr;
						}))
					]
				];
		}

		TSharedRef<SVerticalBox> CardContent = SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f)
			[PortraitWidget]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f)
			[
				SNew(STextBlock)
				.Text(HeroNameText)
				.Font(bOccupied ? FT66Style::Tokens::FontBold(14) : FT66Style::Tokens::FontRegular(14))
				.ColorAndOpacity(bOccupied ? FT66Style::Tokens::Text : FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 2.f)
			[
				SNew(STextBlock)
				.Text(bOccupied ? FText::Format(NSLOCTEXT("T66.SaveSlots", "StageFormat", "Stage {0}"), FText::AsNumber(StageNum)) : FText::GetEmpty())
				.Font(FT66Style::Tokens::FontRegular(12))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f)
			[IdolRow]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
				[
					FT66Style::MakeButton(FT66ButtonParams(PreviewText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandlePreviewClicked, SlotIndex), ET66ButtonType::Neutral).SetMinWidth(90.f).SetEnabled(bOccupied && bSlotVisible))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
				[
					FT66Style::MakeButton(FT66ButtonParams(LoadText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleLoadClicked, SlotIndex), ET66ButtonType::Success).SetMinWidth(90.f).SetEnabled(bOccupied && bSlotVisible))
				]
			];

		return FT66Style::IsDotaTheme()
			? FT66DotaSlate::MakeScreenSurface(
				SNew(SBox).WidthOverride(220.f)
				[
					CardContent
				],
				FMargin(16.f))
			: StaticCastSharedRef<SWidget>(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(bOccupied ? FT66Style::Tokens::Panel2 : FT66Style::Tokens::Panel)
				.Padding(16.f)
				[
					SNew(SBox).WidthOverride(220.f)
					[
						CardContent
					]
				]);
	};

	TSharedRef<SWidget> BgUnderlay = SNew(SBorder).BorderBackgroundColor(FT66Style::Tokens::Bg);
	const float TopInset = UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f;
	const bool bShowBackButton = !(UIManager && UIManager->IsFrontendTopBarVisible());

	const FText PageText = FText::Format(
		NSLOCTEXT("T66.SaveSlots", "PageFormat", "Page {0} / {1}"),
		FText::AsNumber(CurrentPage + 1), FText::AsNumber(TotalPages));

	return SNew(SBox)
		.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
		[
			SNew(SOverlay)
		+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)[BgUnderlay]
		+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SImage)
			.Image(TAttribute<const FSlateBrush*>::Create([this]()
				{ return SaveSlotsBackgroundBrush.IsValid() ? SaveSlotsBackgroundBrush.Get() : nullptr; }))
		]
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 40.f, 0.f, 30.f)
			[SNew(STextBlock).Text(TitleText).Font(FT66Style::Tokens::FontBold(48)).ColorAndOpacity(FT66Style::Tokens::Text)]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 20.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.SaveSlots", "PartyFilterHint", "Showing saves for the current party."))
				.Font(FT66Style::Tokens::FontRegular(14))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 20.f)
			[
				SNew(SBox)
				.Visibility(bHasVisibleSaves ? EVisibility::Collapsed : EVisibility::Visible)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.SaveSlots", "NoPartySaves", "No saves found for the current party."))
					.Font(FT66Style::Tokens::FontRegular(18))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
			]
			+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(SlotPadding, 0.f)[MakeSlotCard(0)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(SlotPadding, 0.f)[MakeSlotCard(1)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(SlotPadding, 0.f)[MakeSlotCard(2)]
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 24.f, 0.f, 20.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(12.f, 0.f)
				[
					FT66Style::MakeButton(FT66ButtonParams(PrevText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandlePrevPageClicked), ET66ButtonType::Neutral).SetMinWidth(100.f).SetEnabled(CurrentPage > 0))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(16.f, 0.f).VAlign(VAlign_Center)
				[SNew(STextBlock).Text(PageText).Font(FT66Style::Tokens::FontRegular(14)).ColorAndOpacity(FT66Style::Tokens::Text)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(12.f, 0.f)
				[
					FT66Style::MakeButton(FT66ButtonParams(NextText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleNextPageClicked), ET66ButtonType::Neutral).SetMinWidth(100.f).SetEnabled(CurrentPage < TotalPages - 1))
				]
			]
		]
		+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(20.f, 0.f, 0.f, 20.f)
		[
			SNew(SBox)
			.Visibility(bShowBackButton ? EVisibility::Visible : EVisibility::Collapsed)
			[
				FT66Style::MakeButton(FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleBackClicked), ET66ButtonType::Neutral).SetMinWidth(120.f))
			]
		]
		];
}

FReply UT66SaveSlotsScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

FReply UT66SaveSlotsScreen::HandleLoadClicked(int32 SlotIndex)
{
	OnLoadClicked(SlotIndex);
	return FReply::Handled();
}

FReply UT66SaveSlotsScreen::HandlePreviewClicked(int32 SlotIndex)
{
	OnPreviewClicked(SlotIndex);
	return FReply::Handled();
}

FReply UT66SaveSlotsScreen::HandlePrevPageClicked()
{
	OnPreviousPageClicked();
	return FReply::Handled();
}

FReply UT66SaveSlotsScreen::HandleNextPageClicked()
{
	OnNextPageClicked();
	return FReply::Handled();
}

void UT66SaveSlotsScreen::RebuildVisibleSlotIndices()
{
	VisibleSlotIndices.Reset();

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	if (!SaveSub)
	{
		TotalPages = 1;
		CurrentPage = 0;
		return;
	}

	for (int32 SlotIndex = 0; SlotIndex < UT66SaveSubsystem::MaxSlots; ++SlotIndex)
	{
		bool bOccupied = false;
		FString LastPlayedUtc;
		FString HeroDisplayName;
		FString MapName;
		SaveSub->GetSlotMeta(SlotIndex, bOccupied, LastPlayedUtc, HeroDisplayName, MapName);
		if (!bOccupied)
		{
			continue;
		}

		UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(SlotIndex);
		if (!Loaded)
		{
			continue;
		}

		if (!PartySubsystem || PartySubsystem->DoesSaveMatchCurrentParty(Loaded))
		{
			VisibleSlotIndices.Add(SlotIndex);
		}
	}

	TotalPages = FMath::Max(1, (VisibleSlotIndices.Num() + SlotsPerPage - 1) / SlotsPerPage);
	CurrentPage = FMath::Clamp(CurrentPage, 0, TotalPages - 1);
}

int32 UT66SaveSlotsScreen::GetVisibleSlotIndexForPageEntry(int32 LocalIndex) const
{
	const int32 VisibleIndex = (CurrentPage * SlotsPerPage) + LocalIndex;
	return VisibleSlotIndices.IsValidIndex(VisibleIndex) ? VisibleSlotIndices[VisibleIndex] : INDEX_NONE;
}

void UT66SaveSlotsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	CurrentPage = 0;
	RebuildVisibleSlotIndices();
}

void UT66SaveSlotsScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	RebuildVisibleSlotIndices();
	ForceRebuildSlate();
}

void UT66SaveSlotsScreen::OnLoadClicked(int32 SlotIndex)
{
	if (!VisibleSlotIndices.Contains(SlotIndex) || !IsSlotOccupied(SlotIndex)) return;

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	if (!GI || !SaveSub) return;

	UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(SlotIndex);
	if (!Loaded) return;

	GI->SelectedHeroID = Loaded->HeroID;
	GI->SelectedHeroBodyType = Loaded->HeroBodyType;
	GI->SelectedCompanionID = Loaded->CompanionID;
	GI->SelectedDifficulty = Loaded->Difficulty;
	GI->SelectedPartySize = Loaded->PartySize;
	GI->PendingLoadedTransform = Loaded->PlayerTransform;
	GI->bApplyLoadedTransform = true;
	GI->bLoadAsPreview = false;
	GI->CurrentSaveSlotIndex = SlotIndex;
	GI->CurrentRunOwnerPlayerId = Loaded->OwnerPlayerId;
	GI->CurrentRunOwnerDisplayName = Loaded->OwnerDisplayName;
	GI->CurrentRunPartyMemberIds = Loaded->PartyMemberIds;
	GI->CurrentRunPartyMemberDisplayNames = Loaded->PartyMemberDisplayNames;

	if (UIManager) UIManager->HideAllUI();
	GI->TransitionToGameplayLevel();
}

void UT66SaveSlotsScreen::OnSlotClicked(int32 SlotIndex)
{
	OnLoadClicked(SlotIndex);
}

void UT66SaveSlotsScreen::OnPreviewClicked(int32 SlotIndex)
{
	if (!VisibleSlotIndices.Contains(SlotIndex) || !IsSlotOccupied(SlotIndex)) return;

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	if (!GI || !SaveSub) return;

	UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(SlotIndex);
	if (!Loaded) return;

	GI->SelectedHeroID = Loaded->HeroID;
	GI->SelectedHeroBodyType = Loaded->HeroBodyType;
	GI->SelectedCompanionID = Loaded->CompanionID;
	GI->SelectedDifficulty = Loaded->Difficulty;
	GI->SelectedPartySize = Loaded->PartySize;
	GI->PendingLoadedTransform = Loaded->PlayerTransform;
	GI->bApplyLoadedTransform = true;
	GI->bLoadAsPreview = true;
	GI->CurrentSaveSlotIndex = SlotIndex;
	GI->CurrentRunOwnerPlayerId = Loaded->OwnerPlayerId;
	GI->CurrentRunOwnerDisplayName = Loaded->OwnerDisplayName;
	GI->CurrentRunPartyMemberIds = Loaded->PartyMemberIds;
	GI->CurrentRunPartyMemberDisplayNames = Loaded->PartyMemberDisplayNames;

	if (UIManager) UIManager->HideAllUI();
	GI->TransitionToGameplayLevel();
}

bool UT66SaveSlotsScreen::IsSlotOccupied(int32 SlotIndex) const
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	if (!SaveSub) return false;
	bool bOccupied = false;
	FString D1, D2, D3;
	SaveSub->GetSlotMeta(SlotIndex, bOccupied, D1, D2, D3);
	return bOccupied;
}
void UT66SaveSlotsScreen::OnPreviousPageClicked()
{
	if (CurrentPage > 0) { CurrentPage--; RefreshScreen(); }
}

void UT66SaveSlotsScreen::OnNextPageClicked()
{
	if (CurrentPage < TotalPages - 1) { CurrentPage++; RefreshScreen(); }
}

void UT66SaveSlotsScreen::OnBackClicked() { NavigateBack(); }
