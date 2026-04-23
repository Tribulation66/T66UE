// Copyright Tribulation 66. All Rights Reserved.

#include "Core/RunState/T66RunStateSubsystem_Private.h"

using namespace T66RunStatePrivate;

const TArray<FName>& UT66RunStateSubsystem::GetAllIdolIDs()
{
	return UT66IdolManagerSubsystem::GetAllIdolIDs();
}


FLinearColor UT66RunStateSubsystem::GetIdolColor(FName IdolID)
{
	return UT66IdolManagerSubsystem::GetIdolColor(IdolID);
}


void UT66RunStateSubsystem::SetDOTDamageApplier(TFunction<void(AActor*, int32, FName)> InApplier)
{
	DOTDamageApplier = MoveTemp(InApplier);
}


void UT66RunStateSubsystem::ApplyDOT(AActor* Target, float Duration, float TickInterval, float DamagePerTick, FName SourceIdolID)
{
	if (!Target || Duration <= 0.f || TickInterval <= 0.f || DamagePerTick <= 0.f) return;

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;

	FT66DotInstance Inst;
	Inst.RemainingDuration = Duration;
	Inst.TickInterval = FMath::Max(0.05f, TickInterval);
	Inst.DamagePerTick = DamagePerTick;
	Inst.NextTickTime = World->GetTimeSeconds() + Inst.TickInterval;
	Inst.SourceIdolID = SourceIdolID;

	FT66DotKey Key;
	Key.Target = TWeakObjectPtr<AActor>(Target);
	Key.SourceIdolID = SourceIdolID;
	ActiveDOTs.FindOrAdd(Key) = Inst;

	// Start timer if not already running.
	if (!DOTTimerHandle.IsValid())
	{
		World->GetTimerManager().SetTimer(DOTTimerHandle, this, &UT66RunStateSubsystem::TickDOT, DOTTickRateSeconds, true);
	}
}


void UT66RunStateSubsystem::TickDOT()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	const double Now = World ? World->GetTimeSeconds() : 0.0;

	TArray<FT66DotKey> ToRemove;
	for (auto& Pair : ActiveDOTs)
	{
		if (!Pair.Key.Target.IsValid())
		{
			ToRemove.Add(Pair.Key);
			continue;
		}
		FT66DotInstance& Inst = Pair.Value;
		Inst.RemainingDuration -= DOTTickRateSeconds;
		if (Inst.RemainingDuration <= 0.f)
		{
			ToRemove.Add(Pair.Key);
			continue;
		}
		if (Now >= Inst.NextTickTime)
		{
			const int32 Damage = FMath::Max(1, FMath::RoundToInt(Inst.DamagePerTick));
			if (DOTDamageApplier)
			{
				DOTDamageApplier(Pair.Key.Target.Get(), Damage, Inst.SourceIdolID);
			}
			Inst.NextTickTime = Now + Inst.TickInterval;
		}
	}
	for (const FT66DotKey& Key : ToRemove)
	{
		ActiveDOTs.Remove(Key);
	}
	if (ActiveDOTs.Num() == 0 && World && DOTTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(DOTTimerHandle);
		DOTTimerHandle.Invalidate();
	}
}


const TArray<FName>& UT66RunStateSubsystem::GetEquippedIdols() const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetEquippedIdols();
	}

	static const TArray<FName> Empty;
	return Empty;
}


const TArray<uint8>& UT66RunStateSubsystem::GetEquippedIdolTierValues() const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetEquippedIdolTierValues();
	}

	static const TArray<uint8> Empty;
	return Empty;
}


bool UT66RunStateSubsystem::EquipIdolInSlot(int32 SlotIndex, FName IdolID)
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->EquipIdolInSlot(SlotIndex, IdolID);
	}
	return false;
}


bool UT66RunStateSubsystem::EquipIdolFirstEmpty(FName IdolID)
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->EquipIdolFirstEmpty(IdolID);
	}
	return false;
}


int32 UT66RunStateSubsystem::GetEquippedIdolLevelInSlot(int32 SlotIndex) const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetEquippedIdolLevelInSlot(SlotIndex);
	}
	return 0;
}


ET66ItemRarity UT66RunStateSubsystem::IdolTierValueToRarity(int32 TierValue)
{
	return UT66IdolManagerSubsystem::IdolTierValueToRarity(TierValue);
}


int32 UT66RunStateSubsystem::IdolRarityToTierValue(ET66ItemRarity Rarity)
{
	return UT66IdolManagerSubsystem::IdolRarityToTierValue(Rarity);
}


ET66ItemRarity UT66RunStateSubsystem::GetEquippedIdolRarityInSlot(int32 SlotIndex) const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetEquippedIdolRarityInSlot(SlotIndex);
	}
	return ET66ItemRarity::Black;
}


bool UT66RunStateSubsystem::SelectIdolFromAltar(FName IdolID)
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->SelectIdolFromAltar(IdolID);
	}
	return false;
}


void UT66RunStateSubsystem::ClearEquippedIdols()
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->ClearEquippedIdols();
	}
}


void UT66RunStateSubsystem::EnsureIdolStock()
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->EnsureIdolStock();
	}
}


void UT66RunStateSubsystem::RerollIdolStock()
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->RerollIdolStock();
	}
}


const TArray<FName>& UT66RunStateSubsystem::GetIdolStockIDs() const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetIdolStockIDs();
	}

	static const TArray<FName> Empty;
	return Empty;
}


int32 UT66RunStateSubsystem::GetIdolStockTierValue(int32 SlotIndex) const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetIdolStockTierValue(SlotIndex);
	}
	return 0;
}


ET66ItemRarity UT66RunStateSubsystem::GetIdolStockRarityInSlot(int32 SlotIndex) const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->GetIdolStockRarityInSlot(SlotIndex);
	}
	return ET66ItemRarity::Black;
}


bool UT66RunStateSubsystem::SelectIdolFromStock(int32 SlotIndex)
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->SelectIdolFromStock(SlotIndex);
	}
	return false;
}


bool UT66RunStateSubsystem::IsIdolStockSlotSelected(int32 SlotIndex) const
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		return IdolManager->IsIdolStockSlotSelected(SlotIndex);
	}
	return false;
}


bool UT66RunStateSubsystem::HasActiveDOT(AActor* Target) const
{
	if (!Target) return false;
	const TWeakObjectPtr<AActor> Key(Target);
	for (const auto& Pair : ActiveDOTs)
	{
		if (Pair.Key.Target == Key && Pair.Value.RemainingDuration > 0.f)
		{
			return true;
		}
	}
	return false;
}


float UT66RunStateSubsystem::GetToxinStackingDamageMultiplier(AActor* Target) const
{
	if (PassiveType != ET66PassiveType::ToxinStacking || !Target) return 1.f;
	return HasActiveDOT(Target) ? 1.15f : 1.f;
}
