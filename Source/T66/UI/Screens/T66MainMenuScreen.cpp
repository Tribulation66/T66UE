// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MainMenuScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Components/T66LeaderboardPanel.h"
#include "UI/Style/T66ReferenceLayout.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
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
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/Layout/SScaleBox.h"
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
	void EnsureMainMenuRuntimeImageBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
	}

	void ConfigureMainMenuBoxBrush(const TSharedPtr<FSlateBrush>& Brush, const FMargin& Margin)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Box;
		Brush->Margin = Margin;
	}

	void SetupT66MainMenuRuntimeImageBrush(
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

		EnsureMainMenuRuntimeImageBrush(Brush, ImageSize);

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

	class ST66MainMenuPlateButton : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66MainMenuPlateButton)
			: _NormalBrush(nullptr)
			, _HoverBrush(nullptr)
			, _PressedBrush(nullptr)
			, _DisabledBrush(nullptr)
			, _ContentPadding(FMargin(0.f))
			, _ButtonStyle(nullptr)
		{}
			SLATE_ARGUMENT(const FSlateBrush*, NormalBrush)
			SLATE_ARGUMENT(const FSlateBrush*, HoverBrush)
			SLATE_ARGUMENT(const FSlateBrush*, PressedBrush)
			SLATE_ARGUMENT(const FSlateBrush*, DisabledBrush)
			SLATE_ARGUMENT(FMargin, ContentPadding)
			SLATE_ARGUMENT(const FButtonStyle*, ButtonStyle)
			SLATE_ARGUMENT(FText, ToolTipText)
			SLATE_EVENT(FOnClicked, OnClicked)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			NormalBrush = InArgs._NormalBrush;
			HoverBrush = InArgs._HoverBrush;
			PressedBrush = InArgs._PressedBrush;
			DisabledBrush = InArgs._DisabledBrush;
			ContentPadding = InArgs._ContentPadding;
			ButtonStyle = InArgs._ButtonStyle
				? *InArgs._ButtonStyle
				: FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			ButtonStyle
				.SetNormalPadding(FMargin(0.f))
				.SetPressedPadding(FMargin(0.f));

			ChildSlot
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						InArgs._OnClicked,
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(this, &ST66MainMenuPlateButton::GetCurrentBrush)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
							.Padding(this, &ST66MainMenuPlateButton::GetContentPadding)
							.RenderTransform(this, &ST66MainMenuPlateButton::GetContentTransform)
							.RenderTransformPivot(FVector2D(0.5f, 0.5f))
							[
								InArgs._Content.Widget
							]
						])
					.SetButtonStyle(&ButtonStyle)
					.SetPadding(FMargin(0.f)),
					&Button)
			];
			if (Button.IsValid())
			{
				Button->SetToolTipText(InArgs._ToolTipText);
			}
		}

	private:
		const FSlateBrush* GetCurrentBrush() const
		{
			if (!Button.IsValid() || !Button->IsEnabled())
			{
				return DisabledBrush ? DisabledBrush : (NormalBrush ? NormalBrush : FCoreStyle::Get().GetBrush("WhiteBrush"));
			}

			if (Button->IsPressed() && PressedBrush)
			{
				return PressedBrush;
			}

			if (Button->IsHovered() && HoverBrush)
			{
				return HoverBrush;
			}

			return NormalBrush ? NormalBrush : FCoreStyle::Get().GetBrush("WhiteBrush");
		}

		FMargin GetContentPadding() const
		{
			if (Button.IsValid() && Button->IsPressed())
			{
				return FMargin(
					ContentPadding.Left,
					ContentPadding.Top + 2.f,
					ContentPadding.Right,
					FMath::Max(0.f, ContentPadding.Bottom - 2.f));
			}

			return ContentPadding;
		}

		TOptional<FSlateRenderTransform> GetContentTransform() const
		{
			if (!Button.IsValid())
			{
				return TOptional<FSlateRenderTransform>();
			}

			if (Button->IsPressed())
			{
				return FSlateRenderTransform(FVector2D(0.f, 1.f));
			}

			if (Button->IsHovered())
			{
				return FSlateRenderTransform(FVector2D(0.f, -1.f));
			}

			return TOptional<FSlateRenderTransform>();
		}

		const FSlateBrush* NormalBrush = nullptr;
		const FSlateBrush* HoverBrush = nullptr;
		const FSlateBrush* PressedBrush = nullptr;
		const FSlateBrush* DisabledBrush = nullptr;
		FMargin ContentPadding = FMargin(0.f);
		FButtonStyle ButtonStyle;
		TSharedPtr<SButton> Button;
	};
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

		if (const UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			StateHash = HashCombine(StateHash, GetTypeHash(Achievements->GetAccountLevel()));
			StateHash = HashCombine(StateHash, GetTypeHash(Achievements->GetAccountExperienceIntoLevel()));
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
	RequestMainMenuChromeBrushes();
	RequestCTAButtonBrushes();
	CachedViewportSize = GetEffectiveFrontendViewportSize();
	LastBuiltViewportSize = CachedViewportSize;
	PendingViewportSize = CachedViewportSize;
	PendingViewportStableTime = 0.f;
	bViewportResponsiveRebuildQueued = false;
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
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

	const FText NewGameText = NSLOCTEXT("T66.MainMenu", "Start", "NEW GAME");
	const FText LoadGameText = NSLOCTEXT("T66.MainMenu", "Continue", "LOAD GAME");
	const FText TaglineText = NSLOCTEXT("T66.MainMenu", "Tagline", "If you're not Chad it's over");

	struct FMenuFriendEntry
	{
		FString PlayerId;
		FString Name;
		int32 Level = 1;
		bool bOnline = false;
		bool bFavorite = false;
		bool bInvitePending = false;
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
				1,
				Friend.bOnline,
				PlayerSettings ? PlayerSettings->IsFavoriteFriend(Friend.PlayerId) : false,
				SessionSubsystem ? SessionSubsystem->IsFriendInvitePending(Friend.PlayerId) : false
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
	if (UTexture2D* LocalAvatarTexture = SteamHelper ? SteamHelper->GetLocalAvatarTexture() : nullptr)
	{
		ProfileAvatarBrush->SetResourceObject(LocalAvatarTexture);
	}
	else
	{
		ProfileAvatarBrush->SetResourceObject(nullptr);
	}

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
			Brush->SetResourceObject(nullptr);
		}
		FriendPortraitBrushes.Add(Brush);
	}

	const FLinearColor ShellFill(0.004f, 0.005f, 0.010f, 0.985f);
	const FLinearColor BrightText(0.86f, 0.87f, 0.89f, 1.0f);
	const FLinearColor HeaderText(0.66f, 0.68f, 0.71f, 1.0f);
	const FLinearColor MutedText(0.50f, 0.52f, 0.56f, 1.0f);
	const FLinearColor OfflineNameText(0.40f, 0.41f, 0.45f, 1.0f);
	const FLinearColor OnlineHeaderText(0.42f, 0.67f, 0.96f, 1.0f);
	const FLinearColor DividerColor(0.20f, 0.22f, 0.25f, 0.48f);
	const FLinearColor AvatarAccentOnline(0.13f, 0.22f, 0.30f, 1.0f);
	const FLinearColor AvatarAccentOffline(0.08f, 0.09f, 0.11f, 1.0f);
	const FLinearColor LeaderSlotAccent(0.29f, 0.24f, 0.13f, 1.0f);
	const FLinearColor PartySlotAccent(0.15f, 0.17f, 0.19f, 1.0f);
	const int32 LeftPanelTitleFontSize = 18;
	const int32 FriendsPanelBodyFontSize = 18;
	const int32 FriendsPanelHeaderFontSize = 20;

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
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						FOnClicked::CreateLambda([this, bOnlineGroup]()
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
						}),
						SNew(SBox)
						.WidthOverride(16.f)
						.HeightOverride(16.f)
						[
							SAssignNew(ExpandArrowImage, SImage)
							.Image(DownArrowBrush)
							.ColorAndOpacity(HeaderText)
							.RenderTransformPivot(FVector2D(0.5f, 0.5f))
							.RenderTransform(
								bExpanded
									? FSlateRenderTransform(FTransform2D())
									: FSlateRenderTransform(FTransform2D(FQuat2D(FMath::DegreesToRadians(90.f)))))
						])
					.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
					.SetPadding(FMargin(0.f)))
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
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						OnClicked,
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
						])
					.SetButtonStyle(&NoBorderButtonStyle)
					.SetPadding(FMargin(0.f))
					.SetEnabled(IsEnabled))
			];
	};

	auto MakeFriendRow = [&, BrightText, MutedText, OfflineNameText, AvatarAccentOnline, AvatarAccentOffline](
		const FMenuFriendEntry& Friend,
		int32 FriendIndex) -> TSharedRef<SWidget>
	{
		const FLinearColor NameColor = Friend.bOnline ? BrightText : OfflineNameText;
		const FLinearColor AccentColor = Friend.bOnline ? AvatarAccentOnline : AvatarAccentOffline;
		const FText FriendLevelText = FText::Format(
			NSLOCTEXT("T66.MainMenu", "FriendLevelFormat", "Level {0}/{1}"),
			FText::AsNumber(FMath::Clamp(Friend.Level, 1, UT66AchievementsSubsystem::AccountMaxLevel)),
			FText::AsNumber(UT66AchievementsSubsystem::AccountMaxLevel));
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
			return FText::FromString(TEXT("\u2606"));
		};
		auto ResolveFavoriteGlyphColor = [PlayerSettings, PlayerId = Friend.PlayerId]() -> FSlateColor
		{
			const bool bFavorite = PlayerSettings && PlayerSettings->IsFavoriteFriend(PlayerId);
			return FSlateColor(bFavorite
				? FLinearColor(0.94f, 0.78f, 0.28f, 1.0f)
				: FLinearColor(0.72f, 0.58f, 0.88f, 0.92f));
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
		auto ShouldShowActionOverlayText = []() -> bool
		{
			return true;
		};
		TSharedPtr<SBox> RowRootBox;
		TSharedPtr<SBorder> RowBorder;
		TSharedPtr<STextBlock> LevelTextWidget;
		TSharedPtr<SButton> FavoriteButton;
		TSharedPtr<STextBlock> FavoriteGlyphText;
		TSharedPtr<SButton> ActionButton;
		TSharedPtr<SBorder> ActionFillBorder;
		TSharedPtr<STextBlock> ActionTextWidget;

		const TSharedRef<SWidget> RowContent =
			SAssignNew(RowBorder, SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
			.BorderBackgroundColor(FLinearColor::Transparent)
			.Padding(6.f, 4.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(T66MainMenuReferenceLayout::Left::FriendAvatarFrameSource.Width)
					.HeightOverride(T66MainMenuReferenceLayout::Left::FriendAvatarFrameSource.Height)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(FriendAvatarFrameBrush.Get())
							.ColorAndOpacity(FriendAvatarFrameBrush.IsValid() ? FLinearColor::White : AccentColor)
						]
						+ SOverlay::Slot()
						.Padding(FMargin(4.f))
						[
							SNew(SImage)
							.Image(FriendPortraitBrushes.IsValidIndex(FriendIndex) ? FriendPortraitBrushes[FriendIndex].Get() : nullptr)
							.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Friend.bOnline ? 1.0f : 0.74f))
						]
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(8.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Friend.Name))
						.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize + 1))
						.ColorAndOpacity(NameColor)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f)
					[
						SAssignNew(LevelTextWidget, STextBlock)
						.Text(FriendLevelText)
						.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize - 1))
						.ColorAndOpacity(Friend.bOnline ? MutedText : OfflineNameText)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(T66MainMenuReferenceLayout::Left::FriendStarButton.Width)
					.HeightOverride(T66MainMenuReferenceLayout::Left::FriendStarButton.Height)
					[
						FT66Style::MakeBareButton(
							FT66BareButtonParams(
								FOnClicked::CreateLambda([this, PlayerSettings, PlayerId = Friend.PlayerId]()
								{
									if (PlayerSettings)
									{
										PlayerSettings->SetFavoriteFriend(PlayerId, !PlayerSettings->IsFavoriteFriend(PlayerId));
										RefreshFriendListVisualState();
									}
									return FReply::Handled();
								}),
								SNew(SOverlay)
								+ SOverlay::Slot()
								.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SAssignNew(FavoriteGlyphText, STextBlock)
								.Text(ResolveFavoriteGlyph())
								.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize + 21))
								.ColorAndOpacity(ResolveFavoriteGlyphColor())
									.Visibility(EVisibility::Visible)
									.Justification(ETextJustify::Center)
								]
							)
							.SetButtonStyle(&NoBorderButtonStyle)
							.SetPadding(FMargin(0.f)),
							&FavoriteButton)
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(T66MainMenuReferenceLayout::Left::FriendInviteButton.Width)
					.HeightOverride(T66MainMenuReferenceLayout::Left::FriendInviteButton.Height)
					[
						FT66Style::MakeBareButton(
							FT66BareButtonParams(
								FOnClicked::CreateLambda([this, PartySubsystem, PlayerId = Friend.PlayerId, PlayerName = Friend.Name]()
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
								}),
								SAssignNew(ActionFillBorder, SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
								.BorderBackgroundColor(FLinearColor::Transparent)
							.Padding(FMargin(0.f))
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									SNew(SImage)
									.Image(TAttribute<const FSlateBrush*>::CreateLambda([this, CanInviteFriend]() -> const FSlateBrush*
									{
										return CanInviteFriend() ? FriendInviteButtonBrush.Get() : FriendOfflineButtonBrush.Get();
									}))
								]
								+ SOverlay::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.Padding(FMargin(8.f, 6.f, 8.f, 6.f))
								[
									SAssignNew(ActionTextWidget, STextBlock)
									.Text(ResolveActionText())
									.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
									.ColorAndOpacity(CanInviteFriend()
										? FLinearColor(0.96f, 0.97f, 0.94f, 1.0f)
										: FLinearColor(0.88f, 0.90f, 0.84f, 0.72f))
									.Visibility(ShouldShowActionOverlayText() ? EVisibility::Visible : EVisibility::Collapsed)
										.Justification(ETextJustify::Center)
									]
								]
							)
							.SetButtonStyle(&NoBorderButtonStyle)
							.SetPadding(FMargin(0.f))
							.SetEnabled(CanInviteFriend()),
							&ActionButton)
					]
				]
			];
		if (FavoriteButton.IsValid())
		{
			FavoriteButton->SetToolTipText((PlayerSettings && PlayerSettings->IsFavoriteFriend(Friend.PlayerId))
				? UnfavoriteFriendTooltip
				: FavoriteFriendTooltip);
		}

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
		RowRefs.Level = Friend.Level;
		RowRefs.bOnline = Friend.bOnline;
		RowRefs.RootBox = RowRootBox;
		RowRefs.RowBorder = RowBorder;
		RowRefs.LevelText = LevelTextWidget;
		RowRefs.FavoriteButton = FavoriteButton;
		RowRefs.FavoriteGlyphText = FavoriteGlyphText;
		RowRefs.ActionButton = ActionButton;
		RowRefs.ActionFillBorder = ActionFillBorder;
		RowRefs.ActionText = ActionTextWidget;

		return RowRoot;
	};

	const TSharedRef<SWidget> LeaderboardWidget =
		SAssignNew(LeaderboardPanel, ST66LeaderboardPanel)
		.LocalizationSubsystem(Loc)
		.LeaderboardSubsystem(LB)
		.UIManager(UIManager)
		.ReferenceMirrorMode(true);

	const TSharedRef<SWidget> LeaderboardShell = LeaderboardWidget;

	const FText FriendSearchHintText = NSLOCTEXT("T66.MainMenu", "FriendSearchHint", "Search friends...");
	const FText NoMatchingFriendsText = NSLOCTEXT("T66.MainMenu", "NoMatchingFriends", "No friends match this search.");

	const TSharedRef<SWidget> FriendSearchBox =
		SNew(SBox)
		.WidthOverride(T66MainMenuReferenceLayout::Left::SearchFieldReference.Width)
		.HeightOverride(T66MainMenuReferenceLayout::Left::SearchFieldReference.Height)
		.Clipping(EWidgetClipping::ClipToBounds)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(SearchFieldShellBrush.Get())
				.ColorAndOpacity(SearchFieldShellBrush.IsValid() ? FLinearColor::White : FLinearColor(0.16f, 0.18f, 0.21f, 0.94f))
			]
			+ SOverlay::Slot()
			.Padding(FMargin(10.f, 7.f, 42.f, 7.f))
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(FriendSearchQuery))
				.OnTextChanged_UObject(this, &UT66MainMenuScreen::HandleFriendSearchTextChanged)
				.HintText(FriendSearchHintText)
				.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
				.ForegroundColor(BrightText)
				.BackgroundColor(FLinearColor::Transparent)
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

	auto MakePartyMemberSlot = [this, LeaderSlotAccent, PartySlotAccent](const FMenuPartyEntry* Member) -> TSharedRef<SWidget>
	{
		const bool bOccupied = Member != nullptr;
		const bool bHost = bOccupied && Member->bIsPartyHost;
		const float SlotWidth = T66MainMenuReferenceLayout::Left::PartySlotSource.Width;
		const float SlotHeight = T66MainMenuReferenceLayout::Left::PartySlotSource.Height;
		const FText TooltipText = bOccupied
			? FText::FromString(Member->DisplayName)
			: FText::GetEmpty();

		return SNew(SBox)
			.WidthOverride(SlotWidth)
			.HeightOverride(SlotHeight)
			[
				SNew(SOverlay)
				.ToolTipText(TooltipText)
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image(PartySlotFrameBrush.Get())
					.ColorAndOpacity(PartySlotFrameBrush.IsValid()
						? FLinearColor::White
						: (bHost ? LeaderSlotAccent : PartySlotAccent))
				]
				+ SOverlay::Slot()
				.Padding(FMargin(11.f))
				[
					SNew(SImage)
					.Image((bOccupied && Member->AvatarBrush.IsValid()) ? Member->AvatarBrush.Get() : nullptr)
					.ColorAndOpacity(bOccupied ? FLinearColor::White : FLinearColor(1.f, 1.f, 1.f, 0.f))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(38.f)
					.HeightOverride(38.f)
					[
						SNew(SImage)
						.Image(PartyPlusIconBrush.Get())
						.ColorAndOpacity(PartyPlusIconBrush.IsValid()
							? FLinearColor::White
							: FLinearColor(0.62f, 0.62f, 0.60f, 0.90f))
						.Visibility(bOccupied ? EVisibility::Collapsed : EVisibility::Visible)
					]
				]
			];
	};

	constexpr int32 MaxPartySlots = 4;
	TSharedRef<SHorizontalBox> PartySlots = SNew(SHorizontalBox);
	for (int32 SlotIndex = 0; SlotIndex < MaxPartySlots; ++SlotIndex)
	{
		PartySlots->AddSlot()
		.AutoWidth()
		.Padding(SlotIndex == 0 ? FMargin(0.f) : FMargin(6.f, 0.f, 0.f, 0.f))
		[
			MakePartyMemberSlot(PartyEntries.IsValidIndex(SlotIndex) ? &PartyEntries[SlotIndex] : nullptr)
		];
	}

	const FString LocalSteamName = SteamHelper ? SteamHelper->GetLocalDisplayName() : FString();
	const FText ProfileNameText = !LocalSteamName.IsEmpty()
		? FText::FromString(LocalSteamName)
		: NSLOCTEXT("T66.MainMenu", "ProfileNameFallback", "Local Player");
	const int32 ProfileLevel = Achievements ? Achievements->GetAccountLevel() : 1;
	const int32 ProfileMaxLevel = Achievements ? Achievements->GetAccountMaxLevel() : UT66AchievementsSubsystem::AccountMaxLevel;
	const int32 ProfileNextLevel = Achievements ? Achievements->GetAccountNextLevel() : 2;
	const float ProfileLevelProgress = Achievements ? Achievements->GetAccountLevelProgress01() : 0.f;
	const FText ProfileLevelText = FText::Format(
		NSLOCTEXT("T66.MainMenu", "ProfileLevelFormat", "Level {0}/{1}"),
		FText::AsNumber(ProfileLevel),
		FText::AsNumber(ProfileMaxLevel));
	const FText ProfileNextLevelText = FText::Format(
		NSLOCTEXT("T66.MainMenu", "ProfileNextLevelFormat", "Level {0}"),
		FText::AsNumber(ProfileNextLevel));

	auto MakeProfileProgressBar = [this, ProfileLevelProgress]() -> TSharedRef<SWidget>
	{
		const float Pct = FMath::Clamp(ProfileLevelProgress > KINDA_SMALL_NUMBER ? ProfileLevelProgress : 0.58f, 0.f, 1.f);
		const FSlateBrush* ShellBrush = ProfileProgressShellBrush.IsValid()
			? ProfileProgressShellBrush.Get()
			: FCoreStyle::Get().GetBrush("WhiteBrush");
		const FSlateBrush* FillBrush = ProfileProgressFillBrush.IsValid()
			? ProfileProgressFillBrush.Get()
			: FCoreStyle::Get().GetBrush("WhiteBrush");

		return SNew(SBorder)
			.BorderImage(ShellBrush)
			.BorderBackgroundColor(ProfileProgressShellBrush.IsValid() ? FLinearColor::White : FLinearColor(0.02f, 0.02f, 0.04f, 1.0f))
			.Padding(FMargin(4.f, 3.f))
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				SNew(SBox)
				.HeightOverride(12.f)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(FMath::Max(Pct, 0.001f))
					[
						SNew(SBorder)
						.Visibility(Pct > 0.001f ? EVisibility::Visible : EVisibility::Collapsed)
						.BorderImage(FillBrush)
						.BorderBackgroundColor(ProfileProgressFillBrush.IsValid() ? FLinearColor::White : FLinearColor(0.14f, 0.02f, 0.36f, 1.0f))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(FMath::Max(1.f - Pct, 0.001f))
					[
						SNew(SSpacer)
					]
				]
			];
	};

	const TSharedRef<SWidget> ProfileCardContent =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(88.f)
			.HeightOverride(81.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					ProfileAvatarFrameBrush.IsValid()
						? StaticCastSharedRef<SWidget>(
							SNew(SImage)
							.Image(ProfileAvatarFrameBrush.Get()))
						: StaticCastSharedRef<SWidget>(SNew(SSpacer))
				]
				+ SOverlay::Slot()
				.Padding(FMargin(8.f, 7.f, 11.f, 9.f))
				[
					SNew(SImage)
					.Image(ProfileAvatarBrush.Get())
					.ColorAndOpacity(TAttribute<FSlateColor>::CreateLambda([this]() -> FSlateColor
					{
						const bool bHasProfileAvatar = ProfileAvatarBrush.IsValid()
							&& ProfileAvatarBrush->GetResourceObject() != nullptr;
						return bHasProfileAvatar
							? FSlateColor(FLinearColor::White)
							: FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
					}))
				]
			]
		]
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(12.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(ProfileNameText)
				.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize + 8))
				.ColorAndOpacity(BrightText)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(ProfileLevelText)
				.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize))
				.ColorAndOpacity(FLinearColor(0.72f, 0.28f, 0.95f, 1.0f))
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(SBox)
					[
						MakeProfileProgressBar()
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(9.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(ProfileNextLevelText)
					.Font(FT66Style::MakeFont(TEXT("Regular"), FriendsPanelBodyFontSize - 2))
					.ColorAndOpacity(MutedText)
				]
			]
		];

	const TSharedRef<SWidget> ProfileCard =
		SNew(SBox)
		.WidthOverride(T66MainMenuReferenceLayout::Left::ProfileCardReference.Width)
		.HeightOverride(T66MainMenuReferenceLayout::Left::ProfileCardReference.Height)
		.Clipping(EWidgetClipping::ClipToBounds)
		[
			FT66Style::MakeBareButton(
				FT66BareButtonParams(
					FOnClicked::CreateLambda([this]()
					{
						OnAccountStatusClicked();
						return FReply::Handled();
					}),
					SNew(SOverlay)
					+ SOverlay::Slot()
					.Padding(FMargin(0.f, 8.f, 0.f, 8.f))
					[
						ProfileCardContent
					])
				.SetButtonStyle(&NoBorderButtonStyle)
				.SetPadding(FMargin(0.f))
				.SetToolTipText(NSLOCTEXT("T66.MainMenu", "OpenProfileTooltip", "Open your account page")))
		];

	const TSharedRef<SWidget> LeftSocialContent =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SScrollBox)
			.ScrollBarVisibility(EVisibility::Collapsed)
			.ScrollBarThickness(FVector2D::ZeroVector)
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
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
			[
				PartySlots
			]
		];

	const FVector2D ReferenceCanvasSize(T66MainMenuReferenceLayout::CanvasWidth, T66MainMenuReferenceLayout::CanvasHeight);
	auto MakeCenterActionButton = [this, &NoBorderButtonStyle](
		const FText& Text,
		FReply (UT66MainMenuScreen::*ClickFunc)(),
		const FButtonStateBrushSet& BrushSet,
		float ReferenceWidth,
		float ReferenceHeight) -> TSharedRef<SWidget>
	{
		FSlateFontInfo CTAFont = FT66Style::MakeFont(TEXT("Bold"), 62);
		CTAFont.LetterSpacing = 1;

		return SNew(SBox)
			.WidthOverride(ReferenceWidth)
			.HeightOverride(ReferenceHeight)
			[
				SNew(ST66MainMenuPlateButton)
				.ButtonStyle(&NoBorderButtonStyle)
				.NormalBrush(BrushSet.NormalBrush.Get())
				.HoverBrush(BrushSet.HoverBrush.Get())
				.PressedBrush(BrushSet.PressedBrush.Get())
				.DisabledBrush(BrushSet.DisabledBrush.Get())
				.ToolTipText(FText::GetEmpty())
				.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
				.ContentPadding(FMargin(0.f))
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Text)
						.Font(CTAFont)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FLinearColor(0.97f, 0.94f, 0.88f, 1.f))
						.ShadowOffset(FVector2D(0.f, 1.f))
						.ShadowColorAndOpacity(FLinearColor(0.10f, 0.07f, 0.03f, 0.95f))
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
				]
			];
	};

	const TSharedRef<SWidget> LeftSocialShell =
		SNew(SBox)
		.WidthOverride(T66MainMenuReferenceLayout::MainMenu::LeftPanelAssembly.Width)
		.HeightOverride(T66MainMenuReferenceLayout::MainMenu::LeftPanelAssembly.Height)
		.Clipping(EWidgetClipping::ClipToBounds)
		[
			SNew(SOverlay)
			.Clipping(EWidgetClipping::ClipToBounds)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(LeftPanelShellBrush.Get())
			]
			+ SOverlay::Slot()
			.Padding(FMargin(22.f, 22.f, 22.f, 22.f))
			[
				SNew(SBox)
				.WidthOverride(T66MainMenuReferenceLayout::MainMenu::LeftPanelAssembly.Width - 44.f)
				.HeightOverride(T66MainMenuReferenceLayout::MainMenu::LeftPanelAssembly.Height - 44.f)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					LeftSocialContent
				]
			]
		];

	const FT66ReferenceRect& LeftPanelAssemblyRect = T66MainMenuReferenceLayout::MainMenu::LeftPanelAssembly;
	const FT66ReferenceRect& RightPanelAssemblyRect = T66MainMenuReferenceLayout::MainMenu::RightPanelAssembly;
	const FT66ReferenceRect& RightShellRect = T66MainMenuReferenceLayout::Right::ShellFullReference;
	const float RightLeaderboardFrameInset = 14.f;
	const FVector2D RightShellOffset(
		RightShellRect.X - RightPanelAssemblyRect.X,
		RightShellRect.Y - RightPanelAssemblyRect.Y);
	const FMargin RightLeaderboardContentPadding(
		RightShellOffset.X + RightLeaderboardFrameInset,
		RightShellOffset.Y + RightLeaderboardFrameInset,
		0.f,
		0.f);

	const TSharedRef<SWidget> RightLeaderboardShell =
		SNew(SBox)
		.WidthOverride(T66MainMenuReferenceLayout::MainMenu::RightPanelAssembly.Width)
		.HeightOverride(T66MainMenuReferenceLayout::MainMenu::RightPanelAssembly.Height)
		.Clipping(EWidgetClipping::ClipToBounds)
		[
			SNew(SOverlay)
			.Clipping(EWidgetClipping::ClipToBounds)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(RightShellOffset.X, RightShellOffset.Y, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(RightShellRect.Width)
				.HeightOverride(RightShellRect.Height)
				[
					SNew(SImage)
					.Image(RightPanelShellBrush.Get())
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(RightLeaderboardContentPadding)
			[
				SNew(SBox)
				.WidthOverride(RightShellRect.Width - (RightLeaderboardFrameInset * 2.f))
				.HeightOverride(RightShellRect.Height - (RightLeaderboardFrameInset * 2.f))
				[
					LeaderboardShell
				]
			]
		];

	const FT66ReferenceRect& TaglineRect = T66MainMenuReferenceLayout::Center::SubtitleLockup;
	const FT66ReferenceRect& TitleRect = T66MainMenuReferenceLayout::Center::TitleLockup;
	const FT66ReferenceRect& CtaStackRect = T66MainMenuReferenceLayout::Center::CtaStackFull;
	const FT66ReferenceRect& NewGamePlateRect = T66MainMenuReferenceLayout::Center::CtaButtonNewGame;
	const FT66ReferenceRect& LoadGamePlateRect = T66MainMenuReferenceLayout::Center::CtaButtonLoadGame;

	FSlateFontInfo TaglineFont = FT66Style::MakeFont(TEXT("Bold"), 33);
	TaglineFont.LetterSpacing = 1;
	TaglineFont.OutlineSettings.OutlineSize = 3;
	TaglineFont.OutlineSettings.OutlineColor = FLinearColor::Black;

	const TSharedRef<SWidget> CenterTagline =
		SNew(SBox)
		.WidthOverride(TaglineRect.Width)
		.HeightOverride(TaglineRect.Height)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
	[
		SNew(STextBlock)
		.Text(TaglineText)
		.Font(TaglineFont)
		.Justification(ETextJustify::Center)
		.ColorAndOpacity(FLinearColor(0.48f, 0.04f, 0.82f, 1.f))
		.ShadowOffset(FVector2D(0.f, 3.f))
		.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 1.f))
		.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		.Clipping(EWidgetClipping::ClipToBounds)
	];
	const TSharedRef<SWidget> NewGameButtonWidget =
		MakeCenterActionButton(NewGameText, &UT66MainMenuScreen::HandleNewGameClicked, NewGameButtonBrushes, NewGamePlateRect.Width, NewGamePlateRect.Height);
	const TSharedRef<SWidget> LoadGameButtonWidget =
		MakeCenterActionButton(LoadGameText, &UT66MainMenuScreen::HandleLoadGameClicked, LoadGameButtonBrushes, LoadGamePlateRect.Width, LoadGamePlateRect.Height);
	const FMargin NewGameButtonLocalOffset(
		NewGamePlateRect.X - CtaStackRect.X,
		NewGamePlateRect.Y - CtaStackRect.Y,
		NewGamePlateRect.Width,
		NewGamePlateRect.Height);
	const FMargin LoadGameButtonLocalOffset(
		LoadGamePlateRect.X - CtaStackRect.X,
		LoadGamePlateRect.Y - CtaStackRect.Y,
		LoadGamePlateRect.Width,
		LoadGamePlateRect.Height);
	const float RuntimeCtaStackHeight = (LoadGamePlateRect.Y - CtaStackRect.Y) + LoadGamePlateRect.Height;
	const TSharedRef<SWidget> CenterTitleArt =
		SNew(SBox)
		.WidthOverride(TitleRect.Width)
		.HeightOverride(TitleRect.Height)
		[
			SNew(SImage)
			.Image(TitleLockupBrush.Get())
		];
	const TSharedRef<SWidget> CenterCtaStack =
		SNew(SBox)
		.WidthOverride(CtaStackRect.Width)
		.HeightOverride(RuntimeCtaStackHeight)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SConstraintCanvas)
				+ SConstraintCanvas::Slot()
				.Alignment(FVector2D(0.f, 0.f))
				.Offset(NewGameButtonLocalOffset)
				[
					NewGameButtonWidget
				]
				+ SConstraintCanvas::Slot()
				.Alignment(FVector2D(0.f, 0.f))
				.Offset(LoadGameButtonLocalOffset)
				[
					LoadGameButtonWidget
				]
			]
		];

	const float RightPanelRightInset = ReferenceCanvasSize.X - RightPanelAssemblyRect.X - RightPanelAssemblyRect.Width;
	const float LeftPanelBottomInset = ReferenceCanvasSize.Y - LeftPanelAssemblyRect.Y - LeftPanelAssemblyRect.Height;
	const float RightPanelBottomInset = ReferenceCanvasSize.Y - RightPanelAssemblyRect.Y - RightPanelAssemblyRect.Height;
	const float CenterTitleLeft = TitleRect.X;
	const float CenterTaglineLeft = TaglineRect.X;
	const float CenterCtaStackLeft = CtaStackRect.X;
	const float LeftPanelTop = ReferenceCanvasSize.Y - LeftPanelBottomInset - LeftPanelAssemblyRect.Height;
	const float RightPanelLeft = ReferenceCanvasSize.X - RightPanelRightInset - RightPanelAssemblyRect.Width;
	const float RightPanelTop = ReferenceCanvasSize.Y - RightPanelBottomInset - RightPanelAssemblyRect.Height;
	const float CtaStackTop = CtaStackRect.Y;
	const TSharedRef<SWidget> ReferenceCanvas =
		SNew(SBox)
		.WidthOverride(ReferenceCanvasSize.X)
		.HeightOverride(ReferenceCanvasSize.Y)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				BuildMainMenuBackgroundWidget()
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(CenterTitleLeft, TitleRect.Y, 0.f, 0.f))
			[
				CenterTitleArt
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(CenterCtaStackLeft, CtaStackTop, 0.f, 0.f))
			[
				CenterCtaStack
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(LeftPanelAssemblyRect.X, LeftPanelTop, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(LeftPanelAssemblyRect.Width)
				.HeightOverride(LeftPanelAssemblyRect.Height)
				[
				SNew(SInvalidationPanel)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					LeftSocialShell
				]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(RightPanelLeft, RightPanelTop, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(RightPanelAssemblyRect.Width)
				.HeightOverride(RightPanelAssemblyRect.Height)
				[
				SNew(SInvalidationPanel)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					RightLeaderboardShell
				]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(CenterTaglineLeft, TaglineRect.Y, 0.f, 0.f))
			[
				CenterTagline
			]
		];

	TSharedRef<SBorder> Root =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		.Padding(0.f)
		[
			SNew(SDPIScaler)
			.DPIScale(TAttribute<float>::CreateLambda([]() -> float
			{
				return 1.f / FMath::Max(0.01f, FT66Style::GetEngineDPIScale());
			}))
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					ReferenceCanvas
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
	ForegroundOccluderBrush.Reset();
	ForegroundOccluderTexture.Reset();
	TitleLockupBrush.Reset();
	TitleLockupTexture.Reset();
	LeftPanelShellBrush.Reset();
	LeftPanelShellTexture.Reset();
	RightPanelShellBrush.Reset();
	RightPanelShellTexture.Reset();
	SearchFieldShellBrush.Reset();
	SearchFieldShellTexture.Reset();
	SearchIconBrush.Reset();
	SearchIconTexture.Reset();
	FriendInviteButtonBrush.Reset();
	FriendInviteButtonTexture.Reset();
	FriendOfflineButtonBrush.Reset();
	FriendOfflineButtonTexture.Reset();
	FriendAvatarFrameBrush.Reset();
	FriendAvatarFrameTexture.Reset();
	PartySlotFrameBrush.Reset();
	PartySlotFrameTexture.Reset();
	PartyPlusIconBrush.Reset();
	PartyPlusIconTexture.Reset();
	CloseButtonBrush.Reset();
	CloseButtonTexture.Reset();
	ProfileAvatarBrush.Reset();
	ProfileAvatarFrameBrush.Reset();
	ProfileAvatarFrameTexture.Reset();
	CenterStackFrameBrush.Reset();
	CenterStackFrameTexture.Reset();
	NewGameButtonBrushes = {};
	LoadGameButtonBrushes = {};
	DailyChallengeButtonBrushes = {};
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
			RowRefs.RowBorder->SetBorderBackgroundColor(FLinearColor::Transparent);
		}

		if (RowRefs.LevelText.IsValid())
		{
			RowRefs.LevelText->SetText(FText::Format(
				NSLOCTEXT("T66.MainMenu", "FriendLevelFormat", "Level {0}/{1}"),
				FText::AsNumber(FMath::Clamp(RowRefs.Level, 1, UT66AchievementsSubsystem::AccountMaxLevel)),
				FText::AsNumber(UT66AchievementsSubsystem::AccountMaxLevel)));
			RowRefs.LevelText->SetColorAndOpacity(
				RowRefs.bOnline
					? FLinearColor(0.50f, 0.52f, 0.56f, 1.0f)
					: FLinearColor(0.40f, 0.41f, 0.45f, 1.0f));
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
			RowRefs.FavoriteGlyphText->SetText(FText::FromString(TEXT("\u2606")));
			RowRefs.FavoriteGlyphText->SetColorAndOpacity(
				bFavorite
					? FLinearColor(0.94f, 0.78f, 0.28f, 1.0f)
					: FLinearColor(0.72f, 0.58f, 0.88f, 0.92f));
			RowRefs.FavoriteGlyphText->SetVisibility(EVisibility::Visible);
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
			RowRefs.ActionText->SetVisibility(EVisibility::Visible);
		}

		if (RowRefs.ActionFillBorder.IsValid())
		{
			RowRefs.ActionFillBorder->SetBorderBackgroundColor(FLinearColor::Transparent);
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

TSharedRef<SWidget> UT66MainMenuScreen::BuildMainMenuBackgroundWidget() const
{
	if (!SkyBackgroundBrush.IsValid() || !SkyBackgroundBrush->GetResourceObject())
	{
		return SNew(SImage).Image(SkyBackgroundBrush.Get());
	}

	TSharedRef<SOverlay> Background = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage).Image(SkyBackgroundBrush.Get())
		];

	if (ForegroundOccluderBrush.IsValid() && ForegroundOccluderBrush->GetResourceObject())
	{
		Background->AddSlot()
		[
			SNew(SImage).Image(ForegroundOccluderBrush.Get())
		];
	}

	return Background;
}

void UT66MainMenuScreen::RequestBackgroundTexture()
{
	SetupT66MainMenuRuntimeImageBrush(
		SkyBackgroundBrush,
		SkyBackgroundTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/ScreenArt/MainMenu/NewMM/main_menu_newmm_base_1920.png"),
		FVector2D(T66MainMenuReferenceLayout::CanvasWidth, T66MainMenuReferenceLayout::CanvasHeight));

	SetupT66MainMenuRuntimeImageBrush(
		ForegroundOccluderBrush,
		ForegroundOccluderTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/ScreenArt/MainMenu/NewMM/main_menu_newmm_foreground_occluder.png"),
		FVector2D(T66MainMenuReferenceLayout::CanvasWidth, T66MainMenuReferenceLayout::CanvasHeight));

	SetupT66MainMenuRuntimeImageBrush(
		TitleLockupBrush,
		TitleLockupTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/ScreenArt/MainMenu/main_menu_wordmark_imagegen_20260425_v1.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::TitleLockup.Width, T66MainMenuReferenceLayout::Center::TitleLockup.Height));

	SetupT66MainMenuRuntimeImageBrush(
		LeftPanelShellBrush,
		LeftPanelShellTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/panel_large_normal.png"),
		FVector2D(T66MainMenuReferenceLayout::Left::ShellFullReference.Width, T66MainMenuReferenceLayout::Left::ShellFullReference.Height));
	ConfigureMainMenuBoxBrush(LeftPanelShellBrush, FMargin(0.067f, 0.043f, 0.067f, 0.043f));

	SetupT66MainMenuRuntimeImageBrush(
		RightPanelShellBrush,
		RightPanelShellTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/panel_large_normal.png"),
		FVector2D(T66MainMenuReferenceLayout::Right::ShellFullReference.Width, T66MainMenuReferenceLayout::Right::ShellFullReference.Height));
	ConfigureMainMenuBoxBrush(RightPanelShellBrush, FMargin(0.067f, 0.043f, 0.067f, 0.043f));
}

void UT66MainMenuScreen::RequestMainMenuChromeBrushes()
{
	SetupT66MainMenuRuntimeImageBrush(
		SearchFieldShellBrush,
		SearchFieldShellTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/Fields/search_field_normal.png"),
		FVector2D(T66MainMenuReferenceLayout::Left::SearchFieldReference.Width, T66MainMenuReferenceLayout::Left::SearchFieldReference.Height));
	ConfigureMainMenuBoxBrush(SearchFieldShellBrush, FMargin(0.037f, 0.231f, 0.037f, 0.231f));

	SetupT66MainMenuRuntimeImageBrush(
		SearchIconBrush,
		SearchIconTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/IconsGenerated/icon_12_search_magnifier_imagegen_20260425_v2.png"),
		FVector2D(T66MainMenuReferenceLayout::Left::SearchIcon.Width, T66MainMenuReferenceLayout::Left::SearchIcon.Height));

	SetupT66MainMenuRuntimeImageBrush(
		FriendInviteButtonBrush,
		FriendInviteButtonTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/invite_small_normal.png"),
		FVector2D(T66MainMenuReferenceLayout::Left::FriendInviteButton.Width, T66MainMenuReferenceLayout::Left::FriendInviteButton.Height));
	ConfigureMainMenuBoxBrush(FriendInviteButtonBrush, FMargin(0.164f, 0.269f, 0.164f, 0.269f));

	SetupT66MainMenuRuntimeImageBrush(
		FriendOfflineButtonBrush,
		FriendOfflineButtonTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/invite_small_disabled.png"),
		FVector2D(T66MainMenuReferenceLayout::Left::FriendOfflineButton.Width, T66MainMenuReferenceLayout::Left::FriendOfflineButton.Height));
	ConfigureMainMenuBoxBrush(FriendOfflineButtonBrush, FMargin(0.164f, 0.269f, 0.164f, 0.269f));

	SetupT66MainMenuRuntimeImageBrush(
		FriendAvatarFrameBrush,
		FriendAvatarFrameTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/Slots/avatar_slot_normal.png"),
		FVector2D(T66MainMenuReferenceLayout::Left::FriendAvatarFrameSource.Width, T66MainMenuReferenceLayout::Left::FriendAvatarFrameSource.Height));
	ConfigureMainMenuBoxBrush(FriendAvatarFrameBrush, FMargin(0.205f, 0.205f, 0.205f, 0.205f));

	SetupT66MainMenuRuntimeImageBrush(
		PartySlotFrameBrush,
		PartySlotFrameTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/Slots/party_slot_normal.png"),
		FVector2D(T66MainMenuReferenceLayout::Left::PartySlotSource.Width, T66MainMenuReferenceLayout::Left::PartySlotSource.Height));
	ConfigureMainMenuBoxBrush(PartySlotFrameBrush, FMargin(0.188f, 0.180f, 0.188f, 0.180f));

	SetupT66MainMenuRuntimeImageBrush(
		PartyPlusIconBrush,
		PartyPlusIconTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/IconsGenerated/icon_15_party_add_plus_purple_20260427.png"),
		FVector2D(38.f, 38.f));

	SetupT66MainMenuRuntimeImageBrush(
		CloseButtonBrush,
		CloseButtonTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/IconsGenerated/icon_08_power_quit_red_imagegen_20260427.png"),
		FVector2D(T66MainMenuReferenceLayout::Left::CloseButton.Width, T66MainMenuReferenceLayout::Left::CloseButton.Height));

	SetupT66MainMenuRuntimeImageBrush(
		ProfileAvatarFrameBrush,
		ProfileAvatarFrameTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/Slots/avatar_slot_normal.png"),
		FVector2D(101.f, 93.f));
	ConfigureMainMenuBoxBrush(ProfileAvatarFrameBrush, FMargin(0.205f, 0.205f, 0.205f, 0.205f));

	SetupT66MainMenuRuntimeImageBrush(
		ProfileProgressShellBrush,
		ProfileProgressShellTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/Controls/dropdown_field_normal.png"),
		FVector2D(512.f, 22.f));
	ConfigureMainMenuBoxBrush(ProfileProgressShellBrush, FMargin(0.06f, 0.34f, 0.06f, 0.34f));

	SetupT66MainMenuRuntimeImageBrush(
		ProfileProgressFillBrush,
		ProfileProgressFillTexture,
		nullptr,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/Misc/progress_fill_dark_purple_twinkle_imagegen_20260427.png"),
		FVector2D(512.f, 64.f));
	ConfigureMainMenuBoxBrush(ProfileProgressFillBrush, FMargin(0.25f, 0.45f, 0.25f, 0.45f));
}

void UT66MainMenuScreen::RequestCTAButtonBrushes()
{
	auto LoadBrush = [](
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& TextureHandle,
		const TCHAR* RelativePath,
		const FVector2D& DesiredSize)
	{
		SetupT66MainMenuRuntimeImageBrush(
			Brush,
			TextureHandle,
			nullptr,
			RelativePath,
			DesiredSize);
	};

	LoadBrush(
		CenterStackFrameBrush,
		CenterStackFrameTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/modal_frame_normal.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaStackFull.Width, T66MainMenuReferenceLayout::Center::CtaStackFull.Height));
	ConfigureMainMenuBoxBrush(CenterStackFrameBrush, FMargin(0.052f, 0.094f, 0.052f, 0.094f));

	LoadBrush(
		NewGameButtonBrushes.NormalBrush,
		NewGameButtonBrushes.NormalTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_normal.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaButtonNewGame.Width, T66MainMenuReferenceLayout::Center::CtaButtonNewGame.Height));
	LoadBrush(
		NewGameButtonBrushes.HoverBrush,
		NewGameButtonBrushes.HoverTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_hover.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaButtonNewGame.Width, T66MainMenuReferenceLayout::Center::CtaButtonNewGame.Height));
	LoadBrush(
		NewGameButtonBrushes.PressedBrush,
		NewGameButtonBrushes.PressedTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_pressed.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaButtonNewGame.Width, T66MainMenuReferenceLayout::Center::CtaButtonNewGame.Height));
	LoadBrush(
		NewGameButtonBrushes.DisabledBrush,
		NewGameButtonBrushes.DisabledTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_disabled.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaButtonNewGame.Width, T66MainMenuReferenceLayout::Center::CtaButtonNewGame.Height));

	LoadBrush(
		LoadGameButtonBrushes.NormalBrush,
		LoadGameButtonBrushes.NormalTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_normal.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaButtonLoadGame.Width, T66MainMenuReferenceLayout::Center::CtaButtonLoadGame.Height));
	LoadBrush(
		LoadGameButtonBrushes.HoverBrush,
		LoadGameButtonBrushes.HoverTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_hover.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaButtonLoadGame.Width, T66MainMenuReferenceLayout::Center::CtaButtonLoadGame.Height));
	LoadBrush(
		LoadGameButtonBrushes.PressedBrush,
		LoadGameButtonBrushes.PressedTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_pressed.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaButtonLoadGame.Width, T66MainMenuReferenceLayout::Center::CtaButtonLoadGame.Height));
	LoadBrush(
		LoadGameButtonBrushes.DisabledBrush,
		LoadGameButtonBrushes.DisabledTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_disabled.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaButtonLoadGame.Width, T66MainMenuReferenceLayout::Center::CtaButtonLoadGame.Height));

	LoadBrush(
		DailyChallengeButtonBrushes.NormalBrush,
		DailyChallengeButtonBrushes.NormalTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_normal.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaButtonDailyChallenge.Width, T66MainMenuReferenceLayout::Center::CtaButtonDailyChallenge.Height));
	LoadBrush(
		DailyChallengeButtonBrushes.HoverBrush,
		DailyChallengeButtonBrushes.HoverTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_hover.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaButtonDailyChallenge.Width, T66MainMenuReferenceLayout::Center::CtaButtonDailyChallenge.Height));
	LoadBrush(
		DailyChallengeButtonBrushes.PressedBrush,
		DailyChallengeButtonBrushes.PressedTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_pressed.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaButtonDailyChallenge.Width, T66MainMenuReferenceLayout::Center::CtaButtonDailyChallenge.Height));
	LoadBrush(
		DailyChallengeButtonBrushes.DisabledBrush,
		DailyChallengeButtonBrushes.DisabledTexture,
		TEXT("SourceAssets/UI/MasterLibrary/Slices/TopBar/topbar_nav_disabled.png"),
		FVector2D(T66MainMenuReferenceLayout::Center::CtaButtonDailyChallenge.Width, T66MainMenuReferenceLayout::Center::CtaButtonDailyChallenge.Height));

	const FMargin CtaMargin(0.104f, 0.250f, 0.104f, 0.250f);
	ConfigureMainMenuBoxBrush(NewGameButtonBrushes.NormalBrush, CtaMargin);
	ConfigureMainMenuBoxBrush(NewGameButtonBrushes.HoverBrush, CtaMargin);
	ConfigureMainMenuBoxBrush(NewGameButtonBrushes.PressedBrush, CtaMargin);
	ConfigureMainMenuBoxBrush(NewGameButtonBrushes.DisabledBrush, CtaMargin);
	ConfigureMainMenuBoxBrush(LoadGameButtonBrushes.NormalBrush, CtaMargin);
	ConfigureMainMenuBoxBrush(LoadGameButtonBrushes.HoverBrush, CtaMargin);
	ConfigureMainMenuBoxBrush(LoadGameButtonBrushes.PressedBrush, CtaMargin);
	ConfigureMainMenuBoxBrush(LoadGameButtonBrushes.DisabledBrush, CtaMargin);
	ConfigureMainMenuBoxBrush(DailyChallengeButtonBrushes.NormalBrush, CtaMargin);
	ConfigureMainMenuBoxBrush(DailyChallengeButtonBrushes.HoverBrush, CtaMargin);
	ConfigureMainMenuBoxBrush(DailyChallengeButtonBrushes.PressedBrush, CtaMargin);
	ConfigureMainMenuBoxBrush(DailyChallengeButtonBrushes.DisabledBrush, CtaMargin);
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

FReply UT66MainMenuScreen::HandleDailyClimbClicked()
{
	OnDailyClimbClicked();
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

void UT66MainMenuScreen::OnDailyClimbClicked()
{
	NavigateTo(ET66ScreenType::DailyClimb);
}

void UT66MainMenuScreen::OnPowerUpClicked()
{
	NavigateTo(ET66ScreenType::PowerUp);
}

void UT66MainMenuScreen::OnMinigamesClicked()
{
	NavigateTo(ET66ScreenType::Minigames);
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
