// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66ItemPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class T66_API AT66ItemPickup : public AActor
{
	GENERATED_BODY()

public:
	AT66ItemPickup();

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	FName ItemID;

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItemID(FName InItemID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	FName GetItemID() const { return ItemID; }

	/** Interaction radius for F to collect */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

protected:
	virtual void BeginPlay() override;
};
