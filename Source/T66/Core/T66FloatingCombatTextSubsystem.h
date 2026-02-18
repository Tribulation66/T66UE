// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66FloatingCombatTextSubsystem.generated.h"

/**
 * Displays floating combat text: damage numbers on the sides of enemies and
 * status/event labels (Crit, DoT, etc.) above the head with different fonts.
 * One subsystem for both; call from TakeDamageFromHero / TakeDamageFromHeroHit.
 */
UCLASS()
class T66_API UT66FloatingCombatTextSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Well-known event types for status labels (use these when applying damage with a modifier). */
	static const FName EventType_Crit;
	static const FName EventType_DoT;

	/** Hero-only status/event types (shown above hero head). */
	static const FName EventType_Burn;
	static const FName EventType_Chill;
	static const FName EventType_Curse;
	static const FName EventType_LevelUp;

	/** Combat proc / event types (Taunt/Confusion on enemy; Invis/Dodge/LifeSteal/Close/Long on hero; Reflect/Crush/Counter on enemy). */
	static const FName EventType_Taunt;
	static const FName EventType_Confusion;
	static const FName EventType_Invisibility;
	static const FName EventType_Dodge;
	static const FName EventType_LifeSteal;
	static const FName EventType_Reflect;
	static const FName EventType_Crush;
	static const FName EventType_CounterAttack;
	static const FName EventType_CloseRange;
	static const FName EventType_LongRange;

	/** Internal: used for hero damage-taken styling (red number, no status label). */
	static const FName EventType_DamageTaken;

	/** Show a damage number at the target (offset to the side). Optionally show a status label above head if EventType is set. */
	UFUNCTION(BlueprintCallable, Category = "FloatingCombatText")
	void ShowDamageNumber(AActor* Target, int32 Amount, FName EventType = NAME_None);

	/** Show a damage-taken number at the target (red, same layout as damage dealt). Use when the hero takes damage. */
	UFUNCTION(BlueprintCallable, Category = "FloatingCombatText")
	void ShowDamageTaken(AActor* Target, int32 Amount);

	/** Show only a status/event label above the target (e.g. "CRIT!", "DoT"). Uses event-specific font/color. */
	UFUNCTION(BlueprintCallable, Category = "FloatingCombatText")
	void ShowStatusEvent(AActor* Target, FName EventType);

private:
	/** Horizontal offset from target center for damage numbers (so they appear on the side). */
	static constexpr float DamageNumberOffsetSide = 80.f;
	/** Height above target for damage number and status label. */
	static constexpr float OffsetAboveHead = 180.f;
	/** Lifetime of each floating text actor (seconds) before it destroys itself. */
	static constexpr float TextLifetimeSeconds = 1.2f;
};
