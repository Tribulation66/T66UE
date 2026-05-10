// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AccountStatusScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66UIManager.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "Engine/TextureDefines.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Data/T66DataTypes.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 AccountFontDelta = -2;

	int32 AdjustAccountFontSize(int32 BaseSize)
	{
		return FMath::Max(6, BaseSize + AccountFontDelta);
	}

	FSlateFontInfo AccountBoldFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontBold(AdjustAccountFontSize(BaseSize));
	}

	FSlateFontInfo AccountRegularFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontRegular(AdjustAccountFontSize(BaseSize));
	}

	float AccountPreviewProgress01(const int32 Index)
	{
		static constexpr float PreviewValues[] =
		{
			0.42f,
			0.58f,
			0.70f,
			0.36f,
			0.51f,
			0.64f
		};
		static constexpr int32 PreviewValueCount = sizeof(PreviewValues) / sizeof(PreviewValues[0]);
		return PreviewValues[Index % PreviewValueCount];
	}

	FLinearColor AccountGold()
	{
		return FLinearColor(0.92f, 0.05f, 0.12f, 1.0f);
	}

	FLinearColor AccountMutedGold()
	{
		return FLinearColor(0.88f, 0.34f, 0.30f, 1.0f);
	}

	FLinearColor AccountChromeText()
	{
		return FLinearColor(0.96f, 0.99f, 1.0f, 1.0f);
	}

	FLinearColor AccountPanelFill()
	{
		return FLinearColor(0.020f, 0.024f, 0.034f, 0.97f);
	}

	FLinearColor AccountPanelInnerFill()
	{
		return FLinearColor(0.046f, 0.018f, 0.020f, 0.96f);
	}

	FLinearColor AccountRowFill()
	{
		return FLinearColor(0.52f, 0.035f, 0.045f, 0.66f);
	}

	FLinearColor AccountRowAltFill()
	{
		return FLinearColor(0.34f, 0.024f, 0.034f, 0.58f);
	}

	FLinearColor AccountHeaderFill()
	{
		return FLinearColor(0.70f, 0.045f, 0.055f, 0.78f);
	}

	FLinearColor AccountText()
	{
		return FLinearColor(0.90f, 0.96f, 1.0f, 1.0f);
	}

	FLinearColor AccountMutedText()
	{
		return FLinearColor(0.58f, 0.70f, 0.78f, 1.0f);
	}

	FLinearColor AccountSuccess()
	{
		return FLinearColor(1.0f, 0.24f, 0.22f, 1.0f);
	}

	FLinearColor AccountDanger()
	{
		return FLinearColor(1.0f, 0.24f, 0.22f, 1.0f);
	}

	struct FAccountReferenceButtonBrushSet
	{
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Normal;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Hovered;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Pressed;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Disabled;
	};

	struct FAccountReferenceButtonStyleEntry
	{
		FButtonStyle Style;
		bool bInitialized = false;
	};

	const FSlateBrush* ResolveAccountReferenceBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const FString& RelativePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel,
			Filter);
	}

	const TCHAR* GAccountStatusReferenceStateFolder = TEXT("Overview");

	FString MakeAccountMainMenuElementPath(const TCHAR* FileName)
	{
		return T66ScreenSlateHelpers::MakeReferenceMainMenuElementAssetPath(FileName);
	}

	FString MakeAccountMainMenuSquareElementPath(const TCHAR* FileName)
	{
		const FString Name(FileName ? FileName : TEXT(""));
		if (Name.StartsWith(TEXT("dropdown_field_"), ESearchCase::IgnoreCase)
			|| Name.StartsWith(TEXT("leaderboard_tab_button_"), ESearchCase::IgnoreCase))
		{
			FString State = FPaths::GetBaseFilename(Name).ToLower();
			State.RemoveFromStart(Name.StartsWith(TEXT("dropdown_field_"), ESearchCase::IgnoreCase)
				? TEXT("dropdown_field_")
				: TEXT("leaderboard_tab_button_"),
				ESearchCase::IgnoreCase);
			return T66ScreenSlateHelpers::MakeReferenceRedSquareButtonAssetPath(*State);
		}
		return T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(FileName);
	}

	FString MakeAccountMainMenuLongPanelPath(const TCHAR* State = TEXT("normal"))
	{
		return T66ScreenSlateHelpers::MakeReferenceLongPanelAssetPath(State);
	}

	FString ResolveAccountReferenceStateName(const FString& RelativePath)
	{
		const FString BaseName = FPaths::GetBaseFilename(RelativePath).ToLower();
		if (BaseName.Contains(TEXT("disabled")))
		{
			return TEXT("disabled");
		}
		if (BaseName.Contains(TEXT("pressed")))
		{
			return TEXT("pressed");
		}
		if (BaseName.Contains(TEXT("hover")))
		{
			return TEXT("hover");
		}
		if (BaseName.Contains(TEXT("selected")) || BaseName.Contains(TEXT("active")) || BaseName.Contains(TEXT("focused")))
		{
			return TEXT("selected");
		}
		return TEXT("normal");
	}

	FString ResolveAccountMainMenuFallbackPath(const FString& StateRelativePath, const FString& LegacyRelativePath)
	{
		FString NormalizedPath = StateRelativePath + TEXT("|") + LegacyRelativePath;
		NormalizedPath.ReplaceInline(TEXT("\\"), TEXT("/"));
		const FString LowerPath = NormalizedPath.ToLower();
		const FString State = ResolveAccountReferenceStateName(NormalizedPath);

		if (LowerPath.Contains(TEXT("screenart")))
		{
			return FString();
		}

		if (LowerPath.Contains(TEXT("scrollbar")))
		{
			return LowerPath.Contains(TEXT("thumb"))
				? MakeAccountMainMenuElementPath(TEXT("progress_bar_fill_red.png"))
				: MakeAccountMainMenuSquareElementPath(TEXT("dropdown_field_normal_square_variant.png"));
		}

		if (LowerPath.Contains(TEXT("buttons/")) || LowerPath.Contains(TEXT("_pill_")))
		{
			return T66ScreenSlateHelpers::MakeReferenceRedSquareButtonAssetPath(*State);
		}

		if (LowerPath.Contains(TEXT("dropdown_field")))
		{
			const FString DropdownState = State.Equals(TEXT("selected"), ESearchCase::IgnoreCase) ? FString(TEXT("pressed")) : State;
			return T66ScreenSlateHelpers::MakeReferenceRedSquareButtonAssetPath(*DropdownState);
		}

		if (LowerPath.Contains(TEXT("slots/")) || LowerPath.Contains(TEXT("avatar_slot")) || LowerPath.Contains(TEXT("square_slot")))
		{
			const FString SlotState = State.Equals(TEXT("pressed"), ESearchCase::IgnoreCase) || State.Equals(TEXT("selected"), ESearchCase::IgnoreCase)
				? FString(TEXT("selected_red"))
				: State;
			return MakeAccountMainMenuSquareElementPath(*FString::Printf(TEXT("profile_slot_%s_square_variant.png"), *SlotState));
		}

		if (LowerPath.Contains(TEXT("progress")) || LowerPath.Contains(TEXT("meter")))
		{
			return MakeAccountMainMenuElementPath(LowerPath.Contains(TEXT("fill")) ? TEXT("progress_bar_fill_red.png") : TEXT("progress_bar_track.png"));
		}

		if (LowerPath.Contains(TEXT("row")) || LowerPath.Contains(TEXT("strip")) || LowerPath.Contains(TEXT("table")))
		{
			return LowerPath.Contains(TEXT("header"))
				? T66ScreenSlateHelpers::MakeReferenceRedSquareButtonAssetPath(TEXT("normal"))
				: MakeAccountMainMenuLongPanelPath(TEXT("normal"));
		}

		if (LowerPath.Contains(TEXT("panels/")) || LowerPath.Contains(TEXT("panel")))
		{
			return MakeAccountMainMenuSquareElementPath(TEXT("main_panel_normal_square_variant.png"));
		}

		return FString();
	}

	bool IsAccountHistoryReferenceState()
	{
		return FCString::Stricmp(GAccountStatusReferenceStateFolder, TEXT("History")) == 0;
	}

	FString ResolveAccountHistoryRelativePath(const FString& StateRelativePath)
	{
		if (!IsAccountHistoryReferenceState())
		{
			return StateRelativePath;
		}

		if (StateRelativePath.StartsWith(TEXT("Buttons/accountstatus_overview_pill_")))
		{
			FString HistoryPath = StateRelativePath;
			HistoryPath.ReplaceInline(TEXT("accountstatus_overview_pill_"), TEXT("accountstatus_history_pill_"));
			return HistoryPath;
		}

		if (StateRelativePath == TEXT("Controls/accountstatus_overview_dropdown_field_normal.png"))
		{
			return TEXT("Controls/accountstatus_history_dropdown_field_normal.png");
		}

		if (StateRelativePath == TEXT("Icons/accountstatus_iconsgenerated_icon_16_dropdown_chevron_v2.png"))
		{
			return TEXT("Icons/accountstatus_history_dropdown_chevron.png");
		}

		if (StateRelativePath == TEXT("Slots/accountstatus_overview_square_slot_frame_normal.png"))
		{
			return TEXT("Slots/accountstatus_history_avatar_slot_frame.png");
		}

		if (StateRelativePath == TEXT("Panels/accountstatus_overview_row_shell.png"))
		{
			return TEXT("Panels/accountstatus_history_row_shell.png");
		}

		if (StateRelativePath == TEXT("Panels/accountstatus_overview_paper_panel.png"))
		{
			return TEXT("Panels/accountstatus_history_table_panel_shell.png");
		}

		return StateRelativePath;
	}

	FString ResolveAccountStateAssetPath(const FString& StateRelativePath, const FString& CommonRelativePath, const FString& LegacyRelativePath)
	{
		const FString ResolvedStateRelativePath = ResolveAccountHistoryRelativePath(StateRelativePath);
		const FString StatePath = FString::Printf(TEXT("SourceAssets/UI/Reference/Screens/AccountStatus/%s/%s"), GAccountStatusReferenceStateFolder, *ResolvedStateRelativePath);
		if (IFileManager::Get().FileExists(*(FPaths::ProjectDir() / StatePath)))
		{
			return StatePath;
		}

		if (!CommonRelativePath.IsEmpty())
		{
			const FString CommonPath = TEXT("SourceAssets/UI/Reference/Screens/AccountStatus/Common/") + CommonRelativePath;
			if (IFileManager::Get().FileExists(*(FPaths::ProjectDir() / CommonPath)))
			{
				return CommonPath;
			}
		}

		const FString MainMenuFallbackPath = ResolveAccountMainMenuFallbackPath(ResolvedStateRelativePath, LegacyRelativePath);
		return MainMenuFallbackPath.IsEmpty() ? LegacyRelativePath : MainMenuFallbackPath;
	}

	const FSlateBrush* ResolveAccountReferenceRegionBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const FString& RelativePath,
		const FMargin& Margin,
		const FBox2f& UVRegion,
		const FVector2D& ImageSize,
		const ESlateBrushDrawType::Type DrawAs,
		const FLinearColor& Tint,
		const TCHAR* DebugLabel,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		UTexture2D* Texture = T66RuntimeUIBrushAccess::LoadOptionalTexture(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel,
			Filter);
		if (!Texture)
		{
			return Entry.bSimpleReferenceFallback && Entry.Brush.IsValid() ? Entry.Brush.Get() : nullptr;
		}
		if (!Entry.Brush.IsValid())
		{
			return nullptr;
		}

		Entry.Brush->DrawAs = DrawAs;
		Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
		Entry.Brush->ImageSize = ImageSize;
		Entry.Brush->Margin = Margin;
		Entry.Brush->TintColor = FSlateColor(Tint);
		Entry.Brush->SetUVRegion(UVRegion);
		Entry.Brush->SetResourceObject(Texture);
		return Entry.Brush.Get();
	}

	const TCHAR* GetAccountReferenceButtonPrefix(ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return TEXT("Buttons/basic_button");
		case ET66ButtonType::Danger:
			return TEXT("Buttons/basic_button");
		case ET66ButtonType::Neutral:
		case ET66ButtonType::Row:
		default:
			return TEXT("Buttons/basic_button");
		}
	}

	FAccountReferenceButtonBrushSet& GetAccountReferenceButtonBrushSet(ET66ButtonType Type)
	{
		static FAccountReferenceButtonBrushSet Neutral;
		static FAccountReferenceButtonBrushSet Success;
		static FAccountReferenceButtonBrushSet Danger;
		static FAccountReferenceButtonBrushSet ToggleActive;
		static FAccountReferenceButtonBrushSet HistoryNeutral;
		static FAccountReferenceButtonBrushSet HistorySuccess;
		static FAccountReferenceButtonBrushSet HistoryDanger;
		static FAccountReferenceButtonBrushSet HistoryToggleActive;

		switch (Type)
		{
		case ET66ButtonType::ToggleActive:
			return IsAccountHistoryReferenceState() ? HistoryToggleActive : ToggleActive;
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
			return IsAccountHistoryReferenceState() ? HistorySuccess : Success;
		case ET66ButtonType::Danger:
			return IsAccountHistoryReferenceState() ? HistoryDanger : Danger;
		case ET66ButtonType::Neutral:
		case ET66ButtonType::Row:
		default:
			return IsAccountHistoryReferenceState() ? HistoryNeutral : Neutral;
		}
	}

	FAccountReferenceButtonStyleEntry& GetAccountReferenceButtonStyleEntry(ET66ButtonType Type)
	{
		static FAccountReferenceButtonStyleEntry Neutral;
		static FAccountReferenceButtonStyleEntry Success;
		static FAccountReferenceButtonStyleEntry Danger;
		static FAccountReferenceButtonStyleEntry ToggleActive;
		static FAccountReferenceButtonStyleEntry HistoryNeutral;
		static FAccountReferenceButtonStyleEntry HistorySuccess;
		static FAccountReferenceButtonStyleEntry HistoryDanger;
		static FAccountReferenceButtonStyleEntry HistoryToggleActive;

		switch (Type)
		{
		case ET66ButtonType::ToggleActive:
			return IsAccountHistoryReferenceState() ? HistoryToggleActive : ToggleActive;
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
			return IsAccountHistoryReferenceState() ? HistorySuccess : Success;
		case ET66ButtonType::Danger:
			return IsAccountHistoryReferenceState() ? HistoryDanger : Danger;
		case ET66ButtonType::Neutral:
		case ET66ButtonType::Row:
		default:
			return IsAccountHistoryReferenceState() ? HistoryNeutral : Neutral;
		}
	}

	const FSlateBrush* ResolveAccountReferenceButtonBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* Prefix,
		const TCHAR* State,
		const TCHAR* DebugLabel)
	{
		(void)Prefix;
		return ResolveAccountReferenceBrush(
			Entry,
			T66ScreenSlateHelpers::MakeReferenceRedSquareButtonAssetPath(State ? State : TEXT("normal")),
			FMargin(0.083f, 0.231f, 0.083f, 0.231f),
			DebugLabel,
			TextureFilter::TF_Nearest);
	}

	const FButtonStyle& GetAccountReferenceButtonStyle(ET66ButtonType Type)
	{
		FAccountReferenceButtonStyleEntry& StyleEntry = GetAccountReferenceButtonStyleEntry(Type);
		if (!StyleEntry.bInitialized)
		{
			StyleEntry.bInitialized = true;
			StyleEntry.Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			StyleEntry.Style.SetNormalPadding(FMargin(0.f));
			StyleEntry.Style.SetPressedPadding(FMargin(0.f));

			FAccountReferenceButtonBrushSet& BrushSet = GetAccountReferenceButtonBrushSet(Type);
			const TCHAR* Prefix = GetAccountReferenceButtonPrefix(Type);
			const bool bUsePressedAsRestState = Type == ET66ButtonType::ToggleActive;
			if (const FSlateBrush* Brush = ResolveAccountReferenceButtonBrush(BrushSet.Normal, Prefix, TEXT("normal"), TEXT("AccountButtonNormal")))
			{
				StyleEntry.Style.SetNormal(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveAccountReferenceButtonBrush(BrushSet.Hovered, Prefix, TEXT("hover"), TEXT("AccountButtonHover")))
			{
				StyleEntry.Style.SetHovered(*Brush);
			}
			if (const FSlateBrush* Brush = ResolveAccountReferenceButtonBrush(BrushSet.Pressed, Prefix, bUsePressedAsRestState ? TEXT("selected") : TEXT("pressed"), TEXT("AccountButtonPressed")))
			{
				StyleEntry.Style.SetPressed(*Brush);
				if (bUsePressedAsRestState)
				{
					StyleEntry.Style.SetNormal(*Brush);
					StyleEntry.Style.SetHovered(*Brush);
				}
			}
			if (const FSlateBrush* Brush = ResolveAccountReferenceButtonBrush(BrushSet.Disabled, TEXT("Buttons/basic_button"), TEXT("disabled"), TEXT("AccountButtonDisabled")))
			{
				StyleEntry.Style.SetDisabled(*Brush);
			}
		}

		return StyleEntry.Style;
	}

	const FSlateBrush* GetAccountContentShellBrush()
	{
		if (IsAccountHistoryReferenceState())
		{
			return nullptr;
		}

		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveAccountReferenceBrush(
			Entry,
			ResolveAccountStateAssetPath(
				TEXT("Panels/accountstatus_overview_content_shell.png"),
				TEXT("Panels/accountstatus_panels_fullscreen_fullscreen_panel_wide.png"),
				TEXT("SourceAssets/UI/Reference/Screens/AccountStatus/Panels/accountstatus_panels_fullscreen_fullscreen_panel_wide.png")),
			FMargin(0.060f, 0.090f, 0.060f, 0.105f),
			TEXT("AccountContentShellV16"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetAccountRowShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush HistoryEntry;
		return ResolveAccountReferenceBrush(
			IsAccountHistoryReferenceState() ? HistoryEntry : Entry,
			ResolveAccountStateAssetPath(
				TEXT("Panels/accountstatus_overview_row_shell.png"),
				TEXT("Panels/accountstatus_panels_fullscreen_row_shell_quiet.png"),
				TEXT("SourceAssets/UI/Reference/Screens/AccountStatus/Panels/accountstatus_panels_fullscreen_row_shell_quiet.png")),
			FMargin(0.070f, 0.155f, 0.070f, 0.155f),
			TEXT("AccountRowShellV16"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetAccountPaperPanelBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush HistoryEntry;
		return ResolveAccountReferenceBrush(
			IsAccountHistoryReferenceState() ? HistoryEntry : Entry,
			ResolveAccountStateAssetPath(
				TEXT("Panels/accountstatus_overview_paper_panel.png"),
				TEXT("Panels/accountstatus_panels_reference_scroll_paper_frame.png"),
				TEXT("SourceAssets/UI/Reference/Screens/AccountStatus/Panels/accountstatus_panels_reference_scroll_paper_frame.png")),
			FMargin(0.085f, 0.125f, 0.085f, 0.125f),
			TEXT("AccountPaperPanel"),
			TextureFilter::TF_Nearest);
	}

	const FScrollBarStyle* GetAccountReferenceScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush TrackEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush ThumbEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush HoverEntry;
		static FScrollBarStyle HistoryStyle = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush HistoryTrackEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush HistoryThumbEntry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush HistoryHoverEntry;

		if (IsAccountHistoryReferenceState())
		{
			const FString TrackPath = MakeAccountMainMenuSquareElementPath(TEXT("dropdown_field_normal_square_variant.png"));
			const FString ThumbPath = MakeAccountMainMenuElementPath(TEXT("progress_bar_fill_red.png"));
			const FSlateBrush* TrackBrush = ResolveAccountReferenceBrush(
				HistoryTrackEntry,
				TrackPath,
				FMargin(0.42f, 0.085f, 0.42f, 0.085f),
				TEXT("AccountHistoryScrollbarTrack"),
				TextureFilter::TF_Nearest);
			const FSlateBrush* ThumbBrush = ResolveAccountReferenceBrush(
				HistoryThumbEntry,
				ThumbPath,
				FMargin(0.38f, 0.115f, 0.38f, 0.115f),
				TEXT("AccountHistoryScrollbarThumb"),
				TextureFilter::TF_Nearest);
			const FSlateBrush* HoverBrush = ResolveAccountReferenceBrush(
				HistoryHoverEntry,
				ThumbPath,
				FMargin(0.38f, 0.115f, 0.38f, 0.115f),
				TEXT("AccountHistoryScrollbarThumbHover"),
				TextureFilter::TF_Nearest);

			if (TrackBrush && ThumbBrush && HoverBrush)
			{
				HistoryStyle
					.SetVerticalBackgroundImage(*TrackBrush)
					.SetVerticalTopSlotImage(*TrackBrush)
					.SetVerticalBottomSlotImage(*TrackBrush)
					.SetNormalThumbImage(*ThumbBrush)
					.SetHoveredThumbImage(*HoverBrush)
					.SetDraggedThumbImage(*HoverBrush)
					.SetThickness(23.f);
			}

			return &HistoryStyle;
		}

		const FString ControlsPath = ResolveAccountStateAssetPath(
			TEXT("Controls/accountstatus_overview_scrollbar_vertical.png"),
			TEXT("Controls/accountstatus_controls_controls_sheet.png"),
			TEXT("SourceAssets/UI/Reference/Screens/AccountStatus/Controls/accountstatus_controls_controls_sheet.png"));
		const FString ThumbPath = ResolveAccountStateAssetPath(
			TEXT("Controls/accountstatus_overview_scrollbar_thumb.png"),
			FString(),
			ControlsPath);
		const FBox2f VerticalBarUV(
			FVector2f(0.f, 0.f),
			FVector2f(1.f, 1.f));

		const FSlateBrush* TrackBrush = ResolveAccountReferenceRegionBrush(
			TrackEntry,
			ControlsPath,
			FMargin(0.42f, 0.085f, 0.42f, 0.085f),
			VerticalBarUV,
			FVector2D(14.f, 120.f),
			ESlateBrushDrawType::Box,
			FLinearColor(0.035f, 0.055f, 0.070f, 0.92f),
			TEXT("AccountScrollbarTrackV16"),
			TextureFilter::TF_Nearest);
		const FSlateBrush* ThumbBrush = ResolveAccountReferenceRegionBrush(
			ThumbEntry,
			ThumbPath,
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			VerticalBarUV,
			FVector2D(16.f, 96.f),
			ESlateBrushDrawType::Box,
			FLinearColor(0.92f, 0.05f, 0.12f, 1.0f),
			TEXT("AccountScrollbarThumbV16"),
			TextureFilter::TF_Nearest);
		const FSlateBrush* HoverBrush = ResolveAccountReferenceRegionBrush(
			HoverEntry,
			ThumbPath,
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			VerticalBarUV,
			FVector2D(16.f, 96.f),
			ESlateBrushDrawType::Box,
			FLinearColor(1.0f, 0.16f, 0.22f, 1.0f),
			TEXT("AccountScrollbarThumbHoverV16"),
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

	const FSlateBrush* GetAccountDropdownFieldBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush HistoryEntry;
		return ResolveAccountReferenceBrush(
			IsAccountHistoryReferenceState() ? HistoryEntry : Entry,
			ResolveAccountStateAssetPath(
				TEXT("Controls/accountstatus_overview_dropdown_field_normal.png"),
				TEXT("Controls/accountstatus_controls_reference_dropdown_field_normal.png"),
				TEXT("SourceAssets/UI/Reference/Screens/AccountStatus/Controls/accountstatus_controls_reference_dropdown_field_normal.png")),
			FMargin(0.06f, 0.34f, 0.06f, 0.34f),
			TEXT("AccountFieldShell"));
	}

	const FSlateBrush* GetAccountDropdownChevronBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush HistoryEntry;
		return ResolveAccountReferenceBrush(
			IsAccountHistoryReferenceState() ? HistoryEntry : Entry,
			ResolveAccountStateAssetPath(
				TEXT("Icons/accountstatus_iconsgenerated_icon_16_dropdown_chevron_v2.png"),
				TEXT("Icons/accountstatus_iconsgenerated_icon_16_dropdown_chevron_v2.png"),
				TEXT("SourceAssets/UI/Reference/Screens/AccountStatus/Icons/accountstatus_iconsgenerated_icon_16_dropdown_chevron_v2.png")),
			FMargin(0.f),
			TEXT("AccountDropdownChevron"));
	}

	const FSlateBrush* GetAccountAvatarSlotBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush HistoryEntry;
		return ResolveAccountReferenceBrush(
			IsAccountHistoryReferenceState() ? HistoryEntry : Entry,
			MakeAccountMainMenuSquareElementPath(TEXT("profile_slot_selected_red_square_variant.png")),
			FMargin(0.205f, 0.205f, 0.205f, 0.205f),
			TEXT("AccountAvatarSlot"));
	}

	const FSlateBrush* GetAccountOverviewProgressTrackBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveAccountReferenceBrush(
			Entry,
			ResolveAccountStateAssetPath(
				TEXT("Progress/accountstatus_overview_progress_track.png"),
				FString(),
				TEXT("SourceAssets/UI/Reference/Shared/Progress/reference_progress_meter_sheet.png")),
			FMargin(0.08f, 0.34f, 0.08f, 0.34f),
			TEXT("AccountOverviewProgressTrack"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetAccountOverviewProgressFillBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveAccountReferenceBrush(
			Entry,
			ResolveAccountStateAssetPath(
				TEXT("Progress/accountstatus_overview_progress_fill.png"),
				FString(),
				TEXT("SourceAssets/UI/Reference/Shared/Progress/reference_progress_meter_sheet.png")),
			FMargin(0.08f, 0.34f, 0.08f, 0.34f),
			TEXT("AccountOverviewProgressFill"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetAccountHistoryTableHeaderBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveAccountReferenceBrush(
			Entry,
			T66ScreenSlateHelpers::MakeReferenceRedSquareButtonAssetPath(TEXT("normal")),
			FMargin(0.130f, 0.165f, 0.130f, 0.165f),
			TEXT("AccountHistoryTableHeader"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetAccountHistoryTableRowBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveAccountReferenceBrush(
			Entry,
			MakeAccountMainMenuLongPanelPath(TEXT("normal")),
			FMargin(0.055f, 0.210f, 0.055f, 0.210f),
			TEXT("AccountHistoryTableRow"),
			TextureFilter::TF_Nearest);
	}

	TSharedRef<SWidget> MakeAccountDropdownChevron(float Size = 18.f)
	{
		const FSlateBrush* ChevronBrush = GetAccountDropdownChevronBrush();
		if (!ChevronBrush)
		{
			return SNew(SBox)
				.WidthOverride(Size)
				.HeightOverride(Size)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Account", "DropdownChevronFallback", "v"))
					.Font(AccountBoldFont(FMath::RoundToInt(Size)))
					.ColorAndOpacity(AccountGold())
					.Justification(ETextJustify::Center)
				];
		}

		return SNew(SBox)
			.WidthOverride(Size)
			.HeightOverride(Size)
			[
				SNew(SImage)
				.Image(ChevronBrush)
				.ColorAndOpacity(ChevronBrush ? FLinearColor::White : AccountMutedText())
			];
	}

	TSharedRef<SWidget> MakeAccountReferenceButton(const FT66ButtonParams& Params)
	{
		const int32 FontSize = Params.FontSize > 0 ? Params.FontSize : 14;
		FSlateFontInfo ButtonFont = FT66Style::MakeFont(*Params.FontWeight, FontSize);
		ButtonFont.LetterSpacing = 0;

		const TAttribute<FText> ButtonText = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		const TAttribute<FSlateColor> TextColor = Params.bHasTextColorOverride
			? Params.TextColorOverride
			: TAttribute<FSlateColor>(FSlateColor(AccountChromeText()));
		const FMargin ContentPadding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(12.f, 6.f);

		const TSharedRef<SWidget> ButtonContent = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					.StretchDirection(EStretchDirection::DownOnly)
					[
						SNew(STextBlock)
						.Text(ButtonText)
						.Font(ButtonFont)
						.ColorAndOpacity(TextColor)
						.Justification(ETextJustify::Center)
						.ShadowOffset(FVector2D(0.f, 1.f))
						.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.70f))
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
				]);

		const FButtonStyle& ButtonStyle = GetAccountReferenceButtonStyle(Params.Type);
		return T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton(
			Params.OnClicked,
			ButtonContent,
			&ButtonStyle.Normal,
			&ButtonStyle.Hovered,
			&ButtonStyle.Pressed,
			&ButtonStyle.Disabled,
			Params.MinWidth,
			Params.Height,
			ContentPadding,
			Params.IsEnabled,
			Params.Visibility);
	}

	TSharedRef<SWidget> MakeAccountFieldShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		if (const FSlateBrush* FieldBrush = GetAccountDropdownFieldBrush())
		{
			return SNew(SBorder)
				.BorderImage(FieldBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel2)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetColor(AccountPanelInnerFill())
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeAccountRedBarPanel(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		if (const FSlateBrush* BarBrush = ResolveAccountReferenceBrush(
			Entry,
			T66ScreenSlateHelpers::MakeReferenceRedSquareButtonAssetPath(TEXT("normal")),
			FMargin(0.083f, 0.231f, 0.083f, 0.231f),
			TEXT("AccountRedBarPanel"),
			TextureFilter::TF_Nearest))
		{
			return SNew(SBorder)
				.BorderImage(BarBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel2)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetColor(AccountPanelInnerFill())
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeAccountProgressBarSized(const float Percent, const FVector2D& DesiredSize, const FLinearColor& FallbackFill);

	TSharedRef<SWidget> MakeAccountProgressBar(const float Percent, const float Height, const FLinearColor& FallbackFill)
	{
		return MakeAccountProgressBarSized(Percent, FVector2D(320.f, Height + 6.f), FallbackFill);
	}

	TSharedRef<SWidget> MakeAccountProgressBarSized(const float Percent, const FVector2D& DesiredSize, const FLinearColor& FallbackFill)
	{
		const float Pct = FMath::Clamp(Percent, 0.f, 1.f);
		const FSlateBrush* TrackBrush = GetAccountOverviewProgressTrackBrush();
		const FSlateBrush* FillBrush = GetAccountOverviewProgressFillBrush();
		if (TrackBrush && FillBrush)
		{
			return SNew(SBox)
				.WidthOverride(DesiredSize.X)
				.HeightOverride(DesiredSize.Y)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Image(TrackBrush)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					[
						SNew(SBox)
						.WidthOverride(DesiredSize.X * Pct)
						.HeightOverride(DesiredSize.Y)
						.Clipping(EWidgetClipping::ClipToBounds)
						[
							SNew(SImage)
							.Image(FillBrush)
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				];
		}

		return T66ScreenSlateHelpers::MakeReferenceProgressBar(
			Pct,
			DesiredSize,
			FallbackFill,
			FMargin(4.f, 3.f));
	}

	TSharedRef<SWidget> MakeAccountReferenceDropdown(const FT66DropdownParams& Params)
	{
		static FComboButtonStyle FlatComboStyle = []()
		{
			FComboButtonStyle Style = FT66Style::GetDropdownComboButtonStyle();
			Style.ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			return Style;
		}();

		TSharedRef<SComboButton> Combo = SNew(SComboButton)
			.ComboButtonStyle(&FlatComboStyle)
			.HasDownArrow(false)
			.OnGetMenuContent_Lambda([OnGet = Params.OnGetMenuContent]()
			{
				return OnGet();
			})
			.ContentPadding(Params.Padding)
			.ButtonContent()
			[
				Params.Content
			];

		return SNew(SBox)
			.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize())
			.Visibility(Params.Visibility)
			[
				MakeAccountFieldShell(Combo, FMargin(6.f, 5.f))
			];
	}

	FText RestrictionText(ET66AccountRestrictionKind Restriction)
	{
		switch (Restriction)
		{
		case ET66AccountRestrictionKind::Suspicion:
			return NSLOCTEXT("T66.Account", "RestrictionSuspicion", "Suspended");
		case ET66AccountRestrictionKind::CheatingCertainty:
			return NSLOCTEXT("T66.Account", "RestrictionCheating", "Restricted");
		case ET66AccountRestrictionKind::None:
		default:
			return NSLOCTEXT("T66.Account", "RestrictionGood", "Good Standing");
		}
	}

	FText AppealStatusText(ET66AppealReviewStatus Status)
	{
		switch (Status)
		{
		case ET66AppealReviewStatus::UnderReview:
			return NSLOCTEXT("T66.Account", "AppealUnderReview", "Appeal: Under Review");
		case ET66AppealReviewStatus::Denied:
			return NSLOCTEXT("T66.Account", "AppealDenied", "Appeal: Denied");
		case ET66AppealReviewStatus::Approved:
			return NSLOCTEXT("T66.Account", "AppealApproved", "Appeal: Approved");
		case ET66AppealReviewStatus::NotSubmitted:
		default:
			return NSLOCTEXT("T66.Account", "AppealNotSubmitted", "Appeal: Not Submitted");
		}
	}

	FText PartySizeText(UT66LocalizationSubsystem* Loc, ET66PartySize PartySize)
	{
		if (!Loc)
		{
			switch (PartySize)
			{
			case ET66PartySize::Duo:
				return NSLOCTEXT("T66.Account", "PartyDuoFallback", "Duo");
			case ET66PartySize::Trio:
				return NSLOCTEXT("T66.Account", "PartyTrioFallback", "Trio");
			case ET66PartySize::Quad:
				return NSLOCTEXT("T66.Account", "PartyQuadFallback", "Quad");
			case ET66PartySize::Solo:
			default:
				return NSLOCTEXT("T66.Account", "PartySoloFallback", "Solo");
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

	FText FormatDurationText(float TotalSeconds)
	{
		const int32 RoundedSeconds = FMath::Max(0, FMath::RoundToInt(TotalSeconds));
		const int32 Hours = RoundedSeconds / 3600;
		const int32 Minutes = (RoundedSeconds % 3600) / 60;
		const int32 Seconds = RoundedSeconds % 60;

		if (Hours > 0)
		{
			return FText::FromString(FString::Printf(TEXT("%d:%02d:%02d"), Hours, Minutes, Seconds));
		}

		return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
	}

	FString DifficultyToApiString(ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Medium: return TEXT("medium");
		case ET66Difficulty::Hard: return TEXT("hard");
		case ET66Difficulty::VeryHard: return TEXT("veryhard");
		case ET66Difficulty::Impossible: return TEXT("impossible");
		case ET66Difficulty::Easy:
		default:
			return TEXT("easy");
		}
	}

	FString PartySizeToApiString(ET66PartySize PartySize)
	{
		switch (PartySize)
		{
		case ET66PartySize::Duo: return TEXT("duo");
		case ET66PartySize::Trio: return TEXT("trio");
		case ET66PartySize::Quad: return TEXT("quad");
		case ET66PartySize::Solo:
		default:
			return TEXT("solo");
		}
	}

	TSharedRef<SWidget> MakeAccountPanel(const TSharedRef<SWidget>& Content, ET66PanelType Type, const FLinearColor& Color, const FMargin& Padding)
	{
		(void)Type;
		(void)Color;
		if (const FSlateBrush* ReferenceBrush = GetAccountPaperPanelBrush())
		{
			const FMargin ReferenceInset(8.f, 10.f, 8.f, 8.f);
			return SNew(SBorder)
				.BorderImage(ReferenceBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding + ReferenceInset)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(Type)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetColor(Color)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeSectionHeader(const FText& Text)
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(AccountBoldFont(18))
			.ColorAndOpacity(AccountText())
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds);
	}

	struct FPersonalBestDisplay
	{
		bool bHasRecord = false;
		bool bHasRankState = false;
		bool bRankRequestSucceeded = false;
		ET66PartySize PartySize = ET66PartySize::Solo;
		int32 Score = 0;
		float Seconds = 0.f;
		int32 GlobalRank = 0;
		FString RunSummarySlotName;
		FDateTime AchievedAtUtc;
	};

	struct FAccountNamedEntry
	{
		FName ID = NAME_None;
		FText DisplayName;
		FString SortKey;
	};
}

UT66AccountStatusScreen::UT66AccountStatusScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::AccountStatus;
	bIsModal = false;
}

void UT66AccountStatusScreen::ApplyAutomationAccountTabOverride()
{
	if (bAutomationTabOverrideApplied)
	{
		return;
	}

	bAutomationTabOverrideApplied = true;

	FString RequestedAccountTab;
	if (!FParse::Value(FCommandLine::Get(), TEXT("T66AccountTab="), RequestedAccountTab))
	{
		return;
	}

	if (RequestedAccountTab.Equals(TEXT("History"), ESearchCase::IgnoreCase))
	{
		ActiveTab = EAccountTab::History;
	}
	else if (RequestedAccountTab.Equals(TEXT("Suspension"), ESearchCase::IgnoreCase))
	{
		ActiveTab = EAccountTab::Suspension;
	}
	else if (RequestedAccountTab.Equals(TEXT("Overview"), ESearchCase::IgnoreCase))
	{
		ActiveTab = EAccountTab::Overview;
	}
}

void UT66AccountStatusScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	bAutomationTabOverrideApplied = false;
	ApplyAutomationAccountTabOverride();
	bAppealEditorOpen = false;
	bShowStandingInfoPopup = false;
	AppealDraftMessage.Reset();
	AppealSubmitStatusMessage.Reset();
	bAppealSubmitStatusIsError = false;
	bAppealSubmitInFlight = false;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			if (BackendMyRankReadyHandle.IsValid())
			{
				Backend->OnMyRankDataReady.Remove(BackendMyRankReadyHandle);
				BackendMyRankReadyHandle.Reset();
			}
			BackendMyRankReadyHandle = Backend->OnMyRankDataReady.AddUObject(this, &UT66AccountStatusScreen::HandleBackendMyRankDataReady);
			Backend->OnAppealSubmitComplete.RemoveDynamic(this, &UT66AccountStatusScreen::HandleBackendAppealSubmitComplete);
			Backend->OnAppealSubmitComplete.AddDynamic(this, &UT66AccountStatusScreen::HandleBackendAppealSubmitComplete);
		}
	}
}

void UT66AccountStatusScreen::OnScreenDeactivated_Implementation()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnMyRankDataReady.Remove(BackendMyRankReadyHandle);
			BackendMyRankReadyHandle.Reset();
			Backend->OnAppealSubmitComplete.RemoveDynamic(this, &UT66AccountStatusScreen::HandleBackendAppealSubmitComplete);
		}
	}

	bAppealSubmitInFlight = false;

	Super::OnScreenDeactivated_Implementation();
}

void UT66AccountStatusScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	RequestDeferredSlateRebuild();
}

void UT66AccountStatusScreen::HandleBackendMyRankDataReady(const FString& Key, bool bSuccess, int32 Rank, int32 TotalEntries)
{
	static_cast<void>(Key);
	static_cast<void>(bSuccess);
	static_cast<void>(Rank);
	static_cast<void>(TotalEntries);

	if (!HasBuiltSlateUI() || !IsVisible())
	{
		return;
	}

	RequestDeferredSlateRebuild();
}

TSharedRef<SWidget> UT66AccountStatusScreen::BuildSlateUI()
{
	ApplyAutomationAccountTabOverride();
	GAccountStatusReferenceStateFolder = ActiveTab == EAccountTab::History ? TEXT("History") : TEXT("Overview");
	UGameInstance* GIBase = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GIBase);
	UT66LocalizationSubsystem* Loc = GIBase ? GIBase->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GIBase ? GIBase->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66BackendSubsystem* Backend = GIBase ? GIBase->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	UT66AchievementsSubsystem* Achievements = GIBase ? GIBase->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66BuffSubsystem* Buffs = GIBase ? GIBase->GetSubsystem<UT66BuffSubsystem>() : nullptr;
	UT66CompanionUnlockSubsystem* CompanionUnlocks = GIBase ? GIBase->GetSubsystem<UT66CompanionUnlockSubsystem>() : nullptr;
	UT66SteamHelper* SteamHelper = GIBase ? GIBase->GetSubsystem<UT66SteamHelper>() : nullptr;
	const TWeakObjectPtr<UT66GameInstance> WeakT66GI = T66GI;
	const TWeakObjectPtr<UT66LocalizationSubsystem> WeakLoc = Loc;
	const TWeakObjectPtr<UT66LeaderboardSubsystem> WeakLB = LB;

	const bool bModalPresentation = UIManager && UIManager->GetCurrentModalType() == ScreenType;
	const T66ScreenSlateHelpers::FTopBarScreenLayoutMetrics ScreenLayout =
		T66ScreenSlateHelpers::MakeTopBarScreenLayoutMetrics(UIManager);
	const bool bUseStackedOverviewLayout = ScreenLayout.bStacked;
	const FT66AccountRestrictionRecord Restriction = LB ? LB->GetAccountRestrictionRecord() : FT66AccountRestrictionRecord();
	const bool bAccountEligible = LB ? LB->IsAccountEligibleForLeaderboard() : true;
	const bool bHasSuspension = Restriction.Restriction != ET66AccountRestrictionKind::None;
	if (!bHasSuspension && ActiveTab == EAccountTab::Suspension)
	{
		ActiveTab = EAccountTab::Overview;
	}
	const TArray<FName> HeroIDs = T66GI ? T66GI->GetAllHeroIDs() : TArray<FName>();
	const TArray<FName> CompanionIDs = T66GI ? T66GI->GetAllCompanionIDs() : TArray<FName>();
	const TArray<FAchievementData> AchievementDefs = Achievements ? Achievements->GetAllAchievements() : TArray<FAchievementData>();
	const TArray<FT66RecentRunRecord> RecentRuns = LB ? LB->GetRecentRuns() : TArray<FT66RecentRunRecord>();

	int32 UnlockedAchievements = 0;
	for (const FAchievementData& A : AchievementDefs) if (A.bIsUnlocked) { ++UnlockedAchievements; }

	const TArray<ET66HeroStatType> PowerStats = {
		ET66HeroStatType::Damage, ET66HeroStatType::AttackSpeed, ET66HeroStatType::AttackScale, ET66HeroStatType::Accuracy,
		ET66HeroStatType::Armor, ET66HeroStatType::Evasion, ET66HeroStatType::Luck, ET66HeroStatType::Speed
	};
	int32 UnlockedPowerUps = 0;
	for (const ET66HeroStatType StatType : PowerStats) { UnlockedPowerUps += Buffs ? Buffs->GetUnlockedFillStepCount(StatType) : 0; }

	int32 UnlockedCompanions = 0;
	for (const FName& CompanionID : CompanionIDs)
	{
		if (!CompanionUnlocks || CompanionUnlocks->IsCompanionUnlocked(CompanionID))
		{
			++UnlockedCompanions;
		}
	}

	const int32 TotalChallengeCount = 6;
	const int32 DisplayChallengesCompleted = 0;

	ProfileAvatarBrush = MakeShared<FSlateBrush>();
	ProfileAvatarBrush->DrawAs = ESlateBrushDrawType::Image;
	ProfileAvatarBrush->Tiling = ESlateBrushTileType::NoTile;
	ProfileAvatarBrush->ImageSize = FVector2D(70.f, 70.f);
	if (UTexture2D* LocalAvatarTexture = SteamHelper ? SteamHelper->GetLocalAvatarTexture() : nullptr)
	{
		ProfileAvatarBrush->SetResourceObject(LocalAvatarTexture);
	}
	else
	{
		ProfileAvatarBrush->SetResourceObject(nullptr);
	}

	const FString LocalSteamName = SteamHelper ? SteamHelper->GetLocalDisplayName() : FString();
	const FText ProfileNameText = !LocalSteamName.IsEmpty()
		? FText::FromString(LocalSteamName)
		: NSLOCTEXT("T66.Account", "ProfileNameFallback", "Local Player");
	const int32 ProfileLevel = Achievements ? Achievements->GetAccountLevel() : 1;
	const int32 ProfileMaxLevel = Achievements ? Achievements->GetAccountMaxLevel() : UT66AchievementsSubsystem::AccountMaxLevel;
	const int32 ProfileNextLevel = Achievements ? Achievements->GetAccountNextLevel() : FMath::Min(ProfileLevel + 1, ProfileMaxLevel);
	const int32 ProfileExperienceIntoLevel = Achievements ? Achievements->GetAccountExperienceIntoLevel() : 0;
	const int32 ProfileExperienceToNextLevel = Achievements ? Achievements->GetAccountExperienceToNextLevel() : UT66AchievementsSubsystem::AccountExperiencePerLevel;
	const FText ProfileLevelText = FText::Format(
		NSLOCTEXT("T66.Account", "ProfileLevelFormat", "Level {0}/{1}"),
		FText::AsNumber(ProfileLevel),
		FText::AsNumber(ProfileMaxLevel));
	const FText ProfileNextLevelText = FText::Format(
		NSLOCTEXT("T66.Account", "ProfileNextLevelFormat", "{0}/{1} XP to Level {2}"),
		FText::AsNumber(ProfileExperienceIntoLevel),
		FText::AsNumber(ProfileExperienceToNextLevel),
		FText::AsNumber(ProfileNextLevel));

	auto DifficultyText = [Loc](ET66Difficulty Difficulty) -> FText
	{
		if (Loc) return Loc->GetText_Difficulty(Difficulty);
		switch (Difficulty)
		{
		case ET66Difficulty::Easy: return NSLOCTEXT("T66.Account", "Easy", "Easy");
		case ET66Difficulty::Medium: return NSLOCTEXT("T66.Account", "Medium", "Medium");
		case ET66Difficulty::Hard: return NSLOCTEXT("T66.Account", "Hard", "Hard");
		case ET66Difficulty::VeryHard: return NSLOCTEXT("T66.Account", "VeryHard", "Very Hard");
		case ET66Difficulty::Impossible: default: return NSLOCTEXT("T66.Account", "Impossible", "Impossible");
		}
	};

	auto CompletionFilterText = [](EHistoryCompletionFilter Filter) -> FText
	{
		switch (Filter)
		{
		case EHistoryCompletionFilter::Completed:
			return NSLOCTEXT("T66.Account", "FilterCompleted", "Completed");
		case EHistoryCompletionFilter::NotCompleted:
			return NSLOCTEXT("T66.Account", "FilterNotCompleted", "Not Completed");
		case EHistoryCompletionFilter::All:
		default:
			return NSLOCTEXT("T66.Account", "FilterAllRuns", "All Runs");
		}
	};

	TArray<FAccountNamedEntry> HistoryHeroFilterEntries;
	for (const FName& HeroID : HeroIDs)
	{
		FHeroData HeroData;
		if (!T66GI || !T66GI->GetHeroData(HeroID, HeroData))
		{
			continue;
		}

		FAccountNamedEntry Entry;
		Entry.ID = HeroID;
		Entry.DisplayName = Loc ? Loc->GetText_HeroName(HeroID) : HeroData.DisplayName;
		Entry.SortKey = Entry.DisplayName.ToString();
		HistoryHeroFilterEntries.Add(Entry);
	}
	HistoryHeroFilterEntries.Sort([](const FAccountNamedEntry& A, const FAccountNamedEntry& B)
	{
		return A.SortKey < B.SortKey;
	});

	auto MakeTabButton = [this](
		const FText& Label,
		bool bActive,
		FReply (UT66AccountStatusScreen::*Handler)(),
		const FLinearColor& ActiveColor = FLinearColor::Transparent,
		const FLinearColor& InactiveColor = FLinearColor::Transparent,
		const FLinearColor& ActiveTextColor = FLinearColor::Transparent,
		const FLinearColor& InactiveTextColor = FLinearColor::Transparent) -> TSharedRef<SWidget>
	{
		(void)ActiveColor;
		(void)InactiveColor;
		const FLinearColor ResolvedActiveTextColor = ActiveTextColor == FLinearColor::Transparent ? AccountChromeText() : ActiveTextColor;
		const FLinearColor ResolvedInactiveTextColor = InactiveTextColor == FLinearColor::Transparent ? AccountChromeText() : InactiveTextColor;
		const T66ScreenSlateHelpers::FFrontendChromeMetrics& ChromeMetrics = T66ScreenSlateHelpers::GetFrontendChromeMetrics();

		return MakeAccountReferenceButton(
			FT66ButtonParams(Label, FOnClicked::CreateUObject(this, Handler), bActive ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
			.SetFontSize(ChromeMetrics.TabFontSize)
			.SetMinWidth(ChromeMetrics.TabMinWidth)
			.SetHeight(ChromeMetrics.TabHeight)
			.SetPadding(ChromeMetrics.TabPadding)
			.SetTextColor(bActive ? ResolvedActiveTextColor : ResolvedInactiveTextColor));
	};

	int32 AccountProgressPreviewIndex = 0;
	auto MakeProgressRow = [&](const FText& Label, int32 Current, int32 Total, const FLinearColor& Fill) -> TSharedRef<SWidget>
	{
		const float Pct = AccountPreviewProgress01(AccountProgressPreviewIndex++);
		const TSharedRef<SWidget> ProgressBar =
			MakeAccountProgressBar(Pct, 7.f, Fill);

		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(AccountBoldFont(11))
					.ColorAndOpacity(AccountText())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(STextBlock)
					.Text(FText::Format(NSLOCTEXT("T66.Account", "CountFmt", "{0}/{1}"), FText::AsNumber(Current), FText::AsNumber(Total)))
					.Font(AccountBoldFont(11))
					.ColorAndOpacity(AccountGold())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
			[
				ProgressBar
			];
	};

	auto MakeProfileLevelPanel = [&]() -> TSharedRef<SWidget>
	{
		constexpr float PreviewExperienceProgress = 0.58f;
		const float Pct = PreviewExperienceProgress;
		const FSlateBrush* AvatarSlotBrush = GetAccountAvatarSlotBrush();

		const TSharedRef<SWidget> LevelBar =
			MakeAccountProgressBar(Pct, 12.f, FLinearColor(0.92f, 0.05f, 0.12f, 1.0f));

		return MakeAccountPanel(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(86.f)
				.HeightOverride(86.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Image(AvatarSlotBrush)
						.ColorAndOpacity(AvatarSlotBrush ? FLinearColor::White : FLinearColor(0.08f, 0.08f, 0.10f, 1.0f))
					]
					+ SOverlay::Slot()
					.Padding(FMargin(8.f))
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
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(16.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(ProfileNameText)
					.Font(AccountBoldFont(19))
					.ColorAndOpacity(AccountText())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(ProfileLevelText)
					.Font(AccountBoldFont(13))
					.ColorAndOpacity(AccountGold())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Account", "ExperienceLabel", "Experience"))
						.Font(AccountBoldFont(11))
						.ColorAndOpacity(AccountText())
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(ProfileNextLevelText)
						.Font(AccountRegularFont(10))
						.ColorAndOpacity(AccountMutedText())
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
				[
					LevelBar
				]
			],
			ET66PanelType::Panel2,
			AccountPanelInnerFill(),
			FMargin(14.f, 12.f));
	};

	auto GetMyRankKey = [this](ET66LeaderboardType Type, ET66Difficulty Difficulty) -> FString
	{
		return UT66BackendSubsystem::MakeMyRankCacheKey(
			Type == ET66LeaderboardType::Score ? TEXT("score") : TEXT("speedrun"),
			TEXT("alltime"),
			PartySizeToApiString(ActivePBPartySize),
			DifficultyToApiString(Difficulty));
	};

	auto PrimeMyRankRequest = [this, Backend, &GetMyRankKey](ET66LeaderboardType Type, ET66Difficulty Difficulty)
	{
		if (!Backend || !Backend->IsBackendConfigured() || !Backend->HasSteamTicket())
		{
			return;
		}

		const FString RankKey = GetMyRankKey(Type, Difficulty);
		if (!Backend->HasCachedMyRank(RankKey))
		{
			Backend->FetchMyRank(
				Type == ET66LeaderboardType::Score ? TEXT("score") : TEXT("speedrun"),
				TEXT("alltime"),
				PartySizeToApiString(ActivePBPartySize),
				DifficultyToApiString(Difficulty));
		}
	};

	auto ResolveMyRank = [Backend, &GetMyRankKey](ET66LeaderboardType Type, ET66Difficulty Difficulty, bool& bOutHasRankState, bool& bOutRankSuccess, int32& OutRank) -> void
	{
		bOutHasRankState = false;
		bOutRankSuccess = false;
		OutRank = 0;

		if (!Backend)
		{
			return;
		}

		const FString RankKey = GetMyRankKey(Type, Difficulty);
		int32 TotalEntries = 0;
		bOutHasRankState = Backend->GetCachedMyRank(RankKey, bOutRankSuccess, OutRank, TotalEntries);
	};

	auto MakePBScore = [&](ET66Difficulty Difficulty) -> FPersonalBestDisplay
	{
		FPersonalBestDisplay Out;
		if (!LB) return Out;
		FT66LocalScoreRecord R;
		if (LB->GetLocalBestScoreRecord(Difficulty, ActivePBPartySize, R))
		{
			Out.bHasRecord = true;
			Out.PartySize = ActivePBPartySize;
			if (ActivePBViewMode == EPersonalBestViewMode::HighestRank && R.BestRankAllTime > 0)
			{
				Out.Score = R.BestRankScore;
				Out.RunSummarySlotName = R.BestRankRunSummarySlotName;
				Out.AchievedAtUtc = R.BestRankAchievedAtUtc;
				Out.bHasRankState = true;
				Out.bRankRequestSucceeded = true;
				Out.GlobalRank = R.BestRankAllTime;
			}
			else
			{
				Out.Score = R.BestScore;
				Out.RunSummarySlotName = R.RunSummarySlotName;
				Out.AchievedAtUtc = R.AchievedAtUtc;
				if (ActivePBViewMode == EPersonalBestViewMode::PersonalBest && R.BestScoreRankAllTime > 0)
				{
					Out.bHasRankState = true;
					Out.bRankRequestSucceeded = true;
					Out.GlobalRank = R.BestScoreRankAllTime;
				}
				else
				{
					PrimeMyRankRequest(ET66LeaderboardType::Score, Difficulty);
					bool bHasRankState = false;
					bool bRankSuccess = false;
					int32 Rank = 0;
					ResolveMyRank(ET66LeaderboardType::Score, Difficulty, bHasRankState, bRankSuccess, Rank);
					Out.bHasRankState = bHasRankState;
					Out.bRankRequestSucceeded = bRankSuccess;
					Out.GlobalRank = (bHasRankState && bRankSuccess) ? Rank : 0;
				}
			}
		}
		return Out;
	};

	auto MakePBTime = [&](ET66Difficulty Difficulty) -> FPersonalBestDisplay
	{
		FPersonalBestDisplay Out;
		if (!LB) return Out;
		FT66LocalCompletedRunTimeRecord R;
		if (LB->GetLocalBestCompletedRunTimeRecord(Difficulty, ActivePBPartySize, R))
		{
			Out.bHasRecord = true;
			Out.PartySize = ActivePBPartySize;
			if (ActivePBViewMode == EPersonalBestViewMode::HighestRank && R.BestRankAllTime > 0)
			{
				Out.Seconds = R.BestRankCompletedSeconds;
				Out.RunSummarySlotName = R.BestRankRunSummarySlotName;
				Out.AchievedAtUtc = R.BestRankAchievedAtUtc;
				Out.bHasRankState = true;
				Out.bRankRequestSucceeded = true;
				Out.GlobalRank = R.BestRankAllTime;
			}
			else
			{
				Out.Seconds = R.BestCompletedSeconds;
				Out.RunSummarySlotName = R.RunSummarySlotName;
				Out.AchievedAtUtc = R.AchievedAtUtc;
				if (ActivePBViewMode == EPersonalBestViewMode::PersonalBest && R.BestCompletedRankAllTime > 0)
				{
					Out.bHasRankState = true;
					Out.bRankRequestSucceeded = true;
					Out.GlobalRank = R.BestCompletedRankAllTime;
				}
				else
				{
					PrimeMyRankRequest(ET66LeaderboardType::SpeedRun, Difficulty);
					bool bHasRankState = false;
					bool bRankSuccess = false;
					int32 Rank = 0;
					ResolveMyRank(ET66LeaderboardType::SpeedRun, Difficulty, bHasRankState, bRankSuccess, Rank);
					Out.bHasRankState = bHasRankState;
					Out.bRankRequestSucceeded = bRankSuccess;
					Out.GlobalRank = (bHasRankState && bRankSuccess) ? Rank : 0;
				}
			}
		}
		return Out;
	};

	TMap<FString, FName> PBHeroIdBySlot;
	auto ResolvePBHeroID = [&PBHeroIdBySlot](const FString& SlotName) -> FName
	{
		if (SlotName.IsEmpty())
		{
			return NAME_None;
		}

		if (const FName* CachedHeroID = PBHeroIdBySlot.Find(SlotName))
		{
			return *CachedHeroID;
		}

		FName HeroID = NAME_None;
		if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		{
			if (USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(SlotName, 0))
			{
				if (const UT66LeaderboardRunSummarySaveGame* Summary = Cast<UT66LeaderboardRunSummarySaveGame>(SaveGame))
				{
					HeroID = Summary->HeroID;
				}
			}
		}

		PBHeroIdBySlot.Add(SlotName, HeroID);
		return HeroID;
	};

	auto HeroText = [T66GI, Loc](FName HeroID) -> FText
	{
		if (HeroID.IsNone())
		{
			return FText::GetEmpty();
		}

		FHeroData HeroData;
		if (T66GI && T66GI->GetHeroData(HeroID, HeroData))
		{
			return Loc ? Loc->GetText_HeroName(HeroID) : HeroData.DisplayName;
		}

		return FText::FromName(HeroID);
	};

	auto GetHeroHistoryFilterText = [this, WeakT66GI, WeakLoc]() -> FText
	{
		if (HistoryHeroFilter.IsNone())
		{
			return NSLOCTEXT("T66.Account", "AllHeroes", "All Heroes");
		}

		UT66GameInstance* RuntimeGI = WeakT66GI.Get();
		UT66LocalizationSubsystem* RuntimeLoc = WeakLoc.Get();
		FHeroData HeroData;
		if (RuntimeGI && RuntimeGI->GetHeroData(HistoryHeroFilter, HeroData))
		{
			return RuntimeLoc ? RuntimeLoc->GetText_HeroName(HistoryHeroFilter) : HeroData.DisplayName;
		}

		return FText::FromName(HistoryHeroFilter);
	};

	auto GetDifficultyHistoryFilterText = [this, DifficultyText]() -> FText
	{
		return HistoryDifficultyFilter.IsSet()
			? DifficultyText(HistoryDifficultyFilter.GetValue())
			: NSLOCTEXT("T66.Account", "AllDifficulties", "All Difficulties");
	};

	auto GetPartySizeHistoryFilterText = [this, Loc]() -> FText
	{
		return HistoryPartySizeFilter.IsSet()
			? PartySizeText(Loc, HistoryPartySizeFilter.GetValue())
			: NSLOCTEXT("T66.Account", "AllPartySizes", "All Party Sizes");
	};

	auto GetCompletionHistoryFilterText = [this, CompletionFilterText]() -> FText
	{
		return CompletionFilterText(HistoryCompletionFilter);
	};

	auto MakeHistoryFilterMenuEntry = [this](const FText& Label, bool bActive, TFunction<void()> OnSelected) -> TSharedRef<SWidget>
	{
		return MakeAccountReferenceButton(
			FT66ButtonParams(
				Label,
				FOnClicked::CreateLambda([this, OnSelected]()
				{
					OnSelected();
					RequestDeferredSlateRebuild();
					FSlateApplication::Get().DismissAllMenus();
					return FReply::Handled();
				}),
				bActive ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
			.SetFontSize(AdjustAccountFontSize(11))
			.SetMinWidth(180.f)
			.SetHeight(32.f)
			.SetPadding(FMargin(10.f, 5.f, 10.f, 4.f))
			.SetTextColor(AccountChromeText()));
	};

	auto MakeHistoryFilterDropdown = [&](const FText& Label, TFunction<FText()> GetValueText, TFunction<TSharedRef<SWidget>()> MakeMenu, float MinWidth) -> TSharedRef<SWidget>
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(AccountRegularFont(9))
				.ColorAndOpacity(AccountMutedText())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
			[
				MakeAccountReferenceDropdown(
					FT66DropdownParams(
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text_Lambda([GetValueText]() { return GetValueText(); })
							.Font(AccountRegularFont(11))
							.ColorAndOpacity(AccountChromeText())
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
						[
							MakeAccountDropdownChevron(15.f)
						],
						MoveTemp(MakeMenu))
					.SetMinWidth(MinWidth)
					.SetHeight(42.f)
					.SetPadding(FMargin(14.f, 9.f, 14.f, 8.f)))
			];
	};

	auto MakeHeroHistoryFilterMenu = [this, HistoryHeroFilterEntries, MakeHistoryFilterMenuEntry]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			MakeHistoryFilterMenuEntry(
				NSLOCTEXT("T66.Account", "AllHeroesMenu", "All Heroes"),
				HistoryHeroFilter.IsNone(),
				[this]() { HistoryHeroFilter = NAME_None; })
		];

		for (const FAccountNamedEntry& Entry : HistoryHeroFilterEntries)
		{
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeHistoryFilterMenuEntry(
					Entry.DisplayName,
					HistoryHeroFilter == Entry.ID,
					[this, HeroID = Entry.ID]() { HistoryHeroFilter = HeroID; })
			];
		}

		return MakeAccountPanel(Menu, ET66PanelType::Panel2, AccountPanelFill(), FMargin(6.f));
	};

	auto MakeDifficultyHistoryFilterMenu = [this, DifficultyText, MakeHistoryFilterMenuEntry]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			MakeHistoryFilterMenuEntry(
				NSLOCTEXT("T66.Account", "AllDifficultiesMenu", "All Difficulties"),
				!HistoryDifficultyFilter.IsSet(),
				[this]() { HistoryDifficultyFilter.Reset(); })
		];

		for (ET66Difficulty Difficulty : { ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible })
		{
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeHistoryFilterMenuEntry(
					DifficultyText(Difficulty),
					HistoryDifficultyFilter.IsSet() && HistoryDifficultyFilter.GetValue() == Difficulty,
					[this, Difficulty]() { HistoryDifficultyFilter = Difficulty; })
			];
		}

		return MakeAccountPanel(Menu, ET66PanelType::Panel2, AccountPanelFill(), FMargin(6.f));
	};

	auto MakePartySizeHistoryFilterMenu = [this, Loc, MakeHistoryFilterMenuEntry]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			MakeHistoryFilterMenuEntry(
				NSLOCTEXT("T66.Account", "AllPartySizesMenu", "All Party Sizes"),
				!HistoryPartySizeFilter.IsSet(),
				[this]() { HistoryPartySizeFilter.Reset(); })
		];

		for (ET66PartySize PartySize : { ET66PartySize::Solo, ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad })
		{
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeHistoryFilterMenuEntry(
					PartySizeText(Loc, PartySize),
					HistoryPartySizeFilter.IsSet() && HistoryPartySizeFilter.GetValue() == PartySize,
					[this, PartySize]() { HistoryPartySizeFilter = PartySize; })
			];
		}

		return MakeAccountPanel(Menu, ET66PanelType::Panel2, AccountPanelFill(), FMargin(6.f));
	};

	auto MakeCompletionHistoryFilterMenu = [this, CompletionFilterText, MakeHistoryFilterMenuEntry]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		for (EHistoryCompletionFilter Filter : { EHistoryCompletionFilter::All, EHistoryCompletionFilter::Completed, EHistoryCompletionFilter::NotCompleted })
		{
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeHistoryFilterMenuEntry(
					CompletionFilterText(Filter),
					HistoryCompletionFilter == Filter,
					[this, Filter]() { HistoryCompletionFilter = Filter; })
			];
		}

		return MakeAccountPanel(Menu, ET66PanelType::Panel2, AccountPanelFill(), FMargin(6.f));
	};

	auto MakePBPartySizeMenu = [this, Loc, MakeHistoryFilterMenuEntry]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		for (ET66PartySize PartySize : { ET66PartySize::Solo, ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad })
		{
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeHistoryFilterMenuEntry(
					PartySizeText(Loc, PartySize),
					ActivePBPartySize == PartySize,
					[this, PartySize]() { ActivePBPartySize = PartySize; })
			];
		}

		return MakeAccountPanel(Menu, ET66PanelType::Panel2, AccountPanelFill(), FMargin(6.f));
	};

	auto GetPBViewModeText = [this]() -> FText
	{
		return ActivePBViewMode == EPersonalBestViewMode::HighestRank
			? NSLOCTEXT("T66.Account", "PBViewHighestRank", "Highest Rank")
			: NSLOCTEXT("T66.Account", "PBViewPersonalBest", "Personal Best");
	};

	auto MakePBViewModeMenu = [this, MakeHistoryFilterMenuEntry]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			MakeHistoryFilterMenuEntry(
				NSLOCTEXT("T66.Account", "PBViewPersonalBest", "Personal Best"),
				ActivePBViewMode == EPersonalBestViewMode::PersonalBest,
				[this]() { ActivePBViewMode = EPersonalBestViewMode::PersonalBest; })
		];
		Menu->AddSlot().AutoHeight()
		[
			MakeHistoryFilterMenuEntry(
				NSLOCTEXT("T66.Account", "PBViewHighestRank", "Highest Rank"),
				ActivePBViewMode == EPersonalBestViewMode::HighestRank,
				[this]() { ActivePBViewMode = EPersonalBestViewMode::HighestRank; })
		];
		return MakeAccountPanel(Menu, ET66PanelType::Panel2, AccountPanelFill(), FMargin(6.f));
	};

	auto MakePBViewModeDropdown = [this, GetPBViewModeText, MakePBViewModeMenu]() -> TSharedRef<SWidget>
	{
		return MakeAccountReferenceDropdown(
			FT66DropdownParams(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([GetPBViewModeText]() { return GetPBViewModeText(); })
					.Font(AccountRegularFont(14))
					.ColorAndOpacity(AccountChromeText())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
				[
					MakeAccountDropdownChevron(18.f)
				],
				MakePBViewModeMenu)
			.SetMinWidth(0.f)
			.SetHeight(0.f)
			.SetPadding(FMargin(12.f, 9.f, 12.f, 8.f)));
	};

	auto MakePBPartySizeDropdown = [this, Loc, MakePBPartySizeMenu]() -> TSharedRef<SWidget>
	{
		return MakeAccountReferenceDropdown(
			FT66DropdownParams(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([this, Loc]() { return PartySizeText(Loc, ActivePBPartySize); })
					.Font(AccountRegularFont(14))
					.ColorAndOpacity(AccountChromeText())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
				[
					MakeAccountDropdownChevron(18.f)
				],
				MakePBPartySizeMenu)
			.SetMinWidth(0.f)
			.SetHeight(0.f)
			.SetPadding(FMargin(12.f, 9.f, 12.f, 8.f)));
	};

	auto DoesRunMatchHistoryFilters = [&](const FT66RecentRunRecord& Run) -> bool
	{
		if (!HistoryHeroFilter.IsNone() && Run.HeroID != HistoryHeroFilter)
		{
			return false;
		}

		if (HistoryDifficultyFilter.IsSet() && Run.Difficulty != HistoryDifficultyFilter.GetValue())
		{
			return false;
		}

		if (HistoryPartySizeFilter.IsSet() && Run.PartySize != HistoryPartySizeFilter.GetValue())
		{
			return false;
		}

		switch (HistoryCompletionFilter)
		{
		case EHistoryCompletionFilter::Completed:
			return Run.bWasFullClear;
		case EHistoryCompletionFilter::NotCompleted:
			return !Run.bWasFullClear;
		case EHistoryCompletionFilter::All:
		default:
			return true;
		}
	};

	TSharedRef<SVerticalBox> HistoryRows = SNew(SVerticalBox);
	int32 FilteredRunCount = 0;
	int32 VisibleHistoryIndex = 0;
	for (const FT66RecentRunRecord& Run : RecentRuns)
	{
		if (!DoesRunMatchHistoryFilters(Run))
		{
			continue;
		}

		++FilteredRunCount;
		FHeroData HeroData;
		FCompanionData CompanionData;
		const bool bHasHeroData = T66GI && T66GI->GetHeroData(Run.HeroID, HeroData);
		const FText HeroName = bHasHeroData ? (Loc ? Loc->GetText_HeroName(Run.HeroID) : HeroData.DisplayName) : FText::FromName(Run.HeroID);
		const FText CompanionName = (T66GI && T66GI->GetCompanionData(Run.CompanionID, CompanionData))
			? (Loc ? Loc->GetText_CompanionName(Run.CompanionID) : CompanionData.DisplayName)
			: (Run.CompanionID.IsNone() ? NSLOCTEXT("T66.Account", "NoComp", "No Companion") : FText::FromName(Run.CompanionID));
		const FText StatusText = Run.bWasFullClear ? NSLOCTEXT("T66.Account", "CompletedRun", "Completed") : NSLOCTEXT("T66.Account", "NotCompletedRun", "Not Completed");
		const FLinearColor StatusColor = Run.bWasFullClear ? AccountSuccess() : AccountDanger();
		const FText RunDetailsText = FText::Format(NSLOCTEXT("T66.Account", "RunDetailsFmt", "{0} / {1}"), DifficultyText(Run.Difficulty), PartySizeText(Loc, Run.PartySize));
		const FText SublineText = FText::Format(NSLOCTEXT("T66.Account", "HistorySublineFmt", "{0} / Stage {1}"), CompanionName, FText::AsNumber(Run.StageReached));
		const bool bCanOpen = !Run.RunSummarySlotName.IsEmpty();
		const FSlateBrush* PortraitBrush = GetOrCreateHeroPortraitBrush(T66GI, Run.HeroID);
		const FString HeroInitial = !HeroName.IsEmptyOrWhitespace() ? HeroName.ToString().Left(1).ToUpper() : FString(TEXT("?"));

		HistoryRows->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
		[
			FT66Style::MakeButton(
				FT66ButtonParams(FText::GetEmpty(), bCanOpen ? FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOpenRunSummaryClicked, Run.RunSummarySlotName) : FOnClicked::CreateLambda([]() { return FReply::Handled(); }), ET66ButtonType::Row)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetPadding(FMargin(0.f))
				.SetEnabled(TAttribute<bool>(bCanOpen))
				.SetUseGlow(false)
				.SetContent(
					MakeAccountPanel(
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.75f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(32.f)
								.HeightOverride(32.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(bHasHeroData ? HeroData.PlaceholderColor : AccountMutedGold())
									.Padding(0.f)
									[
										PortraitBrush
											? StaticCastSharedRef<SWidget>(SNew(SImage).Image(PortraitBrush))
											: StaticCastSharedRef<SWidget>(
												SNew(STextBlock)
												.Text(FText::FromString(HeroInitial))
												.Font(AccountBoldFont(14))
												.ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
												.Justification(ETextJustify::Center))
									]
								]
							]
							+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SNew(STextBlock)
									.Text(HeroName)
									.Font(AccountBoldFont(12))
									.ColorAndOpacity(AccountText())
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
									.Clipping(EWidgetClipping::ClipToBounds)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(SublineText)
									.Font(AccountRegularFont(10))
									.ColorAndOpacity(AccountMutedText())
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
									.Clipping(EWidgetClipping::ClipToBounds)
								]
							]
						]
						+ SHorizontalBox::Slot().FillWidth(0.95f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(StatusText)
							.Font(AccountBoldFont(12))
							.ColorAndOpacity(StatusColor)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						]
						+ SHorizontalBox::Slot().FillWidth(1.10f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Run.EndedAtUtc.ToString(TEXT("%m/%d/%Y %H:%M"))))
							.Font(AccountRegularFont(11))
							.ColorAndOpacity(AccountText())
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						]
						+ SHorizontalBox::Slot().FillWidth(0.85f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FormatDurationText(Run.DurationSeconds))
							.Font(AccountBoldFont(12))
							.ColorAndOpacity(AccountText())
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						]
						+ SHorizontalBox::Slot().FillWidth(1.10f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(RunDetailsText)
							.Font(AccountRegularFont(11))
							.ColorAndOpacity(AccountMutedText())
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						],
						ET66PanelType::Panel2,
						VisibleHistoryIndex % 2 == 0 ? AccountRowFill() : AccountRowAltFill(),
						FMargin(8.f, 6.f, 8.f, 5.f))))
		];

		++VisibleHistoryIndex;
	}

	const bool bHasHistoryFilters = !HistoryHeroFilter.IsNone()
		|| HistoryDifficultyFilter.IsSet()
		|| HistoryPartySizeFilter.IsSet()
		|| HistoryCompletionFilter != EHistoryCompletionFilter::All;
	if (FilteredRunCount == 0)
	{
		HistoryRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(RecentRuns.Num() == 0
				? NSLOCTEXT("T66.Account", "NoRuns", "No runs have been recorded yet.")
				: (bHasHistoryFilters
					? NSLOCTEXT("T66.Account", "NoFilteredRuns", "No runs match the current filters.")
					: NSLOCTEXT("T66.Account", "NoRunsFallback", "No runs have been recorded yet.")))
			.Font(AccountRegularFont(13))
			.ColorAndOpacity(AccountMutedText())
		];
	}

	const TSharedRef<SWidget> HistoryFilterBar =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(0.34f).Padding(0.f, 0.f, 8.f, 0.f)
		[
			MakeHistoryFilterDropdown(
				NSLOCTEXT("T66.Account", "HistoryHeroFilterLabel", "Hero"),
				GetHeroHistoryFilterText,
				MakeHeroHistoryFilterMenu,
				180.f)
		]
		+ SHorizontalBox::Slot().FillWidth(0.22f).Padding(0.f, 0.f, 8.f, 0.f)
		[
			MakeHistoryFilterDropdown(
				NSLOCTEXT("T66.Account", "HistoryDifficultyFilterLabel", "Difficulty"),
				GetDifficultyHistoryFilterText,
				MakeDifficultyHistoryFilterMenu,
				160.f)
		]
		+ SHorizontalBox::Slot().FillWidth(0.22f).Padding(0.f, 0.f, 8.f, 0.f)
		[
			MakeHistoryFilterDropdown(
				NSLOCTEXT("T66.Account", "HistoryPartySizeFilterLabel", "Party Size"),
				GetPartySizeHistoryFilterText,
				MakePartySizeHistoryFilterMenu,
				160.f)
		]
		+ SHorizontalBox::Slot().FillWidth(0.22f)
		[
			MakeHistoryFilterDropdown(
				NSLOCTEXT("T66.Account", "HistoryStatusFilterLabel", "Status"),
				GetCompletionHistoryFilterText,
				MakeCompletionHistoryFilterMenu,
				160.f)
		];

	const TSharedRef<SWidget> HistoryColumnHeader = MakeAccountPanel(
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.75f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "HistColHero", "HERO PLAYED")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)]
		+ SHorizontalBox::Slot().FillWidth(0.95f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "HistColStatus", "STATUS")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)]
		+ SHorizontalBox::Slot().FillWidth(1.10f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "HistColDate", "DATE / TIME")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)]
		+ SHorizontalBox::Slot().FillWidth(0.85f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "HistColDuration", "DURATION")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)]
		+ SHorizontalBox::Slot().FillWidth(1.10f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "HistColRun", "RUN")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)],
		ET66PanelType::Panel2, AccountHeaderFill(), FMargin(8.f, 6.f, 8.f, 5.f));

	auto MakeAccountTableRow = [](const TSharedRef<SWidget>& RowContent, const FLinearColor& BackgroundColor, const FMargin& RowPadding) -> TSharedRef<SWidget>
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(1.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BackgroundColor)
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.f))
				.Padding(RowPadding)
				[
					RowContent
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(1.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BackgroundColor)
				]
			];
	};

	auto MakePBBlock = [&](const FText& Title, bool bTime) -> TSharedRef<SWidget>
	{
		const bool bRankInThirdColumn = ActivePBViewMode == EPersonalBestViewMode::PersonalBest;
		const FText ValueHeaderText = bTime
			? NSLOCTEXT("T66.Account", "PBColTime", "TIME")
			: NSLOCTEXT("T66.Account", "PBColScore", "SCORE");
		const FText RankHeaderText = NSLOCTEXT("T66.Account", "PBColRank", "GLOBAL RANK");
		TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
		for (ET66Difficulty Difficulty : { ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible })
		{
			const FPersonalBestDisplay PB = bTime ? MakePBTime(Difficulty) : MakePBScore(Difficulty);
			const bool bCanOpen = PB.bHasRecord && !PB.RunSummarySlotName.IsEmpty();
			const FName PBHeroID = PB.bHasRecord ? ResolvePBHeroID(PB.RunSummarySlotName) : NAME_None;
			const FText HeroName = PB.bHasRecord
				? (PBHeroID.IsNone()
					? NSLOCTEXT("T66.Account", "PBUnknownHero", "Unknown")
					: HeroText(PBHeroID))
				: FText::GetEmpty();
			const FText Value = PB.bHasRecord ? (bTime ? FormatDurationText(PB.Seconds) : FText::AsNumber(PB.Score)) : NSLOCTEXT("T66.Account", "NoRecord", "--");
			const FText Date = PB.bHasRecord && PB.AchievedAtUtc.GetTicks() > 0 ? FText::FromString(PB.AchievedAtUtc.ToString(TEXT("%m/%d/%Y"))) : FText::GetEmpty();
			FText RankText = FText::GetEmpty();
			if (PB.bHasRecord)
			{
				if (!Backend || !Backend->IsBackendConfigured() || !Backend->HasSteamTicket())
				{
					RankText = NSLOCTEXT("T66.Account", "RankUnavailable", "N/A");
				}
				else if (!PB.bHasRankState)
				{
					RankText = NSLOCTEXT("T66.Account", "RankPending", "...");
				}
				else if (!PB.bRankRequestSucceeded)
				{
					RankText = NSLOCTEXT("T66.Account", "RankFailed", "N/A");
				}
				else if (PB.GlobalRank > 0)
				{
					RankText = FText::Format(NSLOCTEXT("T66.Account", "RankFmt", "#{0}"), FText::AsNumber(PB.GlobalRank));
				}
				else
				{
					RankText = NSLOCTEXT("T66.Account", "RankUnranked", "Unranked");
				}
			}

			const FText ThirdColumnText = bRankInThirdColumn ? RankText : Value;
			const FText FourthColumnText = bRankInThirdColumn ? Value : RankText;
			const FLinearColor ThirdColumnColor = bRankInThirdColumn ? AccountText() : (PB.bHasRecord ? AccountGold() : AccountMutedText());
			const FLinearColor FourthColumnColor = bRankInThirdColumn ? (PB.bHasRecord ? AccountGold() : AccountMutedText()) : AccountText();
			const FSlateFontInfo ThirdColumnFont = bRankInThirdColumn ? AccountBoldFont(12) : AccountBoldFont(15);
			const FSlateFontInfo FourthColumnFont = bRankInThirdColumn ? AccountBoldFont(15) : AccountBoldFont(12);

			Rows->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 1.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(FText::GetEmpty(), bCanOpen ? FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOpenRunSummaryClicked, PB.RunSummarySlotName) : FOnClicked::CreateLambda([]() { return FReply::Handled(); }), ET66ButtonType::Row)
					.SetBorderVisual(ET66ButtonBorderVisual::None).SetBackgroundVisual(ET66ButtonBackgroundVisual::None).SetPadding(FMargin(0.f)).SetEnabled(TAttribute<bool>(bCanOpen)).SetUseGlow(false)
					.SetContent(
						MakeAccountTableRow(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(0.90f).VAlign(VAlign_Center)[SNew(STextBlock).Text(DifficultyText(Difficulty)).Font(AccountBoldFont(11)).ColorAndOpacity(AccountText()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)]
							+ SHorizontalBox::Slot().FillWidth(1.00f).VAlign(VAlign_Center)[SNew(STextBlock).Text(HeroName).Font(AccountRegularFont(10)).ColorAndOpacity(PB.bHasRecord ? AccountText() : AccountMutedText()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)]
							+ SHorizontalBox::Slot().FillWidth(0.95f).VAlign(VAlign_Center)[SNew(STextBlock).Text(Date).Font(AccountRegularFont(10)).ColorAndOpacity(AccountMutedText()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)]
							+ SHorizontalBox::Slot().FillWidth(0.95f).VAlign(VAlign_Center)[SNew(STextBlock).Text(ThirdColumnText).Font(ThirdColumnFont).ColorAndOpacity(ThirdColumnColor).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)]
							+ SHorizontalBox::Slot().FillWidth(1.00f).VAlign(VAlign_Center)[SNew(STextBlock).Text(FourthColumnText).Font(FourthColumnFont).ColorAndOpacity(FourthColumnColor).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)],
							AccountRowFill(), FMargin(8.f, 4.f, 8.f, 3.f))))
			];
		}
		return MakeAccountPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[SNew(STextBlock).Text(Title).Font(AccountBoldFont(16)).ColorAndOpacity(AccountText())]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)
			[
				MakeAccountTableRow(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.90f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "PBColDifficulty", "DIFFICULTY")).Font(AccountBoldFont(11)).ColorAndOpacity(AccountText()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)]
					+ SHorizontalBox::Slot().FillWidth(1.00f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "PBColHero", "HERO")).Font(AccountBoldFont(11)).ColorAndOpacity(AccountText()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)]
					+ SHorizontalBox::Slot().FillWidth(0.95f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "PBColDate", "DATE")).Font(AccountBoldFont(11)).ColorAndOpacity(AccountText()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)]
					+ SHorizontalBox::Slot().FillWidth(0.95f)[SNew(STextBlock).Text(bRankInThirdColumn ? RankHeaderText : ValueHeaderText).Font(AccountBoldFont(11)).ColorAndOpacity(AccountText()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)]
					+ SHorizontalBox::Slot().FillWidth(1.00f)[SNew(STextBlock).Text(bRankInThirdColumn ? ValueHeaderText : RankHeaderText).Font(AccountBoldFont(11)).ColorAndOpacity(AccountText()).OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)],
					AccountHeaderFill(), FMargin(8.f, 5.f, 8.f, 4.f))
			]
			+ SVerticalBox::Slot().AutoHeight()[Rows],
			ET66PanelType::Panel2, AccountPanelInnerFill(), FMargin(8.f));
	};

	auto MakeAccountStatusPanel = [&]() -> TSharedRef<SWidget>
	{
		return MakeAccountPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeSectionHeader(NSLOCTEXT("T66.Account", "StatusHeader", "ACCOUNT STATUS"))]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(RestrictionText(Restriction.Restriction))
					.Font(AccountBoldFont(22))
					.ColorAndOpacity(bAccountEligible ? AccountSuccess() : AccountDanger())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SSpacer)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
				[
					MakeAccountReferenceButton(
						FT66ButtonParams(
							NSLOCTEXT("T66.Account", "StandingHelpButton", "?"),
							FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleStandingInfoClicked),
							ET66ButtonType::Neutral)
						.SetFontSize(AdjustAccountFontSize(11))
						.SetMinWidth(42.f)
						.SetHeight(42.f)
						.SetPadding(FMargin(0.f))
						.SetTextColor(AccountChromeText()))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SBox)
				.Visibility(bShowStandingInfoPopup ? EVisibility::Visible : EVisibility::Collapsed)
				[
					MakeAccountPanel(
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Account", "StandingHelpBody", "Cheating, client tampering, or exploit abuse will suspend this account. Suspended accounts and their runs are not eligible for leaderboards or ranked personal-best tracking until the restriction is cleared."))
						.Font(AccountRegularFont(11))
						.ColorAndOpacity(AccountText())
						.AutoWrapText(true)
						.Clipping(EWidgetClipping::ClipToBounds),
						ET66PanelType::Panel2,
						AccountPanelInnerFill(),
						FMargin(10.f, 8.f))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				SNew(STextBlock)
				.Text(bAccountEligible ? NSLOCTEXT("T66.Account", "EligibleBody", "Your account is eligible for ranked tracking and personal best progression.") : NSLOCTEXT("T66.Account", "RestrictedBody", "This account is suspended from leaderboard submissions until the restriction is cleared."))
				.Font(AccountRegularFont(12))
				.ColorAndOpacity(AccountText())
				.AutoWrapText(true)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
			+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(AppealStatusText(Restriction.AppealStatus)).Font(AccountBoldFont(11)).ColorAndOpacity(AccountMutedText())]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[SNew(SBox).Visibility(!Restriction.RestrictionReason.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed)[MakeAccountPanel(SNew(STextBlock).Text(FText::FromString(Restriction.RestrictionReason)).Font(AccountRegularFont(11)).ColorAndOpacity(AccountText()).AutoWrapText(true), ET66PanelType::Panel2, AccountPanelInnerFill(), FMargin(10.f))]]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)[SNew(SBox).Visibility(LB && LB->HasAccountRestrictionRunSummary() ? EVisibility::Visible : EVisibility::Collapsed)[MakeAccountReferenceButton(FT66ButtonParams(NSLOCTEXT("T66.Account", "ViewReviewed", "VIEW REVIEWED RUN"), FOnClicked::CreateLambda([this, WeakLB]() { if (UT66LeaderboardSubsystem* RuntimeLB = WeakLB.Get(); RuntimeLB && RuntimeLB->RequestOpenAccountRestrictionRunSummary()) { ShowModal(ET66ScreenType::RunSummary); } return FReply::Handled(); }), ET66ButtonType::Primary).SetFontSize(AdjustAccountFontSize(12)).SetMinWidth(190.f).SetHeight(32.f).SetPadding(FMargin(12.f, 6.f, 12.f, 4.f)).SetTextColor(AccountChromeText()))]],
			ET66PanelType::Panel, AccountPanelFill(), FMargin(14.f));
	};

	auto MakeAccountProgressPanel = [&]() -> TSharedRef<SWidget>
	{
		return MakeAccountPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeSectionHeader(NSLOCTEXT("T66.Account", "ProgressHeader", "ACCOUNT PROGRESS"))]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeProgressRow(NSLOCTEXT("T66.Account", "AchProg", "Achievements Unlocked"), UnlockedAchievements, AchievementDefs.Num(), FLinearColor(0.92f, 0.05f, 0.12f, 1.0f))]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeProgressRow(NSLOCTEXT("T66.Account", "PowerProg", "Permanent Buffs Unlocked"), UnlockedPowerUps, PowerStats.Num() * UT66BuffSubsystem::MaxFillStepsPerStat, FLinearColor(0.92f, 0.05f, 0.12f, 1.0f))]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeProgressRow(NSLOCTEXT("T66.Account", "HeroProg", "Heroes Unlocked"), HeroIDs.Num(), HeroIDs.Num(), FLinearColor(0.92f, 0.05f, 0.12f, 1.0f))]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeProgressRow(NSLOCTEXT("T66.Account", "CompProg", "Companions Unlocked"), UnlockedCompanions, CompanionIDs.Num(), FLinearColor(0.92f, 0.05f, 0.12f, 1.0f))]
			+ SVerticalBox::Slot().AutoHeight()[MakeProgressRow(NSLOCTEXT("T66.Account", "ChallengeProg", "Challenges Completed"), DisplayChallengesCompleted, TotalChallengeCount, FLinearColor(0.92f, 0.05f, 0.12f, 1.0f))]
			+ SVerticalBox::Slot().FillHeight(1.f)[SNew(SSpacer)],
			ET66PanelType::Panel, AccountPanelFill(), FMargin(14.f));
	};

	auto MakePersonalBestPanel = [&]() -> TSharedRef<SWidget>
	{
		return MakeAccountPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)[MakePBViewModeDropdown()]
				+ SHorizontalBox::Slot().FillWidth(1.f)[MakePBPartySizeDropdown()]
			]
			+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(SScrollBox)
				.ScrollBarStyle(GetAccountReferenceScrollBarStyle())
				.ScrollBarVisibility(EVisibility::Visible)
				+ SScrollBox::Slot().Padding(0.f, 0.f, 0.f, 8.f)[MakePBBlock(NSLOCTEXT("T66.Account", "TopScore", "HIGHEST SCORE"), false)]
				+ SScrollBox::Slot()[MakePBBlock(NSLOCTEXT("T66.Account", "TopTime", "BEST SPEED RUN"), true)]
			],
			ET66PanelType::Panel, AccountPanelFill(), FMargin(14.f));
	};

	TSharedRef<SWidget> OverviewContent = SNew(SBox);
	if (bUseStackedOverviewLayout)
	{
		OverviewContent =
			SNew(SScrollBox)
			.ScrollBarStyle(GetAccountReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			+ SScrollBox::Slot().Padding(0.f, 0.f, 0.f, 10.f)[MakeProfileLevelPanel()]
			+ SScrollBox::Slot().Padding(0.f, 0.f, 0.f, 10.f)[MakeAccountStatusPanel()]
			+ SScrollBox::Slot().Padding(0.f, 0.f, 0.f, 10.f)[MakeAccountProgressPanel()]
			+ SScrollBox::Slot()[MakePersonalBestPanel()];
	}
	else
	{
		OverviewContent =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.42f).Padding(0.f, 0.f, 16.f, 0.f)
			[
				SNew(SScrollBox)
				.ScrollBarStyle(GetAccountReferenceScrollBarStyle())
				.ScrollBarVisibility(EVisibility::Visible)
				+ SScrollBox::Slot().Padding(0.f, 0.f, 0.f, 10.f)[MakeProfileLevelPanel()]
				+ SScrollBox::Slot().Padding(0.f, 0.f, 0.f, 10.f)[MakeAccountStatusPanel()]
				+ SScrollBox::Slot()[MakeAccountProgressPanel()]
			]
			+ SHorizontalBox::Slot().FillWidth(0.58f).VAlign(VAlign_Top)
			[
				MakePersonalBestPanel()
			];
	}

	TSharedRef<SWidget> HistoryContent =
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
			[
				HistoryFilterBar
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				MakeAccountPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 9.f)
					[
						MakeSectionHeader(NSLOCTEXT("T66.Account", "RunHistoryTitle", "RUN HISTORY"))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
					[
						HistoryColumnHeader
					]
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						SNew(SScrollBox)
						.ScrollBarStyle(GetAccountReferenceScrollBarStyle())
						.ScrollBarVisibility(EVisibility::Visible)
						.ScrollBarThickness(FVector2D(18.f, 18.f))
						.ScrollBarPadding(FMargin(10.f, 0.f, 0.f, 0.f))
						+ SScrollBox::Slot()
						[
							HistoryRows
						]
					],
					ET66PanelType::Panel,
					AccountPanelFill(),
					FMargin(14.f))
			]
		];

	const FText SuspensionHeadline =
		Restriction.Restriction == ET66AccountRestrictionKind::CheatingCertainty
		? NSLOCTEXT("T66.Account", "SuspensionRestrictedHeadline", "ACCOUNT RESTRICTED")
		: NSLOCTEXT("T66.Account", "SuspensionHeadline", "ACCOUNT SUSPENDED");
	const FText SuspensionBody =
		Restriction.Restriction == ET66AccountRestrictionKind::CheatingCertainty
		? NSLOCTEXT("T66.Account", "SuspensionRestrictedBody", "This account is blocked from leaderboard submissions until the restriction is cleared.")
		: NSLOCTEXT("T66.Account", "SuspensionBody", "This account cannot submit leaderboard scores while the suspension is active.");
	const bool bCanSubmitAppeal = LB && LB->CanSubmitAccountAppeal();
	const FText SubmitAppealButtonText = bAppealSubmitInFlight
		? NSLOCTEXT("T66.Account", "SubmitAppealInFlight", "SUBMITTING...")
		: (Loc ? Loc->GetText_AccountStatus_SubmitAppeal() : NSLOCTEXT("T66.Account", "SubmitAppeal", "SUBMIT APPEAL"));
	const TAttribute<bool> CanSubmitAppealMessage = TAttribute<bool>::CreateLambda([this]()
	{
		FString TrimmedMessage = AppealDraftMessage;
		TrimmedMessage.TrimStartAndEndInline();
		return !bAppealSubmitInFlight && !TrimmedMessage.IsEmpty();
	});
	TSharedRef<SWidget> SuspensionContent =
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			MakeAccountPanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeSectionHeader(NSLOCTEXT("T66.Account", "SuspensionHdr", "SUSPENSION"))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(STextBlock)
					.Text(SuspensionHeadline)
					.Font(AccountBoldFont(22))
					.ColorAndOpacity(AccountDanger())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(SuspensionBody)
					.Font(AccountRegularFont(12))
					.ColorAndOpacity(AccountText())
					.AutoWrapText(true)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeAccountPanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.Account", "SuspensionReasonLabel", "REASON"))
							.Font(AccountBoldFont(11))
							.ColorAndOpacity(AccountGold())
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(Restriction.RestrictionReason.IsEmpty() ? NSLOCTEXT("T66.Account", "NoSuspensionReason", "No reason recorded.") : FText::FromString(Restriction.RestrictionReason))
							.Font(AccountRegularFont(12))
							.ColorAndOpacity(AccountText())
							.AutoWrapText(true)
							.Clipping(EWidgetClipping::ClipToBounds)
						],
						ET66PanelType::Panel2,
						AccountPanelInnerFill(),
						FMargin(12.f))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(AppealStatusText(Restriction.AppealStatus))
					.Font(AccountBoldFont(11))
					.ColorAndOpacity(AccountMutedText())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
				[
					SNew(SBox)
					.Visibility(!AppealSubmitStatusMessage.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed)
					[
						SNew(STextBlock)
						.Text(FText::FromString(AppealSubmitStatusMessage))
						.Font(AccountRegularFont(11))
						.ColorAndOpacity(bAppealSubmitStatusIsError ? AccountDanger() : AccountSuccess())
						.AutoWrapText(true)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
				[
					SNew(SBox)
					.Visibility(bCanSubmitAppeal && !bAppealEditorOpen ? EVisibility::Visible : EVisibility::Collapsed)
					[
						MakeAccountReferenceButton(
							FT66ButtonParams(NSLOCTEXT("T66.Account", "OpenAppeal", "APPEAL"), FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOpenAppealClicked), ET66ButtonType::Primary)
							.SetFontSize(AdjustAccountFontSize(12))
							.SetMinWidth(150.f)
							.SetHeight(32.f)
							.SetPadding(FMargin(12.f, 6.f, 12.f, 4.f))
							.SetTextColor(AccountChromeText()))
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
				[
					SNew(SBox)
					.Visibility(bCanSubmitAppeal && bAppealEditorOpen ? EVisibility::Visible : EVisibility::Collapsed)
					[
						MakeAccountPanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(Loc ? Loc->GetText_AccountStatus_AppealTitle() : NSLOCTEXT("T66.Account", "AppealTitle", "APPEAL"))
								.Font(AccountBoldFont(12))
								.ColorAndOpacity(AccountGold())
								.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
								.Clipping(EWidgetClipping::ClipToBounds)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 10.f)
							[
								MakeAccountFieldShell(
									SNew(SBox)
									.HeightOverride(118.f)
									[
										SAssignNew(AppealMessageTextBox, SMultiLineEditableTextBox)
										.AutoWrapText(true)
										.Text(FText::FromString(AppealDraftMessage))
										.HintText(Loc ? Loc->GetText_AccountStatus_AppealHint() : NSLOCTEXT("T66.Account", "AppealHint", "Write your appeal message here..."))
										.ForegroundColor(AccountText())
										.OnTextChanged_Lambda([this](const FText& NewText)
										{
											AppealDraftMessage = NewText.ToString();
										})
									],
									FMargin(12.f, 10.f))
							]
							+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
								[
									MakeAccountReferenceButton(
										FT66ButtonParams(SubmitAppealButtonText, FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleSubmitAppealClicked), ET66ButtonType::Primary)
										.SetFontSize(AdjustAccountFontSize(11))
										.SetMinWidth(168.f)
										.SetHeight(32.f)
										.SetPadding(FMargin(12.f, 6.f, 12.f, 4.f))
										.SetTextColor(AccountChromeText())
										.SetEnabled(CanSubmitAppealMessage))
								]
								+ SHorizontalBox::Slot().AutoWidth()
								[
									MakeAccountReferenceButton(
										FT66ButtonParams(NSLOCTEXT("T66.Account", "CancelAppeal", "CANCEL"), FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleCancelAppealClicked), ET66ButtonType::Neutral)
										.SetFontSize(AdjustAccountFontSize(11))
										.SetMinWidth(118.f)
										.SetHeight(32.f)
										.SetPadding(FMargin(12.f, 6.f, 12.f, 4.f))
										.SetTextColor(AccountChromeText())
										.SetEnabled(TAttribute<bool>::CreateLambda([this]() { return !bAppealSubmitInFlight; })))
								]
							],
							ET66PanelType::Panel2,
							AccountPanelInnerFill(),
							FMargin(12.f))
					]
				],
				ET66PanelType::Panel,
				AccountPanelFill(),
				FMargin(14.f))
		];

	TSharedRef<SWidget> ActiveContent =
		ActiveTab == EAccountTab::Suspension
			? SuspensionContent
			: (ActiveTab == EAccountTab::History ? HistoryContent : OverviewContent);
	if (ActiveTab == EAccountTab::History)
	{
		ActiveContent =
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.006f, 0.006f, 0.012f, 1.0f))
			.Padding(FMargin(0.f))
			[
				ActiveContent
			];
	}
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	auto MakeAccountTabRow = [&]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, bHasSuspension ? 20.f : 0.f, 0.f)
			[
				SNew(SBox)
				.Visibility(bHasSuspension ? EVisibility::Visible : EVisibility::Collapsed)
				[
					MakeTabButton(
						NSLOCTEXT("T66.Account", "SuspensionTab", "SUSPENSION"),
						ActiveTab == EAccountTab::Suspension,
						&UT66AccountStatusScreen::HandleSuspensionTabClicked,
						AccountDanger(),
						FLinearColor(0.22f, 0.08f, 0.07f, 1.0f),
						AccountChromeText(),
						AccountDanger())
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 20.f, 0.f)
			[
				MakeTabButton(NSLOCTEXT("T66.Account", "OverviewTab", "OVERVIEW"), ActiveTab == EAccountTab::Overview, &UT66AccountStatusScreen::HandleOverviewTabClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeTabButton(NSLOCTEXT("T66.Account", "HistoryTab", "HISTORY"), ActiveTab == EAccountTab::History, &UT66AccountStatusScreen::HandleHistoryTabClicked)
			];
	};
	auto MakeAccountTabInfoStrip = [&]() -> TSharedRef<SWidget>
	{
		FText TabInfoText = NSLOCTEXT("T66.Account", "OverviewTabInfo", "Review account standing, profile level, progress, and personal bests.");
		if (ActiveTab == EAccountTab::History)
		{
			TabInfoText = NSLOCTEXT("T66.Account", "HistoryTabInfo", "Filter run history and compare ranked highscores and speedruns.");
		}
		else if (ActiveTab == EAccountTab::Suspension)
		{
			TabInfoText = NSLOCTEXT("T66.Account", "SuspensionTabInfo", "Review moderation status, appeal state, and flagged run details.");
		}

		return SNew(SBox)
			.HeightOverride(54.f)
			[
				MakeAccountRedBarPanel(
					SNew(SBox)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(TabInfoText)
						.Font(AccountRegularFont(14))
						.ColorAndOpacity(AccountChromeText())
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
					],
					FMargin(18.f, 7.f))
			];
	};

	if (bModalPresentation)
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FT66Style::Scrim())]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(FMargin(24.f, 30.f))
			[
				SNew(SBox)
				.MaxDesiredWidth(1600.f)
				[
					MakeAccountPanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)[MakeAccountTabRow()]
						+ SVerticalBox::Slot().AutoHeight().Padding(20.f, 0.f, 20.f, 12.f)[MakeAccountTabInfoStrip()]
						+ SVerticalBox::Slot().AutoHeight()[ActiveContent]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 12.f, 0.f, 0.f)
						[
							MakeAccountReferenceButton(FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleBackClicked), ET66ButtonType::Neutral).SetFontSize(AdjustAccountFontSize(13)).SetMinWidth(116.f).SetHeight(34.f).SetPadding(FMargin(12.f, 7.f, 12.f, 5.f)).SetTextColor(AccountChromeText()))
						],
						ET66PanelType::Panel, AccountPanelFill(), FMargin(18.f))
				]
			];
	}

	const TSharedRef<SWidget> ScreenContent =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.006f, 0.006f, 0.012f, 0.98f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(FMargin(0.f, ScreenLayout.bCompact ? 6.f : 8.f, 0.f, ScreenLayout.bCompact ? 6.f : 8.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, ScreenLayout.bCompact ? 8.f : 10.f)[MakeAccountTabRow()]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, ScreenLayout.bCompact ? 8.f : 10.f)[MakeAccountTabInfoStrip()]
					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						ActiveContent
					]
				]
			]
		];

	TSharedRef<SWidget> BackgroundContent = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black);

	const TSharedRef<SWidget> FullWidthScreenContent =
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			ScreenContent
		];

	return T66ScreenSlateHelpers::MakeTopBarScreenRoot(
		UIManager,
		FullWidthScreenContent,
		BackgroundContent,
		FLinearColor::Transparent,
		FMargin(0.f));
}

const FSlateBrush* UT66AccountStatusScreen::GetOrCreateHeroPortraitBrush(UT66GameInstance* T66GI, FName HeroID)
{
	if (!T66GI || HeroID.IsNone())
	{
		return nullptr;
	}

	if (TSharedPtr<FSlateBrush>* Found = HeroPortraitBrushes.Find(HeroID))
	{
		return Found->Get();
	}

	UT66UITexturePoolSubsystem* TexPool = T66GI->GetSubsystem<UT66UITexturePoolSubsystem>();
	if (!TexPool)
	{
		return nullptr;
	}

	const TSoftObjectPtr<UTexture2D> PortraitSoft = T66GI->ResolveHeroPortrait(HeroID, ET66BodyType::Chad, ET66HeroPortraitVariant::Low);
	if (PortraitSoft.IsNull())
	{
		return nullptr;
	}

	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	Brush->DrawAs = ESlateBrushDrawType::Image;
	Brush->Tiling = ESlateBrushTileType::NoTile;
	Brush->ImageSize = FVector2D(32.f, 32.f);
	Brush->SetResourceObject(T66SlateTexture::GetLoaded(TexPool, PortraitSoft));

	T66SlateTexture::BindSharedBrushAsync(
		TexPool,
		PortraitSoft,
		this,
		Brush,
		FName(*FString::Printf(TEXT("AccountHero_%s"), *HeroID.ToString())),
		/*bClearWhileLoading*/ false);

	HeroPortraitBrushes.Add(HeroID, Brush);
	return Brush.Get();
}

FReply UT66AccountStatusScreen::HandleBackClicked()
{
	if (UIManager)
	{
		if (UIManager->GetCurrentModalType() == ScreenType)
		{
			if (APlayerController* PC = GetOwningPlayer())
			{
				if (PC->IsPaused())
				{
					UIManager->ShowModal(ET66ScreenType::PauseMenu);
					return FReply::Handled();
				}
			}

			UIManager->CloseModal();
			return FReply::Handled();
		}

		UIManager->GoBack();
	}

	return FReply::Handled();
}

void UT66AccountStatusScreen::HandleBackendAppealSubmitComplete(bool bSuccess, const FString& Message)
{
	AppealSubmitStatusMessage = Message;
	bAppealSubmitStatusIsError = !bSuccess;
	bAppealSubmitInFlight = false;
	if (bSuccess)
	{
		bAppealEditorOpen = false;
		AppealDraftMessage.Reset();
	}

	if (!HasBuiltSlateUI() || !IsVisible())
	{
		return;
	}

	RequestDeferredSlateRebuild();
}

FReply UT66AccountStatusScreen::HandleSuspensionTabClicked()
{
	ActiveTab = EAccountTab::Suspension;
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleOverviewTabClicked()
{
	ActiveTab = EAccountTab::Overview;
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleHistoryTabClicked()
{
	ActiveTab = EAccountTab::History;
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleOpenAppealClicked()
{
	bAppealEditorOpen = true;
	AppealSubmitStatusMessage.Reset();
	bAppealSubmitStatusIsError = false;
	bAppealSubmitInFlight = false;
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleStandingInfoClicked()
{
	bShowStandingInfoPopup = !bShowStandingInfoPopup;
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleCancelAppealClicked()
{
	bAppealEditorOpen = false;
	bAppealSubmitInFlight = false;
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleSubmitAppealClicked()
{
	if (bAppealSubmitInFlight)
	{
		return FReply::Handled();
	}

	UT66LeaderboardSubsystem* LB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	FString Message = AppealDraftMessage;
	Message.TrimStartAndEndInline();

	if (!LB || Message.IsEmpty())
	{
		AppealSubmitStatusMessage = Message.IsEmpty()
			? NSLOCTEXT("T66.Account", "AppealRequired", "Appeal message is required.").ToString()
			: NSLOCTEXT("T66.Account", "AppealUnavailable", "Appeals are unavailable right now.").ToString();
		bAppealSubmitStatusIsError = true;
		RequestDeferredSlateRebuild();
		return FReply::Handled();
	}

	AppealSubmitStatusMessage = NSLOCTEXT("T66.Account", "AppealSubmitting", "Submitting appeal...").ToString();
	bAppealSubmitStatusIsError = false;
	bAppealSubmitInFlight = true;
	LB->SubmitAccountAppeal(Message, FString());
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleOpenRunSummaryClicked(FString SlotName)
{
	if (SlotName.IsEmpty())
	{
		return FReply::Handled();
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	if (!LB)
	{
		return FReply::Handled();
	}

	const bool bModalPresentation = UIManager && UIManager->GetCurrentModalType() == ScreenType;
	if (LB->RequestOpenRunSummarySlot(SlotName, bModalPresentation ? ET66ScreenType::AccountStatus : ET66ScreenType::None))
	{
		ShowModal(ET66ScreenType::RunSummary);
	}

	return FReply::Handled();
}
