// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Traps/T66TrapBase.h"
#include "T66ObstacleTrap.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
class UShapeComponent;
class UStaticMeshComponent;

UCLASS(Abstract, Blueprintable)
class T66_API AT66ObstacleTrapBase : public AT66TrapBase
{
	GENERATED_BODY()

public:
	AT66ObstacleTrapBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Obstacle", meta = (ClampMin = "0.0"))
	float LaunchXY = 9500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Obstacle", meta = (ClampMin = "0.0"))
	float LaunchZ = 850.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Obstacle", meta = (ClampMin = "0.0"))
	float ReactionCooldownSeconds = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Visual")
	FLinearColor BaseColor = FLinearColor(0.13f, 0.16f, 0.18f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Visual")
	FLinearColor AccentColor = FLinearColor(0.92f, 0.70f, 0.24f, 1.f);

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void HandleTrapEnabledChanged() override;
	virtual FVector BuildLaunchVelocity(const class AT66HeroBase* Hero, const UPrimitiveComponent* HitComponent, const FVector& HitLocation) const;
	virtual void UpdateObstacleVisuals();

	void ConfigureReactionZone(UShapeComponent* InReactionZone);
	bool TryApplyObstacleReaction(AActor* OtherActor, const UPrimitiveComponent* OtherComp, const FVector& HitLocation);
	FVector ResolveRadialLaunchDirection(const class AT66HeroBase* Hero) const;
	void ApplyMeshColor(UStaticMeshComponent* Mesh, const FLinearColor& Color) const;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UShapeComponent> ReactionZone;

private:
	UFUNCTION()
	void OnObstacleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	float LastReactionTimeSeconds = -9999.f;
};

UCLASS(Blueprintable)
class T66_API AT66SweeperArmTrap : public AT66ObstacleTrapBase
{
	GENERATED_BODY()

public:
	AT66SweeperArmTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Sweeper", meta = (ClampMin = "100.0"))
	float ArmLength = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Sweeper", meta = (ClampMin = "20.0"))
	float ArmThickness = 110.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Sweeper")
	float RotationSpeedDegPerSecond = 105.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Sweeper")
	float ArmHeight = 115.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual FVector BuildLaunchVelocity(const class AT66HeroBase* Hero, const UPrimitiveComponent* HitComponent, const FVector& HitLocation) const override;
	virtual void UpdateObstacleVisuals() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> ArmPivot;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ArmMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HubMesh;
};

UCLASS(Blueprintable)
class T66_API AT66BumperTrap : public AT66ObstacleTrapBase
{
	GENERATED_BODY()

public:
	AT66BumperTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Bumper", meta = (ClampMin = "50.0"))
	float Radius = 320.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Bumper", meta = (ClampMin = "20.0"))
	float Height = 210.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Bumper", meta = (ClampMin = "0.0"))
	float TravelDistance = 240.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Bumper", meta = (ClampMin = "0.20"))
	float CyclePeriodSeconds = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Bumper")
	float InitialPhaseSeconds = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void UpdateObstacleVisuals() override;

private:
	void UpdateBumperPose(float WorldSeconds);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> BumperMotionRoot;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BumperMesh;
};

UCLASS(Blueprintable)
class T66_API AT66WallBumperTrap : public AT66ObstacleTrapBase
{
	GENERATED_BODY()

public:
	AT66WallBumperTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|WallBumper", meta = (ClampMin = "50.0"))
	float Width = 680.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|WallBumper", meta = (ClampMin = "50.0"))
	float PlateThickness = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|WallBumper", meta = (ClampMin = "50.0"))
	float Height = 520.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|WallBumper", meta = (ClampMin = "0.0"))
	float TravelDistance = 320.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|WallBumper", meta = (ClampMin = "0.20"))
	float CyclePeriodSeconds = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|WallBumper")
	float InitialPhaseSeconds = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual FVector BuildLaunchVelocity(const class AT66HeroBase* Hero, const UPrimitiveComponent* HitComponent, const FVector& HitLocation) const override;
	virtual void UpdateObstacleVisuals() override;

private:
	void UpdateBumperPose(float WorldSeconds);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> BumperMotionRoot;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WallMountMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BumperMesh;
};

UCLASS(Blueprintable)
class T66_API AT66LaunchPadTrap : public AT66WallBumperTrap
{
	GENERATED_BODY()

public:
	AT66LaunchPadTrap();
};

UCLASS(Blueprintable)
class T66_API AT66CeilingHammerTrap : public AT66ObstacleTrapBase
{
	GENERATED_BODY()

public:
	AT66CeilingHammerTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Hammer", meta = (ClampMin = "200.0"))
	float HangHeight = 1250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Hammer", meta = (ClampMin = "100.0"))
	float HammerLength = 880.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Hammer", meta = (ClampMin = "40.0"))
	float HammerHeadSize = 260.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Hammer", meta = (ClampMin = "1.0"))
	float SwingPeriodSeconds = 2.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Hammer")
	float MaxSwingAngleDegrees = 58.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Hammer")
	float InitialPhaseSeconds = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual FVector BuildLaunchVelocity(const class AT66HeroBase* Hero, const UPrimitiveComponent* HitComponent, const FVector& HitLocation) const override;
	virtual void UpdateObstacleVisuals() override;

private:
	void UpdateHammerPose(float WorldSeconds);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> HammerPivot;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CableMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HammerHeadMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CeilingMountMesh;

	float LastSwingVelocitySign = 1.f;
};
