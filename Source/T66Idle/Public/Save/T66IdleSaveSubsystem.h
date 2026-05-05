// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Save/T66IdleProfileSaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/StrongObjectPtr.h"
#include "T66IdleSaveSubsystem.generated.h"

UCLASS()
class T66IDLE_API UT66IdleSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UT66IdleProfileSaveGame* LoadOrCreateProfileSave() const;
	bool SaveProfile(UT66IdleProfileSaveGame* ProfileSave) const;
	bool ApplyOfflineProgress(UT66IdleProfileSaveGame* ProfileSave, const class UT66IdleDataSubsystem* DataSubsystem, FTimespan MaxOfflineTime) const;

	static FString MakeProfileSlotName();
	static FString BuildUtcNowString();

private:
	mutable TStrongObjectPtr<UT66IdleProfileSaveGame> CachedProfileSave;
	mutable bool bHasResolvedProfileSave = false;
};
