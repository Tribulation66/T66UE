// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MainMenuScreen.h"
#include "UI/ST66AnimatedBackground.h"
#include "UI/ST66AnimatedBorderGlow.h"
#include "UI/T66UIManager.h"
#include "UI/Components/T66LeaderboardPanel.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"
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
#include "Widgets/Input/SEditableTextBox.h"
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
		TStrongObjectPtr<UTexture2D>& TextureHandle,
		UT66UITexturePoolSubsystem* TexPool,
		UObject* Requester,
		const TCHAR* ObjectPath,
		const TCHAR* PackagePath,
		const TCHAR* SourceRelativePath,
		FName RequestKey)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
			EnsureRuntimeImageBrush(Brush, MainMenuBackgroundImageSize);
		}
		else
		{
			EnsureRuntimeImageBrush(Brush, MainMenuBackgroundImageSize);
		}

		if (!TextureHandle.IsValid())
		{
			if (UTexture2D* AssetTexture = T66RuntimeUITextureAccess::LoadAssetTexture(
				ObjectPath,
				TextureFilter::TF_Trilinear,
				TEXT("MainMenuBackgroundLayer")))
			{
				TextureHandle.Reset(AssetTexture);
			}
		}

		if (TextureHandle.IsValid())
		{
			Brush->SetResourceObject(TextureHandle.Get());
			Brush->ImageSize = FVector2D(
				FMath::Max(1, TextureHandle->GetSizeX()),
				FMath::Max(1, TextureHandle->GetSizeY()));
		}
		else
		{
			Brush->SetResourceObject(nullptr);
			Brush->ImageSize = MainMenuBackgroundImageSize;
		}

		if (!TexPool || !FPackageName::DoesPackageExist(PackagePath) || TextureHandle.IsValid())
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
		const TCHAR* AssetPath,
		const TCHAR* StagedRelativePath,
		const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
		}

		EnsureRuntimeImageBrush(Brush, ImageSize);

		if (!TextureHandle.IsValid())
		{
			static TMap<FString, TWeakObjectPtr<UTexture2D>> CachedRuntimeTextures;
			const FString CacheKey = AssetPath && *AssetPath
				? FString::Printf(TEXT("asset:%s"), AssetPath)
				: FString::Printf(TEXT("stage:%s"), StagedRelativePath ? StagedRelativePath : TEXT(""));
			if (TWeakObjectPtr<UTexture2D>* CachedTexture = CachedRuntimeTextures.Find(CacheKey))
			{
				if (CachedTexture->IsValid())
				{
					TextureHandle.Reset(CachedTexture->Get());
				}
			}

			if (AssetPath && *AssetPath)
			{
				if (UTexture2D* AssetTexture = T66RuntimeUITextureAccess::LoadAssetTexture(
					AssetPath,
					TextureFilter::TF_Trilinear,
					TEXT("MainMenuRuntimeImage")))
				{
					TextureHandle.Reset(AssetTexture);
				}
			}

			if (!TextureHandle.IsValid() && StagedRelativePath && *StagedRelativePath)
			{
				for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(StagedRelativePath))
				{
					if (!FPaths::FileExists(CandidatePath))
					{
						continue;
					}

					if (UTexture2D* FileTexture = T66RuntimeUITextureAccess::ImportFileTexture(
						CandidatePath,
						TextureFilter::TF_Trilinear,
						false,
						TEXT("MainMenuRuntimeImage")))
					{
						TextureHandle.Reset(FileTexture);
						CachedRuntimeTextures.Add(CacheKey, FileTexture);
						break;
					}
				}
			}

			if (TextureHandle.IsValid())
			{
				CachedRuntimeTextures.Add(CacheKey, TextureHandle.Get());
			}
		}

		if (!TextureHandle.IsValid())
		{
			Brush->SetResourceObject(nullptr);
			return;
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

	bool DoesFriendMatchSearchQuery(const FString& SearchQuery, const FString& FriendName)
	{
		const FString NormalizedQuery = SearchQuery.TrimStartAndEnd();
		return NormalizedQuery.IsEmpty() || FriendName.Contains(NormalizedQuery, ESearchCase::IgnoreCase);
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

uint32 UT66MainMenuScreen::CaptureMenuStateHash() const
{
	uint32 StateHash = 0;

	if (const UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		StateHash = HashCombine(StateHash, GetTypeHash(static_cast<uint8>(Loc->GetCurrentLanguage())));
	}

	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (const UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			for (const FT66PartyFriendEntry& Friend : PartySubsystem->GetFriends())
			{
				StateHash = HashCombine(StateHash, GetTypeHash(Friend.PlayerId));
				StateHash = HashCombine(StateHash, GetTypeHash(Friend.DisplayName));
				StateHash = HashCombine(StateHash, GetTypeHash(Friend.PresenceText));
				StateHash = HashCombine(StateHash, GetTypeHash(Friend.bOnline));
			}

			for (const FT66PartyMemberEntry& Member : PartySubsystem->GetPartyMembers())
			{
				StateHash = HashCombine(StateHash, GetTypeHash(Member.PlayerId));
				StateHash = HashCombine(StateHash, GetTypeHash(Member.DisplayName));
				StateHash = HashCombine(StateHash, GetTypeHash(Member.bIsLocal));
				StateHash = HashCombine(StateHash, GetTypeHash(Member.bIsPartyHost));
				StateHash = HashCombine(StateHash, GetTypeHash(Member.bOnline));
			}
		}

		if (const UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			StateHash = HashCombine(StateHash, GetTypeHash(SessionSubsystem->IsPartyLobbyContextActive()));
			StateHash = HashCombine(StateHash, GetTypeHash(SessionSubsystem->GetCurrentLobbyPlayerCount()));
			StateHash = HashCombine(StateHash, GetTypeHash(SessionSubsystem->IsLocalPlayerPartyHost()));
			StateHash = HashCombine(StateHash, GetTypeHash(static_cast<uint8>(SessionSubsystem->GetDesiredPartyFrontendScreen())));
		}
	}

	return StateHash;
}

bool UT66MainMenuScreen::ShouldRebuildRetainedSlate() const
{
	if (!HasBuiltSlateUI())
	{
		return true;
	}

	const UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const ET66Language CurrentLanguage = Loc ? Loc->GetCurrentLanguage() : ET66Language::English;
	if (CurrentLanguage != LastBuiltLanguage)
	{
		return true;
	}

	if (!GetEffectiveFrontendViewportSize().Equals(LastBuiltViewportSize, 1.0f))
	{
		return true;
	}

	return CaptureMenuStateHash() != LastBuiltMenuStateHash;
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
	LastBuiltViewportSize = CachedViewportSize;
	PendingViewportSize = CachedViewportSize;
	PendingViewportStableTime = 0.f;
	bViewportResponsiveRebuildQueued = false;
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PlayerSettings = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	UT66SteamHelper* SteamHelper = GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr;
	LastBuiltLanguage = Loc ? Loc->GetCurrentLanguage() : ET66Language::English;
	LastBuiltMenuStateHash = CaptureMenuStateHash();
	FriendGroupWidgetRefs.Reset();
	FriendRowWidgetRefs.Reset();
	FriendGroupsDividerBox.Reset();
	NoMatchingFriendsBox.Reset();

	const FText NewGameText = NSLOCTEXT("T66.MainMenu", "Start", "START");
	const FText LoadGameText = NSLOCTEXT("T66.MainMenu", "Continue", "CONTINUE");
	SetupRuntimeImageBrush(
		PrimaryCTAFillBrush,
		PrimaryCTAFillTexture,
		nullptr,
		TEXT("RuntimeDependencies/T66/UI/MainMenu/mainmenu_cta_fill_green.png"),
		FVector2D(1024.f, 232.f));

	struct FMenuFriendEntry
	{
		FString PlayerId;
		FString Name;
		FString Status;
		bool bOnline = false;
		bool bFavorite = false;
		bool bInvitePending = false;
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

	TArray<FName> AllHeroIDs = T66GI ? T66GI->GetAllHeroIDs() : TArray<FName>();
	const FName FallbackHeroID = AllHeroIDs.Num() > 0 ? AllHeroIDs[0] : NAME_None;

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
	PartyPortraitBrushes.Reset();
	if (PartySubsystem)
	{
		auto ResolvePartyAvatarTexture = [PartySubsystem, SteamHelper](const FT66PartyMemberEntry& Member) -> UTexture2D*
		{
			if (!SteamHelper)
			{
				return nullptr;
			}

			if (UTexture2D* ExactAvatar = SteamHelper->GetAvatarTextureForSteamId(Member.PlayerId))
			{
				return ExactAvatar;
			}

			if (Member.bIsLocal || Member.DisplayName.Equals(SteamHelper->GetLocalDisplayName(), ESearchCase::IgnoreCase))
			{
				if (UTexture2D* LocalAvatar = SteamHelper->GetLocalAvatarTexture())
				{
					return LocalAvatar;
				}
			}

			const FT66PartyFriendEntry* MatchingFriend = PartySubsystem->GetFriends().FindByPredicate([&Member](const FT66PartyFriendEntry& Friend)
			{
				return Friend.DisplayName.Equals(Member.DisplayName, ESearchCase::IgnoreCase);
			});
			return MatchingFriend ? SteamHelper->GetAvatarTextureForSteamId(MatchingFriend->PlayerId) : nullptr;
		};

		const TArray<FT66PartyFriendEntry>& PartyFriends = PartySubsystem->GetFriends();
		for (int32 FriendIndex = 0; FriendIndex < PartyFriends.Num(); ++FriendIndex)
		{
			const FT66PartyFriendEntry& Friend = PartyFriends[FriendIndex];
			Friends.Add({
				Friend.PlayerId,
				Friend.DisplayName,
				Friend.PresenceText,
				Friend.bOnline,
				PlayerSettings ? PlayerSettings->IsFavoriteFriend(Friend.PlayerId) : false,
				SessionSubsystem ? SessionSubsystem->IsFriendInvitePending(Friend.PlayerId) : false,
				PickHeroID(FriendIndex),
				(FriendIndex % 2 == 0) ? ET66BodyType::TypeA : ET66BodyType::TypeB
			});
		}

		const TArray<FT66PartyMemberEntry>& PartyMembers = PartySubsystem->GetPartyMembers();
		PartyEntries.Reserve(PartyMembers.Num());
		PartyPortraitBrushes.Reserve(PartyMembers.Num());
		for (const FT66PartyMemberEntry& Member : PartyMembers)
		{
			UTexture2D* AvatarTexture = ResolvePartyAvatarTexture(Member);
			TSharedPtr<FSlateBrush> AvatarBrush = MakeAvatarBrush(AvatarTexture, FVector2D(60.f, 60.f));
			PartyPortraitBrushes.Add(AvatarBrush);
			PartyEntries.Add({
				Member.PlayerId,
				Member.DisplayName,
				Member.bIsLocal,
				Member.bIsPartyHost,
				Member.bOnline,
				AvatarBrush
			});
		}
	}

	Friends.Sort([](const FMenuFriendEntry& A, const FMenuFriendEntry& B)
	{
		if (A.bOnline != B.bOnline)
		{
			return A.bOnline;
		}

		if (A.bFavorite != B.bFavorite)
		{
			return A.bFavorite;
		}

		return A.Name < B.Name;
	});

	const TSharedRef<TArray<FMenuFriendEntry>> FriendsModel = MakeShared<TArray<FMenuFriendEntry>>(Friends);
	auto CountMatchingFriends = [this, FriendsModel](bool bOnlineGroup) -> int32
	{
		return Algo::CountIf(*FriendsModel, [this, bOnlineGroup](const FMenuFriendEntry& Entry)
		{
			return Entry.bOnline == bOnlineGroup && DoesFriendMatchSearchQuery(FriendSearchQuery, Entry.Name);
		});
	};
	auto CountAllMatchingFriends = [CountMatchingFriends]() -> int32
	{
		return CountMatchingFriends(true) + CountMatchingFriends(false);
	};

	ProfileAvatarBrush = MakeShared<FSlateBrush>();
	ProfileAvatarBrush->DrawAs = ESlateBrushDrawType::Image;
	ProfileAvatarBrush->Tiling = ESlateBrushTileType::NoTile;
	ProfileAvatarBrush->ImageSize = FVector2D(76.f, 76.f);
	ProfileAvatarBrush->SetResourceObject(SteamHelper ? SteamHelper->GetLocalAvatarTexture() : nullptr);

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
			if (TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(
					TexPool,
					FriendPortraitSoft,
					this,
					Brush,
					FName(*FString::Printf(TEXT("MainMenuFriendPortrait_%d"), FriendPortraitBrushes.Num() + 1)),
					/*bClearWhileLoading*/ true);
			}
			else
			{
				Brush->SetResourceObject(nullptr);
			}
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
	float ResolvedSidePanelWidth = LerpDimension(332.f, 382.f);
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
	const float SidePanelTargetHeight = FMath::Min(AvailableColumnHeight, LerpDimension(494.f, 562.f));
	const float LeaderboardChromeHeight = LerpDimension(52.f, 58.f);
	const float LeaderboardShellTargetHeight = SidePanelTargetHeight + LeaderboardChromeHeight;
	const float CenterPanelHorizontalPadding = LerpDimension(12.f, 16.f);
	const float CenterPanelVerticalPadding = LerpDimension(14.f, 18.f);
	const float CenterButtonWidth = CenterColumnWidth - (CenterPanelHorizontalPadding * 2.f);
	const float CenterButtonHeight = LerpDimension(68.f, 80.f);
	const float CenterButtonGap = LerpDimension(10.f, 14.f);
	const float CenterColumnHeight = (CenterButtonHeight * 2.f) + CenterButtonGap + (CenterPanelVerticalPadding * 2.f);
	const float CenterButtonLift = FMath::Max(LerpDimension(38.f, 52.f), (ColumnTargetHeight - CenterColumnHeight) * 0.24f);
	const float SceneVerticalOffset = LerpDimension(96.f, 128.f);
	const float SceneTitleTopPadding = FMath::Max(0.f, TopReservedInset - LerpDimension(10.f, 18.f));
	const int32 LeftPanelBodyFontSize = LerpInt(10, 11);
	const int32 LeftPanelTitleFontSize = LerpInt(10, 11);
	const int32 FriendsPanelBodyFontSize = FMath::Max(8, LeftPanelBodyFontSize - 2);
	const int32 FriendsPanelHeaderFontSize = FMath::Max(8, LeftPanelTitleFontSize - 2);
	const int32 CenterButtonFontSize = LerpInt(24, 28);
	const int32 SceneTitleFontSize = LerpInt(50, 62);
	const int32 SceneSubtitleFontSize = LerpInt(18, 22);

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

	auto MakeFriendGroupHeader = [this, HeaderText, FriendsPanelHeaderFontSize, CountMatchingFriends](const FText& Label, const FLinearColor& CountColor, bool bOnlineGroup) -> TSharedRef<SWidget>
	{
		FSlateFontInfo GroupFont = FT66Style::MakeFont(TEXT("Bold"), FriendsPanelHeaderFontSize);
		GroupFont.LetterSpacing = 72;
		const bool bExpanded = bOnlineGroup ? bShowOnlineFriends : bShowOfflineFriends;
		const FSlateBrush* DownArrowBrush = &FCoreStyle::Get().GetWidgetStyle<FComboButtonStyle>("ComboButton").DownArrowImage;
		TSharedPtr<SBox> GroupRootBox;
		TSharedPtr<SImage> ExpandArrowImage;
		TSharedPtr<STextBlock> GroupCountText;

		const TSharedRef<SWidget> HeaderRow =
			SNew(SHorizontalBox)
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

					RefreshFriendListVisualState();
					return FReply::Handled();
				}))
				[
					SNew(SBox)
					.WidthOverride(14.f)
					.HeightOverride(14.f)
					[
						SAssignNew(ExpandArrowImage, SImage)
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
				SAssignNew(GroupCountText, STextBlock)
				.Text(FText::Format(
					NSLOCTEXT("T66.MainMenu", "FriendsGroupCount", "({0})"),
					FText::AsNumber(CountMatchingFriends(bOnlineGroup))))
				.Font(GroupFont)
				.ColorAndOpacity(CountColor)
			];

		TSharedRef<SBox> GroupRootWidget =
			SAssignNew(GroupRootBox, SBox)
			.Visibility((FriendSearchQuery.TrimStartAndEnd().IsEmpty() || CountMatchingFriends(bOnlineGroup) > 0)
				? EVisibility::Visible
				: EVisibility::Collapsed)
			[
				HeaderRow
			];

		FFriendGroupWidgetRefs& GroupRefs = FriendGroupWidgetRefs.AddDefaulted_GetRef();
		GroupRefs.bOnlineGroup = bOnlineGroup;
		GroupRefs.RootBox = GroupRootBox;
		GroupRefs.CountText = GroupCountText;
		GroupRefs.ExpandArrowImage = ExpandArrowImage;

		return GroupRootWidget;
	};

	const FText InviteFriendText = NSLOCTEXT("T66.MainMenu", "InviteFriend", "INVITE");
	const FText InvitedFriendText = NSLOCTEXT("T66.MainMenu", "InvitedFriend", "INVITED");
	const FText InPartyText = NSLOCTEXT("T66.MainMenu", "InParty", "In Party");
	const FText PartyFullText = NSLOCTEXT("T66.MainMenu", "PartyFull", "PARTY FULL");
	const FText OfflineActionText = NSLOCTEXT("T66.MainMenu", "FriendOffline", "OFFLINE");
	const FText LeavePartyButtonText = NSLOCTEXT("T66.MainMenu", "LeavePartyButton", "X");
	const FText FavoriteFriendTooltip = NSLOCTEXT("T66.MainMenu", "FavoriteFriendTooltip", "Favorite friend");
	const FText UnfavoriteFriendTooltip = NSLOCTEXT("T66.MainMenu", "UnfavoriteFriendTooltip", "Remove favorite");
	const FButtonStyle& NoBorderButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");

	auto MakeFlatActionButton = [FriendsPanelBodyFontSize, &NoBorderButtonStyle](
		TAttribute<FText> Text,
		const FOnClicked& OnClicked,
		TAttribute<bool> IsEnabled,
		TAttribute<FSlateColor> FillColor) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(80.f)
			.HeightOverride(30.f)
			[
				SNew(SButton)
				.ButtonStyle(&NoBorderButtonStyle)
				.ContentPadding(FMargin(0.f))
				.IsEnabled(IsEnabled)
				.OnClicked(OnClicked)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FillColor)
					.Padding(FMargin(8.f, 5.f))
					[
						SNew(STextBlock)
						.Text(Text)
						.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
						.ColorAndOpacity(TAttribute<FSlateColor>::CreateLambda([IsEnabled]() -> FSlateColor
						{
							return IsEnabled.Get()
								? FSlateColor(FLinearColor(0.96f, 0.97f, 0.94f, 1.0f))
								: FSlateColor(FLinearColor(0.88f, 0.90f, 0.84f, 0.72f));
						}))
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
		const FLinearColor AccentColor = Friend.bOnline ? AvatarAccentOnline : AvatarAccentOffline;
		auto IsFriendInParty = [PartySubsystem, PlayerId = Friend.PlayerId]() -> bool
		{
			return PartySubsystem && PartySubsystem->IsFriendInParty(PlayerId);
		};
		auto IsInvitePending = [SessionSubsystem, PlayerId = Friend.PlayerId, IsFriendInParty]() -> bool
		{
			return !IsFriendInParty() && SessionSubsystem && SessionSubsystem->IsFriendInvitePending(PlayerId);
		};
		auto CanInviteFriend = [PartySubsystem, bOnline = Friend.bOnline, IsFriendInParty, IsInvitePending]() -> bool
		{
			return PartySubsystem && bOnline && !IsFriendInParty() && !IsInvitePending() && PartySubsystem->GetPartyMemberCount() < 4;
		};
		auto ResolveFavoriteGlyph = [PlayerSettings, PlayerId = Friend.PlayerId]() -> FText
		{
			return FText::FromString((PlayerSettings && PlayerSettings->IsFavoriteFriend(PlayerId)) ? TEXT("\u2605") : TEXT("\u2606"));
		};
		auto ResolveFavoriteGlyphColor = [PlayerSettings, PlayerId = Friend.PlayerId]() -> FSlateColor
		{
			const bool bFavorite = PlayerSettings && PlayerSettings->IsFavoriteFriend(PlayerId);
			return FSlateColor(bFavorite
				? FLinearColor(0.94f, 0.78f, 0.28f, 1.0f)
				: FLinearColor(0.55f, 0.58f, 0.62f, 1.0f));
		};
		auto ResolveActionText = [
			PartySubsystem,
			bOnline = Friend.bOnline,
			IsFriendInParty,
			IsInvitePending,
			InPartyText,
			InvitedFriendText,
			OfflineActionText,
			PartyFullText,
			InviteFriendText]() -> FText
		{
			if (IsFriendInParty())
			{
				return InPartyText;
			}
			if (IsInvitePending())
			{
				return InvitedFriendText;
			}
			if (!bOnline)
			{
				return OfflineActionText;
			}
			if (PartySubsystem && PartySubsystem->GetPartyMemberCount() >= 4)
			{
				return PartyFullText;
			}
			return InviteFriendText;
		};
		auto ResolveActionFillColor = [bOnline = Friend.bOnline, IsFriendInParty, IsInvitePending, CanInviteFriend]() -> FSlateColor
		{
			if (IsInvitePending())
			{
				return FSlateColor(FLinearColor(0.56f, 0.42f, 0.16f, 0.98f));
			}
			if (!CanInviteFriend())
			{
				return FSlateColor(FLinearColor(0.42f, 0.47f, 0.40f, 0.72f));
			}
			if (bOnline && !IsFriendInParty())
			{
				return FSlateColor(FLinearColor(0.18f, 0.31f, 0.18f, 0.97f));
			}
			return FSlateColor(FLinearColor(0.27f, 0.30f, 0.34f, 0.94f));
		};
		TSharedPtr<SBox> RowRootBox;
		TSharedPtr<SBorder> RowBorder;
		TSharedPtr<STextBlock> StatusTextWidget;
		TSharedPtr<SButton> FavoriteButton;
		TSharedPtr<STextBlock> FavoriteGlyphText;
		TSharedPtr<SButton> ActionButton;
		TSharedPtr<SBorder> ActionFillBorder;
		TSharedPtr<STextBlock> ActionTextWidget;

		const TSharedRef<SWidget> RowContent =
			SAssignNew(RowBorder, SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(IsFriendInParty()
				? FLinearColor(0.10f, 0.14f, 0.18f, 0.55f)
				: FLinearColor(0.f, 0.f, 0.f, 0.20f))
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
						.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
						.ColorAndOpacity(NameColor)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f)
					[
						SAssignNew(StatusTextWidget, STextBlock)
						.Text(IsFriendInParty() ? InPartyText : FText::FromString(Friend.Status))
						.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
						.ColorAndOpacity(IsFriendInParty()
							? OnlineStatusText
							: (Friend.bOnline ? OnlineStatusText : MutedText))
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(28.f)
					.HeightOverride(28.f)
					[
						SAssignNew(FavoriteButton, SButton)
						.ButtonStyle(&NoBorderButtonStyle)
						.ContentPadding(FMargin(0.f))
						.ToolTipText((PlayerSettings && PlayerSettings->IsFavoriteFriend(Friend.PlayerId))
							? UnfavoriteFriendTooltip
							: FavoriteFriendTooltip)
						.OnClicked(FOnClicked::CreateLambda([this, PlayerSettings, PlayerId = Friend.PlayerId]()
						{
							if (PlayerSettings)
							{
								PlayerSettings->SetFavoriteFriend(PlayerId, !PlayerSettings->IsFavoriteFriend(PlayerId));
								RefreshFriendListVisualState();
							}
							return FReply::Handled();
						}))
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.10f, 0.11f, 0.14f, 0.94f))
							.Padding(FMargin(0.f))
							[
								SAssignNew(FavoriteGlyphText, STextBlock)
								.Text(ResolveFavoriteGlyph())
								.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize + 6))
								.ColorAndOpacity(ResolveFavoriteGlyphColor())
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(80.f)
					.HeightOverride(30.f)
					[
						SAssignNew(ActionButton, SButton)
						.ButtonStyle(&NoBorderButtonStyle)
						.ContentPadding(FMargin(0.f))
						.IsEnabled(CanInviteFriend())
						.OnClicked(FOnClicked::CreateLambda([this, PartySubsystem, PlayerId = Friend.PlayerId, PlayerName = Friend.Name]()
						{
							if (!PartySubsystem)
							{
								return FReply::Handled();
							}

							if (PartySubsystem->InviteFriend(PlayerId, PlayerName))
							{
								RefreshFriendListVisualState();
							}
							return FReply::Handled();
						}))
						[
							SAssignNew(ActionFillBorder, SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(ResolveActionFillColor())
							.Padding(FMargin(8.f, 5.f))
							[
								SAssignNew(ActionTextWidget, STextBlock)
								.Text(ResolveActionText())
								.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
								.ColorAndOpacity(CanInviteFriend()
									? FLinearColor(0.96f, 0.97f, 0.94f, 1.0f)
									: FLinearColor(0.88f, 0.90f, 0.84f, 0.72f))
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
			];

		const bool bSearchActive = !FriendSearchQuery.TrimStartAndEnd().IsEmpty();
		const bool bExpanded = Friend.bOnline ? bShowOnlineFriends : bShowOfflineFriends;
		const bool bMatches = DoesFriendMatchSearchQuery(FriendSearchQuery, Friend.Name);
		const EVisibility InitialVisibility = (bMatches && (bSearchActive || bExpanded))
			? EVisibility::Visible
			: EVisibility::Collapsed;

		TSharedRef<SWidget> RowRoot =
			SAssignNew(RowRootBox, SBox)
			.Visibility(InitialVisibility)
			[
				RowContent
			];

		FFriendRowWidgetRefs& RowRefs = FriendRowWidgetRefs.AddDefaulted_GetRef();
		RowRefs.PlayerId = Friend.PlayerId;
		RowRefs.FriendName = Friend.Name;
		RowRefs.BaseStatus = Friend.Status;
		RowRefs.bOnline = Friend.bOnline;
		RowRefs.RootBox = RowRootBox;
		RowRefs.RowBorder = RowBorder;
		RowRefs.StatusText = StatusTextWidget;
		RowRefs.FavoriteButton = FavoriteButton;
		RowRefs.FavoriteGlyphText = FavoriteGlyphText;
		RowRefs.ActionButton = ActionButton;
		RowRefs.ActionFillBorder = ActionFillBorder;
		RowRefs.ActionText = ActionTextWidget;

		return RowRoot;
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
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.015f, 1.0f))
			.Padding(FMargin(CenterPanelHorizontalPadding, CenterPanelVerticalPadding))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					MakeMenuButton(NewGameText, &UT66MainMenuScreen::HandleNewGameClicked, ET66ButtonType::Success, true, true)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, CenterButtonGap, 0.f, 0.f)
				[
					MakeMenuButton(LoadGameText, &UT66MainMenuScreen::HandleLoadGameClicked, ET66ButtonType::Success, true, false)
				]
			]
		];

	const FText FriendSearchHintText = NSLOCTEXT("T66.MainMenu", "FriendSearchHint", "Search friends...");
	const FText NoMatchingFriendsText = NSLOCTEXT("T66.MainMenu", "NoMatchingFriends", "No friends match this search.");

	const TSharedRef<SWidget> FriendSearchBox =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.16f, 0.18f, 0.21f, 0.94f))
		.Padding(1.f)
		[
			SNew(SBox)
			.HeightOverride(36.f)
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(FriendSearchQuery))
				.OnTextChanged_UObject(this, &UT66MainMenuScreen::HandleFriendSearchTextChanged)
				.HintText(FriendSearchHintText)
				.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
				.ForegroundColor(BrightText)
				.BackgroundColor(FLinearColor(0.03f, 0.04f, 0.06f, 1.0f))
			]
		];

	const TSharedRef<SVerticalBox> FriendsList = SAssignNew(FriendsListContainer, SVerticalBox);
	auto AddFriendGroup = [&](bool bOnlineGroup)
	{
		FriendsList->AddSlot().AutoHeight()
		[
			MakeFriendGroupHeader(
				bOnlineGroup
					? NSLOCTEXT("T66.MainMenu", "OnlineFriendsHeader", "ONLINE")
					: NSLOCTEXT("T66.MainMenu", "OfflineFriendsHeader", "OFFLINE"),
				bOnlineGroup ? OnlineHeaderText : HeaderText,
				bOnlineGroup)
		];

		for (int32 FriendIndex = 0; FriendIndex < Friends.Num(); ++FriendIndex)
		{
			const FMenuFriendEntry& Friend = Friends[FriendIndex];

			FriendsList->AddSlot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(SBox)
				.Visibility_Lambda([this, Friend, bOnlineGroup]()
				{
					const bool bSearchActive = !FriendSearchQuery.TrimStartAndEnd().IsEmpty();
					const bool bExpanded = Friend.bOnline ? bShowOnlineFriends : bShowOfflineFriends;
					const bool bMatches = DoesFriendMatchSearchQuery(FriendSearchQuery, Friend.Name);
					return (Friend.bOnline == bOnlineGroup && bMatches && (bSearchActive || bExpanded))
						? EVisibility::Visible
						: EVisibility::Collapsed;
				})
				[
					MakeFriendRow(Friend, FriendIndex)
				]
			];
		}
	};

	AddFriendGroup(true);
	FriendsList->AddSlot().AutoHeight().Padding(0.f, 20.f, 0.f, 0.f)
	[
		SAssignNew(FriendGroupsDividerBox, SBox)
		.Visibility(EVisibility::Visible)
		[
			SNew(SSpacer)
			.Size(FVector2D(1.f, 1.f))
		]
	];
	AddFriendGroup(false);
	FriendsList->AddSlot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
	[
		SAssignNew(NoMatchingFriendsBox, SBox)
		.Visibility(EVisibility::Collapsed)
		[
			SNew(STextBlock)
			.Text(NoMatchingFriendsText)
			.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
			.ColorAndOpacity(MutedText)
		]
	];

	const bool bCanLeaveParty = (SessionSubsystem && SessionSubsystem->IsPartyLobbyContextActive())
		|| (PartySubsystem && PartySubsystem->HasRemotePartyMembers());

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

	const bool bHasProfileAvatar = ProfileAvatarBrush.IsValid() && ProfileAvatarBrush->GetResourceObject() != nullptr;
	const FString LocalSteamName = SteamHelper ? SteamHelper->GetLocalDisplayName() : FString();
	const FString LocalSteamId = SteamHelper ? SteamHelper->GetLocalSteamId() : FString();
	const FText ProfileNameText = !LocalSteamName.IsEmpty()
		? FText::FromString(LocalSteamName)
		: NSLOCTEXT("T66.MainMenu", "ProfileNameFallback", "Local Player");
	const FText ProfileConnectionText = (SteamHelper && SteamHelper->IsSteamReady())
		? NSLOCTEXT("T66.MainMenu", "ProfileConnected", "Steam Connected")
		: NSLOCTEXT("T66.MainMenu", "ProfileOffline", "Steam Unavailable");
	const FLinearColor ProfileConnectionColor = (SteamHelper && SteamHelper->IsSteamReady())
		? FLinearColor(0.44f, 0.80f, 0.43f, 1.0f)
		: MutedText;
	const FText ProfileMetaText = !LocalSteamId.IsEmpty()
		? FText::Format(NSLOCTEXT("T66.MainMenu", "ProfileMeta", "Steam ID {0}"), FText::FromString(LocalSteamId))
		: NSLOCTEXT("T66.MainMenu", "ProfileMetaFallback", "Account, records, and rewards");
	const FText ProfileActionText = NSLOCTEXT("T66.MainMenu", "ProfileAction", "Open account, history, and rewards");
	const FText ProfileFallbackGlyph = NSLOCTEXT("T66.MainMenu", "ProfileFallbackGlyph", "YOU");

	const TSharedRef<SWidget> ProfileCardContent =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(60.f)
			.HeightOverride(60.f)
			[
				FT66Style::MakeSlotFrame(
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Image(ProfileAvatarBrush.Get())
						.ColorAndOpacity(bHasProfileAvatar ? FLinearColor::White : FLinearColor(1.f, 1.f, 1.f, 0.f))
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(ProfileFallbackGlyph)
						.Font(FT66Style::MakeFont(TEXT("Bold"), FMath::Max(8, FriendsPanelBodyFontSize - 1)))
						.ColorAndOpacity(MutedText)
						.Visibility(bHasProfileAvatar ? EVisibility::Collapsed : EVisibility::Visible)
					],
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
				.Text(ProfileNameText)
				.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
				.ColorAndOpacity(BrightText)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(ProfileConnectionText)
				.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
				.ColorAndOpacity(ProfileConnectionColor)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(ProfileMetaText)
				.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
				.ColorAndOpacity(MutedText)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(ProfileActionText)
				.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
				.ColorAndOpacity(MutedText)
			]
		];

	const TSharedRef<SWidget> ProfileCard =
		SNew(SButton)
		.ButtonStyle(&NoBorderButtonStyle)
		.ContentPadding(FMargin(0.f))
		.ToolTipText(NSLOCTEXT("T66.MainMenu", "OpenProfileTooltip", "Open your account page"))
		.OnClicked(FOnClicked::CreateLambda([this]()
		{
			OnAccountStatusClicked();
			return FReply::Handled();
		}))
		[
			ProfileCardContent
		];

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
						ProfileCard
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
					[
						MakeSubtleDivider()
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
					[
						FriendSearchBox
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

	TSharedRef<SBorder> Root =
		SNew(SBorder)
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
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
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
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 6.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.MainMenu", "SceneSubtitle", "If you're not Chad it's over"))
					.Font([SceneSubtitleFontSize]()
					{
						FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Bold"), SceneSubtitleFontSize);
						Font.LetterSpacing = 38;
						return Font;
					}())
					.ColorAndOpacity(FLinearColor(0.96f, 0.84f, 0.62f, 0.96f))
					.ShadowOffset(FVector2D(1.f, 1.f))
					.ShadowColorAndOpacity(FLinearColor(0.04f, 0.02f, 0.01f, 0.92f))
					.Justification(ETextJustify::Center)
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

	RefreshFriendListVisualState();
	return Root;
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
		PendingViewportSize = CurrentViewportSize;
		PendingViewportStableTime = 0.f;
		return;
	}

	if (PendingViewportSize.IsNearlyZero() || PendingViewportSize.Equals(LastBuiltViewportSize, 1.0f))
	{
		return;
	}

	PendingViewportStableTime += InDeltaTime;
	if (PendingViewportStableTime >= 0.15f)
	{
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
			SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::MainMenu);
		}
	}

	// Subscribe to language changes
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66MainMenuScreen::OnLanguageChanged);
	}

	SyncToSharedPartyScreen();
}

void UT66MainMenuScreen::OnScreenDeactivated_Implementation()
{
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66MainMenuScreen::OnLanguageChanged);
	}

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

void UT66MainMenuScreen::NativeDestruct()
{
	ReleaseRetainedSlateState();
	Super::NativeDestruct();
}

void UT66MainMenuScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();

	if (ShouldRebuildRetainedSlate())
	{
		ForceRebuildSlate();
	}

	if (LeaderboardPanel.IsValid())
	{
		LeaderboardPanel->SetUIManager(UIManager);
	}
}

void UT66MainMenuScreen::ReleaseRetainedSlateState()
{
	LeaderboardPanel.Reset();
	SkyBackgroundBrush.Reset();
	SkyBackgroundTexture.Reset();
	FireMoonBrush.Reset();
	FireMoonTexture.Reset();
	PyramidChadBrush.Reset();
	PyramidChadTexture.Reset();
	ProfileAvatarBrush.Reset();
	PrimaryCTAFillBrush.Reset();
	PrimaryCTAFillTexture.Reset();
	SettingsIconBrush.Reset();
	LanguageIconBrush.Reset();
	FriendPortraitBrushes.Reset();
	PartyPortraitBrushes.Reset();
	FriendsListContainer.Reset();
	FriendGroupWidgetRefs.Reset();
	FriendRowWidgetRefs.Reset();
	FriendGroupsDividerBox.Reset();
	NoMatchingFriendsBox.Reset();
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

	SyncToSharedPartyScreen();
	ForceRebuildSlate();
}

void UT66MainMenuScreen::HandleSessionStateChanged()
{
	SyncToSharedPartyScreen();
	ForceRebuildSlate();
}

void UT66MainMenuScreen::HandleFriendSearchTextChanged(const FText& NewText)
{
	FriendSearchQuery = NewText.ToString();
	RefreshFriendListVisualState();
}

void UT66MainMenuScreen::RefreshFriendListVisualState()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PlayerSettings = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;

	const FString TrimmedQuery = FriendSearchQuery.TrimStartAndEnd();
	const bool bSearchActive = !TrimmedQuery.IsEmpty();
	int32 OnlineVisibleCount = 0;
	int32 OfflineVisibleCount = 0;

	for (FFriendRowWidgetRefs& RowRefs : FriendRowWidgetRefs)
	{
		const bool bMatches = DoesFriendMatchSearchQuery(FriendSearchQuery, RowRefs.FriendName);
		const bool bExpanded = RowRefs.bOnline ? bShowOnlineFriends : bShowOfflineFriends;
		const bool bVisible = bMatches && (bSearchActive || bExpanded);
		if (RowRefs.RootBox.IsValid())
		{
			RowRefs.RootBox->SetVisibility(bVisible ? EVisibility::Visible : EVisibility::Collapsed);
		}

		if (!bMatches)
		{
			continue;
		}

		if (RowRefs.bOnline)
		{
			++OnlineVisibleCount;
		}
		else
		{
			++OfflineVisibleCount;
		}

		const bool bFriendInParty = PartySubsystem && PartySubsystem->IsFriendInParty(RowRefs.PlayerId);
		const bool bInvitePending = !bFriendInParty && SessionSubsystem && SessionSubsystem->IsFriendInvitePending(RowRefs.PlayerId);
		const bool bCanInvite = PartySubsystem
			&& RowRefs.bOnline
			&& !bFriendInParty
			&& !bInvitePending
			&& PartySubsystem->GetPartyMemberCount() < 4;
		const bool bFavorite = PlayerSettings && PlayerSettings->IsFavoriteFriend(RowRefs.PlayerId);

		if (RowRefs.RowBorder.IsValid())
		{
			RowRefs.RowBorder->SetBorderBackgroundColor(
				bFriendInParty
					? FLinearColor(0.10f, 0.14f, 0.18f, 0.55f)
					: FLinearColor(0.f, 0.f, 0.f, 0.20f));
		}

		if (RowRefs.StatusText.IsValid())
		{
			RowRefs.StatusText->SetText(bFriendInParty ? NSLOCTEXT("T66.MainMenu", "InParty", "In Party") : FText::FromString(RowRefs.BaseStatus));
			RowRefs.StatusText->SetColorAndOpacity(
				bFriendInParty
					? FLinearColor(0.50f, 0.88f, 0.55f, 1.0f)
					: (RowRefs.bOnline
						? FLinearColor(0.50f, 0.88f, 0.55f, 1.0f)
						: FLinearColor(0.56f, 0.59f, 0.63f, 1.0f)));
		}

		if (RowRefs.FavoriteButton.IsValid())
		{
			RowRefs.FavoriteButton->SetToolTipText(
				bFavorite
					? NSLOCTEXT("T66.MainMenu", "UnfavoriteFriendTooltip", "Remove favorite")
					: NSLOCTEXT("T66.MainMenu", "FavoriteFriendTooltip", "Favorite friend"));
		}

		if (RowRefs.FavoriteGlyphText.IsValid())
		{
			RowRefs.FavoriteGlyphText->SetText(FText::FromString(bFavorite ? TEXT("\u2605") : TEXT("\u2606")));
			RowRefs.FavoriteGlyphText->SetColorAndOpacity(
				bFavorite
					? FLinearColor(0.94f, 0.78f, 0.28f, 1.0f)
					: FLinearColor(0.55f, 0.58f, 0.62f, 1.0f));
		}

		if (RowRefs.ActionButton.IsValid())
		{
			RowRefs.ActionButton->SetEnabled(bCanInvite);
		}

		if (RowRefs.ActionText.IsValid())
		{
			FText ActionText = NSLOCTEXT("T66.MainMenu", "InviteFriend", "INVITE");
			if (bFriendInParty)
			{
				ActionText = NSLOCTEXT("T66.MainMenu", "InParty", "In Party");
			}
			else if (bInvitePending)
			{
				ActionText = NSLOCTEXT("T66.MainMenu", "InvitedFriend", "INVITED");
			}
			else if (!RowRefs.bOnline)
			{
				ActionText = NSLOCTEXT("T66.MainMenu", "FriendOffline", "OFFLINE");
			}
			else if (PartySubsystem && PartySubsystem->GetPartyMemberCount() >= 4)
			{
				ActionText = NSLOCTEXT("T66.MainMenu", "PartyFull", "PARTY FULL");
			}

			RowRefs.ActionText->SetText(ActionText);
			RowRefs.ActionText->SetColorAndOpacity(
				bCanInvite
					? FLinearColor(0.96f, 0.97f, 0.94f, 1.0f)
					: FLinearColor(0.88f, 0.90f, 0.84f, 0.72f));
		}

		if (RowRefs.ActionFillBorder.IsValid())
		{
			const FLinearColor FillColor = bInvitePending
				? FLinearColor(0.56f, 0.42f, 0.16f, 0.98f)
				: (!bCanInvite
					? FLinearColor(0.42f, 0.47f, 0.40f, 0.72f)
					: (RowRefs.bOnline && !bFriendInParty
						? FLinearColor(0.18f, 0.31f, 0.18f, 0.97f)
						: FLinearColor(0.27f, 0.30f, 0.34f, 0.94f)));
			RowRefs.ActionFillBorder->SetBorderBackgroundColor(FillColor);
		}
	}

	for (const FFriendGroupWidgetRefs& GroupRefs : FriendGroupWidgetRefs)
	{
		const int32 VisibleCount = GroupRefs.bOnlineGroup ? OnlineVisibleCount : OfflineVisibleCount;
		if (GroupRefs.RootBox.IsValid())
		{
			GroupRefs.RootBox->SetVisibility((!bSearchActive || VisibleCount > 0) ? EVisibility::Visible : EVisibility::Collapsed);
		}
		if (GroupRefs.CountText.IsValid())
		{
			GroupRefs.CountText->SetText(FText::Format(
				NSLOCTEXT("T66.MainMenu", "FriendsGroupCount", "({0})"),
				FText::AsNumber(VisibleCount)));
		}
		if (GroupRefs.ExpandArrowImage.IsValid())
		{
			const bool bExpanded = GroupRefs.bOnlineGroup ? bShowOnlineFriends : bShowOfflineFriends;
			GroupRefs.ExpandArrowImage->SetRenderTransform(
				bExpanded
					? FSlateRenderTransform(FTransform2D())
					: FSlateRenderTransform(FTransform2D(FQuat2D(FMath::DegreesToRadians(90.f)))));
		}
	}

	if (FriendGroupsDividerBox.IsValid())
	{
		FriendGroupsDividerBox->SetVisibility((!bSearchActive || (OnlineVisibleCount > 0 && OfflineVisibleCount > 0))
			? EVisibility::Visible
			: EVisibility::Collapsed);
	}

	if (NoMatchingFriendsBox.IsValid())
	{
		NoMatchingFriendsBox->SetVisibility((bSearchActive && (OnlineVisibleCount + OfflineVisibleCount) == 0)
			? EVisibility::Visible
			: EVisibility::Collapsed);
	}

	if (FriendsListContainer.IsValid())
	{
		FriendsListContainer->Invalidate(EInvalidateWidget::Layout);
	}
}

void UT66MainMenuScreen::SyncToSharedPartyScreen()
{
	if (!UIManager)
	{
		return;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI)
	{
		return;
	}

	UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>();
	if (!SessionSubsystem || !SessionSubsystem->IsPartyLobbyContextActive() || SessionSubsystem->IsLocalPlayerPartyHost())
	{
		return;
	}

	const ET66ScreenType DesiredScreen = SessionSubsystem->GetDesiredPartyFrontendScreen();
	if ((DesiredScreen == ET66ScreenType::HeroSelection || DesiredScreen == ET66ScreenType::MainMenu)
		&& UIManager->GetCurrentScreenType() != DesiredScreen)
	{
		UIManager->ShowScreen(DesiredScreen);
	}
}

void UT66MainMenuScreen::RequestBackgroundTexture()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	SetupMainMenuLayerBrush(
		SkyBackgroundBrush,
		SkyBackgroundTexture,
		TexPool,
		this,
		TEXT("/Game/UI/MainMenu/sky_bg.sky_bg"),
		TEXT("/Game/UI/MainMenu/sky_bg"),
		TEXT("SourceAssets/UI/MainMenu/Animated/sky_bg.png"),
		FName(TEXT("MainMenuSkyBg")));

	SetupMainMenuLayerBrush(
		FireMoonBrush,
		FireMoonTexture,
		TexPool,
		this,
		TEXT("/Game/UI/MainMenu/fire_moon.fire_moon"),
		TEXT("/Game/UI/MainMenu/fire_moon"),
		TEXT("SourceAssets/UI/MainMenu/Animated/fire_moon.png"),
		FName(TEXT("MainMenuFireMoon")));

	SetupMainMenuLayerBrush(
		PyramidChadBrush,
		PyramidChadTexture,
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
		static TMap<FString, TWeakObjectPtr<UTexture2D>> CachedGeneratedIcons;

		if (!IconBrush.IsValid())
		{
			for (const FString& IconPath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (TWeakObjectPtr<UTexture2D>* CachedTexture = CachedGeneratedIcons.Find(IconPath))
				{
					if (CachedTexture->IsValid())
					{
						IconBrush = MakeShared<FSlateBrush>();
						EnsureRuntimeImageBrush(IconBrush, FVector2D(24.f, 24.f));
						IconBrush->SetResourceObject(CachedTexture->Get());
						IconBrush->TintColor = FSlateColor(FLinearColor::White);
						break;
					}
				}

				if (!FPaths::FileExists(IconPath))
				{
					continue;
				}

				if (UTexture2D* IconTexture = T66RuntimeUITextureAccess::ImportFileTexture(
					IconPath,
					TextureFilter::TF_Trilinear,
					false,
					TEXT("MainMenuGeneratedIcon")))
				{
					IconBrush = MakeShared<FSlateBrush>();
					EnsureRuntimeImageBrush(IconBrush, FVector2D(24.f, 24.f));
					IconBrush->SetResourceObject(IconTexture);
					IconBrush->TintColor = FSlateColor(FLinearColor::White);
					CachedGeneratedIcons.Add(IconPath, IconTexture);
					break;
				}
			}
		}
	};

	LoadGeneratedIcon(TEXT("RuntimeDependencies/T66/UI/MainMenu/mainmenu-settings-icon-slate.png"), SettingsIconBrush);
	LoadGeneratedIcon(TEXT("RuntimeDependencies/T66/UI/MainMenu/mainmenu-language-icon-slate.png"), LanguageIconBrush);
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
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (SessionSubsystem->IsPartyLobbyContextActive() && !SessionSubsystem->IsLocalPlayerPartyHost())
			{
				return;
			}

			if (SessionSubsystem->IsPartyLobbyContextActive())
			{
				SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::HeroSelection, true);
			}
		}

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
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (SessionSubsystem->IsPartyLobbyContextActive() && !SessionSubsystem->IsLocalPlayerPartyHost())
			{
				return;
			}
		}

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
