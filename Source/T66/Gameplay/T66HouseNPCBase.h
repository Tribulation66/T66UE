// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66HouseNPCBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class USkeletalMeshComponent;
class AT66HeroBase;
class APlayerController;
class UPrimitiveComponent;
struct FHouseNPCData;

/**
 * Base class for corner-house NPC placeholders.
 * - Cylinder visual + floating name text
 * - Interaction sphere (F)
 * - Safe zone sphere (enemies should not enter; hero can't shoot while inside)
 */
UCLASS(Blueprintable)
class T66_API AT66HouseNPCBase : public AActor
{
	GENERATED_BODY()

public:
	AT66HouseNPCBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> SafeZoneSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	/** Imported skeletal mesh visual (optional; driven by DT_CharacterVisuals). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UTextRenderComponent> NameText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FText NPCName = NSLOCTEXT("T66.NPC", "NPC", "NPC");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor NPCColor = FLinearColor::White;

	/** Row ID in DT_HouseNPCs (e.g. Vendor, Gambler, Saint, Ouroboros). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FName NPCID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafeZone")
	float SafeZoneRadius = 650.f;

	/** Visible safe-zone bubble (sphere). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SafeZone")
	TObjectPtr<UStaticMeshComponent> SafeZoneVisual;

	/** Called by PlayerController when pressing F nearby. Returns true if handled. */
	virtual bool Interact(APlayerController* PC);

	/** Apply name/color/radius to components. */
	UFUNCTION(BlueprintCallable, Category = "Visuals")
	void ApplyVisuals();

	/** Load per-NPC tuning from DT_HouseNPCs if available. */
	void LoadFromDataTable();

	/** Safe-zone radius in world units (used by enemy avoidance/repulsion). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SafeZone")
	float GetSafeZoneRadius() const { return SafeZoneRadius; }

protected:
	/** Optional per-NPC customization beyond name/color/radius. */
	virtual void ApplyNPCData(const FHouseNPCData& Data);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** If true, NPC smoothly rotates in place to face the player (yaw only). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facing")
	bool bFacePlayerAlways = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facing")
	float FacePlayerYawInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facing")
	float FacePlayerMaxDistance = 2500.f;

	UFUNCTION()
	void OnSafeZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSafeZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	int32 HeroOverlapCount = 0;

	UPROPERTY(Transient)
	bool bUsingCharacterVisual = false;
};

