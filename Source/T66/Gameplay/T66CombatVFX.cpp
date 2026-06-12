// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66CombatShared.h"

#include "Gameplay/T66ArthurUltimateSword.h"
#include "Gameplay/T66HeroProjectile.h"
#include "Gameplay/T66TemporaryProjectileSystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/StaticMesh.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CoreMisc.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

namespace
{
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

	static TAutoConsoleVariable<int32> CVarT66VFXForcePrimitiveIdolPlaceholders(
		TEXT("T66.VFX.ForcePrimitiveIdolPlaceholders"),
		1,
		TEXT("Temporary content pass: non-zero routes idol activation VFX through basic-shape primitive placeholders instead of imported Niagara."));

	static TAutoConsoleVariable<int32> CVarT66CombatImportedVFXMaxPerFrame(
		TEXT("T66.VFX.CombatImportedMaxPerFrame"),
		24,
		TEXT("Max imported combat Niagara/blueprint effect spawns per frame. Values <= 0 disable this cap."));

	static TAutoConsoleVariable<int32> CVarT66CombatImportedVFXUseEffectsScalability(
		TEXT("T66.VFX.CombatImportedUseEffectsScalability"),
		1,
		TEXT("Scale imported combat VFX caps by sg.EffectsQuality."));

	static TAutoConsoleVariable<float> CVarT66CombatImportedVFXBudgetScale(
		TEXT("T66.VFX.CombatImportedBudgetScale"),
		1.0f,
		TEXT("Global multiplier applied after EffectsQuality scaling for imported combat VFX."));

	int32 GHeroAOEStage4RequestSerial = 0;
	int32 GHeroBounceStage5RequestSerial = 0;
	int32 GHeroDOTStage6RequestSerial = 0;
	int32 GIdolAOEStage7RequestSerial = 0;
	int32 GIdolBounceStage8RequestSerial = 0;
	int32 GIdolDOTStage9RequestSerial = 0;
	uint64 GCombatImportedVFXBudgetFrame = MAX_uint64;
	int32 GCombatImportedVFXEmittedThisFrame = 0;

	bool T66IsCombatImpactSourceVerboseEnabled()
	{
		if (IConsoleVariable* VerboseCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Combat.ImpactSourceVerbose")))
		{
			return VerboseCVar->GetInt() != 0;
		}
		return false;
	}

	// BLACK-tier imported projectile mesh gate. The CVar is registered in
	// T66CombatComponent.cpp (next to the physical-knockback test gate); this TU reads
	// it by name, matching the established cross-file pattern above.
	bool T66UseBlackTierProjectileMeshes()
	{
		if (IConsoleVariable* MeshesCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t66.Combat.ProjectileMeshes")))
		{
			return MeshesCVar->GetInt() != 0;
		}
		return true; // Matches the registered default (1).
	}

	float GetCombatImportedEffectsQualityScale()
	{
		if (CVarT66CombatImportedVFXUseEffectsScalability.GetValueOnGameThread() == 0)
		{
			return 1.0f;
		}

		IConsoleVariable* EffectsQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality"));
		const int32 EffectsQuality = EffectsQualityCVar ? FMath::Clamp(EffectsQualityCVar->GetInt(), 0, 4) : 3;
		switch (EffectsQuality)
		{
		case 0:
			return 0.35f;
		case 1:
			return 0.55f;
		case 2:
			return 0.75f;
		case 4:
			return 1.15f;
		case 3:
		default:
			return 1.0f;
		}
	}

	int32 GetCombatImportedVFXBudget()
	{
		const int32 BaseBudget = CVarT66CombatImportedVFXMaxPerFrame.GetValueOnGameThread();
		if (BaseBudget <= 0)
		{
			return BaseBudget;
		}

		const float Scale = FMath::Max(0.05f, GetCombatImportedEffectsQualityScale() * CVarT66CombatImportedVFXBudgetScale.GetValueOnGameThread());
		return FMath::Max(4, FMath::RoundToInt(static_cast<float>(BaseBudget) * Scale));
	}

	bool TryConsumeCombatImportedVFXBudget()
	{
		if (GCombatImportedVFXBudgetFrame != GFrameCounter)
		{
			GCombatImportedVFXBudgetFrame = GFrameCounter;
			GCombatImportedVFXEmittedThisFrame = 0;
		}

		const int32 Budget = GetCombatImportedVFXBudget();
		if (Budget > 0 && GCombatImportedVFXEmittedThisFrame >= Budget)
		{
			return false;
		}

		++GCombatImportedVFXEmittedThisFrame;
		return true;
	}

	UNiagaraSystem* ResolveNiagaraSystemCached(const TCHAR* AssetPath)
	{
		static TMap<FString, TWeakObjectPtr<UNiagaraSystem>> Cache;
		static TMap<FString, TSharedPtr<FStreamableHandle>> ActiveLoads;
		if (!AssetPath || !*AssetPath)
		{
			return nullptr;
		}

		const FSoftObjectPath Path(AssetPath);
		const FString Key = Path.ToString();
		if (const TWeakObjectPtr<UNiagaraSystem>* Found = Cache.Find(Key))
		{
			if (Found->IsValid())
			{
				return Found->Get();
			}
		}

		if (UNiagaraSystem* Resolved = Cast<UNiagaraSystem>(Path.ResolveObject()))
		{
			Cache.Add(Key, Resolved);
			return Resolved;
		}

		if (!ActiveLoads.Contains(Key))
		{
			TArray<FSoftObjectPath> AssetPaths;
			AssetPaths.Add(Path);
			ActiveLoads.Add(Key, UAssetManager::GetStreamableManager().RequestAsyncLoad(AssetPaths, FStreamableDelegate()));
		}

		return nullptr;
	}

	UClass* ResolveEffectBlueprintClassCached(const TCHAR* ClassPath)
	{
		static TMap<FString, TWeakObjectPtr<UClass>> Cache;
		static TMap<FString, TSharedPtr<FStreamableHandle>> ActiveLoads;
		if (!ClassPath || !*ClassPath)
		{
			return nullptr;
		}

		const FSoftObjectPath Path(ClassPath);
		const FString Key = Path.ToString();
		if (const TWeakObjectPtr<UClass>* Found = Cache.Find(Key))
		{
			if (Found->IsValid())
			{
				return Found->Get();
			}
		}

		if (UClass* Resolved = Cast<UClass>(Path.ResolveObject()))
		{
			Cache.Add(Key, Resolved);
			return Resolved;
		}

		if (!ActiveLoads.Contains(Key))
		{
			TArray<FSoftObjectPath> AssetPaths;
			AssetPaths.Add(Path);
			ActiveLoads.Add(Key, UAssetManager::GetStreamableManager().RequestAsyncLoad(AssetPaths, FStreamableDelegate()));
		}

		return nullptr;
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
		UNiagaraSystem* System = ResolveNiagaraSystemCached(AssetPath);
		if (!World || !System)
		{
			return nullptr;
		}
		if (!TryConsumeCombatImportedVFXBudget())
		{
			return nullptr;
		}

		const bool bAutoDestroy = ActiveDurationSeconds <= 0.f;
		const ENCPoolMethod PoolingMethod = bAutoDestroy ? ENCPoolMethod::AutoRelease : ENCPoolMethod::None;

		UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			System,
			Location,
			Rotation,
			Scale,
			bAutoDestroy,
			true,
			PoolingMethod,
			true);
		if (NiagaraComponent)
		{
			if (!bAutoDestroy)
			{
				NiagaraComponent->SetAutoDestroy(false);
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
		UNiagaraSystem* System = ResolveNiagaraSystemCached(AssetPath);
		if (!World || !System || !AttachComponent)
		{
			return nullptr;
		}
		if (!TryConsumeCombatImportedVFXBudget())
		{
			return nullptr;
		}

		const bool bAutoDestroy = ActiveDurationSeconds <= 0.f;
		const ENCPoolMethod PoolingMethod = bAutoDestroy ? ENCPoolMethod::AutoRelease : ENCPoolMethod::None;

		UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			System,
			AttachComponent,
			NAME_None,
			RelativeLocation,
			RelativeRotation,
			Scale,
			EAttachLocation::KeepRelativeOffset,
			bAutoDestroy,
			PoolingMethod,
			true,
			true);
		if (NiagaraComponent)
		{
			if (!bAutoDestroy)
			{
				NiagaraComponent->SetAutoDestroy(false);
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
		UClass* EffectClass = ResolveEffectBlueprintClassCached(ClassPath);
		if (!World || !EffectClass)
		{
			return false;
		}
		if (!TryConsumeCombatImportedVFXBudget())
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

	bool SpawnImportedEffectAlongLine(
		UWorld* World,
		const TCHAR* AssetPath,
		const FVector& Start,
		const FVector& End,
		const float VisualScale,
		const float QuantityMultiplier)
	{
		UNiagaraSystem* System = ResolveNiagaraSystemCached(AssetPath);
		if (!World || !System)
		{
			return false;
		}

		const FVector Delta = End - Start;
		const float Distance = Delta.Size();
		if (Distance <= KINDA_SMALL_NUMBER)
		{
			return SpawnImportedNiagaraAtLocation(World, AssetPath, Start, FRotator::ZeroRotator, FVector(VisualScale)) != nullptr;
		}

		const FRotator Rotation = Delta.Rotation();
		const int32 SpawnCount = FMath::Clamp(FMath::RoundToInt((Distance / 150.f) * FMath::Max(0.5f, QuantityMultiplier)), 1, 18);
		int32 SpawnedCount = 0;
		for (int32 Index = 0; Index < SpawnCount; ++Index)
		{
			const float T = (SpawnCount > 1) ? static_cast<float>(Index) / static_cast<float>(SpawnCount - 1) : 0.5f;
			const FVector SpawnLocation = FMath::Lerp(Start, End, T);
			if (SpawnImportedNiagaraAtLocation(World, AssetPath, SpawnLocation, Rotation, FVector(VisualScale)))
			{
				++SpawnedCount;
			}
		}
		return SpawnedCount > 0;
	}

	bool SpawnImportedEffectAlongChain(
		UWorld* World,
		const TCHAR* AssetPath,
		const TArray<FVector>& Points,
		const float VisualScale,
		const float QuantityMultiplier)
	{
		if (!World || Points.Num() < 2)
		{
			return false;
		}

		bool bSpawnedAny = false;
		for (int32 Index = 0; Index < Points.Num() - 1; ++Index)
		{
			bSpawnedAny |= SpawnImportedEffectAlongLine(World, AssetPath, Points[Index], Points[Index + 1], VisualScale, QuantityMultiplier);
		}
		return bSpawnedAny;
	}

	const TCHAR* GetIdolNiagaraEffectPath(const FName& IdolID)
	{
		const FName NormalizedIdolID = UT66IdolManagerSubsystem::NormalizeLegacyIdolID(IdolID);
		if (NormalizedIdolID == FName(TEXT("Idol_Fire_DOT")) || NormalizedIdolID == FName(TEXT("Idol_Fire_AOE"))) return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire");
		if (NormalizedIdolID == FName(TEXT("Idol_Fire_Summon")) || NormalizedIdolID == FName(TEXT("Idol_Fire_Bounce"))) return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_01.P_Weapon_01");
		if (NormalizedIdolID == FName(TEXT("Idol_Ice_DOT")) || NormalizedIdolID == FName(TEXT("Idol_Ice_AOE")) || NormalizedIdolID == FName(TEXT("Idol_Ice_Summon")) || NormalizedIdolID == FName(TEXT("Idol_Ice_Bounce"))) return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Ice_Projectile_02.P_Ice_Projectile_02");
		if (NormalizedIdolID == FName(TEXT("Idol_Electricity_DOT")) || NormalizedIdolID == FName(TEXT("Idol_Electricity_AOE")) || NormalizedIdolID == FName(TEXT("Idol_Electricity_Summon")) || NormalizedIdolID == FName(TEXT("Idol_Electricity_Bounce"))) return TEXT("/Game/Stylized_VFX_StPack/Particles/P_Electric_Projectile_02.P_Electric_Projectile_02");
		if (NormalizedIdolID == FName(TEXT("Idol_Nature_DOT"))) return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Poison_02.P_Poison_02");
		if (NormalizedIdolID == FName(TEXT("Idol_Nature_AOE"))) return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Dirt_Spikes_02.P_Dirt_Spikes_02");
		if (NormalizedIdolID == FName(TEXT("Idol_Nature_Summon")) || NormalizedIdolID == FName(TEXT("Idol_Nature_Bounce"))) return TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Web_Projectile_01.P_Web_Projectile_01");
		return nullptr;
	}

	const TCHAR* GetIdolBlueprintEffectClassPath(const FName& IdolID)
	{
		const FName NormalizedIdolID = UT66IdolManagerSubsystem::NormalizeLegacyIdolID(IdolID);
		if (NormalizedIdolID == FName(TEXT("Idol_Electricity_AOE")))
		{
			return TEXT("/Game/Stylized_VFX_StPack/Blueprints/BP_Storm.BP_Storm_C");
		}
		return nullptr;
	}

	enum class ET66PrimitiveIdolElement : uint8
	{
		Fire,
		Ice,
		Electricity,
		Nature,
		Wind,
		Unknown,
	};

	enum class ET66PrimitiveShape : uint8
	{
		Sphere,
		Cone,
		Cylinder,
		Cube,
	};

	ET66PrimitiveIdolElement T66ResolvePrimitiveIdolElement(const FName& IdolID)
	{
		const FString ID = UT66IdolManagerSubsystem::NormalizeLegacyIdolID(IdolID).ToString();
		if (ID.Contains(TEXT("_Fire_"))) return ET66PrimitiveIdolElement::Fire;
		if (ID.Contains(TEXT("_Ice_"))) return ET66PrimitiveIdolElement::Ice;
		if (ID.Contains(TEXT("_Electricity_"))) return ET66PrimitiveIdolElement::Electricity;
		if (ID.Contains(TEXT("_Nature_"))) return ET66PrimitiveIdolElement::Nature;
		if (ID.Contains(TEXT("_Wind_"))) return ET66PrimitiveIdolElement::Wind;
		return ET66PrimitiveIdolElement::Unknown;
	}

	const TCHAR* T66PrimitiveElementName(const ET66PrimitiveIdolElement Element)
	{
		switch (Element)
		{
		case ET66PrimitiveIdolElement::Fire: return TEXT("Fire");
		case ET66PrimitiveIdolElement::Ice: return TEXT("Ice");
		case ET66PrimitiveIdolElement::Electricity: return TEXT("Electricity");
		case ET66PrimitiveIdolElement::Nature: return TEXT("Nature");
		case ET66PrimitiveIdolElement::Wind: return TEXT("Wind");
		default: return TEXT("Unknown");
		}
	}

	FLinearColor T66PrimitiveIdolColor(const FName& IdolID)
	{
		switch (T66ResolvePrimitiveIdolElement(IdolID))
		{
		case ET66PrimitiveIdolElement::Fire:
			return FLinearColor(1.0f, 0.035f, 0.0f, 1.0f);
		case ET66PrimitiveIdolElement::Ice:
			return FLinearColor(0.52f, 0.92f, 1.0f, 1.0f);
		case ET66PrimitiveIdolElement::Electricity:
			return FLinearColor(0.55f, 0.10f, 1.0f, 1.0f);
		case ET66PrimitiveIdolElement::Nature:
			return FLinearColor(0.08f, 0.78f, 0.22f, 1.0f);
		case ET66PrimitiveIdolElement::Wind:
			return FLinearColor(0.62f, 0.66f, 0.70f, 1.0f);
		default:
			return UT66IdolManagerSubsystem::GetIdolColor(IdolID);
		}
	}

	int32 T66RarityIndex(const ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black: return 0;
		case ET66ItemRarity::Red: return 1;
		case ET66ItemRarity::Yellow: return 2;
		case ET66ItemRarity::White: return 3;
		default: return 0;
		}
	}

	int32 T66TieredCount(const ET66ItemRarity Rarity, const int32 Black, const int32 Red, const int32 Yellow, const int32 White)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black: return Black;
		case ET66ItemRarity::Red: return Red;
		case ET66ItemRarity::Yellow: return Yellow;
		case ET66ItemRarity::White: return White;
		default: return Black;
		}
	}

	UStaticMesh* T66PrimitiveShapeMesh(const ET66PrimitiveShape Shape)
	{
		switch (Shape)
		{
		case ET66PrimitiveShape::Cone: return FT66VisualUtil::GetBasicShapeCone();
		case ET66PrimitiveShape::Cylinder: return FT66VisualUtil::GetBasicShapeCylinder();
		case ET66PrimitiveShape::Cube: return FT66VisualUtil::GetBasicShapeCube();
		case ET66PrimitiveShape::Sphere:
		default: return FT66VisualUtil::GetBasicShapeSphere();
		}
	}

	FRotator T66RotationFromUpToDirection(const FVector& Direction)
	{
		const FVector SafeDirection = Direction.IsNearlyZero() ? FVector::UpVector : Direction.GetSafeNormal();
		return FQuat::FindBetweenNormals(FVector::UpVector, SafeDirection).Rotator();
	}

	AActor* T66SpawnPrimitiveShapeActor(
		UWorld* World,
		AActor* Owner,
		const FName Tag,
		const ET66PrimitiveShape Shape,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& Scale,
		const FLinearColor& Color,
		const float LifeSpan,
		AActor* AttachTarget = nullptr)
	{
		if (!World)
		{
			return nullptr;
		}

		UStaticMesh* Mesh = T66PrimitiveShapeMesh(Shape);
		if (!Mesh)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* PlaceholderActor = World->SpawnActor<AActor>(
			AActor::StaticClass(),
			Location,
			Rotation,
			SpawnParams);
		if (!PlaceholderActor)
		{
			return nullptr;
		}

		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(PlaceholderActor, TEXT("T66PrimitiveIdolPlaceholderMesh"));
		if (!MeshComponent)
		{
			PlaceholderActor->Destroy();
			return nullptr;
		}

		PlaceholderActor->Tags.AddUnique(Tag);
		PlaceholderActor->SetRootComponent(MeshComponent);
		MeshComponent->SetStaticMesh(Mesh);
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetGenerateOverlapEvents(false);
		MeshComponent->SetCastShadow(false);
		MeshComponent->RegisterComponent();
		PlaceholderActor->SetActorLocationAndRotation(Location, Rotation);
		MeshComponent->SetWorldScale3D(Scale);
		FT66VisualUtil::ApplyT66Color(MeshComponent, PlaceholderActor, Color);

		if (AttachTarget)
		{
			if (USceneComponent* AttachRoot = AttachTarget->GetRootComponent())
			{
				PlaceholderActor->AttachToComponent(AttachRoot, FAttachmentTransformRules::KeepWorldTransform);
			}
		}

		PlaceholderActor->SetLifeSpan(FMath::Max(0.1f, LifeSpan));
		return PlaceholderActor;
	}

	int32 T66RarityProjectileBodyCount(const ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Red:
			return 3;
		case ET66ItemRarity::Yellow:
			return 5;
		case ET66ItemRarity::Black:
		case ET66ItemRarity::White:
		default:
			return 1;
		}
	}

	float T66RarityProjectileBodyScale(const ET66ItemRarity Rarity)
	{
		return Rarity == ET66ItemRarity::White ? 2.25f : 1.f;
	}

	bool T66ResolveIdolProjectileMeshBinding(
		UWorld* World,
		const FName IdolID,
		const ET66AttackCategory Category,
		FT66CombatVFXBindingData& OutBinding,
		UStaticMesh*& OutMesh)
	{
		OutMesh = nullptr;
		if (!World || IdolID.IsNone() || !T66UseBlackTierProjectileMeshes())
		{
			return false;
		}

		UT66GameInstance* GameInstance = World->GetGameInstance<UT66GameInstance>();
		if (!GameInstance
			|| !GameInstance->GetCombatVFXBindingData(
				ET66CombatVFXBindingSourceType::IdolModifier,
				IdolID,
				Category,
				OutBinding))
		{
			return false;
		}

		OutMesh = OutBinding.ProjectileMesh.LoadSynchronous();
		return OutMesh != nullptr;
	}

	AActor* T66SpawnProjectileMeshBodyActor(
		UWorld* World,
		AActor* Owner,
		const FName Tag,
		UStaticMesh* Mesh,
		const FVector& Location,
		const FRotator& Rotation,
		const float MeshScale,
		const float LifeSpan,
		AActor* AttachTarget = nullptr)
	{
		if (!World || !Mesh)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* BodyActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation, SpawnParams);
		if (!BodyActor)
		{
			return nullptr;
		}

		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(BodyActor, TEXT("T66ProjectileMeshBody"));
		if (!MeshComponent)
		{
			BodyActor->Destroy();
			return nullptr;
		}

		BodyActor->Tags.AddUnique(Tag);
		BodyActor->SetRootComponent(MeshComponent);
		MeshComponent->SetStaticMesh(Mesh);
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetGenerateOverlapEvents(false);
		MeshComponent->SetCastShadow(false);
		MeshComponent->RegisterComponent();
		BodyActor->SetActorLocationAndRotation(Location, Rotation);
		MeshComponent->SetWorldScale3D(FVector(FMath::Max(0.01f, MeshScale)));
		MeshComponent->SetVisibility(true, true);
		MeshComponent->SetHiddenInGame(false, true);
		MeshComponent->SetRenderInMainPass(true);

		if (AttachTarget)
		{
			if (USceneComponent* AttachRoot = AttachTarget->GetRootComponent())
			{
				BodyActor->AttachToComponent(AttachRoot, FAttachmentTransformRules::KeepWorldTransform);
			}
		}

		BodyActor->SetLifeSpan(FMath::Max(0.1f, LifeSpan));
		return BodyActor;
	}

	void T66SpawnPrimitiveDisc(UWorld* World, AActor* Owner, const FName Tag, const FVector& Center, const float Radius, const float Height, const FLinearColor& Color, const float LifeSpan)
	{
		const float SafeRadius = FMath::Max(6.f, Radius);
		T66SpawnPrimitiveShapeActor(
			World,
			Owner,
			Tag,
			ET66PrimitiveShape::Cylinder,
			Center,
			FRotator::ZeroRotator,
			FVector(SafeRadius / 50.f, SafeRadius / 50.f, FMath::Max(2.f, Height) / 100.f),
			Color,
			LifeSpan);
	}

	void T66SpawnPrimitiveLine(
		UWorld* World,
		AActor* Owner,
		const FName Tag,
		const ET66PrimitiveShape Shape,
		const FVector& Start,
		const FVector& End,
		const float Radius,
		const FLinearColor& Color,
		const float LifeSpan)
	{
		const FVector Delta = End - Start;
		const float Length = Delta.Size();
		if (Length <= KINDA_SMALL_NUMBER)
		{
			return;
		}

		const FVector Mid = (Start + End) * 0.5f;
		const float SafeRadius = FMath::Max(3.f, Radius);
		T66SpawnPrimitiveShapeActor(
			World,
			Owner,
			Tag,
			Shape,
			Mid,
			T66RotationFromUpToDirection(Delta),
			FVector(SafeRadius / 50.f, SafeRadius / 50.f, Length / 100.f),
			Color,
			LifeSpan);
	}

	void T66SpawnPrimitiveRingPoints(
		UWorld* World,
		AActor* Owner,
		const FName Tag,
		const ET66PrimitiveShape Shape,
		const FVector& Center,
		const float Radius,
		const int32 Count,
		const float ShapeRadius,
		const FLinearColor& Color,
		const float LifeSpan,
		const float ZOffset = 20.f)
	{
		const int32 SafeCount = FMath::Max(1, Count);
		for (int32 Index = 0; Index < SafeCount; ++Index)
		{
			const float Angle = (static_cast<float>(Index) / static_cast<float>(SafeCount)) * 2.f * PI;
			const FVector Dir(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
			const FVector Location = Center + Dir * Radius + FVector(0.f, 0.f, ZOffset);
			const FRotator Rot = Shape == ET66PrimitiveShape::Cone ? T66RotationFromUpToDirection(Dir + FVector(0.f, 0.f, 0.35f)) : FRotator::ZeroRotator;
			T66SpawnPrimitiveShapeActor(
				World,
				Owner,
				Tag,
				Shape,
				Location,
				Rot,
				FVector(ShapeRadius / 50.f),
				Color,
				LifeSpan);
		}
	}

	void T66SpawnPrimitiveTornadoStack(
		UWorld* World,
		AActor* Owner,
		const FName Tag,
		const FVector& Base,
		const float Radius,
		const float Height,
		const FLinearColor& Color,
		const float LifeSpan,
		AActor* AttachTarget = nullptr)
	{
		const int32 Layers = 4;
		for (int32 Index = 0; Index < Layers; ++Index)
		{
			const float T = static_cast<float>(Index) / static_cast<float>(Layers - 1);
			const float LayerRadius = FMath::Lerp(Radius, Radius * 0.35f, T);
			const float LayerHeight = FMath::Max(6.f, Height / static_cast<float>(Layers));
			const FVector Loc = Base + FVector(0.f, 0.f, LayerHeight * (0.5f + Index));
			T66SpawnPrimitiveShapeActor(
				World,
				Owner,
				Tag,
				Index % 2 == 0 ? ET66PrimitiveShape::Cone : ET66PrimitiveShape::Cylinder,
				Loc,
				FRotator(0.f, static_cast<float>(Index) * 38.f, 0.f),
				FVector(LayerRadius / 50.f, LayerRadius / 50.f, LayerHeight / 100.f),
				Color,
				LifeSpan,
				AttachTarget);
		}
	}

	bool T66SpawnPrimitiveIdolAOEPlaceholder(UWorld* World, AActor* Owner, const FName IdolID, const ET66ItemRarity Rarity, const FVector& Location, const float Radius)
	{
		if (!World)
		{
			return false;
		}

		const ET66PrimitiveIdolElement Element = T66ResolvePrimitiveIdolElement(IdolID);
		const FLinearColor Color = T66PrimitiveIdolColor(IdolID);
		const FName Tag(TEXT("T66PrimitiveIdolAOEPlaceholder"));
		const float RarityScale = T66CombatShared::GetIdolRarityVisualScale(Rarity);
		const float VisualRadius = FMath::Max(48.f, Radius) * FMath::Clamp(RarityScale, 0.8f, 1.75f);
		const float LifeSpan = 1.2f;

		switch (Element)
		{
		case ET66PrimitiveIdolElement::Fire:
		{
			T66SpawnPrimitiveShapeActor(World, Owner, Tag, ET66PrimitiveShape::Sphere, Location + FVector(0.f, 0.f, 42.f), FRotator::ZeroRotator, FVector(VisualRadius / 95.f), Color, LifeSpan);
			T66SpawnPrimitiveRingPoints(World, Owner, Tag, ET66PrimitiveShape::Cone, Location, VisualRadius * 0.42f, T66TieredCount(Rarity, 6, 8, 10, 14), VisualRadius * 0.18f, Color, LifeSpan, 28.f);
			break;
		}
		case ET66PrimitiveIdolElement::Ice:
		{
			T66SpawnPrimitiveDisc(World, Owner, Tag, Location + FVector(0.f, 0.f, 4.f), VisualRadius, 8.f, Color, LifeSpan);
			T66SpawnPrimitiveRingPoints(World, Owner, Tag, ET66PrimitiveShape::Cone, Location, VisualRadius * 0.78f, T66TieredCount(Rarity, 8, 10, 12, 16), VisualRadius * 0.10f, Color, LifeSpan, 18.f);
			break;
		}
		case ET66PrimitiveIdolElement::Electricity:
		{
			const int32 Strikes = T66TieredCount(Rarity, 3, 4, 6, 8);
			for (int32 Index = 0; Index < Strikes; ++Index)
			{
				const float Angle = (static_cast<float>(Index) / static_cast<float>(Strikes)) * 2.f * PI + 0.25f;
				const float Dist = (Index == 0) ? 0.f : VisualRadius * (0.28f + 0.42f * FMath::Fmod(static_cast<float>(Index), 3.f) / 2.f);
				const FVector StrikeBase = Location + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 18.f);
				T66SpawnPrimitiveLine(World, Owner, Tag, ET66PrimitiveShape::Cylinder, StrikeBase + FVector(0.f, 0.f, 280.f), StrikeBase + FVector(0.f, 0.f, 28.f), 9.f * RarityScale, Color, LifeSpan);
				T66SpawnPrimitiveShapeActor(World, Owner, Tag, ET66PrimitiveShape::Sphere, StrikeBase + FVector(0.f, 0.f, 24.f), FRotator::ZeroRotator, FVector(0.32f * RarityScale), Color, LifeSpan);
			}
			break;
		}
		case ET66PrimitiveIdolElement::Nature:
		{
			T66SpawnPrimitiveDisc(World, Owner, Tag, Location + FVector(0.f, 0.f, 5.f), VisualRadius * 0.72f, 6.f, Color, LifeSpan);
			const int32 Branches = T66TieredCount(Rarity, 5, 7, 9, 12);
			for (int32 Index = 0; Index < Branches; ++Index)
			{
				const float Angle = (static_cast<float>(Index) / static_cast<float>(Branches)) * 2.f * PI;
				const FVector Dir(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
				T66SpawnPrimitiveLine(World, Owner, Tag, ET66PrimitiveShape::Cylinder, Location + FVector(0.f, 0.f, 16.f), Location + Dir * VisualRadius + FVector(0.f, 0.f, 36.f), 10.f * RarityScale, Color, LifeSpan);
				T66SpawnPrimitiveShapeActor(World, Owner, Tag, ET66PrimitiveShape::Sphere, Location + Dir * (VisualRadius * 0.82f) + FVector(0.f, 0.f, 52.f), FRotator::ZeroRotator, FVector(0.18f * RarityScale), Color, LifeSpan);
			}
			break;
		}
		case ET66PrimitiveIdolElement::Wind:
		{
			const int32 Tornadoes = T66TieredCount(Rarity, 3, 4, 5, 7);
			T66SpawnPrimitiveDisc(World, Owner, Tag, Location + FVector(0.f, 0.f, 3.f), VisualRadius * 0.92f, 5.f, Color, LifeSpan);
			for (int32 Index = 0; Index < Tornadoes; ++Index)
			{
				const float Angle = (static_cast<float>(Index) / static_cast<float>(Tornadoes)) * 2.f * PI;
				const FVector OrbitLoc = Location + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * (VisualRadius * 0.58f);
				T66SpawnPrimitiveTornadoStack(World, Owner, Tag, OrbitLoc, VisualRadius * 0.12f, 150.f * RarityScale, Color, LifeSpan);
			}
			break;
		}
		default:
			T66SpawnPrimitiveDisc(World, Owner, Tag, Location + FVector(0.f, 0.f, 5.f), VisualRadius, 8.f, Color, LifeSpan);
			break;
		}
		return true;
	}

	bool T66SpawnPrimitiveIdolBouncePlaceholder(UWorld* World, AActor* Owner, const FName IdolID, const ET66ItemRarity Rarity, const TArray<FVector>& ChainPositions)
	{
		if (!World || ChainPositions.Num() < 2)
		{
			return false;
		}

		const ET66PrimitiveIdolElement Element = T66ResolvePrimitiveIdolElement(IdolID);
		const FLinearColor Color = T66PrimitiveIdolColor(IdolID);
		const FName Tag(TEXT("T66PrimitiveIdolBouncePlaceholder"));
		const float RarityScale = T66CombatShared::GetIdolRarityVisualScale(Rarity);
		const float LifeSpan = 1.0f;

		for (int32 Index = 0; Index < ChainPositions.Num() - 1; ++Index)
		{
			const FVector Start = ChainPositions[Index] + FVector(0.f, 0.f, 34.f);
			const FVector End = ChainPositions[Index + 1] + FVector(0.f, 0.f, 34.f);
			switch (Element)
			{
			case ET66PrimitiveIdolElement::Fire:
				T66SpawnPrimitiveLine(World, Owner, Tag, ET66PrimitiveShape::Cylinder, Start, End, 6.f * RarityScale, Color, LifeSpan);
				T66SpawnPrimitiveShapeActor(World, Owner, Tag, ET66PrimitiveShape::Sphere, End, FRotator::ZeroRotator, FVector(0.24f * RarityScale), Color, LifeSpan);
				T66SpawnPrimitiveShapeActor(World, Owner, Tag, ET66PrimitiveShape::Cube, (Start + End) * 0.5f, FRotator(0.f, 45.f, 45.f), FVector(0.16f * RarityScale), Color, LifeSpan);
				break;
			case ET66PrimitiveIdolElement::Ice:
				T66SpawnPrimitiveLine(World, Owner, Tag, ET66PrimitiveShape::Cone, Start, End, 11.f * RarityScale, Color, LifeSpan);
				T66SpawnPrimitiveShapeActor(World, Owner, Tag, ET66PrimitiveShape::Cone, End, T66RotationFromUpToDirection((End - Start).GetSafeNormal()), FVector(0.22f * RarityScale, 0.22f * RarityScale, 0.58f * RarityScale), Color, LifeSpan);
				break;
			case ET66PrimitiveIdolElement::Electricity:
				T66SpawnPrimitiveLine(World, Owner, Tag, ET66PrimitiveShape::Cylinder, Start, End, 7.f * RarityScale, Color, LifeSpan);
				T66SpawnPrimitiveShapeActor(World, Owner, Tag, ET66PrimitiveShape::Sphere, End, FRotator::ZeroRotator, FVector(0.22f * RarityScale), Color, LifeSpan);
				break;
			case ET66PrimitiveIdolElement::Nature:
				T66SpawnPrimitiveLine(World, Owner, Tag, ET66PrimitiveShape::Cylinder, Start, End, 7.f * RarityScale, Color, LifeSpan);
				T66SpawnPrimitiveShapeActor(World, Owner, Tag, ET66PrimitiveShape::Sphere, End, FRotator::ZeroRotator, FVector(0.26f * RarityScale), Color, LifeSpan);
				break;
			case ET66PrimitiveIdolElement::Wind:
				T66SpawnPrimitiveTornadoStack(World, Owner, Tag, End - FVector(0.f, 0.f, 28.f), 16.f * RarityScale, 95.f * RarityScale, Color, LifeSpan);
				break;
			default:
				T66SpawnPrimitiveLine(World, Owner, Tag, ET66PrimitiveShape::Cylinder, Start, End, 8.f * RarityScale, Color, LifeSpan);
				break;
			}
		}
		return true;
	}

	bool T66SpawnPrimitiveIdolDOTPlaceholder(UWorld* World, AActor* Owner, const FName IdolID, const ET66ItemRarity Rarity, AActor* FollowTarget, const FVector& Location, const float Duration)
	{
		if (!World)
		{
			return false;
		}

		const ET66PrimitiveIdolElement Element = T66ResolvePrimitiveIdolElement(IdolID);
		const FLinearColor Color = T66PrimitiveIdolColor(IdolID);
		const FName Tag(TEXT("T66PrimitiveIdolDOTPlaceholder"));
		const float RarityScale = T66CombatShared::GetIdolRarityVisualScale(Rarity);
		const float LifeSpan = FMath::Clamp(Duration, 0.8f, 4.0f);
		const FVector Base = FollowTarget ? FollowTarget->GetActorLocation() : Location;
		const FVector BodyCenter = Base + FVector(0.f, 0.f, 52.f);
		AActor* AttachTarget = FollowTarget;

		switch (Element)
		{
		case ET66PrimitiveIdolElement::Fire:
		{
			const int32 Sparks = T66TieredCount(Rarity, 5, 7, 10, 14);
			for (int32 Index = 0; Index < Sparks; ++Index)
			{
				const float Angle = (static_cast<float>(Index) / static_cast<float>(Sparks)) * 2.f * PI;
				const FVector Offset(FMath::Cos(Angle) * 28.f, FMath::Sin(Angle) * 28.f, 16.f * (Index % 3));
				T66SpawnPrimitiveShapeActor(World, Owner, Tag, Index % 2 == 0 ? ET66PrimitiveShape::Cone : ET66PrimitiveShape::Sphere, BodyCenter + Offset, FRotator(0.f, Angle * 57.29578f, 0.f), FVector(0.20f * RarityScale), Color, LifeSpan, AttachTarget);
			}
			break;
		}
		case ET66PrimitiveIdolElement::Ice:
			T66SpawnPrimitiveShapeActor(World, Owner, Tag, ET66PrimitiveShape::Sphere, BodyCenter, FRotator::ZeroRotator, FVector(0.82f * RarityScale), Color, LifeSpan, AttachTarget);
			T66SpawnPrimitiveShapeActor(World, Owner, Tag, ET66PrimitiveShape::Cube, BodyCenter + FVector(0.f, 0.f, 18.f), FRotator(0.f, 45.f, 45.f), FVector(0.42f * RarityScale), Color, LifeSpan, AttachTarget);
			break;
		case ET66PrimitiveIdolElement::Electricity:
		{
			const int32 Arcs = T66TieredCount(Rarity, 4, 6, 8, 11);
			for (int32 Index = 0; Index < Arcs; ++Index)
			{
				const float Angle = (static_cast<float>(Index) / static_cast<float>(Arcs)) * 2.f * PI;
				const FVector Side(FMath::Cos(Angle) * 32.f, FMath::Sin(Angle) * 32.f, 0.f);
				T66SpawnPrimitiveLine(World, Owner, Tag, ET66PrimitiveShape::Cylinder, BodyCenter + Side + FVector(0.f, 0.f, -34.f), BodyCenter - Side * 0.35f + FVector(0.f, 0.f, 42.f), 6.f * RarityScale, Color, LifeSpan);
			}
			break;
		}
		case ET66PrimitiveIdolElement::Nature:
			T66SpawnPrimitiveShapeActor(World, Owner, Tag, ET66PrimitiveShape::Sphere, BodyCenter, FRotator::ZeroRotator, FVector(0.46f * RarityScale), Color, LifeSpan, AttachTarget);
			T66SpawnPrimitiveRingPoints(World, Owner, Tag, ET66PrimitiveShape::Sphere, BodyCenter, 34.f * RarityScale, T66TieredCount(Rarity, 5, 7, 9, 12), 12.f * RarityScale, Color, LifeSpan, 0.f);
			break;
		case ET66PrimitiveIdolElement::Wind:
			T66SpawnPrimitiveTornadoStack(World, Owner, Tag, Base + FVector(0.f, 0.f, 4.f), 26.f * RarityScale, 145.f * RarityScale, Color, LifeSpan, AttachTarget);
			break;
		default:
			T66SpawnPrimitiveShapeActor(World, Owner, Tag, ET66PrimitiveShape::Sphere, BodyCenter, FRotator::ZeroRotator, FVector(0.44f * RarityScale), Color, LifeSpan, AttachTarget);
			break;
		}
		return true;
	}

	void PreloadImportedCombatVFXAssetsAsync()
	{
		static bool bRequested = false;
		if (bRequested)
		{
			return;
		}
		bRequested = true;

		static const TCHAR* NiagaraPaths[] =
		{
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Portal.P_Cosmic_Portal"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Poison_02.P_Poison_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Liquid_Hit_03.P_Liquid_Hit_03"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/P_Electric_Projectile_02.P_Electric_Projectile_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Ice_Projectile_02.P_Ice_Projectile_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Projectile_02.P_Cosmic_Projectile_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Cosmic_Projectile_03.P_Cosmic_Projectile_03"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Dirt_Spikes_02.P_Dirt_Spikes_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/P_Splash_02.P_Splash_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/P_Laser_02.P_Laser_02"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_01.P_Weapon_01"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Web_Projectile_01.P_Web_Projectile_01"),
			TEXT("/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_02.P_Weapon_02"),
			TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"),
			TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"),
		};

		for (const TCHAR* Path : NiagaraPaths)
		{
			ResolveNiagaraSystemCached(Path);
		}
		ResolveEffectBlueprintClassCached(TEXT("/Game/Stylized_VFX_StPack/Blueprints/BP_Storm.BP_Storm_C"));
	}

	UNiagaraSystem* FindPixelVFXSystem()
	{
		if (UNiagaraSystem* PixelSystem = ResolveNiagaraSystemCached(TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle")))
		{
			return PixelSystem;
		}

		return ResolveNiagaraSystemCached(TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"));
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

	bool TrySpawnHeroAOEVariantPixels(
		UWorld* World,
		UNiagaraSystem* VFX,
		const FVector& Location,
		const float Radius,
		const FVector4& ColorVec,
		const FName HeroID)
	{
		if (HeroID == FName(TEXT("Hero_2")))
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

		if (HeroID == FName(TEXT("Hero_3")))
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

		if (HeroID == FName(TEXT("Hero_12")))
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
		if (HeroID == FName(TEXT("Hero_5")))
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

		if (HeroID == FName(TEXT("Hero_9")))
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

		if (HeroID == FName(TEXT("Hero_7")))
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
		if (HeroID == FName(TEXT("Hero_10")))
		{
			constexpr int32 N = 8;
			for (int32 i = 0; i < N; ++i)
			{
				const FVector Offset(FMath::FRandRange(-15.f, 15.f), FMath::FRandRange(-15.f, 15.f), -static_cast<float>(i) * 12.f);
				T66SpawnBudgetedPixel(World, VFX, Location + Offset, ColorVec, FVector2D(2.0f, 2.0f), ET66PixelVFXPriority::Medium, FRotator::ZeroRotator, true);
			}
			return true;
		}

		if (HeroID == FName(TEXT("Hero_11")))
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

		return false;
	}
}

void UT66CombatComponent::WarmupVFXSystems()
{
	PreloadImportedCombatVFXAssetsAsync();
	if (!CachedSlashVFXNiagara) { CachedSlashVFXNiagara = SlashVFXNiagara.Get(); }
	if (!CachedPixelVFXNiagara) { CachedPixelVFXNiagara = PixelVFXNiagara.Get(); }
	if (!CachedSlashVFXNiagara || !CachedPixelVFXNiagara) { PrimeCombatPresentationAssetsAsync(); }
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
	UT66CombatComponent* MutableThis = const_cast<UT66CombatComponent*>(this);
	if (!MutableThis->CachedPixelVFXNiagara)
	{
		MutableThis->CachedPixelVFXNiagara = PixelVFXNiagara.Get();
	}
	if (!MutableThis->CachedSlashVFXNiagara)
	{
		MutableThis->CachedSlashVFXNiagara = SlashVFXNiagara.Get();
	}
	if (!MutableThis->CachedPixelVFXNiagara && !MutableThis->CachedSlashVFXNiagara)
	{
		MutableThis->PrimeCombatPresentationAssetsAsync();
	}
	return MutableThis->CachedPixelVFXNiagara ? MutableThis->CachedPixelVFXNiagara : MutableThis->CachedSlashVFXNiagara;
}

void UT66CombatComponent::SpawnDeathBurstAtLocation(UWorld* World, const FVector& Location, int32 NumParticles, float BurstRadius)
{
	if (!World)
	{
		return;
	}

	UNiagaraSystem* BloodBurstVFX = FindPixelVFXSystem();
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

void UT66CombatComponent::SpawnLineTargetVFX(const FVector& Start, const FVector& End, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	UNiagaraSystem* VFX = GetActiveVFXSystem();
	if (!World)
	{
		UE_LOG(LogT66Combat, Warning, TEXT("[ATTACK VFX][Legacy] LineTarget spawn skipped: no world."));
		return;
	}
	if (!VFX)
	{
		UE_LOG(
			LogT66Combat,
			Warning,
			TEXT("[ATTACK VFX][Legacy] LineTarget spawn skipped: no active Niagara system. Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f)"),
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

	SpawnLineTargetVFX(Start, End, FLinearColor(1.f, 0.82f, 0.24f, 1.f));
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

bool UT66CombatComponent::TrySpawnBoundIdolImpactVFX(
	const FT66CombatImpactContext& IdolImpactContext,
	const FName IdolID,
	const ET66ItemRarity Rarity,
	const float Radius,
	bool& bOutBindingResolved)
{
	bOutBindingResolved = false;
	FT66CombatVFXBindingData Binding;
	UNiagaraSystem* BoundSystem = nullptr;
	if (!ResolveCombatVFXBinding(
		ET66CombatVFXBindingSourceType::IdolModifier,
		IdolID,
		IdolImpactContext.AttackCategory,
		Binding,
		BoundSystem))
	{
		if (T66IsCombatImpactSourceVerboseEnabled())
		{
			UE_LOG(
				LogT66Combat,
				Display,
				TEXT("CombatVFXIdolImpactBindingLookup SourceType=IdolModifier SourceID=%s ParentSourceID=%s AttackCategory=%s Result=None"),
				*IdolID.ToString(),
				*IdolImpactContext.ParentSourceID.ToString(),
				*UEnum::GetValueAsString(IdolImpactContext.AttackCategory));
		}
		return false;
	}

	bOutBindingResolved = true;
	UWorld* World = GetWorld();
	if (!World || !BoundSystem)
	{
		return false;
	}

	const float BaseVisualRadius = FMath::Max(0.f, Binding.BaseVisualRadius);
	const float VisualScale = FMath::Max(
		0.01f,
		(BaseVisualRadius > KINDA_SMALL_NUMBER)
			? (Radius / BaseVisualRadius) * Binding.VisualScaleMultiplier
			: T66CombatShared::GetIdolRarityVisualScale(Rarity) * Binding.VisualScaleMultiplier);
	const FVector VisualAnchor = IdolImpactContext.bDamageCenterValid
		? IdolImpactContext.DamageCenter
		: IdolImpactContext.ImpactPoint;
	const FVector SpawnLocation = VisualAnchor + FVector(0.f, 0.f, 72.f);
	const FRotator SpawnRotation = IdolImpactContext.Forward.IsNearlyZero()
		? FRotator::ZeroRotator
		: IdolImpactContext.Forward.Rotation();

	UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		BoundSystem,
		SpawnLocation,
		SpawnRotation,
		FVector(VisualScale),
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true);
	if (!Component)
	{
		UE_LOG(
			LogT66Combat,
			Warning,
			TEXT("CombatVFXFallbackPlaceholder Reason=IdolImpactSpawnFailed Binding=%s SourceType=IdolModifier SourceID=%s AttackCategory=%s System=%s"),
			*Binding.BindingID.ToString(),
			*IdolID.ToString(),
			*UEnum::GetValueAsString(IdolImpactContext.AttackCategory),
			*BoundSystem->GetPathName());
		return false;
	}

	Component->SetTranslucentSortPriority(13);
	UE_LOG(
		LogT66Combat,
		Display,
		TEXT("CombatVFXProductionSpawned Binding=%s SourceType=IdolModifier SourceID=%s ParentSourceID=%s AttackCategory=%s System=%s Location=%s VisualAnchor=%s DamageCenter=%s DamageCenterValid=%d ImpactPoint=%s ImpactPointValid=%d Radius=%.2f BaseVisualRadius=%.2f VisualScale=%.3f EffectPacketID=%s"),
		*Binding.BindingID.ToString(),
		*IdolID.ToString(),
		*IdolImpactContext.ParentSourceID.ToString(),
		*UEnum::GetValueAsString(IdolImpactContext.AttackCategory),
		*BoundSystem->GetPathName(),
		*SpawnLocation.ToCompactString(),
		*VisualAnchor.ToCompactString(),
		*IdolImpactContext.DamageCenter.ToCompactString(),
		IdolImpactContext.bDamageCenterValid ? 1 : 0,
		*IdolImpactContext.ImpactPoint.ToCompactString(),
		IdolImpactContext.bImpactPointValid ? 1 : 0,
		Radius,
		BaseVisualRadius,
		VisualScale,
		*Binding.EffectPacketID.ToString());
	return true;
}

void UT66CombatComponent::SpawnWaterIdolImpactPlaceholderVFX(const FT66CombatImpactContext& IdolImpactContext, const float Radius)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UStaticMesh* SphereMesh = FT66VisualUtil::GetBasicShapeSphere();
	if (!SphereMesh)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	constexpr float BasicShapeSphereRadius = 50.f;
	const float VisualRadius = FMath::Max(1.f, Radius);
	const float VisualScale = VisualRadius / BasicShapeSphereRadius;
	const FVector VisualAnchor = IdolImpactContext.bDamageCenterValid
		? IdolImpactContext.DamageCenter
		: IdolImpactContext.ImpactPoint;
	const FVector SpawnLocation = VisualAnchor;
	AActor* PlaceholderActor = World->SpawnActor<AActor>(
		AActor::StaticClass(),
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams);
	if (!PlaceholderActor)
	{
		return;
	}

	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(PlaceholderActor, TEXT("WaterIdolImpactPlaceholderMesh"));
	if (!MeshComponent)
	{
		PlaceholderActor->Destroy();
		return;
	}

	PlaceholderActor->Tags.AddUnique(FName(TEXT("T66CombatVFXWaterIdolImpactPlaceholder")));
	PlaceholderActor->SetRootComponent(MeshComponent);
	MeshComponent->SetRelativeLocation(FVector::ZeroVector);
	MeshComponent->SetStaticMesh(SphereMesh);
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetCastShadow(false);
	MeshComponent->RegisterComponent();
	PlaceholderActor->SetActorLocation(SpawnLocation);
	// Temporary proof area read only; final Water idol art still belongs to Niagara.
	MeshComponent->SetWorldScale3D(FVector(VisualScale));
	FT66VisualUtil::ApplyT66Color(MeshComponent, PlaceholderActor, FLinearColor(0.035f, 0.30f, 1.0f, 1.0f));
	PlaceholderActor->SetLifeSpan(1.25f);

	if (T66IsCombatImpactSourceVerboseEnabled())
	{
		UE_LOG(
			LogT66Combat,
			Display,
			TEXT("CombatVFXIdolImpactPlaceholderSpawned SourceType=IdolModifier SourceID=Idol_Water ParentSourceID=%s DamageCenter=%s DamageCenterValid=%d ImpactPoint=%s ImpactPointValid=%d VisualAnchor=%s VisualLocation=%s Radius=%.2f VisualRadius=%.2f Placeholder=BlueSphereAreaRead VisualScale=%.3f LifeSpan=1.25"),
			*IdolImpactContext.ParentSourceID.ToString(),
			*IdolImpactContext.DamageCenter.ToCompactString(),
			IdolImpactContext.bDamageCenterValid ? 1 : 0,
			*IdolImpactContext.ImpactPoint.ToCompactString(),
			IdolImpactContext.bImpactPointValid ? 1 : 0,
			*VisualAnchor.ToCompactString(),
			*PlaceholderActor->GetActorLocation().ToCompactString(),
			Radius,
			VisualRadius,
			VisualScale);
	}
}

void UT66CombatComponent::SpawnIdolImpactPlaceholderVFX(const FT66CombatImpactContext& IdolImpactContext, const FName IdolID, const ET66ItemRarity Rarity, const ET66AttackCategory Category, const float LingerSeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FLinearColor IdolColor = T66PrimitiveIdolColor(IdolID);
	const FVector Anchor = IdolImpactContext.bDamageCenterValid
		? IdolImpactContext.DamageCenter
		: IdolImpactContext.ImpactPoint;
	const FVector ElevatedAnchor = Anchor + FVector(0.f, 0.f, 18.f);
	const TCHAR* CategoryShape = TEXT("Unknown");

	switch (Category)
	{
	case ET66AttackCategory::AOE:
	{
		const float Radius = FMath::Max(32.f, IdolImpactContext.Radius);
		T66SpawnPrimitiveIdolAOEPlaceholder(World, GetOwner(), IdolID, Rarity, ElevatedAnchor, Radius);
		CategoryShape = TEXT("AreaRead");
		break;
	}
	case ET66AttackCategory::Bounce:
	{
		// Chain read from the official impact point through each downstream link target.
		TArray<FVector> ChainPositions;
		ChainPositions.Add(ElevatedAnchor);
		for (const FT66CombatTargetHandle& Handle : IdolImpactContext.HitTargetHandles)
		{
			if (Handle.IsValid())
			{
				ChainPositions.Add(GetTargetAimPoint(Handle) + FVector(0.f, 0.f, 18.f));
			}
		}
		if (ChainPositions.Num() < 2)
		{
			ChainPositions.Add(ElevatedAnchor + IdolImpactContext.Forward.GetSafeNormal() * 64.f);
		}
		T66SpawnPrimitiveIdolBouncePlaceholder(World, GetOwner(), IdolID, Rarity, ChainPositions);
		CategoryShape = TEXT("BounceChain");
		break;
	}
	case ET66AttackCategory::DOT:
	{
		T66SpawnPrimitiveIdolDOTPlaceholder(World, GetOwner(), IdolID, Rarity, nullptr, ElevatedAnchor, FMath::Max(0.1f, LingerSeconds));
		CategoryShape = TEXT("DotBodyRead");
		break;
	}
	default:
	{
		const float Radius = FMath::Max(32.f, IdolImpactContext.Radius);
		T66SpawnPrimitiveIdolAOEPlaceholder(World, GetOwner(), IdolID, Rarity, ElevatedAnchor, Radius);
		CategoryShape = TEXT("AreaRead");
		break;
	}
	}

	if (T66IsCombatImpactSourceVerboseEnabled())
	{
		UE_LOG(
			LogT66Combat,
			Display,
			TEXT("CombatVFXIdolImpactPlaceholderSpawned SourceType=IdolModifier SourceID=%s ParentSourceID=%s AttackCategory=%s Placeholder=%s DamageCenter=%s DamageCenterValid=%d ImpactPoint=%s ImpactPointValid=%d VisualAnchor=%s LineLength=%.2f Links=%d LingerSeconds=%.2f Color=(%.2f,%.2f,%.2f)"),
			*IdolID.ToString(),
			*IdolImpactContext.ParentSourceID.ToString(),
			*UEnum::GetValueAsString(Category),
			CategoryShape,
			*IdolImpactContext.DamageCenter.ToCompactString(),
			IdolImpactContext.bDamageCenterValid ? 1 : 0,
			*IdolImpactContext.ImpactPoint.ToCompactString(),
			IdolImpactContext.bImpactPointValid ? 1 : 0,
			*Anchor.ToCompactString(),
			IdolImpactContext.LineLength,
			IdolImpactContext.HitTargetHandles.Num(),
			LingerSeconds,
			IdolColor.R,
			IdolColor.G,
			IdolColor.B);
	}
}

void UT66CombatComponent::SpawnIdolAOEVFX(const FName& IdolID, const ET66ItemRarity Rarity, const FVector& Location, const float Radius, const float StartDelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const FLinearColor IdolColor = T66PrimitiveIdolColor(IdolID);
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
	FT66CombatVFXBindingData ProjectileMeshBinding;
	UStaticMesh* ProjectileMesh = nullptr;
	if (T66ResolveIdolProjectileMeshBinding(World, IdolID, ET66AttackCategory::AOE, ProjectileMeshBinding, ProjectileMesh))
	{
		const int32 MeshCount = FMath::Max(1, ProjectileMeshBinding.ProjectileMeshCount) * T66RarityProjectileBodyCount(Rarity);
		const float MeshScale = FMath::Max(0.01f, ProjectileMeshBinding.ProjectileMeshScale)
			* T66RarityProjectileBodyScale(Rarity)
			* FMath::Clamp(Radius / 300.f, 0.75f, 2.4f);
		const float OrbitRadius = MeshCount > 1 ? FMath::Clamp(Radius * 0.22f, 34.f, 110.f) : 0.f;
		bool bSpawnedMesh = false;
		for (int32 MeshIndex = 0; MeshIndex < MeshCount; ++MeshIndex)
		{
			const float Angle = MeshCount > 1
				? (static_cast<float>(MeshIndex) / static_cast<float>(MeshCount)) * UE_TWO_PI
				: 0.f;
			const FVector Offset(
				FMath::Cos(Angle) * OrbitRadius,
				FMath::Sin(Angle) * OrbitRadius,
				58.f);
			const FRotator Rotation(0.f, FMath::RadiansToDegrees(Angle), 0.f);
			if (T66SpawnProjectileMeshBodyActor(
				World,
				GetOwner(),
				FName(TEXT("T66IdolAOEProjectileMeshBody")),
				ProjectileMesh,
				Location + Offset,
				Rotation,
				MeshScale,
				1.15f))
			{
				bSpawnedMesh = true;
			}
		}
		if (bSpawnedMesh)
		{
			UE_LOG(LogT66Combat, Display, TEXT("CombatVFXProjectileMeshBodySpawned Binding=%s SourceID=%s Category=AOE Rarity=%s MeshCount=%d Scale=%.2f Radius=%.2f"),
				*ProjectileMeshBinding.BindingID.ToString(),
				*IdolID.ToString(),
				T66CombatShared::GetItemRarityName(Rarity),
				MeshCount,
				MeshScale,
				Radius);
			return;
		}
	}
	if (CVarT66VFXForcePrimitiveIdolPlaceholders.GetValueOnGameThread() != 0)
	{
		if (T66SpawnPrimitiveIdolAOEPlaceholder(World, GetOwner(), IdolID, Rarity, Location, Radius))
		{
			UE_LOG(LogT66Combat, Display, TEXT("CombatVFXPrimitiveIdolPlaceholderSpawned SourceID=%s Category=AOE Element=%s Rarity=%s Radius=%.2f"),
				*IdolID.ToString(),
				T66PrimitiveElementName(T66ResolvePrimitiveIdolElement(IdolID)),
				T66CombatShared::GetItemRarityName(Rarity),
				Radius);
			return;
		}
	}
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
	const FLinearColor IdolColor = T66PrimitiveIdolColor(IdolID);
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
	FT66CombatVFXBindingData ProjectileMeshBinding;
	UStaticMesh* ProjectileMesh = nullptr;
	if (T66ResolveIdolProjectileMeshBinding(World, IdolID, ET66AttackCategory::Bounce, ProjectileMeshBinding, ProjectileMesh))
	{
		const int32 MeshCount = FMath::Max(1, ProjectileMeshBinding.ProjectileMeshCount) * T66RarityProjectileBodyCount(Rarity);
		const float MeshScale = FMath::Max(0.01f, ProjectileMeshBinding.ProjectileMeshScale)
			* T66RarityProjectileBodyScale(Rarity)
			* FMath::Clamp(VisualScale, 0.75f, 2.2f);
		const float MeshSpeed = ProjectileMeshBinding.ProjectileMeshTravelSpeed > 0.f
			? ProjectileMeshBinding.ProjectileMeshTravelSpeed
			: 2200.f;
		bool bSpawnedMesh = false;
		for (int32 LinkIndex = 0; LinkIndex < ChainPositions.Num() - 1; ++LinkIndex)
		{
			const FVector Start = ChainPositions[LinkIndex] + FVector(0.f, 0.f, 46.f);
			const FVector End = ChainPositions[LinkIndex + 1] + FVector(0.f, 0.f, 46.f);
			const FVector Direction = (End - Start).GetSafeNormal();
			const FVector Side = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
			const float TravelSeconds = FMath::Clamp(FVector::Dist(Start, End) / MeshSpeed, 0.08f, 1.25f);
			for (int32 MeshIndex = 0; MeshIndex < MeshCount; ++MeshIndex)
			{
				const float CenteredIndex = static_cast<float>(MeshIndex) - (static_cast<float>(MeshCount - 1) * 0.5f);
				const FVector Offset = Side * CenteredIndex * 42.f * MeshScale;
				if (AT66HeroProjectile* BounceProjectile = SpawnVisualTravelProjectile(
					Start + Offset,
					End + Offset,
					IdolColor,
					FT66TemporaryProjectileSystem::ProfileHeroBounce(),
					MeshScale,
					TravelSeconds))
				{
					BounceProjectile->ApplyCustomVisualMeshOverride(ProjectileMesh);
					bSpawnedMesh = true;
				}
			}
		}
		if (bSpawnedMesh)
		{
			UE_LOG(LogT66Combat, Display, TEXT("CombatVFXProjectileMeshBodySpawned Binding=%s SourceID=%s Category=Bounce Rarity=%s Links=%d MeshCount=%d Scale=%.2f"),
				*ProjectileMeshBinding.BindingID.ToString(),
				*IdolID.ToString(),
				T66CombatShared::GetItemRarityName(Rarity),
				ChainPositions.Num() - 1,
				MeshCount,
				MeshScale);
			return;
		}
	}
	if (CVarT66VFXForcePrimitiveIdolPlaceholders.GetValueOnGameThread() != 0)
	{
		if (T66SpawnPrimitiveIdolBouncePlaceholder(World, GetOwner(), IdolID, Rarity, ChainPositions))
		{
			UE_LOG(LogT66Combat, Display, TEXT("CombatVFXPrimitiveIdolPlaceholderSpawned SourceID=%s Category=Bounce Element=%s Rarity=%s Links=%d"),
				*IdolID.ToString(),
				T66PrimitiveElementName(T66ResolvePrimitiveIdolElement(IdolID)),
				T66CombatShared::GetItemRarityName(Rarity),
				ChainPositions.Num() - 1);
			return;
		}
	}
	if (const TCHAR* AssetPath = GetIdolNiagaraEffectPath(IdolID))
	{
		TArray<FVector> ElevatedPositions;
		ElevatedPositions.Reserve(ChainPositions.Num());
		for (const FVector& Pos : ChainPositions)
		{
			ElevatedPositions.Add(Pos + FVector(0.f, 0.f, 24.f));
		}
		if (SpawnImportedEffectAlongChain(World, AssetPath, ElevatedPositions, VisualScale, Quantity))
		{
			return;
		}
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
	const FLinearColor IdolColor = T66PrimitiveIdolColor(IdolID);
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
	FT66CombatVFXBindingData ProjectileMeshBinding;
	UStaticMesh* ProjectileMesh = nullptr;
	if (T66ResolveIdolProjectileMeshBinding(World, IdolID, ET66AttackCategory::DOT, ProjectileMeshBinding, ProjectileMesh))
	{
		const int32 MeshCount = FMath::Max(1, ProjectileMeshBinding.ProjectileMeshCount) * T66RarityProjectileBodyCount(Rarity);
		const float MeshScale = FMath::Max(0.01f, ProjectileMeshBinding.ProjectileMeshScale)
			* T66RarityProjectileBodyScale(Rarity)
			* FMath::Clamp(VisualScale, 0.65f, 2.0f);
		const FVector BaseLocation = (FollowTarget ? FollowTarget->GetActorLocation() : Location) + FVector(0.f, 0.f, 66.f);
		const float OrbitRadius = MeshCount > 1 ? 36.f * MeshScale : 0.f;
		bool bSpawnedMesh = false;
		for (int32 MeshIndex = 0; MeshIndex < MeshCount; ++MeshIndex)
		{
			const float Angle = MeshCount > 1
				? (static_cast<float>(MeshIndex) / static_cast<float>(MeshCount)) * UE_TWO_PI
				: 0.f;
			const FVector Offset(FMath::Cos(Angle) * OrbitRadius, FMath::Sin(Angle) * OrbitRadius, 0.f);
			const FRotator Rotation(0.f, FMath::RadiansToDegrees(Angle), 0.f);
			if (T66SpawnProjectileMeshBodyActor(
				World,
				GetOwner(),
				FName(TEXT("T66IdolDOTProjectileMeshBody")),
				ProjectileMesh,
				BaseLocation + Offset,
				Rotation,
				MeshScale,
				FMath::Clamp(Duration, 0.8f, 4.0f),
				FollowTarget))
			{
				bSpawnedMesh = true;
			}
		}
		if (bSpawnedMesh)
		{
			UE_LOG(LogT66Combat, Display, TEXT("CombatVFXProjectileMeshBodySpawned Binding=%s SourceID=%s Category=DOT Rarity=%s Follow=%d MeshCount=%d Scale=%.2f Duration=%.2f"),
				*ProjectileMeshBinding.BindingID.ToString(),
				*IdolID.ToString(),
				T66CombatShared::GetItemRarityName(Rarity),
				FollowTarget ? 1 : 0,
				MeshCount,
				MeshScale,
				Duration);
			return;
		}
	}
	if (CVarT66VFXForcePrimitiveIdolPlaceholders.GetValueOnGameThread() != 0)
	{
		if (T66SpawnPrimitiveIdolDOTPlaceholder(World, GetOwner(), IdolID, Rarity, FollowTarget, Location, Duration))
		{
			UE_LOG(LogT66Combat, Display, TEXT("CombatVFXPrimitiveIdolPlaceholderSpawned SourceID=%s Category=DOT Element=%s Rarity=%s Follow=%d Duration=%.2f"),
				*IdolID.ToString(),
				T66PrimitiveElementName(T66ResolvePrimitiveIdolElement(IdolID)),
				T66CombatShared::GetItemRarityName(Rarity),
				FollowTarget ? 1 : 0,
				Duration);
			return;
		}
	}
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
