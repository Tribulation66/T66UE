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
class UStaticMesh;
class APlayerController;
class UPrimitiveComponent;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals|Cage")
	TObjectPtr<UStaticMeshComponent> CageVisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals|Cage")
	TObjectPtr<UStaticMeshComponent> CageBarFrontLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals|Cage")
	TObjectPtr<UStaticMeshComponent> CageBarFrontRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals|Cage")
	TObjectPtr<UStaticMeshComponent> CageBarBackLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals|Cage")
	TObjectPtr<UStaticMeshComponent> CageBarBackRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals|Cage")
	TObjectPtr<UStaticMeshComponent> CageTopBar;

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

	/** Marks this recruit as a locked stage-boss cage reward. */
	UFUNCTION(BlueprintCallable, Category = "Companion|Cage")
	void SetCagedForBossReward();

	/** Frees a stage-boss cage reward so the player can interact and unlock it. */
	UFUNCTION(BlueprintCallable, Category = "Companion|Cage")
	void FreeFromBossCage();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Companion|Cage")
	bool IsLockedInBossCage() const { return bLockedInBossCage; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Companion|Cage")
	bool IsBossCageUnlockReward() const { return bBossCageUnlockReward; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Companion|Cage")
	bool HasGrantedBossCageUnlock() const { return bUnlockGrantedFromBossCage; }

	/** Press-F interaction. Returns true if handled. */
	virtual bool Interact(APlayerController* PC);

#if !UE_BUILD_SHIPPING
	bool IsInteractionPromptVisibleForAutomation() const { return bInteractionPromptVisible; }
#endif

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PlaceholderMaterial;

	UPROPERTY(Transient)
	bool bUsingCharacterVisual = false;

	UPROPERTY(Transient)
	bool bBossCageUnlockReward = false;

	UPROPERTY(Transient)
	bool bLockedInBossCage = false;

	UPROPERTY(Transient)
	bool bFreedFromBossCage = false;

	UPROPERTY(Transient)
	bool bUnlockGrantedFromBossCage = false;

	UPROPERTY(Transient)
	int32 LocalHeroPromptOverlapCount = 0;

	UPROPERTY(Transient)
	bool bInteractionPromptVisible = false;

	UPROPERTY(Transient)
	bool bImportedCageVisualReady = false;

	UPROPERTY()
	TSoftObjectPtr<UStaticMesh> CageMeshOverride;

	void ApplyPlaceholderColor(const FLinearColor& Color);
	void ApplyCageColor(const FLinearColor& Color);
	void SetCageVisualsVisible(bool bVisible);
	void SnapToGround(bool bTreatOriginAsGroundContact);
	void RefreshInteractionPrompt();
	void HideInteractionPrompt();
	bool IsLocalHeroActor(const AActor* OtherActor) const;
	bool ShouldShowInteractionPrompt() const;

	UFUNCTION()
	void OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

