// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66VendorNPC.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class T66_API AT66VendorNPC : public AActor
{
	GENERATED_BODY()

public:
	AT66VendorNPC();

	/** Interaction radius for F to sell */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> SphereComponent;

	/** Big cylinder visual */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	/** Try to sell first inventory item. Returns true if sold. */
	UFUNCTION(BlueprintCallable, Category = "Vendor")
	bool TrySellFirstItem();
};
