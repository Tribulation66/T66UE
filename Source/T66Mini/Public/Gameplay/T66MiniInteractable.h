// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66MiniDataTypes.h"
#include "GameFramework/Actor.h"
#include "T66MiniInteractable.generated.h"

class UBillboardComponent;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UTexture2D;
class UT66MiniShadowComponent;
class AT66MiniPlayerPawn;

UCLASS()
class T66MINI_API AT66MiniInteractable : public AActor
{
	GENERATED_BODY()

public:
	AT66MiniInteractable();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeInteractable(
		ET66MiniInteractableType InType,
		UTexture2D* InTexture,
		float InLifetimeSeconds = 12.f,
		const FString& InVisualID = FString(),
		int32 InMaterialReward = 0,
		int32 InGoldReward = 0,
		float InExperienceReward = 0.f,
		float InHealAmount = 0.f,
		bool bInRequiresManualInteract = false);
	ET66MiniInteractableType GetInteractableType() const { return InteractableType; }
	float GetLifetimeRemaining() const { return LifetimeRemaining; }
	const FString& GetVisualID() const { return VisualID; }
	bool RequiresManualInteract() const { return bRequiresManualInteract; }
	bool TryInteract(AT66MiniPlayerPawn* PlayerPawn);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void ConsumeInteractable(AT66MiniPlayerPawn* PlayerPawn);
	AT66MiniPlayerPawn* FindClosestPlayerPawn(bool bRequireAlive = true) const;
	void UpdateLifetimePresentation();
	void RefreshVisuals();

	UFUNCTION()
	void OnRep_InteractableVisualState();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBillboardComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> IndicatorMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UT66MiniShadowComponent> ShadowComponent;

	UPROPERTY(ReplicatedUsing = OnRep_InteractableVisualState)
	ET66MiniInteractableType InteractableType = ET66MiniInteractableType::TreasureChest;

	float LifetimeRemaining = 12.f;
	float HoverPhase = 0.f;
	UPROPERTY(Replicated)
	int32 MaterialReward = 0;

	UPROPERTY(Replicated)
	int32 GoldReward = 0;

	UPROPERTY(Replicated)
	float ExperienceReward = 0.f;

	UPROPERTY(Replicated)
	float HealAmount = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_InteractableVisualState)
	FString VisualID;

	UPROPERTY(Replicated)
	bool bRequiresManualInteract = false;

	bool bConsumed = false;
};
