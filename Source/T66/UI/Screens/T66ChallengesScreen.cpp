// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66ChallengesScreen.h"
#include "Core/T66CommunityContentSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66WebImageCache.h"
#include "Data/T66DataTypes.h"
#include "Engine/Texture2D.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	enum class ETabIndex : int32
	{
		Challenges = 0,
		Mods = 1,
		Count,
	};

	enum class ESourceTabIndex : int32
	{
		Official = 0,
		Community = 1,
		Count,
	};

	enum class ET66ChallengeButtonFamily : uint8
	{
		CompactNeutral,
		ToggleOn,
		ToggleOff,
		ToggleInactive
	};

	enum class ET66ChallengeButtonState : uint8
	{
		Normal,
		Hovered,
		Pressed,
		Disabled
	};

	struct FT66ChallengeSpriteBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
	};

	struct FT66ChallengeButtonBrushSet
	{
		FT66ChallengeSpriteBrushEntry Normal;
		FT66ChallengeSpriteBrushEntry Hover;
		FT66ChallengeSpriteBrushEntry Pressed;
		FT66ChallengeSpriteBrushEntry Disabled;
	};

	UT66GameInstance* GetT66GameInstance(const UObject* Context)
	{
		return Context ? Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(Context)) : nullptr;
	}

	FLinearColor ChallengeShellFill()
	{
		return FLinearColor(0.055f, 0.027f, 0.012f, 1.0f);
	}

	FLinearColor ChallengePanelFill()
	{
		return FLinearColor(0.095f, 0.047f, 0.020f, 1.0f);
	}

	FLinearColor ChallengePanelInsetFill()
	{
		return FLinearColor(0.72f, 0.50f, 0.23f, 1.0f);
	}

	FLinearColor ChallengeHeaderFill()
	{
		return FLinearColor(0.18f, 0.34f, 0.28f, 1.0f);
	}

	FLinearColor ChallengeSelectedFill()
	{
		return FLinearColor(0.16f, 0.27f, 0.22f, 1.0f);
	}

	FLinearColor ChallengeRewardTint()
	{
		return FLinearColor(0.36f, 0.20f, 0.07f, 1.0f);
	}

	FLinearColor ChallengeDangerTint()
	{
		return FLinearColor(0.89f, 0.29f, 0.25f, 1.0f);
	}

	FLinearColor ChallengeSuccessTint()
	{
		return FLinearColor(0.86f, 0.67f, 0.34f, 1.0f);
	}

	FLinearColor ChallengeMutedBadgeTint()
	{
		return FLinearColor(0.37f, 0.22f, 0.09f, 1.0f);
	}

	const FLinearColor ChallengeFantasyText(0.953f, 0.925f, 0.835f, 1.0f);
	const FLinearColor ChallengeFantasyMuted(0.738f, 0.708f, 0.648f, 1.0f);
	const FLinearColor ChallengePaperText(0.19f, 0.115f, 0.040f, 1.0f);
	const FLinearColor ChallengePaperMuted(0.35f, 0.235f, 0.100f, 1.0f);
	const FLinearColor ChallengeGoldText(0.92f, 0.74f, 0.42f, 1.0f);

	const FSlateBrush* ResolveChallengeSpriteBrush(
		FT66ChallengeSpriteBrushEntry& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const ESlateBrushDrawType::Type DrawAs,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = DrawAs;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
		}

		if (!Entry.Texture.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					Filter,
					true,
					TEXT("ChallengesReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
	}

	const FSlateBrush* ResolveChallengeSpriteRegionBrush(
		FT66ChallengeSpriteBrushEntry& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const FBox2f& UVRegion,
		const ESlateBrushDrawType::Type DrawAs,
		const FLinearColor& Tint,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = DrawAs;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(Tint);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
			Entry.Brush->SetUVRegion(UVRegion);
		}

		if (!Entry.Texture.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					Filter,
					true,
					TEXT("ChallengesReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
	}

	const FSlateBrush* GetChallengeContentShellBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/Challenges/Panels/challenges_panels_fullscreen_fullscreen_panel_wide.png"),
			FVector2D(1588.f, 653.f),
			FMargin(0.060f, 0.090f, 0.060f, 0.105f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeRowShellBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/Challenges/Panels/challenges_panels_fullscreen_row_shell_quiet.png"),
			FVector2D(1632.f, 209.f),
			FMargin(0.070f, 0.155f, 0.070f, 0.155f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeBrowserRowBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/Challenges/Panels/challenges_panels_challenge_row_plate_v1.png"),
			FVector2D(954.f, 211.f),
			FMargin(0.075f, 0.220f, 0.075f, 0.220f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeDetailPaperBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/Challenges/Panels/challenges_panels_challenge_detail_paper_v1.png"),
			FVector2D(972.f, 419.f),
			FMargin(0.060f, 0.130f, 0.060f, 0.130f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeRulesPaperBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/Challenges/Panels/challenges_panels_challenge_rules_paper_v1.png"),
			FVector2D(377.f, 672.f),
			FMargin(0.135f, 0.075f, 0.135f, 0.075f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeStatusBarBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/Challenges/Panels/challenges_panels_challenge_status_bar_v1.png"),
			FVector2D(845.f, 115.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeTagPillBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/Challenges/Controls/challenges_controls_tag_pill_v1.png"),
			FVector2D(303.f, 99.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeStateSocketBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/Challenges/Controls/challenges_controls_state_socket_v1.png"),
			FVector2D(123.f, 116.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetChallengeHeaderDividerBrush()
	{
		static FT66ChallengeSpriteBrushEntry Entry;
		return ResolveChallengeSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/Challenges/Controls/challenges_controls_header_divider_v1.png"),
			FVector2D(858.f, 60.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	const FScrollBarStyle* GetChallengeReferenceScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static FT66ChallengeSpriteBrushEntry TrackEntry;
		static FT66ChallengeSpriteBrushEntry ThumbEntry;
		static FT66ChallengeSpriteBrushEntry HoverEntry;

		const FString ControlsPath = TEXT("SourceAssets/UI/Reference/Screens/Challenges/Controls/challenges_controls_controls_sheet.png");
		const FBox2f VerticalBarUV(
			FVector2f(4.f / 1350.f, 4.f / 926.f),
			FVector2f(90.f / 1350.f, 644.f / 926.f));

		const FSlateBrush* TrackBrush = ResolveChallengeSpriteRegionBrush(
			TrackEntry,
			ControlsPath,
			FVector2D(14.f, 120.f),
			FMargin(0.42f, 0.085f, 0.42f, 0.085f),
			VerticalBarUV,
			ESlateBrushDrawType::Box,
			FLinearColor(0.35f, 0.34f, 0.30f, 0.70f),
			TextureFilter::TF_Nearest);
		const FSlateBrush* ThumbBrush = ResolveChallengeSpriteRegionBrush(
			ThumbEntry,
			ControlsPath,
			FVector2D(16.f, 96.f),
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			VerticalBarUV,
			ESlateBrushDrawType::Box,
			FLinearColor(0.93f, 0.82f, 0.52f, 1.0f),
			TextureFilter::TF_Nearest);
		const FSlateBrush* HoverBrush = ResolveChallengeSpriteRegionBrush(
			HoverEntry,
			ControlsPath,
			FVector2D(16.f, 96.f),
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			VerticalBarUV,
			ESlateBrushDrawType::Box,
			FLinearColor(1.0f, 0.90f, 0.62f, 1.0f),
			TextureFilter::TF_Nearest);

		if (TrackBrush && ThumbBrush && HoverBrush)
		{
			Style
				.SetVerticalBackgroundImage(*TrackBrush)
				.SetVerticalTopSlotImage(*TrackBrush)
				.SetVerticalBottomSlotImage(*TrackBrush)
				.SetNormalThumbImage(*ThumbBrush)
				.SetHoveredThumbImage(*HoverBrush)
				.SetDraggedThumbImage(*HoverBrush)
				.SetThickness(14.f);
		}

		return &Style;
	}

	FString GetChallengeButtonPath(const ET66ChallengeButtonFamily Family, const ET66ChallengeButtonState State)
	{
		const TCHAR* Suffix = TEXT("normal");
		if (Family == ET66ChallengeButtonFamily::ToggleOn && State == ET66ChallengeButtonState::Normal)
		{
			Suffix = TEXT("selected");
		}
		else if (State == ET66ChallengeButtonState::Hovered)
		{
			Suffix = TEXT("hover");
		}
		else if (State == ET66ChallengeButtonState::Pressed)
		{
			Suffix = TEXT("pressed");
		}
		else if (State == ET66ChallengeButtonState::Disabled)
		{
			Suffix = TEXT("disabled");
		}

		return FString::Printf(
			TEXT("SourceAssets/UI/Reference/Screens/Challenges/Buttons/challenges_buttons_pill_%s.png"),
			Suffix);
	}

	FVector2D GetChallengeButtonSize(const ET66ChallengeButtonFamily Family, const ET66ChallengeButtonState State)
	{
		if (Family == ET66ChallengeButtonFamily::ToggleOn)
		{
			return State == ET66ChallengeButtonState::Pressed ? FVector2D(187.f, 67.f) : FVector2D(180.f, 68.f);
		}
		if (Family == ET66ChallengeButtonFamily::ToggleOff)
		{
			return State == ET66ChallengeButtonState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
		}
		if (Family == ET66ChallengeButtonFamily::ToggleInactive)
		{
			return State == ET66ChallengeButtonState::Hovered ? FVector2D(186.f, 69.f) : FVector2D(180.f, 68.f);
		}
		return State == ET66ChallengeButtonState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
	}

	FT66ChallengeButtonBrushSet& GetChallengeButtonBrushSet(const ET66ChallengeButtonFamily Family)
	{
		static FT66ChallengeButtonBrushSet CompactNeutral;
		static FT66ChallengeButtonBrushSet ToggleOn;
		static FT66ChallengeButtonBrushSet ToggleOff;
		static FT66ChallengeButtonBrushSet ToggleInactive;

		if (Family == ET66ChallengeButtonFamily::ToggleOn)
		{
			return ToggleOn;
		}
		if (Family == ET66ChallengeButtonFamily::ToggleOff)
		{
			return ToggleOff;
		}
		if (Family == ET66ChallengeButtonFamily::ToggleInactive)
		{
			return ToggleInactive;
		}
		return CompactNeutral;
	}

	const FSlateBrush* GetChallengeButtonBrush(const ET66ChallengeButtonFamily Family, const ET66ChallengeButtonState State)
	{
		FT66ChallengeButtonBrushSet& Set = GetChallengeButtonBrushSet(Family);
		FT66ChallengeSpriteBrushEntry* Entry = &Set.Normal;
		if (State == ET66ChallengeButtonState::Hovered)
		{
			Entry = &Set.Hover;
		}
		else if (State == ET66ChallengeButtonState::Pressed)
		{
			Entry = &Set.Pressed;
		}
		else if (State == ET66ChallengeButtonState::Disabled)
		{
			Entry = &Set.Disabled;
		}

		return ResolveChallengeSpriteBrush(
			*Entry,
			GetChallengeButtonPath(Family, State),
			GetChallengeButtonSize(Family, State),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	TSharedRef<SWidget> MakeChallengeSpritePanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const FMargin& Padding,
		const FLinearColor& FallbackColor)
	{
		return SNew(SBorder)
			.BorderImage(Brush ? Brush : FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(Brush ? FLinearColor::White : FallbackColor)
			.Padding(Padding)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakeChallengeHorizontalSlicedPanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const float Height,
		const FMargin& Padding,
		const FLinearColor& FallbackColor,
		const float SourceCapFraction = 0.105f)
	{
		if (!Brush)
		{
			return SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FallbackColor)
				.Padding(Padding)
				[
					Content
				];
		}

		return SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				T66ScreenSlateHelpers::MakeReferenceHorizontalSlicedImage(
					Brush,
					FVector2D(1.f, Height),
					SourceCapFraction)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(Padding)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakeChallengeTagPill(const FText& Label)
	{
		return SNew(SBox)
			.WidthOverride(92.f)
			.HeightOverride(28.f)
			[
				MakeChallengeHorizontalSlicedPanel(
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66Style::Tokens::FontBold(10))
					.ColorAndOpacity(ChallengeGoldText)
					.Justification(ETextJustify::Center)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds),
					GetChallengeTagPillBrush(),
					28.f,
					FMargin(8.f, 4.f, 8.f, 3.f),
					ChallengeMutedBadgeTint(),
					0.180f)
			];
	}

	TSharedRef<SWidget> MakeChallengeSpriteButtonContent(
		const TSharedRef<SWidget>& Content,
		const FOnClicked& OnClicked,
		const ET66ChallengeButtonFamily Family,
		const float MinWidth,
		const float Height,
		const FMargin& ContentPadding)
	{
		const FSlateBrush* NormalBrush = GetChallengeButtonBrush(Family, ET66ChallengeButtonState::Normal);
		const FSlateBrush* HoverBrush = GetChallengeButtonBrush(Family, ET66ChallengeButtonState::Hovered);
		const FSlateBrush* PressedBrush = GetChallengeButtonBrush(Family, ET66ChallengeButtonState::Pressed);
		const FSlateBrush* DisabledBrush = GetChallengeButtonBrush(Family, ET66ChallengeButtonState::Disabled);
		if (!NormalBrush)
		{
			return FT66Style::MakeButton(
				FT66ButtonParams(FText::GetEmpty(), OnClicked, Family == ET66ChallengeButtonFamily::ToggleOn ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
				.SetMinWidth(MinWidth)
				.SetHeight(Height)
				.SetPadding(ContentPadding)
				.SetContent(Content));
		}

		return T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton(
			OnClicked,
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				Content
			],
			NormalBrush,
			HoverBrush,
			PressedBrush,
			DisabledBrush,
			MinWidth,
			Height,
			ContentPadding);
	}

	TSharedRef<SWidget> MakeChallengeSpriteButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66ChallengeButtonFamily Family,
		const float MinWidth,
		const float Height,
		const int32 FontSize,
		const FMargin& ContentPadding = FMargin(12.f, 7.f, 12.f, 6.f))
	{
		return MakeChallengeSpriteButtonContent(
			SNew(STextBlock)
			.Text(Label)
			.Font(FT66Style::Tokens::FontBold(FontSize))
			.ColorAndOpacity(Family == ET66ChallengeButtonFamily::ToggleInactive ? ChallengeFantasyMuted : ChallengeFantasyText)
			.Justification(ETextJustify::Center)
			.AutoWrapText(false),
			OnClicked,
			Family,
			MinWidth,
			Height,
			ContentPadding);
	}

	ET66CommunityContentKind TabIndexToKind(const int32 TabIndex)
	{
		return TabIndex == static_cast<int32>(ETabIndex::Mods)
			? ET66CommunityContentKind::Mod
			: ET66CommunityContentKind::Challenge;
	}
}

UT66ChallengesScreen::UT66ChallengesScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Challenges;
	bIsModal = false;
}

void UT66ChallengesScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		if (!bCommunityDelegateBound)
		{
			Community->OnContentChanged().AddUObject(this, &UT66ChallengesScreen::HandleCommunityContentChanged);
			bCommunityDelegateBound = true;
		}
	}
}

void UT66ChallengesScreen::OnScreenDeactivated_Implementation()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		Community->OnContentChanged().RemoveAll(this);
	}

	bCommunityDelegateBound = false;
	Super::OnScreenDeactivated_Implementation();
}

UT66CommunityContentSubsystem* UT66ChallengesScreen::GetCommunitySubsystem() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UT66CommunityContentSubsystem>();
	}

	return nullptr;
}

ET66CommunityContentKind UT66ChallengesScreen::GetActiveKind() const
{
	return TabIndexToKind(ActiveTabIndex);
}

TArray<FT66CommunityContentEntry> UT66ChallengesScreen::GetEntriesForView(const int32 TabIndex, const int32 SourceTabIndex) const
{
	const UT66CommunityContentSubsystem* Community = GetCommunitySubsystem();
	if (!Community)
	{
		return {};
	}

	const ET66CommunityContentKind Kind = TabIndexToKind(TabIndex);
	return SourceTabIndex == static_cast<int32>(ESourceTabIndex::Official)
		? TArray<FT66CommunityContentEntry>(Community->GetOfficialEntries(Kind))
		: Community->GetCommunityBrowserEntries(Kind);
}

bool UT66ChallengesScreen::FindSelectedEntryForView(const int32 TabIndex, const int32 SourceTabIndex, FT66CommunityContentEntry& OutEntry)
{
	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(TabIndex, SourceTabIndex);
	if (Entries.Num() <= 0)
	{
		return false;
	}

	const FName SelectedId = GetSelectedEntryIdForView(TabIndex, SourceTabIndex);
	const FT66CommunityContentEntry* Found = Entries.FindByPredicate([SelectedId](const FT66CommunityContentEntry& Entry)
	{
		return Entry.LocalId == SelectedId;
	});

	OutEntry = Found ? *Found : Entries[0];
	return true;
}

bool UT66ChallengesScreen::FindCurrentSelectedEntry(FT66CommunityContentEntry& OutEntry)
{
	return FindSelectedEntryForView(
		ActiveTabIndex,
		ActiveSourceTabIndex[ActiveTabIndex],
		OutEntry);
}

bool UT66ChallengesScreen::FindConfirmedEntry(FT66CommunityContentEntry& OutEntry) const
{
	if (const UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		return Community->GetActiveEntry(OutEntry);
	}

	return false;
}

FName UT66ChallengesScreen::GetSelectedEntryIdForView(const int32 TabIndex, const int32 SourceTabIndex)
{
	InitializeSelectionState();

	const int32 SafeTabIndex = FMath::Clamp(TabIndex, 0, static_cast<int32>(ETabIndex::Count) - 1);
	const int32 SafeSourceIndex = FMath::Clamp(SourceTabIndex, 0, static_cast<int32>(ESourceTabIndex::Count) - 1);
	FName& SelectedId = PendingSelections[SafeTabIndex][SafeSourceIndex];
	if (!SelectedId.IsNone())
	{
		return SelectedId;
	}

	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(SafeTabIndex, SafeSourceIndex);
	if (Entries.Num() > 0)
	{
		SelectedId = Entries[0].LocalId;
	}

	return SelectedId;
}

FString UT66ChallengesScreen::GetOriginLabel(const FT66CommunityContentEntry& Entry) const
{
	switch (Entry.Origin)
	{
	case ET66CommunityContentOrigin::Draft:
		return TEXT("Draft");
	case ET66CommunityContentOrigin::Community:
		return TEXT("Community");
	case ET66CommunityContentOrigin::Official:
	default:
		return TEXT("Official");
	}
}

FString UT66ChallengesScreen::GetDraftSubmissionLabel(const FT66CommunityContentEntry& Entry) const
{
	if (!Entry.SubmissionStatus.IsEmpty())
	{
		return Entry.SubmissionStatus;
	}

	if (!Entry.ReviewNote.IsEmpty())
	{
		return Entry.ReviewNote;
	}

	if (!Entry.ModerationStatus.IsEmpty())
	{
		return Entry.ModerationStatus;
	}

	return TEXT("Not submitted");
}

FString UT66ChallengesScreen::GetPassiveLabel(const ET66PassiveType PassiveType) const
{
	if (const UEnum* Enum = StaticEnum<ET66PassiveType>())
	{
		return Enum->GetDisplayNameTextByValue(static_cast<int64>(PassiveType)).ToString();
	}

	return TEXT("None");
}

FString UT66ChallengesScreen::GetUltimateLabel(const ET66UltimateType UltimateType) const
{
	if (const UEnum* Enum = StaticEnum<ET66UltimateType>())
	{
		return Enum->GetDisplayNameTextByValue(static_cast<int64>(UltimateType)).ToString();
	}

	return TEXT("None");
}

FString UT66ChallengesScreen::GetItemLabel(const FName ItemId) const
{
	return ItemId.IsNone() ? TEXT("None") : ItemId.ToString();
}

TArray<FName> UT66ChallengesScreen::GetSelectableItemIds() const
{
	TArray<FName> Result;
	Result.Add(NAME_None);

	if (UT66GameInstance* T66GI = GetT66GameInstance(this))
	{
		if (UDataTable* ItemsTable = T66GI->GetItemsDataTable())
		{
			TArray<FName> RowNames = ItemsTable->GetRowNames();
			RowNames.Sort([](const FName& A, const FName& B)
			{
				return A.LexicalLess(B);
			});
			Result.Append(RowNames);
		}
	}

	return Result;
}

const FSlateBrush* UT66ChallengesScreen::GetOrCreateAvatarBrush(const FString& AvatarUrl)
{
	if (!DefaultAvatarBrush.IsValid())
	{
		DefaultAvatarBrush = MakeShared<FSlateBrush>();
		DefaultAvatarBrush->DrawAs = ESlateBrushDrawType::Image;
		DefaultAvatarBrush->ImageSize = FVector2D(52.0f, 52.0f);
		DefaultAvatarBrush->TintColor = FSlateColor(FLinearColor(0.14f, 0.15f, 0.17f, 0.32f));
	}

	if (AvatarUrl.IsEmpty())
	{
		return DefaultAvatarBrush.Get();
	}

	if (TSharedPtr<FSlateBrush>* Found = AvatarBrushes.Find(AvatarUrl))
	{
		return Found->Get();
	}

	const UGameInstance* GI = GetGameInstance();
	UT66WebImageCache* ImageCache = GI ? GI->GetSubsystem<UT66WebImageCache>() : nullptr;
	if (!ImageCache)
	{
		return DefaultAvatarBrush.Get();
	}

	if (UTexture2D* CachedTexture = ImageCache->GetCachedImage(AvatarUrl))
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->SetResourceObject(CachedTexture);
		Brush->ImageSize = FVector2D(52.0f, 52.0f);
		AvatarBrushes.Add(AvatarUrl, Brush);
		return Brush.Get();
	}

	TWeakObjectPtr<UT66ChallengesScreen> WeakScreen(this);
	ImageCache->RequestImage(AvatarUrl, [WeakScreen](UTexture2D* Texture)
	{
		if (Texture)
		{
			if (UT66ChallengesScreen* Screen = WeakScreen.Get())
			{
				Screen->ForceRebuildSlate();
			}
		}
	});

	return DefaultAvatarBrush.Get();
}

void UT66ChallengesScreen::InitializeSelectionState()
{
	if (bSelectionStateInitialized)
	{
		return;
	}

	bSelectionStateInitialized = true;
	ActiveTabIndex = static_cast<int32>(ETabIndex::Challenges);
	ActiveSourceTabIndex[static_cast<int32>(ETabIndex::Challenges)] = static_cast<int32>(ESourceTabIndex::Official);
	ActiveSourceTabIndex[static_cast<int32>(ETabIndex::Mods)] = static_cast<int32>(ESourceTabIndex::Official);

	for (int32 TabIndex = 0; TabIndex < static_cast<int32>(ETabIndex::Count); ++TabIndex)
	{
		for (int32 SourceTabIndex = 0; SourceTabIndex < static_cast<int32>(ESourceTabIndex::Count); ++SourceTabIndex)
		{
			const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(TabIndex, SourceTabIndex);
			PendingSelections[TabIndex][SourceTabIndex] = Entries.Num() > 0 ? Entries[0].LocalId : NAME_None;
		}
	}

	if (const UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		FT66CommunityContentEntry ActiveEntry;
		if (Community->GetActiveEntry(ActiveEntry))
		{
			ActiveTabIndex = ActiveEntry.Kind == ET66CommunityContentKind::Mod
				? static_cast<int32>(ETabIndex::Mods)
				: static_cast<int32>(ETabIndex::Challenges);
			ActiveSourceTabIndex[ActiveTabIndex] = ActiveEntry.Origin == ET66CommunityContentOrigin::Official
				? static_cast<int32>(ESourceTabIndex::Official)
				: static_cast<int32>(ESourceTabIndex::Community);
			PendingSelections[ActiveTabIndex][ActiveSourceTabIndex[ActiveTabIndex]] = ActiveEntry.LocalId;
			return;
		}
	}

	if (const UT66GameInstance* T66GI = GetT66GameInstance(this))
	{
		if (T66GI->SelectedRunModifierKind == ET66RunModifierKind::Mod)
		{
			ActiveTabIndex = static_cast<int32>(ETabIndex::Mods);
		}
	}
}

void UT66ChallengesScreen::BeginDraftEditor(const FT66CommunityContentEntry& DraftEntry)
{
	bDraftEditorActive = true;
	DraftEditorEntry = DraftEntry;
	ActiveTabIndex = DraftEntry.Kind == ET66CommunityContentKind::Mod
		? static_cast<int32>(ETabIndex::Mods)
		: static_cast<int32>(ETabIndex::Challenges);
	ActiveSourceTabIndex[ActiveTabIndex] = static_cast<int32>(ESourceTabIndex::Community);
	PendingSelections[ActiveTabIndex][ActiveSourceTabIndex[ActiveTabIndex]] = DraftEntry.LocalId;
}

void UT66ChallengesScreen::EndDraftEditor()
{
	bDraftEditorActive = false;
	DraftEditorEntry = FT66CommunityContentEntry{};
}

void UT66ChallengesScreen::CycleDraftPassive(const int32 Direction)
{
	const UEnum* Enum = StaticEnum<ET66PassiveType>();
	if (!Enum)
	{
		return;
	}

	TArray<ET66PassiveType> Values;
	for (int32 EnumIndex = 0; EnumIndex < Enum->NumEnums() - 1; ++EnumIndex)
	{
		Values.Add(static_cast<ET66PassiveType>(Enum->GetValueByIndex(EnumIndex)));
	}

	const int32 CurrentIndex = Values.IndexOfByKey(DraftEditorEntry.Rules.PassiveOverride);
	const int32 NextIndex = Values.IsValidIndex(CurrentIndex)
		? (CurrentIndex + Direction + Values.Num()) % Values.Num()
		: 0;
	DraftEditorEntry.Rules.PassiveOverride = Values.IsValidIndex(NextIndex) ? Values[NextIndex] : ET66PassiveType::None;
}

void UT66ChallengesScreen::CycleDraftUltimate(const int32 Direction)
{
	const UEnum* Enum = StaticEnum<ET66UltimateType>();
	if (!Enum)
	{
		return;
	}

	TArray<ET66UltimateType> Values;
	for (int32 EnumIndex = 0; EnumIndex < Enum->NumEnums() - 1; ++EnumIndex)
	{
		Values.Add(static_cast<ET66UltimateType>(Enum->GetValueByIndex(EnumIndex)));
	}

	const int32 CurrentIndex = Values.IndexOfByKey(DraftEditorEntry.Rules.UltimateOverride);
	const int32 NextIndex = Values.IsValidIndex(CurrentIndex)
		? (CurrentIndex + Direction + Values.Num()) % Values.Num()
		: 0;
	DraftEditorEntry.Rules.UltimateOverride = Values.IsValidIndex(NextIndex) ? Values[NextIndex] : ET66UltimateType::None;
}

void UT66ChallengesScreen::CycleDraftStartingItem(const int32 Direction)
{
	const TArray<FName> ItemIds = GetSelectableItemIds();
	if (ItemIds.Num() <= 0)
	{
		DraftEditorEntry.Rules.StartingItemId = NAME_None;
		return;
	}

	const int32 CurrentIndex = ItemIds.IndexOfByKey(DraftEditorEntry.Rules.StartingItemId);
	const int32 SafeCurrentIndex = CurrentIndex != INDEX_NONE ? CurrentIndex : 0;
	const int32 NextIndex = (SafeCurrentIndex + Direction + ItemIds.Num()) % ItemIds.Num();
	DraftEditorEntry.Rules.StartingItemId = ItemIds[NextIndex];
}

void UT66ChallengesScreen::AdjustDraftStat(const EDraftStatField Field, const int32 Delta)
{
	auto Clamp = [](int32 Value)
	{
		return T66CommunityContentLimits::ClampStatBonus(Value);
	};

	switch (Field)
	{
	case EDraftStatField::Damage:
		DraftEditorEntry.Rules.BonusStats.Damage = Clamp(DraftEditorEntry.Rules.BonusStats.Damage + Delta);
		break;
	case EDraftStatField::AttackSpeed:
		DraftEditorEntry.Rules.BonusStats.AttackSpeed = Clamp(DraftEditorEntry.Rules.BonusStats.AttackSpeed + Delta);
		break;
	case EDraftStatField::AttackScale:
		DraftEditorEntry.Rules.BonusStats.AttackScale = Clamp(DraftEditorEntry.Rules.BonusStats.AttackScale + Delta);
		break;
	case EDraftStatField::Accuracy:
		DraftEditorEntry.Rules.BonusStats.Accuracy = Clamp(DraftEditorEntry.Rules.BonusStats.Accuracy + Delta);
		break;
	case EDraftStatField::Armor:
		DraftEditorEntry.Rules.BonusStats.Armor = Clamp(DraftEditorEntry.Rules.BonusStats.Armor + Delta);
		break;
	case EDraftStatField::Evasion:
		DraftEditorEntry.Rules.BonusStats.Evasion = Clamp(DraftEditorEntry.Rules.BonusStats.Evasion + Delta);
		break;
	case EDraftStatField::Luck:
		DraftEditorEntry.Rules.BonusStats.Luck = Clamp(DraftEditorEntry.Rules.BonusStats.Luck + Delta);
		break;
	case EDraftStatField::Speed:
		DraftEditorEntry.Rules.BonusStats.Speed = Clamp(DraftEditorEntry.Rules.BonusStats.Speed + Delta);
		break;
	default:
		break;
	}
}

TSharedRef<SWidget> UT66ChallengesScreen::BuildSlateUI()
{
	InitializeSelectionState();

	UT66CommunityContentSubsystem* Community = GetCommunitySubsystem();
	if (Community && !bRequestedCommunityRefresh)
	{
		bRequestedCommunityRefresh = true;
		Community->RefreshCommunityCatalog(false);
		Community->RefreshMySubmissionStates(false);
	}

	const int32 SafeFrameHeight = static_cast<int32>(FT66Style::GetSafeFrameSize().Y);
	const float TopBarInset = UIManager ? UIManager->GetFrontendTopBarReservedHeight() : 0.0f;
	const float ListPanelWidth = 960.0f;
	const float DetailPanelWidth = 850.0f;
	const float BodyPanelHeight = FMath::Clamp(static_cast<float>(SafeFrameHeight) - TopBarInset - 245.0f, 620.0f, 675.0f);
	const float DetailColumnWidth = DetailPanelWidth - 74.0f;
	const float ListColumnWidth = ListPanelWidth - 64.0f;
	const int32 HeaderTabFontSize = 18;
	const int32 SourceTabFontSize = 20;
	const int32 ActionButtonFontSize = 19;
	const int32 CurrentSourceTabIndex = ActiveSourceTabIndex[ActiveTabIndex];
	const ET66CommunityContentKind ActiveKind = GetActiveKind();
	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(ActiveTabIndex, CurrentSourceTabIndex);

	FT66CommunityContentEntry SelectedEntry;
	const bool bHasSelectedEntry = !bDraftEditorActive && FindCurrentSelectedEntry(SelectedEntry);

	FT66CommunityContentEntry ConfirmedEntry;
	const bool bHasConfirmedEntry = FindConfirmedEntry(ConfirmedEntry);
	const bool bSelectedEntryConfirmed = bHasSelectedEntry && bHasConfirmedEntry && SelectedEntry.LocalId == ConfirmedEntry.LocalId;

	const FString HeaderTitle = ActiveKind == ET66CommunityContentKind::Mod ? TEXT("Mods") : TEXT("Challenges");
	const FString DetailListHeader = ActiveKind == ET66CommunityContentKind::Mod ? TEXT("Rules") : TEXT("Rules And Requirements");
	const FString StatusMessage = Community ? Community->GetLastStatusMessage() : FString();

	auto MakeConstraintRow = [](const FString& ConstraintText) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 1.f, 8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Challenges", "ConstraintBullet", "\u25C6"))
				.Font(FT66Style::Tokens::FontBold(15))
				.ColorAndOpacity(ChallengePaperMuted)
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(ConstraintText))
				.Font(FT66Style::Tokens::FontBold(18))
				.ColorAndOpacity(ChallengePaperText)
				.AutoWrapText(true)
			];
	};

	auto MakeTopTabButton = [this, HeaderTabFontSize](const int32 TabIndex, const FText& Label) -> TSharedRef<SWidget>
	{
		const bool bActive = ActiveTabIndex == TabIndex;
		const float ButtonWidth = TabIndex == static_cast<int32>(ETabIndex::Challenges) ? 246.f : 178.f;
		return MakeChallengeSpriteButton(
			Label,
			FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleTabSelected, TabIndex),
			bActive ? ET66ChallengeButtonFamily::ToggleOn : ET66ChallengeButtonFamily::CompactNeutral,
			ButtonWidth,
			52.f,
			HeaderTabFontSize,
			FMargin(16.f, 7.f));
	};

	auto MakeSourceTabButton = [this, CurrentSourceTabIndex, SourceTabFontSize](const int32 SourceTabIndex, const FText& Label) -> TSharedRef<SWidget>
	{
		const bool bActive = CurrentSourceTabIndex == SourceTabIndex;
		return MakeChallengeSpriteButton(
			Label,
			FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSourceTabSelected, SourceTabIndex),
			bActive ? ET66ChallengeButtonFamily::ToggleOn : ET66ChallengeButtonFamily::CompactNeutral,
			330.f,
			58.f,
			SourceTabFontSize,
			FMargin(16.f, 8.f));
	};

	auto MakeEntryRow = [this, CurrentSourceTabIndex, Community](const FT66CommunityContentEntry& Entry, const int32 EntryIndex) -> TSharedRef<SWidget>
	{
		const FName SelectedId = GetSelectedEntryIdForView(ActiveTabIndex, CurrentSourceTabIndex);
		const bool bSelected = Entry.LocalId == SelectedId;
		const FT66CommunityContentEntry ActiveEntry = [this]()
		{
			FT66CommunityContentEntry Result;
			FindConfirmedEntry(Result);
			return Result;
		}();
		const bool bConfirmed = !ActiveEntry.LocalId.IsNone() && ActiveEntry.LocalId == Entry.LocalId;

		const TSharedRef<SWidget> StateSocket =
			SNew(SBox)
			.WidthOverride(52.f)
			.HeightOverride(52.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image(GetChallengeStateSocketBrush())
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(bConfirmed ? NSLOCTEXT("T66.Challenges", "ConfirmedMarker", "X") : FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(14))
					.ColorAndOpacity(bConfirmed ? ChallengeGoldText : FLinearColor::Transparent)
					.Justification(ETextJustify::Center)
				]
			];

		const TSharedRef<SWidget> RowContent =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 18.f, 0.f)
			[
				StateSocket
			]
			+ SHorizontalBox::Slot().FillWidth(0.44f).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(Entry.Title))
					.Font(FT66Style::Tokens::FontBold(25))
					.ColorAndOpacity(bSelected ? ChallengePaperText : ChallengePaperMuted)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
				[
					MakeChallengeTagPill(FText::FromString(GetOriginLabel(Entry).ToUpper()))
				]
			]
			+ SHorizontalBox::Slot().FillWidth(0.26f).VAlign(VAlign_Center).Padding(20.f, 0.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Community ? Community->BuildRewardSummary(Entry) : TEXT("No reward data")))
				.Font(FT66Style::Tokens::FontBold(17))
				.ColorAndOpacity(ChallengeRewardTint())
				.AutoWrapText(true)
			]
			+ SHorizontalBox::Slot().FillWidth(0.24f).VAlign(VAlign_Center).Padding(20.f, 0.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Entry.IsDraft() ? GetDraftSubmissionLabel(Entry) : Entry.AuthorDisplayName))
				.Font(FT66Style::Tokens::FontBold(16))
				.ColorAndOpacity(ChallengePaperMuted)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds)
			];

		return FT66Style::MakeBareButton(
			FT66BareButtonParams(
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleEntrySelected, EntryIndex),
				SNew(SBox)
				.HeightOverride(112.f)
				[
					MakeChallengeSpritePanel(
						RowContent,
						GetChallengeBrowserRowBrush(),
						FMargin(28.f, 18.f, 28.f, 16.f),
						ChallengePanelInsetFill())
				])
				.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.SetPadding(FMargin(0.f))
				.SetHAlign(HAlign_Fill)
				.SetVAlign(VAlign_Fill)
				.SetHeight(112.f));
	};

	auto MakeDraftStepRow = [this](const FString& Label, const int32 Value, const FOnClicked& OnMinus, const FOnClicked& OnPlus) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Font(FT66Style::Tokens::FontBold(12))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeChallengeSpriteButton(FText::FromString(TEXT("-")), OnMinus, ET66ChallengeButtonFamily::CompactNeutral, 28.f, 24.f, 12, FMargin(0.f))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(Value))
				.Font(FT66Style::Tokens::FontBold(12))
				.ColorAndOpacity(ChallengeRewardTint())
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeChallengeSpriteButton(FText::FromString(TEXT("+")), OnPlus, ET66ChallengeButtonFamily::CompactNeutral, 28.f, 24.f, 12, FMargin(0.f))
			];
	};

	auto MakeCycleRow = [this](const FString& Label, const FString& Value, const FOnClicked& OnPrev, const FOnClicked& OnNext) -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Font(FT66Style::Tokens::FontBold(12))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeChallengeSpriteButton(FText::FromString(TEXT("<")), OnPrev, ET66ChallengeButtonFamily::CompactNeutral, 28.f, 24.f, 12, FMargin(0.f))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Value))
				.Font(FT66Style::Tokens::FontBold(12))
				.ColorAndOpacity(ChallengeRewardTint())
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeChallengeSpriteButton(FText::FromString(TEXT(">")), OnNext, ET66ChallengeButtonFamily::CompactNeutral, 28.f, 24.f, 12, FMargin(0.f))
			];
	};

	TSharedRef<SVerticalBox> EntryList = SNew(SVerticalBox);
	for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
	{
		EntryList->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			MakeEntryRow(Entries[EntryIndex], EntryIndex)
		];
	}

	const TSharedRef<SWidget> ListPanelContent = Entries.Num() > 0
		? StaticCastSharedRef<SWidget>(
			SNew(SScrollBox)
			.ScrollBarStyle(GetChallengeReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
			+ SScrollBox::Slot()
			[
				EntryList
			])
		: StaticCastSharedRef<SWidget>(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(CurrentSourceTabIndex == static_cast<int32>(ESourceTabIndex::Community)
					? TEXT("No community entries yet. Create the first one.")
					: TEXT("No official entries were found.")))
				.Font(FT66Style::Tokens::FontBold(16))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
			]);

	const TSharedRef<SWidget> CreateButtonContent = MakeChallengeSpriteButton(
		FText::FromString(ActiveKind == ET66CommunityContentKind::Mod ? TEXT("CREATE MOD") : TEXT("CREATE CHALLENGE")),
		FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCreateDraftClicked),
		ET66ChallengeButtonFamily::ToggleOn,
		382.f,
		58.f,
		ActionButtonFontSize,
		FMargin(18.f, 8.f));

	TSharedRef<SWidget> DetailPanelContent = SNew(STextBlock)
		.Text(NSLOCTEXT("T66.Challenges", "NoSelection", "Select an entry or create a new draft."))
		.Font(FT66Style::Tokens::FontRegular(13))
		.ColorAndOpacity(FT66Style::Tokens::TextMuted)
		.AutoWrapText(true);

	if (bDraftEditorActive)
	{
		TSharedRef<SVerticalBox> EditorRows = SNew(SVerticalBox);

		auto AddEditorSpacer = [&EditorRows](float Height)
		{
			EditorRows->AddSlot().AutoHeight().Padding(0.f, Height, 0.f, 0.f)
			[
				SNew(SSpacer)
				.Size(FVector2D(1.f, 1.f))
			];
		};

		EditorRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(DraftEditorEntry.Kind == ET66CommunityContentKind::Mod ? TEXT("Create Mod") : TEXT("Create Challenge")))
			.Font(FT66Style::Tokens::FontBold(28))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		];
		AddEditorSpacer(10.f);
		EditorRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Title")))
			.Font(FT66Style::Tokens::FontBold(12))
			.ColorAndOpacity(ChallengeSuccessTint())
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
		[
			MakeChallengeSpritePanel(
				SNew(SEditableTextBox)
				.Text(FText::FromString(DraftEditorEntry.Title))
				.ForegroundColor(ChallengeFantasyText)
				.BackgroundColor(FLinearColor::Transparent)
				.OnTextChanged_UObject(this, &UT66ChallengesScreen::HandleDraftTitleChanged),
				GetChallengeRowShellBrush(),
				FMargin(10.f, 6.f),
				ChallengePanelInsetFill())
		];
		AddEditorSpacer(10.f);
		EditorRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Description")))
			.Font(FT66Style::Tokens::FontBold(12))
			.ColorAndOpacity(ChallengeSuccessTint())
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
		[
			MakeChallengeSpritePanel(
				SNew(SMultiLineEditableTextBox)
				.Text(FText::FromString(DraftEditorEntry.Description))
				.ForegroundColor(ChallengeFantasyText)
				.OnTextChanged_UObject(this, &UT66ChallengesScreen::HandleDraftDescriptionChanged),
				GetChallengeRowShellBrush(),
				FMargin(10.f, 6.f),
				ChallengePanelInsetFill())
		];

		if (DraftEditorEntry.Kind == ET66CommunityContentKind::Challenge)
		{
			AddEditorSpacer(12.f);
			EditorRows->AddSlot().AutoHeight()
			[
				MakeDraftStepRow(
					TEXT("Suggested Chad Coupons"),
					DraftEditorEntry.SuggestedRewardChadCoupons,
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftReward, -5),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftReward, +5))
			];
		}

		AddEditorSpacer(12.f);
		EditorRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Gameplay Rules")))
			.Font(FT66Style::Tokens::FontBold(12))
			.ColorAndOpacity(ChallengeSuccessTint())
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			MakeDraftStepRow(
				TEXT("Start Level"),
				DraftEditorEntry.Rules.StartLevelOverride,
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftStartLevel, -1),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftStartLevel, +1))
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Max Hero Stats")))
				.Font(FT66Style::Tokens::FontBold(12))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeChallengeSpriteButton(
					FText::FromString(DraftEditorEntry.Rules.bSetMaxHeroStats ? TEXT("Enabled") : TEXT("Disabled")),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleToggleDraftMaxStats),
					DraftEditorEntry.Rules.bSetMaxHeroStats ? ET66ChallengeButtonFamily::ToggleOn : ET66ChallengeButtonFamily::CompactNeutral,
					100.f,
					24.f,
					10,
					FMargin(8.f, 2.f))
			]
		];

		const TArray<TPair<FString, EDraftStatField>> StatFields = {
			TPair<FString, EDraftStatField>(TEXT("Damage"), EDraftStatField::Damage),
			TPair<FString, EDraftStatField>(TEXT("Attack Speed"), EDraftStatField::AttackSpeed),
			TPair<FString, EDraftStatField>(TEXT("Attack Scale"), EDraftStatField::AttackScale),
			TPair<FString, EDraftStatField>(TEXT("Accuracy"), EDraftStatField::Accuracy),
			TPair<FString, EDraftStatField>(TEXT("Armor"), EDraftStatField::Armor),
			TPair<FString, EDraftStatField>(TEXT("Evasion"), EDraftStatField::Evasion),
			TPair<FString, EDraftStatField>(TEXT("Luck"), EDraftStatField::Luck),
			TPair<FString, EDraftStatField>(TEXT("Speed"), EDraftStatField::Speed),
		};

		auto GetDraftStatValue = [this](const EDraftStatField Field)
		{
			switch (Field)
			{
			case EDraftStatField::Damage: return DraftEditorEntry.Rules.BonusStats.Damage;
			case EDraftStatField::AttackSpeed: return DraftEditorEntry.Rules.BonusStats.AttackSpeed;
			case EDraftStatField::AttackScale: return DraftEditorEntry.Rules.BonusStats.AttackScale;
			case EDraftStatField::Accuracy: return DraftEditorEntry.Rules.BonusStats.Accuracy;
			case EDraftStatField::Armor: return DraftEditorEntry.Rules.BonusStats.Armor;
			case EDraftStatField::Evasion: return DraftEditorEntry.Rules.BonusStats.Evasion;
			case EDraftStatField::Luck: return DraftEditorEntry.Rules.BonusStats.Luck;
			case EDraftStatField::Speed: return DraftEditorEntry.Rules.BonusStats.Speed;
			default: return 0;
			}
		};

		for (const TPair<FString, EDraftStatField>& StatField : StatFields)
		{
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				MakeDraftStepRow(
					StatField.Key,
					GetDraftStatValue(StatField.Value),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftStatClicked, StatField.Value, -5),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftStatClicked, StatField.Value, +5))
			];
		}

		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			MakeCycleRow(
				TEXT("Starting Item"),
				GetItemLabel(DraftEditorEntry.Rules.StartingItemId),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftStartingItemClicked, -1),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftStartingItemClicked, +1))
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			MakeCycleRow(
				TEXT("Passive Override"),
				GetPassiveLabel(DraftEditorEntry.Rules.PassiveOverride),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftPassiveClicked, -1),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftPassiveClicked, +1))
		];
		EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			MakeCycleRow(
				TEXT("Ultimate Override"),
				GetUltimateLabel(DraftEditorEntry.Rules.UltimateOverride),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftUltimateClicked, -1),
				FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCycleDraftUltimateClicked, +1))
		];

		if (DraftEditorEntry.Kind == ET66CommunityContentKind::Challenge)
		{
			AddEditorSpacer(12.f);
			EditorRows->AddSlot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Completion Requirements")))
				.Font(FT66Style::Tokens::FontBold(12))
				.ColorAndOpacity(ChallengeSuccessTint())
			];
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				SNew(SCheckBox)
				.IsChecked(DraftEditorEntry.Rules.bRequireFullClear ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_UObject(this, &UT66ChallengesScreen::HandleDraftFullClearChanged)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Require full clear")))
					.Font(FT66Style::Tokens::FontRegular(12))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			];
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				SNew(SCheckBox)
				.IsChecked(DraftEditorEntry.Rules.bRequireNoDamage ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_UObject(this, &UT66ChallengesScreen::HandleDraftNoDamageChanged)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Require no damage")))
					.Font(FT66Style::Tokens::FontRegular(12))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			];
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				MakeDraftStepRow(
					TEXT("Minimum Stage Reached"),
					DraftEditorEntry.Rules.RequiredStageReached,
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftRequiredStage, -1),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftRequiredStage, +1))
			];
			EditorRows->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				MakeDraftStepRow(
					TEXT("Max Run Time (Seconds)"),
					DraftEditorEntry.Rules.MaxRunTimeSeconds,
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftMaxRunTime, -30),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleAdjustDraftMaxRunTime, +30))
			];
		}

		AddEditorSpacer(16.f);
		EditorRows->AddSlot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "SaveDraft", "SAVE DRAFT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSaveDraftClicked), ET66ChallengeButtonFamily::ToggleOn, 136.f, 34.f, 12)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "SubmitDraft", "SUBMIT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSubmitDraftClicked), ET66ChallengeButtonFamily::ToggleOn, 120.f, 34.f, 12)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "PlayDraft", "PLAY DRAFT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandlePlayDraftClicked), ET66ChallengeButtonFamily::CompactNeutral, 120.f, 34.f, 12)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "CancelDraft", "CANCEL"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleCancelDraftEditorClicked), ET66ChallengeButtonFamily::ToggleOff, 112.f, 34.f, 12)
			]
		];

		DetailPanelContent =
			SNew(SScrollBox)
			.ScrollBarStyle(GetChallengeReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
			+ SScrollBox::Slot()
			[
				EditorRows
			];
	}
	else if (bHasSelectedEntry)
	{
		const TArray<FString> RuleLines = Community ? Community->BuildRuleSummaryLines(SelectedEntry) : TArray<FString>{};
		TSharedRef<SVerticalBox> RuleList = SNew(SVerticalBox);
		for (const FString& RuleLine : RuleLines)
		{
			RuleList->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				MakeConstraintRow(RuleLine)
			];
		}

		TSharedRef<SVerticalBox> DetailLayout = SNew(SVerticalBox);
		DetailLayout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(SelectedEntry.Title))
			.Font(FT66Style::Tokens::FontBold(34))
			.ColorAndOpacity(ChallengeFantasyText)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
		];
		DetailLayout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
			[
				SelectedEntry.AuthorAvatarUrl.IsEmpty()
				? StaticCastSharedRef<SWidget>(SNew(SSpacer).Size(FVector2D(0.f, 0.f)))
				: StaticCastSharedRef<SWidget>(
					SNew(SBox)
					.WidthOverride(52.f)
					.HeightOverride(52.f)
					[
						SNew(SImage)
						.Image(GetOrCreateAvatarBrush(SelectedEntry.AuthorAvatarUrl))
					])
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s%s%s"),
					*GetOriginLabel(SelectedEntry),
					SelectedEntry.AuthorDisplayName.IsEmpty() ? TEXT("") : TEXT(" by "),
					*SelectedEntry.AuthorDisplayName)))
				.Font(FT66Style::Tokens::FontBold(16))
				.ColorAndOpacity(ChallengeGoldText)
				.AutoWrapText(true)
			]
		];
		DetailLayout->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
		[
			SNew(SBox)
			.HeightOverride(132.f)
			[
				MakeChallengeSpritePanel(
					SNew(STextBlock)
					.Text(FText::FromString(SelectedEntry.Description))
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(ChallengePaperText)
					.AutoWrapText(true),
					GetChallengeDetailPaperBrush(),
					FMargin(26.f, 24.f),
					ChallengePanelInsetFill())
			]
		];
		DetailLayout->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(DetailListHeader))
			.Font(FT66Style::Tokens::FontBold(25))
			.ColorAndOpacity(ChallengeFantasyText)
		];
		DetailLayout->AddSlot().AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(248.f)
			[
				MakeChallengeSpritePanel(
					SNew(SScrollBox)
					.ScrollBarStyle(GetChallengeReferenceScrollBarStyle())
					.ScrollBarVisibility(EVisibility::Visible)
					.ScrollBarThickness(FVector2D(14.f, 14.f))
					.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
					+ SScrollBox::Slot()
					[
						RuleList
					],
					GetChallengeRulesPaperBrush(),
					FMargin(28.f, 22.f),
					ChallengePanelInsetFill())
			]
		];
		DetailLayout->AddSlot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Community ? Community->BuildRewardSummary(SelectedEntry) : TEXT("No reward data")))
			.Font(FT66Style::Tokens::FontBold(22))
			.ColorAndOpacity(ChallengeGoldText)
		];
		DetailLayout->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(STextBlock)
				.Text(FText::FromString(Community ? Community->BuildSelectionSummary(SelectedEntry) : TEXT("No selection summary available.")))
				.Font(FT66Style::Tokens::FontRegular(11))
				.ColorAndOpacity(ChallengeFantasyMuted)
				.AutoWrapText(true)
		];

		if (SelectedEntry.IsDraft())
		{
			DetailLayout->AddSlot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(GetDraftSubmissionLabel(SelectedEntry)))
				.Font(FT66Style::Tokens::FontBold(14))
				.ColorAndOpacity(ChallengeDangerTint())
			];
			DetailLayout->AddSlot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
				[
					MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "EditDraft", "EDIT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleEditDraftClicked), ET66ChallengeButtonFamily::CompactNeutral, 100.f, 34.f, 12)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
				[
					MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "SubmitSelectedDraft", "SUBMIT"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleSubmitDraftClicked), ET66ChallengeButtonFamily::ToggleOn, 112.f, 34.f, 12)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
				[
					MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "PlaySelectedDraft", "PLAY"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandlePlayDraftClicked), ET66ChallengeButtonFamily::ToggleOn, 112.f, 34.f, 12)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeChallengeSpriteButton(NSLOCTEXT("T66.Challenges", "DeleteSelectedDraft", "DELETE"), FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleDeleteDraftClicked), ET66ChallengeButtonFamily::ToggleOff, 112.f, 34.f, 12)
				]
			];
		}
		else
		{
			DetailLayout->AddSlot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(bSelectedEntryConfirmed ? TEXT("Selected for the next run.") : TEXT("Confirm this entry to arm it for the next run.")))
				.Font(FT66Style::Tokens::FontRegular(11))
				.ColorAndOpacity(ChallengeFantasyMuted)
				.AutoWrapText(true)
			];
			DetailLayout->AddSlot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 16.f, 0.f, 0.f)
			[
				MakeChallengeSpriteButton(
					FText::FromString(bSelectedEntryConfirmed ? TEXT("SELECTED") : TEXT("CONFIRM")),
					FOnClicked::CreateUObject(this, &UT66ChallengesScreen::HandleConfirmClicked),
					ET66ChallengeButtonFamily::ToggleOn,
					270.f,
					62.f,
					20,
					FMargin(22.f, 10.f, 22.f, 8.f))
			];
		}

		DetailPanelContent =
			SNew(SScrollBox)
			.ScrollBarStyle(GetChallengeReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
			+ SScrollBox::Slot()
			[
				DetailLayout
			];
	}

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.030f, 0.015f, 0.008f, 1.0f))
		[
			SNew(SBox)
			.WidthOverride(1920.f)
			.HeightOverride(1080.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(8.f, 26.f, 0.f, 0.f))
				[
					SNew(SBox)
					.WidthOverride(1904.f)
					.HeightOverride(930.f)
					[
						MakeChallengeSpritePanel(
							SNew(SSpacer),
							GetChallengeContentShellBrush(),
							FMargin(0.f),
							ChallengeShellFill())
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(FMargin(0.f, 44.f, 0.f, 0.f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(HeaderTitle.ToUpper()))
					.Font(FT66Style::MakeFont(TEXT("Black"), 62))
					.ColorAndOpacity(ChallengeFantasyText)
					.ShadowOffset(FVector2D(0.f, 2.f))
					.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(1228.f, 52.f, 0.f, 0.f))
				[
					MakeTopTabButton(static_cast<int32>(ETabIndex::Challenges), NSLOCTEXT("T66.Challenges", "ChallengesTab", "CHALLENGES"))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(1494.f, 52.f, 0.f, 0.f))
				[
					MakeTopTabButton(static_cast<int32>(ETabIndex::Mods), NSLOCTEXT("T66.Challenges", "ModsTab", "MODS"))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(374.f, 142.f, 0.f, 0.f))
				[
					MakeSourceTabButton(static_cast<int32>(ESourceTabIndex::Official), NSLOCTEXT("T66.Challenges", "OfficialTab", "OFFICIAL"))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(756.f, 142.f, 0.f, 0.f))
				[
					MakeSourceTabButton(static_cast<int32>(ESourceTabIndex::Community), NSLOCTEXT("T66.Challenges", "CommunityTab", "COMMUNITY"))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(1190.f, 142.f, 0.f, 0.f))
				[
					CreateButtonContent
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(44.f, 212.f, 0.f, 0.f))
				[
					SNew(SBox)
					.WidthOverride(1832.f)
					.HeightOverride(48.f)
					[
						MakeChallengeHorizontalSlicedPanel(
							SNew(STextBlock)
							.Text(FText::FromString(StatusMessage))
							.Font(FT66Style::Tokens::FontBold(17))
							.ColorAndOpacity(ChallengeFantasyMuted)
							.Justification(ETextJustify::Center)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds),
							GetChallengeStatusBarBrush(),
							48.f,
							FMargin(28.f, 12.f, 28.f, 10.f),
							ChallengePanelFill(),
							0.125f)
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(38.f, 278.f, 0.f, 0.f))
				[
					SNew(SBox)
					.WidthOverride(ListPanelWidth)
					.HeightOverride(BodyPanelHeight)
					[
						MakeChallengeSpritePanel(
							MakeChallengeSpritePanel(
								SNew(SBox)
								.WidthOverride(ListColumnWidth)
								[
									ListPanelContent
								],
								GetChallengeDetailPaperBrush(),
								FMargin(18.f, 20.f),
								ChallengePanelInsetFill()),
							GetChallengeContentShellBrush(),
							FMargin(22.f, 28.f),
							ChallengePanelFill())
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(1040.f, 278.f, 0.f, 0.f))
				[
					SNew(SBox)
					.WidthOverride(DetailPanelWidth)
					.HeightOverride(BodyPanelHeight)
					[
						MakeChallengeSpritePanel(
							SNew(SBox)
							.WidthOverride(DetailColumnWidth)
							[
								DetailPanelContent
							],
							GetChallengeContentShellBrush(),
							FMargin(34.f, 28.f, 32.f, 28.f),
							ChallengePanelFill())
					]
				]
			]
		];
}

FReply UT66ChallengesScreen::HandleBackClicked()
{
	bSelectionStateInitialized = false;
	bRequestedCommunityRefresh = false;
	EndDraftEditor();
	if (bIsModal)
	{
		CloseModal();
	}
	else
	{
		NavigateBack();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleTabSelected(const int32 TabIndex)
{
	InitializeSelectionState();
	ActiveTabIndex = FMath::Clamp(TabIndex, 0, static_cast<int32>(ETabIndex::Count) - 1);
	EndDraftEditor();
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleSourceTabSelected(const int32 SourceTabIndex)
{
	InitializeSelectionState();
	ActiveSourceTabIndex[ActiveTabIndex] = FMath::Clamp(SourceTabIndex, 0, static_cast<int32>(ESourceTabIndex::Count) - 1);
	EndDraftEditor();
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleEntrySelected(const int32 EntryIndex)
{
	const int32 SourceTabIndex = ActiveSourceTabIndex[ActiveTabIndex];
	const TArray<FT66CommunityContentEntry> Entries = GetEntriesForView(ActiveTabIndex, SourceTabIndex);
	if (Entries.IsValidIndex(EntryIndex))
	{
		PendingSelections[ActiveTabIndex][SourceTabIndex] = Entries[EntryIndex].LocalId;
	}

	EndDraftEditor();
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleConfirmClicked()
{
	FT66CommunityContentEntry Entry;
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		if (FindCurrentSelectedEntry(Entry))
		{
			Community->ActivateEntry(Entry.LocalId);
		}
	}

	bSelectionStateInitialized = false;
	if (bIsModal)
	{
		CloseModal();
	}
	else
	{
		NavigateBack();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCreateDraftClicked()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		BeginDraftEditor(Community->CreateDraftTemplate(GetActiveKind()));
		ForceRebuildSlate();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleEditDraftClicked()
{
	FT66CommunityContentEntry Entry;
	if (FindCurrentSelectedEntry(Entry) && Entry.IsDraft())
	{
		BeginDraftEditor(Entry);
		ForceRebuildSlate();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleDeleteDraftClicked()
{
	FT66CommunityContentEntry Entry;
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		if (FindCurrentSelectedEntry(Entry) && Entry.IsDraft())
		{
			Community->DeleteDraft(Entry.LocalId);
			EndDraftEditor();
			PendingSelections[ActiveTabIndex][ActiveSourceTabIndex[ActiveTabIndex]] = NAME_None;
			ForceRebuildSlate();
		}
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleSaveDraftClicked()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		Community->SaveDraft(DraftEditorEntry);
		PendingSelections[ActiveTabIndex][static_cast<int32>(ESourceTabIndex::Community)] = DraftEditorEntry.LocalId;
	}
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandlePlayDraftClicked()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		if (bDraftEditorActive)
		{
			Community->SaveDraft(DraftEditorEntry);
			Community->ActivateEntry(DraftEditorEntry.LocalId);
		}
		else
		{
			FT66CommunityContentEntry Entry;
			if (FindCurrentSelectedEntry(Entry) && Entry.IsDraft())
			{
				Community->ActivateEntry(Entry.LocalId);
			}
		}
	}

	bSelectionStateInitialized = false;
	if (bIsModal)
	{
		CloseModal();
	}
	else
	{
		NavigateBack();
	}
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleSubmitDraftClicked()
{
	if (UT66CommunityContentSubsystem* Community = GetCommunitySubsystem())
	{
		const FName DraftId = bDraftEditorActive ? DraftEditorEntry.LocalId : GetSelectedEntryIdForView(ActiveTabIndex, ActiveSourceTabIndex[ActiveTabIndex]);
		if (bDraftEditorActive)
		{
			Community->SaveDraft(DraftEditorEntry);
		}
		Community->SubmitDraftForApproval(DraftId);
	}
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCancelDraftEditorClicked()
{
	EndDraftEditor();
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftReward(const int32 Delta)
{
	DraftEditorEntry.SuggestedRewardChadCoupons = T66CommunityContentLimits::ClampRewardChadCoupons(DraftEditorEntry.SuggestedRewardChadCoupons + Delta);
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftStartLevel(const int32 Delta)
{
	DraftEditorEntry.Rules.StartLevelOverride = T66CommunityContentLimits::ClampStartLevelOverride(DraftEditorEntry.Rules.StartLevelOverride + Delta);
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftRequiredStage(const int32 Delta)
{
	DraftEditorEntry.Rules.RequiredStageReached = T66CommunityContentLimits::ClampRequiredStageReached(DraftEditorEntry.Rules.RequiredStageReached + Delta);
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftMaxRunTime(const int32 Delta)
{
	DraftEditorEntry.Rules.MaxRunTimeSeconds = T66CommunityContentLimits::ClampRunTimeSeconds(DraftEditorEntry.Rules.MaxRunTimeSeconds + Delta);
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleToggleDraftMaxStats()
{
	DraftEditorEntry.Rules.bSetMaxHeroStats = !DraftEditorEntry.Rules.bSetMaxHeroStats;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleAdjustDraftStatClicked(const EDraftStatField Field, const int32 Delta)
{
	AdjustDraftStat(Field, Delta);
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCycleDraftPassiveClicked(const int32 Direction)
{
	CycleDraftPassive(Direction);
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCycleDraftUltimateClicked(const int32 Direction)
{
	CycleDraftUltimate(Direction);
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66ChallengesScreen::HandleCycleDraftStartingItemClicked(const int32 Direction)
{
	CycleDraftStartingItem(Direction);
	ForceRebuildSlate();
	return FReply::Handled();
}

void UT66ChallengesScreen::HandleDraftTitleChanged(const FText& NewText)
{
	DraftEditorEntry.Title = NewText.ToString();
}

void UT66ChallengesScreen::HandleDraftDescriptionChanged(const FText& NewText)
{
	DraftEditorEntry.Description = NewText.ToString();
}

void UT66ChallengesScreen::HandleDraftFullClearChanged(const ECheckBoxState NewState)
{
	DraftEditorEntry.Rules.bRequireFullClear = (NewState == ECheckBoxState::Checked);
}

void UT66ChallengesScreen::HandleDraftNoDamageChanged(const ECheckBoxState NewState)
{
	DraftEditorEntry.Rules.bRequireNoDamage = (NewState == ECheckBoxState::Checked);
}

void UT66ChallengesScreen::HandleCommunityContentChanged()
{
	ForceRebuildSlate();
}
