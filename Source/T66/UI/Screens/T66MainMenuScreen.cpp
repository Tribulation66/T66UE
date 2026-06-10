// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MainMenuScreen.h"
#include "UI/T66UIManager.h"
#include "UI/T66FrontendVideoCatalog.h"
#include "UI/T66FrontendVideoPlayer.h"
#include "UI/Components/T66FlatLeaderboardPanel.h"
#include "UI/Style/T66ReferenceLayout.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66ReleaseVariantSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66ShelvedFeatureGate.h"
#include "Core/T66SteamHelper.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66FriendslopStyle.h"
#include "UI/T66DemoModeUIUtils.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"
#include "Engine/UserInterfaceSettings.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SLeafWidget.h"
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

	void SetupT66MainMenuRuntimeImageBrush(
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& TextureHandle,
		const TCHAR* AssetPath,
		const TCHAR* StagedRelativePath,
		const FVector2D& ImageSize,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
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
					Filter,
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
						Filter,
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

	FVector2D GetMainMenuViewportSizeOrFallback()
	{
		if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
		{
			const FIntPoint Size = GEngine->GameViewport->Viewport->GetSizeXY();
			if (Size.X > 0 && Size.Y > 0)
			{
				return FVector2D(static_cast<float>(Size.X), static_cast<float>(Size.Y));
			}
		}

		return FVector2D(T66MainMenuReferenceLayout::CanvasWidth, T66MainMenuReferenceLayout::CanvasHeight);
	}

	float GetMainMenuEngineDPIScale(const FVector2D& ViewportSize)
	{
		if (const UUserInterfaceSettings* UISettings = GetDefault<UUserInterfaceSettings>())
		{
			const FIntPoint PixelSize(
				FMath::Max(1, FMath::RoundToInt(ViewportSize.X)),
				FMath::Max(1, FMath::RoundToInt(ViewportSize.Y)));
			return FMath::Max(0.01f, UISettings->GetDPIScaleBasedOnSize(PixelSize));
		}

		return 1.f;
	}

	FVector2D GetEffectiveFrontendViewportSize()
	{
		int32 AutomationViewportWidth = 0;
		int32 AutomationViewportHeight = 0;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66AutomationResX="), AutomationViewportWidth)
			&& FParse::Value(FCommandLine::Get(), TEXT("T66AutomationResY="), AutomationViewportHeight)
			&& AutomationViewportWidth > 0
			&& AutomationViewportHeight > 0)
		{
			return FVector2D(static_cast<float>(AutomationViewportWidth), static_cast<float>(AutomationViewportHeight));
		}

		return GetMainMenuViewportSizeOrFallback();
	}

	bool DoesFriendMatchSearchQuery(const FString& SearchQuery, const FString& FriendName)
	{
		const FString NormalizedQuery = SearchQuery.TrimStartAndEnd();
		return NormalizedQuery.IsEmpty() || FriendName.Contains(NormalizedQuery, ESearchCase::IgnoreCase);
	}

	int32 FitMainMenuLabelFontSize(
		const FText& Label,
		const int32 PreferredSize,
		const int32 MinSize,
		const float AvailableWidth,
		TFunctionRef<FSlateFontInfo(int32)> MakeFont)
	{
		const FString LabelString = Label.ToString();
		if (LabelString.IsEmpty())
		{
			return PreferredSize;
		}

		const float ClampedAvailableWidth = FMath::Max(1.f, AvailableWidth);
		for (int32 FontSize = PreferredSize; FontSize >= MinSize; --FontSize)
		{
			const FSlateFontInfo Font = MakeFont(FontSize);
			if (FSlateApplication::IsInitialized())
			{
				if (FSlateRenderer* Renderer = FSlateApplication::Get().GetRenderer())
				{
					const FVector2D MeasuredSize = Renderer->GetFontMeasureService()->Measure(LabelString, Font);
					if (MeasuredSize.X <= ClampedAvailableWidth)
					{
						return FontSize;
					}
					continue;
				}
			}

			const float EstimatedWidth = static_cast<float>(LabelString.Len()) * static_cast<float>(FontSize) * 0.78f;
			if (EstimatedWidth <= ClampedAvailableWidth)
			{
				return FontSize;
			}
		}

		return MinSize;
	}

	TArray<FVector2D> MakeMainMenuCirclePoints(const FVector2D& Center, const float Radius, const int32 Segments)
	{
		TArray<FVector2D> Points;
		Points.Reserve(Segments + 1);
		for (int32 Index = 0; Index <= Segments; ++Index)
		{
			const float Alpha = static_cast<float>(Index) / static_cast<float>(Segments);
			const float Angle = Alpha * 2.f * PI;
			Points.Add(Center + FVector2D(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius));
		}
		return Points;
	}

	class ST66MainMenuSearchGlyph : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66MainMenuSearchGlyph) {}
			SLATE_ARGUMENT(FVector2D, DesiredSize)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			DesiredSize = InArgs._DesiredSize.IsNearlyZero() ? FVector2D(20.f, 20.f) : InArgs._DesiredSize;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return DesiredSize;
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y));
			const FVector2D Center(Size.X * 0.43f, Size.Y * 0.42f);
			const float Radius = MinDim * 0.25f;
			const float Thickness = FMath::Max(2.f, MinDim * 0.12f);
			const FLinearColor Color(0.58f, 0.63f, 0.72f, 0.95f);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				MakeMainMenuCirclePoints(Center, Radius, 28),
				ESlateDrawEffect::None,
				Color,
				true,
				Thickness);

			const TArray<FVector2D> HandleLine = {
				Center + FVector2D(Radius * 0.66f, Radius * 0.66f),
				Center + FVector2D(MinDim * 0.42f, MinDim * 0.42f)
			};
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				HandleLine,
				ESlateDrawEffect::None,
				Color,
				true,
				Thickness);

			return LayerId + 2;
		}

	private:
		FVector2D DesiredSize = FVector2D(20.f, 20.f);
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
	MarkSlateUIBuilt();
	return BuildSlateUI();
}

TSharedRef<SWidget> UT66MainMenuScreen::BuildFlatMainMenuUI()
{
	RequestBackgroundTexture();

	CachedViewportSize = GetEffectiveFrontendViewportSize();
	LastBuiltViewportSize = CachedViewportSize;
	PendingViewportSize = CachedViewportSize;
	PendingViewportStableTime = 0.f;
	bViewportResponsiveRebuildQueued = false;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PlayerSettings = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	UT66SteamHelper* SteamHelper = GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr;
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	LastBuiltLanguage = Loc ? Loc->GetCurrentLanguage() : ET66Language::English;
	LastBuiltMenuStateHash = CaptureMenuStateHash();
	FriendGroupWidgetRefs.Reset();
	FriendRowWidgetRefs.Reset();
	LocalProfileAvatarBrush.Reset();
	CtaSkullIconBrush.Reset();
	CtaSkullIconTexture.Reset();
	FriendFavoriteStarBrush.Reset();
	FriendFavoriteStarTexture.Reset();
	DailyDescentIconBrush.Reset();
	DailyDescentIconTexture.Reset();
	FriendAvatarBrushes.Reset();
	FriendGroupsDividerBox.Reset();
	NoMatchingFriendsBox.Reset();

	FString AutomationDumpPath;
	const bool bAutomationDump = FParse::Value(FCommandLine::Get(), TEXT("T66AutoDumpScreen="), AutomationDumpPath);
	const bool bFriendslopReferenceFixture = FParse::Param(FCommandLine::Get(), TEXT("T66FriendslopReferenceFixture"));

	TArray<FT66PartyFriendEntry> ReferenceFixtureFriends;
	if (bFriendslopReferenceFixture)
	{
		auto AddFixtureFriend = [&ReferenceFixtureFriends](const TCHAR* PlayerId, const TCHAR* DisplayName, const bool bOnline)
		{
			FT66PartyFriendEntry Friend;
			Friend.PlayerId = PlayerId;
			Friend.DisplayName = DisplayName;
			Friend.PresenceText = bOnline ? TEXT("Online") : TEXT("Offline");
			Friend.bOnline = bOnline;
			ReferenceFixtureFriends.Add(MoveTemp(Friend));
		};
		AddFixtureFriend(TEXT("friendslop_fixture_xaropinho"), TEXT("Xaropinho"), true);
		AddFixtureFriend(TEXT("friendslop_fixture_cclubp"), TEXT("\u2663 C \u2663 \u2663 P \u2663"), false);
		AddFixtureFriend(TEXT("friendslop_fixture_cant_hear"), TEXT("I can't hear you"), false);
		AddFixtureFriend(TEXT("friendslop_fixture_tribulation"), TEXT("Tribulation 66 Studios"), false);
		AddFixtureFriend(TEXT("friendslop_fixture_magina"), TEXT("Magina da silva Slark"), false);
	}
	const TArray<FT66PartyFriendEntry>* FriendsForDisplay = bFriendslopReferenceFixture
		? &ReferenceFixtureFriends
		: (PartySubsystem ? &PartySubsystem->GetFriends() : nullptr);

	const FString LocalSteamName = SteamHelper ? SteamHelper->GetLocalDisplayName() : FString();
	const FText ProfileNameText = bFriendslopReferenceFixture
		? FText::FromString(TEXT("Solobro"))
		: (!LocalSteamName.IsEmpty()
		? FText::FromString(LocalSteamName)
		: NSLOCTEXT("T66.MainMenu", "ProfileNameFallback", "Local Player"));
	const int32 ProfileLevel = bAutomationDump || bFriendslopReferenceFixture ? 1 : (Achievements ? Achievements->GetAccountLevel() : 1);
	const int32 ProfileMaxLevel = bFriendslopReferenceFixture ? 100 : (Achievements ? Achievements->GetAccountMaxLevel() : UT66AchievementsSubsystem::AccountMaxLevel);
	const int32 ProfileNextLevel = bAutomationDump || bFriendslopReferenceFixture ? 2 : (Achievements ? Achievements->GetAccountNextLevel() : 2);
	const float ProfileLevelProgress = bAutomationDump || bFriendslopReferenceFixture ? 0.58f : (Achievements ? Achievements->GetAccountLevelProgress01() : 0.58f);
	const FText ProfileLevelText = FText::Format(
		NSLOCTEXT("T66.MainMenu", "ProfileLevelFormat", "Level {0}/{1}"),
		FText::AsNumber(ProfileLevel),
		FText::AsNumber(ProfileMaxLevel));
	const FText ProfileNextLevelText = FText::Format(
		NSLOCTEXT("T66.MainMenu", "ProfileNextLevelFormat", "Level {0}"),
		FText::AsNumber(ProfileNextLevel));

	const int32 OnlineFriendCount = (bAutomationDump && !bFriendslopReferenceFixture) || !FriendsForDisplay ? 0 : Algo::CountIf(*FriendsForDisplay, [](const FT66PartyFriendEntry& Friend)
	{
		return Friend.bOnline;
	});
	const int32 OfflineFriendCount = (bAutomationDump && !bFriendslopReferenceFixture) || !FriendsForDisplay ? 0 : Algo::CountIf(*FriendsForDisplay, [](const FT66PartyFriendEntry& Friend)
	{
		return !Friend.bOnline;
	});

	const FButtonStyle& NoBorderButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
	const FName LeaderboardFilterGroup(TEXT("MainMenuLeaderboardFilter"));
	const FName LeaderboardScopeGroup(TEXT("MainMenuLeaderboardScope"));

	SetupT66MainMenuRuntimeImageBrush(
		CtaSkullIconBrush,
		CtaSkullIconTexture,
		nullptr,
		TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/cta_skull_icon_round06.png"),
		FVector2D(54.f, 54.f),
		TextureFilter::TF_Trilinear);
	SetupT66MainMenuRuntimeImageBrush(
		FriendFavoriteStarBrush,
		FriendFavoriteStarTexture,
		nullptr,
		TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/friend_favorite_star_round06.png"),
		FVector2D(30.f, 30.f),
		TextureFilter::TF_Trilinear);
	SetupT66MainMenuRuntimeImageBrush(
		DailyDescentIconBrush,
		DailyDescentIconTexture,
		nullptr,
		TEXT("RuntimeDependencies/T66/UI/MainMenu/mainmenu_daily_descent_one_run_badge_imagegen_20260510.png"),
		FVector2D(42.f, 42.f),
		TextureFilter::TF_Nearest);
	if (bFriendslopReferenceFixture)
	{
		SetupT66MainMenuRuntimeImageBrush(
			LocalProfileAvatarBrush,
			FixtureAvatarTexture,
			nullptr,
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/fixture_avatar_obey_round06.png"),
			FVector2D(76.f, 76.f),
			TextureFilter::TF_Nearest);
	}

	auto Tag = [](const TCHAR* Name) -> FName
	{
		return FName(Name);
	};

	auto MakeSized = [](const float Width, const float Height, const TSharedRef<SWidget>& Child) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(Width)
			.HeightOverride(Height)
			[
				Child
			];
	};

	auto MakeSpacerPanel = [](const FName InTag, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatPanel(
			State,
			FMargin(0.f),
			SNew(SSpacer),
			nullptr,
			InTag);
	};

	auto MakeLabelBox = [MakeSized](const FText& Text, const ET66FlatLabelRole Role, const FName InTag, const float Width, const float Height, const ETextJustify::Type Justification = ETextJustify::Left) -> TSharedRef<SWidget>
	{
		return MakeSized(
			Width,
			Height,
			SNew(SBox)
			.HAlign(Justification == ETextJustify::Right ? HAlign_Right : (Justification == ETextJustify::Center ? HAlign_Center : HAlign_Left))
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::MakeFlatLabel(Text, Role, Justification, InTag)
			]);
	};

	auto SetTextureBrush = [](TSharedPtr<FSlateBrush>& Brush, UTexture2D* Texture, const FVector2D& FallbackSize) -> const FSlateBrush*
	{
		if (!Texture)
		{
			Brush.Reset();
			return nullptr;
		}

		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->SetResourceObject(Texture);
		Brush->ImageSize = FVector2D(
			FMath::Max(1.f, static_cast<float>(Texture->GetSizeX())),
			FMath::Max(1.f, static_cast<float>(Texture->GetSizeY())));
		if (Brush->ImageSize.IsNearlyZero())
		{
			Brush->ImageSize = FallbackSize;
		}
		return Brush.Get();
	};

	auto MakeAssignedLabel = [&](TSharedPtr<STextBlock>& OutText, const FText& Text, const ET66FlatLabelRole Role, const ETextJustify::Type Justification, const FName InTag) -> TSharedRef<SWidget>
	{
		SAssignNew(OutText, STextBlock)
			.Text(Text)
			.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(
				Role == ET66FlatLabelRole::Button ? 15 : (Role == ET66FlatLabelRole::Caption ? 12 : 16),
				Role == ET66FlatLabelRole::Header || Role == ET66FlatLabelRole::Button || Role == ET66FlatLabelRole::StatValue))
			.ColorAndOpacity(FT66FlatStyle::TextColorForState(ET66FlatState::Default))
			.Justification(Justification)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis);

		return FT66FlatStyle::AttachMetadata(
			OutText.ToSharedRef(),
			InTag,
			TEXT("Label.Body"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	};

	// UI Reimagine 2026-06-10: hellfire identity assets (approved reference v7).
	const FString HellfireDir = TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/MainMenu/");
	auto HellfireBrush = [HellfireDir](const TCHAR* File, const FMargin& Margin, const ESlateBrushDrawType::Type DrawAs, const FVector2D& FallbackSize) -> const FSlateBrush*
	{
		return FT66FriendslopStyle::GetCustomBrush(HellfireDir + File, Margin, DrawAs, FallbackSize);
	};

	auto MakeTitleRegion = [&]() -> TSharedRef<SWidget>
	{
		auto MakeSubtitleTextLayer = [](const FText& Text, const int32 FontSize, const FLinearColor& Color, const FVector2D& Offset) -> TSharedRef<STextBlock>
		{
			return SNew(STextBlock)
				.Text(Text)
				.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(FontSize, true))
				.ColorAndOpacity(Color)
				.Justification(ETextJustify::Center)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.RenderTransform(FSlateRenderTransform(Offset));
		};

		auto MakeLayeredSubtitleText = [&](const FText& Text, const FName InTag) -> TSharedRef<SWidget>
		{
			const int32 SubtitleFontSize = FitMainMenuLabelFontSize(
				Text,
				36,
				28,
				620.f,
				[](const int32 FontSize)
				{
					return T66RuntimeUIFontAccess::MakeFriendslopFont(FontSize, true);
				});

			TSharedRef<SOverlay> SubtitleOverlay = SNew(SOverlay);
			SubtitleOverlay->AddSlot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					MakeSubtitleTextLayer(Text, SubtitleFontSize, FLinearColor(0.50f, 0.02f, 0.03f, 1.f), FVector2D(2.f, -23.f))
				];
			SubtitleOverlay->AddSlot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					FT66FlatStyle::AttachMetadata(
						MakeSubtitleTextLayer(Text, SubtitleFontSize, FLinearColor(1.0f, 0.95f, 0.90f, 1.f), FVector2D(0.f, -27.f)),
						InTag,
						TEXT("Label.Header"),
						ET66FlatState::Default,
						TOptional<FLinearColor>(),
						false,
						NAME_None,
						true)
				];
			return SubtitleOverlay;
		};

		TSharedRef<SConstraintCanvas> TitleCanvas = SNew(SConstraintCanvas);
		TitleCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(0.f, 4.f, 700.f, 152.f))
			[
				FT66FriendslopStyle::MakeCustomFixedImage(
					HellfireDir + TEXT("title_chadpocalypse.png"),
					FMargin(0.f),
					ESlateBrushDrawType::Image,
					FVector2D(700.f, 152.f),
					Tag(TEXT("MainMenu.Center.Title")),
					TEXT("TitleLogo"))
			];
		TitleCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(70.f, 168.f, 560.f, 70.f))
			[
				FT66FriendslopStyle::MakeCustomSurface(
					HellfireDir + TEXT("subtitle_banner_plate.png"),
					FMargin(0.18f, 0.30f),
					ESlateBrushDrawType::Box,
					FVector2D(900.f, 140.f),
					ET66FlatState::Default,
					FMargin(20.f, 6.f),
					SNew(SBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						MakeLayeredSubtitleText(
							NSLOCTEXT("T66.MainMenu", "BloodyRetroSubtitle", "If you're not Chad it's over"),
							Tag(TEXT("MainMenu.Center.Subtitle")))
					],
					nullptr,
					Tag(TEXT("MainMenu.Center.SubtitleBanner")),
					TEXT("SubtitleBanner"))
			];

		return FT66FlatStyle::AttachMetadata(
			MakeSized(700.f, 238.f, TitleCanvas),
			Tag(TEXT("MainMenu.Center.TitleRegion")),
			TEXT("TitleRegion"),
			ET66FlatState::Default);
	};

	auto MakeCtaIcon = [&](const FSlateBrush* IconBrush, const FVector2D& Size, const FName InTag) -> TSharedRef<SWidget>
	{
		const TSharedRef<SWidget> IconContent = IconBrush
			? StaticCastSharedRef<SWidget>(SNew(SImage).Image(IconBrush).ColorAndOpacity(FLinearColor::White))
			: StaticCastSharedRef<SWidget>(SNew(SSpacer));

		return FT66FlatStyle::AttachMetadata(
			SNew(SBox)
			.WidthOverride(Size.X)
			.HeightOverride(Size.Y)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				IconContent
			],
			InTag,
			TEXT("Icon"),
			ET66FlatState::Default);
	};

	auto MakeCtaContent = [&](const FText& Text, const ET66FlatState State, const float Width, const float Height, const FName InTag, const FSlateBrush* LeftIcon, const FSlateBrush* RightIcon) -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
		const float IconSize = Height >= 100.f ? 54.f : 42.f;
		const float IconWidthBudget = (LeftIcon ? IconSize + 16.f : 0.f) + (RightIcon ? IconSize + 16.f : 0.f);
		const float LabelWidth = FMath::Max(160.f, Width - IconWidthBudget - 128.f);
		const int32 LabelFontSize = FitMainMenuLabelFontSize(
			Text,
			Height >= 100.f ? 36 : 30,
			22,
			LabelWidth,
			[](const int32 FontSize)
			{
				return T66RuntimeUIFontAccess::MakeFriendslopFont(FontSize, true);
			});
		if (LeftIcon)
		{
			Row->AddSlot()
				.AutoWidth()
				.Padding(0.f, 0.f, 16.f, 0.f)
				.VAlign(VAlign_Center)
				[
					MakeCtaIcon(LeftIcon, FVector2D(IconSize, IconSize), FName(*(InTag.ToString() + TEXT(".LeftIcon"))))
				];
		}

		Row->AddSlot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(LabelWidth)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					FT66FlatStyle::AttachMetadata(
						SNew(STextBlock)
						.Text(Text)
						.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(LabelFontSize, true))
						.ColorAndOpacity(FT66FriendslopStyle::TextColorForState(State))
						.Justification(ETextJustify::Center)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
						FName(*(InTag.ToString() + TEXT(".Label"))),
						TEXT("Label.Button"),
						State,
						TOptional<FLinearColor>(),
						false,
						NAME_None,
						true)
				]
			];

		if (RightIcon)
		{
			Row->AddSlot()
				.AutoWidth()
				.Padding(16.f, 0.f, 0.f, 0.f)
				.VAlign(VAlign_Center)
				[
					MakeCtaIcon(RightIcon, FVector2D(IconSize, IconSize), FName(*(InTag.ToString() + TEXT(".RightIcon"))))
				];
		}

		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(FMath::Max(1.f, Width - 36.f))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					Row
				]
			];
	};

	auto IsDailyDescentAvailable = [this]() -> bool
	{
		if (!FT66ShelvedFeatureGate::IsDailyDescentEnabled())
		{
			return false;
		}

		const UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
		const UT66ReleaseVariantSubsystem* ReleaseVariant = GI ? GI->GetSubsystem<UT66ReleaseVariantSubsystem>() : nullptr;
		return !ReleaseVariant || !ReleaseVariant->IsDemoModeActive();
	};

	auto MakeCtaButton = [&](const FText& Text, FReply (UT66MainMenuScreen::*ClickFunc)(), const ET66FlatState State, const float Width, const float Height, const FName InTag, const FSlateBrush* LeftIcon = nullptr, const FSlateBrush* RightIcon = nullptr, const bool bEnabled = true) -> TSharedRef<SWidget>
	{
		const ET66FlatState RenderState = bEnabled ? State : ET66FlatState::Disabled;
		const TCHAR* CtaPlateFile = State == ET66FlatState::Selected
			? TEXT("cta_hero.png")
			: TEXT("cta_secondary.png");
		return FT66FriendslopStyle::MakeCustomToggleGroupButton(
			HellfireDir + CtaPlateFile,
			FMargin(0.18f, 0.30f),
			FVector2D(780.f, 150.f),
			RenderState,
			MakeCtaContent(Text, RenderState, Width, Height, InTag, LeftIcon, RightIcon),
			bEnabled ? FOnClicked::CreateUObject(this, ClickFunc) : FOnClicked(),
			FMargin(18.f, 8.f),
			Width,
			Height,
			bEnabled,
			InTag,
			NAME_None);
	};

	constexpr float LeftContentWidth = 460.f;
	constexpr float LeftPanelContentInset = 30.f;
	constexpr float LeftPanelWidth = LeftContentWidth + (LeftPanelContentInset * 2.f);
	constexpr float PartySlotSize = 80.f;
	constexpr float PartySlotGap = 12.f;
	constexpr float PartySlotGroupWidth = PartySlotSize * 4.f + PartySlotGap * 3.f;
	constexpr float PartySidePad = (LeftContentWidth - PartySlotGroupWidth) * 0.5f;
	constexpr float ProfileRowHeight = 112.f;
	constexpr float ProfileAvatarSize = 72.f;
	constexpr float FriendRowHeight = 66.f;
	constexpr float FriendAvatarSize = 42.f;
	constexpr float FriendActionButtonWidth = 96.f;
	constexpr float FriendOnlineActionHeight = 44.f;
	constexpr float FriendOfflineActionHeight = 42.f;

	auto MakeProfileButton = [&]() -> TSharedRef<SWidget>
	{
		const FSlateBrush* ProfileAvatarBrush = bFriendslopReferenceFixture && LocalProfileAvatarBrush.IsValid()
			? LocalProfileAvatarBrush.Get()
			: SetTextureBrush(
				LocalProfileAvatarBrush,
				SteamHelper ? SteamHelper->GetLocalAvatarTexture() : nullptr,
				FVector2D(ProfileAvatarSize, ProfileAvatarSize));

		TSharedRef<SVerticalBox> ProfileInfo = SNew(SVerticalBox);
		ProfileInfo->AddSlot()
			.AutoHeight()
			[
				MakeLabelBox(ProfileNameText, ET66FlatLabelRole::Header, Tag(TEXT("MainMenu.Left.ProfileName")), 360.f, 34.f)
			];
		ProfileInfo->AddSlot()
			.AutoHeight()
			.Padding(0.f, 4.f, 0.f, 0.f)
			[
				MakeLabelBox(ProfileLevelText, ET66FlatLabelRole::Body, Tag(TEXT("MainMenu.Left.ProfileLevel")), 360.f, 24.f)
			];
		ProfileInfo->AddSlot()
			.AutoHeight()
			.Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					FT66FlatStyle::MakeFlatProgressBar(
						TAttribute<float>(FMath::Clamp(ProfileLevelProgress, 0.f, 1.f)),
						TOptional<FLinearColor>(),
						Tag(TEXT("MainMenu.Left.ProfileProgress")))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(10.f, 0.f, 0.f, 0.f)
				.VAlign(VAlign_Center)
				[
					MakeLabelBox(ProfileNextLevelText, ET66FlatLabelRole::Caption, Tag(TEXT("MainMenu.Left.ProfileNextLevel")), 62.f, 22.f, ETextJustify::Right)
				]
			];

		TSharedRef<SWidget> Content = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::MakeFlatPortraitSlot(
					ET66FlatState::Selected,
					ProfileAvatarBrush,
					nullptr,
					FVector2D(ProfileAvatarSize, ProfileAvatarSize),
					Tag(TEXT("MainMenu.Left.ProfileAvatar")))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(16.f, 0.f, 0.f, 0.f)
			.VAlign(VAlign_Center)
			[
				ProfileInfo
			];

		TSharedRef<SButton> ProfileButton =
			SNew(SButton)
			.ButtonStyle(&NoBorderButtonStyle)
			.ContentPadding(FMargin(0.f))
			.OnClicked_Lambda([this]()
			{
				OnAccountStatusClicked();
				return FReply::Handled();
			})
			[
				SNew(SBox)
				.WidthOverride(LeftContentWidth)
				.HeightOverride(ProfileRowHeight)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
					.Padding(FMargin(16.f, 10.f))
					[
						Content
					]
				]
			];

		return FT66FlatStyle::AttachMetadata(
			ProfileButton,
			Tag(TEXT("MainMenu.Left.ProfileButton")),
			TEXT("ProfileButton"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			true,
			NAME_None,
			false,
			true);
	};

	auto MakeSearchField = [&]() -> TSharedRef<SWidget>
	{
		TSharedRef<SWidget> SearchContent = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::AttachMetadata(
					StaticCastSharedRef<SWidget>(
						SNew(SBox)
						.WidthOverride(20.f)
						.HeightOverride(20.f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(ST66MainMenuSearchGlyph)
							.DesiredSize(FVector2D(20.f, 20.f))
						]),
					Tag(TEXT("MainMenu.Left.SearchIcon")),
					TEXT("Icon.Search"),
					ET66FlatState::Default)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(14.f, 0.f, 0.f, 0.f)
			.VAlign(VAlign_Center)
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(FriendSearchQuery))
				.OnTextChanged_UObject(this, &UT66MainMenuScreen::HandleFriendSearchTextChanged)
				.HintText(NSLOCTEXT("T66.MainMenu", "FriendSearchHint", "Search friends..."))
				.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(18, false))
				.ForegroundColor(FT66FlatStyle::PrimaryText())
				.BackgroundColor(FLinearColor::Transparent)
			];

		return FT66FriendslopStyle::MakeCustomSurface(
			HellfireDir + TEXT("field_search.png"),
			FMargin(0.14f, 0.30f),
			ESlateBrushDrawType::Box,
			FVector2D(540.f, 78.f),
			ET66FlatState::Default,
			FMargin(18.f, 10.f),
			SearchContent,
			nullptr,
			Tag(TEXT("MainMenu.Left.SearchField")),
			TEXT("SearchField"),
			true,
			NAME_None,
			true);
	};

	auto MakeFriendGroupToggle = [&](const bool bOnlineGroup, const FText& LabelText, const int32 Count, const FName ToggleTag, const FName LabelTag, const FName CountTag) -> TSharedRef<SWidget>
	{
		FFriendGroupWidgetRefs GroupRefs;
		GroupRefs.bOnlineGroup = bOnlineGroup;
		static const FSlateRoundedBoxBrush OnlineDotBrush(FLinearColor(0.20f, 0.88f, 0.25f, 1.f), 5.f);
		static const FSlateRoundedBoxBrush OfflineDotBrush(FLinearColor(0.54f, 0.57f, 0.62f, 1.f), 5.f);

		TSharedRef<SWidget> Content = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::AttachMetadata(
					SAssignNew(GroupRefs.ExpandArrowText, STextBlock)
					.Text(FText::FromString(bOnlineGroup ? TEXT("v") : TEXT(">")))
					.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(16, true))
					.ColorAndOpacity(FT66FlatStyle::TextColorForState(ET66FlatState::Default))
					.Justification(ETextJustify::Center),
					Tag(*(ToggleTag.ToString() + TEXT(".Arrow"))),
					TEXT("Label.PurpleAccent"),
					ET66FlatState::Default,
					TOptional<FLinearColor>(),
					false,
					NAME_None,
					true)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(9.f, 0.f, 0.f, 0.f)
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::AttachMetadata(
					SNew(SBox)
					.WidthOverride(10.f)
					.HeightOverride(10.f)
					[
						SNew(SBorder)
						.BorderImage(bOnlineGroup ? &OnlineDotBrush : &OfflineDotBrush)
						.Padding(FMargin(0.f))
					],
					Tag(*(ToggleTag.ToString() + TEXT(".StatusDot"))),
					TEXT("StatusDot"),
					bOnlineGroup ? ET66FlatState::Ready : ET66FlatState::Disabled)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(7.f, 0.f, 0.f, 0.f)
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::MakeFlatLabel(LabelText, ET66FlatLabelRole::SubHeader, ETextJustify::Left, LabelTag)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.f, 0.f, 0.f, 0.f)
			.VAlign(VAlign_Center)
			[
				MakeAssignedLabel(
					GroupRefs.CountText,
					FText::Format(NSLOCTEXT("T66.MainMenu", "FriendsGroupCount", "({0})"), FText::AsNumber(Count)),
					ET66FlatLabelRole::SubHeader,
					ETextJustify::Left,
					CountTag)
			];

		TSharedRef<SWidget> ToggleButton = FT66FriendslopStyle::MakeToggleGroupButton(
			ET66FlatState::Default,
			Content,
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
				RequestDeferredSlateRebuild();
				return FReply::Handled();
			}),
			FMargin(12.f, 6.f),
			LeftContentWidth,
			40.f,
			true,
			ToggleTag,
			NAME_None,
			ET66FriendslopChrome::SectionHeaderRound06);

		TSharedRef<SBox> RootBox = SAssignNew(GroupRefs.RootBox, SBox)
			[
				ToggleButton
			];
		FriendGroupWidgetRefs.Add(GroupRefs);
		return RootBox;
	};

	auto MakeFriendRow = [&](const FT66PartyFriendEntry& Friend, const int32 DisplayIndex) -> TSharedRef<SWidget>
	{
		FFriendRowWidgetRefs RowRefs;
		RowRefs.PlayerId = Friend.PlayerId;
		RowRefs.FriendName = Friend.DisplayName.IsEmpty() ? Friend.PlayerId : Friend.DisplayName;
		RowRefs.bOnline = Friend.bOnline;

		const FString RowTagPrefix = FString::Printf(
			TEXT("MainMenu.Left.%sFriendRow%02d"),
			Friend.bOnline ? TEXT("Online") : TEXT("Offline"),
			DisplayIndex + 1);

		TSharedPtr<FSlateBrush>& CachedFriendAvatarBrush = FriendAvatarBrushes.FindOrAdd(Friend.PlayerId);
		SetTextureBrush(
			CachedFriendAvatarBrush,
			SteamHelper ? SteamHelper->GetAvatarTextureForSteamId(Friend.PlayerId) : nullptr,
			FVector2D(FriendAvatarSize, FriendAvatarSize));
		RowRefs.AvatarBrush = CachedFriendAvatarBrush;

		const FString FriendId = Friend.PlayerId;
		const FString FriendName = RowRefs.FriendName;
		TSharedRef<SWidget> RowContent = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 10.f, 0.f)
			[
				FT66FlatStyle::AttachMetadata(
					SNew(SBox)
					.WidthOverride(16.f)
					.HeightOverride(16.f)
					[
						SNew(SImage)
						.Image(HellfireBrush(Friend.bOnline ? TEXT("ball_online.png") : TEXT("ball_offline.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(16.f, 16.f)))
						.ColorAndOpacity(FLinearColor::White)
					],
					Tag(*(RowTagPrefix + TEXT(".StatusBall"))),
					TEXT("Icon.Status"),
					Friend.bOnline ? ET66FlatState::Ready : ET66FlatState::Disabled)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::MakeFlatPortraitSlot(
					Friend.bOnline ? ET66FlatState::Default : ET66FlatState::Disabled,
					RowRefs.AvatarBrush.Get(),
					nullptr,
					FVector2D(FriendAvatarSize, FriendAvatarSize),
					Tag(*(RowTagPrefix + TEXT(".Avatar"))))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(12.f, 0.f, 8.f, 0.f)
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::MakeFlatLabel(
					FText::FromString(FriendName),
					ET66FlatLabelRole::Body,
					ETextJustify::Left,
					Tag(*(RowTagPrefix + TEXT(".Name"))))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.f, 0.f, 6.f, 0.f)
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::AttachMetadata(
					SAssignNew(RowRefs.FavoriteButton, SButton)
					.ButtonStyle(&NoBorderButtonStyle)
					.ContentPadding(FMargin(0.f))
					.OnClicked_Lambda([this, FriendId]()
					{
						UGameInstance* ClickGI = UGameplayStatics::GetGameInstance(this);
						if (UT66PlayerSettingsSubsystem* ClickSettings = ClickGI ? ClickGI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr)
						{
							ClickSettings->SetFavoriteFriend(FriendId, !ClickSettings->IsFavoriteFriend(FriendId));
							RefreshFriendListVisualState();
						}
						return FReply::Handled();
					})
					[
						SNew(SBox)
						.WidthOverride(30.f)
						.HeightOverride(30.f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SAssignNew(RowRefs.FavoriteGlyphImage, SImage)
							.Image(FriendFavoriteStarBrush.Get())
							.ColorAndOpacity(FLinearColor::White)
						]
					],
					Tag(*(RowTagPrefix + TEXT(".FavoriteButton"))),
					TEXT("Button"),
					ET66FlatState::Default,
					TOptional<FLinearColor>(),
					true,
					NAME_None,
					false,
					true)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::AttachMetadata(
					SAssignNew(RowRefs.ActionButton, SButton)
					.ButtonStyle(&NoBorderButtonStyle)
					.ContentPadding(FMargin(0.f))
					.IsEnabled_Lambda([this, FriendId, bFriendslopReferenceFixture, bFriendOnline = Friend.bOnline]()
					{
						if (bFriendslopReferenceFixture)
						{
							return bFriendOnline;
						}

						UGameInstance* ClickGI = UGameplayStatics::GetGameInstance(this);
						UT66PartySubsystem* ClickParty = ClickGI ? ClickGI->GetSubsystem<UT66PartySubsystem>() : nullptr;
						UT66SessionSubsystem* ClickSession = ClickGI ? ClickGI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
						const FT66PartyFriendEntry* FoundFriend = nullptr;
						if (ClickParty)
						{
							FoundFriend = ClickParty->GetFriends().FindByPredicate([&FriendId](const FT66PartyFriendEntry& Candidate)
							{
								return Candidate.PlayerId == FriendId;
							});
						}
						return FoundFriend
							&& FoundFriend->bOnline
							&& ClickParty
							&& !ClickParty->IsFriendInParty(FriendId)
							&& (!ClickSession || !ClickSession->IsFriendInvitePending(FriendId))
							&& ClickParty->GetPartyMemberCount() < 4;
					})
					.OnClicked_Lambda([this, FriendId, FriendName]()
					{
						UGameInstance* ClickGI = UGameplayStatics::GetGameInstance(this);
						if (UT66PartySubsystem* ClickParty = ClickGI ? ClickGI->GetSubsystem<UT66PartySubsystem>() : nullptr)
						{
							ClickParty->InviteFriend(FriendId, FriendName);
							RefreshFriendListVisualState();
						}
						return FReply::Handled();
					})
					[
						SNew(SBox)
						.WidthOverride(FriendActionButtonWidth)
						.HeightOverride(Friend.bOnline ? FriendOnlineActionHeight : FriendOfflineActionHeight)
						[
							FT66FriendslopStyle::MakeSurface(
								Friend.bOnline ? ET66FriendslopChrome::InviteButtonGreenRound06 : ET66FriendslopChrome::OfflineButtonDarkRound06,
								Friend.bOnline ? ET66FlatState::Default : ET66FlatState::Disabled,
								FMargin(8.f, 4.f),
								MakeAssignedLabel(
									RowRefs.ActionText,
									NSLOCTEXT("T66.MainMenu", "InviteFriend", "INVITE"),
									ET66FlatLabelRole::Button,
									ETextJustify::Center,
									Tag(*(RowTagPrefix + TEXT(".ActionText")))),
								nullptr,
								Tag(*(RowTagPrefix + TEXT(".ActionPanel"))))
						]
					],
					Tag(*(RowTagPrefix + TEXT(".ActionButton"))),
					TEXT("Button"),
					ET66FlatState::Default,
					TOptional<FLinearColor>(),
					true,
					NAME_None,
					false,
					true)
			];

		TSharedRef<SWidget> RowSurface = FT66FriendslopStyle::MakeCustomSurface(
			HellfireDir + TEXT("row_idle.png"),
			FMargin(0.12f, 0.30f),
			ESlateBrushDrawType::Box,
			FVector2D(540.f, 84.f),
			Friend.bOnline ? ET66FlatState::Default : ET66FlatState::Disabled,
			FMargin(12.f, 7.f),
			RowContent,
			nullptr,
			Tag(*RowTagPrefix));

		TSharedRef<SBox> RootBox = SAssignNew(RowRefs.RootBox, SBox)
			.HeightOverride(FriendRowHeight)
			[
				RowSurface
			];
		FriendRowWidgetRefs.Add(RowRefs);
		return RootBox;
	};

	auto AddFriendRowsForGroup = [&](const bool bOnlineGroup)
	{
		if (!FriendsForDisplay)
		{
			return;
		}

		int32 DisplayIndex = 0;
		for (const FT66PartyFriendEntry& Friend : *FriendsForDisplay)
		{
			if (Friend.bOnline != bOnlineGroup)
			{
				continue;
			}

			FriendsListContainer->AddSlot()
				.AutoHeight()
				.Padding(0.f, DisplayIndex == 0 ? 6.f : 5.f, 0.f, 0.f)
				[
					MakeFriendRow(Friend, DisplayIndex)
				];
			++DisplayIndex;
		}
	};

	auto MakeFriendsList = [&]() -> TSharedRef<SWidget>
	{
		SAssignNew(FriendsListContainer, SVerticalBox);
		FriendsListContainer->AddSlot()
			.AutoHeight()
			[
				MakeFriendGroupToggle(
					true,
					NSLOCTEXT("T66.MainMenu", "OnlineFriendsHeader", "ONLINE"),
					OnlineFriendCount,
					Tag(TEXT("MainMenu.Left.OnlineToggle")),
					Tag(TEXT("MainMenu.Left.OnlineLabel")),
					Tag(TEXT("MainMenu.Left.OnlineCount")))
			];
		AddFriendRowsForGroup(true);

		FriendsListContainer->AddSlot()
			.AutoHeight()
			.Padding(0.f, 10.f, 0.f, 4.f)
			[
				SAssignNew(FriendGroupsDividerBox, SBox)
				.HeightOverride(4.f)
				[
					FT66FlatStyle::MakeFlatDivider(
						Orient_Horizontal,
						0.f,
						TOptional<FLinearColor>(),
						Tag(TEXT("MainMenu.Left.FriendGroupsDivider")))
				]
			];

		FriendsListContainer->AddSlot()
			.AutoHeight()
			[
				MakeFriendGroupToggle(
					false,
					NSLOCTEXT("T66.MainMenu", "OfflineFriendsHeader", "OFFLINE"),
					OfflineFriendCount,
					Tag(TEXT("MainMenu.Left.OfflineToggle")),
					Tag(TEXT("MainMenu.Left.OfflineLabel")),
					Tag(TEXT("MainMenu.Left.OfflineCount")))
			];
		AddFriendRowsForGroup(false);

		FriendsListContainer->AddSlot()
			.AutoHeight()
			.Padding(0.f, 14.f, 0.f, 0.f)
			[
				SAssignNew(NoMatchingFriendsBox, SBox)
				.Visibility(EVisibility::Collapsed)
				[
					FT66FlatStyle::MakeFlatLabel(
						NSLOCTEXT("T66.MainMenu", "NoMatchingFriends", "No matching friends"),
						ET66FlatLabelRole::Caption,
						ETextJustify::Center,
						Tag(TEXT("MainMenu.Left.NoMatchingFriends")))
				]
			];

		TSharedRef<SScrollBox> FriendsScroll = SNew(SScrollBox)
			.Orientation(Orient_Vertical)
			.ScrollBarAlwaysVisible(false)
			+ SScrollBox::Slot()
			[
				FriendsListContainer.ToSharedRef()
			];

		return FriendsScroll;
	};

	auto MakePartySlots = [&]() -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
		Row->AddSlot()
			.AutoWidth()
			[
				SNew(SSpacer).Size(FVector2D(PartySidePad, 1.f))
			];
		for (int32 SlotIndex = 0; SlotIndex < 4; ++SlotIndex)
		{
			const FString TagName = FString::Printf(TEXT("MainMenu.Left.PartySlot%02d"), SlotIndex + 1);
			const TSharedRef<SWidget> SlotContent = SlotIndex == 0 && LocalProfileAvatarBrush.IsValid()
				? StaticCastSharedRef<SWidget>(SNew(SImage).Image(LocalProfileAvatarBrush.Get()).ColorAndOpacity(FLinearColor::White))
				: StaticCastSharedRef<SWidget>(SNew(STextBlock)
					.Text(FText::FromString(TEXT("+")))
					.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(24, true))
					.ColorAndOpacity(FT66FriendslopStyle::TextColorForState(ET66FlatState::Default))
					.Justification(ETextJustify::Center));
			Row->AddSlot()
				.AutoWidth()
				.Padding(SlotIndex == 0 ? FMargin(0.f) : FMargin(PartySlotGap, 0.f, 0.f, 0.f))
				[
					FT66FriendslopStyle::MakeCustomSurface(
						HellfireDir + TEXT("slot_party.png"),
						FMargin(0.24f),
						ESlateBrushDrawType::Box,
						FVector2D(150.f, 150.f),
						SlotIndex == 0 ? ET66FlatState::Selected : ET66FlatState::Default,
						FMargin(8.f),
						SNew(SBox)
						.WidthOverride(PartySlotSize - 16.f)
						.HeightOverride(PartySlotSize - 16.f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SlotContent
						],
						nullptr,
						FName(*TagName))
				];
		}
		Row->AddSlot()
			.AutoWidth()
			[
				SNew(SSpacer).Size(FVector2D(PartySidePad, 1.f))
			];
		return Row;
	};

	auto MakeLeftPanel = [&]() -> TSharedRef<SWidget>
	{
		TSharedRef<SConstraintCanvas> LeftCanvas = SNew(SConstraintCanvas);
		LeftCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(0.f, 16.f, LeftContentWidth, ProfileRowHeight))
			[
				MakeProfileButton()
			];
		LeftCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(0.f, 148.f, LeftContentWidth, 60.f))
			[
				MakeSearchField()
			];
		LeftCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(0.f, 220.f, LeftContentWidth, 504.f))
			[
				FT66FlatStyle::AttachMetadata(
					MakeSized(LeftContentWidth, 504.f, MakeFriendsList()),
					Tag(TEXT("MainMenu.Left.FriendsPanel")),
					TEXT("FriendsPanel"),
					ET66FlatState::Default)
			];
		LeftCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(0.f, 736.f, LeftContentWidth, 142.f))
			[
				FT66FlatStyle::AttachMetadata(
					MakeSized(LeftContentWidth, 142.f,
						SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(PartySidePad, 0.f, 0.f, 0.f)
					[
						MakeLabelBox(
							NSLOCTEXT("T66.MainMenu", "PartySection", "PARTY"),
							ET66FlatLabelRole::Header,
							Tag(TEXT("MainMenu.Left.PartyLabel")),
							LeftContentWidth - PartySidePad,
							36.f)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 12.f, 0.f, 0.f)
					[
						MakePartySlots()
					]),
					Tag(TEXT("MainMenu.Left.PartyPanel")),
					TEXT("PartyPanel"),
					ET66FlatState::Default)
			];

		return FT66FriendslopStyle::MakeCustomPanel(
			HellfireDir + TEXT("panel_side.png"),
			FMargin(0.10f, 0.08f),
			FVector2D(680.f, 920.f),
			ET66FlatState::Default,
			FMargin(LeftPanelContentInset, 26.f, LeftPanelContentInset, 26.f),
			LeftCanvas,
			nullptr,
			Tag(TEXT("MainMenu.Left.Panel")));
	};

	auto MakeCtaStack = [&]() -> TSharedRef<SWidget>
	{
		const bool bDailyDescentAvailable = IsDailyDescentAvailable();
		TSharedRef<SConstraintCanvas> CtaCanvas = SNew(SConstraintCanvas);
		CtaCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(0.f, 0.f, 680.f, 104.f))
			[
				MakeCtaButton(
					NSLOCTEXT("T66.MainMenu", "EnterTribulation", "ENTER TRIBULATION"),
					&UT66MainMenuScreen::HandleNewGameClicked,
					ET66FlatState::Selected,
					680.f,
					104.f,
					Tag(TEXT("MainMenu.Center.EnterTribulationButton")),
					HellfireBrush(TEXT("ic_skull.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(54.f, 54.f)),
					HellfireBrush(TEXT("ic_skull.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(54.f, 54.f)))
			];
		CtaCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(10.f, 150.f, 660.f, 94.f))
			[
				MakeCtaButton(
					NSLOCTEXT("T66.MainMenu", "Continue", "LOAD GAME"),
					&UT66MainMenuScreen::HandleLoadGameClicked,
					ET66FlatState::Default,
					660.f,
					94.f,
					Tag(TEXT("MainMenu.Center.LoadGameButton")),
					HellfireBrush(TEXT("ic_load.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(54.f, 54.f)))
			];
		if (bDailyDescentAvailable)
		{
			CtaCanvas->AddSlot()
				.Alignment(FVector2D(0.f, 0.f))
				.Offset(FMargin(10.f, 296.f, 660.f, 94.f))
				[
					MakeCtaButton(
						NSLOCTEXT("T66.MainMenu", "DailyDungeon", "DAILY DUNGEON"),
						&UT66MainMenuScreen::HandleDailyDescentClicked,
						ET66FlatState::Default,
						660.f,
						94.f,
						Tag(TEXT("MainMenu.Center.DailyDescentButton")),
						HellfireBrush(TEXT("ic_calendar.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(54.f, 54.f)),
						nullptr,
						true)
				];
		}

		return FT66FlatStyle::AttachMetadata(
			MakeSized(680.f, 390.f, CtaCanvas),
			Tag(TEXT("MainMenu.Center.CtaStack")),
			TEXT("CtaStack"),
			ET66FlatState::Default);
	};

	auto MakeFilterButton = [&](const int32 FilterIndex, const FText& Label, const FName InTag) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatButton(
			FlatMainMenuLeaderboardFilterIndex == FilterIndex ? ET66FlatState::Selected : ET66FlatState::Default,
			Label,
			FOnClicked::CreateLambda([this, FilterIndex]()
			{
				FlatMainMenuLeaderboardFilterIndex = FilterIndex;
				RequestDeferredSlateRebuild();
				return FReply::Handled();
			}),
			nullptr,
			nullptr,
			FMargin(8.f),
			136.f,
			72.f,
			true,
			18,
			InTag,
			LeaderboardFilterGroup);
	};

	auto MakeScopeButton = [&](const int32 ScopeIndex, const FText& Label, const FName InTag) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatButton(
			FlatMainMenuLeaderboardScopeIndex == ScopeIndex ? ET66FlatState::Selected : ET66FlatState::Default,
			Label,
			FOnClicked::CreateLambda([this, ScopeIndex]()
			{
				FlatMainMenuLeaderboardScopeIndex = ScopeIndex;
				RequestDeferredSlateRebuild();
				return FReply::Handled();
			}),
			nullptr,
			nullptr,
			FMargin(10.f, 6.f),
			215.f,
			57.f,
			true,
			18,
			InTag,
			LeaderboardScopeGroup);
	};

	auto MakeFlatDropdown = [&](const FText& Label, const FName InTag) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatDropdown(
			ET66FlatState::Default,
			TAttribute<FText>(Label),
			[]()
			{
				return StaticCastSharedRef<SWidget>(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						FT66FlatStyle::MakeFlatLabel(FText::FromString(TEXT("GLOBAL")), ET66FlatLabelRole::Body)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						FT66FlatStyle::MakeFlatLabel(FText::FromString(TEXT("FRIENDS")), ET66FlatLabelRole::Body)
					]);
			},
			false,
			215.f,
			57.f,
			18,
			InTag);
	};

	auto MakeRankingRow = [&](const FString& Rank, const FString& Name, const FString& Score, const FName InTag) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatButton(
			ET66FlatState::Default,
			FText::FromString(FString::Printf(TEXT("%s %s %s"), *Rank, *Name, *Score)),
			FOnClicked::CreateLambda([]()
			{
				return FReply::Handled();
			}),
			nullptr,
			nullptr,
			FMargin(10.f, 4.f),
			426.f,
			40.f,
			true,
			18,
			InTag);
	};

	auto MakeRightPanel = [&]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Column = SNew(SVerticalBox);
		Column->AddSlot()
			.AutoHeight()
			.Padding(0.f, 30.f, 0.f, 0.f)
			[
				FT66FlatStyle::MakeFlatLabel(
					NSLOCTEXT("T66.Leaderboard", "GlobalChadRanking", "GLOBAL CHAD RANKING"),
					ET66FlatLabelRole::Header,
					ETextJustify::Center,
					Tag(TEXT("MainMenu.Right.LeaderboardHeader")))
			];
		Column->AddSlot()
			.AutoHeight()
			.Padding(0.f, 18.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					MakeScopeButton(0, FText::FromString(TEXT("SCORE")), Tag(TEXT("MainMenu.Right.ScoreScopeButton")))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(10.f, 0.f, 0.f, 0.f)
				[
					MakeScopeButton(1, FText::FromString(TEXT("SPEED RUN")), Tag(TEXT("MainMenu.Right.SpeedrunScopeButton")))
				]
			];
		Column->AddSlot()
			.AutoHeight()
			.Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					MakeFlatDropdown(FText::FromString(TEXT("GLOBAL")), Tag(TEXT("MainMenu.Right.TypeDropdown")))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(10.f, 0.f, 0.f, 0.f)
				[
					MakeFlatDropdown(FText::FromString(TEXT("SOLO")), Tag(TEXT("MainMenu.Right.ModeDropdown")))
				]
			];
		Column->AddSlot()
			.AutoHeight()
			.Padding(0.f, 25.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					MakeLabelBox(FText::FromString(TEXT("RANK")), ET66FlatLabelRole::SubHeader, Tag(TEXT("MainMenu.Right.RankHeader")), 82.f, 26.f)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					MakeLabelBox(FText::FromString(TEXT("NAME")), ET66FlatLabelRole::SubHeader, Tag(TEXT("MainMenu.Right.NameHeader")), 270.f, 26.f)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					MakeLabelBox(FText::FromString(TEXT("SCORE")), ET66FlatLabelRole::SubHeader, Tag(TEXT("MainMenu.Right.ScoreHeader")), 80.f, 26.f, ETextJustify::Right)
				]
			];

		const TArray<TTuple<FString, FString, FString, FName>> Rows = {
			MakeTuple(FString(TEXT("#1")), FString(TEXT("CROWNED CHAD")), FString(TEXT("184250")), Tag(TEXT("MainMenu.Right.RankingRow01"))),
			MakeTuple(FString(TEXT("#2")), FString(TEXT("PIXEL WIZARD")), FString(TEXT("171900")), Tag(TEXT("MainMenu.Right.RankingRow02"))),
			MakeTuple(FString(TEXT("#3")), FString(TEXT("BOSS DELETE")), FString(TEXT("165420")), Tag(TEXT("MainMenu.Right.RankingRow03"))),
			MakeTuple(FString(TEXT("#4")), FString(TEXT("RUN GOD")), FString(TEXT("158760")), Tag(TEXT("MainMenu.Right.RankingRow04"))),
			MakeTuple(FString(TEXT("#5")), FString(TEXT("NO HIT NATE")), FString(TEXT("151300")), Tag(TEXT("MainMenu.Right.RankingRow05"))),
			MakeTuple(FString(TEXT("#6")), FString(TEXT("CRIT QUEEN")), FString(TEXT("146880")), Tag(TEXT("MainMenu.Right.RankingRow06"))),
			MakeTuple(FString(TEXT("#7")), FString(TEXT("LOOT LARRY")), FString(TEXT("139440")), Tag(TEXT("MainMenu.Right.RankingRow07"))),
			MakeTuple(FString(TEXT("#8")), FString(TEXT("STAGE SKIP")), FString(TEXT("133910")), Tag(TEXT("MainMenu.Right.RankingRow08"))),
			MakeTuple(FString(TEXT("#9")), FString(TEXT("MAGE MAIN")), FString(TEXT("128650")), Tag(TEXT("MainMenu.Right.RankingRow09"))),
			MakeTuple(FString(TEXT("#42")), FString(TEXT("DOPRA")), FString(TEXT("118700")), Tag(TEXT("MainMenu.Right.RankingRowLocal")))
		};

		for (int32 RowIndex = 0; RowIndex < Rows.Num(); ++RowIndex)
		{
			Column->AddSlot()
				.AutoHeight()
				.Padding(0.f, RowIndex == 0 ? 12.f : 9.f, 0.f, 0.f)
				[
					MakeRankingRow(
						Rows[RowIndex].Get<0>(),
						Rows[RowIndex].Get<1>(),
						Rows[RowIndex].Get<2>(),
						Rows[RowIndex].Get<3>())
				];
		}

		return FT66FlatStyle::AttachMetadata(
			MakeSized(440.f, 764.f, Column),
			Tag(TEXT("MainMenu.Right.LeaderboardPanel")),
			TEXT("LeaderboardPanel"),
			ET66FlatState::Default);
	};

	TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddCanvasSlot = [Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X, Y, W, H))
			[
				Widget
			];
	};

	AddCanvasSlot(
		0.f,
		0.f,
		1920.f,
		1080.f,
		FT66FlatStyle::AttachMetadata(
			BuildMainMenuBackgroundWidget(),
			Tag(TEXT("MainMenu.BackgroundRegion")),
			TEXT("BackgroundRegion"),
			ET66FlatState::Default));
	AddCanvasSlot(0.f, 130.f, LeftPanelWidth, 950.f, MakeLeftPanel());
	AddCanvasSlot(610.f, 70.f, 700.f, 250.f, MakeTitleRegion());
	AddCanvasSlot(620.f, 620.f, 680.f, 390.f, MakeCtaStack());
	constexpr float RightLeaderboardPanelWidth = ST66FlatLeaderboardPanel::GetPanelWidth();
	constexpr float RightLeaderboardPanelHeight = ST66FlatLeaderboardPanel::GetPanelHeight();
	AddCanvasSlot(
		1920.f - RightLeaderboardPanelWidth,
		130.f,
		RightLeaderboardPanelWidth,
		RightLeaderboardPanelHeight,
		SAssignNew(FlatLeaderboardPanel, ST66FlatLeaderboardPanel)
		.LocalizationSubsystem(Loc)
		.LeaderboardSubsystem(LB)
		.UIManager(UIManager)
		.TagPrefix(TEXT("MainMenu.Right")));

	const FVector2D MainMenuViewportSize = GetEffectiveFrontendViewportSize();
	TSharedRef<SWidget> RootContent =
		SNew(SBox)
		.WidthOverride(1920.f)
		.HeightOverride(1080.f)
		[
			Canvas
		];

	TSharedRef<SWidget> Root =
		FT66FlatStyle::AttachMetadata(
			SNew(SBox)
			.WidthOverride(FMath::Max(1.f, MainMenuViewportSize.X))
			.HeightOverride(FMath::Max(1.f, MainMenuViewportSize.Y))
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
				.Padding(0.f)
				[
					SNew(SDPIScaler)
					.DPIScale(TAttribute<float>::CreateLambda([]() -> float
					{
						return 1.f / GetMainMenuEngineDPIScale(GetEffectiveFrontendViewportSize());
					}))
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFit)
						.StretchDirection(EStretchDirection::Both)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							RootContent
						]
					]
				]
			],
			Tag(TEXT("MainMenu.Root")),
			TEXT("Root"),
			ET66FlatState::Default);

	RefreshFriendListVisualState();
	return Root;
}

TSharedRef<SWidget> UT66MainMenuScreen::BuildSlateUI()
{
	return BuildFlatMainMenuUI();
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
		RequestDeferredSlateRebuild();
	}
}

void UT66MainMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	UE_LOG(LogT66MainMenu, Verbose, TEXT("MainMenuScreen activated."));

	if (FlatLeaderboardPanel.IsValid())
	{
		FlatLeaderboardPanel->SetUIManager(UIManager);
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
		RequestDeferredSlateRebuild();
	}

	if (FlatLeaderboardPanel.IsValid())
	{
		FlatLeaderboardPanel->SetUIManager(UIManager);
	}
}

void UT66MainMenuScreen::ReleaseRetainedSlateState()
{
	FlatLeaderboardPanel.Reset();
	if (MainMenuBackgroundVideoPlayer)
	{
		MainMenuBackgroundVideoPlayer->CloseVideo();
		MainMenuBackgroundVideoPlayer = nullptr;
	}
	SkyBackgroundBrush.Reset();
	SkyBackgroundTexture.Reset();
	LocalProfileAvatarBrush.Reset();
	CtaSkullIconBrush.Reset();
	CtaSkullIconTexture.Reset();
	FriendFavoriteStarBrush.Reset();
	FriendFavoriteStarTexture.Reset();
	DailyDescentIconBrush.Reset();
	DailyDescentIconTexture.Reset();
	FriendAvatarBrushes.Reset();
	FriendsListContainer.Reset();
	FriendGroupWidgetRefs.Reset();
	FriendRowWidgetRefs.Reset();
	FriendGroupsDividerBox.Reset();
	NoMatchingFriendsBox.Reset();
}

void UT66MainMenuScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	// Rebuild UI when language changes
	RequestDeferredSlateRebuild();
	if (FlatLeaderboardPanel.IsValid())
	{
		FlatLeaderboardPanel->SetUIManager(UIManager);
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
	RequestDeferredSlateRebuild();
}

void UT66MainMenuScreen::HandleSessionStateChanged()
{
	SyncToSharedPartyScreen();
	RequestDeferredSlateRebuild();
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

		if (RowRefs.FavoriteButton.IsValid())
		{
			RowRefs.FavoriteButton->SetToolTipText(
				bFavorite
					? NSLOCTEXT("T66.MainMenu", "UnfavoriteFriendTooltip", "Remove favorite")
					: NSLOCTEXT("T66.MainMenu", "FavoriteFriendTooltip", "Favorite friend"));
		}

		if (RowRefs.FavoriteGlyphImage.IsValid())
		{
			RowRefs.FavoriteGlyphImage->SetColorAndOpacity(
				bFavorite
					? FLinearColor::White
					: FLinearColor(1.f, 1.f, 1.f, 0.46f));
			RowRefs.FavoriteGlyphImage->SetVisibility(EVisibility::Visible);
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
					? FLinearColor(0.98f, 0.96f, 1.0f, 1.0f)
					: FLinearColor(0.64f, 0.44f, 0.42f, 0.88f));
			RowRefs.ActionText->SetVisibility(EVisibility::Visible);
		}

		if (RowRefs.ActionFillBorder.IsValid())
		{
			RowRefs.ActionFillBorder->SetBorderBackgroundColor(FT66FlatStyle::BorderForState(
				bCanInvite ? ET66FlatState::Default : ET66FlatState::Disabled));
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
		if (GroupRefs.ExpandArrowText.IsValid())
		{
			const bool bExpanded = GroupRefs.bOnlineGroup ? bShowOnlineFriends : bShowOfflineFriends;
			GroupRefs.ExpandArrowText->SetText(FText::FromString(bExpanded ? TEXT("v") : TEXT(">")));
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
	// UI Reimagine 2026-06-10: hellfire backdrop replaces video/sky when present.
	if (const FSlateBrush* HellfireBackground = FT66FriendslopStyle::GetCustomBrush(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/MainMenu/bg_hellfire_altar.png"),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			FVector2D(1536.f, 1024.f));
		HellfireBackground && HellfireBackground->GetResourceObject())
	{
		return SNew(SImage).Image(HellfireBackground);
	}

	const FSlateBrush* VideoBrush = MainMenuBackgroundVideoPlayer
		? MainMenuBackgroundVideoPlayer->GetVideoBrush()
		: nullptr;

	if ((!SkyBackgroundBrush.IsValid() || !SkyBackgroundBrush->GetResourceObject()) && !VideoBrush)
	{
		return SNew(SImage).Image(SkyBackgroundBrush.Get());
	}

	TSharedRef<SOverlay> Background = SNew(SOverlay);

	if (SkyBackgroundBrush.IsValid() && SkyBackgroundBrush->GetResourceObject())
	{
		Background->AddSlot()
		[
			SNew(SImage).Image(SkyBackgroundBrush.Get())
		];
	}

	if (VideoBrush)
	{
		Background->AddSlot()
		[
			SNew(SImage).Image(VideoBrush)
		];
	}

	return Background;
}

void UT66MainMenuScreen::RequestBackgroundTexture()
{
	const TCHAR* BackgroundPath = TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/mainmenu_screen_art_mainmenu_newmm_rubbery_friendslop_pass25_1920.png");

	SetupT66MainMenuRuntimeImageBrush(
		SkyBackgroundBrush,
		SkyBackgroundTexture,
		nullptr,
		BackgroundPath,
		FVector2D(T66MainMenuReferenceLayout::CanvasWidth, T66MainMenuReferenceLayout::CanvasHeight));

	if (MainMenuBackgroundVideoPlayer)
	{
		MainMenuBackgroundVideoPlayer->CloseVideo();
		MainMenuBackgroundVideoPlayer = nullptr;
	}

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

FReply UT66MainMenuScreen::HandleDailyDescentClicked()
{
	OnDailyDescentClicked();
	return FReply::Handled();
}

FReply UT66MainMenuScreen::HandlePowerUpClicked()
{
	OnPowerUpClicked();
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

	RequestDeferredSlateRebuild();
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

void UT66MainMenuScreen::OnDailyDescentClicked()
{
	if (!FT66ShelvedFeatureGate::IsDailyDescentEnabled())
	{
		return;
	}

	const UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	const UT66ReleaseVariantSubsystem* ReleaseVariant = GI ? GI->GetSubsystem<UT66ReleaseVariantSubsystem>() : nullptr;
	if (ReleaseVariant && ReleaseVariant->IsDemoModeActive())
	{
		return;
	}

	NavigateTo(ET66ScreenType::DailyDescent);
}

void UT66MainMenuScreen::OnPowerUpClicked()
{
	NavigateTo(ET66ScreenType::PowerUp);
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
