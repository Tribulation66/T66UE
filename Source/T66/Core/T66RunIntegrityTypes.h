// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66RunIntegrityTypes.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66RunIntegrityContext
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	FString Verdict = TEXT("unknown");

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	TArray<FString> Reasons;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	FString SteamAppId;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	int32 SteamBuildId = 0;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	FString SteamBetaName;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	FString ManifestId;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	FString ManifestRootHash;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	FString ModuleListHash;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	FString MountedContentHash;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	FString BaselineHash;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	FString FinalHash;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	FString RunModifierKind;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	FString RunModifierId;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	bool bBaselineCaptured = false;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	bool bFinalized = false;

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Integrity")
	bool bLoadedFromSnapshot = false;

	bool IsPristine() const
	{
		return Verdict.Equals(TEXT("pristine"), ESearchCase::IgnoreCase);
	}

	bool ShouldAllowRankedSubmission() const
	{
		return IsPristine();
	}

	void AddReason(const FString& Reason)
	{
		if (!Reason.IsEmpty() && !Reasons.Contains(Reason))
		{
			Reasons.Add(Reason);
		}
	}

	void PromoteToVerdict(const FString& InVerdict, const FString& Reason = FString())
	{
		if (!InVerdict.IsEmpty() && (Verdict.IsEmpty() || Verdict.Equals(TEXT("unknown"), ESearchCase::IgnoreCase) || IsPristine()))
		{
			Verdict = InVerdict;
		}

		AddReason(Reason);
	}

	void MarkPristineIfUnset()
	{
		if (Verdict.IsEmpty() || Verdict.Equals(TEXT("unknown"), ESearchCase::IgnoreCase))
		{
			Verdict = TEXT("pristine");
		}
	}
};
