// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66AudioTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66AudioSubsystem.generated.h"

class USoundAttenuation;
class USoundBase;
class USoundConcurrency;
class UDataTable;

DECLARE_LOG_CATEGORY_EXTERN(LogT66Audio, Log, All);

/**
 * Central data-driven audio router for gameplay and Slate-triggered UI sounds.
 */
UCLASS()
class T66_API UT66AudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "T66|Audio")
	bool PlayEvent(FName EventID, UObject* WorldContextObject, FVector Location = FVector::ZeroVector, AActor* OwningActor = nullptr);

	bool PlayEventAtActor(FName EventID, AActor* Actor);
	bool IsEventConfigured(FName EventID) const;

	static bool PlayEventFromWorldContext(UObject* WorldContextObject, FName EventID, const FVector& Location = FVector::ZeroVector, AActor* OwningActor = nullptr);
	static bool PlayEventAtActorFromWorldContext(UObject* WorldContextObject, FName EventID, AActor* Actor);
	static bool PlayUIEventFromAnyWorld(FName EventID = FName(TEXT("UI.Click")));

private:
	void LoadAudioEvents();
	void PrimeConfiguredAssets();

	USoundBase* ResolveSound(const FSoftObjectPath& AssetPath, bool bWarnOnFailure);
	USoundAttenuation* ResolveAttenuation(const FSoftObjectPath& AssetPath);
	USoundConcurrency* ResolveConcurrency(const FSoftObjectPath& AssetPath);
	USoundBase* SelectSoundForRow(FName EventID, const FT66AudioEventRow& Row);

	float ResolveVolumeMultiplier(const FT66AudioEventRow& Row) const;
	float ResolvePitchMultiplier(const FT66AudioEventRow& Row) const;
	bool IsThrottled(FName EventID, const FT66AudioEventRow& Row);
	UWorld* ResolveWorld(UObject* WorldContextObject, AActor* OwningActor) const;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> AudioEventTable = nullptr;

	UPROPERTY(Transient)
	TMap<FName, FT66AudioEventRow> AudioEvents;

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<USoundBase>> CachedSounds;

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<USoundAttenuation>> CachedAttenuations;

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<USoundConcurrency>> CachedConcurrencies;

	TMap<FName, double> LastPlayTimeByEvent;
	TSet<FString> WarnedMissingSoundPaths;
};
