#include "Core/T66PlayerExperienceSubSystem.h"

namespace
{
	static int32 T66ResolveScaledScore(const int32 BasePointValue, const float DifficultyScalar, const float BaseMultiplier, const bool bScaleWithDifficultyScalar)
	{
		const float AppliedScalar = bScaleWithDifficultyScalar ? FMath::Max(1.0f, DifficultyScalar) : 1.0f;
		const float RawScore = static_cast<float>(FMath::Max(0, BasePointValue)) * FMath::Max(0.0f, BaseMultiplier) * AppliedScalar;
		return FMath::Max(0, FMath::RoundToInt(RawScore));
	}
}

int32 UT66PlayerExperienceSubSystem::ResolveEnemyScoreAtSpawn(
	const ET66Difficulty Difficulty,
	const int32 BasePointValue,
	const float DifficultyScalar) const
{
	const FT66EnemyScoreTuning& ScoreTuning = GetDifficultyTuning(Difficulty).EnemyScoreTuning;
	return T66ResolveScaledScore(BasePointValue, DifficultyScalar, ScoreTuning.BaseMultiplier, ScoreTuning.bScaleWithDifficultyScalar);
}

int32 UT66PlayerExperienceSubSystem::ResolveBossScore(
	const ET66Difficulty Difficulty,
	const int32 BasePointValue,
	const float DifficultyScalar) const
{
	const FT66BossScoreTuning& ScoreTuning = GetDifficultyTuning(Difficulty).BossScoreTuning;
	return T66ResolveScaledScore(BasePointValue, DifficultyScalar, ScoreTuning.BaseMultiplier, ScoreTuning.bScaleWithDifficultyScalar);
}
