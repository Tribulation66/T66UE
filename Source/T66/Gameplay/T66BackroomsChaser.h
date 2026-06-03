// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "T66BackroomsChaser.generated.h"

class AT66HeroBase;
class UStaticMeshComponent;

/** Unkillable Backrooms chaser. Movement target/pathing is owned by GameMode's Backrooms maze state. */
UCLASS(Blueprintable)
class T66_API AT66BackroomsChaser : public ACharacter
{
	GENERATED_BODY()

public:
	AT66BackroomsChaser();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	FName GetUniqueEnemyID() const { return UniqueEnemyID; }
	int32 GetTouchDamageHP() const { return TouchDamageHP; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void LoadUniqueEnemyData();
	void ApplyVisual();
	void TryTouchHero(AT66HeroBase* Hero);

	FName UniqueEnemyID = FName(TEXT("BackroomsChaser"));
	FName CharacterVisualID = FName(TEXT("Slime"));
	float MoveSpeed = 760.f;
	int32 TouchDamageHP = 999999;
	bool bTouchTriggered = false;
};
