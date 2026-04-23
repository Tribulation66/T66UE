#include "Core/T66PlayerExperienceSubSystem.h"

FT66SpawnBudget UT66PlayerExperienceSubSystem::BuildTowerSpawnBudget(
	const ET66Difficulty Difficulty,
	const float DifficultyScalar,
	const float InitialPopulationScalar,
	const float RuntimeTrickleCountScalar,
	const float RuntimeTrickleIntervalScalar,
	const float StageTimerSeconds) const
{
	FT66SpawnBudget Budget = GetDifficultyTuning(Difficulty).TowerSpawnBudgetBase;
	Budget.GameplayFloorsPerStage = FMath::Max(1, Budget.GameplayFloorsPerStage);

	const float SafeDifficultyScalar = FMath::Max(1.0f, DifficultyScalar);
	const float SafeInitialScalar = FMath::Max(0.10f, InitialPopulationScalar);
	const float SafeRuntimeCountScalar = FMath::Max(0.10f, RuntimeTrickleCountScalar);
	const float SafeRuntimeIntervalScalar = FMath::Max(0.10f, RuntimeTrickleIntervalScalar);

	Budget.InitialEnemiesPerGameplayFloor = FMath::Clamp(
		FMath::RoundToInt(static_cast<float>(FMath::Max(0, Budget.InitialEnemiesPerGameplayFloor)) * SafeInitialScalar * SafeDifficultyScalar),
		0,
		128);
	Budget.TotalInitialEnemiesPerStage = Budget.InitialEnemiesPerGameplayFloor * Budget.GameplayFloorsPerStage;

	Budget.RuntimeEnemiesPerWave = FMath::Clamp(
		FMath::RoundToInt(static_cast<float>(FMath::Max(1, Budget.RuntimeEnemiesPerWave)) * SafeRuntimeCountScalar * SafeDifficultyScalar),
		1,
		128);
	Budget.RuntimeMaxAliveEnemies = FMath::Clamp(
		FMath::RoundToInt(static_cast<float>(FMath::Max(1, Budget.RuntimeMaxAliveEnemies)) * SafeRuntimeCountScalar * SafeDifficultyScalar),
		Budget.RuntimeEnemiesPerWave,
		1024);
	Budget.RuntimeSpawnIntervalSeconds = FMath::Max(
		0.5f,
		(FMath::Max(0.10f, Budget.RuntimeSpawnIntervalSeconds) * SafeRuntimeIntervalScalar) / SafeDifficultyScalar);

	Budget.EstimatedRuntimeWavesPerStage = FMath::CeilToInt(FMath::Max(0.0f, StageTimerSeconds) / Budget.RuntimeSpawnIntervalSeconds);
	Budget.EstimatedRuntimeEnemiesPerStage = Budget.EstimatedRuntimeWavesPerStage * Budget.RuntimeEnemiesPerWave;
	Budget.EstimatedTotalEnemiesPerStage = Budget.TotalInitialEnemiesPerStage + Budget.EstimatedRuntimeEnemiesPerStage;
	return Budget;
}
