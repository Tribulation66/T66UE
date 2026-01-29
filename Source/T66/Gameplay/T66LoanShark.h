// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "T66LoanShark.generated.h"

class UStaticMeshComponent;
class UT66RunStateSubsystem;

/**
 * Loan Shark: spawns when entering a stage with unpaid debt (once the stage timer starts).
 * Chases the hero and deals touch damage. Despawns immediately when debt is paid to 0.
 */
UCLASS(Blueprintable)
class T66_API AT66LoanShark : public ACharacter
{
	GENERATED_BODY()

public:
	AT66LoanShark();

	/** Visible mesh (placeholder cylinder). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UT66RunStateSubsystem* GetRunState() const;

	void UpdateTuningFromDebt();

	int32 CachedDebt = -1;
	float LastTouchDamageTime = -9999.f;
	static constexpr float TouchDamageCooldown = 0.5f;

	// Tuning (loaded from DT_LoanShark if available)
	float BaseMoveSpeed = 650.f;
	float MoveSpeedPer100Debt = 50.f;
	int32 BaseDamageHearts = 1;
	int32 DebtPerExtraHeart = 200;

	int32 CurrentDamageHearts = 1;
};

