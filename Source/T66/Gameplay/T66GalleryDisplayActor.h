// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66GalleryDisplayActor.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
class APlayerController;
struct FHitResult;

/** Lightweight inert actor used by the start-gallery wings to display character visual rows without gameplay AI. */
UCLASS(Blueprintable)
class T66_API AT66GalleryDisplayActor : public AActor
{
	GENERATED_BODY()

public:
	AT66GalleryDisplayActor();

	void ConfigureDisplayVisual(FName InVisualID, float InActorScale = 1.0f);
	void ConfigureInteractionPromptTarget(const FText& InPromptTargetName);
	UPrimitiveComponent* GetInteractionPromptPrimitive() const;
	bool Interact(APlayerController* PC);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ApplyConfiguredVisual();
	void RefreshInteractionPrompt();
	void HideInteractionPrompt();
	bool IsLocalHeroActor(const AActor* OtherActor) const;

	UFUNCTION()
	void OnPromptBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPromptEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, Category = "Gallery")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Gallery")
	TObjectPtr<USphereComponent> PromptSphere;

	UPROPERTY(VisibleAnywhere, Category = "Gallery")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Gallery")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(EditAnywhere, Category = "Gallery")
	FName VisualID = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Gallery")
	FText InteractionPromptTargetName;

	UPROPERTY(Transient)
	int32 LocalHeroPromptOverlapCount = 0;
};
