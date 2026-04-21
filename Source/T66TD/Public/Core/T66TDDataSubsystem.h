// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66TDDataTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66TDDataSubsystem.generated.h"

UCLASS()
class T66TD_API UT66TDDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "TD")
	void ReloadData();

	const TArray<FT66TDHeroDefinition>& GetHeroes() const { return Heroes; }
	const TArray<FT66TDDifficultyDefinition>& GetDifficulties() const { return Difficulties; }
	const TArray<FT66TDMapDefinition>& GetMaps() const { return Maps; }

	const FT66TDHeroDefinition* FindHero(FName HeroID) const;
	const FT66TDDifficultyDefinition* FindDifficulty(FName DifficultyID) const;
	const FT66TDMapDefinition* FindMap(FName MapID) const;
	const FT66TDMapLayoutDefinition* FindLayout(FName MapID) const;
	TArray<const FT66TDMapDefinition*> GetMapsForDifficulty(FName DifficultyID) const;

private:
	void LoadHeroes();
	void LoadDifficulties();
	void LoadMaps();
	void LoadLayouts();

	TArray<FT66TDHeroDefinition> Heroes;
	TArray<FT66TDDifficultyDefinition> Difficulties;
	TArray<FT66TDMapDefinition> Maps;
	TMap<FName, FT66TDMapLayoutDefinition> Layouts;
};
