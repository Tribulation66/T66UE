// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66TDFrontendStateSubsystem.generated.h"

UCLASS()
class T66TD_API UT66TDFrontendStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void ResetRunSetup();
	void BeginNewRun();
	void SelectDifficulty(FName DifficultyID);
	void SelectMap(FName MapID);

	FName GetSelectedDifficultyID() const { return SelectedDifficultyID; }
	FName GetSelectedMapID() const { return SelectedMapID; }
	bool HasSelectedDifficulty() const { return SelectedDifficultyID != NAME_None; }
	bool HasSelectedMap() const { return SelectedMapID != NAME_None; }

private:
	UPROPERTY()
	FName SelectedDifficultyID = NAME_None;

	UPROPERTY()
	FName SelectedMapID = NAME_None;
};
