// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66DamageLogSubsystem.h"

const FName UT66DamageLogSubsystem::SourceID_AutoAttack(TEXT("AutoAttack"));
const FName UT66DamageLogSubsystem::SourceID_Ultimate(TEXT("Ultimate"));
const FName UT66DamageLogSubsystem::SourceID_Environment(TEXT("Environment"));

namespace
{
	void T66AddDamageToMap(TMap<FName, int32>& DamageMap, const FName SourceID, const int32 Amount)
	{
		if (Amount <= 0 || SourceID.IsNone()) return;

		int32& Total = DamageMap.FindOrAdd(SourceID);
		Total = FMath::Clamp(Total + Amount, 0, 2000000000);
	}

	TArray<FDamageLogEntry> T66BuildDamageEntriesSorted(const TMap<FName, int32>& DamageMap)
	{
		TArray<FDamageLogEntry> Out;
		Out.Reserve(DamageMap.Num());
		for (const auto& Pair : DamageMap)
		{
			if (Pair.Value <= 0)
			{
				continue;
			}

			FDamageLogEntry E;
			E.SourceID = Pair.Key;
			E.TotalDamage = Pair.Value;
			Out.Add(E);
		}

		Out.Sort([](const FDamageLogEntry& A, const FDamageLogEntry& B)
		{
			if (A.TotalDamage != B.TotalDamage)
			{
				return A.TotalDamage > B.TotalDamage;
			}

			return A.SourceID.LexicalLess(B.SourceID);
		});
		return Out;
	}
}

void UT66DamageLogSubsystem::RecordDamageDealt(FName SourceID, int32 Amount)
{
	if (Amount <= 0 || SourceID.IsNone()) return;
	T66AddDamageToMap(DamageBySource, SourceID, Amount);

	const double NowSeconds = GetCurrentWorldTimeSeconds();
	TrimRecentDamageSamples(NowSeconds);

	FRecentDamageSample& Sample = RecentDamageSamples.AddDefaulted_GetRef();
	Sample.TimeSeconds = NowSeconds;
	Sample.Amount = Amount;
	RecentDamageInWindow = FMath::Clamp(RecentDamageInWindow + Amount, 0, 2000000000);
}

void UT66DamageLogSubsystem::RecordDamageReceived(FName SourceID, int32 Amount)
{
	if (SourceID.IsNone())
	{
		SourceID = SourceID_Environment;
	}

	T66AddDamageToMap(DamageReceivedBySource, SourceID, Amount);
}

TArray<FDamageLogEntry> UT66DamageLogSubsystem::GetDamageBySourceSorted() const
{
	return T66BuildDamageEntriesSorted(DamageBySource);
}

TArray<FDamageLogEntry> UT66DamageLogSubsystem::GetDamageReceivedBySourceSorted() const
{
	return T66BuildDamageEntriesSorted(DamageReceivedBySource);
}

float UT66DamageLogSubsystem::GetRollingDPS()
{
	const double NowSeconds = GetCurrentWorldTimeSeconds();
	TrimRecentDamageSamples(NowSeconds);
	return static_cast<float>(RecentDamageInWindow) / static_cast<float>(RollingDPSWindowSeconds);
}

void UT66DamageLogSubsystem::ResetForNewRun()
{
	DamageBySource.Empty();
	DamageReceivedBySource.Empty();
	RecentDamageSamples.Empty();
	RecentDamageSampleHeadIndex = 0;
	RecentDamageInWindow = 0;
}

double UT66DamageLogSubsystem::GetCurrentWorldTimeSeconds() const
{
	const UWorld* World = GetWorld();
	return World ? static_cast<double>(World->GetTimeSeconds()) : 0.0;
}

void UT66DamageLogSubsystem::TrimRecentDamageSamples(double NowSeconds)
{
	while (RecentDamageSamples.IsValidIndex(RecentDamageSampleHeadIndex))
	{
		const FRecentDamageSample& Sample = RecentDamageSamples[RecentDamageSampleHeadIndex];
		if ((NowSeconds - Sample.TimeSeconds) <= RollingDPSWindowSeconds)
		{
			break;
		}

		RecentDamageInWindow = FMath::Max(0, RecentDamageInWindow - Sample.Amount);
		++RecentDamageSampleHeadIndex;
	}

	if (RecentDamageSampleHeadIndex > 64 && RecentDamageSampleHeadIndex >= (RecentDamageSamples.Num() / 2))
	{
		RecentDamageSamples.RemoveAt(0, RecentDamageSampleHeadIndex, EAllowShrinking::No);
		RecentDamageSampleHeadIndex = 0;
	}
}
