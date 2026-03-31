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
#include "Core/T66RunSaveGame.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "GameFramework/GameUserSettings.h"
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
#include "Widgets/Layout/SScaleBox.h"
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
		FVector2D EffectiveSize = FT66Style::GetViewportSize();

		if (GEngine)
		{
			if (UGameUserSettings* GameUserSettings = GEngine->GetGameUserSettings())
			{
				const FIntPoint ScreenResolution = GameUserSettings->GetScreenResolution();
				if (ScreenResolution.X > 0 && ScreenResolution.Y > 0
					&& GameUserSettings->GetFullscreenMode() != EWindowMode::Windowed)
				{
					EffectiveSize.X = FMath::Max(EffectiveSize.X, static_cast<float>(ScreenResolution.X));
					EffectiveSize.Y = FMath::Max(EffectiveSize.Y, static_cast<float>(ScreenResolution.Y));
				}
			}
		}

		return EffectiveSize;
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
	// This screen already computes its own viewport-relative layout, so applying
	// the shared responsive root would scale the entire menu a second time.
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

	const FText NewGameText = Loc ? Loc->GetText_NewGame() : NSLOCTEXT("T66.MainMenu", "NewGame", "NEW GAME");
	const FText LoadGameText = Loc ? Loc->GetText_LoadGame() : NSLOCTEXT("T66.MainMenu", "LoadGame", "LOAD GAME");
	SetupRuntimeImageBrush(
		PrimaryCTAFillBrush,
		PrimaryCTAFillTexture,
		TEXT("SourceAssets/UI/MainMenuGenerated/mainmenu_cta_fill_green.png"),
		FVector2D(1024.f, 232.f));

	struct FLastRunSnapshot
	{
		FName HeroID = NAME_None;
		ET66BodyType HeroBodyType = ET66BodyType::TypeA;
		ET66Difficulty Difficulty = ET66Difficulty::Easy;
		ET66PartySize PartySize = ET66PartySize::Solo;
		int32 StageReached = 1;
		bool bHasSave = false;
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

	FLastRunSnapshot LastRun;
	FString LatestUtc;
	int32 LatestSlot = INDEX_NONE;
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
				if (LatestSlot == INDEX_NONE || (!SlotUtc.IsEmpty() && (LatestUtc.IsEmpty() || SlotUtc > LatestUtc)))
				{
					LatestSlot = SlotIndex;
					LatestUtc = SlotUtc;
				}
			}
		}
	}

	if (LatestSlot != INDEX_NONE)
	{
		if (UT66RunSaveGame* LatestSave = SaveSubsystem ? SaveSubsystem->LoadFromSlot(LatestSlot) : nullptr)
		{
			LastRun.HeroID = LatestSave->HeroID;
			LastRun.HeroBodyType = LatestSave->HeroBodyType;
			LastRun.Difficulty = LatestSave->Difficulty;
			LastRun.PartySize = LatestSave->PartySize;
			LastRun.StageReached = LatestSave->StageReached;
			LastRun.bHasSave = true;
		}
	}

	if (!LastRun.bHasSave && T66GI)
	{
		LastRun.HeroID = T66GI->SelectedHeroID;
		LastRun.HeroBodyType = T66GI->SelectedHeroBodyType;
		LastRun.Difficulty = T66GI->SelectedDifficulty;
		LastRun.PartySize = T66GI->SelectedPartySize;
		LastRun.StageReached = 1;
	}

	TArray<FName> AllHeroIDs = T66GI ? T66GI->GetAllHeroIDs() : TArray<FName>();
	if (LastRun.HeroID.IsNone() && AllHeroIDs.Num() > 0)
	{
		LastRun.HeroID = AllHeroIDs[0];
	}

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

	auto PickHeroID = [&AllHeroIDs, &LastRun](int32 Index) -> FName
	{
		if (AllHeroIDs.IsValidIndex(Index) && !AllHeroIDs[Index].IsNone())
		{
			return AllHeroIDs[Index];
		}
		return LastRun.HeroID;
	};

	TArray<FMenuFriendEntry> Friends;
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
	LastRunHeroBrush->ImageSize = FVector2D(84.f, 84.f);
	LastRunHeroBrush->SetResourceObject(TexPool ? T66SlateTexture::GetLoaded(TexPool, LastRunPortraitSoft) : nullptr);

	FriendPortraitBrushes.Reset();
	for (const FMenuFriendEntry& Friend : Friends)
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = FVector2D(44.f, 44.f);
		const TSoftObjectPtr<UTexture2D> FriendPortraitSoft = ResolvePortraitSoft(Friend.HeroID, Friend.BodyType, ET66HeroPortraitVariant::Low);
		Brush->SetResourceObject(TexPool ? T66SlateTexture::GetLoaded(TexPool, FriendPortraitSoft) : nullptr);
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
	const FLinearColor PartySlotAccentInactive(0.08f, 0.09f, 0.10f, 1.0f);
	const FLinearColor PlaceholderTint(0.20f, 0.22f, 0.24f, 0.55f);
	const float CenterColumnWidth = 500.f;
	const float ColumnHeight = 770.f;
	const float CenterButtonWidth = 500.f;
	const float CenterButtonHeight = 114.f;
	const float CenterButtonGap = 26.f;
	const float CenterColumnHeight = (CenterButtonHeight * 2.f) + CenterButtonGap;
	const float TopInset = UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f;
	const FVector2D ViewportSize = GetEffectiveFrontendViewportSize();
	const float HorizontalFramePadding = 44.f;
	const float BottomFramePadding = 34.f;
	const FVector2D ReferenceViewportSize(1920.f, 1080.f);
	const float ReferenceTopInset = 118.f;
	const float ReferenceContentWidthBudget = ReferenceViewportSize.X - (HorizontalFramePadding * 2.f);
	const float AvailableColumnHeight = FMath::Max(1.f, ViewportSize.Y - TopInset - BottomFramePadding);
	const float ReferenceAvailableColumnHeight = FMath::Max(1.f, ReferenceViewportSize.Y - ReferenceTopInset - BottomFramePadding);
	const float AvailableContentWidth = FMath::Max(1.f, ViewportSize.X - (HorizontalFramePadding * 2.f));
	const float WidthResponsiveScale = AvailableContentWidth / FMath::Max(ReferenceContentWidthBudget, 1.f);
	const float HeightResponsiveScale = AvailableColumnHeight / FMath::Max(ReferenceAvailableColumnHeight, 1.f);
	const float LayoutResponsiveScale = FMath::Clamp(
		FMath::Sqrt(WidthResponsiveScale * HeightResponsiveScale),
		0.70f,
		3.0f);
	const float LeaderboardPanelTargetWidth = FMath::Max(300.f, (ReferenceContentWidthBudget * 0.31f) * LayoutResponsiveScale);
	const float ColumnTargetHeight = FMath::Min(AvailableColumnHeight, FMath::Max(540.f, ColumnHeight * LayoutResponsiveScale));
	const float SidePanelTargetHeight = ColumnTargetHeight;
	const float CenterSlotMinimumWidth = 280.f;
	const float UnifiedSidePanelTargetWidth = LeaderboardPanelTargetWidth + HorizontalFramePadding;
	const float LeaderboardShellTargetWidth = FMath::Max(340.f, UnifiedSidePanelTargetWidth * 0.84f);
	const float SocialPanelTargetWidth = FMath::Max(280.f, UnifiedSidePanelTargetWidth * 0.76f);
	const float SideColumnsCombinedTargetWidth = SocialPanelTargetWidth + LeaderboardShellTargetWidth;
	const float SideColumnsMaxWidth = FMath::Max(1.f, AvailableContentWidth - CenterSlotMinimumWidth - (HorizontalFramePadding * 2.f));
	const float SideColumnsWidthScale = SideColumnsCombinedTargetWidth > 1.f
		? FMath::Min(1.f, SideColumnsMaxWidth / SideColumnsCombinedTargetWidth)
		: 1.f;
	const float ResolvedSidePanelWidth = SocialPanelTargetWidth * SideColumnsWidthScale;
	const float ResolvedLeaderboardPanelWidth = LeaderboardShellTargetWidth * SideColumnsWidthScale;
	const float CenterButtonLift = FMath::Max(80.f, (ColumnTargetHeight - CenterColumnHeight) * 0.30f);
	const int32 LeftPanelBodyFontSize = 14;
	const int32 LeftPanelTitleFontSize = 14;
	const int32 CenterButtonFontSize = 42;

	auto MakeRadianceFont = [](int32 Size, int32 LetterSpacing = 0) -> FSlateFontInfo
	{
		FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Regular"), Size);
		Font.LetterSpacing = LetterSpacing;
		return Font;
	};

	auto MakeSectionTitle = [HeaderText, LeftPanelTitleFontSize](const FText& Text) -> TSharedRef<SWidget>
	{
		FSlateFontInfo HeaderFont = FT66Style::MakeFont(TEXT("Bold"), LeftPanelTitleFontSize + 2);
		HeaderFont.LetterSpacing = 140;

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
		GroupFont.LetterSpacing = 120;
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
			+ SHorizontalBox::Slot().AutoWidth().Padding(12.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(GroupFont)
				.ColorAndOpacity(HeaderText)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::Format(NSLOCTEXT("T66.MainMenu", "FriendsGroupCount", "({0})"), FText::AsNumber(Count)))
				.Font(GroupFont)
				.ColorAndOpacity(CountColor)
			];
	};

	const FText InviteFriendText = NSLOCTEXT("T66.MainMenu", "InviteFriend", "INVITE");
	const FText RemoveFriendText = NSLOCTEXT("T66.MainMenu", "RemoveFriend", "REMOVE");
	const FText InPartyText = NSLOCTEXT("T66.MainMenu", "InParty", "In Party");
	const FText PartyFullText = NSLOCTEXT("T66.MainMenu", "PartyFull", "PARTY FULL");
	const FText OfflineActionText = NSLOCTEXT("T66.MainMenu", "FriendOffline", "OFFLINE");
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
			.WidthOverride(100.f)
			.HeightOverride(38.f)
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
					.Padding(FMargin(12.f, 7.f))
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
			? RemoveFriendText
			: (!Friend.bOnline
				? OfflineActionText
				: ((PartySubsystem && PartySubsystem->GetPartyMemberCount() >= 4) ? PartyFullText : InviteFriendText));

		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(bInParty ? FLinearColor(0.10f, 0.14f, 0.18f, 0.55f) : FLinearColor(0.f, 0.f, 0.f, 0.20f))
			.Padding(8.f, 6.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(54.f)
					.HeightOverride(54.f)
					[
						FT66Style::MakeSlotFrame(
							SNew(SImage)
							.Image(FriendPortraitBrushes.IsValidIndex(FriendIndex) ? FriendPortraitBrushes[FriendIndex].Get() : nullptr)
							.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Friend.bOnline ? 1.0f : 0.74f)),
							AccentColor,
							FMargin(1.f))
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(12.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
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
				+ SHorizontalBox::Slot().AutoWidth().Padding(12.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
				[
					MakeFlatActionButton(
						ActionText,
						FOnClicked::CreateLambda([this, PartySubsystem, PlayerId = Friend.PlayerId, bInParty]()
						{
							if (!PartySubsystem)
							{
								return FReply::Handled();
							}

							if (bInParty)
							{
								PartySubsystem->RemovePartyMember(PlayerId);
							}
							else
							{
								PartySubsystem->InviteFriend(PlayerId);
							}

							ForceRebuildSlate();
							return FReply::Handled();
						}),
						bCanInvite || bInParty,
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
			.SetPadding(FMargin(28.f, 22.f, 28.f, 18.f))
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

	const int32 ActivePartySlots = PartySubsystem
		? FMath::Clamp(PartySubsystem->GetPartyMemberCount(), 1, 4)
		: 1;

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

			FriendsList->AddSlot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				MakeFriendRow(Friend, FriendIndex)
			];
		}
	};

	AddFriendGroup(true);
	FriendsList->AddSlot().AutoHeight().Padding(0.f, 26.f, 0.f, 0.f)[SNew(SSpacer).Size(FVector2D(1.f, 1.f))];
	AddFriendGroup(false);

	TSharedRef<SHorizontalBox> PartySlots = SNew(SHorizontalBox);
	for (int32 SlotIndex = 0; SlotIndex < 4; ++SlotIndex)
	{
		const bool bPartyEnabledSlot = SlotIndex < ActivePartySlots;
		const bool bLeaderSlot = SlotIndex == 0;
		const FLinearColor SlotAccent = bLeaderSlot
			? LeaderSlotAccent
			: (bPartyEnabledSlot ? PartySlotAccent : PartySlotAccentInactive);
		const float PlaceholderOpacity = bPartyEnabledSlot ? 0.55f : 0.28f;

		const TSharedRef<SWidget> SlotContent =
			bLeaderSlot
			? StaticCastSharedRef<SWidget>(
				SNew(SImage)
				.Image(LastRunHeroBrush.Get()))
			: StaticCastSharedRef<SWidget>(
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(0.f, 12.f, 0.f, 0.f)
				[
					SNew(SBox)
					.WidthOverride(14.f)
					.HeightOverride(14.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(PlaceholderTint.R, PlaceholderTint.G, PlaceholderTint.B, PlaceholderOpacity))
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Bottom)
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					SNew(SBox)
					.WidthOverride(26.f)
					.HeightOverride(18.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(PlaceholderTint.R, PlaceholderTint.G, PlaceholderTint.B, PlaceholderOpacity))
					]
				]);

		PartySlots->AddSlot()
		.AutoWidth()
		.Padding(SlotIndex > 0 ? FMargin(8.f, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			SNew(SBox)
			.WidthOverride(70.f)
			.HeightOverride(70.f)
			[
				FT66Style::MakeSlotFrame(SlotContent, SlotAccent, bLeaderSlot ? FMargin(1.f) : FMargin(7.f))
			]
		];
	}

	const TSharedRef<SWidget> LastRunRow =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(78.f)
			.HeightOverride(78.f)
			[
				FT66Style::MakeSlotFrame(
					SNew(SImage)
					.Image(LastRunHeroBrush.Get()),
					LeaderSlotAccent,
					FMargin(1.f))
			]
		]
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(14.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(ResolveHeroName(LastRun.HeroID))
				.Font(FT66Style::MakeFont(TEXT("Regular"), LeftPanelBodyFontSize))
				.ColorAndOpacity(BrightText)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::Format(
					NSLOCTEXT("T66.MainMenu", "LastRunStage", "Stage {0}"),
					FText::AsNumber(LastRun.StageReached)))
				.Font(FT66Style::MakeFont(TEXT("Regular"), LeftPanelBodyFontSize))
				.ColorAndOpacity(MutedText)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::Format(
					NSLOCTEXT("T66.MainMenu", "LastRunMeta", "{0}  |  {1}"),
					Loc ? Loc->GetText_Difficulty(LastRun.Difficulty) : NSLOCTEXT("T66.Difficulty", "Easy", "Easy"),
					GetPartySizeText(LastRun.PartySize)))
				.Font(FT66Style::MakeFont(TEXT("Regular"), LeftPanelBodyFontSize))
				.ColorAndOpacity(MutedText)
			]
		];

	const TSharedRef<SWidget> LeftSocialShell =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(ShellFill)
		.Padding(FMargin(18.f, 18.f, 18.f, 16.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				MakeSectionTitle(NSLOCTEXT("T66.MainMenu", "LastRunSection", "LAST RUN"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				LastRunRow
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 20.f, 0.f, 0.f)
			[
				MakeSubtleDivider()
			]
			+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 18.f, 0.f, 0.f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					FriendsList
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
			[
				MakeSectionTitle(NSLOCTEXT("T66.MainMenu", "PartySection", "PARTY"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				PartySlots
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
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(0.f, TopInset, 0.f, 0.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Bottom)
				[
					SNew(SBox)
					.WidthOverride(ResolvedSidePanelWidth)
					.HeightOverride(SidePanelTargetHeight)
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
				.Padding(0.f, 0.f, 0.f, BottomFramePadding + CenterButtonLift)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					.StretchDirection(EStretchDirection::DownOnly)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Bottom)
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
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				[
					SNew(SBox)
					.WidthOverride(ResolvedLeaderboardPanelWidth)
					.HeightOverride(SidePanelTargetHeight)
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

	// Subscribe to language changes
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66MainMenuScreen::OnLanguageChanged);
	}
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

// UFUNCTION handlers (call navigation)
void UT66MainMenuScreen::OnNewGameClicked()
{
	bool bShouldOpenLobby = false;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bIsNewGameFlow = true;
		GI->bHeroSelectionFromLobby = false;
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->ApplyCurrentPartyToGameInstanceRunContext();
			bShouldOpenLobby = PartySubsystem->HasRemotePartyMembers();
		}
	}
	NavigateTo(bShouldOpenLobby ? ET66ScreenType::Lobby : ET66ScreenType::HeroSelection);
}

void UT66MainMenuScreen::OnLoadGameClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bIsNewGameFlow = false;
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
