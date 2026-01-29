// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66HouseBlock.generated.h"

class UStaticMeshComponent;

/** Simple placeholder "house" as a big colored cube. */
UCLASS(Blueprintable)
class T66_API AT66HouseBlock : public AActor
{
	GENERATED_BODY()

public:
	AT66HouseBlock();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "House")
	FLinearColor HouseColor = FLinearColor::Black;

	/** Re-apply the color to the cube material. */
	void ApplyVisuals();

protected:
	virtual void BeginPlay() override;
};

