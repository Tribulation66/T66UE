// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiniPickup.generated.h"

class UBillboardComponent;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UTexture2D;
class UT66MiniShadowComponent;

UCLASS()
class T66MINI_API AT66MiniPickup : public AActor
{
	GENERATED_BODY()

public:
	AT66MiniPickup();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializePickup(int32 InMaterialValue, float InExperienceValue, float InHealValue, UTexture2D* InTexture, const FString& InVisualID = FString(), float InLifetimeRemaining = 20.f, FName InGrantedItemID = NAME_None);
	int32 GetMaterialValue() const { return MaterialValue; }
	float GetExperienceValue() const { return ExperienceValue; }
	float GetHealValue() const { return HealValue; }
	float GetLifetimeRemaining() const { return LifetimeRemaining; }
	const FString& GetVisualID() const { return VisualID; }
	FName GetGrantedItemID() const { return GrantedItemID; }

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

	void CollectPickup(class AT66MiniPlayerPawn* PlayerPawn);
	class AT66MiniPlayerPawn* FindClosestPlayerPawn(bool bRequireAlive = true) const;
	void RefreshVisuals();

	UFUNCTION()
	void OnRep_VisualState();

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

	UPROPERTY(Replicated)
	int32 MaterialValue = 0;

	UPROPERTY(Replicated)
	float ExperienceValue = 0.f;

	UPROPERTY(Replicated)
	float HealValue = 0.f;

	float LifetimeRemaining = 20.f;
	float HoverPhase = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_VisualState)
	FString VisualID;

	UPROPERTY(Replicated)
	FName GrantedItemID = NAME_None;

	bool bCollected = false;
};
