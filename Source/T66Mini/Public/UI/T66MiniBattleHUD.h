// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "InputCoreTypes.h"
#include "T66MiniBattleHUD.generated.h"

UCLASS()
class T66MINI_API AT66MiniBattleHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	virtual void BeginPlay() override;

private:
	void RefreshCachedUiState();

	UPROPERTY()
	TObjectPtr<class UTexture2D> HeartTexture;

	UPROPERTY()
	TObjectPtr<class UTexture2D> UltimateIconTexture;

	UPROPERTY()
	TObjectPtr<class UTexture2D> PassiveIconTexture;

	UPROPERTY()
	TObjectPtr<class UTexture2D> QuickReviveIconTexture;

	UPROPERTY()
	TObjectPtr<class UTexture2D> MouseLeftIconTexture;

	UPROPERTY()
	TObjectPtr<class UTexture2D> MouseRightIconTexture;

	UPROPERTY()
	TObjectPtr<class UTexture2D> InteractHandIconTexture;

	FKey CachedPauseKey;
	FKey CachedInteractKey;
	FKey CachedUltimateKey;
	FString CachedPauseKeyLabel;
	FString CachedInteractKeyLabel;
	FString CachedUltimateKeyLabel;
};
