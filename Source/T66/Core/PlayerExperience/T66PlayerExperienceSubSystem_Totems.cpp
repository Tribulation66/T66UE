#include "Core/T66PlayerExperienceSubSystem.h"

FT66TotemRules UT66PlayerExperienceSubSystem::GetDifficultyTotemRules(const ET66Difficulty Difficulty) const
{
	FT66TotemRules Rules = GetDifficultyTuning(Difficulty).TotemRules;
	Rules.UsesPerTotem = FMath::Max(1, Rules.UsesPerTotem);
	Rules.TotemsPerGameplayStage = FMath::Max(1, Rules.TotemsPerGameplayStage);
	Rules.GameplayStagesWithTotems = FMath::Max(1, Rules.GameplayStagesWithTotems);
	Rules.SkullColorBandSize = FMath::Max(1, Rules.SkullColorBandSize);
	return Rules;
}

int32 UT66PlayerExperienceSubSystem::GetDifficultyTotemUsesPerTotem(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTotemRules(Difficulty).UsesPerTotem;
}

int32 UT66PlayerExperienceSubSystem::GetDifficultySkullColorBandSize(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTotemRules(Difficulty).SkullColorBandSize;
}

bool UT66PlayerExperienceSubSystem::ShouldSpawnDifficultyTotemOnTowerFloor(
	const ET66Difficulty Difficulty,
	const bool bBossRushFinaleStage,
	const int32 FloorNumber,
	const int32 FirstGameplayFloorNumber,
	const int32 LastGameplayFloorNumber) const
{
	if (bBossRushFinaleStage)
	{
		return false;
	}

	const FT66TotemRules Rules = GetDifficultyTotemRules(Difficulty);
	if (Rules.UsesPerTotem <= 0)
	{
		return false;
	}

	return FloorNumber >= FirstGameplayFloorNumber && FloorNumber <= LastGameplayFloorNumber;
}
