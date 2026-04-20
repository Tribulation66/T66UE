// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/T66RunIntegrityTypes.h"
#include "T66RunIntegritySubsystem.generated.h"

UCLASS()
class T66_API UT66RunIntegritySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void ResetActiveRunContext();
	void CaptureFreshRunBaseline();
	void RestoreActiveRunContext(const FT66RunIntegrityContext& SavedContext);
	void MarkRunModifierSelected(const FString& ModifierKind, const FName ModifierId);
	void MarkLoadedSnapshot();
	void FinalizeCurrentRun();
	bool IsCurrentEnvironmentConsistentWithBaseline() const;

	const FT66RunIntegrityContext& GetCurrentContext() const { return ActiveContext; }
	void CopyCurrentContextTo(FT66RunIntegrityContext& OutContext) const;

private:
	void MergeCurrentEnvironmentInto(FT66RunIntegrityContext& Context) const;
	static FString ComputeStableHash(const TArray<FString>& Parts);
	static FString ComputeCombinedHash(const FString& SteamAppId, int32 SteamBuildId, const FString& SteamBetaName, const FString& ManifestId, const FString& ManifestRootHash, const FString& ModuleListHash, const FString& MountedContentHash);
	static void AppendDirectoryFileStamps(const FString& Directory, const TArray<FString>& Patterns, TArray<FString>& OutEntries);
	static void AppendFileStamp(const FString& FilePath, TArray<FString>& OutEntries);
	static FString NormalizeForHash(const FString& Path);
	static void AddUniqueReason(FT66RunIntegrityContext& Context, const FString& Reason);

	FT66RunIntegrityContext ActiveContext;
};
