// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniMainMenuScreen.h"

#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "Styling/CoreStyle.h"
#include "UI/Components/T66LeaderboardPanel.h"
#include "UI/ST66AnimatedBackground.h"
#include "UI/ST66AnimatedBorderGlow.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UITypes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SInvalidationPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FVector2D MiniBgSize(1920.f, 1080.f);

	void EnsureBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
	}

	void SetupLooseBrush(
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& TextureHandle,
		const TCHAR* RelativePath,
		const FVector2D& ImageSize,
		const bool bGenerateMips,
		const TCHAR* DebugLabel)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
		}
		EnsureBrush(Brush, ImageSize);

		if (!TextureHandle.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (!FPaths::FileExists(CandidatePath))
				{
					continue;
				}

				UTexture2D* Texture = bGenerateMips
					? T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Trilinear, DebugLabel)
					: T66RuntimeUITextureAccess::ImportFileTexture(CandidatePath, TextureFilter::TF_Trilinear, false, DebugLabel);
				if (Texture)
				{
					TextureHandle.Reset(Texture);
					break;
				}
			}
		}

		Brush->SetResourceObject(TextureHandle.Get());
		if (TextureHandle.IsValid())
		{
			Brush->ImageSize = FVector2D(
				FMath::Max(1, TextureHandle->GetSizeX()),
				FMath::Max(1, TextureHandle->GetSizeY()));
		}
	}

	void SetupCookedBrush(
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& TextureHandle,
		const TCHAR* AssetPath,
		const FVector2D& ImageSize,
		const TCHAR* DebugLabel)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
		}
		EnsureBrush(Brush, ImageSize);

		if (!TextureHandle.IsValid())
		{
			if (UTexture2D* Texture = T66RuntimeUITextureAccess::LoadAssetTexture(
				AssetPath,
				TextureFilter::TF_Trilinear,
				DebugLabel))
			{
				TextureHandle.Reset(Texture);
			}
		}

		Brush->SetResourceObject(TextureHandle.Get());
		if (TextureHandle.IsValid())
		{
			Brush->ImageSize = FVector2D(
				FMath::Max(1, TextureHandle->GetSizeX()),
				FMath::Max(1, TextureHandle->GetSizeY()));
		}
	}

	TSharedPtr<FSlateBrush> MakeAvatarBrush(UTexture2D* Texture, const FVector2D& ImageSize)
	{
		if (!Texture)
		{
			return nullptr;
		}

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
		Brush->SetResourceObject(Texture);
		return Brush;
	}

	bool DoesFriendMatchQuery(const FString& Query, const FT66PartyFriendEntry& Friend)
	{
		const FString Normalized = Query.TrimStartAndEnd();
		return Normalized.IsEmpty() || Friend.DisplayName.Contains(Normalized, ESearchCase::IgnoreCase);
	}
}

UT66MiniMainMenuScreen::UT66MiniMainMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MiniMainMenu;
	bIsModal = false;
}

void UT66MiniMainMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	if (LeaderboardPanel.IsValid())
	{
		LeaderboardPanel->SetUIManager(UIManager);
	}
}

void UT66MiniMainMenuScreen::OnScreenDeactivated_Implementation()
{
	ReleaseRetainedSlateState();
	Super::OnScreenDeactivated_Implementation();
}

void UT66MiniMainMenuScreen::NativeDestruct()
{
	ReleaseRetainedSlateState();
	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66MiniMainMenuScreen::BuildSlateUI()
{
	RequestMiniMenuTextures();

	UGameInstance* GameInstance = GetGameInstance();
	UT66LocalizationSubsystem* LocSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LeaderboardSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66PartySubsystem* PartySubsystem = GameInstance ? GameInstance->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SteamHelper* SteamHelper = GameInstance ? GameInstance->GetSubsystem<UT66SteamHelper>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;

	const FLinearColor ShellFill(0.004f, 0.005f, 0.010f, 0.985f);
	const FLinearColor BrightText(0.86f, 0.87f, 0.89f, 1.0f);
	const FLinearColor HeaderText(0.66f, 0.68f, 0.71f, 1.0f);
	const FLinearColor MutedText(0.50f, 0.52f, 0.56f, 1.0f);
	const FLinearColor DividerColor(0.20f, 0.22f, 0.25f, 0.48f);
	const FLinearColor OnlineAccent(0.13f, 0.22f, 0.30f, 1.0f);
	const FLinearColor OfflineAccent(0.08f, 0.09f, 0.11f, 1.0f);
	const FLinearColor PartyAccent(0.15f, 0.17f, 0.19f, 1.0f);
	const float SideWidth = 382.f;
	const float SideHeight = 562.f;
	const float CenterWidth = 392.f;
	const float CenterButtonWidth = 352.f;
	const float CenterButtonHeight = 72.f;
	const float CenterButtonGap = 16.f;
	const float CenterHeight = (CenterButtonHeight * 2.f) + CenterButtonGap + 30.f;
	const float TitleTopPadding = 42.f;
	const float SceneOffset = 128.f;

	TArray<FT66PartyFriendEntry> OnlineFriends;
	TArray<FT66PartyFriendEntry> OfflineFriends;
	if (PartySubsystem)
	{
		for (const FT66PartyFriendEntry& Friend : PartySubsystem->GetFriends())
		{
			if (!DoesFriendMatchQuery(FriendSearchQuery, Friend))
			{
				continue;
			}

			(Friend.bOnline ? OnlineFriends : OfflineFriends).Add(Friend);
		}
	}

	TArray<FT66PartyMemberEntry> PartyMembers = PartySubsystem ? PartySubsystem->GetPartyMembers() : TArray<FT66PartyMemberEntry>();
	if (PartyMembers.Num() == 0)
	{
		FT66PartyMemberEntry LocalMember;
		LocalMember.DisplayName = SteamHelper && !SteamHelper->GetLocalDisplayName().IsEmpty() ? SteamHelper->GetLocalDisplayName() : TEXT("Local Player");
		LocalMember.bIsLocal = true;
		LocalMember.bOnline = SteamHelper && SteamHelper->IsSteamReady();
		PartyMembers.Add(LocalMember);
	}

	ProfileAvatarBrush = MakeAvatarBrush(SteamHelper ? SteamHelper->GetLocalAvatarTexture() : nullptr, FVector2D(76.f, 76.f));
	FriendAvatarBrushes.Reset();
	PartyAvatarBrushes.Reset();

	auto MakeAvatarSlot = [&](const TSharedPtr<FSlateBrush>& Brush, const FVector2D& Size, const FString& Fallback, const FLinearColor& Accent) -> TSharedRef<SWidget>
	{
		const TSharedRef<SWidget> Content = Brush.IsValid()
			? StaticCastSharedRef<SWidget>(SNew(SImage).Image(Brush.Get()))
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(FText::FromString(Fallback))
				.Font(FT66Style::MakeFont(TEXT("Bold"), 16))
				.ColorAndOpacity(MutedText)
				.Justification(ETextJustify::Center));

		return SNew(SBox)
			.WidthOverride(Size.X)
			.HeightOverride(Size.Y)
			[
				FT66Style::MakeSlotFrame(Content, Accent, FMargin(1.f))
			];
	};

	auto MakeSectionTitle = [&](const FText& Text) -> TSharedRef<SWidget>
	{
		FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Bold"), 13);
		Font.LetterSpacing = 112;
		return SNew(STextBlock).Text(Text).Font(Font).ColorAndOpacity(HeaderText);
	};

	auto MakeDivider = [&]() -> TSharedRef<SWidget>
	{
		return SNew(SBox).HeightOverride(1.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(DividerColor)
			];
	};

	auto MakeFriendGroupHeader = [&](const FText& Label, const int32 Count, const FLinearColor& Color) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(FText::Format(NSLOCTEXT("T66Mini.MainMenu", "FriendHeaderFmt", "{0}  ({1})"), Label, FText::AsNumber(Count)))
			.Font(FT66Style::MakeFont(TEXT("Bold"), 11))
			.ColorAndOpacity(Color);
	};

	auto MakeFriendRow = [&](const FT66PartyFriendEntry& Friend) -> TSharedRef<SWidget>
	{
		TSharedPtr<FSlateBrush> AvatarBrush = MakeAvatarBrush(
			SteamHelper ? SteamHelper->GetAvatarTextureForSteamId(Friend.PlayerId) : nullptr,
			FVector2D(40.f, 40.f));
		FriendAvatarBrushes.Add(AvatarBrush);

		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.03f, 0.04f, 0.06f, 0.92f))
			.Padding(FMargin(8.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					MakeAvatarSlot(
						AvatarBrush,
						FVector2D(42.f, 42.f),
						Friend.DisplayName.IsEmpty() ? TEXT("?") : Friend.DisplayName.Left(1).ToUpper(),
						Friend.bOnline ? OnlineAccent : OfflineAccent)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(10.f, 0.f, 0.f, 0.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Friend.DisplayName))
						.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
						.ColorAndOpacity(BrightText)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Friend.PresenceText.IsEmpty() ? (Friend.bOnline ? TEXT("Online") : TEXT("Offline")) : Friend.PresenceText))
						.Font(FT66Style::MakeFont(TEXT("Regular"), 10))
						.ColorAndOpacity(MutedText)
					]
				]
			];
	};

	const FString ProfileName = SteamHelper && !SteamHelper->GetLocalDisplayName().IsEmpty()
		? SteamHelper->GetLocalDisplayName()
		: TEXT("Local Player");
	const FText ProfileStatus = SteamHelper && SteamHelper->IsSteamReady()
		? NSLOCTEXT("T66Mini.MainMenu", "ProfileSteamReady", "Steam Online")
		: NSLOCTEXT("T66Mini.MainMenu", "ProfileSteamMissing", "Steam Unavailable");

	TSharedRef<SVerticalBox> FriendsList = SNew(SVerticalBox);
	FriendsList->AddSlot().AutoHeight()[MakeFriendGroupHeader(NSLOCTEXT("T66Mini.MainMenu", "Online", "ONLINE"), OnlineFriends.Num(), FLinearColor(0.42f, 0.67f, 0.96f, 1.0f))];
	for (int32 Index = 0; Index < OnlineFriends.Num() && Index < 3; ++Index)
	{
		FriendsList->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[MakeFriendRow(OnlineFriends[Index])];
	}
	FriendsList->AddSlot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)[MakeDivider()];
	FriendsList->AddSlot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)[MakeFriendGroupHeader(NSLOCTEXT("T66Mini.MainMenu", "Offline", "OFFLINE"), OfflineFriends.Num(), HeaderText)];
	for (int32 Index = 0; Index < OfflineFriends.Num() && Index < 3; ++Index)
	{
		FriendsList->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[MakeFriendRow(OfflineFriends[Index])];
	}

	TSharedRef<SHorizontalBox> PartySlots = SNew(SHorizontalBox);
	for (int32 SlotIndex = 0; SlotIndex < 4; ++SlotIndex)
	{
		const FT66PartyMemberEntry* Member = PartyMembers.IsValidIndex(SlotIndex) ? &PartyMembers[SlotIndex] : nullptr;
		TSharedPtr<FSlateBrush> AvatarBrush = MakeAvatarBrush(
			(SteamHelper && Member && !Member->PlayerId.IsEmpty()) ? SteamHelper->GetAvatarTextureForSteamId(Member->PlayerId) : (Member && Member->bIsLocal && SteamHelper ? SteamHelper->GetLocalAvatarTexture() : nullptr),
			FVector2D(56.f, 56.f));
		PartyAvatarBrushes.Add(AvatarBrush);

		PartySlots->AddSlot().FillWidth(1.f).Padding(SlotIndex > 0 ? FMargin(8.f, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				MakeAvatarSlot(
					AvatarBrush,
					FVector2D(58.f, 58.f),
					(Member && !Member->DisplayName.IsEmpty()) ? Member->DisplayName.Left(1).ToUpper() : TEXT("+"),
					PartyAccent)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f).HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Member ? Member->DisplayName : FString(TEXT("Empty"))))
				.Font(FT66Style::MakeFont(TEXT("Regular"), 10))
				.ColorAndOpacity(Member ? BrightText : MutedText)
				.Justification(ETextJustify::Center)
			]
		];
	}

	const TSharedRef<SWidget> LeftShell =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(ShellFill)
		.Padding(FMargin(12.f, 12.f, 12.f, 10.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.03f, 0.04f, 0.06f, 0.92f))
						.Padding(FMargin(12.f))
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								MakeAvatarSlot(ProfileAvatarBrush, FVector2D(76.f, 76.f), ProfileName.Left(1).ToUpper(), OnlineAccent)
							]
							+ SHorizontalBox::Slot().FillWidth(1.f).Padding(12.f, 0.f, 0.f, 0.f)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(ProfileName)).Font(FT66Style::MakeFont(TEXT("Regular"), 12)).ColorAndOpacity(BrightText)]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)[SNew(STextBlock).Text(ProfileStatus).Font(FT66Style::MakeFont(TEXT("Regular"), 11)).ColorAndOpacity(MutedText)]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.MainMenu", "ProfileMeta", "Account, records, and rewards")).Font(FT66Style::MakeFont(TEXT("Regular"), 11)).ColorAndOpacity(MutedText)]
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)[MakeDivider()]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
					[
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
								.OnTextChanged_UObject(this, &UT66MiniMainMenuScreen::HandleFriendSearchTextChanged)
								.HintText(NSLOCTEXT("T66Mini.MainMenu", "SearchFriends", "Search friends..."))
								.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
								.ForegroundColor(BrightText)
								.BackgroundColor(FLinearColor(0.03f, 0.04f, 0.06f, 1.0f))
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)[FriendsList]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[MakeSectionTitle(NSLOCTEXT("T66Mini.MainMenu", "PartySection", "PARTY"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[PartySlots]
			]
		];

	auto MakeMenuButton = [&](const FText& Text, FReply (UT66MiniMainMenuScreen::*ClickFunc)(), const bool bGlow, const bool bEnabled) -> TSharedRef<SWidget>
	{
		FT66ButtonParams Params(Text, FOnClicked::CreateUObject(this, ClickFunc), ET66ButtonType::Success);
		Params.SetMinWidth(CenterButtonWidth).SetHeight(CenterButtonHeight).SetFontSize(24).SetPadding(FMargin(16.f, 10.f, 16.f, 8.f)).SetUseGlow(false).SetUseDotaPlateOverlay(true).SetDotaPlateOverrideBrush(PrimaryCTAFillBrush.Get()).SetTextColor(FLinearColor(0.96f, 0.96f, 0.94f, 1.0f)).SetStateTextShadowColors(FLinearColor(0.f, 0.f, 0.f, 0.36f), FLinearColor(0.f, 0.f, 0.f, 0.42f), FLinearColor(0.f, 0.f, 0.f, 0.28f)).SetTextShadowOffset(FVector2D(0.f, 1.f)).SetEnabled(bEnabled);
		TSharedRef<SWidget> Button = FT66Style::MakeButton(Params);
		if (!bGlow)
		{
			return SNew(SBox).WidthOverride(CenterButtonWidth)[Button];
		}

		return SNew(SBox).WidthOverride(CenterButtonWidth)
			[
				SNew(ST66AnimatedBorderGlow)
				.GlowColor(FLinearColor(0.32f, 0.83f, 0.56f, 1.0f))
				.SweepColor(FLinearColor(0.78f, 1.00f, 0.87f, 1.0f))
				.BorderInset(2.0f)
				.InnerThickness(2.0f)
				.MidThickness(5.0f)
				.OuterThickness(9.0f)
				.SweepLengthFraction(0.26f)
				.SweepSpeed(0.12f)
				[
					Button
				]
			];
	};

	bool bHasAnyMiniSave = false;
	if (SaveSubsystem)
	{
		for (const FT66MiniSaveSlotSummary& Summary : SaveSubsystem->BuildRunSlotSummaries(nullptr))
		{
			if (Summary.bOccupied)
			{
				bHasAnyMiniSave = true;
				break;
			}
		}
	}

	const TSharedRef<SWidget> CenterButtons =
		SNew(SBox)
		.WidthOverride(CenterWidth)
		.HeightOverride(CenterHeight)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.015f, 0.82f))
			.Padding(FMargin(18.f, 16.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[MakeMenuButton(NSLOCTEXT("T66Mini.MainMenu", "Start", "START"), &UT66MiniMainMenuScreen::HandleNewGameClicked, true, true)]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, CenterButtonGap, 0.f, 0.f)
				[
					MakeMenuButton(NSLOCTEXT("T66Mini.MainMenu", "Continue", "CONTINUE"), &UT66MiniMainMenuScreen::HandleLoadGameClicked, false, bHasAnyMiniSave)
				]
			]
		];

	FT66ButtonParams BackButtonParams(
		NSLOCTEXT("T66Mini.MainMenu", "BackToMainMenu", "BACK TO MAIN MENU"),
		FOnClicked::CreateUObject(this, &UT66MiniMainMenuScreen::HandleBackToMainMenuClicked),
		ET66ButtonType::Neutral);
	BackButtonParams
		.SetMinWidth(224.f)
		.SetHeight(40.f)
		.SetFontSize(14)
		.SetPadding(FMargin(14.f, 8.f, 14.f, 6.f))
		.SetUseGlow(false)
		.SetTextColor(FLinearColor(0.94f, 0.95f, 0.97f, 1.0f))
		.SetStateTextShadowColors(
			FLinearColor(0.f, 0.f, 0.f, 0.32f),
			FLinearColor(0.f, 0.f, 0.f, 0.38f),
			FLinearColor(0.f, 0.f, 0.f, 0.24f))
		.SetTextShadowOffset(FVector2D(0.f, 1.f));
	const TSharedRef<SWidget> BackToMainMenuButton =
		SNew(SBox)
		.WidthOverride(224.f)
		.HeightOverride(40.f)
		[
			FT66Style::MakeButton(BackButtonParams)
		];

	const TSharedRef<SWidget> LeaderboardShell =
		SAssignNew(LeaderboardPanel, ST66LeaderboardPanel)
		.LocalizationSubsystem(LocSubsystem)
		.LeaderboardSubsystem(LeaderboardSubsystem)
		.UIManager(UIManager);

	TArray<FT66AnimatedBackgroundLayer> BackgroundLayers;
	BackgroundLayers.Reserve(3);
	auto& SkyLayer = BackgroundLayers.AddDefaulted_GetRef();
	SkyLayer.Brush = SkyBackgroundBrush.Get();
	SkyLayer.SwayAmplitude = FVector2D(15.f, 5.f);
	SkyLayer.SwayFrequency = 0.15f;
	SkyLayer.OpacityMin = 1.f;
	SkyLayer.OpacityMax = 1.f;
	auto& FireLayer = BackgroundLayers.AddDefaulted_GetRef();
	FireLayer.Brush = FireMoonBrush.Get();
	FireLayer.BaseOffset = FVector2D(0.f, SceneOffset * 0.72f);
	FireLayer.SwayAmplitude = FVector2D(3.f, 2.f);
	FireLayer.SwayFrequency = 0.10f;
	FireLayer.ScalePulseAmplitude = 0.05f;
	FireLayer.ScalePulseFrequency = 0.30f;
	FireLayer.OpacityMin = 0.70f;
	FireLayer.OpacityMax = 1.00f;
	FireLayer.OpacityFrequency = 0.40f;
	FireLayer.PhaseOffset = 0.21f;
	auto& PyramidLayer = BackgroundLayers.AddDefaulted_GetRef();
	PyramidLayer.Brush = PyramidChadBrush.Get();
	PyramidLayer.BaseOffset = FVector2D(0.f, SceneOffset);
	PyramidLayer.SwayAmplitude = FVector2D(2.f, 1.f);
	PyramidLayer.SwayFrequency = 0.08f;
	PyramidLayer.OpacityMin = 1.f;
	PyramidLayer.OpacityMax = 1.f;
	PyramidLayer.PhaseOffset = 0.43f;

	auto MakeTitle = [](const FLinearColor& Color, const FVector2D& Offset) -> TSharedRef<SWidget>
	{
		FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Black"), 38);
		Font.LetterSpacing = 46;
		return SNew(STextBlock)
			.Text(NSLOCTEXT("T66Mini.MainMenu", "MiniTitle", "MINI CHADPOCALYPSE"))
			.Font(Font)
			.ColorAndOpacity(Color)
			.RenderTransform(FSlateRenderTransform(Offset))
			.ShadowOffset(FVector2D(2.f, 2.f))
			.ShadowColorAndOpacity(FLinearColor(1.0f, 0.72f, 0.18f, 0.42f));
	};

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()[SNew(ST66AnimatedBackground).Layers(BackgroundLayers)]
			+ SOverlay::Slot()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.36f))]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, TitleTopPadding, 0.f, 0.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()[MakeTitle(FLinearColor(0.98f, 0.68f, 0.18f, 0.30f), FVector2D(2.f, 2.f))]
				+ SOverlay::Slot()[MakeTitle(FLinearColor(0.02f, 0.01f, 0.01f, 0.98f), FVector2D::ZeroVector)]
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(FMargin(20.f, 20.f, 0.f, 0.f))
			[
				BackToMainMenuButton
			]
			+ SOverlay::Slot()
			[
				SNew(SOverlay)
				+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(FMargin(20.f, 0.f, 0.f, 16.f))
				[
					SNew(SBox).WidthOverride(SideWidth).HeightOverride(SideHeight).Clipping(EWidgetClipping::ClipToBounds)
					[
						SNew(SInvalidationPanel)[LeftShell]
					]
				]
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 124.f)
				[
					SNew(SBox).WidthOverride(CenterWidth).HeightOverride(CenterHeight)
					[
						SNew(SInvalidationPanel)[CenterButtons]
					]
				]
				+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(FMargin(0.f, 0.f, 20.f, 16.f))
				[
					SNew(SBox).WidthOverride(SideWidth).HeightOverride(SideHeight + 58.f).Clipping(EWidgetClipping::ClipToBounds)
					[
						SNew(SInvalidationPanel)[LeaderboardShell]
					]
				]
			]
		];
}

FReply UT66MiniMainMenuScreen::HandleBackToMainMenuClicked()
{
	NavigateTo(ET66ScreenType::MainMenu);
	return FReply::Handled();
}

FReply UT66MiniMainMenuScreen::HandleNewGameClicked()
{
	if (UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr)
	{
		FrontendState->BeginNewRun();
	}

	NavigateTo(ET66ScreenType::MiniCharacterSelect);
	return FReply::Handled();
}

FReply UT66MiniMainMenuScreen::HandleLoadGameClicked()
{
	NavigateTo(ET66ScreenType::MiniSaveSlots);
	return FReply::Handled();
}

void UT66MiniMainMenuScreen::HandleFriendSearchTextChanged(const FText& InText)
{
	FriendSearchQuery = InText.ToString();
	ForceRebuildSlate();
}

void UT66MiniMainMenuScreen::RequestMiniMenuTextures()
{
	SetupCookedBrush(SkyBackgroundBrush, SkyBackgroundTexture, TEXT("/Game/UI/MainMenu/sky_bg.sky_bg"), MiniBgSize, TEXT("MiniMainMenuSky"));
	SetupCookedBrush(FireMoonBrush, FireMoonTexture, TEXT("/Game/UI/MainMenu/fire_moon.fire_moon"), MiniBgSize, TEXT("MiniMainMenuFire"));
	SetupCookedBrush(PyramidChadBrush, PyramidChadTexture, TEXT("/Game/UI/MainMenu/pyramid_chad.pyramid_chad"), MiniBgSize, TEXT("MiniMainMenuPyramid"));
	SetupLooseBrush(PrimaryCTAFillBrush, PrimaryCTAFillTexture, TEXT("RuntimeDependencies/T66/UI/MainMenu/mainmenu_cta_fill_green.png"), FVector2D(1024.f, 232.f), false, TEXT("MiniMainMenuCTA"));
}

void UT66MiniMainMenuScreen::ReleaseRetainedSlateState()
{
	LeaderboardPanel.Reset();
	SkyBackgroundBrush.Reset();
	SkyBackgroundTexture.Reset();
	FireMoonBrush.Reset();
	FireMoonTexture.Reset();
	PyramidChadBrush.Reset();
	PyramidChadTexture.Reset();
	PrimaryCTAFillBrush.Reset();
	PrimaryCTAFillTexture.Reset();
	ProfileAvatarBrush.Reset();
	FriendAvatarBrushes.Reset();
	PartyAvatarBrushes.Reset();
}
