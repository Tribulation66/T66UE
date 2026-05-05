// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/StrongObjectPtr.h"
#include "T66TDSaveSubsystem.generated.h"

class UT66TDDataSubsystem;
class UT66TDFrontendStateSubsystem;
class UT66TDProfileSaveGame;
class UT66TDRunSaveGame;
struct FT66TDStageDefinition;

UCLASS()
class T66TD_API UT66TDSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UT66TDRunSaveGame* LoadRun() const;
	bool SaveRun(UT66TDRunSaveGame* RunSave) const;
	UT66TDRunSaveGame* CreateSeededRunSave(const UT66TDFrontendStateSubsystem* FrontendState, const UT66TDDataSubsystem* DataSubsystem) const;

	UT66TDProfileSaveGame* LoadOrCreateProfileSave() const;
	bool SaveProfile(UT66TDProfileSaveGame* ProfileSave) const;
	bool RecordStageResult(const FT66TDStageDefinition* Stage, bool bVictory, int32 Score, int32 GoldEarned, int32 MaterialsEarned) const;

	static FString MakeRunSlotName();
	static FString MakeProfileSlotName();

private:
	static FString BuildUtcNowString();

	mutable TStrongObjectPtr<UT66TDRunSaveGame> CachedRunSave;
	mutable TStrongObjectPtr<UT66TDProfileSaveGame> CachedProfileSave;
	mutable bool bHasResolvedProfileSave = false;
};
