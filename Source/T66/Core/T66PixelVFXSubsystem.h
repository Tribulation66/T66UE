// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "T66PixelVFXSubsystem.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UENUM()
enum class ET66PixelVFXPriority : uint8
{
	Low,
	Medium,
	High,
};

/**
 * Shared pixel VFX facade.
 * Keeps Niagara pooling, but centralizes per-frame budgets and parameter writes.
 */
UCLASS()
class T66_API UT66PixelVFXSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	UNiagaraComponent* SpawnPixelAtLocation(
		const FVector& Location,
		const FLinearColor& Tint,
		const FVector2D& SpriteSize,
		ET66PixelVFXPriority Priority = ET66PixelVFXPriority::Medium,
		const FRotator& Rotation = FRotator::ZeroRotator,
		const FVector& Scale = FVector(1.f),
		UNiagaraSystem* SystemOverride = nullptr,
		bool bAutoDestroy = true,
		bool bAutoActivate = true);

	UNiagaraSystem* GetDefaultPixelSystem();

	int32 GetRequestedThisFrame() const { return RequestedThisFrame; }
	int32 GetEmittedThisFrame() const { return EmittedThisFrame; }
	int32 GetDroppedThisFrame() const { return DroppedThisFrame; }
	int64 GetTotalRequested() const { return TotalRequested; }
	int64 GetTotalEmitted() const { return TotalEmitted; }
	int64 GetTotalDropped() const { return TotalDropped; }

private:
	void ResetFrameBudgetIfNeeded();
	bool ConsumeBudget(ET66PixelVFXPriority Priority);

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> CachedPixelSystem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> CachedLegacyPixelSystem = nullptr;

	uint64 LastBudgetFrame = MAX_uint64;
	int32 RequestedThisFrame = 0;
	int32 EmittedThisFrame = 0;
	int32 DroppedThisFrame = 0;

	int64 TotalRequested = 0;
	int64 TotalEmitted = 0;
	int64 TotalDropped = 0;
};
