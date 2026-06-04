// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TowerDescentHole.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
class AT66EnemyBase;
class AT66HeroBase;

UCLASS(Blueprintable)
class T66_API AT66TowerDescentHole : public AActor
{
	GENERATED_BODY()

public:
	AT66TowerDescentHole();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> GateCoverMesh;

	void InitializeHole(
		int32 InFromFloorNumber,
		int32 InToFloorNumber,
		const FVector& InTriggerExtent,
		bool bInRequiresWeaponSelection,
		bool bInRequiresGuardianDefeated);
	void SetGuardianEnemy(AT66EnemyBase* InGuardianEnemy);
	bool Interact(AT66HeroBase* Hero);
	int32 GetFromFloorNumber() const { return FromFloorNumber; }
	int32 GetToFloorNumber() const { return ToFloorNumber; }
#if !UE_BUILD_SHIPPING
	bool AutomationCanOpenForHero(const AT66HeroBase* Hero) const;
	AT66EnemyBase* AutomationGetGuardianEnemy() const;
#endif

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void RefreshGateVisualState();
	bool CanOpenGate(const AT66HeroBase* Hero) const;
	bool IsGuardianDefeated() const;
	void TriggerDescentForHero(AT66HeroBase* Hero);

	int32 FromFloorNumber = INDEX_NONE;
	int32 ToFloorNumber = INDEX_NONE;
	bool bGateOpen = false;
	bool bRequiresWeaponSelection = false;
	bool bRequiresGuardianDefeated = false;
	TSet<TWeakObjectPtr<AActor>> ActiveActors;
	TWeakObjectPtr<AT66EnemyBase> GuardianEnemy;
};
