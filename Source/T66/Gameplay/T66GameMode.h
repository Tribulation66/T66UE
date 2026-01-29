// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T66GameMode.generated.h"

class AT66HeroBase;
class AT66CompanionBase;
class UT66GameInstance;
class ADirectionalLight;
class ASkyLight;
class APlayerStart;

/**
 * Game Mode for Tribulation 66 gameplay levels
 *
 * Responsibilities:
 * - Spawn the correct hero based on Game Instance selection
 * - Initialize hero with data from DataTable
 * - Handle basic game flow
 * - Auto-setup level with floor/lighting if missing (for development)
 */
UCLASS(Blueprintable)
class T66_API AT66GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AT66GameMode();

	/** Default hero class to spawn (can be overridden per-hero in DataTable) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AT66HeroBase> DefaultHeroClass;

	/** Whether to auto-create floor/lighting if missing (useful for development) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Setup")
	bool bAutoSetupLevel = true;

	/** Default spawn height when no PlayerStart exists */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level Setup")
	float DefaultSpawnHeight = 200.f;

	/**
	 * Spawn the hero based on current Game Instance selections
	 * Called automatically or can be called manually for testing
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	AT66HeroBase* SpawnSelectedHero(AController* Controller);

protected:
	virtual void BeginPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/** Get the Game Instance cast to our type */
	UT66GameInstance* GetT66GameInstance() const;

	/** Set up basic level elements if missing (floor, lighting, player start) */
	void EnsureLevelSetup();

	/** Spawn a floor plane if none exists */
	void SpawnFloorIfNeeded();

	/** Spawn lighting if none exists */
	void SpawnLightingIfNeeded();

	/** Spawn a PlayerStart if none exists */
	void SpawnPlayerStartIfNeeded();

	/** Spawn the selected companion (if any) and attach follow behavior */
	void SpawnCompanionForPlayer(AController* Player);

	/** Spawn vendor NPC near the hero (big cylinder) */
	void SpawnVendorForPlayer(AController* Player);

private:
	/** Track spawned setup actors for cleanup */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedSetupActors;
};
