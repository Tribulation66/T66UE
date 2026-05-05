// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66MiniDataTypes.h"
#include "Save/T66MiniProfileSaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/StrongObjectPtr.h"
#include "T66MiniSaveSubsystem.generated.h"

class UT66MiniDataSubsystem;
class UT66MiniFrontendStateSubsystem;
class UT66MiniRunSaveGame;

UCLASS()
class T66MINI_API UT66MiniSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	enum { MaxSlots = 6 };
	static constexpr int32 CompanionUnityTierGoodStages = 5;
	static constexpr int32 CompanionUnityTierMediumStages = 10;
	static constexpr int32 CompanionUnityTierHyperStages = 20;

	virtual void Deinitialize() override;

	TArray<FT66MiniSaveSlotSummary> BuildRunSlotSummaries(const UT66MiniDataSubsystem* DataSubsystem) const;
	UT66MiniRunSaveGame* LoadRunFromSlot(int32 SlotIndex) const;
	bool SaveRunToSlot(int32 SlotIndex, UT66MiniRunSaveGame* RunSave) const;
	bool DeleteRunFromSlot(int32 SlotIndex) const;
	UT66MiniRunSaveGame* CreateSeededRunSave(const UT66MiniFrontendStateSubsystem* FrontendState) const;
	UT66MiniProfileSaveGame* LoadOrCreateProfileSave(const UT66MiniDataSubsystem* DataSubsystem) const;
	bool SaveProfile(UT66MiniProfileSaveGame* ProfileSave) const;
	bool RecordRunSummary(const FT66MiniRunSummary& Summary, const UT66MiniDataSubsystem* DataSubsystem) const;
	bool IsCompanionUnlocked(FName CompanionID, const UT66MiniDataSubsystem* DataSubsystem) const;
	FName GetFirstUnlockedCompanionID(const UT66MiniDataSubsystem* DataSubsystem) const;
	int32 GetUnlockedCompanionCount(const UT66MiniDataSubsystem* DataSubsystem) const;
	int32 GetTotalStagesCleared(const UT66MiniDataSubsystem* DataSubsystem) const;
	int32 GetCompanionUnityStagesCleared(FName CompanionID, const UT66MiniDataSubsystem* DataSubsystem) const;
	float GetCompanionUnityProgress01(FName CompanionID, const UT66MiniDataSubsystem* DataSubsystem) const;
	float GetCompanionHealingPerSecond(FName CompanionID, const UT66MiniDataSubsystem* DataSubsystem) const;
	bool RecordClearedMiniStage(FName CompanionID, const UT66MiniDataSubsystem* DataSubsystem, TArray<FName>* OutNewlyUnlocked = nullptr) const;

private:
	static FString MakeRunSlotName(int32 SlotIndex);
	static FString MakeProfileSlotName();
	static FString BuildUtcNowString();

	mutable TStrongObjectPtr<UT66MiniProfileSaveGame> CachedProfileSave;
	mutable bool bHasResolvedProfileSave = false;
};
