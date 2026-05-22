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
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66Style.h"

#include "Data/T66DataTypes.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
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
		return FT66FlatStyle::Tokens::FontBold(AdjustAccountFontSize(BaseSize));
	}

	FSlateFontInfo AccountRegularFont(int32 BaseSize)
	{
		return FT66FlatStyle::Tokens::FontRegular(AdjustAccountFontSize(BaseSize));
	}

	const FSlateBrush* AccountFlatIconBrush(const TCHAR* RelativePath, const FVector2D& ImageSize, const TCHAR* DebugLabel)
	{
		struct FCache
		{
			TSharedPtr<FSlateBrush> Brush;
			TStrongObjectPtr<UTexture2D> Texture;
		};

		static TMap<FString, FCache> Cache;
		const FString Key = FString::Printf(TEXT("%s|%.0fx%.0f"), RelativePath, ImageSize.X, ImageSize.Y);
		FCache& Entry = Cache.FindOrAdd(Key);
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = ESlateBrushDrawType::Image;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
			Entry.Brush->ImageSize = ImageSize;
		}

		if (!Entry.Texture.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					DebugLabel))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->ImageSize = ImageSize;
		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
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
		return FT66FlatStyle::GetFlatMainMenuElementAssetPath(FileName);
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
			return FT66FlatStyle::GetFlatRedSquareButtonAssetPath(*State);
		}
		return FT66FlatStyle::GetFlatChromeElementAssetPath(FileName);
	}

	FString MakeAccountMainMenuLongPanelPath(const TCHAR* State = TEXT("normal"))
	{
		return FT66FlatStyle::GetFlatLongPanelAssetPath(State);
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
			return FT66FlatStyle::GetFlatRedSquareButtonAssetPath(*State);
		}

		if (LowerPath.Contains(TEXT("dropdown_field")))
		{
			const FString DropdownState = State.Equals(TEXT("selected"), ESearchCase::IgnoreCase) ? FString(TEXT("pressed")) : State;
			return FT66FlatStyle::GetFlatRedSquareButtonAssetPath(*DropdownState);
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
				? FT66FlatStyle::GetFlatRedSquareButtonAssetPath(TEXT("normal"))
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
			FT66FlatStyle::GetFlatRedSquareButtonAssetPath(State ? State : TEXT("normal")),
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
			FT66FlatStyle::GetFlatRedSquareButtonAssetPath(TEXT("normal")),
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
		FSlateFontInfo ButtonFont = FT66FlatStyle::MakeFont(*Params.FontWeight, FontSize);
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
		return FT66FlatStyle::BuildFlatSlicedPlateButton(
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

		return FT66FlatStyle::MakePanel(
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
			FT66FlatStyle::GetFlatRedSquareButtonAssetPath(TEXT("normal")),
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

		return FT66FlatStyle::MakePanel(
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

		return FT66FlatStyle::BuildFlatProgressBar(
			Pct,
			DesiredSize,
			FallbackFill,
			FMargin(4.f, 3.f));
	}

	TSharedRef<SWidget> MakeAccountReferenceDropdown(const FT66DropdownParams& Params)
	{
		static FComboButtonStyle FlatComboStyle = []()
		{
			FComboButtonStyle Style = FT66FlatStyle::GetDropdownComboButtonStyle();
			Style.ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			return Style;
		}();

		TSharedRef<SComboButton> Combo = SNew(SComboButton)
			.ComboButtonStyle(&FlatComboStyle)
			.MenuPlacement(MenuPlacement_BelowAnchor)
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

		return FT66FlatStyle::MakePanel(
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
		FString RequestedFrontendScreen;
		if (!FParse::Value(FCommandLine::Get(), TEXT("T66FrontendScreen="), RequestedFrontendScreen)
			|| (!RequestedFrontendScreen.Equals(TEXT("Overview"), ESearchCase::IgnoreCase)
				&& !RequestedFrontendScreen.Equals(TEXT("History"), ESearchCase::IgnoreCase)))
		{
			return;
		}
		RequestedAccountTab = RequestedFrontendScreen;
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
	const FT66FlatStyle::FTopBarScreenLayoutMetrics ScreenLayout =
		FT66FlatStyle::MakeTopBarScreenLayoutMetrics(UIManager);
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
		const FT66FlatStyle::FFrontendChromeMetrics& ChromeMetrics = FT66FlatStyle::GetFrontendChromeMetrics();

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

	if (ActiveTab == EAccountTab::Overview)
	{
		{
			auto OTag = [](const TCHAR* Tag) -> FName
			{
				return FName(Tag);
			};

			auto UpperText = [](const FText& Text) -> FText
			{
				FString Value = Text.ToString();
				Value.ToUpperInline();
				return FText::FromString(Value);
			};

			constexpr float OverviewCanvasW = 1920.f;
			constexpr float OverviewCanvasH = 1080.f;
			const TSharedRef<SConstraintCanvas> OverviewCanvas = SNew(SConstraintCanvas);

			auto AddCanvas = [OverviewCanvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
			{
				OverviewCanvas->AddSlot()
				.Anchors(FAnchors(0.f, 0.f))
				.Alignment(FVector2D(0.f, 0.f))
				.Offset(FMargin(X, Y, W, H))
				[
					Widget
				];
			};

			auto AddN = [&AddCanvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
			{
				AddCanvas(X * OverviewCanvasW, Y * OverviewCanvasH, W * OverviewCanvasW, H * OverviewCanvasH, Widget);
			};

			auto MakeMetadataRegion = [](const FName Tag, const FString& Role, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
			{
				return FT66FlatStyle::AttachMetadata(
					SNew(SSpacer),
					Tag,
					Role,
					State);
			};

			auto MakePanelSurface = [](const FName Tag, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
			{
				return FT66FlatStyle::MakeFlatPanel(
					State,
					FMargin(0.f),
					SNew(SSpacer),
					nullptr,
					Tag);
			};

			auto TaggedText = [](
				const FName Tag,
				const FText& Text,
				const int32 FontSize,
				const FLinearColor& Color,
				const bool bBold = true,
				const ETextJustify::Type Justification = ETextJustify::Left,
				const bool bWrap = false) -> TSharedRef<SWidget>
			{
				return FT66FlatStyle::AttachMetadata(
					SNew(STextBlock)
					.Visibility(EVisibility::HitTestInvisible)
					.Text(Text)
					.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
					.ColorAndOpacity(Color)
					.Justification(Justification)
					.AutoWrapText(bWrap)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds),
					Tag,
					TEXT("Label"),
					ET66FlatState::Default,
					TOptional<FLinearColor>(),
					false,
					NAME_None,
					true);
			};

			auto PlainText = [](
				const FText& Text,
				const int32 FontSize,
				const FLinearColor& Color,
				const bool bBold = false,
				const ETextJustify::Type Justification = ETextJustify::Left) -> TSharedRef<SWidget>
			{
				return SNew(STextBlock)
					.Visibility(EVisibility::HitTestInvisible)
					.Text(Text)
					.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
					.ColorAndOpacity(Color)
					.Justification(Justification)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds);
			};

			auto MakeIcon = [](const FName Tag, const FSlateBrush* Brush, const FText& FallbackText) -> TSharedRef<SWidget>
			{
				const TSharedRef<SWidget> IconContent = Brush
					? StaticCastSharedRef<SWidget>(SNew(SImage).Visibility(EVisibility::HitTestInvisible).Image(Brush).ColorAndOpacity(FLinearColor::White))
					: StaticCastSharedRef<SWidget>(
						SNew(STextBlock)
						.Visibility(EVisibility::HitTestInvisible)
						.Text(FallbackText)
						.Font(FT66FlatStyle::MakeBoldFont(18))
						.ColorAndOpacity(FT66FlatStyle::PrimaryText())
						.Justification(ETextJustify::Center));

				return FT66FlatStyle::AttachMetadata(
					SNew(SBox)
					.Visibility(EVisibility::HitTestInvisible)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						IconContent
					],
					Tag,
					TEXT("Icon"),
					ET66FlatState::Default);
			};

			auto MakeProgressVisual = [](const FName Tag, const float Percent, const float WidthNorm) -> TSharedRef<SWidget>
			{
				const float ClampedPercent = FMath::Clamp(Percent, 0.f, 1.f);
				const float FillWidth = FMath::Max(0.f, WidthNorm * OverviewCanvasW * ClampedPercent);
				const TSharedRef<SWidget> Track = SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FT66FlatStyle::BorderForState(ET66FlatState::Default))
						.Padding(2.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FT66FlatStyle::FillForState(ET66FlatState::Default))
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					[
						SNew(SBox)
						.WidthOverride(FillWidth)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FT66FlatStyle::SelectedBorder())
						]
					];

				return FT66FlatStyle::AttachMetadata(
					Track,
					Tag,
					TEXT("ProgressBar"),
					ET66FlatState::Ready);
			};

			const FSlateBrush* InfoBrush = AccountFlatIconBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/info.png"), FVector2D(24.f, 24.f), TEXT("OverviewInfoIcon"));
			const FSlateBrush* ShieldBrush = AccountFlatIconBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/shield.png"), FVector2D(72.f, 72.f), TEXT("OverviewShieldIcon"));
			const FSlateBrush* ChartBrush = AccountFlatIconBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/bar_chart.png"), FVector2D(58.f, 58.f), TEXT("OverviewBarChartIcon"));
			const FSlateBrush* TrophyBrush = AccountFlatIconBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/trophy_laurel.png"), FVector2D(48.f, 48.f), TEXT("OverviewTrophyIcon"));
			const FSlateBrush* StopwatchBrush = AccountFlatIconBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/stopwatch.png"), FVector2D(50.f, 50.f), TEXT("OverviewStopwatchIcon"));

			AddN(0.013f, 0.123f, 0.974f, 0.838f, MakeMetadataRegion(OTag(TEXT("Overview.Root")), TEXT("Root")));
			AddN(0.013f, 0.201f, 0.974f, 0.760f, MakeMetadataRegion(OTag(TEXT("Overview.MainBody")), TEXT("Body")));
			AddN(0.013f, 0.207f, 0.393f, 0.754f, MakeMetadataRegion(OTag(TEXT("Overview.LeftColumn")), TEXT("Column")));
			AddN(0.420f, 0.201f, 0.566f, 0.760f, MakeMetadataRegion(OTag(TEXT("Overview.RightColumn")), TEXT("Column")));
			AddN(0.441f, 0.230f, 0.526f, 0.056f, MakeMetadataRegion(OTag(TEXT("Overview.FilterRow")), TEXT("FilterRow")));

			AddN(0.013f, 0.207f, 0.393f, 0.221f, MakePanelSurface(OTag(TEXT("Overview.PlayerBlock"))));
			AddN(0.013f, 0.454f, 0.393f, 0.182f, MakePanelSurface(OTag(TEXT("Overview.AccountStatusPanel"))));
			AddN(0.013f, 0.654f, 0.393f, 0.307f, MakePanelSurface(OTag(TEXT("Overview.AccountProgressPanel"))));
			AddN(0.420f, 0.201f, 0.566f, 0.760f, MakePanelSurface(OTag(TEXT("Overview.RightColumn.OuterPanel"))));
			AddN(0.440f, 0.308f, 0.527f, 0.278f, MakePanelSurface(OTag(TEXT("Overview.HighestScorePanel"))));
			AddN(0.440f, 0.616f, 0.527f, 0.296f, MakePanelSurface(OTag(TEXT("Overview.BestSpeedRunPanel"))));

			const TSharedRef<SWidget> OverviewButtonContent = SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					PlainText(NSLOCTEXT("T66.Account", "OverviewTabFlatFixed", "OVERVIEW"), 24, FT66FlatStyle::SelectedText(), true, ETextJustify::Center)
				];
			const TSharedRef<SWidget> HistoryButtonContent = SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					PlainText(NSLOCTEXT("T66.Account", "HistoryTabFlatFixed", "HISTORY"), 24, FT66FlatStyle::PrimaryText(), true, ETextJustify::Center)
				];

			AddN(0.148f, 0.123f, 0.690f, 0.060f, MakeMetadataRegion(OTag(TEXT("Overview.SubTabs")), TEXT("ToggleGroup.AccountTabs")));
			AddN(
				0.148f,
				0.123f,
				0.320f,
				0.060f,
				FT66FlatStyle::MakeFlatToggleGroupButton(
					ET66FlatState::Selected,
					OverviewButtonContent,
					FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOverviewTabClicked),
					FMargin(0.f),
					0.320f * OverviewCanvasW,
					0.060f * OverviewCanvasH,
					true,
					OTag(TEXT("Overview.SubTabs.OverviewButton")),
					OTag(TEXT("AccountTabs"))));
			AddN(
				0.498f,
				0.123f,
				0.340f,
				0.060f,
				FT66FlatStyle::MakeFlatToggleGroupButton(
					ET66FlatState::Default,
					HistoryButtonContent,
					FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleHistoryTabClicked),
					FMargin(0.f),
					0.340f * OverviewCanvasW,
					0.060f * OverviewCanvasH,
					true,
					OTag(TEXT("Overview.SubTabs.HistoryButton")),
					OTag(TEXT("AccountTabs"))));

			AddN(0.438f, 0.140f, 0.016f, 0.030f, FT66FlatStyle::MakeFlatTooltipIcon(
				ET66FlatState::Selected,
				InfoBrush,
				NSLOCTEXT("T66.Account", "OverviewTabInfoTooltipFixed", "Account overview"),
				FVector2D(31.f, 31.f),
				OTag(TEXT("Overview.SubTabs.OverviewInfoIcon")),
				FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOverviewTabClicked)));
			AddN(0.806f, 0.140f, 0.016f, 0.030f, FT66FlatStyle::MakeFlatTooltipIcon(
				ET66FlatState::Default,
				InfoBrush,
				NSLOCTEXT("T66.Account", "HistoryTabInfoTooltipFixed", "Run history"),
				FVector2D(31.f, 31.f),
				OTag(TEXT("Overview.SubTabs.HistoryInfoIcon")),
				FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleHistoryTabClicked)));

			const FText OverviewProfileNameText = !LocalSteamName.IsEmpty()
				? FText::FromString(LocalSteamName)
				: NSLOCTEXT("T66.Account", "ProfileNameRandomChadFixed", "RandomChad");
			const float ExperiencePct = ProfileExperienceToNextLevel > 0
				? FMath::Clamp(static_cast<float>(ProfileExperienceIntoLevel) / static_cast<float>(ProfileExperienceToNextLevel), 0.f, 1.f)
				: 0.f;
			const FSlateBrush* ProfileBrush = ProfileAvatarBrush.IsValid() && ProfileAvatarBrush->GetResourceObject()
				? ProfileAvatarBrush.Get()
				: nullptr;
			AddN(0.026f, 0.230f, 0.102f, 0.173f, FT66FlatStyle::MakeFlatPortraitSlot(ET66FlatState::Default, ProfileBrush, nullptr, FVector2D(0.102f * OverviewCanvasW, 0.173f * OverviewCanvasH), OTag(TEXT("Overview.PlayerBlock.Avatar"))));
			AddN(0.145f, 0.238f, 0.106f, 0.033f, TaggedText(OTag(TEXT("Overview.PlayerBlock.Name")), OverviewProfileNameText, 26, FT66FlatStyle::PrimaryText()));
			AddN(0.145f, 0.291f, 0.077f, 0.027f, TaggedText(OTag(TEXT("Overview.PlayerBlock.Level")), UpperText(ProfileLevelText), 18, FT66FlatStyle::SelectedText()));
			AddN(0.145f, 0.346f, 0.065f, 0.026f, TaggedText(OTag(TEXT("Overview.PlayerBlock.ExperienceLabel")), NSLOCTEXT("T66.Account", "ExperienceLabelFlatFixed", "EXPERIENCE"), 17, FT66FlatStyle::PrimaryText()));
			AddN(0.298f, 0.346f, 0.095f, 0.026f, TaggedText(OTag(TEXT("Overview.PlayerBlock.ExperienceValue")), UpperText(ProfileNextLevelText), 15, FT66FlatStyle::SecondaryText(), false, ETextJustify::Right));
			AddN(0.145f, 0.379f, 0.247f, 0.026f, MakeProgressVisual(OTag(TEXT("Overview.PlayerBlock.ExperienceProgress")), ExperiencePct, 0.247f));

			const FText StatusValue = bAccountEligible
				? NSLOCTEXT("T66.Account", "GoodStandingFlatFixed", "GOOD STANDING")
				: UpperText(RestrictionText(Restriction.Restriction));
			AddN(0.029f, 0.483f, 0.131f, 0.030f, TaggedText(OTag(TEXT("Overview.AccountStatusPanel.Header")), NSLOCTEXT("T66.Account", "StatusHeaderFlatFixed", "ACCOUNT STATUS"), 22, FT66FlatStyle::PrimaryText()));
			AddN(0.167f, 0.483f, 0.196f, 0.030f, TaggedText(OTag(TEXT("Overview.AccountStatusPanel.Status")), StatusValue, 22, bAccountEligible ? FT66FlatStyle::GoodStandingGreen() : FT66FlatStyle::SelectedText()));
			AddN(
				0.090f,
				0.525f,
				0.299f,
				0.080f,
				TaggedText(
					OTag(TEXT("Overview.AccountStatusPanel.Warning")),
					NSLOCTEXT("T66.Account", "OverviewWarningFlatFixed", "Your account is eligible for the leaderboard. If you cheat or manipulate runs or submissions, your account will be flagged and eternally removed from the leaderboard."),
					17,
					FT66FlatStyle::SecondaryText(),
					false,
					ETextJustify::Left,
					true));

			int32 OverviewProgressIndex = 0;
			auto AddProgressRow = [&](const TCHAR* RowTag, const FText& Label, const int32 Current, const int32 Total, const float X, const float Y, const float W, const float H, const float LabelW)
			{
				const FString BaseTag(RowTag);
				const float Percent = AccountPreviewProgress01(OverviewProgressIndex++);
				AddN(X, Y, W, H, MakeMetadataRegion(OTag(RowTag), TEXT("ProgressRow")));
				AddN(X, Y, LabelW, H * 0.90f, TaggedText(OTag(*FString::Printf(TEXT("%s.Label"), *BaseTag)), Label, 16, FT66FlatStyle::PrimaryText(), false));
				AddN(0.231f, Y, 0.115f, 0.025f, MakeProgressVisual(OTag(*FString::Printf(TEXT("%s.ProgressBar"), *BaseTag)), Percent, 0.115f));
				AddN(0.359f, Y, 0.026f, H * 0.90f, TaggedText(
					OTag(*FString::Printf(TEXT("%s.Value"), *BaseTag)),
					FText::Format(NSLOCTEXT("T66.Account", "CountFmtFlatFixed", "{0}/{1}"), FText::AsNumber(Current), FText::AsNumber(Total)),
					16,
					FT66FlatStyle::SelectedText(),
					true,
					ETextJustify::Right));
			};

			AddProgressRow(TEXT("Overview.AccountProgressPanel.Achievements"), NSLOCTEXT("T66.Account", "AchProgFlatFixed", "Achievements Unlocked"), UnlockedAchievements, AchievementDefs.Num(), 0.029f, 0.686f, 0.356f, 0.043f, 0.190f);
			AddProgressRow(TEXT("Overview.AccountProgressPanel.PowerUps"), NSLOCTEXT("T66.Account", "PowerProgFlatFixed", "Permanent Buffs Unlocked"), UnlockedPowerUps, PowerStats.Num() * UT66BuffSubsystem::MaxFillStepsPerStat, 0.029f, 0.737f, 0.356f, 0.043f, 0.190f);
			AddProgressRow(TEXT("Overview.AccountProgressPanel.Heroes"), NSLOCTEXT("T66.Account", "HeroProgFlatFixed", "Heroes Unlocked"), HeroIDs.Num(), HeroIDs.Num(), 0.029f, 0.788f, 0.356f, 0.043f, 0.190f);
			AddProgressRow(TEXT("Overview.AccountProgressPanel.Companions"), NSLOCTEXT("T66.Account", "CompProgFlatFixed", "Companions Unlocked"), UnlockedCompanions, CompanionIDs.Num(), 0.029f, 0.839f, 0.356f, 0.043f, 0.190f);
			AddProgressRow(TEXT("Overview.AccountProgressPanel.Challenges"), NSLOCTEXT("T66.Account", "ChallengeProgFlatFixed", "Challenges Completed"), DisplayChallengesCompleted, TotalChallengeCount, 0.029f, 0.890f, 0.356f, 0.043f, 0.190f);

			auto MakePBMenuEntry = [&OTag, &UpperText](
				const TCHAR* Tag,
				const FText& Label,
				const bool bActive,
				TFunction<void()> OnSelected) -> TSharedRef<SWidget>
			{
				return FT66FlatStyle::MakeFlatDropdownOptionButton(bActive ? ET66FlatState::Selected : ET66FlatState::Default, UpperText(Label), FOnClicked::CreateLambda([OnSelected = MoveTemp(OnSelected)]()
					{
						OnSelected();
						FSlateApplication::Get().DismissAllMenus();
						return FReply::Handled();
					}), 220.f, 38.f, 16, OTag(Tag), TEXT("OverviewFilterSelection"));
			};

			auto MakePBViewModeMenuFlat = [&]() -> TSharedRef<SWidget>
			{
				TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
				Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					MakePBMenuEntry(
						TEXT("Overview.FilterRow.PersonalBestDropdown.Option.PersonalBest"),
						NSLOCTEXT("T66.Account", "PBViewPersonalBestFlatFixed", "Personal Best"),
						ActivePBViewMode == EPersonalBestViewMode::PersonalBest,
						[this]() { ActivePBViewMode = EPersonalBestViewMode::PersonalBest; RequestDeferredSlateRebuild(); })
				];
				Menu->AddSlot().AutoHeight()
				[
					MakePBMenuEntry(
						TEXT("Overview.FilterRow.PersonalBestDropdown.Option.HighestRank"),
						NSLOCTEXT("T66.Account", "PBViewHighestRankFlatFixed", "Highest Rank"),
						ActivePBViewMode == EPersonalBestViewMode::HighestRank,
						[this]() { ActivePBViewMode = EPersonalBestViewMode::HighestRank; RequestDeferredSlateRebuild(); })
				];
				return Menu;
			};

			auto MakePBPartySizeMenuFlat = [&]() -> TSharedRef<SWidget>
			{
				TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
				for (ET66PartySize PartySize : { ET66PartySize::Solo, ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad })
				{
					const FString Tag = FString::Printf(TEXT("Overview.FilterRow.SoloDropdown.Option.%s"), *PartySizeToApiString(PartySize));
					Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[
						MakePBMenuEntry(
							*Tag,
							PartySizeText(Loc, PartySize),
							ActivePBPartySize == PartySize,
							[this, PartySize]() { ActivePBPartySize = PartySize; RequestDeferredSlateRebuild(); })
					];
				}
				return Menu;
			};

			auto GetPBViewModeTextFlat = [this, &UpperText]() -> FText
			{
				return UpperText(ActivePBViewMode == EPersonalBestViewMode::HighestRank
					? NSLOCTEXT("T66.Account", "PBViewHighestRankFlatValueFixed", "Highest Rank")
					: NSLOCTEXT("T66.Account", "PBViewPersonalBestFlatValueFixed", "Personal Best"));
			};

			AddN(0.441f, 0.230f, 0.252f, 0.056f, FT66FlatStyle::MakeFlatDropdown(
				ET66FlatState::Selected,
				TAttribute<FText>::CreateLambda([GetPBViewModeTextFlat]() { return GetPBViewModeTextFlat(); }),
				MakePBViewModeMenuFlat,
				true,
				0.252f * OverviewCanvasW,
				0.056f * OverviewCanvasH,
				20,
				OTag(TEXT("Overview.FilterRow.PersonalBestDropdown"))));
			AddN(0.713f, 0.230f, 0.254f, 0.056f, FT66FlatStyle::MakeFlatDropdown(
				ET66FlatState::Selected,
				TAttribute<FText>::CreateLambda([this, Loc, UpperText]() { return UpperText(PartySizeText(Loc, ActivePBPartySize)); }),
				MakePBPartySizeMenuFlat,
				true,
				0.254f * OverviewCanvasW,
				0.056f * OverviewCanvasH,
				20,
				OTag(TEXT("Overview.FilterRow.SoloDropdown"))));

			struct FOverviewPBCellData
			{
				FText Difficulty;
				FText Hero;
				FText Date;
				FText Primary;
				FText Secondary;
				bool bCanOpen = false;
				FString SlotName;
			};

			auto MakePBCellData = [&](const bool bTime, const ET66Difficulty Difficulty) -> FOverviewPBCellData
			{
				const FPersonalBestDisplay PB = bTime ? MakePBTime(Difficulty) : MakePBScore(Difficulty);
				const bool bCanOpen = PB.bHasRecord && !PB.RunSummarySlotName.IsEmpty();
				const FName PBHeroID = PB.bHasRecord ? ResolvePBHeroID(PB.RunSummarySlotName) : NAME_None;
				const FText HeroName = PB.bHasRecord
					? (PBHeroID.IsNone() ? NSLOCTEXT("T66.Account", "PBUnknownHeroFlatFixed", "Unknown") : HeroText(PBHeroID))
					: FText::GetEmpty();
				const FText Value = PB.bHasRecord ? (bTime ? FormatDurationText(PB.Seconds) : FText::AsNumber(PB.Score)) : NSLOCTEXT("T66.Account", "NoRecordFlatFixed", "--");
				const FText Date = PB.bHasRecord && PB.AchievedAtUtc.GetTicks() > 0 ? FText::FromString(PB.AchievedAtUtc.ToString(TEXT("%m/%d/%Y"))) : FText::GetEmpty();
				FText RankText = FText::GetEmpty();
				if (PB.bHasRecord)
				{
					if (!Backend || !Backend->IsBackendConfigured() || !Backend->HasSteamTicket())
					{
						RankText = NSLOCTEXT("T66.Account", "RankUnavailableFlatFixed", "N/A");
					}
					else if (!PB.bHasRankState)
					{
						RankText = NSLOCTEXT("T66.Account", "RankPendingFlatFixed", "...");
					}
					else if (!PB.bRankRequestSucceeded)
					{
						RankText = NSLOCTEXT("T66.Account", "RankFailedFlatFixed", "N/A");
					}
					else if (PB.GlobalRank > 0)
					{
						RankText = FText::Format(NSLOCTEXT("T66.Account", "RankFmtFlatFixed", "#{0}"), FText::AsNumber(PB.GlobalRank));
					}
					else
					{
						RankText = NSLOCTEXT("T66.Account", "RankUnrankedFlatFixed", "Unranked");
					}
				}

				const bool bRankInThirdColumn = ActivePBViewMode == EPersonalBestViewMode::PersonalBest;
				FOverviewPBCellData Data;
				Data.Difficulty = UpperText(DifficultyText(Difficulty));
				Data.Hero = HeroName;
				Data.Date = Date;
				Data.Primary = bRankInThirdColumn ? RankText : Value;
				Data.Secondary = bRankInThirdColumn ? Value : RankText;
				Data.bCanOpen = bCanOpen;
				Data.SlotName = PB.RunSummarySlotName;
				return Data;
			};

			auto AddPBTable = [&](
				const TCHAR* PanelTag,
				const TCHAR* TableTag,
				const TCHAR* IconTag,
				const TCHAR* HeaderTag,
				const FText& Title,
				const FSlateBrush* IconBrush,
				const bool bTime,
				const float PanelY,
				const float TableY,
				const float TableH,
				const float HeaderY,
				const float IconH,
				const TArray<float>& RowYs,
				const TArray<float>& RowHs)
			{
				AddN(0.451f, HeaderY, bTime ? 0.182f : 0.186f, 0.034f, TaggedText(OTag(HeaderTag), Title, 22, FT66FlatStyle::PrimaryText()));
				AddN(0.440f, TableY, 0.527f, TableH, MakeMetadataRegion(OTag(TableTag), TEXT("Table")));
				AddN(0.440f, TableY, 0.527f, bTime ? 0.033f : 0.030f, MakeMetadataRegion(OTag(*FString::Printf(TEXT("%s.TableHeader"), PanelTag)), TEXT("TableHeader")));

				const bool bRankInThirdColumn = ActivePBViewMode == EPersonalBestViewMode::PersonalBest;
				const FText ValueHeaderText = bTime ? NSLOCTEXT("T66.Account", "PBColTimeFlatFixed", "TIME") : NSLOCTEXT("T66.Account", "PBColScoreFlatFixed", "SCORE");
				const FText RankHeaderText = NSLOCTEXT("T66.Account", "PBColRankFlatFixed", "GLOBAL RANK");
				const float HeaderTextY = TableY + 0.004f;
				AddN(0.449f, HeaderTextY, 0.118f, 0.024f, PlainText(NSLOCTEXT("T66.Account", "PBColDifficultyFlatFixed", "DIFFICULTY"), 13, FT66FlatStyle::SelectedText(), true));
				AddN(0.586f, HeaderTextY, 0.082f, 0.024f, PlainText(NSLOCTEXT("T66.Account", "PBColHeroFlatFixed", "HERO"), 13, FT66FlatStyle::SelectedText(), true));
				AddN(0.682f, HeaderTextY, 0.082f, 0.024f, PlainText(NSLOCTEXT("T66.Account", "PBColDateFlatFixed", "DATE"), 13, FT66FlatStyle::SelectedText(), true));
				AddN(0.779f, HeaderTextY, 0.100f, 0.024f, PlainText(bRankInThirdColumn ? RankHeaderText : ValueHeaderText, 13, FT66FlatStyle::SelectedText(), true));
				AddN(0.895f, HeaderTextY, 0.062f, 0.024f, PlainText(bRankInThirdColumn ? ValueHeaderText : RankHeaderText, 13, FT66FlatStyle::SelectedText(), true, ETextJustify::Right));

				const ET66Difficulty Difficulties[] = {
					ET66Difficulty::Easy,
					ET66Difficulty::Medium,
					ET66Difficulty::Hard,
					ET66Difficulty::VeryHard,
					ET66Difficulty::Impossible
				};

				for (int32 RowIndex = 0; RowIndex < 5; ++RowIndex)
				{
					const FString RowTag = FString::Printf(TEXT("%s.Row%02d"), PanelTag, RowIndex + 1);
					const FOverviewPBCellData RowData = MakePBCellData(bTime, Difficulties[RowIndex]);
					const ET66FlatState RowState = ET66FlatState::Default;
					const TSharedRef<SWidget> RowSurface = RowData.bCanOpen
						? FT66FlatStyle::MakeFlatToggleGroupButton(
							RowState,
							SNew(SSpacer),
							FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOpenRunSummaryClicked, RowData.SlotName),
							FMargin(0.f),
							0.527f * OverviewCanvasW,
							RowHs[RowIndex] * OverviewCanvasH,
							true,
							OTag(*RowTag))
						: FT66FlatStyle::MakeFlatSubPanel(
							RowState,
							FMargin(0.f),
							SNew(SSpacer),
							nullptr,
							OTag(*RowTag));

					AddN(0.440f, RowYs[RowIndex], 0.527f, RowHs[RowIndex], RowSurface);
					const float TextY = RowYs[RowIndex] + 0.006f;
					AddN(0.449f, TextY, 0.118f, 0.024f, PlainText(RowData.Difficulty, 13, FT66FlatStyle::PrimaryText(), true));
					AddN(0.586f, TextY, 0.082f, 0.024f, PlainText(RowData.Hero, 13, RowData.Hero.IsEmpty() ? FT66FlatStyle::SecondaryText() : FT66FlatStyle::PrimaryText()));
					AddN(0.682f, TextY, 0.082f, 0.024f, PlainText(RowData.Date, 13, FT66FlatStyle::SecondaryText()));
					AddN(0.779f, TextY, 0.100f, 0.024f, PlainText(RowData.Primary, 13, bRankInThirdColumn ? FT66FlatStyle::PrimaryText() : FT66FlatStyle::SelectedText(), true));
					AddN(0.895f, TextY, 0.062f, 0.024f, PlainText(RowData.Secondary, 13, bRankInThirdColumn ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText(), true, ETextJustify::Right));
				}
			};

			AddPBTable(
				TEXT("Overview.HighestScorePanel"),
				TEXT("Overview.HighestScorePanel.Table"),
				TEXT("Overview.HighestScorePanel.Icon"),
				TEXT("Overview.HighestScorePanel.Header"),
				NSLOCTEXT("T66.Account", "TopScoreFlatFixed", "HIGHEST SCORE"),
				TrophyBrush,
				false,
				0.308f,
				0.373f,
				0.214f,
				0.332f,
				0.047f,
				TArray<float>{ 0.403f, 0.440f, 0.476f, 0.513f, 0.550f },
				TArray<float>{ 0.036f, 0.036f, 0.036f, 0.036f, 0.037f });
			AddPBTable(
				TEXT("Overview.BestSpeedRunPanel"),
				TEXT("Overview.BestSpeedRunPanel.Table"),
				TEXT("Overview.BestSpeedRunPanel.Icon"),
				TEXT("Overview.BestSpeedRunPanel.Header"),
				NSLOCTEXT("T66.Account", "TopTimeFlatFixed", "BEST SPEED RUN"),
				StopwatchBrush,
				true,
				0.616f,
				0.681f,
				0.231f,
				0.642f,
				0.052f,
				TArray<float>{ 0.714f, 0.751f, 0.789f, 0.832f, 0.874f },
				TArray<float>{ 0.037f, 0.037f, 0.037f, 0.037f, 0.038f });

			TSharedRef<SWidget> OverviewContent = SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
				]
				+ SOverlay::Slot()
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					[
						SNew(SBox)
						.WidthOverride(OverviewCanvasW)
						.HeightOverride(OverviewCanvasH)
						[
							OverviewCanvas
						]
					]
				];

			return OverviewContent;
		}

		auto OTag = [](const TCHAR* Tag) -> FName
		{
			return FName(Tag);
		};

		auto UpperText = [](const FText& Text) -> FText
		{
			FString Value = Text.ToString();
			Value.ToUpperInline();
			return FText::FromString(Value);
		};

		auto TaggedText = [&OTag](
			const TCHAR* Tag,
			const FText& Text,
			const int32 FontSize,
			const FLinearColor& Color,
			const bool bBold = true,
			const ETextJustify::Type Justification = ETextJustify::Left,
			const bool bWrap = false) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(STextBlock)
				.Text(Text)
				.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
				.ColorAndOpacity(Color)
				.Justification(Justification)
				.AutoWrapText(bWrap)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds),
				OTag(Tag),
				TEXT("Label"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true);
		};

		auto TaggedBox = [&OTag](const TCHAR* Tag, const TSharedRef<SWidget>& Content, const FString& Role = TEXT("Region")) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				[
					Content
				],
				OTag(Tag),
				Role,
				ET66FlatState::Default);
		};

		auto MakeIcon = [&OTag, &TaggedText](
			const TCHAR* Tag,
			const FSlateBrush* Brush,
			const FVector2D& Size,
			const FText& FallbackText) -> TSharedRef<SWidget>
		{
			const TSharedRef<SWidget> IconContent = Brush
				? StaticCastSharedRef<SWidget>(SNew(SImage).Image(Brush).ColorAndOpacity(FLinearColor::White))
				: TaggedText(TEXT("Overview.Internal.IconFallback"), FallbackText, 16, FT66FlatStyle::PrimaryText(), true, ETextJustify::Center);

			return FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.WidthOverride(Size.X)
				.HeightOverride(Size.Y)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					IconContent
				],
				OTag(Tag),
				TEXT("Icon"),
				ET66FlatState::Default);
		};

		const FSlateBrush* InfoBrush = AccountFlatIconBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/info.png"), FVector2D(24.f, 24.f), TEXT("OverviewInfoIcon"));
		const FSlateBrush* ShieldBrush = AccountFlatIconBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/shield.png"), FVector2D(40.f, 40.f), TEXT("OverviewShieldIcon"));
		const FSlateBrush* ChartBrush = AccountFlatIconBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/bar_chart.png"), FVector2D(40.f, 40.f), TEXT("OverviewBarChartIcon"));
		const FSlateBrush* TrophyBrush = AccountFlatIconBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/trophy_laurel.png"), FVector2D(40.f, 40.f), TEXT("OverviewTrophyIcon"));
		const FSlateBrush* StopwatchBrush = AccountFlatIconBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/stopwatch.png"), FVector2D(40.f, 40.f), TEXT("OverviewStopwatchIcon"));

		auto MakeInfoIcon = [InfoBrush](const TCHAR* Tag, const FText& TooltipText, FOnClicked OnClicked) -> TSharedPtr<SWidget>
		{
			return FT66FlatStyle::MakeFlatTooltipIcon(
				ET66FlatState::Default,
				InfoBrush,
				TooltipText,
				FVector2D(22.f, 22.f),
				FName(Tag),
				MoveTemp(OnClicked));
		};

		FT66FlatToggleGroupParams AccountTabs;
		AccountTabs.GroupName = TEXT("AccountTabs");
		AccountTabs.bMutuallyExclusive = true;

		FT66FlatToggleGroupItem OverviewTab;
		OverviewTab.State = ET66FlatState::Default;
		OverviewTab.bIsSelected = true;
		OverviewTab.Label = NSLOCTEXT("T66.Account", "OverviewTabFlat", "OVERVIEW");
		OverviewTab.OnClicked = FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOverviewTabClicked);
		OverviewTab.OptionalRightIcon = MakeInfoIcon(TEXT("Overview.SubTabs.OverviewInfoIcon"), NSLOCTEXT("T66.Account", "OverviewTabInfoTooltip", "Account overview"), FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOverviewTabClicked));
		OverviewTab.MinWidth = 268.f;
		OverviewTab.Height = 54.f;
		OverviewTab.FontSize = 24;
		OverviewTab.Tag = TEXT("Overview.SubTabs.OverviewButton");
		AccountTabs.Items.Add(OverviewTab);

		FT66FlatToggleGroupItem HistoryTab;
		HistoryTab.State = ET66FlatState::Default;
		HistoryTab.bIsSelected = false;
		HistoryTab.Label = NSLOCTEXT("T66.Account", "HistoryTabFlat", "HISTORY");
		HistoryTab.OnClicked = FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleHistoryTabClicked);
		HistoryTab.OptionalRightIcon = MakeInfoIcon(TEXT("Overview.SubTabs.HistoryInfoIcon"), NSLOCTEXT("T66.Account", "HistoryTabInfoTooltip", "Run history"), FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleHistoryTabClicked));
		HistoryTab.MinWidth = 268.f;
		HistoryTab.Height = 54.f;
		HistoryTab.FontSize = 24;
		HistoryTab.Tag = TEXT("Overview.SubTabs.HistoryButton");
		AccountTabs.Items.Add(HistoryTab);

		const TArray<TSharedRef<SWidget>> AccountTabButtons = FT66FlatStyle::MakeFlatToggleGroup(AccountTabs);
		const TSharedRef<SWidget> SubTabs = TaggedBox(
			TEXT("Overview.SubTabs"),
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
			[
				AccountTabButtons[0]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				AccountTabButtons[1]
			],
			TEXT("ToggleGroup.AccountTabs"));

		const FText OverviewProfileNameText = !LocalSteamName.IsEmpty()
			? FText::FromString(LocalSteamName)
			: NSLOCTEXT("T66.Account", "ProfileNameRandomChad", "RandomChad");
		const float ExperiencePct = ProfileExperienceToNextLevel > 0
			? FMath::Clamp(static_cast<float>(ProfileExperienceIntoLevel) / static_cast<float>(ProfileExperienceToNextLevel), 0.f, 1.f)
			: 0.f;
		const FSlateBrush* ProfileBrush = ProfileAvatarBrush.IsValid() && ProfileAvatarBrush->GetResourceObject()
			? ProfileAvatarBrush.Get()
			: nullptr;

		const TSharedRef<SWidget> PlayerBlock = FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(18.f, 16.f),
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				FT66FlatStyle::MakeFlatPortraitSlot(ET66FlatState::Default, ProfileBrush, nullptr, FVector2D(92.f, 92.f), OTag(TEXT("Overview.PlayerBlock.Avatar")))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(18.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					TaggedText(TEXT("Overview.PlayerBlock.Name"), OverviewProfileNameText, 26, FT66FlatStyle::PrimaryText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 5.f, 0.f, 0.f)
				[
					TaggedText(TEXT("Overview.PlayerBlock.Level"), UpperText(ProfileLevelText), 18, FT66FlatStyle::SelectedText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						TaggedText(TEXT("Overview.PlayerBlock.ExperienceLabel"), NSLOCTEXT("T66.Account", "ExperienceLabelFlat", "EXPERIENCE"), 17, FT66FlatStyle::PrimaryText())
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						TaggedText(TEXT("Overview.PlayerBlock.ExperienceValue"), UpperText(ProfileNextLevelText), 15, FT66FlatStyle::SecondaryText(), false, ETextJustify::Right)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 7.f, 0.f, 0.f)
				[
					FT66FlatStyle::MakeFlatProgressBar(TAttribute<float>(ExperiencePct), TOptional<FLinearColor>(FT66FlatStyle::SelectedBorder()), OTag(TEXT("Overview.PlayerBlock.ExperienceProgress")))
				]
			],
			nullptr,
			OTag(TEXT("Overview.PlayerBlock")));

		const FText StatusValue = bAccountEligible
			? NSLOCTEXT("T66.Account", "GoodStandingFlat", "GOOD STANDING")
			: UpperText(RestrictionText(Restriction.Restriction));
		const TSharedRef<SWidget> StatusPanel = FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(18.f, 16.f),
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f).VAlign(VAlign_Top)
			[
				MakeIcon(TEXT("Overview.AccountStatusPanel.Icon"), ShieldBrush, FVector2D(42.f, 42.f), FText::FromString(TEXT("S")))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						TaggedText(TEXT("Overview.AccountStatusPanel.Header"), NSLOCTEXT("T66.Account", "StatusHeaderFlat", "ACCOUNT STATUS"), 22, FT66FlatStyle::PrimaryText())
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f, 10.f, 0.f)
					[
						TaggedText(TEXT("Overview.AccountStatusPanel.Separator"), FText::FromString(TEXT("-")), 22, FT66FlatStyle::SecondaryText())
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						TaggedText(TEXT("Overview.AccountStatusPanel.Status"), StatusValue, 22, bAccountEligible ? FT66FlatStyle::GoodStandingGreen() : FT66FlatStyle::SelectedText())
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 13.f, 0.f, 0.f)
				[
					TaggedText(
						TEXT("Overview.AccountStatusPanel.Warning"),
						NSLOCTEXT("T66.Account", "OverviewWarningFlat", "Your account is eligible for the leaderboard. If you cheat or manipulate runs or submissions, your account will be flagged and eternally removed from the leaderboard."),
						17,
						FT66FlatStyle::SecondaryText(),
						false,
						ETextJustify::Left,
						true)
				]
			],
			nullptr,
			OTag(TEXT("Overview.AccountStatusPanel")));

		int32 OverviewProgressIndex = 0;
		auto MakeOverviewProgressRow = [&](
			const TCHAR* RowTag,
			const FText& Label,
			const int32 Current,
			const int32 Total) -> TSharedRef<SWidget>
		{
			const float Percent = AccountPreviewProgress01(OverviewProgressIndex++);
			const FString BaseTag(RowTag);
			return FT66FlatStyle::AttachMetadata(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						TaggedText(*FString::Printf(TEXT("%s.Label"), *BaseTag), Label, 16, FT66FlatStyle::PrimaryText(), false)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						TaggedText(
							*FString::Printf(TEXT("%s.Value"), *BaseTag),
							FText::Format(NSLOCTEXT("T66.Account", "CountFmtFlat", "{0}/{1}"), FText::AsNumber(Current), FText::AsNumber(Total)),
							16,
							FT66FlatStyle::SelectedText(),
							true,
							ETextJustify::Right)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
				[
					FT66FlatStyle::MakeFlatProgressBar(
						TAttribute<float>(Percent),
						TOptional<FLinearColor>(FT66FlatStyle::SelectedBorder()),
						OTag(*FString::Printf(TEXT("%s.ProgressBar"), *BaseTag)))
				],
				OTag(RowTag),
				TEXT("ProgressRow"),
				ET66FlatState::Default);
		};

		const TSharedRef<SWidget> ProgressPanel = FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(18.f, 16.f),
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 13.f, 0.f).VAlign(VAlign_Center)
				[
					MakeIcon(TEXT("Overview.AccountProgressPanel.Icon"), ChartBrush, FVector2D(38.f, 38.f), FText::FromString(TEXT("#")))
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					TaggedText(TEXT("Overview.AccountProgressPanel.Header"), NSLOCTEXT("T66.Account", "ProgressHeaderFlat", "ACCOUNT PROGRESS"), 22, FT66FlatStyle::PrimaryText())
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
			[
				MakeOverviewProgressRow(TEXT("Overview.AccountProgressPanel.Achievements"), NSLOCTEXT("T66.Account", "AchProgFlat", "Achievements Unlocked"), UnlockedAchievements, AchievementDefs.Num())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				MakeOverviewProgressRow(TEXT("Overview.AccountProgressPanel.PowerUps"), NSLOCTEXT("T66.Account", "PowerProgFlat", "Permanent Buffs Unlocked"), UnlockedPowerUps, PowerStats.Num() * UT66BuffSubsystem::MaxFillStepsPerStat)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				MakeOverviewProgressRow(TEXT("Overview.AccountProgressPanel.Heroes"), NSLOCTEXT("T66.Account", "HeroProgFlat", "Heroes Unlocked"), HeroIDs.Num(), HeroIDs.Num())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				MakeOverviewProgressRow(TEXT("Overview.AccountProgressPanel.Companions"), NSLOCTEXT("T66.Account", "CompProgFlat", "Companions Unlocked"), UnlockedCompanions, CompanionIDs.Num())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
			[
				MakeOverviewProgressRow(TEXT("Overview.AccountProgressPanel.Challenges"), NSLOCTEXT("T66.Account", "ChallengeProgFlat", "Challenges Completed"), DisplayChallengesCompleted, TotalChallengeCount)
			],
			nullptr,
			OTag(TEXT("Overview.AccountProgressPanel")));

		auto MakePBMenuEntry = [&OTag, &UpperText](
			const TCHAR* Tag,
			const FText& Label,
			const bool bActive,
			TFunction<void()> OnSelected) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatDropdownOptionButton(bActive ? ET66FlatState::Selected : ET66FlatState::Default, UpperText(Label), FOnClicked::CreateLambda([OnSelected = MoveTemp(OnSelected)]()
				{
					OnSelected();
					FSlateApplication::Get().DismissAllMenus();
					return FReply::Handled();
				}), 220.f, 38.f, 16, OTag(Tag), TEXT("OverviewFilterSelection"));
		};

		auto MakePBViewModeMenuFlat = [&]() -> TSharedRef<SWidget>
		{
			TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakePBMenuEntry(
					TEXT("Overview.FilterRow.PersonalBestDropdown.Option.PersonalBest"),
					NSLOCTEXT("T66.Account", "PBViewPersonalBestFlat", "Personal Best"),
					ActivePBViewMode == EPersonalBestViewMode::PersonalBest,
					[this]() { ActivePBViewMode = EPersonalBestViewMode::PersonalBest; RequestDeferredSlateRebuild(); })
			];
			Menu->AddSlot().AutoHeight()
			[
				MakePBMenuEntry(
					TEXT("Overview.FilterRow.PersonalBestDropdown.Option.HighestRank"),
					NSLOCTEXT("T66.Account", "PBViewHighestRankFlat", "Highest Rank"),
					ActivePBViewMode == EPersonalBestViewMode::HighestRank,
					[this]() { ActivePBViewMode = EPersonalBestViewMode::HighestRank; RequestDeferredSlateRebuild(); })
			];
			return Menu;
		};

		auto MakePBPartySizeMenuFlat = [&]() -> TSharedRef<SWidget>
		{
			TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
			for (ET66PartySize PartySize : { ET66PartySize::Solo, ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad })
			{
				const FString Tag = FString::Printf(TEXT("Overview.FilterRow.SoloDropdown.Option.%s"), *PartySizeToApiString(PartySize));
				Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					MakePBMenuEntry(
						*Tag,
						PartySizeText(Loc, PartySize),
						ActivePBPartySize == PartySize,
						[this, PartySize]() { ActivePBPartySize = PartySize; RequestDeferredSlateRebuild(); })
				];
			}
			return Menu;
		};

		auto GetPBViewModeTextFlat = [this, &UpperText]() -> FText
		{
			return UpperText(ActivePBViewMode == EPersonalBestViewMode::HighestRank
				? NSLOCTEXT("T66.Account", "PBViewHighestRankFlatValue", "Highest Rank")
				: NSLOCTEXT("T66.Account", "PBViewPersonalBestFlatValue", "Personal Best"));
		};

		const TSharedRef<SWidget> FilterRow = TaggedBox(
			TEXT("Overview.FilterRow"),
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 14.f, 0.f)
			[
				FT66FlatStyle::MakeFlatDropdown(
					ET66FlatState::Selected,
					TAttribute<FText>::CreateLambda([GetPBViewModeTextFlat]() { return GetPBViewModeTextFlat(); }),
					MakePBViewModeMenuFlat,
					true,
					260.f,
					50.f,
					20,
					OTag(TEXT("Overview.FilterRow.PersonalBestDropdown")))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				FT66FlatStyle::MakeFlatDropdown(
					ET66FlatState::Selected,
					TAttribute<FText>::CreateLambda([this, Loc, UpperText]() { return UpperText(PartySizeText(Loc, ActivePBPartySize)); }),
					MakePBPartySizeMenuFlat,
					true,
					220.f,
					50.f,
					20,
					OTag(TEXT("Overview.FilterRow.SoloDropdown")))
			],
			TEXT("FilterRow"));

		auto MakeTableText = [&TaggedText](const TCHAR* Tag, const FText& Text, const int32 Size, const FLinearColor& Color, const bool bBold = false, const ETextJustify::Type Justification = ETextJustify::Left) -> TSharedRef<SWidget>
		{
			return TaggedText(Tag, Text, Size, Color, bBold, Justification);
		};

		auto MakePBTableRowFlat = [&](
			const TCHAR* RowTag,
			const bool bTime,
			const ET66Difficulty Difficulty,
			const int32 RowIndex) -> TSharedRef<SWidget>
		{
			const FPersonalBestDisplay PB = bTime ? MakePBTime(Difficulty) : MakePBScore(Difficulty);
			const bool bCanOpen = PB.bHasRecord && !PB.RunSummarySlotName.IsEmpty();
			const FName PBHeroID = PB.bHasRecord ? ResolvePBHeroID(PB.RunSummarySlotName) : NAME_None;
			const FText HeroName = PB.bHasRecord
				? (PBHeroID.IsNone() ? NSLOCTEXT("T66.Account", "PBUnknownHeroFlat", "Unknown") : HeroText(PBHeroID))
				: FText::GetEmpty();
			const FText Value = PB.bHasRecord ? (bTime ? FormatDurationText(PB.Seconds) : FText::AsNumber(PB.Score)) : NSLOCTEXT("T66.Account", "NoRecordFlat", "--");
			const FText Date = PB.bHasRecord && PB.AchievedAtUtc.GetTicks() > 0 ? FText::FromString(PB.AchievedAtUtc.ToString(TEXT("%m/%d/%Y"))) : FText::GetEmpty();
			FText RankText = FText::GetEmpty();
			if (PB.bHasRecord)
			{
				if (!Backend || !Backend->IsBackendConfigured() || !Backend->HasSteamTicket())
				{
					RankText = NSLOCTEXT("T66.Account", "RankUnavailableFlat", "N/A");
				}
				else if (!PB.bHasRankState)
				{
					RankText = NSLOCTEXT("T66.Account", "RankPendingFlat", "...");
				}
				else if (!PB.bRankRequestSucceeded)
				{
					RankText = NSLOCTEXT("T66.Account", "RankFailedFlat", "N/A");
				}
				else if (PB.GlobalRank > 0)
				{
					RankText = FText::Format(NSLOCTEXT("T66.Account", "RankFmtFlat", "#{0}"), FText::AsNumber(PB.GlobalRank));
				}
				else
				{
					RankText = NSLOCTEXT("T66.Account", "RankUnrankedFlat", "Unranked");
				}
			}

			const bool bRankInThirdColumn = ActivePBViewMode == EPersonalBestViewMode::PersonalBest;
			const FText ThirdColumnText = bRankInThirdColumn ? RankText : Value;
			const FText FourthColumnText = bRankInThirdColumn ? Value : RankText;
			const FString BaseTag(RowTag);
			TSharedRef<SWidget> RowContent = SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.90f).VAlign(VAlign_Center)
				[
					MakeTableText(*FString::Printf(TEXT("%s.Difficulty"), *BaseTag), UpperText(DifficultyText(Difficulty)), 14, FT66FlatStyle::PrimaryText(), true)
				]
				+ SHorizontalBox::Slot().FillWidth(1.00f).VAlign(VAlign_Center)
				[
					MakeTableText(*FString::Printf(TEXT("%s.Hero"), *BaseTag), HeroName, 13, PB.bHasRecord ? FT66FlatStyle::PrimaryText() : FT66FlatStyle::SecondaryText())
				]
				+ SHorizontalBox::Slot().FillWidth(0.95f).VAlign(VAlign_Center)
				[
					MakeTableText(*FString::Printf(TEXT("%s.Date"), *BaseTag), Date, 13, FT66FlatStyle::SecondaryText())
				]
				+ SHorizontalBox::Slot().FillWidth(0.95f).VAlign(VAlign_Center)
				[
					MakeTableText(*FString::Printf(TEXT("%s.PrimaryValue"), *BaseTag), ThirdColumnText, 14, bRankInThirdColumn ? FT66FlatStyle::PrimaryText() : FT66FlatStyle::SelectedText(), true)
				]
				+ SHorizontalBox::Slot().FillWidth(1.00f).VAlign(VAlign_Center)
				[
					MakeTableText(*FString::Printf(TEXT("%s.SecondaryValue"), *BaseTag), FourthColumnText, 14, bRankInThirdColumn ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText(), true)
				];

			if (bCanOpen)
			{
				return FT66FlatStyle::MakeFlatToggleGroupButton(
					RowIndex == 0 ? ET66FlatState::Selected : ET66FlatState::Default,
					RowContent,
					FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOpenRunSummaryClicked, PB.RunSummarySlotName),
					FMargin(9.f, 5.f),
					0.f,
					40.f,
					true,
					OTag(RowTag));
			}

			return FT66FlatStyle::MakeFlatSubPanel(
				RowIndex == 0 ? ET66FlatState::Selected : ET66FlatState::Default,
				FMargin(9.f, 5.f),
				RowContent,
				nullptr,
				OTag(RowTag));
		};

		auto MakePBBlockFlat = [&](
			const TCHAR* PanelTag,
			const TCHAR* IconTag,
			const TCHAR* HeaderTag,
			const FText& Title,
			const FSlateBrush* IconBrush,
			const bool bTime) -> TSharedRef<SWidget>
		{
			const bool bRankInThirdColumn = ActivePBViewMode == EPersonalBestViewMode::PersonalBest;
			const FText ValueHeaderText = bTime ? NSLOCTEXT("T66.Account", "PBColTimeFlat", "TIME") : NSLOCTEXT("T66.Account", "PBColScoreFlat", "SCORE");
			const FText RankHeaderText = NSLOCTEXT("T66.Account", "PBColRankFlat", "GLOBAL RANK");
			TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
			int32 RowIndex = 0;
			for (ET66Difficulty Difficulty : { ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible })
			{
				const FString RowTag = FString::Printf(TEXT("%s.Row%02d"), PanelTag, RowIndex + 1);
				Rows->AddSlot().AutoHeight().Padding(0.f, RowIndex > 0 ? 4.f : 0.f, 0.f, 0.f)
				[
					MakePBTableRowFlat(*RowTag, bTime, Difficulty, RowIndex)
				];
				++RowIndex;
			}

			const FString HeaderRowTag = FString::Printf(TEXT("%s.TableHeader"), PanelTag);
			return FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(14.f, 13.f),
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f).VAlign(VAlign_Center)
					[
						MakeIcon(IconTag, IconBrush, FVector2D(36.f, 36.f), bTime ? FText::FromString(TEXT("T")) : FText::FromString(TEXT("#")))
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						TaggedText(HeaderTag, Title, 22, FT66FlatStyle::PrimaryText())
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 6.f)
				[
					FT66FlatStyle::AttachMetadata(
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(0.90f)[MakeTableText(*FString::Printf(TEXT("%s.Difficulty"), *HeaderRowTag), NSLOCTEXT("T66.Account", "PBColDifficultyFlat", "DIFFICULTY"), 13, FT66FlatStyle::SelectedText(), true)]
						+ SHorizontalBox::Slot().FillWidth(1.00f)[MakeTableText(*FString::Printf(TEXT("%s.Hero"), *HeaderRowTag), NSLOCTEXT("T66.Account", "PBColHeroFlat", "HERO"), 13, FT66FlatStyle::SelectedText(), true)]
						+ SHorizontalBox::Slot().FillWidth(0.95f)[MakeTableText(*FString::Printf(TEXT("%s.Date"), *HeaderRowTag), NSLOCTEXT("T66.Account", "PBColDateFlat", "DATE"), 13, FT66FlatStyle::SelectedText(), true)]
						+ SHorizontalBox::Slot().FillWidth(0.95f)[MakeTableText(*FString::Printf(TEXT("%s.Primary"), *HeaderRowTag), bRankInThirdColumn ? RankHeaderText : ValueHeaderText, 13, FT66FlatStyle::SelectedText(), true)]
						+ SHorizontalBox::Slot().FillWidth(1.00f)[MakeTableText(*FString::Printf(TEXT("%s.Secondary"), *HeaderRowTag), bRankInThirdColumn ? ValueHeaderText : RankHeaderText, 13, FT66FlatStyle::SelectedText(), true)],
						OTag(*HeaderRowTag),
						TEXT("TableHeader"),
						ET66FlatState::Default)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					Rows
				],
				nullptr,
				OTag(PanelTag));
		};

		const TSharedRef<SWidget> RightColumn = FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(14.f),
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				FilterRow
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
			[
				MakePBBlockFlat(
					TEXT("Overview.HighestScorePanel"),
					TEXT("Overview.HighestScorePanel.Icon"),
					TEXT("Overview.HighestScorePanel.Header"),
					NSLOCTEXT("T66.Account", "TopScoreFlat", "HIGHEST SCORE"),
					TrophyBrush,
					false)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
			[
				MakePBBlockFlat(
					TEXT("Overview.BestSpeedRunPanel"),
					TEXT("Overview.BestSpeedRunPanel.Icon"),
					TEXT("Overview.BestSpeedRunPanel.Header"),
					NSLOCTEXT("T66.Account", "TopTimeFlat", "BEST SPEED RUN"),
					StopwatchBrush,
					true)
			],
			nullptr,
			OTag(TEXT("Overview.RightColumn")));

		const TSharedRef<SWidget> LeftColumn = TaggedBox(
			TEXT("Overview.LeftColumn"),
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				PlayerBlock
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
			[
				StatusPanel
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
			[
				ProgressPanel
			],
			TEXT("Column"));

		const TSharedRef<SWidget> Body = TaggedBox(
			TEXT("Overview.MainBody"),
			bUseStackedOverviewLayout
				? StaticCastSharedRef<SWidget>(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						LeftColumn
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
					[
						RightColumn
					])
				: StaticCastSharedRef<SWidget>(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.42f).Padding(0.f, 0.f, 18.f, 0.f)
					[
						LeftColumn
					]
					+ SHorizontalBox::Slot().FillWidth(0.58f)
					[
						RightColumn
					]),
			TEXT("Body"));

		const TSharedRef<SWidget> OverviewContent = FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
			.Padding(FMargin(20.f, ScreenLayout.bCompact ? 8.f : 10.f, 20.f, ScreenLayout.bCompact ? 8.f : 10.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SubTabs
				]
				+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 16.f, 0.f, 0.f)
				[
					Body
				]
			],
			OTag(TEXT("Overview.Root")),
			TEXT("Root"),
			ET66FlatState::Default);

		TSharedRef<SWidget> BackgroundContent = SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black);

		return FT66FlatStyle::MakeTopBarScreenRoot(
			UIManager,
			OverviewContent,
			BackgroundContent);
	}

	if (ActiveTab == EAccountTab::History)
	{
		auto HTag = [](const TCHAR* Tag) -> FName
		{
			return FName(Tag);
		};

		auto UpperText = [](const FText& Text) -> FText
		{
			FString Value = Text.ToString();
			Value.ToUpperInline();
			return FText::FromString(Value);
		};

		constexpr float HistoryCanvasW = 1920.f;
		constexpr float HistoryCanvasH = 1080.f;
		const FButtonStyle& NoBorderButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("NoBorder"));
		const TSharedRef<SConstraintCanvas> HistoryCanvas = SNew(SConstraintCanvas);

		auto AddCanvas = [HistoryCanvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			HistoryCanvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X, Y, W, H))
			[
				Widget
			];
		};

		auto AddN = [&AddCanvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			AddCanvas(X * HistoryCanvasW, Y * HistoryCanvasH, W * HistoryCanvasW, H * HistoryCanvasH, Widget);
		};

		auto MakeMetadataRegion = [](const FName Tag, const FString& Role, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(SNew(SSpacer), Tag, Role, State);
		};

		auto MakePanelSurface = [](const FName Tag, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatPanel(
				State,
				FMargin(0.f),
				SNew(SSpacer),
				nullptr,
				Tag);
		};

		auto TaggedText = [](
			const FName Tag,
			const FText& Text,
			const int32 FontSize,
			const FLinearColor& Color,
			const bool bBold = true,
			const ETextJustify::Type Justification = ETextJustify::Left,
			const bool bWrap = false) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(STextBlock)
				.Visibility(EVisibility::HitTestInvisible)
				.Text(Text)
				.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
				.ColorAndOpacity(Color)
				.Justification(Justification)
				.AutoWrapText(bWrap)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds),
				Tag,
				TEXT("Label"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true);
		};

		auto PlainText = [](
			const FText& Text,
			const int32 FontSize,
			const FLinearColor& Color,
			const bool bBold = true,
			const ETextJustify::Type Justification = ETextJustify::Left) -> TSharedRef<SWidget>
		{
			return SNew(STextBlock)
				.Visibility(EVisibility::HitTestInvisible)
				.Text(Text)
				.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
				.ColorAndOpacity(Color)
				.Justification(Justification)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds);
		};

		auto MakeIcon = [](const FName Tag, const FSlateBrush* Brush, const FText& FallbackText) -> TSharedRef<SWidget>
		{
			const TSharedRef<SWidget> IconContent = Brush
				? StaticCastSharedRef<SWidget>(SNew(SImage).Visibility(EVisibility::HitTestInvisible).Image(Brush).ColorAndOpacity(FLinearColor::White))
				: StaticCastSharedRef<SWidget>(
					SNew(STextBlock)
					.Visibility(EVisibility::HitTestInvisible)
					.Text(FallbackText)
					.Font(FT66FlatStyle::MakeBoldFont(18))
					.ColorAndOpacity(FT66FlatStyle::PrimaryText())
					.Justification(ETextJustify::Center));

			return FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.Visibility(EVisibility::HitTestInvisible)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					IconContent
				],
				Tag,
				TEXT("Icon"),
				ET66FlatState::Default);
		};

		auto MakeBareInteractive = [&NoBorderButtonStyle](
			const FName Tag,
			const FString& Role,
			const TSharedRef<SWidget>& Content,
			FOnClicked OnClicked,
			const ET66FlatState State = ET66FlatState::Default,
			const FName ToggleGroup = NAME_None) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				FT66FlatStyle::MakeBareButton(
					FT66BareButtonParams(MoveTemp(OnClicked), Content)
					.SetButtonStyle(&NoBorderButtonStyle)
					.SetPadding(FMargin(0.f))
					.SetDebounceClick(false)),
				Tag,
				Role,
				State,
				TOptional<FLinearColor>(),
				true,
				ToggleGroup,
				false,
				State != ET66FlatState::Disabled);
		};

		const FSlateBrush* InfoBrush = AccountFlatIconBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/info.png"), FVector2D(24.f, 24.f), TEXT("HistoryInfoIcon"));

		AddN(0.013f, 0.123f, 0.974f, 0.838f, MakeMetadataRegion(HTag(TEXT("History.Root")), TEXT("Root")));
		AddN(0.013f, 0.201f, 0.974f, 0.760f, MakeMetadataRegion(HTag(TEXT("History.MainBody")), TEXT("Body")));
		AddN(0.148f, 0.123f, 0.690f, 0.060f, MakeMetadataRegion(HTag(TEXT("History.SubTabs")), TEXT("ToggleGroup.AccountTabs")));
		AddN(0.011f, 0.201f, 0.977f, 0.168f, MakePanelSurface(HTag(TEXT("History.FilterPanel"))));
		AddN(0.011f, 0.393f, 0.977f, 0.568f, MakePanelSurface(HTag(TEXT("History.RunHistoryPanel"))));

		AddN(
			0.148f,
			0.123f,
			0.320f,
			0.060f,
			FT66FlatStyle::MakeFlatToggleGroupButton(
				ET66FlatState::Default,
				SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					PlainText(NSLOCTEXT("T66.Account", "HistoryOverviewTabFlat", "OVERVIEW"), 24, FT66FlatStyle::PrimaryText(), true, ETextJustify::Center)
				],
				FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOverviewTabClicked),
				FMargin(0.f),
				0.320f * HistoryCanvasW,
				0.060f * HistoryCanvasH,
				true,
				HTag(TEXT("History.SubTabs.OverviewButton")),
				HTag(TEXT("AccountTabs"))));
		AddN(
			0.498f,
			0.123f,
			0.340f,
			0.060f,
			FT66FlatStyle::MakeFlatToggleGroupButton(
				ET66FlatState::Selected,
				SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					PlainText(NSLOCTEXT("T66.Account", "HistoryHistoryTabFlat", "HISTORY"), 24, FT66FlatStyle::SelectedText(), true, ETextJustify::Center)
				],
				FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleHistoryTabClicked),
				FMargin(0.f),
				0.340f * HistoryCanvasW,
				0.060f * HistoryCanvasH,
				true,
				HTag(TEXT("History.SubTabs.HistoryButton")),
				HTag(TEXT("AccountTabs"))));

		AddN(0.438f, 0.140f, 0.016f, 0.030f, FT66FlatStyle::MakeFlatTooltipIcon(
			ET66FlatState::Default,
			InfoBrush,
			NSLOCTEXT("T66.Account", "HistoryOverviewInfoTooltip", "Account overview"),
			FVector2D(31.f, 31.f),
			HTag(TEXT("History.SubTabs.OverviewInfoIcon")),
			FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOverviewTabClicked)));
		AddN(0.806f, 0.140f, 0.016f, 0.030f, FT66FlatStyle::MakeFlatTooltipIcon(
			ET66FlatState::Selected,
			InfoBrush,
			NSLOCTEXT("T66.Account", "HistoryHistoryInfoTooltip", "Run history"),
			FVector2D(31.f, 31.f),
			HTag(TEXT("History.SubTabs.HistoryInfoIcon")),
			FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleHistoryTabClicked)));

		auto MakeMenuEntry = [&HTag, &UpperText](
			const TCHAR* Tag,
			const FText& Label,
			const bool bActive,
			TFunction<void()> OnSelected) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatDropdownOptionButton(bActive ? ET66FlatState::Selected : ET66FlatState::Default, UpperText(Label), FOnClicked::CreateLambda([OnSelected = MoveTemp(OnSelected)]()
				{
					OnSelected();
					FSlateApplication::Get().DismissAllMenus();
					return FReply::Handled();
				}), 220.f, 38.f, 16, HTag(Tag), TEXT("HistoryFilterSelection"));
		};

		auto MakeHeroMenu = [&]() -> TSharedRef<SWidget>
		{
			TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeMenuEntry(TEXT("History.FilterPanel.HeroDropdown.Option.All"), NSLOCTEXT("T66.Account", "HistoryHeroAllFlat", "All"), HistoryHeroFilter.IsNone(), [this]() { HistoryHeroFilter = NAME_None; RequestDeferredSlateRebuild(); })
			];
			for (const FAccountNamedEntry& Entry : HistoryHeroFilterEntries)
			{
				const FString Tag = FString::Printf(TEXT("History.FilterPanel.HeroDropdown.Option.%s"), *Entry.ID.ToString());
				Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					MakeMenuEntry(*Tag, Entry.DisplayName, HistoryHeroFilter == Entry.ID, [this, HeroID = Entry.ID]() { HistoryHeroFilter = HeroID; RequestDeferredSlateRebuild(); })
				];
			}
			return Menu;
		};

		auto MakeDifficultyMenu = [&]() -> TSharedRef<SWidget>
		{
			TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeMenuEntry(TEXT("History.FilterPanel.DifficultyDropdown.Option.All"), NSLOCTEXT("T66.Account", "HistoryDifficultyAllFlat", "All"), !HistoryDifficultyFilter.IsSet(), [this]() { HistoryDifficultyFilter.Reset(); RequestDeferredSlateRebuild(); })
			];
			for (ET66Difficulty Difficulty : { ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible })
			{
				const FString Tag = FString::Printf(TEXT("History.FilterPanel.DifficultyDropdown.Option.%d"), static_cast<int32>(Difficulty));
				Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					MakeMenuEntry(*Tag, DifficultyText(Difficulty), HistoryDifficultyFilter.IsSet() && HistoryDifficultyFilter.GetValue() == Difficulty, [this, Difficulty]() { HistoryDifficultyFilter = Difficulty; RequestDeferredSlateRebuild(); })
				];
			}
			return Menu;
		};

		auto MakePartySizeMenu = [&]() -> TSharedRef<SWidget>
		{
			TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeMenuEntry(TEXT("History.FilterPanel.PartySizeDropdown.Option.All"), NSLOCTEXT("T66.Account", "HistoryPartyAllFlat", "All"), !HistoryPartySizeFilter.IsSet(), [this]() { HistoryPartySizeFilter.Reset(); RequestDeferredSlateRebuild(); })
			];
			for (ET66PartySize PartySize : { ET66PartySize::Solo, ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad })
			{
				const FString Tag = FString::Printf(TEXT("History.FilterPanel.PartySizeDropdown.Option.%s"), *PartySizeToApiString(PartySize));
				Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					MakeMenuEntry(*Tag, PartySizeText(Loc, PartySize), HistoryPartySizeFilter.IsSet() && HistoryPartySizeFilter.GetValue() == PartySize, [this, PartySize]() { HistoryPartySizeFilter = PartySize; RequestDeferredSlateRebuild(); })
				];
			}
			return Menu;
		};

		auto MakeStatusMenu = [&]() -> TSharedRef<SWidget>
		{
			TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
			for (EHistoryCompletionFilter Filter : { EHistoryCompletionFilter::All, EHistoryCompletionFilter::Completed, EHistoryCompletionFilter::NotCompleted })
			{
				const FString Tag = FString::Printf(TEXT("History.FilterPanel.StatusDropdown.Option.%d"), static_cast<int32>(Filter));
				Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					MakeMenuEntry(*Tag, Filter == EHistoryCompletionFilter::All ? NSLOCTEXT("T66.Account", "HistoryStatusAllFlat", "All") : CompletionFilterText(Filter), HistoryCompletionFilter == Filter, [this, Filter]() { HistoryCompletionFilter = Filter; RequestDeferredSlateRebuild(); })
				];
			}
			return Menu;
		};

		auto FilterValueText = [&UpperText](const bool bUnset, const FText& Value) -> FText
		{
			return bUnset ? NSLOCTEXT("T66.Account", "HistoryFilterAllValue", "ALL") : UpperText(Value);
		};

		AddN(0.031f, 0.233f, 0.056f, 0.030f, TaggedText(HTag(TEXT("History.FilterPanel.HeroLabel")), NSLOCTEXT("T66.Account", "HistoryHeroLabelFlat", "HERO"), 20, FT66FlatStyle::PurpleAccent()));
		AddN(0.251f, 0.233f, 0.107f, 0.030f, TaggedText(HTag(TEXT("History.FilterPanel.DifficultyLabel")), NSLOCTEXT("T66.Account", "HistoryDifficultyLabelFlat", "DIFFICULTY"), 20, FT66FlatStyle::PurpleAccent()));
		AddN(0.467f, 0.233f, 0.099f, 0.030f, TaggedText(HTag(TEXT("History.FilterPanel.PartySizeLabel")), NSLOCTEXT("T66.Account", "HistoryPartySizeLabelFlat", "PARTY SIZE"), 20, FT66FlatStyle::PurpleAccent()));
		AddN(0.672f, 0.233f, 0.070f, 0.030f, TaggedText(HTag(TEXT("History.FilterPanel.StatusLabel")), NSLOCTEXT("T66.Account", "HistoryStatusLabelFlat", "STATUS"), 20, FT66FlatStyle::PurpleAccent()));
		AddN(0.876f, 0.233f, 0.097f, 0.030f, TaggedText(HTag(TEXT("History.FilterPanel.DailyDescentLabel")), NSLOCTEXT("T66.Account", "HistoryDailyLabelFlat", "DAILY DESCENT"), 20, FT66FlatStyle::PurpleAccent()));

		AddN(0.031f, 0.273f, 0.200f, 0.072f, FT66FlatStyle::MakeFlatDropdown(
			ET66FlatState::Selected,
			TAttribute<FText>::CreateLambda([this, WeakT66GI, WeakLoc, FilterValueText]()
			{
				if (HistoryHeroFilter.IsNone())
				{
					return NSLOCTEXT("T66.Account", "HistoryHeroValueAllFlat", "ALL");
				}
				UT66GameInstance* RuntimeGI = WeakT66GI.Get();
				UT66LocalizationSubsystem* RuntimeLoc = WeakLoc.Get();
				FHeroData HeroData;
				return FilterValueText(false, RuntimeGI && RuntimeGI->GetHeroData(HistoryHeroFilter, HeroData)
					? (RuntimeLoc ? RuntimeLoc->GetText_HeroName(HistoryHeroFilter) : HeroData.DisplayName)
					: FText::FromName(HistoryHeroFilter));
			}),
			MakeHeroMenu,
			true,
			0.200f * HistoryCanvasW,
			0.072f * HistoryCanvasH,
			20,
			HTag(TEXT("History.FilterPanel.HeroDropdown"))));
		AddN(0.251f, 0.273f, 0.193f, 0.072f, FT66FlatStyle::MakeFlatDropdown(
			ET66FlatState::Selected,
			TAttribute<FText>::CreateLambda([this, DifficultyText, FilterValueText]() { return FilterValueText(!HistoryDifficultyFilter.IsSet(), HistoryDifficultyFilter.IsSet() ? DifficultyText(HistoryDifficultyFilter.GetValue()) : FText::GetEmpty()); }),
			MakeDifficultyMenu,
			true,
			0.193f * HistoryCanvasW,
			0.072f * HistoryCanvasH,
			20,
			HTag(TEXT("History.FilterPanel.DifficultyDropdown"))));
		AddN(0.467f, 0.273f, 0.184f, 0.072f, FT66FlatStyle::MakeFlatDropdown(
			ET66FlatState::Selected,
			TAttribute<FText>::CreateLambda([this, Loc, FilterValueText]() { return FilterValueText(!HistoryPartySizeFilter.IsSet(), HistoryPartySizeFilter.IsSet() ? PartySizeText(Loc, HistoryPartySizeFilter.GetValue()) : FText::GetEmpty()); }),
			MakePartySizeMenu,
			true,
			0.184f * HistoryCanvasW,
			0.072f * HistoryCanvasH,
			20,
			HTag(TEXT("History.FilterPanel.PartySizeDropdown"))));
		AddN(0.672f, 0.273f, 0.178f, 0.072f, FT66FlatStyle::MakeFlatDropdown(
			ET66FlatState::Selected,
			TAttribute<FText>::CreateLambda([this, CompletionFilterText, FilterValueText]() { return FilterValueText(HistoryCompletionFilter == EHistoryCompletionFilter::All, HistoryCompletionFilter == EHistoryCompletionFilter::All ? FText::GetEmpty() : CompletionFilterText(HistoryCompletionFilter)); }),
			MakeStatusMenu,
			true,
			0.178f * HistoryCanvasW,
			0.072f * HistoryCanvasH,
			20,
			HTag(TEXT("History.FilterPanel.StatusDropdown"))));
		AddN(0.882f, 0.286f, 0.024f, 0.044f, FT66FlatStyle::MakeFlatCheckbox(
			ET66FlatState::Default,
			TAttribute<ECheckBoxState>::CreateLambda([this]() { return bHistoryDailyDescentOnly ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }),
			FOnCheckStateChanged::CreateLambda([this](const ECheckBoxState NewState)
			{
				bHistoryDailyDescentOnly = NewState == ECheckBoxState::Checked;
				RequestDeferredSlateRebuild();
			}),
			TAttribute<FText>(),
			HTag(TEXT("History.FilterPanel.DailyDescentCheckbox"))));

		TArray<FT66RecentRunRecord> FilteredRuns;
		for (const FT66RecentRunRecord& Run : RecentRuns)
		{
			if (!HistoryHeroFilter.IsNone() && Run.HeroID != HistoryHeroFilter)
			{
				continue;
			}
			if (HistoryDifficultyFilter.IsSet() && Run.Difficulty != HistoryDifficultyFilter.GetValue())
			{
				continue;
			}
			if (HistoryPartySizeFilter.IsSet() && Run.PartySize != HistoryPartySizeFilter.GetValue())
			{
				continue;
			}
			if (HistoryCompletionFilter == EHistoryCompletionFilter::Completed && !Run.bWasFullClear)
			{
				continue;
			}
			if (HistoryCompletionFilter == EHistoryCompletionFilter::NotCompleted && Run.bWasFullClear)
			{
				continue;
			}
			if (bHistoryDailyDescentOnly)
			{
				continue;
			}
			FilteredRuns.Add(Run);
		}

		auto HeroNameForRun = [T66GI, Loc](const FT66RecentRunRecord& Run) -> FText
		{
			FHeroData HeroData;
			if (T66GI && T66GI->GetHeroData(Run.HeroID, HeroData))
			{
				return Loc ? Loc->GetText_HeroName(Run.HeroID) : HeroData.DisplayName;
			}
			return Run.HeroID.IsNone() ? FText::FromString(TEXT("--")) : FText::FromName(Run.HeroID);
		};

		FilteredRuns.Sort([&](const FT66RecentRunRecord& A, const FT66RecentRunRecord& B)
		{
			int32 Compare = 0;
			switch (HistorySortColumn)
			{
			case EHistorySortColumn::HeroPlayed:
				Compare = HeroNameForRun(A).ToString().Compare(HeroNameForRun(B).ToString());
				break;
			case EHistorySortColumn::Status:
				Compare = static_cast<int32>(A.bWasFullClear) - static_cast<int32>(B.bWasFullClear);
				break;
			case EHistorySortColumn::Score:
				Compare = A.Score == B.Score ? 0 : (A.Score < B.Score ? -1 : 1);
				break;
			case EHistorySortColumn::Duration:
				Compare = FMath::IsNearlyEqual(A.DurationSeconds, B.DurationSeconds) ? 0 : (A.DurationSeconds < B.DurationSeconds ? -1 : 1);
				break;
			case EHistorySortColumn::Date:
			default:
				Compare = A.EndedAtUtc == B.EndedAtUtc ? 0 : (A.EndedAtUtc < B.EndedAtUtc ? -1 : 1);
				break;
			}
			return bHistorySortAscending ? Compare < 0 : Compare > 0;
		});

		auto SortClicked = [this](const EHistorySortColumn Column)
		{
			if (HistorySortColumn == Column)
			{
				bHistorySortAscending = !bHistorySortAscending;
			}
			else
			{
				HistorySortColumn = Column;
				bHistorySortAscending = false;
			}
			RequestDeferredSlateRebuild();
			return FReply::Handled();
		};

		auto MakeSortButton = [&MakeBareInteractive, &PlainText, &HTag, &SortClicked](const TCHAR* Tag, const FText& Label, const EHistorySortColumn Column) -> TSharedRef<SWidget>
		{
			TSharedRef<SWidget> Content = SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					PlainText(Label, 18, FT66FlatStyle::PurpleAccent(), true)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
				[
					PlainText(FText::FromString(TEXT("^")), 18, FT66FlatStyle::PurpleAccent(), true)
				];

			return MakeBareInteractive(
				HTag(Tag),
				TEXT("Button"),
				Content,
				FOnClicked::CreateLambda([SortClicked, Column]() { return SortClicked(Column); }));
		};

		AddN(0.027f, 0.433f, 0.945f, 0.057f, MakeMetadataRegion(HTag(TEXT("History.TableHeader")), TEXT("TableHeader")));
		AddN(
			0.027f,
			0.489f,
			0.945f,
			0.004f,
			FT66FlatStyle::AttachMetadata(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(FT66FlatStyle::BorderForState(ET66FlatState::Default)),
				HTag(TEXT("History.TableDivider")),
				TEXT("Divider"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(FT66FlatStyle::BorderForState(ET66FlatState::Default))));
		AddN(0.033f, 0.434f, 0.110f, 0.037f, MakeSortButton(TEXT("History.TableHeader.HeroPlayedButton"), NSLOCTEXT("T66.Account", "HistoryHeroPlayedHeaderFlat", "HERO PLAYED"), EHistorySortColumn::HeroPlayed));
		AddN(0.245f, 0.434f, 0.072f, 0.037f, MakeSortButton(TEXT("History.TableHeader.DateButton"), NSLOCTEXT("T66.Account", "HistoryDateHeaderFlat", "DATE"), EHistorySortColumn::Date));
		AddN(0.407f, 0.434f, 0.090f, 0.037f, MakeSortButton(TEXT("History.TableHeader.StatusButton"), NSLOCTEXT("T66.Account", "HistoryStatusHeaderFlat", "STATUS"), EHistorySortColumn::Status));
		AddN(0.566f, 0.434f, 0.085f, 0.037f, MakeSortButton(TEXT("History.TableHeader.ScoreButton"), NSLOCTEXT("T66.Account", "HistoryScoreHeaderFlat", "SCORE"), EHistorySortColumn::Score));
		AddN(0.715f, 0.434f, 0.112f, 0.037f, MakeSortButton(TEXT("History.TableHeader.DurationButton"), NSLOCTEXT("T66.Account", "HistoryDurationHeaderFlat", "DURATION"), EHistorySortColumn::Duration));
		AddN(
			0.876f,
			0.434f,
			0.080f,
			0.037f,
			MakeBareInteractive(
				HTag(TEXT("History.TableHeader.RankDropdown")),
				TEXT("Dropdown"),
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					PlainText(NSLOCTEXT("T66.Account", "HistoryRankHeaderFlat", "RANK"), 18, FT66FlatStyle::PurpleAccent(), true)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
				[
					PlainText(FText::FromString(TEXT("v")), 22, FT66FlatStyle::PurpleAccent(), true)
				],
				FOnClicked::CreateLambda([this]()
				{
					bHistoryRankSelectorDaily = !bHistoryRankSelectorDaily;
					RequestDeferredSlateRebuild();
					return FReply::Handled();
				})));

		if (FilteredRuns.Num() == 0)
		{
			AddN(0.036f, 0.521f, 0.220f, 0.033f, TaggedText(
				HTag(TEXT("History.EmptyState")),
				RecentRuns.Num() == 0
					? NSLOCTEXT("T66.Account", "HistoryNoRunsFlat", "No runs have been recorded yet.")
					: NSLOCTEXT("T66.Account", "HistoryNoFilteredRunsFlat", "No runs have been recorded yet."),
				18,
				FT66FlatStyle::SecondaryText(),
				false));
		}
		else
		{
			const float RowX = 0.027f;
			const float RowW = 0.945f;
			const float RowH = 0.046f;
			const int32 MaxRows = FMath::Min(8, FilteredRuns.Num());
			for (int32 RowIndex = 0; RowIndex < MaxRows; ++RowIndex)
			{
				const FT66RecentRunRecord& Run = FilteredRuns[RowIndex];
				const float RowY = 0.515f + RowIndex * 0.050f;
				const FString RowTag = FString::Printf(TEXT("History.RunRow.%02d"), RowIndex + 1);
				const bool bCanOpen = !Run.RunSummarySlotName.IsEmpty();
				const TSharedRef<SWidget> RowSurface = bCanOpen
					? FT66FlatStyle::MakeFlatToggleGroupButton(
						RowIndex == 0 ? ET66FlatState::Selected : ET66FlatState::Default,
						SNew(SSpacer),
						FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOpenRunSummaryClicked, Run.RunSummarySlotName),
						FMargin(0.f),
						RowW * HistoryCanvasW,
						RowH * HistoryCanvasH,
						true,
						HTag(*RowTag))
					: FT66FlatStyle::MakeFlatSubPanel(RowIndex == 0 ? ET66FlatState::Selected : ET66FlatState::Default, FMargin(0.f), SNew(SSpacer), nullptr, HTag(*RowTag));
				AddN(RowX, RowY, RowW, RowH, RowSurface);
				AddN(0.035f, RowY + 0.010f, 0.170f, 0.026f, TaggedText(HTag(*FString::Printf(TEXT("%s.Hero"), *RowTag)), UpperText(HeroNameForRun(Run)), 14, FT66FlatStyle::PrimaryText()));
				AddN(0.245f, RowY + 0.010f, 0.115f, 0.026f, TaggedText(HTag(*FString::Printf(TEXT("%s.Date"), *RowTag)), FText::FromString(Run.EndedAtUtc.ToString(TEXT("%m/%d/%Y"))), 14, FT66FlatStyle::SecondaryText()));
				AddN(0.407f, RowY + 0.010f, 0.110f, 0.026f, TaggedText(HTag(*FString::Printf(TEXT("%s.Status"), *RowTag)), Run.bWasFullClear ? NSLOCTEXT("T66.Account", "HistoryCompletedFlat", "Completed") : NSLOCTEXT("T66.Account", "HistoryNotCompletedFlat", "Not Completed"), 14, Run.bWasFullClear ? FT66FlatStyle::GoodStandingGreen() : FT66FlatStyle::SelectedText()));
				AddN(0.566f, RowY + 0.010f, 0.090f, 0.026f, TaggedText(HTag(*FString::Printf(TEXT("%s.Score"), *RowTag)), FText::AsNumber(Run.Score), 14, FT66FlatStyle::PrimaryText()));
				AddN(0.715f, RowY + 0.010f, 0.120f, 0.026f, TaggedText(HTag(*FString::Printf(TEXT("%s.Duration"), *RowTag)), FormatDurationText(Run.DurationSeconds), 14, FT66FlatStyle::PrimaryText()));
				AddN(0.876f, RowY + 0.010f, 0.080f, 0.026f, TaggedText(HTag(*FString::Printf(TEXT("%s.Rank"), *RowTag)), NSLOCTEXT("T66.Account", "HistoryRankPendingFlat", "--"), 14, FT66FlatStyle::SecondaryText(), true, ETextJustify::Right));
			}
		}

		TSharedRef<SWidget> HistoryContent = SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
			]
			+ SOverlay::Slot()
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					SNew(SBox)
					.WidthOverride(HistoryCanvasW)
					.HeightOverride(HistoryCanvasH)
					[
						HistoryCanvas
					]
				]
			];

		return HistoryContent;
	}

	if (ActiveTab == EAccountTab::Suspension)
	{
		auto MTag = [](const TCHAR* Tag) -> FName
		{
			return FName(Tag);
		};

		constexpr float ModalCanvasW = 1920.f;
		constexpr float ModalCanvasH = 1080.f;
		const TSharedRef<SConstraintCanvas> ModalCanvas = SNew(SConstraintCanvas);

		auto AddCanvas = [ModalCanvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			ModalCanvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X, Y, W, H))
			[
				Widget
			];
		};

		auto AddN = [&AddCanvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
		{
			constexpr float TopBarContentX = 0.0693f;
			constexpr float TopBarContentY = 0.1291f;
			constexpr float TopBarContentScale = 0.8613f;
			AddCanvas(
				((X - TopBarContentX) / TopBarContentScale) * ModalCanvasW,
				((Y - TopBarContentY) / TopBarContentScale) * ModalCanvasH,
				(W / TopBarContentScale) * ModalCanvasW,
				(H / TopBarContentScale) * ModalCanvasH,
				Widget);
		};

		auto MakeMetadataRegion = [](const FName Tag, const FString& Role, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(SNew(SSpacer), Tag, Role, State);
		};

		auto MakePanelSurface = [](const FName Tag, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatPanel(
				State,
				FMargin(0.f),
				SNew(SSpacer),
				nullptr,
				Tag);
		};

		auto TaggedText = [](
			const FName Tag,
			const FText& Text,
			const int32 FontSize,
			const FLinearColor& Color,
			const bool bBold = true,
			const ETextJustify::Type Justification = ETextJustify::Left,
			const bool bWrap = false) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::AttachMetadata(
				SNew(STextBlock)
				.Visibility(EVisibility::HitTestInvisible)
				.Text(Text)
				.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
				.ColorAndOpacity(Color)
				.Justification(Justification)
				.AutoWrapText(bWrap)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds),
				Tag,
				TEXT("Label"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true);
		};

		auto PlainText = [](
			const FText& Text,
			const int32 FontSize,
			const FLinearColor& Color,
			const bool bBold = true,
			const ETextJustify::Type Justification = ETextJustify::Center) -> TSharedRef<SWidget>
		{
			return SNew(STextBlock)
				.Visibility(EVisibility::HitTestInvisible)
				.Text(Text)
				.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
				.ColorAndOpacity(Color)
				.Justification(Justification)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.Clipping(EWidgetClipping::ClipToBounds);
		};

		auto MakeModalTabButton = [&PlainText, &MTag, this](
			const FName Tag,
			const FText& Label,
			const ET66FlatState State,
			FReply (UT66AccountStatusScreen::*Handler)()) -> TSharedRef<SWidget>
		{
			return FT66FlatStyle::MakeFlatToggleGroupButton(
				State,
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					PlainText(
						Label,
						30,
						State == ET66FlatState::Selected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText(),
						true,
						ETextJustify::Center)
				],
				FOnClicked::CreateUObject(this, Handler),
				FMargin(0.f),
				0.285f * ModalCanvasW,
				0.096f * ModalCanvasH,
				true,
				Tag,
				MTag(TEXT("AccountStatusModalTabs")));
		};

		const FText ModalSuspensionHeadline =
			Restriction.Restriction == ET66AccountRestrictionKind::CheatingCertainty
			? NSLOCTEXT("T66.Account", "SuspensionRestrictedHeadlineFlat", "ACCOUNT RESTRICTED")
			: NSLOCTEXT("T66.Account", "SuspensionHeadlineFlat", "ACCOUNT SUSPENDED");
		const FText ModalSuspensionBody =
			Restriction.Restriction == ET66AccountRestrictionKind::CheatingCertainty
			? NSLOCTEXT("T66.Account", "SuspensionRestrictedBodyFlat", "This account is blocked from leaderboard submissions until the restriction is cleared.")
			: NSLOCTEXT("T66.Account", "SuspensionBodyFlat", "This account cannot submit leaderboard scores while the suspension is active.");
		const bool bModalCanSubmitAppeal = LB && LB->CanSubmitAccountAppeal();
		const FText ModalSubmitAppealButtonText = bAppealSubmitInFlight
			? NSLOCTEXT("T66.Account", "SubmitAppealInFlightFlat", "SUBMITTING...")
			: (Loc ? Loc->GetText_AccountStatus_SubmitAppeal() : NSLOCTEXT("T66.Account", "SubmitAppealFlat", "SUBMIT APPEAL"));
		const TAttribute<bool> ModalCanSubmitAppealMessage = TAttribute<bool>::CreateLambda([this]()
		{
			FString TrimmedMessage = AppealDraftMessage;
			TrimmedMessage.TrimStartAndEndInline();
			return !bAppealSubmitInFlight && !TrimmedMessage.IsEmpty();
		});
		const FText ReasonText = Restriction.RestrictionReason.IsEmpty()
			? NSLOCTEXT("T66.Account", "NoSuspensionReasonFlat", "No reason recorded.")
			: FText::FromString(Restriction.RestrictionReason);

		AddN(
			0.f,
			0.f,
			1.f,
			1.f,
			FT66FlatStyle::AttachMetadata(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(FT66FlatStyle::BackgroundColor()),
				MTag(TEXT("AccountStatusModal.Background")),
				TEXT("Background"),
				ET66FlatState::Default));
		AddN(0.f, 0.f, 1.f, 1.f, MakeMetadataRegion(MTag(TEXT("AccountStatusModal.Root")), TEXT("Root")));

		AddN(0.0575f, 0.1424f, 0.8850f, 0.0960f, MakeMetadataRegion(MTag(TEXT("AccountStatusModal.SubTabs")), TEXT("ToggleGroup.AccountStatusModalTabs"), ET66FlatState::Selected));
		AddN(0.0575f, 0.1424f, 0.2850f, 0.0960f, MakeModalTabButton(MTag(TEXT("AccountStatusModal.SubTabs.SuspensionButton")), NSLOCTEXT("T66.Account", "SuspensionTabFlat", "SUSPENSION"), ET66FlatState::Selected, &UT66AccountStatusScreen::HandleSuspensionTabClicked));
		AddN(0.3575f, 0.1424f, 0.2850f, 0.0960f, MakeModalTabButton(MTag(TEXT("AccountStatusModal.SubTabs.OverviewButton")), NSLOCTEXT("T66.Account", "OverviewTabFlatSuspension", "OVERVIEW"), ET66FlatState::Default, &UT66AccountStatusScreen::HandleOverviewTabClicked));
		AddN(0.6575f, 0.1424f, 0.2850f, 0.0960f, MakeModalTabButton(MTag(TEXT("AccountStatusModal.SubTabs.HistoryButton")), NSLOCTEXT("T66.Account", "HistoryTabFlatSuspension", "HISTORY"), ET66FlatState::Default, &UT66AccountStatusScreen::HandleHistoryTabClicked));

		AddN(
			0.0015f,
			0.2517f,
			0.9970f,
			0.0720f,
			FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Selected,
				FMargin(18.f, 7.f),
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					TaggedText(
						MTag(TEXT("AccountStatusModal.InfoStrip.Text")),
						NSLOCTEXT("T66.Account", "SuspensionTabInfoFlat", "Review moderation status, appeal state, and flagged run details."),
						16,
						FT66FlatStyle::PrimaryText(),
						false,
						ETextJustify::Center,
						true)
				],
				nullptr,
				MTag(TEXT("AccountStatusModal.InfoStrip"))));

		AddN(0.0015f, 0.3371f, 0.9970f, 0.6400f, MakePanelSurface(MTag(TEXT("AccountStatusModal.ContentPanel"))));
		AddN(0.0180f, 0.3691f, 0.9640f, 0.5787f, MakeMetadataRegion(MTag(TEXT("AccountStatusModal.Content")), TEXT("Content")));
		AddN(0.0180f, 0.3691f, 0.9640f, 0.0306f, TaggedText(MTag(TEXT("AccountStatusModal.SectionLabel")), NSLOCTEXT("T66.Account", "SuspensionHdrFlat", "SUSPENSION"), 18, FT66FlatStyle::PrimaryText(), true));
		AddN(0.0180f, 0.4130f, 0.9640f, 0.0380f, TaggedText(MTag(TEXT("AccountStatusModal.Headline")), ModalSuspensionHeadline, 24, FT66FlatStyle::SelectedText(), true));
		AddN(0.0180f, 0.4589f, 0.9640f, 0.0185f, TaggedText(MTag(TEXT("AccountStatusModal.Description")), ModalSuspensionBody, 14, FT66FlatStyle::PrimaryText(), false, ETextJustify::Left, true));
		AddN(0.0180f, 0.4908f, 0.9640f, 0.0992f, FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(0.f), SNew(SSpacer), nullptr, MTag(TEXT("AccountStatusModal.ReasonPanel"))));
		AddN(0.0330f, 0.5201f, 0.9340f, 0.0167f, TaggedText(MTag(TEXT("AccountStatusModal.ReasonLabel")), NSLOCTEXT("T66.Account", "SuspensionReasonLabelFlat", "REASON"), 13, FT66FlatStyle::SelectedText(), true));
		AddN(0.0330f, 0.5448f, 0.9340f, 0.0185f, TaggedText(MTag(TEXT("AccountStatusModal.ReasonValue")), ReasonText, 14, FT66FlatStyle::PrimaryText(), false, ETextJustify::Left, true));
		AddN(0.0180f, 0.6033f, 0.9640f, 0.0167f, TaggedText(MTag(TEXT("AccountStatusModal.AppealStatus")), AppealStatusText(Restriction.AppealStatus), 13, FT66FlatStyle::SecondaryText(), true));

		if (!AppealSubmitStatusMessage.IsEmpty())
		{
			AddN(
				0.0180f,
				0.6240f,
				0.9640f,
				0.0200f,
				TaggedText(
					MTag(TEXT("AccountStatusModal.AppealSubmitStatus")),
					FText::FromString(AppealSubmitStatusMessage),
					13,
					bAppealSubmitStatusIsError ? FT66FlatStyle::SelectedText() : FT66FlatStyle::GoodStandingGreen(),
					false,
					ETextJustify::Left,
					true));
		}

		if (bModalCanSubmitAppeal && !bAppealEditorOpen)
		{
			AddN(
				0.0180f,
				0.6360f,
				0.9640f,
				0.0427f,
				FT66FlatStyle::MakeFlatButton(
					ET66FlatState::Selected,
					NSLOCTEXT("T66.Account", "OpenAppealFlat", "APPEAL"),
					FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOpenAppealClicked),
					nullptr,
					nullptr,
					FMargin(12.f, 6.f),
					0.964f * ModalCanvasW,
					0.0427f * ModalCanvasH,
					true,
					16,
					MTag(TEXT("AccountStatusModal.AppealButton"))));
		}
		else if (bModalCanSubmitAppeal && bAppealEditorOpen)
		{
			AddN(0.0180f, 0.6360f, 0.9640f, 0.2150f, FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(0.f), SNew(SSpacer), nullptr, MTag(TEXT("AccountStatusModal.AppealEditor"))));
			AddN(0.0330f, 0.6560f, 0.9340f, 0.0200f, TaggedText(MTag(TEXT("AccountStatusModal.AppealEditor.Title")), Loc ? Loc->GetText_AccountStatus_AppealTitle() : NSLOCTEXT("T66.Account", "AppealTitleFlat", "APPEAL"), 15, FT66FlatStyle::SelectedText(), true));
			AddN(
				0.0330f,
				0.6880f,
				0.9340f,
				0.0950f,
				FT66FlatStyle::AttachMetadata(
					FT66FlatStyle::MakeFlatSubPanel(
						ET66FlatState::Default,
						FMargin(12.f, 10.f),
						SAssignNew(AppealMessageTextBox, SMultiLineEditableTextBox)
						.AutoWrapText(true)
						.Text(FText::FromString(AppealDraftMessage))
						.HintText(Loc ? Loc->GetText_AccountStatus_AppealHint() : NSLOCTEXT("T66.Account", "AppealHintFlat", "Write your appeal message here..."))
						.ForegroundColor(FT66FlatStyle::PrimaryText())
						.OnTextChanged_Lambda([this](const FText& NewText)
						{
							AppealDraftMessage = NewText.ToString();
						})),
					MTag(TEXT("AccountStatusModal.AppealEditor.MessageInput")),
					TEXT("TextInput"),
					ET66FlatState::Default,
					TOptional<FLinearColor>(),
					true));
			AddN(
				0.6730f,
				0.8020f,
				0.1580f,
				0.0430f,
				FT66FlatStyle::MakeFlatButton(
					ET66FlatState::Selected,
					ModalSubmitAppealButtonText,
					FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleSubmitAppealClicked),
					nullptr,
					nullptr,
					FMargin(12.f, 6.f),
					0.158f * ModalCanvasW,
					0.043f * ModalCanvasH,
					ModalCanSubmitAppealMessage,
					15,
					MTag(TEXT("AccountStatusModal.AppealEditor.SubmitButton"))));
			AddN(
				0.8460f,
				0.8020f,
				0.1210f,
				0.0430f,
				FT66FlatStyle::MakeFlatButton(
					ET66FlatState::Default,
					NSLOCTEXT("T66.Account", "CancelAppealFlat", "CANCEL"),
					FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleCancelAppealClicked),
					nullptr,
					nullptr,
					FMargin(12.f, 6.f),
					0.121f * ModalCanvasW,
					0.043f * ModalCanvasH,
					TAttribute<bool>::CreateLambda([this]() { return !bAppealSubmitInFlight; }),
					15,
					MTag(TEXT("AccountStatusModal.AppealEditor.CancelButton"))));
		}
		else
		{
			AddN(0.0180f, 0.6360f, 0.9640f, 0.0427f, TaggedText(MTag(TEXT("AccountStatusModal.NoAppealAvailable")), NSLOCTEXT("T66.Account", "NoAppealAvailableFlat", "NO APPEAL AVAILABLE"), 14, FT66FlatStyle::SecondaryText(), true, ETextJustify::Center));
		}

		TSharedRef<SWidget> SuspensionScreenContent = SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					SNew(SBox)
					.WidthOverride(ModalCanvasW)
					.HeightOverride(ModalCanvasH)
					[
						ModalCanvas
					]
				]
			];

		TSharedRef<SWidget> BackgroundContent = SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(FLinearColor::Black);

		return FT66FlatStyle::MakeTopBarScreenRoot(
			UIManager,
			SuspensionScreenContent,
			BackgroundContent,
			FLinearColor::Transparent,
			FMargin(0.f));
	}

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
			FT66FlatStyle::MakeButton(
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
				FT66FlatStyle::MakeButton(
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
			+ SOverlay::Slot()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FT66FlatStyle::Scrim())]
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

	return FT66FlatStyle::MakeTopBarScreenRoot(
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
