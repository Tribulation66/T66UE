// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66MiniDataTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MiniDataSubsystem.generated.h"

UCLASS()
class T66MINI_API UT66MiniDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Mini")
	void ReloadData();

	const TArray<FT66MiniHeroDefinition>& GetHeroes() const { return Heroes; }
	const TArray<FT66MiniIdolDefinition>& GetIdols() const { return Idols; }
	const TArray<FT66MiniCompanionDefinition>& GetCompanions() const { return Companions; }
	const TArray<FT66MiniDifficultyDefinition>& GetDifficulties() const { return Difficulties; }
	const TArray<FT66MiniEnemyDefinition>& GetEnemies() const { return Enemies; }
	const TArray<FT66MiniBossDefinition>& GetBosses() const { return Bosses; }
	const TArray<FT66MiniWaveDefinition>& GetWaves() const { return Waves; }
	const TArray<FT66MiniInteractableDefinition>& GetInteractables() const { return Interactables; }
	const TArray<FT66MiniItemDefinition>& GetItems() const { return Items; }

	const FT66MiniHeroDefinition* FindHero(FName HeroID) const;
	const FT66MiniIdolDefinition* FindIdol(FName IdolID) const;
	const FT66MiniCompanionDefinition* FindCompanion(FName CompanionID) const;
	const FT66MiniDifficultyDefinition* FindDifficulty(FName DifficultyID) const;
	const FT66MiniEnemyDefinition* FindEnemy(FName EnemyID) const;
	const FT66MiniBossDefinition* FindBoss(FName BossID) const;
	const FT66MiniWaveDefinition* FindWave(FName DifficultyID, int32 WaveIndex) const;
	const FT66MiniInteractableDefinition* FindInteractable(FName InteractableID) const;
	const FT66MiniItemDefinition* FindItem(FName ItemID) const;

private:
	void LoadHeroes();
	void LoadIdols();
	void LoadCompanions();
	void LoadDifficulties();
	void LoadEnemies();
	void LoadBosses();
	void LoadWaves();
	void LoadInteractables();
	void LoadItems();

	TArray<FT66MiniHeroDefinition> Heroes;
	TArray<FT66MiniIdolDefinition> Idols;
	TArray<FT66MiniCompanionDefinition> Companions;
	TArray<FT66MiniDifficultyDefinition> Difficulties;
	TArray<FT66MiniEnemyDefinition> Enemies;
	TArray<FT66MiniBossDefinition> Bosses;
	TArray<FT66MiniWaveDefinition> Waves;
	TArray<FT66MiniInteractableDefinition> Interactables;
	TArray<FT66MiniItemDefinition> Items;
};
