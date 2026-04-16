// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MiniRunStateSubsystem.generated.h"

class UT66MiniFrontendStateSubsystem;
class UT66MiniRunSaveGame;
class UT66MiniSaveSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnT66MiniActiveRunChanged);

UCLASS()
class T66MINI_API UT66MiniRunStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Mini")
	FOnT66MiniActiveRunChanged ActiveRunChanged;

	UFUNCTION(BlueprintCallable, Category = "Mini")
	bool BootstrapRunFromFrontend(UT66MiniFrontendStateSubsystem* FrontendState, UT66MiniSaveSubsystem* SaveSubsystem);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	bool BootstrapOnlineRunFromFrontend(UT66MiniFrontendStateSubsystem* FrontendState, UT66MiniSaveSubsystem* SaveSubsystem);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	bool BootstrapTransientRunFromFrontend(UT66MiniFrontendStateSubsystem* FrontendState, UT66MiniSaveSubsystem* SaveSubsystem);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	bool LoadRunFromSave(UT66MiniRunSaveGame* InRunSave);

	UFUNCTION(BlueprintCallable, Category = "Mini")
	void ResetActiveRun();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	UT66MiniRunSaveGame* GetActiveRun() const { return ActiveRun; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	bool HasActiveRun() const { return ActiveRun != nullptr; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	int32 GetActiveSaveSlot() const { return ActiveSaveSlot; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mini")
	FString GetLastBootstrapStatus() const { return LastBootstrapStatus; }

private:
	int32 ResolveTargetSaveSlot(const UT66MiniFrontendStateSubsystem* FrontendState, UT66MiniSaveSubsystem* SaveSubsystem) const;

	UPROPERTY()
	TObjectPtr<UT66MiniRunSaveGame> ActiveRun;

	UPROPERTY()
	int32 ActiveSaveSlot = INDEX_NONE;

	UPROPERTY()
	FString LastBootstrapStatus;
};
