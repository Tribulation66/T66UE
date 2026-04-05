// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MainMenuScreen.h"
#include "UI/ST66AnimatedBackground.h"
#include "UI/ST66AnimatedBorderGlow.h"
#include "UI/T66UIManager.h"
#include "UI/Components/T66LeaderboardPanel.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"
#include "ImageUtils.h"
#include "Brushes/SlateImageBrush.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SInvalidationPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SNullWidget.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "Algo/Count.h"
#include "Math/TransformCalculus2D.h"
#include "Rendering/SlateRenderTransform.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MainMenu, Log, All);

namespace
{
	const FVector2D MainMenuBackgroundImageSize(1920.f, 1080.f);

	void EnsureRuntimeImageBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
	}

	void SetupMainMenuLayerBrush(
		TSharedPtr<FSlateBrush>& Brush,
		UT66UITexturePoolSubsystem* TexPool,
		UObject* Requester,
		const TCHAR* ObjectPath,
		const TCHAR* PackagePath,
		const TCHAR* SourceRelativePath,
		FName RequestKey)
	{
		const FString SourcePath = FPaths::ProjectDir() / SourceRelativePath;
		if (FPaths::FileExists(SourcePath))
		{
			Brush = MakeShared<FSlateImageBrush>(SourcePath, MainMenuBackgroundImageSize, FLinearColor::White);
		}
		else if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
			EnsureRuntimeImageBrush(Brush, MainMenuBackgroundImageSize);
		}
		else
		{
			EnsureRuntimeImageBrush(Brush, MainMenuBackgroundImageSize);
		}

		if (!TexPool || !FPackageName::DoesPackageExist(PackagePath))
		{
			return;
		}

		const TSoftObjectPtr<UTexture2D> Soft{ FSoftObjectPath(ObjectPath) };
		if (UTexture2D* LoadedTexture = TexPool->GetLoadedTexture(Soft))
		{
			Brush->SetResourceObject(LoadedTexture);
			Brush->ImageSize = FVector2D(
				FMath::Max(1, LoadedTexture->GetSizeX()),
				FMath::Max(1, LoadedTexture->GetSizeY()));
			return;
		}

		T66SlateTexture::BindSharedBrushAsync(TexPool, Soft, Requester, Brush, RequestKey, false);
	}

	void SetupRuntimeImageBrush(
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& TextureHandle,
		const TCHAR* SourceRelativePath,
		const FVector2D& ImageSize)
	{
		const FString SourcePath = FPaths::ProjectDir() / SourceRelativePath;
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
		}

		EnsureRuntimeImageBrush(Brush, ImageSize);

		if (!FPaths::FileExists(SourcePath))
		{
			Brush->SetResourceObject(nullptr);
			return;
		}

		if (!TextureHandle.IsValid())
		{
			if (UTexture2D* FileTexture = FImageUtils::ImportFileAsTexture2D(SourcePath))
			{
				FileTexture->SRGB = true;
				FileTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
				FileTexture->NeverStream = true;
				FileTexture->Filter = TextureFilter::TF_Trilinear;
				FileTexture->CompressionSettings = TC_EditorIcon;
				FileTexture->UpdateResource();
				TextureHandle.Reset(FileTexture);
			}
		}

		if (TextureHandle.IsValid())
		{
			Brush->SetResourceObject(TextureHandle.Get());
			Brush->ImageSize = FVector2D(
				FMath::Max(1, TextureHandle->GetSizeX()),
				FMath::Max(1, TextureHandle->GetSizeY()));
		}
	}

	FVector2D GetEffectiveFrontendViewportSize()
	{
		return FT66Style::GetViewportSize();
	}
}

UT66MainMenuScreen::UT66MainMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MainMenu;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66MainMenuScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

TSharedRef<SWidget> UT66MainMenuScreen::RebuildWidget()
{
	// This screen already computes its layout from the live safe-frame size.
	// Wrapping it in the shared responsive root shrinks it a second time.
	return BuildSlateUI();
}

TSharedRef<SWidget> UT66MainMenuScreen::BuildSlateUI()
{
	RequestBackgroundTexture();
	CachedViewportSize = GetEffectiveFrontendViewportSize();
	bViewportResponsiveRebuildQueued = false;
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66SaveSubsystem* SaveSubsystem = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	UT66SteamHelper* SteamHelper = GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr;

	const FText NewGameText = Loc ? Loc->GetText_NewGame() : NSLOCTEXT("T66.MainMenu", "NewGame", "NEW GAME");
	const FText LoadGameText = Loc ? Loc->GetText_LoadGame() : NSLOCTEXT("T66.MainMenu", "LoadGame", "LOAD GAME");
	SetupRuntimeImageBrush(
		PrimaryCTAFillBrush,
		PrimaryCTAFillTexture,
		TEXT("SourceAssets/UI/MainMenuGenerated/mainmenu_cta_fill_green.png"),
		FVector2D(1024.f, 232.f));

	enum class ELastRunStatus : uint8
	{
		None,
		InProgress,
		Completed,
		NotCompleted
	};

	struct FLastRunSnapshot
	{
		FName HeroID = NAME_None;
		ET66BodyType HeroBodyType = ET66BodyType::TypeA;
		ET66Difficulty Difficulty = ET66Difficulty::Easy;
		ET66PartySize PartySize = ET66PartySize::Solo;
		int32 StageReached = 1;
		ELastRunStatus Status = ELastRunStatus::None;
		int32 SaveSlotIndex = INDEX_NONE;
		FString RunSummarySlotName;
		FDateTime Timestamp = FDateTime::MinValue();
	};

	struct FMenuFriendEntry
	{
		FString PlayerId;
		FString Name;
		FString Status;
		bool bOnline = false;
		FName HeroID = NAME_None;
		ET66BodyType BodyType = ET66BodyType::TypeA;
	};

	struct FMenuPartyEntry
	{
		FString PlayerId;
		FString DisplayName;
		bool bIsLocal = false;
		bool bIsPartyHost = false;
		bool bOnline = false;
		TSharedPtr<FSlateBrush> AvatarBrush;
	};

	auto MakeAvatarBrush = [](UTexture2D* AvatarTexture, const FVector2D& ImageSize) -> TSharedPtr<FSlateBrush>
	{
		if (!AvatarTexture)
		{
			return nullptr;
		}

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
		Brush->SetResourceObject(AvatarTexture);
		return Brush;
	};

	auto ParseIsoUtc = [](const FString& IsoString) -> FDateTime
	{
		FDateTime Parsed = FDateTime::MinValue();
		if (!IsoString.IsEmpty())
		{
			FDateTime::ParseIso8601(*IsoString, Parsed);
		}
		return Parsed;
	};

	auto IsLastRunSet = [](const FLastRunSnapshot& Snapshot) -> bool
	{
		return Snapshot.Status != ELastRunStatus::None;
	};

	FLastRunSnapshot LatestSaveRun;
	FDateTime LatestSaveUtc = FDateTime::MinValue();
	int32 LatestSaveSlot = INDEX_NONE;
	if (SaveSubsystem)
	{
		for (int32 SlotIndex = 0; SlotIndex < UT66SaveSubsystem::MaxSlots; ++SlotIndex)
		{
			bool bOccupied = false;
			FString SlotUtc;
			FString HeroDisplayName;
			FString MapName;
			if (SaveSubsystem->GetSlotMeta(SlotIndex, bOccupied, SlotUtc, HeroDisplayName, MapName) && bOccupied)
			{
				const FDateTime ParsedUtc = ParseIsoUtc(SlotUtc);
				if (LatestSaveSlot == INDEX_NONE || ParsedUtc > LatestSaveUtc)
				{
					LatestSaveSlot = SlotIndex;
					LatestSaveUtc = ParsedUtc;
				}
			}
		}
	}

	if (LatestSaveSlot != INDEX_NONE)
	{
		if (UT66RunSaveGame* LatestSave = SaveSubsystem ? SaveSubsystem->LoadFromSlot(LatestSaveSlot) : nullptr)
		{
			LatestSaveRun.HeroID = LatestSave->HeroID;
			LatestSaveRun.HeroBodyType = LatestSave->HeroBodyType;
			LatestSaveRun.Difficulty = LatestSave->Difficulty;
			LatestSaveRun.PartySize = LatestSave->PartySize;
			LatestSaveRun.StageReached = LatestSave->StageReached;
			LatestSaveRun.Status = ELastRunStatus::InProgress;
			LatestSaveRun.SaveSlotIndex = LatestSaveSlot;
			LatestSaveRun.Timestamp = LatestSaveUtc;
		}
	}

	FLastRunSnapshot LatestFinishedRun;
	if (LB)
	{
		const TArray<FT66RecentRunRecord> RecentRuns = LB->GetRecentRuns();
		for (const FT66RecentRunRecord& Run : RecentRuns)
		{
			if (!IsLastRunSet(LatestFinishedRun) || Run.EndedAtUtc > LatestFinishedRun.Timestamp)
			{
				LatestFinishedRun.HeroID = Run.HeroID;
				LatestFinishedRun.Difficulty = Run.Difficulty;
				LatestFinishedRun.PartySize = Run.PartySize;
				LatestFinishedRun.StageReached = Run.StageReached;
				LatestFinishedRun.Status = Run.bWasFullClear ? ELastRunStatus::Completed : ELastRunStatus::NotCompleted;
				LatestFinishedRun.RunSummarySlotName = Run.RunSummarySlotName;
				LatestFinishedRun.Timestamp = Run.EndedAtUtc;
			}
		}
	}

	FLastRunSnapshot LastRun;
	if (IsLastRunSet(LatestSaveRun) && IsLastRunSet(LatestFinishedRun))
	{
		LastRun = (LatestSaveRun.Timestamp >= LatestFinishedRun.Timestamp) ? LatestSaveRun : LatestFinishedRun;
	}
	else if (IsLastRunSet(LatestSaveRun))
	{
		LastRun = LatestSaveRun;
	}
	else if (IsLastRunSet(LatestFinishedRun))
	{
		LastRun = LatestFinishedRun;
	}

	if ((LastRun.Status == ELastRunStatus::Completed || LastRun.Status == ELastRunStatus::NotCompleted)
		&& !LastRun.RunSummarySlotName.IsEmpty())
	{
		if (UT66LeaderboardRunSummarySaveGame* LatestSummary = Cast<UT66LeaderboardRunSummarySaveGame>(UGameplayStatics::LoadGameFromSlot(LastRun.RunSummarySlotName, 0)))
		{
			LastRun.HeroBodyType = LatestSummary->HeroBodyType;
		}
	}

	TArray<FName> AllHeroIDs = T66GI ? T66GI->GetAllHeroIDs() : TArray<FName>();
	const FName FallbackHeroID = AllHeroIDs.Num() > 0 ? AllHeroIDs[0] : NAME_None;

	auto GetPartySizeText = [Loc](ET66PartySize PartySize) -> FText
	{
		if (Loc)
		{
			switch (PartySize)
			{
			case ET66PartySize::Duo: return Loc->GetText_Duo();
			case ET66PartySize::Trio: return Loc->GetText_Trio();
			case ET66PartySize::Quad: return Loc->GetText_Quad();
			case ET66PartySize::Solo:
			default: return Loc->GetText_Solo();
			}
		}

		switch (PartySize)
		{
		case ET66PartySize::Duo: return NSLOCTEXT("T66.PartySize", "Duo", "Duo");
		case ET66PartySize::Trio: return NSLOCTEXT("T66.PartySize", "Trio", "Trio");
		case ET66PartySize::Quad: return NSLOCTEXT("T66.PartySize", "Quad", "Quad");
		case ET66PartySize::Solo:
		default: return NSLOCTEXT("T66.PartySize", "Solo", "Solo");
		}
	};

	auto ResolveHeroName = [Loc, T66GI](FName HeroID) -> FText
	{
		if (!HeroID.IsNone() && Loc)
		{
			return Loc->GetText_HeroName(HeroID);
		}

		FHeroData HeroData;
		if (T66GI && T66GI->GetHeroData(HeroID, HeroData))
		{
			return HeroData.DisplayName;
		}
		return HeroID.IsNone() ? NSLOCTEXT("T66.MainMenu", "UnknownHero", "Unknown Hero") : FText::FromName(HeroID);
	};

	auto ResolvePortraitSoft = [T66GI](FName HeroID, ET66BodyType BodyType, ET66HeroPortraitVariant Variant) -> TSoftObjectPtr<UTexture2D>
	{
		return T66GI ? T66GI->ResolveHeroPortrait(HeroID, BodyType, Variant) : TSoftObjectPtr<UTexture2D>();
	};

	auto PickHeroID = [&AllHeroIDs, FallbackHeroID](int32 Index) -> FName
	{
		if (AllHeroIDs.IsValidIndex(Index) && !AllHeroIDs[Index].IsNone())
		{
			return AllHeroIDs[Index];
		}
		return FallbackHeroID;
	};

	TArray<FMenuFriendEntry> Friends;
	TArray<FMenuPartyEntry> PartyEntries;
	if (PartySubsystem)
	{
		const TArray<FT66PartyFriendEntry>& PartyFriends = PartySubsystem->GetFriends();
		for (int32 FriendIndex = 0; FriendIndex < PartyFriends.Num(); ++FriendIndex)
		{
			const FT66PartyFriendEntry& Friend = PartyFriends[FriendIndex];
			Friends.Add({
				Friend.PlayerId,
				Friend.DisplayName,
				Friend.PresenceText,
				Friend.bOnline,
				PickHeroID(FriendIndex),
				(FriendIndex % 2 == 0) ? ET66BodyType::TypeA : ET66BodyType::TypeB
			});
		}

		const TArray<FT66PartyMemberEntry>& PartyMembers = PartySubsystem->GetPartyMembers();
		PartyEntries.Reserve(PartyMembers.Num());
		for (const FT66PartyMemberEntry& Member : PartyMembers)
		{
			UTexture2D* AvatarTexture = SteamHelper ? SteamHelper->GetAvatarTextureForSteamId(Member.PlayerId) : nullptr;
			PartyEntries.Add({
				Member.PlayerId,
				Member.DisplayName,
				Member.bIsLocal,
				Member.bIsPartyHost,
				Member.bOnline,
				MakeAvatarBrush(AvatarTexture, FVector2D(60.f, 60.f))
			});
		}
	}

	TArray<FSoftObjectPath> PortraitPaths;
	const TSoftObjectPtr<UTexture2D> LastRunPortraitSoft = ResolvePortraitSoft(LastRun.HeroID, LastRun.HeroBodyType, ET66HeroPortraitVariant::Half);
	if (!LastRunPortraitSoft.IsNull())
	{
		PortraitPaths.AddUnique(LastRunPortraitSoft.ToSoftObjectPath());
	}

	for (const FMenuFriendEntry& Friend : Friends)
	{
		const TSoftObjectPtr<UTexture2D> FriendPortraitSoft = ResolvePortraitSoft(Friend.HeroID, Friend.BodyType, ET66HeroPortraitVariant::Low);
		if (!FriendPortraitSoft.IsNull())
		{
			PortraitPaths.AddUnique(FriendPortraitSoft.ToSoftObjectPath());
		}
	}

	if (TexPool && PortraitPaths.Num() > 0)
	{
		TexPool->EnsureTexturesLoadedSync(PortraitPaths);
	}

	LastRunHeroBrush = MakeShared<FSlateBrush>();
	LastRunHeroBrush->DrawAs = ESlateBrushDrawType::Image;
	LastRunHeroBrush->Tiling = ESlateBrushTileType::NoTile;
	LastRunHeroBrush->ImageSize = FVector2D(76.f, 76.f);
	LastRunHeroBrush->SetResourceObject(TexPool ? T66SlateTexture::GetLoaded(TexPool, LastRunPortraitSoft) : nullptr);

	FriendPortraitBrushes.Reset();
	for (const FMenuFriendEntry& Friend : Friends)
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = FVector2D(40.f, 40.f);
		if (UTexture2D* SteamAvatarTexture = SteamHelper ? SteamHelper->GetAvatarTextureForSteamId(Friend.PlayerId) : nullptr)
		{
			Brush->SetResourceObject(SteamAvatarTexture);
		}
		else
		{
			const TSoftObjectPtr<UTexture2D> FriendPortraitSoft = ResolvePortraitSoft(Friend.HeroID, Friend.BodyType, ET66HeroPortraitVariant::Low);
			Brush->SetResourceObject(TexPool ? T66SlateTexture::GetLoaded(TexPool, FriendPortraitSoft) : nullptr);
		}
		FriendPortraitBrushes.Add(Brush);
	}

	const FLinearColor ShellFill(0.004f, 0.005f, 0.010f, 0.985f);
	const FLinearColor BrightText(0.86f, 0.87f, 0.89f, 1.0f);
	const FLinearColor HeaderText(0.66f, 0.68f, 0.71f, 1.0f);
	const FLinearColor MutedText(0.50f, 0.52f, 0.56f, 1.0f);
	const FLinearColor OfflineNameText(0.40f, 0.41f, 0.45f, 1.0f);
	const FLinearColor OnlineHeaderText(0.42f, 0.67f, 0.96f, 1.0f);
	const FLinearColor OnlineStatusText(0.34f, 0.61f, 0.90f, 1.0f);
	const FLinearColor DividerColor(0.20f, 0.22f, 0.25f, 0.48f);
	const FLinearColor AvatarAccentOnline(0.13f, 0.22f, 0.30f, 1.0f);
	const FLinearColor AvatarAccentOffline(0.08f, 0.09f, 0.11f, 1.0f);
	const FLinearColor LeaderSlotAccent(0.29f, 0.24f, 0.13f, 1.0f);
	const FLinearColor PartySlotAccent(0.15f, 0.17f, 0.19f, 1.0f);
	const float TopInset = UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f;
	const float TopReservedInset = UIManager ? UIManager->GetFrontendTopBarReservedHeight() : TopInset;
	const FVector2D ViewportLogicalSize = FT66Style::GetViewportLogicalSize();
	const float HorizontalFramePadding = 20.f;
	const float BottomFramePadding = 16.f;
	const FVector2D ReferenceViewportSize(FT66Style::Tokens::ReferenceLayoutWidth, FT66Style::Tokens::ReferenceLayoutHeight);
	const float ReferenceTopInset = 107.f;
	const float ReferenceContentWidthBudget = ReferenceViewportSize.X - (HorizontalFramePadding * 2.f);
	const float AvailableColumnHeight = FMath::Max(1.f, ViewportLogicalSize.Y - TopInset - BottomFramePadding);
	const float ReferenceAvailableColumnHeight = FMath::Max(1.f, ReferenceViewportSize.Y - ReferenceTopInset - BottomFramePadding);
	const float AvailableContentWidth = FMath::Max(1.f, ViewportLogicalSize.X - (HorizontalFramePadding * 2.f));
	const float WidthResponsiveScale = AvailableContentWidth / FMath::Max(ReferenceContentWidthBudget, 1.f);
	const float HeightResponsiveScale = AvailableColumnHeight / FMath::Max(ReferenceAvailableColumnHeight, 1.f);
	const float LayoutResponsiveScale = FMath::Clamp(
		FMath::Sqrt(WidthResponsiveScale * HeightResponsiveScale),
		0.85f,
		1.35f);
	const float WidthLayoutAlpha = FMath::Clamp((AvailableContentWidth - ReferenceContentWidthBudget) / 640.f, 0.f, 1.f);
	auto LerpDimension = [WidthLayoutAlpha](float CompactValue, float FullValue) -> float
	{
		return FMath::Lerp(CompactValue, FullValue, WidthLayoutAlpha);
	};
	auto LerpInt = [WidthLayoutAlpha](int32 CompactValue, int32 FullValue) -> int32
	{
		return FMath::RoundToInt(FMath::Lerp(static_cast<float>(CompactValue), static_cast<float>(FullValue), WidthLayoutAlpha));
	};

	float CenterColumnWidth = LerpDimension(304.f, 372.f);
	float ResolvedSidePanelWidth = LerpDimension(298.f, 342.f);
	float ResolvedLeaderboardPanelWidth = ResolvedSidePanelWidth;
	const float ColumnGap = LerpDimension(18.f, 28.f);

	float WidthBudgetShortfall = FMath::Max(
		0.f,
		((ResolvedSidePanelWidth * 2.f) + CenterColumnWidth + (ColumnGap * 2.f)) - AvailableContentWidth);
	auto ConsumeWidthBudget = [&WidthBudgetShortfall](float& Value, float MinValue)
	{
		if (WidthBudgetShortfall <= 0.f)
		{
			return;
		}

		const float Reduction = FMath::Min(WidthBudgetShortfall, FMath::Max(0.f, Value - MinValue));
		Value -= Reduction;
		WidthBudgetShortfall -= Reduction;
	};

	ConsumeWidthBudget(CenterColumnWidth, 292.f);
	if (WidthBudgetShortfall > 0.f)
	{
		const float MirroredSideReduction = FMath::Min(
			WidthBudgetShortfall * 0.5f,
			FMath::Max(0.f, ResolvedSidePanelWidth - 252.f));
		ResolvedSidePanelWidth -= MirroredSideReduction;
		WidthBudgetShortfall -= MirroredSideReduction * 2.f;
	}
	ResolvedLeaderboardPanelWidth = ResolvedSidePanelWidth;

	const float ColumnTargetHeight = FMath::Min(AvailableColumnHeight, LerpDimension(500.f, 540.f) * LayoutResponsiveScale);
	const float SidePanelTargetHeight = FMath::Min(AvailableColumnHeight, LerpDimension(448.f, 508.f));
	const float LeaderboardChromeHeight = LerpDimension(52.f, 58.f);
	const float LeaderboardShellTargetHeight = SidePanelTargetHeight + LeaderboardChromeHeight;
	const float CenterButtonWidth = CenterColumnWidth;
	const float CenterButtonHeight = LerpDimension(68.f, 80.f);
	const float CenterButtonGap = LerpDimension(10.f, 14.f);
	const float CenterColumnHeight = (CenterButtonHeight * 2.f) + CenterButtonGap;
	const float CenterButtonLift = FMath::Max(LerpDimension(38.f, 52.f), (ColumnTargetHeight - CenterColumnHeight) * 0.24f);
	const float SceneVerticalOffset = LerpDimension(96.f, 128.f);
	const float SceneTitleTopPadding = TopReservedInset + LerpDimension(8.f, 12.f);
	const int32 LeftPanelBodyFontSize = LerpInt(10, 11);
	const int32 LeftPanelTitleFontSize = LerpInt(10, 11);
	const int32 CenterButtonFontSize = LerpInt(24, 28);
	const int32 SceneTitleFontSize = LerpInt(42, 52);

	auto MakeRadianceFont = [](int32 Size, int32 LetterSpacing = 0) -> FSlateFontInfo
	{
		FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Regular"), Size);
		Font.LetterSpacing = LetterSpacing;
		return Font;
	};

	auto MakeSectionTitle = [HeaderText, LeftPanelTitleFontSize](const FText& Text) -> TSharedRef<SWidget>
	{
		FSlateFontInfo HeaderFont = FT66Style::MakeFont(TEXT("Bold"), LeftPanelTitleFontSize + 2);
		HeaderFont.LetterSpacing = 112;

		return SNew(STextBlock)
			.Text(Text)
			.Font(HeaderFont)
			.ColorAndOpacity(HeaderText);
	};

	auto MakeSubtleDivider = [DividerColor]() -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HeightOverride(1.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(DividerColor)
			];
	};

	auto MakeFriendGroupHeader = [this, HeaderText, LeftPanelTitleFontSize](const FText& Label, int32 Count, const FLinearColor& CountColor, bool bOnlineGroup) -> TSharedRef<SWidget>
	{
		FSlateFontInfo GroupFont = FT66Style::MakeFont(TEXT("Bold"), LeftPanelTitleFontSize + 2);
		GroupFont.LetterSpacing = 96;
		const bool bExpanded = bOnlineGroup ? bShowOnlineFriends : bShowOfflineFriends;
		const FSlateBrush* DownArrowBrush = &FCoreStyle::Get().GetWidgetStyle<FComboButtonStyle>("ComboButton").DownArrowImage;

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.ContentPadding(FMargin(0.f))
				.OnClicked(FOnClicked::CreateLambda([this, bOnlineGroup]()
				{
					if (bOnlineGroup)
					{
						bShowOnlineFriends = !bShowOnlineFriends;
					}
					else
					{
						bShowOfflineFriends = !bShowOfflineFriends;
					}

					ForceRebuildSlate();
					return FReply::Handled();
				}))
				[
					SNew(SBox)
					.WidthOverride(14.f)
					.HeightOverride(14.f)
					[
						SNew(SImage)
						.Image(DownArrowBrush)
						.ColorAndOpacity(HeaderText)
						.RenderTransformPivot(FVector2D(0.5f, 0.5f))
						.RenderTransform(
							bExpanded
								? FSlateRenderTransform(FTransform2D())
								: FSlateRenderTransform(FTransform2D(FQuat2D(FMath::DegreesToRadians(90.f)))))
					]
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(GroupFont)
				.ColorAndOpacity(HeaderText)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::Format(NSLOCTEXT("T66.MainMenu", "FriendsGroupCount", "({0})"), FText::AsNumber(Count)))
				.Font(GroupFont)
				.ColorAndOpacity(CountColor)
			];
	};

	const FText InviteFriendText = NSLOCTEXT("T66.MainMenu", "InviteFriend", "INVITE");
	const FText InPartyText = NSLOCTEXT("T66.MainMenu", "InParty", "In Party");
	const FText PartyFullText = NSLOCTEXT("T66.MainMenu", "PartyFull", "PARTY FULL");
	const FText OfflineActionText = NSLOCTEXT("T66.MainMenu", "FriendOffline", "OFFLINE");
	const FText LeavePartyButtonText = NSLOCTEXT("T66.MainMenu", "LeavePartyButton", "X");
	const FButtonStyle& NoBorderButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");

	auto MakeFlatActionButton = [LeftPanelBodyFontSize, &NoBorderButtonStyle](const FText& Text, const FOnClicked& OnClicked, bool bEnabled, bool bPrimary) -> TSharedRef<SWidget>
	{
		const FLinearColor FillColor = !bEnabled
			? FLinearColor(0.42f, 0.47f, 0.40f, 0.72f)
			: (bPrimary ? FLinearColor(0.18f, 0.31f, 0.18f, 0.97f) : FLinearColor(0.27f, 0.30f, 0.34f, 0.94f));
		const FLinearColor TextColor = bEnabled
			? FLinearColor(0.96f, 0.97f, 0.94f, 1.0f)
			: FLinearColor(0.88f, 0.90f, 0.84f, 0.72f);

		return SNew(SBox)
			.WidthOverride(80.f)
			.HeightOverride(30.f)
			[
				SNew(SButton)
				.ButtonStyle(&NoBorderButtonStyle)
				.ContentPadding(FMargin(0.f))
				.IsEnabled(bEnabled)
				.OnClicked(OnClicked)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FillColor)
					.Padding(FMargin(8.f, 5.f))
					[
						SNew(STextBlock)
						.Text(Text)
						.Font(FT66Style::MakeFont(TEXT("Regular"), LeftPanelBodyFontSize))
						.ColorAndOpacity(TextColor)
						.Justification(ETextJustify::Center)
					]
				]
			];
	};

	auto MakeFriendRow = [&, BrightText, MutedText, OfflineNameText, OnlineStatusText, AvatarAccentOnline, AvatarAccentOffline](
		const FMenuFriendEntry& Friend,
		int32 FriendIndex) -> TSharedRef<SWidget>
	{
		const FLinearColor NameColor = Friend.bOnline ? BrightText : OfflineNameText;
		const FLinearColor StatusColor = Friend.bOnline ? OnlineStatusText : MutedText;
		const FLinearColor AccentColor = Friend.bOnline ? AvatarAccentOnline : AvatarAccentOffline;
		const bool bInParty = PartySubsystem && PartySubsystem->IsFriendInParty(Friend.PlayerId);
		const bool bCanInvite = PartySubsystem && Friend.bOnline && !bInParty && PartySubsystem->GetPartyMemberCount() < 4;
		const FText ActionText = bInParty
			? InPartyText
			: (!Friend.bOnline
				? OfflineActionText
				: ((PartySubsystem && PartySubsystem->GetPartyMemberCount() >= 4) ? PartyFullText : InviteFriendText));

		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(bInParty ? FLinearColor(0.10f, 0.14f, 0.18f, 0.55f) : FLinearColor(0.f, 0.f, 0.f, 0.20f))
			.Padding(6.f, 4.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(44.f)
					.HeightOverride(44.f)
					[
						FT66Style::MakeSlotFrame(
							SNew(SImage)
							.Image(FriendPortraitBrushes.IsValidIndex(FriendIndex) ? FriendPortraitBrushes[FriendIndex].Get() : nullptr)
							.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Friend.bOnline ? 1.0f : 0.74f)),
							AccentColor,
							FMargin(1.f))
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(8.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Friend.Name))
						.Font(FT66Style::MakeFont(TEXT("Regular"), LeftPanelBodyFontSize))
						.ColorAndOpacity(NameColor)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(bInParty ? InPartyText : FText::FromString(Friend.Status))
						.Font(FT66Style::MakeFont(TEXT("Regular"), LeftPanelBodyFontSize))
						.ColorAndOpacity(StatusColor)
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
				[
					MakeFlatActionButton(
						ActionText,
						FOnClicked::CreateLambda([this, PartySubsystem, PlayerId = Friend.PlayerId]()
						{
							if (!PartySubsystem)
							{
								return FReply::Handled();
							}

							PartySubsystem->InviteFriend(PlayerId);

							ForceRebuildSlate();
							return FReply::Handled();
						}),
						bCanInvite,
						!bInParty && Friend.bOnline)
				]
			];
	};

	auto MakeMenuButton = [this, CenterButtonWidth, CenterButtonHeight, CenterButtonFontSize](
		const FText& Text,
		FReply (UT66MainMenuScreen::*ClickFunc)(),
		ET66ButtonType ButtonType = ET66ButtonType::Neutral,
		bool bUseDotaPlateOverlay = false,
		bool bUseAnimatedGlow = false) -> TSharedRef<SWidget>
	{
		FT66ButtonParams MenuButtonParams(Text, FOnClicked::CreateUObject(this, ClickFunc), ButtonType);
		MenuButtonParams
			.SetMinWidth(CenterButtonWidth)
			.SetHeight(CenterButtonHeight)
			.SetFontSize(CenterButtonFontSize)
			.SetPadding(FMargin(16.f, 10.f, 16.f, 8.f))
			.SetUseGlow(false)
			.SetUseDotaPlateOverlay(bUseDotaPlateOverlay)
			.SetDotaPlateOverrideBrush(PrimaryCTAFillBrush.Get())
			.SetTextColor(FLinearColor(0.96f, 0.96f, 0.94f, 1.0f))
			.SetStateTextShadowColors(
				FLinearColor(0.f, 0.f, 0.f, 0.36f),
				FLinearColor(0.f, 0.f, 0.f, 0.42f),
				FLinearColor(0.f, 0.f, 0.f, 0.28f))
			.SetTextShadowOffset(FVector2D(0.f, 1.f));

		TSharedRef<SWidget> ButtonWidget = FT66Style::MakeButton(MenuButtonParams);
		if (bUseAnimatedGlow)
		{
			ButtonWidget = SNew(ST66AnimatedBorderGlow)
				.GlowColor(FLinearColor(0.32f, 0.83f, 0.56f, 1.0f))
				.SweepColor(FLinearColor(0.78f, 1.00f, 0.87f, 1.0f))
				.BorderInset(2.0f)
				.InnerThickness(2.0f)
				.MidThickness(5.0f)
				.OuterThickness(9.0f)
				.SweepLengthFraction(0.26f)
				.SweepSpeed(0.12f)
				[
					ButtonWidget
				];
		}

		return SNew(SBox)
			.WidthOverride(CenterButtonWidth)
			[
				ButtonWidget
			];
	};

	const TSharedRef<SWidget> LeaderboardWidget =
		SAssignNew(LeaderboardPanel, ST66LeaderboardPanel)
		.LocalizationSubsystem(Loc)
		.LeaderboardSubsystem(LB)
		.UIManager(UIManager);

	const TSharedRef<SWidget> LeaderboardShell = LeaderboardWidget;

	const TSharedRef<SWidget> CenterMenuButtons =
		SNew(SBox)
		.WidthOverride(CenterColumnWidth)
		.HeightOverride(CenterColumnHeight)
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeMenuButton(NewGameText, &UT66MainMenuScreen::HandleNewGameClicked, ET66ButtonType::Success, true, true)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, CenterButtonGap, 0.f, 0.f)
				[
					MakeMenuButton(LoadGameText, &UT66MainMenuScreen::HandleLoadGameClicked, ET66ButtonType::Success, true, false)
				]
		];

	const int32 OnlineCount = Algo::CountIf(Friends, [](const FMenuFriendEntry& Entry) { return Entry.bOnline; });
	const int32 OfflineCount = Friends.Num() - OnlineCount;

	const TSharedRef<SVerticalBox> FriendsList = SNew(SVerticalBox);
	auto AddFriendGroup = [&](bool bOnlineGroup)
	{
		FriendsList->AddSlot().AutoHeight()
		[
			MakeFriendGroupHeader(
				bOnlineGroup
					? NSLOCTEXT("T66.MainMenu", "OnlineFriendsHeader", "ONLINE")
					: NSLOCTEXT("T66.MainMenu", "OfflineFriendsHeader", "OFFLINE"),
				bOnlineGroup ? OnlineCount : OfflineCount,
				bOnlineGroup ? OnlineHeaderText : HeaderText,
				bOnlineGroup)
		];

		const bool bExpanded = bOnlineGroup ? bShowOnlineFriends : bShowOfflineFriends;
		if (!bExpanded)
		{
			return;
		}

		for (int32 FriendIndex = 0; FriendIndex < Friends.Num(); ++FriendIndex)
		{
			const FMenuFriendEntry& Friend = Friends[FriendIndex];
			if (Friend.bOnline != bOnlineGroup)
			{
				continue;
			}

			FriendsList->AddSlot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				MakeFriendRow(Friend, FriendIndex)
			];
		}
	};

	AddFriendGroup(true);
	FriendsList->AddSlot().AutoHeight().Padding(0.f, 20.f, 0.f, 0.f)[SNew(SSpacer).Size(FVector2D(1.f, 1.f))];
	AddFriendGroup(false);

	const bool bCanLeaveParty = SessionSubsystem
		&& (SessionSubsystem->IsPartySessionActive() || (PartySubsystem && PartySubsystem->HasRemotePartyMembers()));

	auto MakePartyMemberSlot = [LeaderSlotAccent, PartySlotAccent](const FMenuPartyEntry* Member) -> TSharedRef<SWidget>
	{
		const bool bOccupied = Member != nullptr;
		const bool bHost = bOccupied && Member->bIsPartyHost;
		const FLinearColor SlotAccent = !bOccupied
			? FLinearColor(0.12f, 0.13f, 0.15f, 0.95f)
			: (bHost ? LeaderSlotAccent : PartySlotAccent);
		const FLinearColor FillColor = bOccupied
			? FLinearColor(0.04f, 0.05f, 0.07f, 1.0f)
			: FLinearColor(0.06f, 0.07f, 0.09f, 0.88f);
		const FText TooltipText = bOccupied
			? FText::FromString(Member->DisplayName)
			: FText::GetEmpty();

		return SNew(SBox)
			.WidthOverride(64.f)
			.HeightOverride(64.f)
			[
				SNew(SBorder)
				.ToolTipText(TooltipText)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
				.BorderBackgroundColor(FLinearColor::Transparent)
				.Padding(0.f)
				[
					FT66Style::MakeSlotFrame(
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FillColor)
						.Padding(2.f)
						[
							SNew(SImage)
							.Image((bOccupied && Member->AvatarBrush.IsValid()) ? Member->AvatarBrush.Get() : nullptr)
							.ColorAndOpacity(bOccupied ? FLinearColor::White : FLinearColor(1.f, 1.f, 1.f, 0.f))
						],
						SlotAccent,
						FMargin(1.f))
				]
			];
	};

	constexpr int32 MaxPartySlots = 4;
	TSharedRef<SHorizontalBox> PartySlots = SNew(SHorizontalBox);
	for (int32 SlotIndex = 0; SlotIndex < MaxPartySlots; ++SlotIndex)
	{
		PartySlots->AddSlot()
		.AutoWidth()
		.Padding(SlotIndex == 0 ? FMargin(0.f) : FMargin(8.f, 0.f, 0.f, 0.f))
		[
			MakePartyMemberSlot(PartyEntries.IsValidIndex(SlotIndex) ? &PartyEntries[SlotIndex] : nullptr)
		];
	}

	const FLinearColor LastRunInProgressText(0.86f, 0.73f, 0.42f, 1.0f);
	const FLinearColor LastRunCompletedText(0.44f, 0.80f, 0.43f, 1.0f);
	const FLinearColor LastRunFailedText(0.84f, 0.31f, 0.25f, 1.0f);

	auto GetLastRunStatusText = [](ELastRunStatus Status) -> FText
	{
		switch (Status)
		{
		case ELastRunStatus::InProgress:
			return NSLOCTEXT("T66.MainMenu", "LastRunStatusInProgress", "In Progress");
		case ELastRunStatus::Completed:
			return NSLOCTEXT("T66.MainMenu", "LastRunStatusCompleted", "Completed");
		case ELastRunStatus::NotCompleted:
			return NSLOCTEXT("T66.MainMenu", "LastRunStatusNotCompleted", "Not Completed");
		case ELastRunStatus::None:
		default:
			return NSLOCTEXT("T66.MainMenu", "LastRunStatusNone", "No Run Data");
		}
	};

	auto GetLastRunStatusColor = [&](ELastRunStatus Status) -> FLinearColor
	{
		switch (Status)
		{
		case ELastRunStatus::InProgress:
			return LastRunInProgressText;
		case ELastRunStatus::Completed:
			return LastRunCompletedText;
		case ELastRunStatus::NotCompleted:
			return LastRunFailedText;
		case ELastRunStatus::None:
		default:
			return MutedText;
		}
	};

	const bool bHasLastRunData = IsLastRunSet(LastRun);
	const bool bCanResumeLastRun = LastRun.Status == ELastRunStatus::InProgress && LastRun.SaveSlotIndex != INDEX_NONE;
	const bool bCanViewLastRunSummary = (LastRun.Status == ELastRunStatus::Completed || LastRun.Status == ELastRunStatus::NotCompleted)
		&& !LastRun.RunSummarySlotName.IsEmpty();
	const bool bCanActivateLastRun = bCanResumeLastRun || bCanViewLastRunSummary;

	const FText LastRunHeroText = bHasLastRunData
		? ResolveHeroName(LastRun.HeroID)
		: NSLOCTEXT("T66.MainMenu", "LastRunEmptyTitle", "No runs recorded yet");
	const FText LastRunStatusText = bHasLastRunData
		? GetLastRunStatusText(LastRun.Status)
		: NSLOCTEXT("T66.MainMenu", "LastRunEmptyBody", "Start, save, or finish a run to populate this panel.");
	const FText LastRunStageText = bHasLastRunData
		? FText::Format(NSLOCTEXT("T66.MainMenu", "LastRunStage", "Stage {0}"), FText::AsNumber(LastRun.StageReached))
		: FText::GetEmpty();
	const FText LastRunMetaText = bHasLastRunData
		? FText::Format(
			NSLOCTEXT("T66.MainMenu", "LastRunMeta", "{0}  |  {1}"),
			Loc ? Loc->GetText_Difficulty(LastRun.Difficulty) : NSLOCTEXT("T66.Difficulty", "Easy", "Easy"),
			GetPartySizeText(LastRun.PartySize))
		: FText::GetEmpty();

	const TSharedRef<SWidget> LastRunRowContent =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(60.f)
			.HeightOverride(60.f)
			[
				FT66Style::MakeSlotFrame(
					SNew(SImage)
					.Image(LastRunHeroBrush.Get()),
					LeaderSlotAccent,
					FMargin(1.f))
			]
		]
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(12.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(LastRunHeroText)
				.Font(FT66Style::MakeFont(TEXT("Regular"), LeftPanelBodyFontSize))
				.ColorAndOpacity(BrightText)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(LastRunStatusText)
				.Font(FT66Style::MakeFont(TEXT("Regular"), LeftPanelBodyFontSize))
				.ColorAndOpacity(GetLastRunStatusColor(LastRun.Status))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(LastRunStageText)
				.Font(FT66Style::MakeFont(TEXT("Regular"), LeftPanelBodyFontSize))
				.ColorAndOpacity(MutedText)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(LastRunMetaText)
				.Font(FT66Style::MakeFont(TEXT("Regular"), LeftPanelBodyFontSize))
				.ColorAndOpacity(MutedText)
				.Visibility(bHasLastRunData ? EVisibility::Visible : EVisibility::Collapsed)
			]
		];

	const TSharedRef<SWidget> LastRunRow =
		bCanActivateLastRun
		? StaticCastSharedRef<SWidget>(
			SNew(SButton)
			.ButtonStyle(&NoBorderButtonStyle)
			.ContentPadding(FMargin(0.f))
			.ToolTipText(
				bCanResumeLastRun
					? NSLOCTEXT("T66.MainMenu", "ResumeLastRunTooltip", "Resume this saved run")
					: NSLOCTEXT("T66.MainMenu", "OpenLastRunTooltip", "Open this run summary"))
			.OnClicked(FOnClicked::CreateLambda([this, LB, SaveSubsystem, PartySubsystem, SessionSubsystem, SlotIndex = LastRun.SaveSlotIndex, RunSummarySlotName = LastRun.RunSummarySlotName, Status = LastRun.Status]()
			{
				if (Status == ELastRunStatus::InProgress)
				{
					UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
					if (!GI || !SaveSubsystem || SlotIndex == INDEX_NONE)
					{
						return FReply::Handled();
					}

					UT66RunSaveGame* Loaded = SaveSubsystem->LoadFromSlot(SlotIndex);
					if (!Loaded)
					{
						return FReply::Handled();
					}

					const bool bIsPartyResumeFlow = PartySubsystem && SessionSubsystem
						&& SessionSubsystem->IsPartySessionActive()
						&& PartySubsystem->HasRemotePartyMembers();
					if (bIsPartyResumeFlow)
					{
						if (!SessionSubsystem->IsHostingPartySession())
						{
							return FReply::Handled();
						}

						if (SessionSubsystem->StartLoadedGameplayTravel(Loaded, SlotIndex))
						{
							if (UIManager)
							{
								UIManager->HideAllUI();
							}
							return FReply::Handled();
						}
					}

					GI->SelectedHeroID = Loaded->HeroID;
					GI->SelectedHeroBodyType = Loaded->HeroBodyType;
					GI->SelectedCompanionID = Loaded->CompanionID;
					GI->SelectedDifficulty = Loaded->Difficulty;
					GI->SelectedPartySize = Loaded->PartySize;
					GI->RunSeed = Loaded->RunSeed;
					GI->PendingLoadedTransform = Loaded->PlayerTransform;
					GI->bApplyLoadedTransform = true;
					GI->bLoadAsPreview = false;
					GI->PendingLoadedRunSnapshot = Loaded->RunSnapshot;
					GI->bApplyLoadedRunSnapshot = Loaded->RunSnapshot.bValid;
					GI->CurrentSaveSlotIndex = SlotIndex;
					GI->bRunIneligibleForLeaderboard = Loaded->bRunIneligibleForLeaderboard;
					GI->CurrentRunOwnerPlayerId = Loaded->OwnerPlayerId;
					GI->CurrentRunOwnerDisplayName = Loaded->OwnerDisplayName;
					GI->CurrentRunPartyMemberIds = Loaded->PartyMemberIds;
					GI->CurrentRunPartyMemberDisplayNames = Loaded->PartyMemberDisplayNames;

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

					return FReply::Handled();
				}

				if (LB && LB->RequestOpenRunSummarySlot(RunSummarySlotName, ET66ScreenType::None))
				{
					ShowModal(ET66ScreenType::RunSummary);
				}

				return FReply::Handled();
			}))
			[
				LastRunRowContent
			])
		: LastRunRowContent;

	const TSharedRef<SWidget> LeftSocialShell =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(ShellFill)
		.Padding(FMargin(12.f, 12.f, 12.f, 10.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SScrollBox)
				.ScrollBarVisibility(EVisibility::Visible)
				+ SScrollBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeSectionTitle(NSLOCTEXT("T66.MainMenu", "LastRunSection", "LAST RUN"))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
					[
						LastRunRow
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
					[
						MakeSubtleDivider()
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
					[
						FriendsList
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 14.f, 0.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						MakeSectionTitle(NSLOCTEXT("T66.MainMenu", "PartySection", "PARTY"))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(30.f)
						.HeightOverride(30.f)
						[
							SNew(SButton)
							.ButtonStyle(&NoBorderButtonStyle)
							.ContentPadding(FMargin(0.f))
							.IsEnabled(bCanLeaveParty)
							.ToolTipText(NSLOCTEXT("T66.MainMenu", "LeavePartyTooltip", "Leave party"))
							.OnClicked(FOnClicked::CreateUObject(this, &UT66MainMenuScreen::HandleLeavePartyClicked))
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(bCanLeaveParty ? FLinearColor(0.70f, 0.16f, 0.16f, 0.95f) : FLinearColor(0.32f, 0.19f, 0.19f, 0.55f))
								.Padding(FMargin(0.f))
								[
									SNew(STextBlock)
									.Text(LeavePartyButtonText)
									.Font(FT66Style::MakeFont(TEXT("Bold"), LeftPanelBodyFontSize + 1))
									.ColorAndOpacity(FLinearColor(0.96f, 0.96f, 0.96f, 1.0f))
									.Justification(ETextJustify::Center)
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
				[
					PartySlots
				]
			]
		];

	TArray<FT66AnimatedBackgroundLayer> BackgroundLayers;
	BackgroundLayers.Reserve(3);

	FT66AnimatedBackgroundLayer& SkyLayer = BackgroundLayers.AddDefaulted_GetRef();
	SkyLayer.Brush = SkyBackgroundBrush.Get();
	SkyLayer.SwayAmplitude = FVector2D(15.f, 5.f);
	SkyLayer.SwayFrequency = 0.15f;
	SkyLayer.OpacityMin = 1.f;
	SkyLayer.OpacityMax = 1.f;
	SkyLayer.PhaseOffset = 0.00f;

	FT66AnimatedBackgroundLayer& FireMoonLayer = BackgroundLayers.AddDefaulted_GetRef();
	FireMoonLayer.Brush = FireMoonBrush.Get();
	FireMoonLayer.BaseOffset = FVector2D(0.f, SceneVerticalOffset * 0.72f);
	FireMoonLayer.SwayAmplitude = FVector2D(3.f, 2.f);
	FireMoonLayer.SwayFrequency = 0.10f;
	FireMoonLayer.ScalePulseAmplitude = 0.05f;
	FireMoonLayer.ScalePulseFrequency = 0.30f;
	FireMoonLayer.OpacityMin = 0.70f;
	FireMoonLayer.OpacityMax = 1.00f;
	FireMoonLayer.OpacityFrequency = 0.40f;
	FireMoonLayer.PhaseOffset = 0.21f;

	FT66AnimatedBackgroundLayer& PyramidLayer = BackgroundLayers.AddDefaulted_GetRef();
	PyramidLayer.Brush = PyramidChadBrush.Get();
	PyramidLayer.BaseOffset = FVector2D(0.f, SceneVerticalOffset);
	PyramidLayer.SwayAmplitude = FVector2D(2.f, 1.f);
	PyramidLayer.SwayFrequency = 0.08f;
	PyramidLayer.OpacityMin = 1.f;
	PyramidLayer.OpacityMax = 1.f;
	PyramidLayer.PhaseOffset = 0.43f;

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		.Padding(0.f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(ST66AnimatedBackground)
				.Layers(BackgroundLayers)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.36f))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.Padding(0.f, SceneTitleTopPadding, 0.f, 0.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.MainMenu", "SceneTitle", "CHADPOCALYPSE"))
					.Font([SceneTitleFontSize]()
					{
						FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Black"), SceneTitleFontSize);
						Font.LetterSpacing = 104;
						return Font;
					}())
					.ColorAndOpacity(FLinearColor(0.98f, 0.68f, 0.18f, 0.30f))
					.ShadowOffset(FVector2D(0.f, 0.f))
					.RenderTransform(FSlateRenderTransform(FVector2D(2.f, 2.f)))
				]
				+ SOverlay::Slot()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.MainMenu", "SceneTitle", "CHADPOCALYPSE"))
					.Font([SceneTitleFontSize]()
					{
						FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Black"), SceneTitleFontSize);
						Font.LetterSpacing = 104;
						return Font;
					}())
					.ColorAndOpacity(FLinearColor(0.02f, 0.01f, 0.01f, 0.98f))
					.ShadowOffset(FVector2D(2.f, 2.f))
					.ShadowColorAndOpacity(FLinearColor(1.0f, 0.72f, 0.18f, 0.42f))
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Bottom)
				.Padding(FMargin(HorizontalFramePadding, 0.f, 0.f, BottomFramePadding))
				[
					SNew(SBox)
					.WidthOverride(ResolvedSidePanelWidth)
					.HeightOverride(SidePanelTargetHeight)
					.Clipping(EWidgetClipping::ClipToBounds)
					[
						SNew(SInvalidationPanel)
						[
							LeftSocialShell
						]
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Bottom)
				.Padding(0.f, 0.f, 0.f, CenterButtonLift + BottomFramePadding)
				[
					SNew(SBox)
					.WidthOverride(CenterColumnWidth)
					.HeightOverride(CenterColumnHeight)
					[
						SNew(SInvalidationPanel)
						[
							CenterMenuButtons
						]
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				.Padding(FMargin(0.f, 0.f, HorizontalFramePadding, BottomFramePadding))
				[
					SNew(SBox)
					.WidthOverride(ResolvedLeaderboardPanelWidth)
					.HeightOverride(LeaderboardShellTargetHeight)
					.Clipping(EWidgetClipping::ClipToBounds)
					[
						SNew(SInvalidationPanel)
						[
							LeaderboardShell
						]
					]
				]
			]
		];
}

void UT66MainMenuScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bViewportResponsiveRebuildQueued)
	{
		return;
	}

	const FVector2D CurrentViewportSize = GetEffectiveFrontendViewportSize();
	if (!CurrentViewportSize.Equals(CachedViewportSize, 1.0f))
	{
		CachedViewportSize = CurrentViewportSize;
		bViewportResponsiveRebuildQueued = true;
		ForceRebuildSlate();
	}
}

void UT66MainMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	UE_LOG(LogT66MainMenu, Verbose, TEXT("MainMenuScreen activated."));

	// Important: Screen UI can be built before UIManager is assigned by UT66UIManager.
	// Inject it here so the leaderboard panel can open modals on row click.
	if (LeaderboardPanel.IsValid())
	{
		LeaderboardPanel->SetUIManager(UIManager);
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartyStateChangedHandle = PartySubsystem->OnPartyStateChanged().AddUObject(this, &UT66MainMenuScreen::HandlePartyStateChanged);
			PartySubsystem->ApplyCurrentPartyToGameInstanceRunContext();
		}

		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionStateChangedHandle = SessionSubsystem->OnSessionStateChanged().AddUObject(this, &UT66MainMenuScreen::HandleSessionStateChanged);
			SessionSubsystem->HandlePartyHubScreenActivated();
		}
	}

	// Subscribe to language changes
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66MainMenuScreen::OnLanguageChanged);
	}
}

void UT66MainMenuScreen::OnScreenDeactivated_Implementation()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->OnPartyStateChanged().Remove(PartyStateChangedHandle);
			PartyStateChangedHandle.Reset();
		}

		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->OnSessionStateChanged().Remove(SessionStateChangedHandle);
			SessionStateChangedHandle.Reset();
		}
	}

	Super::OnScreenDeactivated_Implementation();
}

void UT66MainMenuScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	// Force rebuild UI with current language (deferred to next tick for safety)
	ForceRebuildSlate();

	if (LeaderboardPanel.IsValid())
	{
		LeaderboardPanel->SetUIManager(UIManager);
	}
}

void UT66MainMenuScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	// Rebuild UI when language changes
	ForceRebuildSlate();
	if (LeaderboardPanel.IsValid())
	{
		LeaderboardPanel->SetUIManager(UIManager);
	}
}

void UT66MainMenuScreen::HandlePartyStateChanged()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->ApplyCurrentPartyToGameInstanceRunContext();
		}
	}

	ForceRebuildSlate();
}

void UT66MainMenuScreen::HandleSessionStateChanged()
{
	ForceRebuildSlate();
}

void UT66MainMenuScreen::RequestBackgroundTexture()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	if (TexPool)
	{
		TArray<FSoftObjectPath> ExistingBackgroundPaths;
		ExistingBackgroundPaths.Reserve(3);

		if (FPackageName::DoesPackageExist(TEXT("/Game/UI/MainMenu/sky_bg")))
		{
			ExistingBackgroundPaths.Add(FSoftObjectPath(TEXT("/Game/UI/MainMenu/sky_bg.sky_bg")));
		}
		if (FPackageName::DoesPackageExist(TEXT("/Game/UI/MainMenu/fire_moon")))
		{
			ExistingBackgroundPaths.Add(FSoftObjectPath(TEXT("/Game/UI/MainMenu/fire_moon.fire_moon")));
		}
		if (FPackageName::DoesPackageExist(TEXT("/Game/UI/MainMenu/pyramid_chad")))
		{
			ExistingBackgroundPaths.Add(FSoftObjectPath(TEXT("/Game/UI/MainMenu/pyramid_chad.pyramid_chad")));
		}

		if (ExistingBackgroundPaths.Num() > 0)
		{
			TexPool->EnsureTexturesLoadedSync(ExistingBackgroundPaths);
		}
	}

	SetupMainMenuLayerBrush(
		SkyBackgroundBrush,
		TexPool,
		this,
		TEXT("/Game/UI/MainMenu/sky_bg.sky_bg"),
		TEXT("/Game/UI/MainMenu/sky_bg"),
		TEXT("SourceAssets/UI/MainMenu/Animated/sky_bg.png"),
		FName(TEXT("MainMenuSkyBg")));

	SetupMainMenuLayerBrush(
		FireMoonBrush,
		TexPool,
		this,
		TEXT("/Game/UI/MainMenu/fire_moon.fire_moon"),
		TEXT("/Game/UI/MainMenu/fire_moon"),
		TEXT("SourceAssets/UI/MainMenu/Animated/fire_moon.png"),
		FName(TEXT("MainMenuFireMoon")));

	SetupMainMenuLayerBrush(
		PyramidChadBrush,
		TexPool,
		this,
		TEXT("/Game/UI/MainMenu/pyramid_chad.pyramid_chad"),
		TEXT("/Game/UI/MainMenu/pyramid_chad"),
		TEXT("SourceAssets/UI/MainMenu/Animated/pyramid_chad.png"),
		FName(TEXT("MainMenuPyramidChad")));
}

void UT66MainMenuScreen::RequestUtilityButtonIcons()
{
	auto LoadGeneratedIcon = [](const FString& RelativePath, TSharedPtr<FSlateBrush>& IconBrush)
	{
		if (!IconBrush.IsValid())
		{
			const FString IconPath = FPaths::ProjectDir() / RelativePath;
			if (FPaths::FileExists(IconPath))
			{
				IconBrush = MakeShared<FSlateImageBrush>(IconPath, FVector2D(24.f, 24.f), FLinearColor::White);
			}
		}
	};

	LoadGeneratedIcon(TEXT("SourceAssets/UI/MainMenu/Generated/mainmenu-settings-icon-slate.png"), SettingsIconBrush);
	LoadGeneratedIcon(TEXT("SourceAssets/UI/MainMenu/Generated/mainmenu-language-icon-slate.png"), LanguageIconBrush);
}

// Slate click handlers (return FReply)
FReply UT66MainMenuScreen::HandleNewGameClicked()
{
	OnNewGameClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleLoadGameClicked()
{
	OnLoadGameClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandlePowerUpClicked()
{
	OnPowerUpClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleMinigamesClicked()
{
	OnMinigamesClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleAchievementsClicked()
{
	OnAchievementsClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleSettingsClicked()
{
	OnSettingsClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleLanguageClicked()
{
	OnLanguageClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleQuitClicked()
{
	OnQuitClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandleLeavePartyClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (SessionSubsystem->LeaveFrontendLobby(ET66ScreenType::MainMenu))
			{
				return FReply::Handled();
			}
		}

		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->ResetToLocalParty();
		}
	}

	ForceRebuildSlate();
	return FReply::Handled();
}

// UFUNCTION handlers (call navigation)
void UT66MainMenuScreen::OnNewGameClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bIsNewGameFlow = true;
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->ApplyCurrentPartyToGameInstanceRunContext();
		}
	}
	NavigateTo(ET66ScreenType::HeroSelection);
}

void UT66MainMenuScreen::OnLoadGameClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bIsNewGameFlow = false;
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->ApplyCurrentPartyToGameInstanceRunContext();
		}
	}
	NavigateTo(ET66ScreenType::SaveSlots);
}

void UT66MainMenuScreen::OnPowerUpClicked()
{
	NavigateTo(ET66ScreenType::PowerUp);
}

void UT66MainMenuScreen::OnMinigamesClicked()
{
	NavigateTo(ET66ScreenType::Unlocks);
}

void UT66MainMenuScreen::OnSettingsClicked()
{
	NavigateTo(ET66ScreenType::Settings);
}

void UT66MainMenuScreen::OnAchievementsClicked()
{
	NavigateTo(ET66ScreenType::Achievements);
}

void UT66MainMenuScreen::OnLanguageClicked()
{
	NavigateTo(ET66ScreenType::LanguageSelect);
}

void UT66MainMenuScreen::OnQuitClicked()
{
	ShowModal(ET66ScreenType::QuitConfirmation);
}

void UT66MainMenuScreen::OnAccountStatusClicked()
{
	NavigateTo(ET66ScreenType::AccountStatus);
}

bool UT66MainMenuScreen::ShouldShowAccountStatus() const
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	return LB && LB->ShouldShowAccountStatusButton();
}
