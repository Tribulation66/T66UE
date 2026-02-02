// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/T66DataTypes.h"
#include "T66RecruitableCompanion.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class UMaterialInstanceDynamic;
class APlayerController;

/**
 * A companion actor placed in-world for recruitment/switching.
 * Interactable with the same "press F" system as NPCs (world dialogue).
 */
UCLASS(Blueprintable)
class T66_API AT66RecruitableCompanion : public AActor
{
	GENERATED_BODY()

public:
	AT66RecruitableCompanion();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	/** Imported skeletal mesh visual (optional; driven by DT_CharacterVisuals). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

	UPROPERTY(BlueprintReadOnly, Category = "Companion")
	FName CompanionID;

	UPROPERTY(BlueprintReadOnly, Category = "Companion")
	FCompanionData CompanionData;

	/** Called by GameMode after spawning so we can apply data-driven visuals. */
	UFUNCTION(BlueprintCallable, Category = "Companion")
	void InitializeRecruit(const FCompanionData& InData);

	/** Press-F interaction. Returns true if handled. */
	virtual bool Interact(APlayerController* PC);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PlaceholderMaterial;

	UPROPERTY(Transient)
	bool bUsingCharacterVisual = false;

	void ApplyPlaceholderColor(const FLinearColor& Color);
	void SnapToGround(bool bTreatOriginAsGroundContact);
};

