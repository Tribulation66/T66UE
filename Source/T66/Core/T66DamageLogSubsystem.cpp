// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66DamageLogSubsystem.h"

const FName UT66DamageLogSubsystem::SourceID_AutoAttack(TEXT("AutoAttack"));
const FName UT66DamageLogSubsystem::SourceID_Ultimate(TEXT("Ultimate"));

void UT66DamageLogSubsystem::RecordDamageDealt(FName SourceID, int32 Amount)
{
	if (Amount <= 0 || SourceID.IsNone()) return;
	int32& Total = DamageBySource.FindOrAdd(SourceID);
	Total = FMath::Clamp(Total + Amount, 0, 2000000000);
}

TArray<FDamageLogEntry> UT66DamageLogSubsystem::GetDamageBySourceSorted() const
{
	TArray<FDamageLogEntry> Out;
	Out.Reserve(DamageBySource.Num());
	for (const auto& Pair : DamageBySource)
	{
		FDamageLogEntry E;
		E.SourceID = Pair.Key;
		E.TotalDamage = Pair.Value;
		Out.Add(E);
	}
	Out.Sort([](const FDamageLogEntry& A, const FDamageLogEntry& B) { return A.TotalDamage > B.TotalDamage; });
	return Out;
}

void UT66DamageLogSubsystem::ResetForNewRun()
{
	DamageBySource.Empty();
}
