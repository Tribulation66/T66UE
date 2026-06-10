// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66FriendslopStyle.h"

#include "Engine/Texture2D.h"
#include "HAL/IConsoleManager.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66FriendslopStyle, Log, All);

static TAutoConsoleVariable<int32> CVarT66UICandyKit(
	TEXT("T66.UI.CandyKit"),
	1,
	TEXT("When 1 (default), Friendslop chrome prefers the bright candy plate kit under FriendslopStyle/Candy/ (falls back per-asset to the classic plates when a candy asset is missing). 0 restores the classic Friendslop look instantly."),
	ECVF_Default);

namespace
{
	struct FFriendslopChromeDescriptor
	{
		const TCHAR* FileName;
		FMargin Margin;
		ESlateBrushDrawType::Type DrawAs;
		FVector2D FallbackSize;
		FLinearColor FallbackTint;
	};

	bool UseCandyKit()
	{
		return CVarT66UICandyKit.GetValueOnGameThread() > 0;
	}

	// Candy kit twin of the classic descriptor table. Same enum order. A null FileName
	// means the candy kit has no dedicated plate for that chrome yet — classic is used.
	// Margins are tuned per generated plate; FallbackSize mirrors the source PNG.
	struct FCandyChromeDescriptor
	{
		const TCHAR* FileName;
		FMargin Margin;
		ESlateBrushDrawType::Type DrawAs;
	};

	struct FFriendslopBrushEntry
	{
		TSharedPtr<FSlateBrush> Brush;
		TStrongObjectPtr<UTexture2D> Texture;
		bool bAttemptedLoad = false;
	};

	const FFriendslopChromeDescriptor& GetDescriptor(const ET66FriendslopChrome Chrome)
	{
		static const FFriendslopChromeDescriptor Descriptors[] = {
			{ TEXT("panel_large_dark.png"), FMargin(0.20f, 0.22f, 0.20f, 0.22f), ESlateBrushDrawType::Box, FVector2D(662.f, 464.f), FLinearColor(0.07f, 0.075f, 0.095f, 1.f) },
			{ TEXT("button_long_dark.png"), FMargin(0.18f, 0.26f, 0.18f, 0.26f), ESlateBrushDrawType::Box, FVector2D(666.f, 133.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("button_primary_red.png"), FMargin(0.18f, 0.26f, 0.18f, 0.26f), ESlateBrushDrawType::Box, FVector2D(667.f, 145.f), FLinearColor(0.62f, 0.04f, 0.075f, 1.f) },
			{ TEXT("button_action_green.png"), FMargin(0.18f, 0.26f, 0.18f, 0.26f), ESlateBrushDrawType::Box, FVector2D(667.f, 145.f), FLinearColor(0.05f, 0.42f, 0.16f, 1.f) },
			{ TEXT("icon_button_dark.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(157.f, 147.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("pill_dark.png"), FMargin(0.28f, 0.32f, 0.28f, 0.32f), ESlateBrushDrawType::Box, FVector2D(231.f, 113.f), FLinearColor(0.07f, 0.075f, 0.095f, 1.f) },
			{ TEXT("row_dark.png"), FMargin(0.12f, 0.30f, 0.12f, 0.30f), ESlateBrushDrawType::Box, FVector2D(858.f, 100.f), FLinearColor(0.075f, 0.08f, 0.105f, 1.f) },
			{ TEXT("row_selected_red.png"), FMargin(0.12f, 0.30f, 0.12f, 0.30f), ESlateBrushDrawType::Box, FVector2D(858.f, 105.f), FLinearColor(0.50f, 0.02f, 0.06f, 1.f) },
			{ TEXT("party_slot_dark.png"), FMargin(0.24f, 0.24f, 0.24f, 0.24f), ESlateBrushDrawType::Box, FVector2D(276.f, 236.f), FLinearColor(0.075f, 0.08f, 0.105f, 1.f) },
			{ TEXT("small_square_dark.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(169.f, 167.f), FLinearColor(0.075f, 0.08f, 0.105f, 1.f) },
			{ TEXT("checkbox_checked_red.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(149.f, 140.f), FLinearColor(0.72f, 0.05f, 0.085f, 1.f) },
			{ TEXT("checkbox_empty_dark.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(153.f, 141.f), FLinearColor(0.075f, 0.08f, 0.105f, 1.f) },
			{ TEXT("topbar_strip_round06.png"), FMargin(0.030f, 0.24f, 0.030f, 0.24f), ESlateBrushDrawType::Box, FVector2D(1920.f, 127.f), FLinearColor(0.07f, 0.075f, 0.095f, 1.f) },
			{ TEXT("topbar_icon_dark_round06.png"), FMargin(0.18f, 0.24f, 0.18f, 0.24f), ESlateBrushDrawType::Box, FVector2D(96.f, 74.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("topbar_tab_dark_round06.png"), FMargin(0.14f, 0.24f, 0.14f, 0.24f), ESlateBrushDrawType::Box, FVector2D(292.f, 74.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("topbar_tab_red_round06.png"), FMargin(0.14f, 0.24f, 0.14f, 0.24f), ESlateBrushDrawType::Box, FVector2D(325.f, 80.f), FLinearColor(0.62f, 0.04f, 0.075f, 1.f) },
			{ TEXT("topbar_ticket_round06.png"), FMargin(0.16f, 0.24f, 0.16f, 0.24f), ESlateBrushDrawType::Box, FVector2D(172.f, 74.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("topbar_power_red_round06.png"), FMargin(0.18f, 0.24f, 0.18f, 0.24f), ESlateBrushDrawType::Box, FVector2D(96.f, 74.f), FLinearColor(0.62f, 0.04f, 0.075f, 1.f) },
			{ TEXT("topbar_settings_icon_button_round06.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(100.f, 75.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("topbar_coupon_icon_button_round06.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(182.f, 75.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("topbar_power_icon_button_round06.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(100.f, 75.f), FLinearColor(0.62f, 0.04f, 0.075f, 1.f) },
			{ TEXT("left_panel_round06.png"), FMargin(0.050f, 0.035f, 0.050f, 0.035f), ESlateBrushDrawType::Box, FVector2D(640.f, 935.f), FLinearColor(0.07f, 0.075f, 0.095f, 1.f) },
			{ TEXT("profile_row_round06.png"), FMargin(0.12f, 0.24f, 0.12f, 0.24f), ESlateBrushDrawType::Box, FVector2D(520.f, 112.f), FLinearColor(0.075f, 0.08f, 0.105f, 1.f) },
			{ TEXT("search_field_round06.png"), FMargin(0.12f, 0.24f, 0.12f, 0.24f), ESlateBrushDrawType::Box, FVector2D(520.f, 60.f), FLinearColor(0.07f, 0.075f, 0.095f, 1.f) },
			{ TEXT("section_header_round06.png"), FMargin(0.11f, 0.24f, 0.11f, 0.24f), ESlateBrushDrawType::Box, FVector2D(520.f, 42.f), FLinearColor(0.075f, 0.08f, 0.105f, 1.f) },
			{ TEXT("friend_row_round06.png"), FMargin(0.11f, 0.24f, 0.11f, 0.24f), ESlateBrushDrawType::Box, FVector2D(520.f, 66.f), FLinearColor(0.075f, 0.08f, 0.105f, 1.f) },
			{ TEXT("invite_button_green_round06.png"), FMargin(0.18f, 0.24f, 0.18f, 0.24f), ESlateBrushDrawType::Box, FVector2D(96.f, 44.f), FLinearColor(0.05f, 0.42f, 0.16f, 1.f) },
			{ TEXT("offline_button_dark_round06.png"), FMargin(0.18f, 0.24f, 0.18f, 0.24f), ESlateBrushDrawType::Box, FVector2D(96.f, 42.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("party_slot_round06.png"), FMargin(0.24f, 0.24f, 0.24f, 0.24f), ESlateBrushDrawType::Box, FVector2D(80.f, 80.f), FLinearColor(0.075f, 0.08f, 0.105f, 1.f) },
			{ TEXT("title_logo_round06.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(680.f, 93.f), FLinearColor::White },
			{ TEXT("cta_primary_round06.png"), FMargin(0.13f, 0.24f, 0.13f, 0.24f), ESlateBrushDrawType::Box, FVector2D(680.f, 104.f), FLinearColor(0.62f, 0.04f, 0.075f, 1.f) },
			{ TEXT("cta_secondary_round06.png"), FMargin(0.13f, 0.24f, 0.13f, 0.24f), ESlateBrushDrawType::Box, FVector2D(660.f, 94.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("filter_rail_round06.png"), FMargin(0.13f, 0.08f, 0.13f, 0.08f), ESlateBrushDrawType::Box, FVector2D(88.f, 250.f), FLinearColor(0.07f, 0.075f, 0.095f, 1.f) },
			{ TEXT("filter_panel_round09.png"), FMargin(0.055f, 0.20f, 0.055f, 0.20f), ESlateBrushDrawType::Box, FVector2D(580.f, 90.f), FLinearColor(0.07f, 0.075f, 0.095f, 1.f) },
			{ TEXT("filter_icon_red_round06.png"), FMargin(0.16f, 0.26f, 0.16f, 0.26f), ESlateBrushDrawType::Box, FVector2D(132.f, 58.f), FLinearColor(0.62f, 0.04f, 0.075f, 1.f) },
			{ TEXT("filter_icon_dark_round06.png"), FMargin(0.16f, 0.26f, 0.16f, 0.26f), ESlateBrushDrawType::Box, FVector2D(132.f, 58.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("filter_global_icon_button_round06.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(132.f, 58.f), FLinearColor(0.62f, 0.04f, 0.075f, 1.f) },
			{ TEXT("filter_friends_icon_button_round06.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(132.f, 58.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("filter_streamer_icon_button_round06.png"), FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(132.f, 58.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("leaderboard_panel_round06.png"), FMargin(0.045f, 0.035f, 0.045f, 0.035f), ESlateBrushDrawType::Box, FVector2D(580.f, 846.f), FLinearColor(0.07f, 0.075f, 0.095f, 1.f) },
			{ TEXT("leaderboard_tab_red_round06.png"), FMargin(0.14f, 0.26f, 0.14f, 0.26f), ESlateBrushDrawType::Box, FVector2D(225.f, 50.f), FLinearColor(0.62f, 0.04f, 0.075f, 1.f) },
			{ TEXT("leaderboard_tab_dark_round06.png"), FMargin(0.14f, 0.26f, 0.14f, 0.26f), ESlateBrushDrawType::Box, FVector2D(225.f, 50.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("dropdown_dark_round06.png"), FMargin(0.14f, 0.26f, 0.14f, 0.26f), ESlateBrushDrawType::Box, FVector2D(225.f, 50.f), FLinearColor(0.08f, 0.085f, 0.11f, 1.f) },
			{ TEXT("checkbox_checked_round06.png"), FMargin(0.24f, 0.24f, 0.24f, 0.24f), ESlateBrushDrawType::Box, FVector2D(28.f, 28.f), FLinearColor(0.72f, 0.05f, 0.085f, 1.f) },
			{ TEXT("checkbox_empty_round06.png"), FMargin(0.24f, 0.24f, 0.24f, 0.24f), ESlateBrushDrawType::Box, FVector2D(28.f, 28.f), FLinearColor(0.075f, 0.08f, 0.105f, 1.f) },
			{ TEXT("table_header_band_round06.png"), FMargin(0.08f, 0.24f, 0.08f, 0.24f), ESlateBrushDrawType::Box, FVector2D(476.f, 28.f), FLinearColor(0.075f, 0.08f, 0.105f, 1.f) },
			{ TEXT("ranking_row_red_round06.png"), FMargin(0.10f, 0.24f, 0.10f, 0.24f), ESlateBrushDrawType::Box, FVector2D(460.f, 42.f), FLinearColor(0.50f, 0.02f, 0.06f, 1.f) },
		};

		const int32 Index = static_cast<int32>(Chrome);
		if (Index >= 0 && Index < UE_ARRAY_COUNT(Descriptors))
		{
			return Descriptors[Index];
		}
		return Descriptors[0];
	}

	// Candy filenames for the core shared chrome. Tail enum entries (screen-specific
	// Round06 plates) intentionally have no candy twin yet; they fall back to classic.
	const FCandyChromeDescriptor* GetCandyDescriptor(const ET66FriendslopChrome Chrome)
	{
		static const TMap<uint8, FCandyChromeDescriptor> CandyDescriptors = {
			{ static_cast<uint8>(ET66FriendslopChrome::PanelLargeDark),      { TEXT("panel_large.png"),      FMargin(0.16f, 0.18f, 0.16f, 0.18f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::ButtonLongDark),      { TEXT("button_secondary.png"), FMargin(0.18f, 0.26f, 0.18f, 0.26f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::ButtonPrimaryRed),    { TEXT("button_primary.png"),   FMargin(0.18f, 0.26f, 0.18f, 0.26f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::ButtonActionGreen),   { TEXT("button_action.png"),    FMargin(0.18f, 0.26f, 0.18f, 0.26f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::IconButtonDark),      { TEXT("icon_button.png"),      FMargin(0.f), ESlateBrushDrawType::Image } },
			{ static_cast<uint8>(ET66FriendslopChrome::PillDark),            { TEXT("pill.png"),             FMargin(0.28f, 0.32f, 0.28f, 0.32f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::RowDark),             { TEXT("row.png"),              FMargin(0.12f, 0.30f, 0.12f, 0.30f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::RowSelectedRed),      { TEXT("row_selected.png"),     FMargin(0.12f, 0.30f, 0.12f, 0.30f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::PartySlotDark),       { TEXT("slot.png"),             FMargin(0.24f, 0.24f, 0.24f, 0.24f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::SmallSquareDark),     { TEXT("small_square.png"),     FMargin(0.f), ESlateBrushDrawType::Image } },
			{ static_cast<uint8>(ET66FriendslopChrome::CheckboxCheckedRed),  { TEXT("checkbox_checked.png"), FMargin(0.f), ESlateBrushDrawType::Image } },
			{ static_cast<uint8>(ET66FriendslopChrome::CheckboxEmptyDark),   { TEXT("checkbox_empty.png"),   FMargin(0.f), ESlateBrushDrawType::Image } },
			{ static_cast<uint8>(ET66FriendslopChrome::DropdownDarkRound06), { TEXT("dropdown.png"),         FMargin(0.14f, 0.26f, 0.14f, 0.26f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::CheckboxCheckedRound06), { TEXT("checkbox_checked.png"), FMargin(0.f), ESlateBrushDrawType::Image } },
			{ static_cast<uint8>(ET66FriendslopChrome::CheckboxEmptyRound06),   { TEXT("checkbox_empty.png"),   FMargin(0.f), ESlateBrushDrawType::Image } },
			{ static_cast<uint8>(ET66FriendslopChrome::TableHeaderBandRound06), { TEXT("header_band.png"),    FMargin(0.08f, 0.24f, 0.08f, 0.24f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::RankingRowRedRound06),   { TEXT("row_selected.png"),   FMargin(0.10f, 0.24f, 0.10f, 0.24f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::TopbarTabDarkRound06),   { TEXT("tab_idle.png"),       FMargin(0.14f, 0.24f, 0.14f, 0.24f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::TopbarTabRedRound06),    { TEXT("tab_selected.png"),   FMargin(0.14f, 0.24f, 0.14f, 0.24f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::LeaderboardTabDarkRound06), { TEXT("tab_idle.png"),     FMargin(0.14f, 0.26f, 0.14f, 0.26f), ESlateBrushDrawType::Box } },
			{ static_cast<uint8>(ET66FriendslopChrome::LeaderboardTabRedRound06),  { TEXT("tab_selected.png"), FMargin(0.14f, 0.26f, 0.14f, 0.26f), ESlateBrushDrawType::Box } },
		};
		return CandyDescriptors.Find(static_cast<uint8>(Chrome));
	}

	FFriendslopBrushEntry& GetBrushEntry(const ET66FriendslopChrome Chrome)
	{
		static TMap<uint8, FFriendslopBrushEntry> Entries;
		return Entries.FindOrAdd(static_cast<uint8>(Chrome));
	}

	FFriendslopBrushEntry& GetCandyBrushEntry(const ET66FriendslopChrome Chrome)
	{
		static TMap<uint8, FFriendslopBrushEntry> Entries;
		return Entries.FindOrAdd(static_cast<uint8>(Chrome));
	}

	FFriendslopBrushEntry& GetCustomBrushEntry(const FString& RelativePath)
	{
		static TMap<FString, FFriendslopBrushEntry> Entries;
		return Entries.FindOrAdd(RelativePath);
	}

	FSlateColor FallbackSurfaceTintForState(const ET66FlatState State, const FFriendslopChromeDescriptor& Descriptor)
	{
		switch (State)
		{
		case ET66FlatState::Selected:
			return FSlateColor(FLinearColor(0.62f, 0.04f, 0.075f, 1.f));
		case ET66FlatState::Ready:
			return FSlateColor(FLinearColor(0.05f, 0.42f, 0.16f, 1.f));
		case ET66FlatState::Disabled:
			return FSlateColor(FLinearColor(0.06f, 0.058f, 0.07f, 0.74f));
		case ET66FlatState::Default:
		default:
			return FSlateColor(Descriptor.FallbackTint);
		}
	}

	FName MakeChildTag(const FName Tag, const TCHAR* Suffix)
	{
		if (Tag.IsNone() || !Suffix || !*Suffix)
		{
			return NAME_None;
		}
		return FName(*(Tag.ToString() + TEXT(".") + Suffix));
	}
}

const FSlateBrush* FT66FriendslopStyle::GetChromeBrush(const ET66FriendslopChrome Chrome)
{
	// Candy kit first: per-asset override with graceful per-asset fallback to classic.
	if (UseCandyKit())
	{
		if (const FCandyChromeDescriptor* Candy = GetCandyDescriptor(Chrome))
		{
			FFriendslopBrushEntry& CandyEntry = GetCandyBrushEntry(Chrome);
			if (!CandyEntry.Brush.IsValid())
			{
				CandyEntry.Brush = MakeShared<FSlateBrush>();
				CandyEntry.Brush->DrawAs = Candy->DrawAs;
				CandyEntry.Brush->Margin = Candy->Margin;
				CandyEntry.Brush->Tiling = ESlateBrushTileType::NoTile;
			}
			if (!CandyEntry.bAttemptedLoad)
			{
				CandyEntry.bAttemptedLoad = true;
				const FString CandyRelativePath = FString::Printf(
					TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Candy/%s"),
					Candy->FileName);
				for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(CandyRelativePath))
				{
					if (!FPaths::FileExists(CandidatePath))
					{
						continue;
					}
					if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
						CandidatePath,
						TextureFilter::TF_Trilinear,
						true,
						TEXT("FriendslopCandy")))
					{
						CandyEntry.Texture.Reset(Texture);
						break;
					}
				}
			}
			if (CandyEntry.Texture.IsValid())
			{
				CandyEntry.Brush->SetResourceObject(CandyEntry.Texture.Get());
				CandyEntry.Brush->ImageSize = FVector2D(
					FMath::Max(1.f, static_cast<float>(CandyEntry.Texture->GetSizeX())),
					FMath::Max(1.f, static_cast<float>(CandyEntry.Texture->GetSizeY())));
				CandyEntry.Brush->TintColor = FSlateColor(FLinearColor::White);
				return CandyEntry.Brush.Get();
			}
			// fall through to classic when the candy asset is absent/unloadable
		}
	}

	const FFriendslopChromeDescriptor& Descriptor = GetDescriptor(Chrome);
	FFriendslopBrushEntry& Entry = GetBrushEntry(Chrome);

	if (!Entry.Brush.IsValid())
	{
		Entry.Brush = MakeShared<FSlateBrush>();
		Entry.Brush->DrawAs = Descriptor.DrawAs;
		Entry.Brush->Margin = Descriptor.Margin;
		Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
		Entry.Brush->ImageSize = Descriptor.FallbackSize;
	}

	if (!Entry.bAttemptedLoad)
	{
		Entry.bAttemptedLoad = true;
		const FString RelativePath = FString::Printf(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/%s"),
			Descriptor.FileName);
		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
				CandidatePath,
				TextureFilter::TF_Trilinear,
				true,
				TEXT("FriendslopStyle")))
			{
				Entry.Texture.Reset(Texture);
				break;
			}
		}

		if (!Entry.Texture.IsValid())
		{
			UE_LOG(LogT66FriendslopStyle, Warning, TEXT("FriendslopStyle could not load runtime chrome '%s'"), Descriptor.FileName);
		}
	}

	if (Entry.Texture.IsValid())
	{
		Entry.Brush->SetResourceObject(Entry.Texture.Get());
		Entry.Brush->ImageSize = FVector2D(
			FMath::Max(1.f, static_cast<float>(Entry.Texture->GetSizeX())),
			FMath::Max(1.f, static_cast<float>(Entry.Texture->GetSizeY())));
		Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
		return Entry.Brush.Get();
	}

	return FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
}

const FSlateBrush* FT66FriendslopStyle::GetCustomBrush(
	const FString& RelativePath,
	const FMargin& Margin,
	const ESlateBrushDrawType::Type DrawAs,
	const FVector2D& FallbackSize,
	const FLinearColor& FallbackTint)
{
	FFriendslopBrushEntry& Entry = GetCustomBrushEntry(RelativePath);

	if (!Entry.Brush.IsValid())
	{
		Entry.Brush = MakeShared<FSlateBrush>();
		Entry.Brush->DrawAs = DrawAs;
		Entry.Brush->Margin = Margin;
		Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
		Entry.Brush->ImageSize = FallbackSize;
		Entry.Brush->TintColor = FSlateColor(FallbackTint);
	}

	if (!Entry.bAttemptedLoad)
	{
		Entry.bAttemptedLoad = true;
		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
				CandidatePath,
				TextureFilter::TF_Trilinear,
				true,
				TEXT("FriendslopStyle")))
			{
				Entry.Texture.Reset(Texture);
				break;
			}
		}

		if (!Entry.Texture.IsValid())
		{
			UE_LOG(LogT66FriendslopStyle, Warning, TEXT("FriendslopStyle could not load runtime chrome '%s'"), *RelativePath);
		}
	}

	if (Entry.Texture.IsValid())
	{
		Entry.Brush->SetResourceObject(Entry.Texture.Get());
		Entry.Brush->ImageSize = FVector2D(
			FMath::Max(1.f, static_cast<float>(Entry.Texture->GetSizeX())),
			FMath::Max(1.f, static_cast<float>(Entry.Texture->GetSizeY())));
		Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
		return Entry.Brush.Get();
	}

	return FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
}

ET66FriendslopChrome FT66FriendslopStyle::ButtonChromeForState(const ET66FlatState State)
{
	switch (State)
	{
	case ET66FlatState::Selected:
		return ET66FriendslopChrome::ButtonPrimaryRed;
	case ET66FlatState::Ready:
		return ET66FriendslopChrome::ButtonActionGreen;
	case ET66FlatState::Disabled:
	case ET66FlatState::Default:
	default:
		return ET66FriendslopChrome::ButtonLongDark;
	}
}

ET66FriendslopChrome FT66FriendslopStyle::RowChromeForState(const ET66FlatState State)
{
	return State == ET66FlatState::Selected
		? ET66FriendslopChrome::RowSelectedRed
		: ET66FriendslopChrome::RowDark;
}

const FSlateBrush* FT66FriendslopStyle::GetCheckboxBrush(const bool bChecked)
{
	return GetChromeBrush(bChecked ? ET66FriendslopChrome::CheckboxCheckedRound06 : ET66FriendslopChrome::CheckboxEmptyRound06);
}

FSlateColor FT66FriendslopStyle::TextColorForState(const ET66FlatState State)
{
	switch (State)
	{
	case ET66FlatState::Selected:
		return FSlateColor(FLinearColor(1.f, 0.95f, 0.95f, 1.f));
	case ET66FlatState::Ready:
		return FSlateColor(FLinearColor(0.97f, 1.f, 0.93f, 1.f));
	case ET66FlatState::Disabled:
		return FSlateColor(FLinearColor(0.61f, 0.52f, 0.55f, 0.85f));
	case ET66FlatState::Default:
	default:
		return FSlateColor(FLinearColor(0.94f, 0.93f, 1.f, 1.f));
	}
}

TSharedRef<SWidget> FT66FriendslopStyle::MakeSurface(
	const ET66FriendslopChrome Chrome,
	const ET66FlatState State,
	const FMargin& Padding,
	const TSharedRef<SWidget>& Content,
	TSharedPtr<SBorder>* OutBorder,
	const FName Tag,
	const TCHAR* Role,
	const bool bInteractive,
	const FName ToggleGroup,
	const bool bHoverCapable)
{
	const FFriendslopChromeDescriptor& Descriptor = GetDescriptor(Chrome);
	const FSlateBrush* ChromeBrush = GetChromeBrush(Chrome);
	const bool bBrushLoaded = GetBrushEntry(Chrome).Texture.IsValid();
	TSharedPtr<SBorder> Border;
	SAssignNew(Border, SBorder)
		.BorderImage(ChromeBrush)
		.BorderBackgroundColor(bBrushLoaded ? FLinearColor::White : FallbackSurfaceTintForState(State, Descriptor))
		.Padding(Padding)
		[
			Content
		];

	if (OutBorder)
	{
		*OutBorder = Border;
	}

	return FT66FlatStyle::AttachMetadata(
		Border.ToSharedRef(),
		Tag,
		Role,
		State,
		TOptional<FLinearColor>(),
		bInteractive,
		ToggleGroup,
		false,
		bHoverCapable);
}

TSharedRef<SWidget> FT66FriendslopStyle::MakeCustomSurface(
	const FString& RelativePath,
	const FMargin& Margin,
	const ESlateBrushDrawType::Type DrawAs,
	const FVector2D& FallbackSize,
	const ET66FlatState State,
	const FMargin& Padding,
	const TSharedRef<SWidget>& Content,
	TSharedPtr<SBorder>* OutBorder,
	const FName Tag,
	const TCHAR* Role,
	const bool bInteractive,
	const FName ToggleGroup,
	const bool bHoverCapable,
	const FLinearColor& FallbackTint)
{
	const FSlateBrush* ChromeBrush = GetCustomBrush(RelativePath, Margin, DrawAs, FallbackSize, FallbackTint);
	const bool bBrushLoaded = GetCustomBrushEntry(RelativePath).Texture.IsValid();

	TSharedPtr<SBorder> Border;
	SAssignNew(Border, SBorder)
		.BorderImage(ChromeBrush)
		.BorderBackgroundColor(bBrushLoaded ? FLinearColor::White : FallbackTint)
		.Padding(Padding)
		[
			Content
		];

	if (OutBorder)
	{
		*OutBorder = Border;
	}

	return FT66FlatStyle::AttachMetadata(
		Border.ToSharedRef(),
		Tag,
		Role,
		State,
		TOptional<FLinearColor>(),
		bInteractive,
		ToggleGroup,
		false,
		bHoverCapable);
}

TSharedRef<SWidget> FT66FriendslopStyle::MakePanel(
	const ET66FlatState State,
	const FMargin& Padding,
	const TSharedRef<SWidget>& Content,
	TSharedPtr<SBorder>* OutBorder,
	const FName Tag,
	const ET66FriendslopChrome Chrome)
{
	return MakeSurface(Chrome, State, Padding, Content, OutBorder, Tag, TEXT("Panel"), false, NAME_None, false);
}

TSharedRef<SWidget> FT66FriendslopStyle::MakeCustomPanel(
	const FString& RelativePath,
	const FMargin& Margin,
	const FVector2D& FallbackSize,
	const ET66FlatState State,
	const FMargin& Padding,
	const TSharedRef<SWidget>& Content,
	TSharedPtr<SBorder>* OutBorder,
	const FName Tag,
	const FLinearColor& FallbackTint)
{
	return MakeCustomSurface(
		RelativePath,
		Margin,
		ESlateBrushDrawType::Box,
		FallbackSize,
		State,
		Padding,
		Content,
		OutBorder,
		Tag,
		TEXT("Panel"),
		false,
		NAME_None,
		false,
		FallbackTint);
}

TSharedRef<SWidget> FT66FriendslopStyle::MakeButton(
	const ET66FlatState State,
	const TAttribute<FText>& Label,
	FOnClicked OnClicked,
	const TSharedPtr<SWidget>& OptionalLeftIcon,
	const TSharedPtr<SWidget>& OptionalRightIcon,
	const FMargin& Padding,
	const float MinWidth,
	const float Height,
	const TAttribute<bool> IsEnabled,
	const int32 FontSize,
	const FName Tag,
	const FName ToggleGroup,
	const ET66FriendslopChrome Chrome)
{
	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
	if (OptionalLeftIcon.IsValid())
	{
		Row->AddSlot()
			.AutoWidth()
			.Padding(0.f, 0.f, 10.f, 0.f)
			.VAlign(VAlign_Center)
			[
				OptionalLeftIcon.ToSharedRef()
			];
	}

	Row->AddSlot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			FT66FlatStyle::AttachMetadata(
				SNew(STextBlock)
				.Text(Label)
				.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(FontSize, true))
				.ColorAndOpacity(TextColorForState(State))
				.Justification(ETextJustify::Center)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				.ShadowOffset(FVector2D(2.f, 2.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.55f)),
				MakeChildTag(Tag, TEXT("Label")),
				TEXT("Label.Button"),
				State,
				TOptional<FLinearColor>(),
				false,
				NAME_None,
				true)
		];

	if (OptionalRightIcon.IsValid())
	{
		Row->AddSlot()
			.AutoWidth()
			.Padding(10.f, 0.f, 0.f, 0.f)
			.VAlign(VAlign_Center)
			[
				OptionalRightIcon.ToSharedRef()
			];
	}

	return MakeToggleGroupButton(
		State,
		Row,
		MoveTemp(OnClicked),
		Padding,
		MinWidth,
		Height,
		IsEnabled,
		Tag,
		ToggleGroup,
		Chrome);
}

TSharedRef<SWidget> FT66FriendslopStyle::MakeButton(
	const ET66FlatState State,
	const FText& Label,
	FOnClicked OnClicked,
	const TSharedPtr<SWidget>& OptionalLeftIcon,
	const TSharedPtr<SWidget>& OptionalRightIcon,
	const FMargin& Padding,
	const float MinWidth,
	const float Height,
	const TAttribute<bool> IsEnabled,
	const int32 FontSize,
	const FName Tag,
	const FName ToggleGroup,
	const ET66FriendslopChrome Chrome)
{
	return MakeButton(
		State,
		TAttribute<FText>(Label),
		MoveTemp(OnClicked),
		OptionalLeftIcon,
		OptionalRightIcon,
		Padding,
		MinWidth,
		Height,
		IsEnabled,
		FontSize,
		Tag,
		ToggleGroup,
		Chrome);
}

TSharedRef<SWidget> FT66FriendslopStyle::MakeToggleGroupButton(
	const ET66FlatState State,
	const TSharedRef<SWidget>& Content,
	FOnClicked OnClicked,
	const FMargin& Padding,
	const float MinWidth,
	const float Height,
	const TAttribute<bool> IsEnabled,
	const FName Tag,
	const FName ToggleGroup,
	const ET66FriendslopChrome Chrome)
{
	const bool bInteractive = OnClicked.IsBound();
	TSharedRef<SButton> Button = SNew(SButton)
		.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("NoBorder")))
		.ContentPadding(FMargin(0.f))
		.ClickMethod(EButtonClickMethod::MouseDown)
		.IsEnabled(IsEnabled)
		.OnClicked(MoveTemp(OnClicked))
		[
			SNew(SBox)
			.MinDesiredWidth(MinWidth)
			.HeightOverride(Height > 0.f ? Height : 44.f)
			[
				MakeSurface(
					Chrome,
					State,
					Padding,
					Content,
					nullptr,
					NAME_None,
					TEXT("ButtonSurface"),
					false,
					NAME_None,
					false)
			]
		];

	return FT66FlatStyle::AttachMetadata(
		Button,
		Tag,
		TEXT("Button"),
		State,
		TOptional<FLinearColor>(),
		bInteractive,
		ToggleGroup,
		false,
		bInteractive);
}

TSharedRef<SWidget> FT66FriendslopStyle::MakeCustomToggleGroupButton(
	const FString& RelativePath,
	const FMargin& Margin,
	const FVector2D& FallbackSize,
	const ET66FlatState State,
	const TSharedRef<SWidget>& Content,
	FOnClicked OnClicked,
	const FMargin& Padding,
	const float MinWidth,
	const float Height,
	const TAttribute<bool> IsEnabled,
	const FName Tag,
	const FName ToggleGroup,
	const FLinearColor& FallbackTint,
	const ESlateBrushDrawType::Type DrawAs)
{
	const bool bInteractive = OnClicked.IsBound();
	TSharedRef<SButton> Button = SNew(SButton)
		.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("NoBorder")))
		.ContentPadding(FMargin(0.f))
		.ClickMethod(EButtonClickMethod::MouseDown)
		.IsEnabled(IsEnabled)
		.OnClicked(MoveTemp(OnClicked))
		[
			SNew(SBox)
			.WidthOverride(MinWidth)
			.HeightOverride(Height > 0.f ? Height : 44.f)
			[
				MakeCustomSurface(
					RelativePath,
					Margin,
					DrawAs,
					FallbackSize,
					State,
					Padding,
					Content,
					nullptr,
					NAME_None,
					TEXT("ButtonSurface"),
					false,
					NAME_None,
					false,
					FallbackTint)
			]
		];

	return FT66FlatStyle::AttachMetadata(
		Button,
		Tag,
		TEXT("Button"),
		State,
		TOptional<FLinearColor>(),
		bInteractive,
		ToggleGroup,
		false,
		bInteractive);
}

TSharedRef<SWidget> FT66FriendslopStyle::MakeFixedImage(
	const ET66FriendslopChrome Chrome,
	const FVector2D& Size,
	const FName Tag,
	const TCHAR* Role)
{
	return FT66FlatStyle::AttachMetadata(
		SNew(SBox)
		.WidthOverride(Size.X)
		.HeightOverride(Size.Y)
		[
			SNew(SImage)
			.Image(GetChromeBrush(Chrome))
			.ColorAndOpacity(FLinearColor::White)
		],
		Tag,
		Role,
		ET66FlatState::Default);
}

TSharedRef<SWidget> FT66FriendslopStyle::MakeCustomFixedImage(
	const FString& RelativePath,
	const FMargin& Margin,
	const ESlateBrushDrawType::Type DrawAs,
	const FVector2D& Size,
	const FName Tag,
	const TCHAR* Role,
	const FLinearColor& FallbackTint)
{
	return FT66FlatStyle::AttachMetadata(
		SNew(SBox)
		.WidthOverride(Size.X)
		.HeightOverride(Size.Y)
		[
			SNew(SImage)
			.Image(GetCustomBrush(RelativePath, Margin, DrawAs, Size, FallbackTint))
			.ColorAndOpacity(FLinearColor::White)
		],
		Tag,
		Role,
		ET66FlatState::Default);
}
