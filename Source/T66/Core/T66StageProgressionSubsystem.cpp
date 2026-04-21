// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66StageProgressionSubsystem.h"

#include "Core/T66GameInstance.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RunStateSubsystem.h"

namespace
{
	static bool AreSnapshotsEqual(const FT66StageProgressionSnapshot& A, const FT66StageProgressionSnapshot& B)
	{
		return A.SelectedDifficulty == B.SelectedDifficulty
			&& A.GlobalStage == B.GlobalStage
			&& A.DifficultyStartStage == B.DifficultyStartStage
			&& A.DifficultyEndStage == B.DifficultyEndStage
			&& A.LocalStageIndex == B.LocalStageIndex
			&& A.LocalStageCount == B.LocalStageCount
			&& FMath::IsNearlyEqual(A.DifficultyScalar, B.DifficultyScalar, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(A.FinaleScalar, B.FinaleScalar, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(A.EnemyStatScalar, B.EnemyStatScalar, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(A.InitialPopulationScalar, B.InitialPopulationScalar, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(A.RuntimeTrickleCountScalar, B.RuntimeTrickleCountScalar, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(A.RuntimeTrickleIntervalScalar, B.RuntimeTrickleIntervalScalar, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(A.TrapDamageScalar, B.TrapDamageScalar, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(A.TrapSpeedScalar, B.TrapSpeedScalar, KINDA_SMALL_NUMBER)
			&& A.ColorSaturation.Equals(B.ColorSaturation);
	}
}

void UT66StageProgressionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CachedTuning.LoadFromConfig();
	if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->StageChanged.AddDynamic(this, &UT66StageProgressionSubsystem::HandleStageChanged);
		RunState->DifficultyChanged.AddDynamic(this, &UT66StageProgressionSubsystem::HandleDifficultyChanged);
	}

	RefreshSnapshot(false);
}

void UT66StageProgressionSubsystem::Deinitialize()
{
	if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->StageChanged.RemoveDynamic(this, &UT66StageProgressionSubsystem::HandleStageChanged);
		RunState->DifficultyChanged.RemoveDynamic(this, &UT66StageProgressionSubsystem::HandleDifficultyChanged);
	}

	Super::Deinitialize();
}

void UT66StageProgressionSubsystem::RefreshSnapshot(const bool bBroadcastIfChanged)
{
	const FT66StageProgressionSnapshot NewSnapshot = BuildSnapshot();
	const bool bChanged = !AreSnapshotsEqual(CachedSnapshot, NewSnapshot);
	CachedSnapshot = NewSnapshot;

	if (bBroadcastIfChanged && bChanged)
	{
		StageProgressionChanged.Broadcast();
	}
}

void UT66StageProgressionSubsystem::HandleStageChanged()
{
	RefreshSnapshot(true);
}

void UT66StageProgressionSubsystem::HandleDifficultyChanged()
{
	RefreshSnapshot(true);
}

FT66StageProgressionSnapshot UT66StageProgressionSubsystem::BuildSnapshot() const
{
	FT66StageProgressionSnapshot Snapshot;

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66GameInstance* T66GameInstance = Cast<UT66GameInstance>(GameInstance);
	const UT66RunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const UT66PlayerExperienceSubSystem* PlayerExperience = GameInstance ? GameInstance->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;

	Snapshot.SelectedDifficulty = T66GameInstance ? T66GameInstance->SelectedDifficulty : ET66Difficulty::Easy;
	Snapshot.GlobalStage = RunState ? RunState->GetCurrentStage() : 1;
	Snapshot.DifficultyStartStage = PlayerExperience
		? PlayerExperience->GetDifficultyStartStage(Snapshot.SelectedDifficulty)
		: 1;
	Snapshot.DifficultyEndStage = PlayerExperience
		? PlayerExperience->GetDifficultyEndStage(Snapshot.SelectedDifficulty)
		: 4;
	Snapshot.DifficultyStartStage = FMath::Clamp(Snapshot.DifficultyStartStage, 1, 23);
	Snapshot.DifficultyEndStage = FMath::Clamp(Snapshot.DifficultyEndStage, Snapshot.DifficultyStartStage, 23);
	Snapshot.LocalStageCount = FMath::Max(1, Snapshot.DifficultyEndStage - Snapshot.DifficultyStartStage + 1);
	Snapshot.LocalStageIndex = FMath::Clamp(Snapshot.GlobalStage - Snapshot.DifficultyStartStage + 1, 1, Snapshot.LocalStageCount);
	Snapshot.DifficultyScalar = RunState ? RunState->GetDifficultyScalar() : 1.0f;
	Snapshot.FinaleScalar = RunState ? RunState->GetFinalSurvivalEnemyScalar() : 1.0f;

	const FT66StageProgressionStageTuning& StageTuning = CachedTuning.GetStageTuning(Snapshot.LocalStageIndex);
	Snapshot.EnemyStatScalar = FMath::Max(0.10f, StageTuning.EnemyStatScalar);
	Snapshot.InitialPopulationScalar = FMath::Max(0.10f, StageTuning.InitialPopulationScalar);
	Snapshot.RuntimeTrickleCountScalar = FMath::Max(0.10f, StageTuning.RuntimeTrickleCountScalar);
	Snapshot.RuntimeTrickleIntervalScalar = FMath::Max(0.10f, StageTuning.RuntimeTrickleIntervalScalar);
	Snapshot.TrapDamageScalar = FMath::Max(0.10f, StageTuning.TrapDamageScalar);
	Snapshot.TrapSpeedScalar = FMath::Max(0.10f, StageTuning.TrapSpeedScalar);
	Snapshot.ColorSaturation = StageTuning.ColorSaturation;
	return Snapshot;
}
