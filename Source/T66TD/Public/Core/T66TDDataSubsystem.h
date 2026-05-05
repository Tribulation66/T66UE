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
	const TArray<FT66TDHeroCombatDefinition>& GetHeroCombatDefinitions() const { return HeroCombatDefinitions; }
	const TArray<FT66TDEnemyArchetypeDefinition>& GetEnemyArchetypes() const { return EnemyArchetypes; }
	const TMap<FName, float>& GetBattleTuningValues() const { return BattleTuningValues; }
	const TArray<FT66TDThemeModifierRule>& GetThemeModifierRules() const { return ThemeModifierRules; }
	const TArray<FT66TDDifficultyDefinition>& GetDifficulties() const { return Difficulties; }
	const TArray<FT66TDMapDefinition>& GetMaps() const { return Maps; }
	const TArray<FT66TDStageDefinition>& GetStages() const { return Stages; }

	const FT66TDHeroDefinition* FindHero(FName HeroID) const;
	const FT66TDHeroCombatDefinition* FindHeroCombatDefinition(FName HeroID) const;
	const FT66TDEnemyArchetypeDefinition* FindEnemyArchetype(FName EnemyID) const;
	const FT66TDDifficultyDefinition* FindDifficulty(FName DifficultyID) const;
	const FT66TDMapDefinition* FindMap(FName MapID) const;
	const FT66TDStageDefinition* FindStage(FName StageID) const;
	const FT66TDStageDefinition* FindStageForMap(FName MapID) const;
	const FT66TDStageDefinition* FindStage(FName DifficultyID, int32 StageIndex) const;
	const FT66TDMapLayoutDefinition* FindLayout(FName MapID) const;
	TArray<const FT66TDMapDefinition*> GetMapsForDifficulty(FName DifficultyID) const;
	TArray<const FT66TDStageDefinition*> GetStagesForDifficulty(FName DifficultyID) const;

private:
	void LoadHeroes();
	void LoadHeroCombatDefinitions();
	void LoadEnemyArchetypes();
	void LoadBattleTuning();
	void LoadThemeModifierRules();
	void LoadDifficulties();
	void LoadMaps();
	void LoadStages();
	void LoadLayouts();

	TArray<FT66TDHeroDefinition> Heroes;
	TArray<FT66TDHeroCombatDefinition> HeroCombatDefinitions;
	TArray<FT66TDEnemyArchetypeDefinition> EnemyArchetypes;
	TMap<FName, float> BattleTuningValues;
	TArray<FT66TDThemeModifierRule> ThemeModifierRules;
	TArray<FT66TDDifficultyDefinition> Difficulties;
	TArray<FT66TDMapDefinition> Maps;
	TArray<FT66TDStageDefinition> Stages;
	TMap<FName, FT66TDMapLayoutDefinition> Layouts;
};
