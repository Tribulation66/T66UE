// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ArcadeGameCatalog.h"

#include "Core/T66GameInstance.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"

namespace
{
	const TArray<FT66ArcadeGameCatalogEntry>& T66BuildArcadeCatalog()
	{
		static const TArray<FT66ArcadeGameCatalogEntry> Entries = {
			{
				ET66ArcadeGameType::WhackAMole,
				FName(TEXT("Arcade_WhackAMole")),
				NSLOCTEXT("T66.ArcadeCatalog", "WhackAMolePrototypeName", "WHAC-A-MOLE COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "WhackAMoleInternalName", "Whack-a-Mole"),
				NSLOCTEXT("T66.ArcadeCatalog", "WhackAMoleDescription", "Fast target bonks across a lit 3x3 board."),
				NSLOCTEXT("T66.ArcadeCatalog", "WhackAMoleCode", "MOL"),
				FLinearColor(0.24f, 0.84f, 0.42f, 1.f)
			},
			{
				ET66ArcadeGameType::Topwar,
				FName(TEXT("Arcade_Topwar")),
				NSLOCTEXT("T66.ArcadeCatalog", "TopwarPrototypeName", "TOP WAR COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "TopwarInternalName", "Topwar"),
				NSLOCTEXT("T66.ArcadeCatalog", "TopwarDescription", "Choose power gates and grow the squad score."),
				NSLOCTEXT("T66.ArcadeCatalog", "TopwarCode", "TOP"),
				FLinearColor(0.95f, 0.44f, 0.16f, 1.f)
			},
			{
				ET66ArcadeGameType::GoldMiner,
				FName(TEXT("Arcade_GoldMiner")),
				NSLOCTEXT("T66.ArcadeCatalog", "GoldMinerPrototypeName", "GOLD MINER COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "GoldMinerInternalName", "Gold Miner"),
				NSLOCTEXT("T66.ArcadeCatalog", "GoldMinerDescription", "Swing, hook, and reel treasure from the pit."),
				NSLOCTEXT("T66.ArcadeCatalog", "GoldMinerCode", "GLD"),
				FLinearColor(0.95f, 0.76f, 0.20f, 1.f)
			},
			{
				ET66ArcadeGameType::BladeSweep,
				FName(TEXT("Arcade_BladeSweep")),
				NSLOCTEXT("T66.ArcadeCatalog", "BladeSweepPrototypeName", "FRUIT NINJA COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "BladeSweepInternalName", "Blade Sweep"),
				NSLOCTEXT("T66.ArcadeCatalog", "BladeSweepDescription", "Sweep cursed fruit and dodge bad tiles."),
				NSLOCTEXT("T66.ArcadeCatalog", "BladeSweepCode", "BLD"),
				FLinearColor(0.98f, 0.24f, 0.46f, 1.f)
			},
			{
				ET66ArcadeGameType::RuneSwipe,
				FName(TEXT("Arcade_RuneSwipe")),
				NSLOCTEXT("T66.ArcadeCatalog", "RuneSwipePrototypeName", "PIANO TILES COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "RuneSwipeInternalName", "Rune Swipe"),
				NSLOCTEXT("T66.ArcadeCatalog", "RuneSwipeDescription", "Tap the glowing rune chain before it fades."),
				NSLOCTEXT("T66.ArcadeCatalog", "RuneSwipeCode", "RUN"),
				FLinearColor(0.16f, 0.82f, 0.78f, 1.f)
			},
			{
				ET66ArcadeGameType::CartSwitcher,
				FName(TEXT("Arcade_CartSwitcher")),
				NSLOCTEXT("T66.ArcadeCatalog", "CartSwitcherPrototypeName", "TRAIN TRACK COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "CartSwitcherInternalName", "Cart Switcher"),
				NSLOCTEXT("T66.ArcadeCatalog", "CartSwitcherDescription", "Switch mine tracks into the active lane."),
				NSLOCTEXT("T66.ArcadeCatalog", "CartSwitcherCode", "CRT"),
				FLinearColor(0.94f, 0.56f, 0.18f, 1.f)
			},
			{
				ET66ArcadeGameType::CrystalDash,
				FName(TEXT("Arcade_CrystalDash")),
				NSLOCTEXT("T66.ArcadeCatalog", "CrystalDashPrototypeName", "SUBWAY SURFERS COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "CrystalDashInternalName", "Crystal Dash"),
				NSLOCTEXT("T66.ArcadeCatalog", "CrystalDashDescription", "Dash through crystals while avoiding hazards."),
				NSLOCTEXT("T66.ArcadeCatalog", "CrystalDashCode", "DSH"),
				FLinearColor(0.62f, 0.72f, 1.f, 1.f)
			},
			{
				ET66ArcadeGameType::PotionPour,
				FName(TEXT("Arcade_PotionPour")),
				NSLOCTEXT("T66.ArcadeCatalog", "PotionPourPrototypeName", "PERFECT POUR COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "PotionPourInternalName", "Potion Pour"),
				NSLOCTEXT("T66.ArcadeCatalog", "PotionPourDescription", "Stop the pour on the glowing mark."),
				NSLOCTEXT("T66.ArcadeCatalog", "PotionPourCode", "POT"),
				FLinearColor(0.90f, 0.32f, 0.56f, 1.f)
			},
			{
				ET66ArcadeGameType::RelicStack,
				FName(TEXT("Arcade_RelicStack")),
				NSLOCTEXT("T66.ArcadeCatalog", "RelicStackPrototypeName", "STACK COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "RelicStackInternalName", "Relic Stack"),
				NSLOCTEXT("T66.ArcadeCatalog", "RelicStackDescription", "Drop moving relics over the center stack."),
				NSLOCTEXT("T66.ArcadeCatalog", "RelicStackCode", "REL"),
				FLinearColor(0.72f, 0.92f, 0.28f, 1.f)
			},
			{
				ET66ArcadeGameType::ShieldParry,
				FName(TEXT("Arcade_ShieldParry")),
				NSLOCTEXT("T66.ArcadeCatalog", "ShieldParryPrototypeName", "ARROW PARRY COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "ShieldParryInternalName", "Shield Parry"),
				NSLOCTEXT("T66.ArcadeCatalog", "ShieldParryDescription", "Parry the lit projectile direction."),
				NSLOCTEXT("T66.ArcadeCatalog", "ShieldParryCode", "SHD"),
				FLinearColor(0.18f, 0.72f, 1.f, 1.f)
			},
			{
				ET66ArcadeGameType::MimicMemory,
				FName(TEXT("Arcade_MimicMemory")),
				NSLOCTEXT("T66.ArcadeCatalog", "MimicMemoryPrototypeName", "SIMON SAYS COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "MimicMemoryInternalName", "Mimic Memory"),
				NSLOCTEXT("T66.ArcadeCatalog", "MimicMemoryDescription", "Repeat the chest sequence under pressure."),
				NSLOCTEXT("T66.ArcadeCatalog", "MimicMemoryCode", "MEM"),
				FLinearColor(0.88f, 0.12f, 0.10f, 1.f)
			},
			{
				ET66ArcadeGameType::BombSorter,
				FName(TEXT("Arcade_BombSorter")),
				NSLOCTEXT("T66.ArcadeCatalog", "BombSorterPrototypeName", "SORTING GAME COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "BombSorterInternalName", "Bomb Sorter"),
				NSLOCTEXT("T66.ArcadeCatalog", "BombSorterDescription", "Sort the lit bomb into the matching chute."),
				NSLOCTEXT("T66.ArcadeCatalog", "BombSorterCode", "BOM"),
				FLinearColor(0.94f, 0.18f, 0.14f, 1.f)
			},
			{
				ET66ArcadeGameType::LanternLeap,
				FName(TEXT("Arcade_LanternLeap")),
				NSLOCTEXT("T66.ArcadeCatalog", "LanternLeapPrototypeName", "DOODLE JUMP COPY"),
				NSLOCTEXT("T66.ArcadeCatalog", "LanternLeapInternalName", "Lantern Leap"),
				NSLOCTEXT("T66.ArcadeCatalog", "LanternLeapDescription", "Leap onto the glowing lantern platform."),
				NSLOCTEXT("T66.ArcadeCatalog", "LanternLeapCode", "LMP"),
				FLinearColor(0.54f, 0.95f, 0.36f, 1.f)
			},
		};

		return Entries;
	}
}

const TArray<FT66ArcadeGameCatalogEntry>& T66ArcadeGameCatalog::GetPlayableEntries()
{
	return T66BuildArcadeCatalog();
}

const FT66ArcadeGameCatalogEntry* T66ArcadeGameCatalog::FindEntry(const ET66ArcadeGameType GameType)
{
	for (const FT66ArcadeGameCatalogEntry& Entry : GetPlayableEntries())
	{
		if (Entry.GameType == GameType)
		{
			return &Entry;
		}
	}

	return nullptr;
}

bool T66ArcadeGameCatalog::IsPlayable(const ET66ArcadeGameType GameType)
{
	return FindEntry(GameType) != nullptr;
}

FName T66ArcadeGameCatalog::GetRowID(const ET66ArcadeGameType GameType)
{
	if (const FT66ArcadeGameCatalogEntry* Entry = FindEntry(GameType))
	{
		return Entry->RowID;
	}

	return NAME_None;
}

FText T66ArcadeGameCatalog::GetPrototypeDisplayName(const ET66ArcadeGameType GameType)
{
	if (const FT66ArcadeGameCatalogEntry* Entry = FindEntry(GameType))
	{
		return Entry->PrototypeDisplayName;
	}

	return NSLOCTEXT("T66.ArcadeCatalog", "UnknownPrototypeName", "ARCADE COPY");
}

FText T66ArcadeGameCatalog::GetInternalDisplayName(const ET66ArcadeGameType GameType)
{
	if (const FT66ArcadeGameCatalogEntry* Entry = FindEntry(GameType))
	{
		return Entry->InternalDisplayName;
	}

	return NSLOCTEXT("T66.ArcadeCatalog", "UnknownInternalName", "Arcade");
}

FText T66ArcadeGameCatalog::GetDescription(const ET66ArcadeGameType GameType)
{
	if (const FT66ArcadeGameCatalogEntry* Entry = FindEntry(GameType))
	{
		return Entry->Description;
	}

	return NSLOCTEXT("T66.ArcadeCatalog", "UnknownDescription", "Boot the selected arcade machine cartridge.");
}

FText T66ArcadeGameCatalog::GetShortCode(const ET66ArcadeGameType GameType)
{
	if (const FT66ArcadeGameCatalogEntry* Entry = FindEntry(GameType))
	{
		return Entry->ShortCode;
	}

	return NSLOCTEXT("T66.ArcadeCatalog", "UnknownCode", "???");
}

FLinearColor T66ArcadeGameCatalog::GetAccentColor(const ET66ArcadeGameType GameType, const int32 FallbackIndex)
{
	if (const FT66ArcadeGameCatalogEntry* Entry = FindEntry(GameType))
	{
		return Entry->AccentColor;
	}

	static const FLinearColor Palette[] =
	{
		FLinearColor(0.16f, 0.82f, 0.78f, 1.f),
		FLinearColor(0.94f, 0.56f, 0.18f, 1.f),
		FLinearColor(0.62f, 0.72f, 1.f, 1.f),
		FLinearColor(0.90f, 0.32f, 0.56f, 1.f),
		FLinearColor(0.72f, 0.92f, 0.28f, 1.f),
	};
	return Palette[FMath::Abs(FallbackIndex) % UE_ARRAY_COUNT(Palette)];
}

bool T66ArcadeGameCatalog::TryResolveRowData(const UObject* WorldContextObject, const FName RowID, FT66ArcadeInteractableData& OutData)
{
	if (RowID.IsNone())
	{
		return false;
	}

	if (const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr)
	{
		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance()))
		{
			if (T66GI->GetArcadeInteractableData(RowID, OutData))
			{
				return true;
			}
		}
	}

	if (UDataTable* DataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_ArcadeInteractables.DT_ArcadeInteractables")))
	{
		if (const FT66ArcadeInteractableRow* FoundRow = DataTable->FindRow<FT66ArcadeInteractableRow>(RowID, TEXT("ArcadeGameCatalogResolve")))
		{
			OutData = FoundRow->ArcadeData;
			if (OutData.ArcadeID.IsNone())
			{
				OutData.ArcadeID = RowID;
			}
			return true;
		}
	}

	return false;
}

bool T66ArcadeGameCatalog::BuildSessionDataForGame(
	const UObject* WorldContextObject,
	const ET66ArcadeGameType GameType,
	FT66ArcadeInteractableData& OutData)
{
	if (!IsPlayable(GameType))
	{
		return false;
	}

	FT66ArcadeInteractableData SessionData;
	if (const FName RowID = GetRowID(GameType); RowID.IsNone() || !TryResolveRowData(WorldContextObject, RowID, SessionData))
	{
		SessionData.ArcadeID = GetRowID(GameType);
		SessionData.InteractionVerb = FText::Format(
			NSLOCTEXT("T66.ArcadeCatalog", "FallbackPlayPrototypeVerb", "play {0}"),
			GetPrototypeDisplayName(GameType));
	}

	SessionData.ArcadeClass = ET66ArcadeInteractableClass::PopupArcade;
	SessionData.ArcadeGameType = GameType;
	SessionData.DisplayName = GetPrototypeDisplayName(GameType);
	OutData = MoveTemp(SessionData);
	return true;
}
