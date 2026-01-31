// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TutorialPromptActor.generated.h"

class UBoxComponent;
class UTextRenderComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FT66TutorialPromptPassed);

/** Simple tutorial sky-text prompt that advances when the player passes it. */
UCLASS(Blueprintable)
class T66_API AT66TutorialPromptActor : public AActor
{
	GENERATED_BODY()

public:
	AT66TutorialPromptActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial")
	TObjectPtr<UTextRenderComponent> Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText PromptText;

	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FT66TutorialPromptPassed OnPassed;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void SetActive(bool bActive);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bTriggered = false;
};

