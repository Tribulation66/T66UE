// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TrapBase.generated.h"

class AT66HeroBase;
class USceneComponent;

UCLASS(Abstract)
class T66_API AT66TrapBase : public AActor
{
	GENERATED_BODY()

public:
	AT66TrapBase();

	virtual FName GetTrapTypeID() const { return TrapTypeID; }

	void SetTrapEnabled(const bool bInEnabled) { bTrapEnabled = bInEnabled; }
	bool IsTrapEnabled() const { return bTrapEnabled; }

	void SetTowerFloorNumber(const int32 InTowerFloorNumber) { TowerFloorNumber = InTowerFloorNumber; }
	int32 GetTowerFloorNumber() const { return TowerFloorNumber; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool IsHeroTargetable(const AT66HeroBase* Hero) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	bool bTrapEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	FName TrapTypeID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	int32 TowerFloorNumber = INDEX_NONE;
};
