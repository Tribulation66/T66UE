// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66IdleDataTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66IdleDataSubsystem.generated.h"

UCLASS()
class T66IDLE_API UT66IdleDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Idle")
	void ReloadData();

	const TArray<FT66IdleHeroDefinition>& GetHeroes() const { return Heroes; }
	const TArray<FT66IdleCompanionDefinition>& GetCompanions() const { return Companions; }
	const TArray<FT66IdleItemDefinition>& GetItems() const { return Items; }
	const TArray<FT66IdleIdolDefinition>& GetIdols() const { return Idols; }
	const TArray<FT66IdleEnemyDefinition>& GetEnemies() const { return Enemies; }
	const TArray<FT66IdleZoneDefinition>& GetZones() const { return Zones; }
	const TArray<FT66IdleStageDefinition>& GetStages() const { return Stages; }
	const FT66IdleTuningDefinition& GetTuning() const { return Tuning; }

	const FT66IdleHeroDefinition* FindHero(FName HeroID) const;
	const FT66IdleCompanionDefinition* FindCompanion(FName CompanionID) const;
	const FT66IdleItemDefinition* FindItem(FName ItemID) const;
	const FT66IdleIdolDefinition* FindIdol(FName IdolID) const;
	const FT66IdleEnemyDefinition* FindEnemy(FName EnemyID) const;
	const FT66IdleZoneDefinition* FindZone(FName ZoneID) const;
	const FT66IdleZoneDefinition* FindZoneForStage(int32 Stage) const;
	const FT66IdleStageDefinition* FindStage(FName StageID) const;
	const FT66IdleStageDefinition* FindStageByIndex(int32 StageIndex) const;

private:
	void LoadHeroes();
	void LoadCompanions();
	void LoadItems();
	void LoadIdols();
	void LoadEnemies();
	void LoadZones();
	void LoadStages();
	void LoadTuning();

	TArray<FT66IdleHeroDefinition> Heroes;
	TArray<FT66IdleCompanionDefinition> Companions;
	TArray<FT66IdleItemDefinition> Items;
	TArray<FT66IdleIdolDefinition> Idols;
	TArray<FT66IdleEnemyDefinition> Enemies;
	TArray<FT66IdleZoneDefinition> Zones;
	TArray<FT66IdleStageDefinition> Stages;
	FT66IdleTuningDefinition Tuning;
};
