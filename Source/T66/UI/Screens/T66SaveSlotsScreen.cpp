// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SaveSlotsScreen.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66RunIntegritySubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66UIManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float T66SaveFlowDesignWidth = 1920.f;
	constexpr float T66SaveFlowDesignHeight = 1080.f;
	constexpr float T66SaveFlowActionHeight = 54.f;
	constexpr float T66SaveFlowPreviewSlotSize = 90.f;

	FLinearColor T66SaveFlowGoldText()
	{
		return FT66FlatStyle::SelectedText();
	}

	FLinearColor T66SaveFlowBrightText()
	{
		return FT66FlatStyle::PrimaryText();
	}

	FLinearColor T66SaveFlowMutedText()
	{
		return FT66FlatStyle::SecondaryText();
	}

	FLinearColor T66SaveFlowWarningText()
	{
		return FT66FlatStyle::SelectedText();
	}

	FLinearColor T66SaveFlowInkText()
	{
		return FT66FlatStyle::PrimaryText();
	}

	FLinearColor T66SaveFlowInkMutedText()
	{
		return FT66FlatStyle::SecondaryText();
	}

	TSharedRef<SWidget> MakeSaveFlowBackground()
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
			];
	}

	TSharedRef<SWidget> MakeSaveFlowDesignSlot(const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		return SNew(SBox)
			.WidthOverride(W)
			.HeightOverride(H)
			[
				Widget
			];
	}

	void AddSaveFlowCanvasSlot(
		const TSharedRef<SConstraintCanvas>& Canvas,
		const float X,
		const float Y,
		const float W,
		const float H,
		const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			.Offset(FMargin(X, Y, W, H))
			[
				MakeSaveFlowDesignSlot(W, H, Widget)
			];
	}

	TSharedRef<SWidget> MakeSaveFlowThinLine(const FLinearColor& Tint = FT66FlatStyle::DefaultBorder())
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(Tint);
	}

	TSharedRef<SWidget> MakeSaveFlowShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		return FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, Padding, Content);
	}

	TSharedRef<SWidget> MakeSaveFlowRowShell(const TSharedRef<SWidget>& Content, const FMargin& Padding, const bool bSelected, const bool bEnabled)
	{
		return FT66FlatStyle::MakeFlatPanel(
			bSelected ? ET66FlatState::Selected : (bEnabled ? ET66FlatState::Default : ET66FlatState::Disabled),
			Padding,
			Content);
	}

	TSharedRef<SWidget> MakeSaveFlowCardShell(const TSharedRef<SWidget>& Content, const FMargin& Padding, const bool bSelected, const bool bEnabled)
	{
		return MakeSaveFlowRowShell(Content, Padding, bSelected, bEnabled);
	}

	TSharedRef<SWidget> MakeSaveFlowDropdown(
		const FText& CurrentValueText,
		TFunction<TSharedRef<SWidget>()> OptionsProvider,
		const float MinWidth,
		const float Height,
		const FName Tag)
	{
		return FT66FlatStyle::MakeFlatDropdown(
			ET66FlatState::Selected,
			TAttribute<FText>(CurrentValueText),
			MoveTemp(OptionsProvider),
			true,
			MinWidth,
			Height,
			20,
			Tag);
	}

	TSharedRef<SWidget> MakeSaveFlowPlateButton(
		const FText& Label,
		FOnClicked OnClicked,
		const ET66FlatState State,
		const bool bEnabled,
		const float MinWidth,
		const float Height,
		const int32 FontSize,
		const FName Tag)
	{
		return FT66FlatStyle::MakeFlatButton(
			bEnabled ? State : ET66FlatState::Disabled,
			Label,
			MoveTemp(OnClicked),
			nullptr,
			nullptr,
			FMargin(14.f, 8.f),
			MinWidth,
			Height,
			bEnabled,
			FontSize,
			Tag);
	}

	int32 T66PartySizeToMemberCount(const ET66PartySize PartySize)
	{
		switch (PartySize)
		{
		case ET66PartySize::Duo:
			return 2;
		case ET66PartySize::Trio:
			return 3;
		case ET66PartySize::Quad:
			return 4;
		case ET66PartySize::Solo:
		default:
			return 1;
		}
	}

	FText T66PartySizeText(UT66LocalizationSubsystem* Loc, const ET66PartySize PartySize)
	{
		if (!Loc)
		{
			switch (PartySize)
			{
			case ET66PartySize::Duo:
				return NSLOCTEXT("T66.SaveSlots", "DuoFallback", "Duo");
			case ET66PartySize::Trio:
				return NSLOCTEXT("T66.SaveSlots", "TrioFallback", "Trio");
			case ET66PartySize::Quad:
				return NSLOCTEXT("T66.SaveSlots", "QuadFallback", "Quad");
			case ET66PartySize::Solo:
			default:
				return NSLOCTEXT("T66.SaveSlots", "SoloFallback", "Solo");
			}
		}

		switch (PartySize)
		{
		case ET66PartySize::Duo:
			return Loc->GetText_Duo();
		case ET66PartySize::Trio:
			return Loc->GetText_Trio();
		case ET66PartySize::Quad:
			return Loc->GetText_Quad();
		case ET66PartySize::Solo:
		default:
			return Loc->GetText_Solo();
		}
	}

	FText T66DifficultyText(UT66LocalizationSubsystem* Loc, const ET66Difficulty Difficulty)
	{
		if (!Loc)
		{
			switch (Difficulty)
			{
			case ET66Difficulty::Medium:
				return NSLOCTEXT("T66.SaveSlots", "MediumFallback", "Medium");
			case ET66Difficulty::Hard:
				return NSLOCTEXT("T66.SaveSlots", "HardFallback", "Hard");
			case ET66Difficulty::VeryHard:
				return NSLOCTEXT("T66.SaveSlots", "VeryHardFallback", "Very Hard");
			case ET66Difficulty::Impossible:
				return NSLOCTEXT("T66.SaveSlots", "ImpossibleFallback", "Impossible");
			case ET66Difficulty::Easy:
			default:
				return NSLOCTEXT("T66.SaveSlots", "EasyFallback", "Easy");
			}
		}

		switch (Difficulty)
		{
		case ET66Difficulty::Medium:
			return Loc->GetText_Medium();
		case ET66Difficulty::Hard:
			return Loc->GetText_Hard();
		case ET66Difficulty::VeryHard:
			return Loc->GetText_VeryHard();
		case ET66Difficulty::Impossible:
			return Loc->GetText_Impossible();
		case ET66Difficulty::Easy:
		default:
			return Loc->GetText_Easy();
		}
	}

	void T66BuildSavedPartyPlayers(const UT66RunSaveGame* Loaded, TArray<FT66SavedPartyPlayerState>& OutPlayers)
	{
		OutPlayers.Reset();
		if (!Loaded)
		{
			return;
		}

		if (Loaded->SavedPartyPlayers.Num() > 0)
		{
			OutPlayers = Loaded->SavedPartyPlayers;
			return;
		}

		FT66SavedPartyPlayerState& HostPlayer = OutPlayers.AddDefaulted_GetRef();
		HostPlayer.PlayerId = Loaded->OwnerPlayerId;
		HostPlayer.DisplayName = Loaded->OwnerDisplayName;
		HostPlayer.HeroID = Loaded->HeroID;
		HostPlayer.HeroBodyType = Loaded->HeroBodyType;
		HostPlayer.CompanionID = Loaded->CompanionID;
		HostPlayer.PlayerTransform = Loaded->PlayerTransform;
		HostPlayer.bIsPartyHost = true;
	}

	FString T66BuildDateString(const FString& LastPlayedUtc)
	{
		FDateTime ParsedUtc;
		if (FDateTime::ParseIso8601(*LastPlayedUtc, ParsedUtc))
		{
			return ParsedUtc.ToString(TEXT("%Y-%m-%d"));
		}

		return LastPlayedUtc.IsEmpty() ? TEXT("--") : LastPlayedUtc.Left(10);
	}

	FString T66BuildTimeString(const FString& LastPlayedUtc)
	{
		FDateTime ParsedUtc;
		if (FDateTime::ParseIso8601(*LastPlayedUtc, ParsedUtc))
		{
			return ParsedUtc.ToString(TEXT("%H:%M"));
		}

		return LastPlayedUtc.IsEmpty() ? TEXT("--:--") : LastPlayedUtc.Mid(11, 5);
	}

	FString T66BuildInitials(const FString& DisplayName, const int32 SlotIndex)
	{
		const FString Trimmed = DisplayName.TrimStartAndEnd();
		if (Trimmed.IsEmpty())
		{
			return FString::Printf(TEXT("%d"), SlotIndex + 1);
		}

		TArray<FString> Parts;
		Trimmed.ParseIntoArrayWS(Parts);
		if (Parts.Num() >= 2)
		{
			return Parts[0].Left(1).ToUpper() + Parts[1].Left(1).ToUpper();
		}

		return Trimmed.Left(2).ToUpper();
	}
}

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
	UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	UT66SteamHelper* SteamHelper = GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr;
	const bool bIsPartyResumeFlow = SessionSubsystem
		&& SessionSubsystem->IsPartyLobbyContextActive()
		&& SessionSubsystem->GetCurrentLobbyPlayerCount() > 1;
	const bool bHostCanStartPartyLoad = !bIsPartyResumeFlow || (SessionSubsystem && SessionSubsystem->IsHostingPartySession());

	const FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");
	const FText LoadText = NSLOCTEXT("T66.SaveSlots", "Load", "LOAD");
	const FText LoadGameTitleText = NSLOCTEXT("T66.SaveSlots", "LoadGameTitle", "LOAD GAME");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText PrevText = NSLOCTEXT("T66.SaveSlots", "PrevPage", "PREV");
	const FText NextText = NSLOCTEXT("T66.SaveSlots", "NextPage", "NEXT");
	const FText EmptySlotText = NSLOCTEXT("T66.SaveSlots", "EmptySlot", "Empty Slot");
	const FText StagePlaceholderText = NSLOCTEXT("T66.SaveSlots", "StagePlaceholder", "Stage --");
	const FText DatePlaceholderText = NSLOCTEXT("T66.SaveSlots", "DatePlaceholder", "--");
	const FText TimePlaceholderText = NSLOCTEXT("T66.SaveSlots", "TimePlaceholder", "--:--");

	SlotPartyAvatarBrushes.SetNum(SlotsPerPage);
	SlotHeroPortraitBrushes.SetNum(SlotsPerPage);
	for (int32 LocalIndex = 0; LocalIndex < SlotsPerPage; ++LocalIndex)
	{
		SlotPartyAvatarBrushes[LocalIndex].SetNum(MaxPartyPreviewSlots);
		SlotHeroPortraitBrushes[LocalIndex].SetNum(MaxPartyPreviewSlots);
		for (int32 PartyIndex = 0; PartyIndex < MaxPartyPreviewSlots; ++PartyIndex)
		{
			if (!SlotPartyAvatarBrushes[LocalIndex][PartyIndex].IsValid())
			{
				SlotPartyAvatarBrushes[LocalIndex][PartyIndex] = MakeShared<FSlateBrush>();
				SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->DrawAs = ESlateBrushDrawType::Image;
				SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->Tiling = ESlateBrushTileType::NoTile;
				SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->ImageSize = FVector2D(48.f, 48.f);
			}

			if (!SlotHeroPortraitBrushes[LocalIndex][PartyIndex].IsValid())
			{
				SlotHeroPortraitBrushes[LocalIndex][PartyIndex] = MakeShared<FSlateBrush>();
				SlotHeroPortraitBrushes[LocalIndex][PartyIndex]->DrawAs = ESlateBrushDrawType::Image;
				SlotHeroPortraitBrushes[LocalIndex][PartyIndex]->Tiling = ESlateBrushTileType::NoTile;
				SlotHeroPortraitBrushes[LocalIndex][PartyIndex]->ImageSize = FVector2D(48.f, 48.f);
			}

			SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->SetResourceObject(nullptr);
			SlotHeroPortraitBrushes[LocalIndex][PartyIndex]->SetResourceObject(nullptr);
		}
	}

	const float CardWidth = 754.f;
	const float CardHeight = 365.f;
	const int32 CurrentPartyCount = PartySubsystem ? FMath::Max(1, PartySubsystem->GetPartyMemberCount()) : 1;

	auto MakePartySizeMenu = [this, Loc]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
		for (const ET66PartySize PartySize : { ET66PartySize::Solo, ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad })
		{
			const bool bSelectedPartySize = ActivePartySizeFilter == PartySize;
			Box->AddSlot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 6.f)
				[
					FT66FlatStyle::MakeFlatButton(
						bSelectedPartySize ? ET66FlatState::Selected : ET66FlatState::Default,
						T66PartySizeText(Loc, PartySize),
						FOnClicked::CreateLambda([this, PartySize]()
						{
							ActivePartySizeFilter = PartySize;
							CurrentPage = 0;
							RefreshScreen();
							FSlateApplication::Get().DismissAllMenus();
							return FReply::Handled();
						}),
						nullptr,
						nullptr,
						FMargin(10.f, 5.f),
						150.f,
						38.f,
						true,
						18,
						FName(*FString::Printf(TEXT("SaveSlots.PartyFilter.%s"), *T66PartySizeText(Loc, PartySize).ToString())))
				];
		}
		return Box;
	};

	auto MakePartyMemberSlot = [this](const int32 LocalIndex, const int32 PartyIndex, const FT66SavedPartyPlayerState* SavedPlayer) -> TSharedRef<SWidget>
	{
		const bool bHasAvatar = SlotPartyAvatarBrushes.IsValidIndex(LocalIndex)
			&& SlotPartyAvatarBrushes[LocalIndex].IsValidIndex(PartyIndex)
			&& SlotPartyAvatarBrushes[LocalIndex][PartyIndex].IsValid()
			&& ::IsValid(SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->GetResourceObject());
		const FString Initials = SavedPlayer ? T66BuildInitials(SavedPlayer->DisplayName, PartyIndex) : FString::Printf(TEXT("%d"), PartyIndex + 1);

		return SNew(SBox)
			.WidthOverride(T66SaveFlowPreviewSlotSize)
			.HeightOverride(T66SaveFlowPreviewSlotSize)
			[
				FT66FlatStyle::MakeFlatSubPanel(
					SavedPlayer ? (SavedPlayer->bIsPartyHost ? ET66FlatState::Selected : ET66FlatState::Default) : ET66FlatState::Disabled,
					FMargin(12.f),
					bHasAvatar
						? StaticCastSharedRef<SWidget>(SNew(SImage).Image(SlotPartyAvatarBrushes[LocalIndex][PartyIndex].Get()))
						: StaticCastSharedRef<SWidget>(
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FT66FlatStyle::DisabledFill())
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(Initials))
								.Font(FT66FlatStyle::MakeBoldFont(14))
								.ColorAndOpacity(SavedPlayer ? T66SaveFlowGoldText() : T66SaveFlowMutedText())
								.Justification(ETextJustify::Center)
							]))
			];
	};

	auto MakeHeroSlot = [this](const int32 LocalIndex, const int32 PartyIndex, const FT66SavedPartyPlayerState* SavedPlayer) -> TSharedRef<SWidget>
	{
		const bool bHasPortrait = SlotHeroPortraitBrushes.IsValidIndex(LocalIndex)
			&& SlotHeroPortraitBrushes[LocalIndex].IsValidIndex(PartyIndex)
			&& SlotHeroPortraitBrushes[LocalIndex][PartyIndex].IsValid()
			&& ::IsValid(SlotHeroPortraitBrushes[LocalIndex][PartyIndex]->GetResourceObject());
		const FText PlaceholderText = SavedPlayer && !SavedPlayer->HeroID.IsNone()
			? FText::FromName(SavedPlayer->HeroID)
			: FText::FromString(TEXT("--"));

		return SNew(SBox)
			.WidthOverride(T66SaveFlowPreviewSlotSize)
			.HeightOverride(T66SaveFlowPreviewSlotSize)
			[
				FT66FlatStyle::MakeFlatSubPanel(
					SavedPlayer ? ET66FlatState::Default : ET66FlatState::Disabled,
					FMargin(12.f),
					bHasPortrait
						? StaticCastSharedRef<SWidget>(SNew(SImage).Image(SlotHeroPortraitBrushes[LocalIndex][PartyIndex].Get()))
						: StaticCastSharedRef<SWidget>(
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FT66FlatStyle::DisabledFill())
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(PlaceholderText)
								.Font(FT66FlatStyle::MakeBoldFont(13))
								.ColorAndOpacity(SavedPlayer ? T66SaveFlowMutedText() : FT66FlatStyle::DisabledText())
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
								.WrapTextAt(T66SaveFlowPreviewSlotSize - 28.f)
								.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
								.Clipping(EWidgetClipping::ClipToBounds)
							]))
			];
	};

	TArray<int32> PageSlotIndices;
	TArray<bool> PageSlotHasVisibleSave;
	PageSlotIndices.Init(INDEX_NONE, SlotsPerPage);
	PageSlotHasVisibleSave.Init(false, SlotsPerPage);
	TSet<int32> UsedDisplaySlotIndices;
	auto CanUsePlaceholderSlot = [&UsedDisplaySlotIndices](const int32 SlotIndex) -> bool
	{
		return SlotIndex >= 0
			&& SlotIndex < UT66SaveSubsystem::MaxSlots
			&& !UsedDisplaySlotIndices.Contains(SlotIndex);
	};

	for (int32 LocalIndex = 0; LocalIndex < SlotsPerPage; ++LocalIndex)
	{
		const int32 VisibleIndex = (CurrentPage * SlotsPerPage) + LocalIndex;
		if (VisibleSlotIndices.IsValidIndex(VisibleIndex))
		{
			const int32 SlotIndex = VisibleSlotIndices[VisibleIndex];
			PageSlotIndices[LocalIndex] = SlotIndex;
			PageSlotHasVisibleSave[LocalIndex] = true;
			UsedDisplaySlotIndices.Add(SlotIndex);
		}
	}

	for (int32 LocalIndex = 0; LocalIndex < SlotsPerPage; ++LocalIndex)
	{
		if (PageSlotIndices[LocalIndex] != INDEX_NONE)
		{
			continue;
		}

		const int32 PreferredSlotIndex = (CurrentPage * SlotsPerPage) + LocalIndex;
		if (CanUsePlaceholderSlot(PreferredSlotIndex))
		{
			PageSlotIndices[LocalIndex] = PreferredSlotIndex;
			UsedDisplaySlotIndices.Add(PreferredSlotIndex);
			continue;
		}

		for (int32 CandidateSlotIndex = 0; CandidateSlotIndex < UT66SaveSubsystem::MaxSlots; ++CandidateSlotIndex)
		{
			if (CanUsePlaceholderSlot(CandidateSlotIndex))
			{
				PageSlotIndices[LocalIndex] = CandidateSlotIndex;
				UsedDisplaySlotIndices.Add(CandidateSlotIndex);
				break;
			}
		}
	}

	auto MakeSlotCard = [this, SaveSub, Loc, GI, TexPool, SteamHelper, PreviewText, LoadText, EmptySlotText,
		StagePlaceholderText, DatePlaceholderText, TimePlaceholderText, CardWidth, CardHeight, CurrentPartyCount,
		bHostCanStartPartyLoad, PageSlotIndices, PageSlotHasVisibleSave, MakePartyMemberSlot, MakeHeroSlot](
		const int32 LocalIndex) -> TSharedRef<SWidget>
	{
		const int32 SlotIndex = PageSlotIndices.IsValidIndex(LocalIndex) ? PageSlotIndices[LocalIndex] : INDEX_NONE;
		if (SlotIndex == INDEX_NONE)
		{
			return SNew(SBox).Visibility(EVisibility::Collapsed);
		}

		const bool bHasVisibleSave = PageSlotHasVisibleSave.IsValidIndex(LocalIndex) && PageSlotHasVisibleSave[LocalIndex];
		UT66RunSaveGame* Loaded = (bHasVisibleSave && SaveSub) ? SaveSub->LoadFromSlot(SlotIndex) : nullptr;

		TArray<FT66SavedPartyPlayerState> SavedPlayers;
		T66BuildSavedPartyPlayers(Loaded, SavedPlayers);

		for (int32 PartyIndex = 0; PartyIndex < MaxPartyPreviewSlots; ++PartyIndex)
		{
			SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->SetResourceObject(nullptr);
			SlotHeroPortraitBrushes[LocalIndex][PartyIndex]->SetResourceObject(nullptr);

			const FT66SavedPartyPlayerState* SavedPlayer = SavedPlayers.IsValidIndex(PartyIndex) ? &SavedPlayers[PartyIndex] : nullptr;
			if (!SavedPlayer)
			{
				continue;
			}

			if (SteamHelper)
			{
				if (UTexture2D* AvatarTexture = SteamHelper->GetAvatarTextureForSteamId(SavedPlayer->PlayerId))
				{
					SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->SetResourceObject(AvatarTexture);
				}
				else if (SavedPlayer->PlayerId == SteamHelper->GetLocalSteamId())
				{
					SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->SetResourceObject(SteamHelper->GetLocalAvatarTexture());
				}
			}

			if (GI && TexPool && !SavedPlayer->HeroID.IsNone())
			{
				FHeroData HeroData;
				if (GI->GetHeroData(SavedPlayer->HeroID, HeroData))
				{
					const TSoftObjectPtr<UTexture2D> PortraitSoft = GI->ResolveHeroPortrait(HeroData, SavedPlayer->HeroBodyType, ET66HeroPortraitVariant::Half);
					if (!PortraitSoft.IsNull())
					{
						T66SlateTexture::BindSharedBrushAsync(
							TexPool,
							PortraitSoft,
							this,
							SlotHeroPortraitBrushes[LocalIndex][PartyIndex],
							FName(*FString::Printf(TEXT("SaveSlotHero_%d_%d"), SlotIndex, PartyIndex)),
							true);
					}
				}
			}
		}

		const bool bHasRunData = Loaded != nullptr;
		const int32 RequiredPartyCount = bHasRunData
			? FMath::Max(1, SavedPlayers.Num() > 0 ? SavedPlayers.Num() : T66PartySizeToMemberCount(Loaded->PartySize))
			: 1;
		const bool bPartyCountMatches = CurrentPartyCount == RequiredPartyCount;
		const bool bCanLoad = bHasRunData && bHostCanStartPartyLoad && bPartyCountMatches;
		const FText StatusText = !bHasRunData
			? EmptySlotText
			: (bCanLoad
			? FText::Format(
				NSLOCTEXT("T66.SaveSlots", "DifficultyParty", "{0} / {1}"),
				T66DifficultyText(Loc, Loaded->Difficulty),
				T66PartySizeText(Loc, Loaded->PartySize))
			: FText::Format(
				NSLOCTEXT("T66.SaveSlots", "RequiresParty", "Requires {0} party"),
				T66PartySizeText(Loc, Loaded->PartySize)));
		const bool bIsSelected = bHasRunData && GI && GI->CurrentSaveSlotIndex == SlotIndex;
		const FText StageText = bHasRunData
			? FText::Format(NSLOCTEXT("T66.SaveSlots", "StageLabel", "Stage {0}"), FText::AsNumber(Loaded->StageReached))
			: StagePlaceholderText;
		const FText DateText = bHasRunData
			? FText::FromString(T66BuildDateString(Loaded->LastPlayedUtc))
			: DatePlaceholderText;
		const FText TimeText = bHasRunData
			? FText::FromString(T66BuildTimeString(Loaded->LastPlayedUtc))
			: TimePlaceholderText;

		TSharedRef<SGridPanel> SlotGrid = SNew(SGridPanel);
		for (int32 PartyIndex = 0; PartyIndex < MaxPartyPreviewSlots; ++PartyIndex)
		{
			const FT66SavedPartyPlayerState* SavedPlayer = SavedPlayers.IsValidIndex(PartyIndex) ? &SavedPlayers[PartyIndex] : nullptr;
			const bool bLastPreviewColumn = PartyIndex == MaxPartyPreviewSlots - 1;
			SlotGrid->AddSlot(PartyIndex, 0)
				.Padding(0.f, 0.f, bLastPreviewColumn ? 0.f : 8.f, 10.f)
				[
					MakePartyMemberSlot(LocalIndex, PartyIndex, SavedPlayer)
				];

			SlotGrid->AddSlot(PartyIndex, 1)
				.Padding(0.f, 0.f, bLastPreviewColumn ? 0.f : 8.f, 0.f)
				[
					MakeHeroSlot(LocalIndex, PartyIndex, SavedPlayer)
				];
		}

		TSharedRef<SConstraintCanvas> CardCanvas = SNew(SConstraintCanvas);
		AddSaveFlowCanvasSlot(
			CardCanvas,
			34.f, 22.f, 188.f, 36.f,
			SNew(STextBlock)
			.Text(FText::Format(NSLOCTEXT("T66.SaveSlots", "SlotHeader", "Save Slot {0}"), FText::AsNumber(SlotIndex + 1)))
			.Font(FT66FlatStyle::MakeBoldFont(22))
			.ColorAndOpacity(bIsSelected ? FT66FlatStyle::SelectedText() : T66SaveFlowInkText())
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds));
		AddSaveFlowCanvasSlot(CardCanvas, 210.f, 40.f, 388.f, 4.f, MakeSaveFlowThinLine(FT66FlatStyle::DefaultBorder()));
		AddSaveFlowCanvasSlot(CardCanvas, 34.f, 74.f, 392.f, 190.f, SlotGrid);
		AddSaveFlowCanvasSlot(CardCanvas, 444.f, 70.f, 3.f, 190.f, MakeSaveFlowThinLine(FT66FlatStyle::DefaultBorder()));
		AddSaveFlowCanvasSlot(
			CardCanvas,
			475.f, 72.f, 258.f, 188.f,
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(StageText)
				.Font(FT66FlatStyle::MakeBoldFont(18))
				.ColorAndOpacity(bHasRunData ? T66SaveFlowInkText() : T66SaveFlowInkMutedText())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(DateText)
				.Font(FT66FlatStyle::MakeBoldFont(16))
				.ColorAndOpacity(T66SaveFlowInkMutedText())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(TimeText)
				.Font(FT66FlatStyle::MakeBoldFont(16))
				.ColorAndOpacity(T66SaveFlowInkMutedText())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 16.f, 0.f, 0.f)
			[
			SNew(SBox)
			.WidthOverride(258.f)
			.HeightOverride(4.f)
			[
				MakeSaveFlowThinLine(FT66FlatStyle::DefaultBorder())
			]
		]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 14.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(StatusText)
				.Font(FT66FlatStyle::MakeBoldFont(17))
				.ColorAndOpacity(!bHasRunData ? T66SaveFlowInkText() : (bCanLoad ? FT66FlatStyle::ReadyBorder() : T66SaveFlowWarningText()))
				.AutoWrapText(true)
				.WrapTextAt(258.f)
				.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 5.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(TimePlaceholderText)
				.Font(FT66FlatStyle::MakeBoldFont(16))
				.ColorAndOpacity(T66SaveFlowInkMutedText())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
				.Visibility(bHasRunData ? EVisibility::Collapsed : EVisibility::Visible)
			]);
		AddSaveFlowCanvasSlot(
			CardCanvas,
			39.f, 279.f, 337.f, 54.f,
			MakeSaveFlowPlateButton(
				PreviewText,
				FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandlePreviewClicked, SlotIndex),
				ET66FlatState::Default,
				bHasRunData,
				337.f,
				54.f,
				22,
				FName(*FString::Printf(TEXT("SaveSlots.Slot%d.PreviewButton"), SlotIndex + 1))));
		AddSaveFlowCanvasSlot(
			CardCanvas,
			389.f, 279.f, 337.f, 54.f,
			MakeSaveFlowPlateButton(
				LoadText,
				FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleLoadClicked, SlotIndex),
				ET66FlatState::Selected,
				bCanLoad,
				337.f,
				54.f,
				22,
				FName(*FString::Printf(TEXT("SaveSlots.Slot%d.LoadButton"), SlotIndex + 1))));

		return SNew(SBox)
			.WidthOverride(CardWidth)
			.HeightOverride(CardHeight)
			[
				MakeSaveFlowCardShell(
					CardCanvas,
					FMargin(0.f),
					bIsSelected,
					bHasRunData && bCanLoad)
			];
	};

	const FText PageText = FText::Format(
		NSLOCTEXT("T66.SaveSlots", "PageFormat", "Page {0} / {1}"),
		FText::AsNumber(CurrentPage + 1),
		FText::AsNumber(TotalPages));
	const FText FilterHintText = FText::Format(
		NSLOCTEXT("T66.SaveSlots", "FilterHint", "Showing {0} saves stored on this machine."),
		T66PartySizeText(Loc, ActivePartySizeFilter));
	const bool bCanGoPrev = CurrentPage > 0;
	const bool bCanGoNext = CurrentPage < TotalPages - 1;

	TSharedRef<SConstraintCanvas> DesignCanvas = SNew(SConstraintCanvas);

	AddSaveFlowCanvasSlot(
		DesignCanvas,
		0.f, 0.f, T66SaveFlowDesignWidth, T66SaveFlowDesignHeight,
		SNew(SSpacer));

	AddSaveFlowCanvasSlot(
		DesignCanvas,
		163.f, 239.f, 1591.f, 752.f,
		MakeSaveFlowShell(SNew(SSpacer), FMargin(0.f)));

	AddSaveFlowCanvasSlot(
		DesignCanvas,
		169.f, 60.f, 143.f, 62.f,
		MakeSaveFlowPlateButton(
			BackText,
			FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleBackClicked),
			ET66FlatState::Selected,
			true,
			143.f,
			62.f,
			22,
			TEXT("SaveSlots.BackButton")));

	AddSaveFlowCanvasSlot(
		DesignCanvas,
		748.f, 58.f, 397.f, 61.f,
		SNew(STextBlock)
		.Text(LoadGameTitleText)
		.Font(FT66FlatStyle::MakeBoldFont(54))
		.ColorAndOpacity(T66SaveFlowGoldText())
		.Justification(ETextJustify::Center));

	AddSaveFlowCanvasSlot(
		DesignCanvas,
		651.f, 139.f, 588.f, 4.f,
		MakeSaveFlowThinLine(FT66FlatStyle::SelectedBorder()));

	AddSaveFlowCanvasSlot(
		DesignCanvas,
		167.f, 158.f, 242.f, 64.f,
		MakeSaveFlowDropdown(
			T66PartySizeText(Loc, ActivePartySizeFilter),
			MakePartySizeMenu,
			242.f,
			64.f,
			TEXT("SaveSlots.PartyFilterDropdown")));

	AddSaveFlowCanvasSlot(
		DesignCanvas,
		432.f, 178.f, 566.f, 28.f,
		SNew(STextBlock)
		.Text(FilterHintText)
		.Font(FT66FlatStyle::MakeBoldFont(18))
		.ColorAndOpacity(T66SaveFlowMutedText())
		.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		.Clipping(EWidgetClipping::ClipToBounds));

	AddSaveFlowCanvasSlot(
		DesignCanvas,
		1642.f, 179.f, 109.f, 27.f,
		SNew(STextBlock)
		.Text(PageText)
		.Font(FT66FlatStyle::MakeBoldFont(18))
		.ColorAndOpacity(T66SaveFlowGoldText())
		.Justification(ETextJustify::Right)
		.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		.Clipping(EWidgetClipping::ClipToBounds));

	AddSaveFlowCanvasSlot(DesignCanvas, 179.f, 254.f, CardWidth, CardHeight, MakeSlotCard(0));
	AddSaveFlowCanvasSlot(DesignCanvas, 975.f, 254.f, CardWidth, CardHeight, MakeSlotCard(1));
	AddSaveFlowCanvasSlot(DesignCanvas, 179.f, 632.f, CardWidth, CardHeight, MakeSlotCard(2));
	AddSaveFlowCanvasSlot(DesignCanvas, 975.f, 632.f, CardWidth, CardHeight, MakeSlotCard(3));

	AddSaveFlowCanvasSlot(
		DesignCanvas,
		175.f, 1000.f, 179.f, 60.f,
		MakeSaveFlowPlateButton(
			PrevText,
			FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandlePrevPageClicked),
			ET66FlatState::Default,
			bCanGoPrev,
			179.f,
			60.f,
			22,
			TEXT("SaveSlots.PrevButton")));

	AddSaveFlowCanvasSlot(
		DesignCanvas,
		367.f, 1000.f, 181.f, 60.f,
		MakeSaveFlowPlateButton(
			NextText,
			FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleNextPageClicked),
			ET66FlatState::Default,
			bCanGoNext,
			181.f,
			60.f,
			22,
			TEXT("SaveSlots.NextButton")));

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			MakeSaveFlowBackground()
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			[
				SNew(SBox)
				.WidthOverride(T66SaveFlowDesignWidth)
				.HeightOverride(T66SaveFlowDesignHeight)
				[
					DesignCanvas
				]
			]
		];
}

FReply UT66SaveSlotsScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66SaveSlotsScreen::HandleLoadClicked(const int32 SlotIndex)
{
	OnLoadClicked(SlotIndex);
	return FReply::Handled();
}

FReply UT66SaveSlotsScreen::HandlePreviewClicked(const int32 SlotIndex)
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
		if (!Loaded || Loaded->PartySize != ActivePartySizeFilter)
		{
			continue;
		}

		VisibleSlotIndices.Add(SlotIndex);
	}

	TotalPages = FMath::Max(1, (VisibleSlotIndices.Num() + SlotsPerPage - 1) / SlotsPerPage);
	CurrentPage = FMath::Clamp(CurrentPage, 0, TotalPages - 1);
}

int32 UT66SaveSlotsScreen::GetVisibleSlotIndexForPageEntry(const int32 LocalIndex) const
{
	const int32 VisibleIndex = (CurrentPage * SlotsPerPage) + LocalIndex;
	return VisibleSlotIndices.IsValidIndex(VisibleIndex) ? VisibleSlotIndices[VisibleIndex] : INDEX_NONE;
}

void UT66SaveSlotsScreen::PrepareGameInstanceForLoadedSave(UT66GameInstance* GI, const UT66RunSaveGame* Loaded, const int32 SlotIndex, const bool bPreviewMode)
{
	if (!GI || !Loaded)
	{
		return;
	}

	GI->SelectedHeroID = GI->ResolvePlayableHeroID(Loaded->HeroID);
	GI->SelectedHeroBodyType = Loaded->HeroBodyType;
	GI->SelectedCompanionID = Loaded->CompanionID;
	GI->SelectedDifficulty = GI->ResolvePlayableDifficulty(Loaded->Difficulty);
	GI->SelectedPartySize = Loaded->PartySize;
	GI->RunSeed = Loaded->RunSeed;
	if (Loaded->bIsDailyClimbRun && Loaded->DailyClimbChallenge.IsValid())
	{
		GI->CachedDailyClimbChallenge = Loaded->DailyClimbChallenge;
		GI->ActiveDailyClimbChallenge = Loaded->DailyClimbChallenge;
		GI->bIsDailyClimbRunActive = true;
		GI->SelectedRunModifierKind = ET66RunModifierKind::None;
		GI->SelectedRunModifierID = NAME_None;
		GI->SelectedPartySize = ET66PartySize::Solo;
	}
	else
	{
		GI->ClearActiveDailyClimbRun();
	}
	GI->CurrentMainMapLayoutVariant = ET66MainMapLayoutVariant::Tower;
	GI->PendingLoadedTransform = Loaded->PlayerTransform;
	GI->bApplyLoadedTransform = true;
	GI->PendingLoadedRunSnapshot = Loaded->RunSnapshot;
	GI->bApplyLoadedRunSnapshot = Loaded->RunSnapshot.bValid;
	GI->CurrentSaveSlotIndex = SlotIndex;
	GI->bRunIneligibleForLeaderboard = Loaded->bRunIneligibleForLeaderboard;
	if (UT66RunIntegritySubsystem* Integrity = GI->GetSubsystem<UT66RunIntegritySubsystem>())
	{
		Integrity->RestoreActiveRunContext(Loaded->IntegrityContext);
		Integrity->MarkLoadedSnapshot();
		GI->bRunIneligibleForLeaderboard = GI->bRunIneligibleForLeaderboard || !Integrity->GetCurrentContext().ShouldAllowRankedSubmission();
	}
	GI->CurrentRunOwnerPlayerId = Loaded->OwnerPlayerId;
	GI->CurrentRunOwnerDisplayName = Loaded->OwnerDisplayName;
	GI->CurrentRunPartyMemberIds = Loaded->PartyMemberIds;
	GI->CurrentRunPartyMemberDisplayNames = Loaded->PartyMemberDisplayNames;
	GI->bSaveSlotPreviewMode = bPreviewMode;
	GI->PersistRememberedSelectionDefaults();
}

bool UT66SaveSlotsScreen::CanLoadSlotWithCurrentParty(const UT66RunSaveGame* Loaded) const
{
	if (!Loaded)
	{
		return false;
	}

	const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	const UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	const UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	const bool bIsPartyResumeFlow = SessionSubsystem
		&& SessionSubsystem->IsPartyLobbyContextActive()
		&& SessionSubsystem->GetCurrentLobbyPlayerCount() > 1;
	if (bIsPartyResumeFlow && !SessionSubsystem->IsHostingPartySession())
	{
		return false;
	}

	const int32 CurrentPartyCount = PartySubsystem ? FMath::Max(1, PartySubsystem->GetPartyMemberCount()) : 1;
	const int32 RequiredPartyCount = FMath::Max(1, Loaded->SavedPartyPlayers.Num() > 0 ? Loaded->SavedPartyPlayers.Num() : T66PartySizeToMemberCount(Loaded->PartySize));
	return CurrentPartyCount == RequiredPartyCount;
}

void UT66SaveSlotsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	if (GI && GI->bRestoreSaveSlotsState)
	{
		ActivePartySizeFilter = GI->PendingSaveSlotsPartyFilter;
		CurrentPage = FMath::Max(0, GI->PendingSaveSlotsPage);
		GI->bRestoreSaveSlotsState = false;
	}
	else
	{
		ActivePartySizeFilter = PartySubsystem ? PartySubsystem->GetCurrentPartySizeEnum() : ET66PartySize::Solo;
		CurrentPage = 0;
	}

	RebuildVisibleSlotIndices();
}

void UT66SaveSlotsScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	RebuildVisibleSlotIndices();
	ForceRebuildSlate();
}

void UT66SaveSlotsScreen::OnLoadClicked(const int32 SlotIndex)
{
	if (!VisibleSlotIndices.Contains(SlotIndex) || !IsSlotOccupied(SlotIndex))
	{
		return;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (!GI || !SaveSub)
	{
		return;
	}

	UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(SlotIndex);
	if (!Loaded || !CanLoadSlotWithCurrentParty(Loaded))
	{
		return;
	}

	const bool bIsPartyResumeFlow = SessionSubsystem
		&& SessionSubsystem->IsPartyLobbyContextActive()
		&& SessionSubsystem->GetCurrentLobbyPlayerCount() > 1;
	if (bIsPartyResumeFlow)
	{
		GI->bSaveSlotPreviewMode = false;
		if (!SessionSubsystem->IsHostingPartySession())
		{
			return;
		}

		if (SessionSubsystem->StartLoadedGameplayTravel(Loaded, SlotIndex))
		{
			if (UIManager)
			{
				UIManager->HideAllUI();
			}
			return;
		}
	}

	GI->bRestoreSaveSlotsState = false;
	GI->PendingSaveSlotsPage = 0;
	PrepareGameInstanceForLoadedSave(GI, Loaded, SlotIndex, false);

	if (UIManager)
	{
		UIManager->HideAllUI();
	}

	if (Loaded->RunSnapshot.bValid)
	{
		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetGameplayLevelName());
	}
	else
	{
		GI->TransitionToGameplayLevel();
	}
}

void UT66SaveSlotsScreen::OnPreviewClicked(const int32 SlotIndex)
{
	if (!VisibleSlotIndices.Contains(SlotIndex) || !IsSlotOccupied(SlotIndex))
	{
		return;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	if (!GI || !SaveSub)
	{
		return;
	}

	UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(SlotIndex);
	if (!Loaded)
	{
		return;
	}

	GI->bRestoreSaveSlotsState = true;
	GI->PendingSaveSlotsPartyFilter = ActivePartySizeFilter;
	GI->PendingSaveSlotsPage = CurrentPage;
	PrepareGameInstanceForLoadedSave(GI, Loaded, SlotIndex, true);

	if (UIManager)
	{
		UIManager->HideAllUI();
	}

	if (Loaded->RunSnapshot.bValid)
	{
		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetGameplayLevelName());
	}
	else
	{
		GI->TransitionToGameplayLevel();
	}
}

void UT66SaveSlotsScreen::OnSlotClicked(const int32 SlotIndex)
{
	OnLoadClicked(SlotIndex);
}

bool UT66SaveSlotsScreen::IsSlotOccupied(const int32 SlotIndex) const
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	if (!SaveSub)
	{
		return false;
	}

	bool bOccupied = false;
	FString LastPlayedUtc;
	FString HeroDisplayName;
	FString MapName;
	SaveSub->GetSlotMeta(SlotIndex, bOccupied, LastPlayedUtc, HeroDisplayName, MapName);
	return bOccupied;
}

void UT66SaveSlotsScreen::OnPreviousPageClicked()
{
	if (CurrentPage > 0)
	{
		--CurrentPage;
		RefreshScreen();
	}
}

void UT66SaveSlotsScreen::OnNextPageClicked()
{
	if (CurrentPage < TotalPages - 1)
	{
		++CurrentPage;
		RefreshScreen();
	}
}

void UT66SaveSlotsScreen::OnBackClicked()
{
	NavigateBack();
}
