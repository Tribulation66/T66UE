// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "T66MiniGameState.generated.h"

class UT66MiniRunSaveGame;

UCLASS()
class T66MINI_API AT66MiniGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Mini")
	void ApplyRunSave(const UT66MiniRunSaveGame* RunSave);

	UPROPERTY(BlueprintReadOnly, Category = "Mini")
	FName HeroID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Mini")
	FName CompanionID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Mini")
	FName DifficultyID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Mini")
	int32 WaveIndex = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Mini")
	float WaveSecondsRemaining = 180.f;
};
