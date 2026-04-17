// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66CombatShared.h"

#include "Gameplay/T66ArthurUltimateSword.h"
#include "Gameplay/T66HeroOneAttackVFX.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "CollisionQueryParams.h"
#include "HAL/IConsoleManager.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	static TAutoConsoleVariable<int32> CVarT66VFXHeroOneVerbose(
		TEXT("T66.VFX.Hero1Verbose"),
		0,
		TEXT("Emit detailed logs for the Hero_1 pierce VFX path."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroPierceVerbose(
		TEXT("T66.VFX.HeroPierceVerbose"),
		0,
		TEXT("Emit detailed logs for the generic hero pierce VFX path."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolPierceVerbose(
		TEXT("T66.VFX.IdolPierceVerbose"),
		0,
		TEXT("Emit detailed logs for idol pierce VFX requests."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroAOEVerbose(
		TEXT("T66.VFX.HeroAOEVerbose"),
		0,
		TEXT("Emit detailed logs for hero AOE VFX requests."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroBounceVerbose(
		TEXT("T66.VFX.HeroBounceVerbose"),
		0,
		TEXT("Emit detailed logs for hero bounce VFX requests."));

	static TAutoConsoleVariable<int32> CVarT66VFXHeroDOTVerbose(
		TEXT("T66.VFX.HeroDOTVerbose"),
		0,
		TEXT("Emit detailed logs for hero DOT VFX requests."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolAOEVerbose(
		TEXT("T66.VFX.IdolAOEVerbose"),
		0,
		TEXT("Emit detailed logs for idol AOE VFX requests."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolBounceVerbose(
		TEXT("T66.VFX.IdolBounceVerbose"),
		0,
		TEXT("Emit detailed logs for idol bounce VFX requests."));

	static TAutoConsoleVariable<int32> CVarT66VFXIdolDOTVerbose(
		TEXT("T66.VFX.IdolDOTVerbose"),
		0,
		TEXT("Emit detailed logs for idol DOT VFX requests."));

	int32 GHeroOneStage1RequestSerial = 0;
	int32 GHeroPierceStage2RequestSerial = 0;
	int32 GIdolPierceStage3RequestSerial = 0;
	int32 GHeroAOEStage4RequestSerial = 0;
	int32 GHeroBounceStage5RequestSerial = 0;
	int32 GHeroDOTStage6RequestSerial = 0;
	int32 GIdolAOEStage7RequestSerial = 0;
	int32 GIdolBounceStage8RequestSerial = 0;
	int32 GIdolDOTStage9RequestSerial = 0;

	UNiagaraSystem* LoadNiagaraSystemCached(const TCHAR* AssetPath)
	{
		static TMap<FString, TWeakObjectPtr<UNiagaraSystem>> Cache;
		if (!AssetPath || !*AssetPath)
		{
			return nullptr;
		}

		const FString Key(AssetPath);
		if (const TWeakObjectPtr<UNiagaraSystem>* Found = Cache.Find(Key))
		{
			if (Found->IsValid())
			{
				return Found->Get();
			}
		}

		UNiagaraSystem* Loaded = LoadObject<UNiagaraSystem>(nullptr, AssetPath);
		Cache.Add(Key, Loaded);
		return Loaded;
	}

	UClass* LoadEffectBlueprintClassCached(const TCHAR* ClassPath)
	{
		static TMap<FString, TWeakObjectPtr<UClass>> Cache;
		if (!ClassPath || !*ClassPath)
		{
			return nullptr;
		}

		const FString Key(ClassPath);
		if (const TWeakObjectPtr<UClass>* Found = Cache.Find(Key))
		{
			if (Found->IsValid())
			{
				return Found->Get();
			}
		}

		UClass* Loaded = StaticLoadClass(AActor::StaticClass(), nullptr, ClassPath);
		Cache.Add(Key, Loaded);
		return Loaded;
	}

	void ScheduleNiagaraDeactivate(UWorld* World, UNiagaraComponent* NiagaraComponent, const float DelaySeconds)
	{
		if (!World || !NiagaraComponent || DelaySeconds <= 0.f)
		{
			return;
		}

		TWeakObjectPtr<UNiagaraComponent> WeakComponent = NiagaraComponent;
		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda([WeakComponent]()
			{
				if (WeakComponent.IsValid())
				{
					WeakComponent->Deactivate();
					WeakComponent->SetAutoDestroy(true);
				}
			}),
			DelaySeconds,
			false);
	}

	UNiagaraComponent* SpawnImportedNiagaraAtLocation(
		UWorld* World,
		const TCHAR* AssetPath,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& Scale,
		const float ActiveDurationSeconds = 0.f)
	{
		UNiagaraSystem* System = LoadNiagaraSystemCached(AssetPath);
		if (!World || !System)
		{
			return nullptr;
		}

		UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			System,
			Location,
			Rotation,
			Scale,
			false,
			true,
			ENCPoolMethod::None,
			true);
		if (NiagaraComponent)
		{
			NiagaraComponent->SetAutoDestroy(ActiveDurationSeconds <= 0.f);
			if (ActiveDurationSeconds > 0.f)
			{
				ScheduleNiagaraDeactivate(World, NiagaraComponent, ActiveDurationSeconds);
			}
		}
		return NiagaraComponent;
	}

	UNiagaraComponent* SpawnImportedNiagaraAttached(
		UWorld* World,
		const TCHAR* AssetPath,
		USceneComponent* AttachComponent,
		const FVector& RelativeLocation,
		const FRotator& RelativeRotation,
		const FVector& Scale,
		const float ActiveDurationSeconds)
	{
		UNiagaraSystem* System = LoadNiagaraSystemCached(AssetPath);
		if (!World || !System || !AttachComponent)
		{
			return nullptr;
		}

		UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			System,
			AttachComponent,
			NAME_None,
			RelativeLocation,
			RelativeRotation,
			Scale,
			EAttachLocation::KeepRelativeOffset,
			false,
			ENCPoolMethod::None,
			true,
			true);
		if (NiagaraComponent)
		{
			NiagaraComponent->SetAutoDestroy(ActiveDurationSeconds <= 0.f);
			if (ActiveDurationSeconds > 0.f)
			{
				ScheduleNiagaraDeactivate(World, NiagaraComponent, ActiveDurationSeconds);
			}
		}
		return NiagaraComponent;
	}

	bool SpawnImportedEffectBlueprint(
		UWorld* World,
		const TCHAR* ClassPath,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& Scale,
		const float LifeSpanSeconds)
	{
		UClass* EffectClass = LoadEffectBlueprintClassCached(ClassPath);
		if (!World || !EffectClass)
		{
			return false;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AActor* EffectActor = World->SpawnActor<AActor>(EffectClass, Location, Rotation, SpawnParams))
		{
			EffectActor->SetActorScale3D(Scale);
			if (LifeSpanSeconds > 0.f)
			{
				EffectActor->SetLifeSpan(LifeSpanSeconds);
			}
			return true;
		}
		return false;
	}

	void SpawnImportedEffectAlongLine(
		UWorld* World,
		const TCHAR* AssetPath,
		const FVector& Start,
		const FVector& End,
		const float VisualScale,
		const float QuantityMultiplier)
	{
		UNiagaraSystem* System = LoadNiagaraSystemCached(AssetPath);
		if (!World || !System)
		{
			return;
		}

		const FVector Delta = End - Start;
		const float Distance = Delta.Size();
		if (Distance <= KINDA_SMALL_NUMBER)
		{
			SpawnImportedNiagaraAtLocation(World, AssetPath, Start, FRotator::ZeroRotator, FVector(VisualScale));
			return;
		}

		const FRotator Rotation = Delta.Rotation();
		const int32 SpawnCount = FMath::Clamp(FMath::RoundToInt((Distance / 150.f) * FMath::Max(0.5f, QuantityMultiplier)), 1, 18);
		for (int32 Index = 0; Index < SpawnCount; ++Index)
		{
			const float T = (SpawnCount > 1) ? static_cast<float>(Index) / static_cast<float>(SpawnCount - 1) : 0.5f;
			const FVector SpawnLocation = FMath::Lerp(Start, End, T);
			SpawnImportedNiagaraAtLocation(World, AssetPath, SpawnLocation, Rotation, FVector(VisualScale));
		}
	}

	void SpawnImportedEffectAlongChain(
		UWorld* World,
		const TCHAR* AssetPath,
		const TArray<FVector>& Points,
		const float VisualScale,
		const float QuantityMultiplier)
	{
		if (!World || Points.Num() < 2)
		{
			return;
		}

		for (int32 Index = 0; Index < Points.Num() - 1; ++Index)
		{
			SpawnImportedEffectAlongLine(World, AssetPath, Points[Index], Points[Index + 1], VisualScale, QuantityMultiplier);
		}
	}

	const TCHAR* GetIdolNiagaraEffectPath(const FName& IdolID)
	{
		if (IdolID == FName(TEXT("Idol_Curse")))     return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Portal.P_Cosmic_Portal");
		if (IdolID == FName(TEXT("Idol_Lava")))      return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire");
		if (IdolID == FName(TEXT("Idol_Poison")))    return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Poison_02.P_Poison_02");
		if (IdolID == FName(TEXT("Idol_Bleed")))     return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Liquid_Hit_03.P_Liquid_Hit_03");
		if (IdolID == FName(TEXT("Idol_Electric")))  return TEXT("/Game/Stylized_VFX_StPack/Particles/P_Electric_Projectile_02.P_Electric_Projectile_02");
		if (IdolID == FName(TEXT("Idol_Ice")))       return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Ice_Projectile_02.P_Ice_Projectile_02");
		if (IdolID == FName(TEXT("Idol_Shadow")))    return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Projectile_02.P_Cosmic_Projectile_02");
		if (IdolID == FName(TEXT("Idol_Star")))      return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Projectile_03.P_Cosmic_Projectile_03");
		if (IdolID == FName(TEXT("Idol_Earth")))     return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Dirt_Spikes_02.P_Dirt_Spikes_02");
		if (IdolID == FName(TEXT("Idol_Water")))     return TEXT("/Game/Stylized_VFX_StPack/Particles/P_Splash_02.P_Splash_02");
		if (IdolID == FName(TEXT("Idol_BlackHole"))) return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Portal.P_Cosmic_Portal");
		if (IdolID == FName(TEXT("Idol_Light")))     return TEXT("/Game/Stylized_VFX_StPack/Particles/P_Laser_02.P_Laser_02");
		if (IdolID == FName(TEXT("Idol_Steel")))     return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_01.P_Weapon_01");
		if (IdolID == FName(TEXT("Idol_Wood")))      return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Web_Projectile_01.P_Web_Projectile_01");
		if (IdolID == FName(TEXT("Idol_Bone")))      return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_02.P_Weapon_02");
		return nullptr;
	}

	const TCHAR* GetIdolBlueprintEffectClassPath(const FName& IdolID)
	{
		if (IdolID == FName(TEXT("Idol_Storm")))
		{
			return TEXT("/Game/Stylized_VFX_StPack/Blueprints/BP_Storm.BP_Storm_C");
		}
		return nullptr;
	}

	UNiagaraSystem* FindPixelVFXSystem()
	{
		static TObjectPtr<UNiagaraSystem> CachedPixelSystem = nullptr;
		static TObjectPtr<UNiagaraSystem> CachedFallbackSystem = nullptr;
		if (!CachedPixelSystem)
		{
			CachedPixelSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"));
		}
		if (!CachedPixelSystem && !CachedFallbackSystem)
		{
			CachedFallbackSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"));
		}
		return CachedPixelSystem ? CachedPixelSystem.Get() : CachedFallbackSystem.Get();
	}

	FVector4 T66MakeBloodTint(bool bBrightCore)
	{
		if (bBrightCore)
		{
			return FVector4(
				FMath::FRandRange(0.80f, 1.00f),
				FMath::FRandRange(0.01f, 0.05f),
				FMath::FRandRange(0.01f, 0.04f),
				1.f);
		}

		return FVector4(
			FMath::FRandRange(0.35f, 0.72f),
			FMath::FRandRange(0.00f, 0.03f),
			FMath::FRandRange(0.00f, 0.03f),
			1.f);
	}

	void T66ApplyPixelVFXParams(UNiagaraComponent* NiagaraComponent, const FVector4& Tint, const FVector2D& SpriteSize)
	{
		if (!NiagaraComponent)
		{
			return;
		}

		const FLinearColor TintColor(Tint.X, Tint.Y, Tint.Z, Tint.W);
		NiagaraComponent->SetVariableLinearColor(FName(TEXT("User.Tint")), TintColor);
		NiagaraComponent->SetVariableLinearColor(FName(TEXT("User.Color")), TintColor);
		NiagaraComponent->SetVariableVec2(FName(TEXT("User.SpriteSize")), SpriteSize);
	}

	UNiagaraComponent* T66SpawnBudgetedPixel(
		UWorld* World,
		UNiagaraSystem* VFX,
		const FVector& Location,
		const FVector4& Tint,
		const FVector2D& SpriteSize,
		ET66PixelVFXPriority Priority,
		const FRotator& Rotation = FRotator::ZeroRotator,
		bool bAutoDestroy = true)
	{
		if (!World || !VFX)
		{
			return nullptr;
		}

		if (UT66PixelVFXSubsystem* PixelVFX = World->GetSubsystem<UT66PixelVFXSubsystem>())
		{
			return PixelVFX->SpawnPixelAtLocation(
				Location,
				FLinearColor(Tint.X, Tint.Y, Tint.Z, Tint.W),
				SpriteSize,
				Priority,
				Rotation,
				FVector(1.f),
				VFX,
				bAutoDestroy);
		}

		const ENCPoolMethod PoolingMethod = bAutoDestroy ? ENCPoolMethod::AutoRelease : ENCPoolMethod::None;

		UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			VFX,
			Location,
			Rotation,
			FVector(1.f),
			bAutoDestroy,
			true,
			PoolingMethod);
		T66ApplyPixelVFXParams(NiagaraComponent, Tint, SpriteSize);
		return NiagaraComponent;
	}

	void T66SpawnBloodSpray(
		UWorld* World,
		UNiagaraSystem* VFX,
		const FVector& Location,
		int32 PixelCount,
		float BurstRadius,
		float CoreRadiusScale)
	{
		if (!World || !VFX)
		{
			return;
		}

		const int32 TotalPixels = FMath::Max(PixelCount, 24);
		const int32 CoreCount = FMath::Max(8, FMath::RoundToInt(static_cast<float>(TotalPixels) * 0.25f));
		const int32 JetCount = FMath::Clamp(FMath::RoundToInt(static_cast<float>(TotalPixels) / 8.f), 4, 10);
		const int32 SprayCount = FMath::Max(12, TotalPixels - CoreCount);
		const float CoreRadius = BurstRadius * CoreRadiusScale;

		for (int32 i = 0; i < CoreCount; ++i)
		{
			const FVector Offset = FMath::VRand() * FMath::FRandRange(0.f, CoreRadius);
			T66SpawnBudgetedPixel(
				World,
				VFX,
				Location + Offset,
				T66MakeBloodTint(true),
				FVector2D(FMath::FRandRange(4.0f, 6.0f), FMath::FRandRange(4.0f, 6.0f)),
				ET66PixelVFXPriority::High);
		}

		int32 Remaining = SprayCount;
		for (int32 JetIndex = 0; JetIndex < JetCount; ++JetIndex)
		{
			const int32 JetsLeft = JetCount - JetIndex;
			const int32 JetPixels = FMath::Max(2, Remaining / FMath::Max(1, JetsLeft));
			Remaining -= JetPixels;

			FVector Dir(
				FMath::FRandRange(-1.0f, 1.0f),
				FMath::FRandRange(-1.0f, 1.0f),
				FMath::FRandRange(-0.25f, 0.45f));
			Dir = Dir.GetSafeNormal();
			if (Dir.IsNearlyZero())
			{
				Dir = FVector::ForwardVector;
			}

			FVector Right = FVector::CrossProduct(Dir, FVector::UpVector).GetSafeNormal();
			if (Right.IsNearlyZero())
			{
				Right = FVector::RightVector;
			}
			const FVector Upish = FVector::CrossProduct(Right, Dir).GetSafeNormal();
			const float JetLength = FMath::FRandRange(BurstRadius * 0.30f, BurstRadius * 0.95f);

			for (int32 PixelIndex = 0; PixelIndex < JetPixels; ++PixelIndex)
			{
				const float T = FMath::Clamp(
					(static_cast<float>(PixelIndex) + FMath::FRandRange(0.15f, 0.95f)) / static_cast<float>(JetPixels),
					0.f,
					1.f);
				const float Along = JetLength * FMath::Pow(T, 0.72f);
				const float Jitter = FMath::Lerp(BurstRadius * 0.02f, BurstRadius * 0.10f, T);
				const FVector Offset =
					Dir * Along +
					Right * FMath::FRandRange(-Jitter, Jitter) +
					Upish * FMath::FRandRange(-Jitter * 0.35f, Jitter * 0.35f);

				const bool bCoreStreak = T < 0.25f;
				T66SpawnBudgetedPixel(
					World,
					VFX,
					Location + Offset,
					T66MakeBloodTint(bCoreStreak),
					bCoreStreak
						? FVector2D(FMath::FRandRange(3.5f, 5.0f), FMath::FRandRange(3.5f, 5.0f))
						: FVector2D(FMath::FRandRange(2.0f, 3.8f), FMath::FRandRange(2.0f, 3.8f)),
					ET66PixelVFXPriority::High);
			}
		}
	}

	bool TrySpawnHeroPierceVariantPixels(
		UWorld* World,
		UNiagaraSystem* VFX,
		const FVector& Start,
		const FVector& End,
		const FVector4& ColorVec,
		const FName HeroID)
	{
		const FVector Dir = (End - Start).GetSafeNormal();
		const float Length = FVector::Dist(Start, End);

		if (HeroID == FName(TEXT("Hero_5")))
		{
			constexpr int32 N = 48;
			for (int32 i = 0; i < N; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(N - 1);
				T66SpawnBudgetedPixel(World, VFX, FMath::Lerp(Start, End, T), ColorVec, FVector2D(1.5f, 1.5f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
			}
			return true;
		}

		if (HeroID == FName(TEXT("Hero_8")))
		{
			constexpr int32 N = 16;
			constexpr float ConeDeg = 45.f;
			for (int32 i = 0; i < N; ++i)
			{
				const float T = (N > 1) ? (static_cast<float>(i) / static_cast<float>(N - 1)) : 0.5f;
				const float Angle = -ConeDeg * 0.5f + ConeDeg * T;
				const FVector ShotDir = Dir.RotateAngleAxis(Angle, FVector::UpVector);
				const float Dist = FMath::FRandRange(Length * 0.2f, Length * 0.6f);
				T66SpawnBudgetedPixel(World, VFX, Start + ShotDir * Dist, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
			}
			return true;
		}

		if (HeroID == FName(TEXT("Hero_11")))
		{
			const FVector Right = FVector::CrossProduct(FVector::UpVector, Dir).GetSafeNormal() * 30.f;
			constexpr int32 N = 8;
			for (int32 Line = 0; Line < 2; ++Line)
			{
				const FVector Offset = (Line == 0) ? Right : -Right;
				for (int32 i = 0; i < N; ++i)
				{
					const float T = static_cast<float>(i) / static_cast<float>(N - 1);
					T66SpawnBudgetedPixel(World, VFX, FMath::Lerp(Start + Offset, End + Offset, T), ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
				}
			}
			return true;
		}

		return false;
	}

	bool TrySpawnHeroAOEVariantPixels(
		UWorld* World,
		UNiagaraSystem* VFX,
		const FVector& Location,
		const float Radius,
		const FVector4& ColorVec,
		const FName HeroID)
	{
		if (HeroID == FName(TEXT("Hero_3")))
		{
			constexpr int32 N = 28;
			constexpr float ArcDeg = 180.f;
			for (int32 i = 0; i < N; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(N - 1);
				const float Angle = FMath::DegreesToRadians(-ArcDeg * 0.5f + ArcDeg * T);
				const FVector Offset(FMath::Cos(Angle) * Radius * 0.4f, FMath::Sin(Angle) * Radius * 0.4f, 0.f);
				T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(2.5f, 2.5f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
			}
			return true;
		}

		if (HeroID == FName(TEXT("Hero_4")))
		{
			constexpr int32 N = 16;
			constexpr float ArcDeg = 90.f;
			for (int32 i = 0; i < N; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(N - 1);
				const float Angle = FMath::DegreesToRadians(-ArcDeg * 0.5f + ArcDeg * T);
				const float R = Radius * 0.25f * FMath::FRandRange(0.5f, 1.f);
				const FVector Offset(FMath::Cos(Angle) * R, FMath::Sin(Angle) * R, 0.f);
				T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(3.0f, 3.0f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
			}
			return true;
		}

		if (HeroID == FName(TEXT("Hero_10")))
		{
			constexpr int32 N = 32;
			for (int32 i = 0; i < N; ++i)
			{
				const float Angle = (2.f * PI * static_cast<float>(i)) / static_cast<float>(N);
				const FVector Offset(FMath::Cos(Angle) * Radius * 0.35f, FMath::Sin(Angle) * Radius * 0.35f, 0.f);
				T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
			}
			return true;
		}

		if (HeroID == FName(TEXT("Hero_15")))
		{
			constexpr int32 N = 12;
			for (int32 i = 0; i < N; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(N - 1);
				const float InnerAngle = FMath::DegreesToRadians(-60.f + 120.f * T);
				const FVector InnerOff(FMath::Cos(InnerAngle) * Radius * 0.25f, FMath::Sin(InnerAngle) * Radius * 0.25f, 0.f);
				T66SpawnBudgetedPixel(World, VFX, Location + InnerOff, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);

				const float OuterAngle = FMath::DegreesToRadians(-75.f + 150.f * T);
				const FVector OuterOff(FMath::Cos(OuterAngle) * Radius * 0.4f, FMath::Sin(OuterAngle) * Radius * 0.4f, 0.f);
				T66SpawnBudgetedPixel(World, VFX, Location + OuterOff, FVector4(0.95f, 0.85f, 0.1f, 1.f), FVector2D(2.5f, 2.5f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
			}
			return true;
		}

		return false;
	}

	bool TrySpawnHeroBounceVariantPixels(
		UWorld* World,
		UNiagaraSystem* VFX,
		const TArray<FVector>& ChainPositions,
		const FVector4& ColorVec,
		const FName HeroID)
	{
		if (HeroID == FName(TEXT("Hero_6")))
		{
			for (int32 i = 0; i < ChainPositions.Num() - 1; ++i)
			{
				const FVector A = ChainPositions[i];
				const FVector B = ChainPositions[i + 1];
				const float Dist = FVector::Dist(A, B);
				const int32 N = FMath::Max(4, FMath::RoundToInt(Dist / 15.f));
				for (int32 j = 0; j < N; ++j)
				{
					const float T = static_cast<float>(j) / static_cast<float>(N - 1);
					T66SpawnBudgetedPixel(World, VFX, FMath::Lerp(A, B, T), ColorVec, FVector2D(1.5f, 1.5f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
				}
			}
			return true;
		}

		if (HeroID == FName(TEXT("Hero_7")))
		{
			for (int32 i = 0; i < ChainPositions.Num() - 1; ++i)
			{
				const FVector A = ChainPositions[i];
				const FVector B = ChainPositions[i + 1];
				const float Dist = FVector::Dist(A, B);
				const int32 N = FMath::Max(3, FMath::RoundToInt(Dist / 60.f));
				for (int32 j = 0; j < N; ++j)
				{
					const float T = static_cast<float>(j) / static_cast<float>(N - 1);
					T66SpawnBudgetedPixel(World, VFX, FMath::Lerp(A, B, T), ColorVec, FVector2D(3.5f, 3.5f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
				}
			}
			return true;
		}

		if (HeroID == FName(TEXT("Hero_12")))
		{
			for (int32 i = 0; i < ChainPositions.Num() - 1; ++i)
			{
				const FVector A = ChainPositions[i];
				const FVector B = ChainPositions[i + 1];
				const float Dist = FVector::Dist(A, B);
				const int32 N = FMath::Max(6, FMath::RoundToInt(Dist / 15.f));
				for (int32 j = 0; j < N; ++j)
				{
					const float T = static_cast<float>(j) / static_cast<float>(N - 1);
					const FVector Jitter(FMath::FRandRange(-20.f, 20.f), FMath::FRandRange(-20.f, 20.f), FMath::FRandRange(-10.f, 10.f));
					T66SpawnBudgetedPixel(World, VFX, FMath::Lerp(A, B, T) + Jitter, ColorVec, FVector2D(1.5f, 1.5f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
				}
			}
			return true;
		}

		if (HeroID == FName(TEXT("Hero_9")))
		{
			for (int32 i = 0; i < ChainPositions.Num() - 1; ++i)
			{
				const FVector A = ChainPositions[i];
				const FVector B = ChainPositions[i + 1];
				const float Dist = FVector::Dist(A, B);
				const int32 N = FMath::Max(4, FMath::RoundToInt(Dist / 25.f));
				for (int32 j = 0; j < N; ++j)
				{
					const float T = static_cast<float>(j) / static_cast<float>(N - 1);
					T66SpawnBudgetedPixel(World, VFX, FMath::Lerp(A, B, T), ColorVec, FVector2D(1.5f, 1.5f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
				}
			}
			return true;
		}

		return false;
	}

	bool TrySpawnHeroDOTVariantPixels(
		UWorld* World,
		UNiagaraSystem* VFX,
		const FVector& Location,
		const float Radius,
		const FVector4& ColorVec,
		const FName HeroID)
	{
		if (HeroID == FName(TEXT("Hero_2")))
		{
			constexpr int32 N = 12;
			for (int32 i = 0; i < N; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(N);
				const float Angle = T * 4.f * PI;
				const float R = Radius * 0.3f * T;
				const FVector Offset(FMath::Cos(Angle) * R, FMath::Sin(Angle) * R, T * 30.f);
				T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
			}
			return true;
		}

		if (HeroID == FName(TEXT("Hero_13")))
		{
			constexpr int32 N = 8;
			for (int32 i = 0; i < N; ++i)
			{
				const FVector Offset(FMath::FRandRange(-15.f, 15.f), FMath::FRandRange(-15.f, 15.f), -static_cast<float>(i) * 12.f);
				T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
			}
			return true;
		}

		if (HeroID == FName(TEXT("Hero_14")))
		{
			constexpr int32 N = 16;
			for (int32 i = 0; i < N; ++i)
			{
				const float Angle = (2.f * PI * static_cast<float>(i)) / static_cast<float>(N);
				const FVector Offset(FMath::Cos(Angle) * Radius * 0.4f, FMath::Sin(Angle) * Radius * 0.4f, 0.f);
				T66SpawnBudgetedPixel(World, VFX, Location + Offset, FVector4(0.5f, 0.8f, 1.f, 1.f), FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
			}
			return true;
		}

		if (HeroID == FName(TEXT("Hero_16")))
		{
			constexpr int32 N = 6;
			for (int32 i = 0; i < N; ++i)
			{
				const FVector Offset(FMath::FRandRange(-20.f, 20.f), FMath::FRandRange(-20.f, 20.f), FMath::FRandRange(-10.f, 10.f));
				T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(2.5f, 2.5f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
			}
			return true;
		}

		return false;
	}
}

void UT66CombatComponent::WarmupVFXSystems()
{
	CachedSlashVFXNiagara = SlashVFXNiagara.Get();
	if (!CachedSlashVFXNiagara)
	{
		CachedSlashVFXNiagara = SlashVFXNiagara.LoadSynchronous();
	}

	CachedPixelVFXNiagara = PixelVFXNiagara.Get();
	if (!CachedPixelVFXNiagara)
	{
		CachedPixelVFXNiagara = PixelVFXNiagara.LoadSynchronous();
	}
	if (CachedPixelVFXNiagara)
	{
		UE_LOG(LogT66Combat, Log, TEXT("[VFX] Pixel particle system loaded: NS_PixelParticle"));
	}
	else
	{
		UE_LOG(LogT66Combat, Warning, TEXT("[VFX] NS_PixelParticle not found; falling back to VFX_Attack1. Run CreatePixelParticleNiagara.py + configure in editor."));
	}
}

UNiagaraSystem* UT66CombatComponent::GetActiveVFXSystem() const
{
	return CachedPixelVFXNiagara ? CachedPixelVFXNiagara : CachedSlashVFXNiagara;
}

void UT66CombatComponent::SpawnDeathBurstAtLocation(UWorld* World, const FVector& Location, int32 NumParticles, float BurstRadius)
{
	if (!World)
	{
		return;
	}

	static TWeakObjectPtr<UNiagaraSystem> CachedBloodBurstVFX;
	UNiagaraSystem* BloodBurstVFX = CachedBloodBurstVFX.Get();
	if (!BloodBurstVFX)
	{
		BloodBurstVFX = FindPixelVFXSystem();
		CachedBloodBurstVFX = BloodBurstVFX;
	}
	if (!BloodBurstVFX)
	{
		return;
	}

	T66SpawnBloodSpray(World, BloodBurstVFX, Location, FMath::Max(NumParticles * 3, 42), BurstRadius, 0.08f);
}

void UT66CombatComponent::SpawnDeathVFX(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!VFX)
	{
		return;
	}

	T66SpawnBloodSpray(World, VFX, Location, 84, 150.f, 0.09f);
}

void UT66CombatComponent::SpawnSlashVFX(const FVector& Location, float Radius, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World || !VFX)
	{
		return;
	}

	constexpr int32 NumParticles = 24;
	constexpr float ArcAngleDeg = 120.f;
	constexpr float StartAngleDeg = -ArcAngleDeg * 0.5f;
	constexpr float SpreadScale = 0.35f;
	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	for (int32 i = 0; i < NumParticles; ++i)
	{
		const float T = (NumParticles > 1) ? (static_cast<float>(i) / static_cast<float>(NumParticles - 1)) : 0.5f;
		const float AngleRad = FMath::DegreesToRadians(StartAngleDeg + ArcAngleDeg * T);
		const FVector Offset(FMath::Cos(AngleRad) * Radius * SpreadScale, FMath::Sin(AngleRad) * Radius * SpreadScale, 0.f);
		const FVector SpawnLoc = Location + Offset;
		T66SpawnBudgetedPixel(World, VFX, SpawnLoc, ColorVec, FVector2D(2.5f, 2.5f), ET66PixelVFXPriority::Medium);
	}
}

void UT66CombatComponent::SpawnPierceVFX(const FVector& Start, const FVector& End, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World)
	{
		UE_LOG(LogT66Combat, Warning, TEXT("[ATTACK VFX][Legacy] Pierce spawn skipped: no world."));
		return;
	}
	if (!VFX)
	{
		UE_LOG(
			LogT66Combat,
			Warning,
			TEXT("[ATTACK VFX][Legacy] Pierce spawn skipped: no active Niagara system. Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f)"),
			Start.X, Start.Y, Start.Z,
			End.X, End.Y, End.Z);
		return;
	}

	constexpr int32 NumParticles = 40;
	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	for (int32 i = 0; i < NumParticles; ++i)
	{
		const float T = (NumParticles > 1) ? (static_cast<float>(i) / static_cast<float>(NumParticles - 1)) : 0.5f;
		const FVector SpawnLoc = FMath::Lerp(Start, End, T);
		T66SpawnBudgetedPixel(World, VFX, SpawnLoc, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium);
	}
}

void UT66CombatComponent::SpawnHeroOnePierceVFX(const FVector& Start, const FVector& End, const FVector& ImpactLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const int32 RequestId = ++GHeroOneStage1RequestSerial;
	const float TraceLength2D = FVector::Dist2D(Start, End);
	const bool bVerbose = CVarT66VFXHeroOneVerbose.GetValueOnGameThread() != 0;
	if (bVerbose)
	{
		UE_LOG(
			LogT66Combat,
			Log,
			TEXT("[ATTACK VFX][Stage1] Hero_1 Pierce request Req=%d Owner=%s Time=%.3f Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f) Impact=(%.1f,%.1f,%.1f) Trace2D=%.1f"),
			RequestId,
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			Start.X, Start.Y, Start.Z,
			End.X, End.Y, End.Z,
			ImpactLocation.X, ImpactLocation.Y, ImpactLocation.Z,
			TraceLength2D);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66HeroOneAttackVFX* Effect = World->SpawnActor<AT66HeroOneAttackVFX>(
		AT66HeroOneAttackVFX::StaticClass(),
		Start,
		FRotator::ZeroRotator,
		SpawnParams);
	if (Effect)
	{
		if (bVerbose)
		{
			UE_LOG(
				LogT66Combat,
				Log,
				TEXT("[ATTACK VFX][Stage1] Hero_1 Example actor spawned Req=%d Actor=%s Owner=%s"),
				RequestId,
				*Effect->GetName(),
				GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
		}
		Effect->InitEffect(Start, End, ImpactLocation, FLinearColor(1.f, 0.97f, 0.88f, 1.f), RequestId, FName(TEXT("Hero_1")));
		return;
	}

	UE_LOG(LogT66Combat, Warning, TEXT("[ATTACK VFX][Stage1] Hero_1 pierce actor spawn failed Req=%d."), RequestId);
}

void UT66CombatComponent::SpawnArthurUltimateSwordVFX(const FVector& Start, const FVector& End)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AT66ArthurUltimateSword* Sword = World->SpawnActor<AT66ArthurUltimateSword>(
		AT66ArthurUltimateSword::StaticClass(),
		Start,
		FRotator::ZeroRotator,
		SpawnParams))
	{
		Sword->InitSwordFlight(Start, End);
	}

	SpawnPierceVFX(Start, End, FLinearColor(1.f, 0.82f, 0.24f, 1.f));
}

void UT66CombatComponent::SpawnBounceVFX(const TArray<FVector>& ChainPositions, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World || !VFX || ChainPositions.Num() < 2)
	{
		return;
	}

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	for (int32 i = 0; i < ChainPositions.Num() - 1; ++i)
	{
		const FVector ChainStart = ChainPositions[i];
		const FVector ChainEnd = ChainPositions[i + 1];
		const FVector Dir = (ChainEnd - ChainStart).GetSafeNormal();
		if (Dir.IsNearlyZero())
		{
			continue;
		}
		const FRotator Rot = Dir.Rotation();
		const float Dist = FVector::Dist(ChainStart, ChainEnd);

		const int32 Num = FMath::Max(8, FMath::RoundToInt(Dist / 20.f));
		for (int32 j = 0; j < Num; ++j)
		{
			const float T = (Num > 1) ? (static_cast<float>(j) / static_cast<float>(Num - 1)) : 0.5f;
			T66SpawnBudgetedPixel(
				World,
				VFX,
				FMath::Lerp(ChainStart, ChainEnd, T),
				ColorVec,
				FVector2D(2.0f, 2.0f),
				ET66PixelVFXPriority::Medium,
				Rot);
		}

		T66SpawnBudgetedPixel(World, VFX, ChainStart, ColorVec, FVector2D(3.5f, 3.5f), ET66PixelVFXPriority::High);
	}
}

void UT66CombatComponent::SpawnDOTVFX(const FVector& Location, float Duration, float Radius, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World || !VFX)
	{
		return;
	}

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);
	UNiagaraComponent* NiagaraComponent = T66SpawnBudgetedPixel(
		World,
		VFX,
		Location,
		ColorVec,
		FVector2D(2.0f, 2.0f),
		ET66PixelVFXPriority::Medium,
		FRotator::ZeroRotator,
		true);
	if (NiagaraComponent)
	{
		if (AActor* OwnerActor = NiagaraComponent->GetOwner())
		{
			OwnerActor->SetLifeSpan(FMath::Max(0.1f, Duration));
		}
	}
}

void UT66CombatComponent::SpawnIdolPierceVFX(const FName& IdolID, const ET66ItemRarity Rarity, const FVector& Start, const FVector& End, const FVector& ImpactLocation, const float StartDelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const FLinearColor IdolColor = UT66IdolManagerSubsystem::GetIdolColor(IdolID);
	const int32 RequestId = ++GIdolPierceStage3RequestSerial;
	if (CVarT66VFXIdolPierceVerbose.GetValueOnGameThread() != 0)
	{
		UE_LOG(
			LogT66Combat,
			Log,
			TEXT("[ATTACK VFX][IdolPierce] Req=%d Idol=%s Rarity=%s Owner=%s Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f) Impact=(%.1f,%.1f,%.1f) Delay=%.3f"),
			RequestId,
			*IdolID.ToString(),
			T66CombatShared::GetItemRarityName(Rarity),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			Start.X, Start.Y, Start.Z,
			End.X, End.Y, End.Z,
			ImpactLocation.X, ImpactLocation.Y, ImpactLocation.Z,
			StartDelaySeconds);
	}

	const float VisualScale = FMath::Clamp(
		T66CombatShared::GetIdolRarityVisualScale(Rarity) * FMath::Max(0.1f, ProjectileScaleMultiplier) * T66CombatShared::GetCategorySubScaleMultiplier(CachedRunState, ET66AttackCategory::Pierce),
		0.35f,
		8.0f);
	const float Quantity = T66CombatShared::GetIdolRarityVisualQuantity(Rarity) * T66CombatShared::GetCategorySubScaleMultiplier(CachedRunState, ET66AttackCategory::Pierce);
	if (const TCHAR* AssetPath = GetIdolNiagaraEffectPath(IdolID))
	{
		SpawnImportedEffectAlongLine(World, AssetPath, Start + FVector(0.f, 0.f, 18.f), End + FVector(0.f, 0.f, 18.f), VisualScale, Quantity);
		return;
	}

	SpawnPierceVFX(Start, End, IdolColor);
}

void UT66CombatComponent::SpawnIdolAOEVFX(const FName& IdolID, const ET66ItemRarity Rarity, const FVector& Location, const float Radius, const float StartDelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const FLinearColor IdolColor = UT66IdolManagerSubsystem::GetIdolColor(IdolID);
	const int32 RequestId = ++GIdolAOEStage7RequestSerial;
	const bool bVerbose = CVarT66VFXIdolAOEVerbose.GetValueOnGameThread() != 0;

	if (bVerbose)
	{
		UE_LOG(
			LogT66Combat,
			Log,
			TEXT("[ATTACK VFX][Stage7] Idol AOE request Req=%d Idol=%s Rarity=%s Owner=%s Time=%.3f Radius=%.1f Delay=%.3f Center=(%.1f,%.1f,%.1f) IdolColor=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*IdolID.ToString(),
			T66CombatShared::GetItemRarityName(Rarity),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			Radius,
			StartDelaySeconds,
			Location.X, Location.Y, Location.Z,
			IdolColor.R, IdolColor.G, IdolColor.B, IdolColor.A);
	}

	const float RadiusVisualFactor = FMath::Clamp(Radius / 260.f, 0.55f, 4.5f);
	const float VisualScale = FMath::Clamp(
		T66CombatShared::GetIdolRarityVisualScale(Rarity) * RadiusVisualFactor * FMath::Max(0.1f, ProjectileScaleMultiplier) * T66CombatShared::GetCategorySubScaleMultiplier(CachedRunState, ET66AttackCategory::AOE),
		0.35f,
		10.0f);
	if (const TCHAR* BlueprintClassPath = GetIdolBlueprintEffectClassPath(IdolID))
	{
		if (SpawnImportedEffectBlueprint(World, BlueprintClassPath, Location + FVector(0.f, 0.f, 6.f), FRotator::ZeroRotator, FVector(VisualScale), FMath::Max(2.0f, 2.5f * VisualScale)))
		{
			return;
		}
	}
	if (const TCHAR* AssetPath = GetIdolNiagaraEffectPath(IdolID))
	{
		if (SpawnImportedNiagaraAtLocation(World, AssetPath, Location + FVector(0.f, 0.f, 8.f), FRotator::ZeroRotator, FVector(VisualScale)))
		{
			return;
		}
	}

	SpawnSlashVFX(Location, Radius, IdolColor);
}

void UT66CombatComponent::SpawnIdolBounceVFX(const FName& IdolID, const ET66ItemRarity Rarity, const TArray<FVector>& ChainPositions, const float StartDelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World || ChainPositions.Num() < 2)
	{
		return;
	}
	const FLinearColor IdolColor = UT66IdolManagerSubsystem::GetIdolColor(IdolID);
	const int32 RequestId = ++GIdolBounceStage8RequestSerial;
	const bool bVerbose = CVarT66VFXIdolBounceVerbose.GetValueOnGameThread() != 0;

	if (bVerbose)
	{
		UE_LOG(
			LogT66Combat,
			Log,
			TEXT("[ATTACK VFX][Stage8] Idol Bounce request Req=%d Idol=%s Rarity=%s Owner=%s Time=%.3f Links=%d Delay=%.3f Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f) IdolColor=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*IdolID.ToString(),
			T66CombatShared::GetItemRarityName(Rarity),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			ChainPositions.Num() - 1,
			StartDelaySeconds,
			ChainPositions[0].X, ChainPositions[0].Y, ChainPositions[0].Z,
			ChainPositions.Last().X, ChainPositions.Last().Y, ChainPositions.Last().Z,
			IdolColor.R, IdolColor.G, IdolColor.B, IdolColor.A);
	}

	const float VisualScale = FMath::Clamp(
		T66CombatShared::GetIdolRarityVisualScale(Rarity) * FMath::Max(0.1f, ProjectileScaleMultiplier) * T66CombatShared::GetCategorySubScaleMultiplier(CachedRunState, ET66AttackCategory::Bounce),
		0.35f,
		6.0f);
	const float Quantity = T66CombatShared::GetIdolRarityVisualQuantity(Rarity) * T66CombatShared::GetCategorySubScaleMultiplier(CachedRunState, ET66AttackCategory::Bounce);
	if (const TCHAR* AssetPath = GetIdolNiagaraEffectPath(IdolID))
	{
		TArray<FVector> ElevatedPositions;
		ElevatedPositions.Reserve(ChainPositions.Num());
		for (const FVector& Pos : ChainPositions)
		{
			ElevatedPositions.Add(Pos + FVector(0.f, 0.f, 24.f));
		}
		SpawnImportedEffectAlongChain(World, AssetPath, ElevatedPositions, VisualScale, Quantity);
		return;
	}

	TArray<FVector> ElevatedPositions;
	ElevatedPositions.Reserve(ChainPositions.Num());
	for (const FVector& Pos : ChainPositions)
	{
		ElevatedPositions.Add(Pos + FVector(0.f, 0.f, 24.f));
	}
	SpawnBounceVFX(ElevatedPositions, IdolColor);
}

void UT66CombatComponent::SpawnIdolDOTVFX(const FName& IdolID, const ET66ItemRarity Rarity, AActor* FollowTarget, const FVector& Location, const float Duration, const float Radius, const float StartDelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const FLinearColor IdolColor = UT66IdolManagerSubsystem::GetIdolColor(IdolID);
	const int32 RequestId = ++GIdolDOTStage9RequestSerial;
	const bool bVerbose = CVarT66VFXIdolDOTVerbose.GetValueOnGameThread() != 0;

	if (bVerbose)
	{
		UE_LOG(
			LogT66Combat,
			Log,
			TEXT("[ATTACK VFX][Stage9] Idol DOT request Req=%d Idol=%s Rarity=%s Owner=%s Time=%.3f Follow=%d Duration=%.2f Radius=%.1f Delay=%.3f Location=(%.1f,%.1f,%.1f) IdolColor=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*IdolID.ToString(),
			T66CombatShared::GetItemRarityName(Rarity),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			FollowTarget ? 1 : 0,
			Duration,
			Radius,
			StartDelaySeconds,
			Location.X, Location.Y, Location.Z,
			IdolColor.R, IdolColor.G, IdolColor.B, IdolColor.A);
	}

	const float DurationVisualFactor = FMath::Clamp(Duration / 3.0f, 0.75f, 4.5f);
	const float VisualScale = FMath::Clamp(
		T66CombatShared::GetIdolRarityVisualScale(Rarity) * DurationVisualFactor * FMath::Max(0.1f, ProjectileScaleMultiplier) * T66CombatShared::GetCategorySubScaleMultiplier(CachedRunState, ET66AttackCategory::DOT),
		0.25f,
		5.0f);
	if (const TCHAR* AssetPath = GetIdolNiagaraEffectPath(IdolID))
	{
		if (FollowTarget)
		{
			if (USceneComponent* AttachComp = FollowTarget->GetRootComponent())
			{
				const FVector RelativeOffset(0.f, 0.f, 40.f);
				if (SpawnImportedNiagaraAttached(World, AssetPath, AttachComp, RelativeOffset, FRotator::ZeroRotator, FVector(VisualScale), Duration))
				{
					return;
				}
			}
		}
		if (SpawnImportedNiagaraAtLocation(World, AssetPath, Location + FVector(0.f, 0.f, 28.f), FRotator::ZeroRotator, FVector(VisualScale), Duration))
		{
			return;
		}
	}

	SpawnDOTVFX(Location + FVector(0.f, 0.f, 28.f), FMath::Min(Duration, 1.6f), Radius, IdolColor);
}

void UT66CombatComponent::SpawnHeroPierceVFX(const FVector& Start, const FVector& End, const FVector& ImpactLocation, const FLinearColor& Color, const FName& HeroID)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World)
	{
		return;
	}

	const int32 RequestId = ++GHeroPierceStage2RequestSerial;
	const bool bVerbose = CVarT66VFXHeroPierceVerbose.GetValueOnGameThread() != 0;
	const FString ActiveVFXName = VFX ? VFX->GetName() : TEXT("None");
	const float TraceLength2D = FVector::Dist2D(Start, End);

	if (bVerbose)
	{
		UE_LOG(
			LogT66Combat,
			Log,
			TEXT("[ATTACK VFX][Stage2] Hero Pierce request Req=%d Hero=%s Owner=%s Time=%.3f Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f) Impact=(%.1f,%.1f,%.1f) Trace2D=%.1f ActiveVFX=%s BaseTint=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*HeroID.ToString(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			Start.X, Start.Y, Start.Z,
			End.X, End.Y, End.Z,
			ImpactLocation.X, ImpactLocation.Y, ImpactLocation.Z,
			TraceLength2D,
			*ActiveVFXName,
			Color.R, Color.G, Color.B, Color.A);
	}

	if (!VFX)
	{
		UE_LOG(LogT66Combat, Warning, TEXT("[ATTACK VFX][Stage2] Hero Pierce VFX unavailable Req=%d Hero=%s ActiveVFX=%s"), RequestId, *HeroID.ToString(), *ActiveVFXName);
		return;
	}

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	if (HeroID == FName(TEXT("Hero_1")))
	{
		SpawnHeroOnePierceVFX(Start, End, ImpactLocation);
		return;
	}
	if (!TrySpawnHeroPierceVariantPixels(World, VFX, Start, End, ColorVec, HeroID))
	{
		SpawnPierceVFX(Start, End, Color);
	}
}

void UT66CombatComponent::SpawnHeroSlashVFX(const FVector& Location, float Radius, const FLinearColor& Color, const FName& HeroID)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World)
	{
		return;
	}

	const int32 RequestId = ++GHeroAOEStage4RequestSerial;
	const bool bVerbose = CVarT66VFXHeroAOEVerbose.GetValueOnGameThread() != 0;
	const FString ActiveVFXName = VFX ? VFX->GetName() : TEXT("None");

	if (bVerbose)
	{
		UE_LOG(
			LogT66Combat,
			Log,
			TEXT("[ATTACK VFX][Stage4] Hero AOE request Req=%d Hero=%s Owner=%s Time=%.3f Location=(%.1f,%.1f,%.1f) Radius=%.1f ActiveVFX=%s BaseTint=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*HeroID.ToString(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			Location.X, Location.Y, Location.Z,
			Radius,
			*ActiveVFXName,
			Color.R, Color.G, Color.B, Color.A);
	}

	if (!VFX)
	{
		UE_LOG(LogT66Combat, Warning, TEXT("[ATTACK VFX][Stage4] Hero AOE VFX unavailable Req=%d Hero=%s ActiveVFX=%s"), RequestId, *HeroID.ToString(), *ActiveVFXName);
		return;
	}

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	if (!TrySpawnHeroAOEVariantPixels(World, VFX, Location, Radius, ColorVec, HeroID))
	{
		SpawnSlashVFX(Location, Radius, Color);
	}
}

void UT66CombatComponent::SpawnHeroBounceVFX(const TArray<FVector>& ChainPositions, const FLinearColor& Color, const FName& HeroID)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World || ChainPositions.Num() < 2)
	{
		return;
	}

	const int32 RequestId = ++GHeroBounceStage5RequestSerial;
	const bool bVerbose = CVarT66VFXHeroBounceVerbose.GetValueOnGameThread() != 0;
	const FString ActiveVFXName = VFX ? VFX->GetName() : TEXT("None");

	if (bVerbose)
	{
		UE_LOG(
			LogT66Combat,
			Log,
			TEXT("[ATTACK VFX][Stage5] Hero Bounce request Req=%d Hero=%s Owner=%s Time=%.3f Links=%d ActiveVFX=%s BaseTint=(%.2f,%.2f,%.2f,%.2f) Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f)"),
			RequestId,
			*HeroID.ToString(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			ChainPositions.Num() - 1,
			*ActiveVFXName,
			Color.R, Color.G, Color.B, Color.A,
			ChainPositions[0].X, ChainPositions[0].Y, ChainPositions[0].Z,
			ChainPositions.Last().X, ChainPositions.Last().Y, ChainPositions.Last().Z);
	}

	if (!VFX)
	{
		UE_LOG(LogT66Combat, Warning, TEXT("[ATTACK VFX][Stage5] Hero Bounce VFX unavailable Req=%d Hero=%s ActiveVFX=%s"), RequestId, *HeroID.ToString(), *ActiveVFXName);
		return;
	}

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	if (!TrySpawnHeroBounceVariantPixels(World, VFX, ChainPositions, ColorVec, HeroID))
	{
		SpawnBounceVFX(ChainPositions, Color);
	}
}

void UT66CombatComponent::SpawnHeroDOTVFX(AActor* FollowTarget, const FVector& Location, float Duration, float Radius, const FLinearColor& Color, const FName& HeroID)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World)
	{
		return;
	}

	const int32 RequestId = ++GHeroDOTStage6RequestSerial;
	const bool bVerbose = CVarT66VFXHeroDOTVerbose.GetValueOnGameThread() != 0;
	const FString ActiveVFXName = VFX ? VFX->GetName() : TEXT("None");

	if (bVerbose)
	{
		UE_LOG(
			LogT66Combat,
			Log,
			TEXT("[ATTACK VFX][Stage6] Hero DOT request Req=%d Hero=%s Owner=%s Time=%.3f Follow=%d ActiveVFX=%s Location=(%.1f,%.1f,%.1f) Duration=%.2f Radius=%.1f BaseTint=(%.2f,%.2f,%.2f,%.2f)"),
			RequestId,
			*HeroID.ToString(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			World->GetTimeSeconds(),
			FollowTarget ? 1 : 0,
			*ActiveVFXName,
			Location.X, Location.Y, Location.Z,
			Duration,
			Radius,
			Color.R, Color.G, Color.B, Color.A);
	}

	if (!VFX)
	{
		UE_LOG(LogT66Combat, Warning, TEXT("[ATTACK VFX][Stage6] Hero DOT VFX unavailable Req=%d Hero=%s ActiveVFX=%s"), RequestId, *HeroID.ToString(), *ActiveVFXName);
		return;
	}

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	if (!TrySpawnHeroDOTVariantPixels(World, VFX, Location, Radius, ColorVec, HeroID))
	{
		SpawnDOTVFX(Location, Duration, Radius, Color);
	}
}
