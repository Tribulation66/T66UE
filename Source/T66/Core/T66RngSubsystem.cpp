// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RngSubsystem.h"
#include "Core/T66GameInstance.h"

namespace
{
	static float Clamp01(float V) { return FMath::Clamp(V, 0.f, 1.f); }
}

void UT66RngSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Start in a deterministic-but-unique state. BeginRun() should be called when a run actually begins.
	RunSeed = static_cast<int32>(FPlatformTime::Cycles());
	RunStream.Initialize(RunSeed);
	LuckStat = 1;
	Luck01 = 0.f;
}

void UT66RngSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

const UT66RngTuningConfig* UT66RngSubsystem::GetTuning() const
{
	return GetDefault<UT66RngTuningConfig>();
}

void UT66RngSubsystem::RecomputeLuck01()
{
	const UT66RngTuningConfig* T = GetTuning();

	const float PerPoint = T ? FMath::Max(0.f, T->LuckPerPoint) : 0.03f;
	const float Max01 = T ? Clamp01(T->LuckMax01) : 0.95f;
	const float Points = static_cast<float>(FMath::Max(0, LuckStat - 1));
	Luck01 = Clamp01((Max01 > 0.f) ? (1.f - FMath::Exp(-PerPoint * Points)) : 0.f);
	Luck01 = FMath::Min(Luck01, Max01);
}

void UT66RngSubsystem::BeginRun(int32 InLuckStat)
{
	LuckStat = FMath::Max(1, InLuckStat);

	// Seed is stable per run but unique across launches; include selection hashes so different runs differ.
	int32 Seed = static_cast<int32>(FPlatformTime::Cycles());
	if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		Seed ^= static_cast<int32>(GetTypeHash(T66GI->SelectedHeroID));
		Seed ^= static_cast<int32>(GetTypeHash(T66GI->SelectedCompanionID));
		Seed ^= static_cast<int32>(T66GI->SelectedDifficulty) * 7919;
		Seed ^= static_cast<int32>(T66GI->SelectedPartySize) * 1543;
	}
	Seed ^= (LuckStat * 1337);

	RunSeed = Seed;
	RunStream.Initialize(RunSeed);

	RecomputeLuck01();
}

void UT66RngSubsystem::UpdateLuckStat(int32 InLuckStat)
{
	LuckStat = FMath::Max(1, InLuckStat);
	RecomputeLuck01();
}

float UT66RngSubsystem::BiasHigh01(float U01) const
{
	const UT66RngTuningConfig* T = GetTuning();
	const float Strength = T ? FMath::Max(0.f, T->RangeHighBiasStrength) : 1.75f;

	// Power transform: k=1 -> uniform. Higher k -> biased toward high values.
	const float K = 1.f + (Luck01 * Strength);
	const float U = Clamp01(U01);
	const float Biased = 1.f - FMath::Pow(1.f - U, K);
	return Clamp01(Biased);
}

float UT66RngSubsystem::BiasChance01(float BaseChance01) const
{
	const UT66RngTuningConfig* T = GetTuning();
	const float Strength = T ? FMath::Max(0.f, T->BernoulliBiasStrength) : 1.25f;

	const float P0 = Clamp01(BaseChance01);
	if (P0 <= 0.f) return 0.f;
	if (P0 >= 1.f) return 1.f;

	// Push probability upward with luck, without exceeding 1.
	// multiplier = 1 (no luck) -> unchanged; higher -> increases chance with diminishing returns.
	const float Mult = 1.f + (Luck01 * Strength);
	const float P = 1.f - FMath::Pow(1.f - P0, Mult);
	return Clamp01(P);
}

ET66Rarity UT66RngSubsystem::RollRarityWeighted(const FT66RarityWeights& BaseWeights, FRandomStream& Stream) const
{
	const UT66RngTuningConfig* T = GetTuning();
	const float Strength = T ? FMath::Max(0.f, T->RarityBiasStrength) : 1.35f;

	// Exponential tilt toward higher tiers as luck increases.
	const float Beta = Luck01 * Strength;
	const float W0 = FMath::Max(0.f, BaseWeights.Black) * FMath::Exp(Beta * 0.f);
	const float W1 = FMath::Max(0.f, BaseWeights.Red) * FMath::Exp(Beta * 1.f);
	const float W2 = FMath::Max(0.f, BaseWeights.Yellow) * FMath::Exp(Beta * 2.f);
	const float W3 = FMath::Max(0.f, BaseWeights.White) * FMath::Exp(Beta * 3.f);

	const float Sum = W0 + W1 + W2 + W3;
	if (Sum <= KINDA_SMALL_NUMBER)
	{
		return ET66Rarity::Black;
	}

	const float R = Stream.FRandRange(0.f, Sum);
	float Acc = 0.f;
	Acc += W0; if (R <= Acc) return ET66Rarity::Black;
	Acc += W1; if (R <= Acc) return ET66Rarity::Red;
	Acc += W2; if (R <= Acc) return ET66Rarity::Yellow;
	return ET66Rarity::White;
}

int32 UT66RngSubsystem::RollIntRangeBiased(const FT66IntRange& Range, FRandomStream& Stream) const
{
	const int32 A = FMath::Min(Range.Min, Range.Max);
	const int32 B = FMath::Max(Range.Min, Range.Max);
	if (B <= A) return A;

	const float U = BiasHigh01(Stream.GetFraction());
	const float Span = static_cast<float>(B - A);
	const int32 V = A + FMath::Clamp(FMath::FloorToInt(U * (Span + 1e-3f) + 0.00001f), 0, (B - A));
	return FMath::Clamp(V, A, B);
}

float UT66RngSubsystem::RollFloatRangeBiased(const FT66FloatRange& Range, FRandomStream& Stream) const
{
	const float A = FMath::Min(Range.Min, Range.Max);
	const float B = FMath::Max(Range.Min, Range.Max);
	if (B <= A) return A;

	const float U = BiasHigh01(Stream.GetFraction());
	return A + (B - A) * U;
}

