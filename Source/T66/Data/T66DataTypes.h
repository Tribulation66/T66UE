// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "T66DataTypes.generated.h"

/**
 * Hero data row for the Hero DataTable
 * Each row represents one selectable hero in the game
 */
USTRUCT(BlueprintType)
struct T66_API FHeroData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier for this hero (used for spawning, saving, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName HeroID;

	/** Display name shown in UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText DisplayName;

	/** Short description of the hero's playstyle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText Description;

	/** Placeholder color for the cylinder representation (until real mesh is added) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor PlaceholderColor = FLinearColor::White;

	/** The actual character Blueprint class to spawn (can be null for placeholder) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSoftClassPtr<APawn> HeroClass;

	/** Hero portrait texture for UI (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> Portrait;

	/** Map theme associated with this hero (for Solo runs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FName MapTheme;

	/** Whether this hero is unlocked by default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	bool bUnlockedByDefault = true;

	FHeroData()
		: HeroID(NAME_None)
		, PlaceholderColor(FLinearColor::White)
		, bUnlockedByDefault(true)
	{}
};

/**
 * Companion data row for the Companion DataTable
 * Each row represents one selectable companion
 */
USTRUCT(BlueprintType)
struct T66_API FCompanionData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier for this companion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName CompanionID;

	/** Display name shown in UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText DisplayName;

	/** Lore text (begins with "She tells me...") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (MultiLine = true))
	FText LoreText;

	/** Placeholder color for visual representation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor PlaceholderColor = FLinearColor::White;

	/** The companion actor class to spawn alongside the hero */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSoftClassPtr<AActor> CompanionClass;

	/** Companion portrait texture for UI (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> Portrait;

	/** Whether this companion is unlocked by default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	bool bUnlockedByDefault = true;

	FCompanionData()
		: CompanionID(NAME_None)
		, PlaceholderColor(FLinearColor::White)
		, bUnlockedByDefault(true)
	{}
};

/**
 * Party size enumeration
 */
UENUM(BlueprintType)
enum class ET66PartySize : uint8
{
	Solo UMETA(DisplayName = "Solo"),
	Duo UMETA(DisplayName = "Duo"),
	Trio UMETA(DisplayName = "Trio")
};

/**
 * Difficulty levels matching leaderboard categories
 */
UENUM(BlueprintType)
enum class ET66Difficulty : uint8
{
	Easy UMETA(DisplayName = "Easy"),
	Medium UMETA(DisplayName = "Medium"),
	Hard UMETA(DisplayName = "Hard"),
	VeryHard UMETA(DisplayName = "Very Hard"),
	Impossible UMETA(DisplayName = "Impossible"),
	Perdition UMETA(DisplayName = "Perdition"),
	Final UMETA(DisplayName = "Final")
};

/**
 * Body type for heroes and companions
 */
UENUM(BlueprintType)
enum class ET66BodyType : uint8
{
	TypeA UMETA(DisplayName = "Type A"),
	TypeB UMETA(DisplayName = "Type B")
};
