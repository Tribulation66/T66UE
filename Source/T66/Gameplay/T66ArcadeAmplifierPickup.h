// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "GameFramework/Actor.h"
#include "T66ArcadeAmplifierPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;

UCLASS(Blueprintable)
class T66_API AT66ArcadeAmplifierPickup : public AActor
{
	GENERATED_BODY()

public:
	AT66ArcadeAmplifierPickup();

	UFUNCTION(BlueprintCallable, Category = "Arcade|Amplifier")
	void ConfigureAmplifier(ET66HeroStatType InStatType, int32 InBonusStatPoints, float InDurationSeconds);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Amplifier")
	ET66HeroStatType StatType = ET66HeroStatType::Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Amplifier")
	int32 BonusStatPoints = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Amplifier")
	float DurationSeconds = 10.f;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void RefreshVisuals();
	FLinearColor ResolveStatTint() const;
};
