// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TrapBase.generated.h"

class AActor;
class AT66EnemyBase;
class AT66HeroBase;
class USceneComponent;

UENUM(BlueprintType)
enum class ET66TrapActivationMode : uint8
{
	Timed UMETA(DisplayName = "Timed"),
	Triggered UMETA(DisplayName = "Triggered"),
	Hybrid UMETA(DisplayName = "Hybrid"),
};

UENUM(BlueprintType)
enum class ET66TrapTriggerTarget : uint8
{
	HeroesOnly UMETA(DisplayName = "Heroes Only"),
	EnemiesOnly UMETA(DisplayName = "Enemies Only"),
	HeroesAndEnemies UMETA(DisplayName = "Heroes And Enemies"),
};

UCLASS(Abstract)
class T66_API AT66TrapBase : public AActor
{
	GENERATED_BODY()

public:
	AT66TrapBase();

	virtual FName GetTrapTypeID() const { return TrapTypeID; }
	virtual FName GetTrapFamilyID() const { return TrapFamilyID; }

	void SetTrapTypeID(const FName InTrapTypeID) { TrapTypeID = InTrapTypeID; }
	void SetTrapFamilyID(const FName InTrapFamilyID) { TrapFamilyID = InTrapFamilyID; }

	void SetTrapEnabled(bool bInEnabled);
	bool IsTrapEnabled() const { return bTrapEnabled; }

	void SetTowerFloorNumber(const int32 InTowerFloorNumber) { TowerFloorNumber = InTowerFloorNumber; }
	int32 GetTowerFloorNumber() const { return TowerFloorNumber; }

	void SetActivationMode(ET66TrapActivationMode InActivationMode) { ActivationMode = InActivationMode; }
	ET66TrapActivationMode GetActivationMode() const { return ActivationMode; }
	bool UsesTimedActivation() const;
	bool UsesTriggeredActivation() const;

	void SetTriggerTargetMode(ET66TrapTriggerTarget InTriggerTargetMode) { TriggerTargetMode = InTriggerTargetMode; }
	ET66TrapTriggerTarget GetTriggerTargetMode() const { return TriggerTargetMode; }

	void SetDamagesHeroes(bool bInDamagesHeroes) { bDamagesHeroes = bInDamagesHeroes; }
	void SetDamagesEnemies(bool bInDamagesEnemies) { bDamagesEnemies = bInDamagesEnemies; }
	bool CanDamageHeroes() const { return bDamagesHeroes; }
	bool CanDamageEnemies() const { return bDamagesEnemies; }

	bool TryTriggerTrap(AActor* TriggeringActor);
	void ResetTriggerLock() { bExternalTriggerLocked = false; }

	void ApplyProgressionScalars(float InDamageScalar, float InSpeedScalar);
	float GetProgressionDamageScalar() const { return ProgressionDamageScalar; }
	float GetProgressionSpeedScalar() const { return ProgressionSpeedScalar; }
	int32 ScaleTrapDamage(int32 BaseDamage) const;
	float ScaleTrapDuration(float BaseSeconds) const;
	bool CanAffectHero(const AT66HeroBase* Hero) const { return IsHeroTargetable(Hero); }
	bool CanAffectEnemy(const AT66EnemyBase* Enemy) const { return IsEnemyTargetable(Enemy); }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool IsHeroTargetable(const AT66HeroBase* Hero) const;
	bool IsEnemyTargetable(const AT66EnemyBase* Enemy) const;
	bool CanTriggerFromActor(const AActor* Actor) const;
	virtual void HandleTrapEnabledChanged();
	virtual void HandleProgressionScalarsChanged();
	virtual bool CanAcceptExternalTrigger() const;
	virtual bool HandleTrapTriggered(AActor* TriggeringActor);
	bool IsActorOnCompatibleTowerFloor(const AActor* Actor) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	bool bTrapEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	FName TrapTypeID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	FName TrapFamilyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	int32 TowerFloorNumber = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	ET66TrapActivationMode ActivationMode = ET66TrapActivationMode::Timed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	ET66TrapTriggerTarget TriggerTargetMode = ET66TrapTriggerTarget::HeroesAndEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	bool bDamagesHeroes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	bool bDamagesEnemies = true;

	bool bExternalTriggerLocked = false;
	float ProgressionDamageScalar = 1.0f;
	float ProgressionSpeedScalar = 1.0f;
};
