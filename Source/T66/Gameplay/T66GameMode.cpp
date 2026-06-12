// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66GameMode.h"
#include "Gameplay/GameMode/T66GameModePrivate.h"

DEFINE_LOG_CATEGORY(LogT66GameMode);

namespace T66GameModePrivate
{

constexpr int32 T66LastCompanionUnlockIndex = 12;

// Helper: avoid PIE warnings ("StaticMeshComponent has to be 'Movable' if you'd like to move")
// by temporarily setting mobility to Movable while we apply transforms.
void T66_SetStaticMeshActorMobility(AStaticMeshActor* Actor, EComponentMobility::Type Mobility)
{
if (!Actor) return;
if (UStaticMeshComponent* SMC = Actor->GetStaticMeshComponent())
{
if (SMC->Mobility != Mobility)
{
SMC->SetMobility(Mobility);
}
}
}

bool T66_IsCompanionUnlockStage(int32 StageNum)
{
return T66_CompanionBaseIndexForStage(StageNum) != INDEX_NONE;
}

bool T66_IsDifficultyBossStage(int32 StageNum)
{
return StageNum == 4 || StageNum == 8 || StageNum == 12 || StageNum == 16 || StageNum == 20;
}

int32 T66_CompanionBaseIndexForStage(int32 StageNum)
{
switch (StageNum)
{
case 1: return 1;
case 2: return 2;
case 3: return 3;
case 5: return 4;
case 6: return 5;
case 7: return 6;
case 9: return 7;
case 10: return 8;
case 11: return 9;
case 13: return 10;
case 14: return 11;
case 15: return 12;
default: return INDEX_NONE;
}
}

void T66_AppendCompanionUnlockIDsForStage(const int32 StageNum, TArray<FName>& OutCompanionIDs)
{
const int32 BaseIndex = T66_CompanionBaseIndexForStage(StageNum);
if (BaseIndex == INDEX_NONE)
{
return;
}

auto AddCompanionByIndex = [&OutCompanionIDs](const int32 Index)
{
if (Index <= 0 || Index > T66LastCompanionUnlockIndex)
{
return;
}

OutCompanionIDs.AddUnique(FName(*FString::Printf(TEXT("Companion_%02d"), Index)));
};

AddCompanionByIndex(BaseIndex);
}

int32 T66ResolveFallbackBossStageNum(const FName BossID, const int32 DefaultStageNum)
{
const int32 ClampedDefaultStage = FMath::Clamp(DefaultStageNum, 1, T66MaxGlobalStage);
const FString BossName = BossID.ToString();
int32 SeparatorIndex = INDEX_NONE;
if (!BossName.FindLastChar(TEXT('_'), SeparatorIndex))
{
return ClampedDefaultStage;
}

const FString Suffix = BossName.Mid(SeparatorIndex + 1);
if (Suffix.IsEmpty())
{
return ClampedDefaultStage;
}

for (const TCHAR Char : Suffix)
{
if (!FChar::IsDigit(Char))
{
return ClampedDefaultStage;
}
}

return FMath::Clamp(FCString::Atoi(*Suffix), 1, T66MaxGlobalStage);
}

void T66BuildFallbackBossData(const int32 StageNum, const FName BossID, FBossData& OutBossData)
{
const int32 S = FMath::Clamp(StageNum, 1, T66MaxGlobalStage);
const float T = static_cast<float>(S - 1) / static_cast<float>(T66MaxGlobalStage - 1); // 0..1

OutBossData = FBossData{};
OutBossData.BossID = BossID.IsNone()
? FName(*FString::Printf(TEXT("Boss_%02d"), S))
: BossID;
OutBossData.MaxHP = 1000 + (S * 250);
OutBossData.AwakenDistance = 900.f;
OutBossData.MoveSpeed = 350.f + (S * 2.f);
OutBossData.FireIntervalSeconds = FMath::Clamp(2.0f - (S * 0.015f), 0.65f, 3.5f);
OutBossData.ProjectileSpeed = 900.f + (S * 15.f);
OutBossData.ProjectileDamageHearts = 1 + (S / 20);

const float Hue = FMath::Fmod(static_cast<float>(S) * 31.f, 360.f);
FLinearColor Color = FLinearColor::MakeFromHSV8(static_cast<uint8>(Hue / 360.f * 255.f), 210, 245);
Color.A = 1.f;
Color.R = FMath::Lerp(Color.R * 0.85f, Color.R, T);
Color.G = FMath::Lerp(Color.G * 0.85f, Color.G, T);
Color.B = FMath::Lerp(Color.B * 0.85f, Color.B, T);
OutBossData.PlaceholderColor = Color;
}

UClass* T66LoadBossClassSync(const FBossData& BossData)
{
UClass* BossClass = AT66BossBase::StaticClass();
if (!BossData.BossClass.IsNull())
{
if (UClass* LoadedClass = BossData.BossClass.LoadSynchronous())
{
if (LoadedClass->IsChildOf(AT66BossBase::StaticClass()))
{
BossClass = LoadedClass;
}
}
}
return BossClass;
}

FVector T66ComputeBossClusterLocation(const FVector& Center, const int32 BossIndex, const int32 BossCount)
{
if (BossCount <= 0)
{
return Center;
}

const float Radius = 760.f + (90.f * static_cast<float>(FMath::Max(0, BossCount - 1)));
const float Angle = (BossCount == 1)
? 0.f
: (2.f * PI * static_cast<float>(BossIndex) / static_cast<float>(BossCount));
return Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
}

bool T66_HasAnyFloorTag(const AActor* A)
{
if (!A) return false;
// Covers: T66_Floor_Main/Start/Boss/Conn*, plus tutorial helper floors.
static const FName Prefix(TEXT("T66_Floor"));
for (const FName& Tag : A->Tags)
{
const FString S = Tag.ToString();
if (S.StartsWith(Prefix.ToString()))
{
return true;
}
}
return false;
}

const FName T66MainMapTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
const FName T66MainMapTerrainMaterialsReadyTag(TEXT("T66_MainMapTerrain_MaterialsReady"));
const FName T66TowerCeilingTag(TEXT("T66_Tower_Ceiling"));
const FName T66TowerTraceBarrierTag(TEXT("T66_Map_TraversalBarrier"));
const TCHAR* T66TowerFloorTagPrefix = TEXT("T66_TowerFloor_");

struct FT66PlayerStartCache
{
TWeakObjectPtr<UWorld> World;
TArray<TWeakObjectPtr<APlayerStart>> Starts;
bool bScanned = false;
};

struct FT66TaggedActorCache
{
TWeakObjectPtr<UWorld> World;
TWeakObjectPtr<AActor> MainMapTerrainVisualActor;
TMap<FName, TWeakObjectPtr<AActor>> ActorsByTag;
};

static FT66PlayerStartCache GT66PlayerStartCache;
static FT66TaggedActorCache GT66TaggedActorCache;

void T66ResetTaggedActorCache(UWorld* World)
{
GT66TaggedActorCache.World = World;
GT66TaggedActorCache.MainMapTerrainVisualActor.Reset();
GT66TaggedActorCache.ActorsByTag.Reset();
}

void T66InvalidatePlayerStartCache(UWorld* World)
{
if (!World || GT66PlayerStartCache.World.Get() == World)
{
			GT66PlayerStartCache.World = World;
			GT66PlayerStartCache.Starts.Reset();
			GT66PlayerStartCache.bScanned = false;
		}
	}

	void T66RefreshPlayerStartCache(UWorld* World, const bool bForceRefresh)
	{
		if (!World)
		{
			GT66PlayerStartCache.World.Reset();
			GT66PlayerStartCache.Starts.Reset();
			GT66PlayerStartCache.bScanned = false;
			return;
		}

		if (!bForceRefresh
			&& GT66PlayerStartCache.World.Get() == World
			&& GT66PlayerStartCache.bScanned)
		{
			GT66PlayerStartCache.Starts.RemoveAllSwap(
				[](const TWeakObjectPtr<APlayerStart>& WeakStart)
				{
					return !WeakStart.IsValid();
				},
				EAllowShrinking::No);
			return;
		}

		GT66PlayerStartCache.World = World;
		GT66PlayerStartCache.Starts.Reset();
		GT66PlayerStartCache.bScanned = true;

		// Startup cache fill only. Later spawn/trace code reads GT66PlayerStartCache
		// instead of walking the world again.
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			if (APlayerStart* PlayerStart = *It)
			{
				GT66PlayerStartCache.Starts.Add(PlayerStart);
			}
		}
	}

	void T66AddCachedPlayerStartsToTraceIgnore(UWorld* World, FCollisionQueryParams& Params)
	{
		T66RefreshPlayerStartCache(World);
		for (const TWeakObjectPtr<APlayerStart>& WeakPlayerStart : GT66PlayerStartCache.Starts)
		{
			if (APlayerStart* PlayerStart = WeakPlayerStart.Get())
			{
				Params.AddIgnoredActor(PlayerStart);
			}
		}
	}

	bool T66HasAnyCachedPlayerStarts(UWorld* World)
	{
		T66RefreshPlayerStartCache(World);
		return GT66PlayerStartCache.Starts.Num() > 0;
	}

	void T66DestroyCachedPlayerStarts(UWorld* World)
	{
		T66RefreshPlayerStartCache(World);
		for (const TWeakObjectPtr<APlayerStart>& WeakPlayerStart : GT66PlayerStartCache.Starts)
		{
			if (APlayerStart* PlayerStart = WeakPlayerStart.Get())
			{
				PlayerStart->Destroy();
			}
		}

		T66InvalidatePlayerStartCache(World);
	}

	AActor* T66FindMainMapTerrainVisualActor(UWorld* World)
	{
		if (!World)
		{
			T66ResetTaggedActorCache(nullptr);
			return nullptr;
		}

		if (GT66TaggedActorCache.World.Get() != World)
		{
			T66ResetTaggedActorCache(World);
		}

		if (AActor* CachedActor = GT66TaggedActorCache.MainMapTerrainVisualActor.Get())
		{
			return CachedActor;
		}

		if (TWeakObjectPtr<AActor>* CachedByTag = GT66TaggedActorCache.ActorsByTag.Find(T66MainMapTerrainVisualTag))
		{
			if (AActor* Actor = CachedByTag->Get())
			{
				GT66TaggedActorCache.MainMapTerrainVisualActor = Actor;
				return Actor;
			}

			GT66TaggedActorCache.ActorsByTag.Remove(T66MainMapTerrainVisualTag);
		}

		// Cache-miss fallback for legacy/authored terrain markers.
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->ActorHasTag(T66MainMapTerrainVisualTag))
			{
				GT66TaggedActorCache.MainMapTerrainVisualActor = Actor;
				GT66TaggedActorCache.ActorsByTag.Add(T66MainMapTerrainVisualTag, Actor);
				return Actor;
			}
		}

		return nullptr;
	}

	AActor* T66FindTaggedActor(UWorld* World, const FName Tag)
	{
		if (!World)
		{
			T66ResetTaggedActorCache(nullptr);
			return nullptr;
		}

		if (Tag.IsNone())
		{
			return nullptr;
		}

		if (Tag == T66MainMapTerrainVisualTag)
		{
			return T66FindMainMapTerrainVisualActor(World);
		}

		if (GT66TaggedActorCache.World.Get() != World)
		{
			T66ResetTaggedActorCache(World);
		}

		if (TWeakObjectPtr<AActor>* CachedActor = GT66TaggedActorCache.ActorsByTag.Find(Tag))
		{
			if (AActor* Actor = CachedActor->Get())
			{
				return Actor;
			}

			GT66TaggedActorCache.ActorsByTag.Remove(Tag);
		}

		// Cache-miss fallback for stage setup tags. Spawners call
		// T66RememberTaggedActor once they create new runtime actors.
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->ActorHasTag(Tag))
			{
				GT66TaggedActorCache.ActorsByTag.Add(Tag, Actor);
				return Actor;
			}
		}

		return nullptr;
	}

	void T66RememberTaggedActor(AActor* Actor, const FName Tag)
	{
		if (!Actor || Tag.IsNone())
		{
			return;
		}

		UWorld* World = Actor->GetWorld();
		if (!World)
		{
			return;
		}

		if (GT66TaggedActorCache.World.Get() != World)
		{
			T66ResetTaggedActorCache(World);
		}

		GT66TaggedActorCache.ActorsByTag.Add(Tag, Actor);
		if (Tag == T66MainMapTerrainVisualTag)
		{
			GT66TaggedActorCache.MainMapTerrainVisualActor = Actor;
		}
	}

	void T66ForgetTaggedActor(UWorld* World, const FName Tag)
	{
		if (!World || Tag.IsNone() || GT66TaggedActorCache.World.Get() != World)
		{
			return;
		}

		GT66TaggedActorCache.ActorsByTag.Remove(Tag);
		if (Tag == T66MainMapTerrainVisualTag)
		{
			GT66TaggedActorCache.MainMapTerrainVisualActor.Reset();
		}
	}

	bool T66HasRegisteredCasinoNPC(UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			for (const TWeakObjectPtr<AT66NPCBase>& WeakNPC : Registry->GetNPCs())
			{
				if (Cast<AT66CasinoNPC>(WeakNPC.Get()))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool T66ShouldIgnoreTowerCeilingHit(const FHitResult& Hit)
	{
		const AActor* HitActor = Hit.GetActor();
		return HitActor
			&& (HitActor->ActorHasTag(T66TowerCeilingTag) || HitActor->ActorHasTag(T66TowerTraceBarrierTag));
	}

	FName T66MakeTowerFloorTag(const int32 FloorNumber)
	{
		return FName(*FString::Printf(TEXT("%s%02d"), T66TowerFloorTagPrefix, FloorNumber));
	}

	int32 T66ReadTowerFloorTag(const AActor* Actor)
	{
		if (!Actor)
		{
			return INDEX_NONE;
		}

		for (const FName& Tag : Actor->Tags)
		{
			const FString TagString = Tag.ToString();
			if (!TagString.StartsWith(T66TowerFloorTagPrefix))
			{
				continue;
			}

			const FString NumberString = TagString.RightChop(FCString::Strlen(T66TowerFloorTagPrefix));
			return FCString::Atoi(*NumberString);
		}

		return INDEX_NONE;
	}

	void T66AssignTowerFloorTag(AActor* Actor, const int32 FloorNumber)
	{
		if (!Actor || FloorNumber == INDEX_NONE)
		{
			return;
		}

		for (int32 Index = Actor->Tags.Num() - 1; Index >= 0; --Index)
		{
			if (Actor->Tags[Index].ToString().StartsWith(T66TowerFloorTagPrefix))
			{
				Actor->Tags.RemoveAt(Index);
			}
		}

		Actor->Tags.AddUnique(T66MakeTowerFloorTag(FloorNumber));
	}

	const T66TowerMapTerrain::FFloor* T66FindTowerFloorByNumber(const T66TowerMapTerrain::FLayout& Layout, const int32 FloorNumber)
	{
		for (const T66TowerMapTerrain::FFloor& Floor : Layout.Floors)
		{
			if (Floor.FloorNumber == FloorNumber)
			{
				return &Floor;
			}
		}

		return nullptr;
	}

	bool T66ShouldLogTowerGatePlacement(const AActor* Actor)
	{
		return Actor && Actor->IsA(AT66StageGate::StaticClass());
	}

	bool T66TrySnapActorToTowerFloor(UWorld* World, AActor* Actor, const T66TowerMapTerrain::FLayout& Layout, const int32 FloorNumber, const FVector& DesiredLocation)
	{
		if (!World || !Actor)
		{
			return false;
		}

		const T66TowerMapTerrain::FFloor* Floor = T66FindTowerFloorByNumber(Layout, FloorNumber);
		if (!Floor)
		{
			return false;
		}

		const float TraceUp = FMath::Max(700.0f, Layout.FloorSpacing * 0.45f);
		const float TraceDown = FMath::Max(1000.0f, Layout.FloorThickness + 1200.0f);
		const FVector TraceStart(DesiredLocation.X, DesiredLocation.Y, Floor->SurfaceZ + TraceUp);
		const FVector TraceEnd(DesiredLocation.X, DesiredLocation.Y, Floor->SurfaceZ - TraceDown);
		const bool bLogGatePlacement = T66ShouldLogTowerGatePlacement(Actor);

		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66SnapActorToTowerFloor), false, Actor);
		T66AddCachedPlayerStartsToTraceIgnore(World, Params);

		FHitResult GroundHit;
		bool bFoundGround = false;
		TArray<FHitResult> Hits;
		if (World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, ECC_WorldStatic, Params))
		{
			const float SurfaceTolerance = FMath::Max(120.0f, (Layout.FloorThickness * 0.5f) + 20.0f);
			for (const FHitResult& Hit : Hits)
			{
				if (T66ShouldIgnoreTowerCeilingHit(Hit))
				{
					continue;
				}

				if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
				{
					continue;
				}

				if (FMath::Abs(Hit.ImpactPoint.Z - Floor->SurfaceZ) > SurfaceTolerance)
				{
					continue;
				}

				GroundHit = Hit;
				bFoundGround = true;
				break;
			}
		}

		float HalfHeight = 0.0f;
		if (const UCapsuleComponent* Capsule = Actor->FindComponentByClass<UCapsuleComponent>())
		{
			HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		const float GroundZ = bFoundGround ? GroundHit.ImpactPoint.Z : Floor->SurfaceZ;
		if (bLogGatePlacement)
		{
			UE_LOG(
				LogT66GameMode,
				Warning,
				TEXT("Tower gate snap: actor=%s class=%s floor=%d desired=(%.0f, %.0f, %.0f) floorCenter=(%.0f, %.0f, %.0f) floorSurfaceZ=%.0f traceStart=(%.0f, %.0f, %.0f) traceEnd=(%.0f, %.0f, %.0f) hits=%d foundGround=%d hitActor=%s hitPoint=(%.0f, %.0f, %.0f) halfHeight=%.0f finalZ=%.0f"),
				*Actor->GetName(),
				*Actor->GetClass()->GetName(),
				FloorNumber,
				DesiredLocation.X,
				DesiredLocation.Y,
				DesiredLocation.Z,
				Floor->Center.X,
				Floor->Center.Y,
				Floor->Center.Z,
				Floor->SurfaceZ,
				TraceStart.X,
				TraceStart.Y,
				TraceStart.Z,
				TraceEnd.X,
				TraceEnd.Y,
				TraceEnd.Z,
				Hits.Num(),
				bFoundGround ? 1 : 0,
				bFoundGround && GroundHit.GetActor() ? *GroundHit.GetActor()->GetName() : TEXT("None"),
				bFoundGround ? GroundHit.ImpactPoint.X : DesiredLocation.X,
				bFoundGround ? GroundHit.ImpactPoint.Y : DesiredLocation.Y,
				bFoundGround ? GroundHit.ImpactPoint.Z : Floor->SurfaceZ,
				HalfHeight,
				GroundZ + HalfHeight + 5.0f);
		}

		Actor->SetActorLocation(
			FVector(DesiredLocation.X, DesiredLocation.Y, GroundZ + HalfHeight + 5.0f),
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
		return true;
	}

	bool T66AreMainMapTerrainMaterialsReady(UWorld* World)
	{
		if (AActor* TerrainVisualActor = T66FindMainMapTerrainVisualActor(World))
		{
			return TerrainVisualActor->ActorHasTag(T66MainMapTerrainMaterialsReadyTag);
		}

		return false;
	}

	bool T66UsesMainMapTerrainStage(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance()))
		{
			return !T66GI->IsTutorialRun() && !T66GI->IsLabRun() && !T66GI->IsTestRoomRun();
		}

		return true;
	}

	void T66DestroyMiasmaBoundaryActors(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			bool bDestroyedAnyBoundary = false;
			for (const TWeakObjectPtr<AT66MiasmaBoundary>& WeakBoundary : Registry->GetMiasmaBoundaries())
			{
				if (AT66MiasmaBoundary* ExistingBoundary = WeakBoundary.Get())
				{
					ExistingBoundary->Destroy();
					bDestroyedAnyBoundary = true;
				}
			}

			if (bDestroyedAnyBoundary)
			{
				return;
			}
		}

		// Registry is authoritative for runtime boundaries. The iterator is only
		// a teardown fallback for old maps or actors created before registration.
		for (TActorIterator<AT66MiasmaBoundary> It(World); It; ++It)
		{
			if (AT66MiasmaBoundary* ExistingBoundary = *It)
			{
				ExistingBoundary->Destroy();
			}
		}
	}

	bool T66HasRegisteredMiasmaBoundary(UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			for (const TWeakObjectPtr<AT66MiasmaBoundary>& WeakBoundary : Registry->GetMiasmaBoundaries())
			{
				if (WeakBoundary.IsValid())
				{
					return true;
				}
			}
		}

		return false;
	}

	AT66MiasmaBoundary* T66EnsureMiasmaBoundaryActor(UWorld* World, const FActorSpawnParameters& SpawnParams)
	{
		if (!World || T66HasRegisteredMiasmaBoundary(World))
		{
			return nullptr;
		}

		return World->SpawnActor<AT66MiasmaBoundary>(
			AT66MiasmaBoundary::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams);
	}

	void T66PauseTowerMiasma(AT66MiasmaManager* MiasmaManager)
	{
		if (!MiasmaManager)
		{
			return;
		}

		MiasmaManager->ClearAllMiasma();
		MiasmaManager->SetActorTickEnabled(false);
		MiasmaManager->SetActorHiddenInGame(true);
	}

	void T66ActivateStageMiasma(AT66MiasmaManager* MiasmaManager)
	{
		if (!MiasmaManager)
		{
			return;
		}

		MiasmaManager->SetActorHiddenInGame(false);
		MiasmaManager->SetActorTickEnabled(true);
	}


	int32 T66EnsureRunSeed(UT66GameInstance* GI)
	{
		if (!GI)
		{
			return FMath::Rand();
		}

		if (GI->RunSeed == 0)
		{
			GI->RunSeed = FMath::Rand();
		}

		return GI->RunSeed;
	}

	FT66MapPreset T66BuildMainMapPreset(UT66GameInstance* GI)
	{
		const ET66Difficulty Difficulty = GI ? GI->SelectedDifficulty : ET66Difficulty::Easy;
		// Geometry/layout stays stable for the current difficulty + run seed. Local stage progression
		// should come from stage tuning, enemy rosters, and interactable/trap rolls rather than a new tower shell.
		return T66MainMapTerrain::BuildPresetForDifficulty(Difficulty, T66EnsureRunSeed(GI));
	}

	const TCHAR* T66GetMainMapLayoutVariantLabel()
	{
		return TEXT("Tower");
	}

	const AT66SessionPlayerState* T66GetSessionPlayerState(const AController* Controller)
	{
		return Controller ? Controller->GetPlayerState<AT66SessionPlayerState>() : nullptr;
	}

	FName T66GetSelectedHeroID(const UT66GameInstance* GI, const AController* Controller)
	{
		if (const AT66SessionPlayerState* SessionPlayerState = T66GetSessionPlayerState(Controller))
		{
			if (!SessionPlayerState->GetSelectedHeroID().IsNone())
			{
				return GI
					? const_cast<UT66GameInstance*>(GI)->ResolvePlayableHeroID(SessionPlayerState->GetSelectedHeroID())
					: SessionPlayerState->GetSelectedHeroID();
			}
		}

		return GI
			? const_cast<UT66GameInstance*>(GI)->ResolvePlayableHeroID(GI->SelectedHeroID)
			: FName(TEXT("Hero_1"));
	}

	FName T66GetSelectedCompanionID(const UT66GameInstance* GI, const AController* Controller)
	{
		if (const AT66SessionPlayerState* SessionPlayerState = T66GetSessionPlayerState(Controller))
		{
			const FName SessionCompanionID = SessionPlayerState->GetSelectedCompanionID();
			if (!SessionCompanionID.IsNone())
			{
				return SessionCompanionID;
			}

			// Local gameplay entry can spawn the companion before the fresh
			// player state receives the lobby profile. Remote per-player None
			// remains None so host GI state is not reused for another player.
			if (Controller && !Controller->IsLocalController())
			{
				return NAME_None;
			}

			const FName GameInstanceCompanionID = GI ? GI->SelectedCompanionID : NAME_None;
			UE_LOG(
				LogT66GameMode,
				Log,
				TEXT("Companion spawn using local game-instance fallback because session companion is None. GICompanion=%s"),
				*GameInstanceCompanionID.ToString());
			return GameInstanceCompanionID;
		}

		const FName GameInstanceCompanionID = GI ? GI->SelectedCompanionID : NAME_None;
		UE_LOG(
			LogT66GameMode,
			Log,
			TEXT("Companion spawn using game-instance fallback because no session player state. GICompanion=%s"),
			*GameInstanceCompanionID.ToString());
		return GameInstanceCompanionID;
	}

	ET66BodyType T66GetSelectedHeroBodyType(const UT66GameInstance* GI, const AController* Controller)
	{
		if (const AT66SessionPlayerState* SessionPlayerState = T66GetSessionPlayerState(Controller))
		{
			return SessionPlayerState->GetSelectedHeroBodyType();
		}

		return GI ? GI->SelectedHeroBodyType : ET66BodyType::Chad;
	}

	FName T66GetSelectedHeroSkinID(const UT66GameInstance* GI, const AController* Controller)
	{
		if (const AT66SessionPlayerState* SessionPlayerState = T66GetSessionPlayerState(Controller))
		{
			const FName SkinID = SessionPlayerState->GetSelectedHeroSkinID();
			if (!SkinID.IsNone())
			{
				return SkinID;
			}
		}

		if (GI && !GI->SelectedHeroSkinID.IsNone())
		{
			return GI->SelectedHeroSkinID;
		}

		return FName(TEXT("Default"));
	}

	int32 T66GetConnectedPlayerCount(const UWorld* World)
	{
		if (!World)
		{
			return 1;
		}

		int32 Count = 0;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (It->Get())
			{
				++Count;
			}
		}

		return FMath::Max(1, Count);
	}

	int32 T66GetPlayerSlotIndex(const UWorld* World, const AController* Controller)
	{
		if (!World || !Controller)
		{
			return 0;
		}

		int32 Index = 0;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (It->Get() == Controller)
			{
				return Index;
			}

			++Index;
		}

		return 0;
	}

	bool T66IsTutorialRun(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance());
		return T66GI && T66GI->IsTutorialRun();
	}

	bool T66TryGetTaggedActorTransform(const UWorld* World, const FName Tag, FVector& OutLocation, FRotator& OutRotation)
	{
		if (!World || Tag.IsNone())
		{
			return false;
		}

		if (AActor* Actor = T66FindTaggedActor(const_cast<UWorld*>(World), Tag))
		{
			OutLocation = Actor->GetActorLocation();
			OutRotation = Actor->GetActorRotation();
			return true;
		}

		return false;
	}

	void T66FaceActorTowardLocation2D(AActor* Actor, const FVector& TargetLocation)
	{
		if (!Actor)
		{
			return;
		}

		FVector FacingDirection = TargetLocation - Actor->GetActorLocation();
		FacingDirection.Z = 0.f;
		if (FacingDirection.IsNearlyZero())
		{
			return;
		}

		Actor->SetActorRotation(FacingDirection.Rotation());
	}

	bool T66TryBuildFacingRotation2D(const FVector& FromLocation, const FVector& TargetLocation, FRotator& OutRotation)
	{
		FVector FacingDirection = TargetLocation - FromLocation;
		FacingDirection.Z = 0.f;
		if (FacingDirection.IsNearlyZero())
		{
			return false;
		}

		OutRotation = FacingDirection.Rotation();
		OutRotation.Pitch = 0.f;
		OutRotation.Roll = 0.f;
		return true;
	}

	void T66SyncPawnAndControllerRotation(APawn* Pawn, AController* Controller, const FRotator& DesiredRotation)
	{
		if (!Pawn)
		{
			return;
		}

		FRotator FlatRotation = DesiredRotation;
		FlatRotation.Pitch = 0.f;
		FlatRotation.Roll = 0.f;
		Pawn->SetActorRotation(FlatRotation, ETeleportType::TeleportPhysics);

		if (!Controller)
		{
			return;
		}

		Controller->SetControlRotation(FlatRotation);
		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			PlayerController->ClientSetRotation(FlatRotation, true);
		}
	}

}

using namespace T66GameModePrivate;

AT66GameMode::AT66GameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	// Default to our hero base class
	DefaultPawnClass = AT66HeroBase::StaticClass();
	DefaultHeroClass = AT66HeroBase::StaticClass();
	PlayerStateClass = AT66SessionPlayerState::StaticClass();
	bUseSeamlessTravel = true;

	FT66TerrainThemeAssets::FillDefaultCliffSideMaterials(CliffSideMaterials);

	PixalTestMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALTEST/SM_PIXALTEST.SM_PIXALTEST")));
	PixalTestTexture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALTEST/T_PIXALTEST.T_PIXALTEST")));
	PixalTest2Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALTEST2/SM_PIXALTEST2.SM_PIXALTEST2")));
	PixalTest2Texture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALTEST2/T_PIXALTEST2.T_PIXALTEST2")));
	PixalSlimeMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALSLIME/SM_PIXALSLIME.SM_PIXALSLIME")));
	PixalSlimeTexture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALSLIME/T_PIXALSLIME.T_PIXALSLIME")));
	PixalSlimeHifiMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALSLIME_HIFIRUSH/SM_PIXALSLIME_HIFIRUSH.SM_PIXALSLIME_HIFIRUSH")));
	PixalSlimeHifiTexture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALSLIME_HIFIRUSH/T_PIXALSLIME_HIFIRUSH.T_PIXALSLIME_HIFIRUSH")));
	PixalStandaloneTestMeshes = {
		TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALTEST3/SM_PIXALTEST3.SM_PIXALTEST3"))),
		TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALTEST4/SM_PIXALTEST4.SM_PIXALTEST4"))),
		TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALTEST5/SM_PIXALTEST5.SM_PIXALTEST5"))),
	};
	PixalStandaloneTestTextures = {
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALTEST3/T_PIXALTEST3.T_PIXALTEST3"))),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALTEST4/T_PIXALTEST4.T_PIXALTEST4"))),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALTEST5/T_PIXALTEST5.T_PIXALTEST5"))),
	};
	PixalEasyDungeonHifiMeshes = {
		TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALBONEWALKER_HIFIRUSH/SM_PIXALBONEWALKER_HIFIRUSH.SM_PIXALBONEWALKER_HIFIRUSH"))),
		TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALRATPACK_HIFIRUSH/SM_PIXALRATPACK_HIFIRUSH.SM_PIXALRATPACK_HIFIRUSH"))),
		TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALCAVEBAT_HIFIRUSH/SM_PIXALCAVEBAT_HIFIRUSH.SM_PIXALCAVEBAT_HIFIRUSH"))),
		TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALHEXSLINGER_HIFIRUSH/SM_PIXALHEXSLINGER_HIFIRUSH.SM_PIXALHEXSLINGER_HIFIRUSH"))),
		TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALTOMBSPIDER_HIFIRUSH/SM_PIXALTOMBSPIDER_HIFIRUSH.SM_PIXALTOMBSPIDER_HIFIRUSH"))),
		TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALSTONESENTINEL_HIFIRUSH/SM_PIXALSTONESENTINEL_HIFIRUSH.SM_PIXALSTONESENTINEL_HIFIRUSH"))),
		TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALMIMICLURE_HIFIRUSH/SM_PIXALMIMICLURE_HIFIRUSH.SM_PIXALMIMICLURE_HIFIRUSH"))),
		TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALBONECONJURER_HIFIRUSH/SM_PIXALBONECONJURER_HIFIRUSH.SM_PIXALBONECONJURER_HIFIRUSH"))),
		TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALCRYPTWRAITH_HIFIRUSH/SM_PIXALCRYPTWRAITH_HIFIRUSH.SM_PIXALCRYPTWRAITH_HIFIRUSH"))),
	};
	PixalEasyDungeonHifiTextures = {
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALBONEWALKER_HIFIRUSH/T_PIXALBONEWALKER_HIFIRUSH.T_PIXALBONEWALKER_HIFIRUSH"))),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALRATPACK_HIFIRUSH/T_PIXALRATPACK_HIFIRUSH.T_PIXALRATPACK_HIFIRUSH"))),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALCAVEBAT_HIFIRUSH/T_PIXALCAVEBAT_HIFIRUSH.T_PIXALCAVEBAT_HIFIRUSH"))),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALHEXSLINGER_HIFIRUSH/T_PIXALHEXSLINGER_HIFIRUSH.T_PIXALHEXSLINGER_HIFIRUSH"))),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALTOMBSPIDER_HIFIRUSH/T_PIXALTOMBSPIDER_HIFIRUSH.T_PIXALTOMBSPIDER_HIFIRUSH"))),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALSTONESENTINEL_HIFIRUSH/T_PIXALSTONESENTINEL_HIFIRUSH.T_PIXALSTONESENTINEL_HIFIRUSH"))),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALMIMICLURE_HIFIRUSH/T_PIXALMIMICLURE_HIFIRUSH.T_PIXALMIMICLURE_HIFIRUSH"))),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALBONECONJURER_HIFIRUSH/T_PIXALBONECONJURER_HIFIRUSH.T_PIXALBONECONJURER_HIFIRUSH"))),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Characters/Mobs/PIXALCRYPTWRAITH_HIFIRUSH/T_PIXALCRYPTWRAITH_HIFIRUSH.T_PIXALCRYPTWRAITH_HIFIRUSH"))),
	};
	PixalTestSharedMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Materials/MI_GLB_Unlit_Character_Shared.MI_GLB_Unlit_Character_Shared")));
}

AT66EnemyDirector* AT66GameMode::FindOrCacheEnemyDirector(UWorld* World)
{
	if (AT66EnemyDirector* CachedEnemyDirector = EnemyDirector.Get())
	{
		return CachedEnemyDirector;
	}

	if (!World)
	{
		return nullptr;
	}

	// One-shot fallback for legacy/authored maps. Runtime callers should use the
	// cached GameMode seam instead of scanning their own world state.
	for (TActorIterator<AT66EnemyDirector> It(World); It; ++It)
	{
		if (AT66EnemyDirector* ExistingDirector = *It)
		{
			EnemyDirector = ExistingDirector;
			return ExistingDirector;
		}
	}

	return nullptr;
}

void AT66GameMode::SetEnemyDirectorSpawningPaused(const bool bPaused)
{
	if (AT66EnemyDirector* ExistingDirector = FindOrCacheEnemyDirector(GetWorld()))
	{
		ExistingDirector->SetSpawningPaused(bPaused);
	}
}

AT66EnemyDirector* AT66GameMode::GetEnemyDirectorForDiagnostics()
{
	return FindOrCacheEnemyDirector(GetWorld());
}

AT66EnemyDirector* AT66GameMode::EnsureEnemyDirector(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	if (AT66EnemyDirector* ExistingDirector = FindOrCacheEnemyDirector(World))
	{
		return ExistingDirector;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	EnemyDirector = World->SpawnActor<AT66EnemyDirector>(
		AT66EnemyDirector::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams);
	return EnemyDirector.Get();
}

void AT66GameMode::DestroyEnemyDirectors(UWorld* World)
{
	TArray<AT66EnemyDirector*> DirectorsToDestroy;
	if (AT66EnemyDirector* CachedEnemyDirector = EnemyDirector.Get())
	{
		DirectorsToDestroy.Add(CachedEnemyDirector);
	}

	if (World)
	{
		// Cleanup-only fallback: this runs during world/stage teardown where we
		// must remove any legacy duplicate director, not during gameplay ticks.
		for (TActorIterator<AT66EnemyDirector> It(World); It; ++It)
		{
			if (AT66EnemyDirector* ExistingDirector = *It)
			{
				DirectorsToDestroy.AddUnique(ExistingDirector);
			}
		}
	}

	for (AT66EnemyDirector* ExistingDirector : DirectorsToDestroy)
	{
		if (ExistingDirector)
		{
			ExistingDirector->Destroy();
		}
	}

	EnemyDirector = nullptr;
}

UStaticMesh* AT66GameMode::GetCubeMesh()
{
	if (!CachedCubeMesh)
	{
		CachedCubeMesh = FT66VisualUtil::GetBasicShapeCube();
	}
	return CachedCubeMesh;
}
void AT66GameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyBackroomsPocket();

	// Unbind from long-lived RunState delegates (RunState is a GameInstanceSubsystem).
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->StageTimerChanged.RemoveDynamic(this, &AT66GameMode::HandleStageTimerChanged);
			RunState->StageChanged.RemoveDynamic(this, &AT66GameMode::HandleStageChanged);
			RunState->DifficultyChanged.RemoveDynamic(this, &AT66GameMode::HandleDifficultyChanged);
		}
		if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
		{
			IdolManager->IdolStateChanged.RemoveDynamic(this, &AT66GameMode::HandleIdolStateChanged);
		}
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.RemoveDynamic(this, &AT66GameMode::HandleSettingsChanged);
		}
	}

	ActiveAsyncLoadHandles.Reset();
	PlayerCompanions.Reset();
	StageBoss = nullptr;
	EnemyDirector = nullptr;
	TutorialManager = nullptr;
	IdolAltar = nullptr;
	bTowerMiasmaActive = false;
	TowerMiasmaStartWorldSeconds = 0.f;
	TowerMiasmaUpdateAccumulator = 0.f;
	TowerIdolSelectionsAtStageStart = 0;
	T66InvalidatePlayerStartCache(GetWorld());
	T66ResetTaggedActorCache(GetWorld());

	Super::EndPlay(EndPlayReason);
}
void AT66GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bBackroomsChallengeActive)
	{
		return;
	}

	const bool bTowerLayout = IsUsingTowerMainMapLayout();
	const float TerrainSafetyIntervalSeconds = bTowerLayout ? 0.10f : 0.15f;
	TowerTerrainSafetyAccumulator += DeltaTime;
	if (!bTerrainCollisionReady || TowerTerrainSafetyAccumulator >= TerrainSafetyIntervalSeconds)
	{
		TowerTerrainSafetyAccumulator = 0.f;
		MaintainPlayerTerrainSafety();
	}

	SyncTowerTrapActivation();
	TryActivateMainMapCombat();
	UpdateTowerMiasma(DeltaTime);
	SyncTowerBossEntryState();

	// Frame-level lag: log when a frame exceeded budget (e.g. >20ms).
	if (DeltaTime > 0.02f)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66LagTrackerSubsystem* Lag = GI->GetSubsystem<UT66LagTrackerSubsystem>())
			{
				if (Lag->IsEnabled())
				{
					UE_LOG(LogT66GameMode, Verbose, TEXT("[LAG] Frame: %.2fms (%.1f FPS)"), DeltaTime * 1000.f, 1.f / FMath::Max(0.001f, DeltaTime));
				}
			}
		}
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->TickStageTimer(DeltaTime);
			RunState->TickSpeedRunTimer(DeltaTime);
			RunState->TickHeroTimers(DeltaTime);
			TickFinalDifficultySurvival(DeltaTime);

			// Skill Rating: time is driven here; damage is an input event from RunState.
			if (UT66SkillRatingSubsystem* Skill = GI->GetSubsystem<UT66SkillRatingSubsystem>())
			{
				Skill->SetTrackingActive(RunState->GetStageTimerActive());
				Skill->TickSkillRating(DeltaTime);
			}

			// Robust: if the timer is already active (even if we missed the delegate),
			// try spawning the LoanShark when pending.
			if (!IsLabRun() && RunState->GetStageTimerActive())
			{
				TrySpawnLoanSharkIfNeeded();
			}

			// Despawn loan shark immediately once debt is paid.
			if (LoanShark && RunState->GetCurrentDebt() <= 0)
			{
				LoanShark->Destroy();
				LoanShark = nullptr;
			}
		}
	}
}

void AT66GameMode::Logout(AController* Exiting)
{
	UWorld* World = GetWorld();
	UT66SessionSubsystem* SessionSubsystem = nullptr;
	const FString CurrentMapName = World ? UWorld::RemovePIEPrefix(World->GetMapName()) : FString();
	const FString FrontendMapName = UT66GameInstance::GetFrontendLevelName().ToString();
	const bool bShouldCollapsePartySession =
		World
		&& World->GetNetMode() != NM_Standalone
		&& !CurrentMapName.Contains(FrontendMapName)
		&& !World->URL.HasOption(TEXT("closed"))
		&& (SessionSubsystem = GetT66GameInstance() ? GetT66GameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr)
		&& !SessionSubsystem->IsGameplaySaveAndReturnInProgress();

	if (bShouldCollapsePartySession)
	{
		UE_LOG(LogT66GameMode, Warning, TEXT("Gameplay player logout detected (%s); saving the active run on the host and returning the party to frontend."), *GetNameSafe(Exiting));
		SessionSubsystem->SaveCurrentRunAndReturnToFrontend();
	}

	Super::Logout(Exiting);
}

void AT66GameMode::HandleStageTimerChanged()
{
	if (bBackroomsChallengeActive) return;
	if (IsLabRun()) return;
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	FLagScopedScope LagScope(GetWorld(), TEXT("HandleStageTimerChanged (Miasma+LoanShark)"));

	// Perf: non-tower miasma updates are event-driven (StageTimerChanged broadcasts at most once per second).
	if (!IsUsingTowerMainMapLayout() && MiasmaManager)
	{
		MiasmaManager->UpdateFromRunState();
	}

	if (RunState->GetStageTimerActive())
	{
		TrySpawnLoanSharkIfNeeded();
	}
}

void AT66GameMode::HandleStageChanged()
{
	RefreshProgressionDrivenSystems(true);
}

void AT66GameMode::HandleIdolStateChanged()
{
	if (!IsUsingTowerMainMapLayout() || bTowerMiasmaActive || !IsValid(IdolAltar))
	{
		return;
	}

	if (TowerIdolSelectionsAtStageStart > 0 && IdolAltar->RemainingSelections < TowerIdolSelectionsAtStageStart)
	{
		const FVector IdolAnchor = IdolAltar->GetActorLocation();
		TryStartTowerMiasma(&IdolAnchor, CachedTowerMainMapLayout.StartFloorNumber);
	}
}

void AT66GameMode::HandleDifficultyChanged()
{
	RefreshProgressionDrivenSystems(true);
}
void AT66GameMode::RefreshProgressionDrivenSystems(const bool bRescaleLiveEnemies)
{
	if (IsLabRun())
	{
		return;
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66StageProgressionSubsystem* StageProgression = GI ? GI->GetSubsystem<UT66StageProgressionSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	if (StageProgression)
	{
		StageProgression->RefreshSnapshot(false);
	}

	const FT66StageProgressionSnapshot Snapshot = StageProgression
		? StageProgression->GetCurrentSnapshot()
		: FT66StageProgressionSnapshot{};

	if (bRescaleLiveEnemies && World)
	{
		const int32 Stage = RunState->GetCurrentStage();
		const float DifficultyScalar = RunState->GetDifficultyScalar();
		const float FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
		UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
		const TArray<TWeakObjectPtr<AT66EnemyBase>>& Enemies = Registry ? Registry->GetEnemies() : TArray<TWeakObjectPtr<AT66EnemyBase>>();
		for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Enemies)
		{
			if (AT66EnemyBase* Enemy = WeakEnemy.Get())
			{
				Enemy->ApplyStageScaling(Stage);
				Enemy->ApplyDifficultyScalar(DifficultyScalar);
				Enemy->ApplyProgressionEnemyScalar(Snapshot.EnemyStatScalar);
				Enemy->ApplyFinaleScaling(FinaleScalar);
			}
		}

		const TArray<TWeakObjectPtr<AT66BossBase>>& Bosses = Registry ? Registry->GetBosses() : TArray<TWeakObjectPtr<AT66BossBase>>();
		for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Bosses)
		{
			if (AT66BossBase* Boss = WeakBoss.Get())
			{
				Boss->ApplyDifficultyScalar(DifficultyScalar);
			}
		}
	}

	if (World)
	{
		if (UT66TrapSubsystem* TrapSubsystem = World->GetSubsystem<UT66TrapSubsystem>())
		{
			TrapSubsystem->RefreshRegisteredTrapProgression();
		}

		if (AT66EnemyDirector* ExistingEnemyDirector = FindOrCacheEnemyDirector(World))
		{
			ExistingEnemyDirector->RefreshSpawningFromProgression();
		}
	}

	ApplyStageProgressionVisuals();
}

void AT66GameMode::ApplyStageProgressionVisuals()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66StageProgressionSubsystem* StageProgression = GI ? GI->GetSubsystem<UT66StageProgressionSubsystem>() : nullptr;
	if (StageProgression)
	{
		StageProgression->RefreshSnapshot(false);
		FT66StageProgressionVisuals::ApplyToWorld(World, StageProgression->GetCurrentSnapshot());
	}

	FT66WorldVisualSetup::EnsureAtmosphereForWorld(World);
}

void AT66GameMode::TrySpawnLoanSharkIfNeeded()
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	// Only spawn if the previous stage was exited with debt.
	if (!RunState->GetLoanSharkPending())
	{
		return;
	}
	if (RunState->GetCurrentDebt() <= 0)
	{
		RunState->SetLoanSharkPending(false);
		return;
	}
	if (LoanShark)
	{
		RunState->SetLoanSharkPending(false);
		return;
	}

	APawn* PlayerPawn = nullptr;
	if (UWorld* SearchWorld = GetWorld())
	{
		for (FConstPlayerControllerIterator It = SearchWorld->GetPlayerControllerIterator(); It; ++It)
		{
			PlayerPawn = It->Get() ? It->Get()->GetPawn() : nullptr;
			if (PlayerPawn)
			{
				break;
			}
		}
	}
	if (!PlayerPawn) return;
	UWorld* World = GetWorld();
	if (!World) return;

	FVector PlayerLoc = PlayerPawn->GetActorLocation();
	FVector SpawnLoc = PlayerLoc + FVector(-1200.f, 0.f, 0.f);

	// Try a few times to avoid spawning inside safe zones
	for (int32 Try = 0; Try < 10; ++Try)
	{
		float Angle = FMath::RandRange(0.f, 2.f * PI);
		float Dist = FMath::RandRange(900.f, 1400.f);
		FVector Offset(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
		SpawnLoc = PlayerLoc + Offset;

		bool bInSafe = false;
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const TArray<TWeakObjectPtr<AT66NPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66NPCBase>>();
		for (const TWeakObjectPtr<AT66NPCBase>& WeakNPC : NPCs)
		{
			AT66NPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			const float R = NPC->GetSafeZoneRadius();
			if (FVector::DistSquared2D(SpawnLoc, NPC->GetActorLocation()) < (R * R))
			{
				bInSafe = true;
				break;
			}
		}
		if (!bInSafe)
		{
			break;
		}
	}

	// Trace down for ground
	FHitResult Hit;
	FVector Start = SpawnLoc + FVector(0.f, 0.f, 500.f);
	FVector End = SpawnLoc - FVector(0.f, 0.f, 1000.f);
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
	{
		SpawnLoc = Hit.ImpactPoint + FVector(0.f, 0.f, 90.f);
	}
	else
	{
		SpawnLoc.Z = PlayerLoc.Z;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	LoanShark = World->SpawnActor<AT66LoanShark>(AT66LoanShark::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	RunState->SetLoanSharkPending(false);
}
// ============================================================================
// Main map terrain
// ============================================================================

void AT66GameMode::HandleSettingsChanged()
{
	FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(GetWorld());
	const UT66GameInstance* T66GI = GetT66GameInstance();
	const bool bTestRoomRun = T66GI && T66GI->IsTestRoomRun();
	if (!bTestRoomRun)
	{
		ApplyStageProgressionVisuals();
	}
}
UT66GameInstance* AT66GameMode::GetT66GameInstance() const
{
	return Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
}

bool AT66GameMode::TrySnapActorToTerrain(AActor* Actor) const
{
	return TrySnapActorToTerrainAtLocation(Actor, Actor ? Actor->GetActorLocation() : FVector::ZeroVector);
}

bool AT66GameMode::TrySnapActorToTerrainAtLocation(AActor* Actor, const FVector& TraceLocation) const
{
	UWorld* World = GetWorld();
	if (!World || !Actor)
	{
		return false;
	}

	FHitResult GroundHit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66SnapActorToTerrain), false, Actor);
	T66AddCachedPlayerStartsToTraceIgnore(World, Params);
	FVector TraceStart = FVector::ZeroVector;
	FVector TraceEnd = FVector::ZeroVector;
	if (IsUsingTowerMainMapLayout())
	{
		const float LocalTraceUp = FMath::Max(900.0f, CachedTowerMainMapLayout.FloorSpacing - CachedTowerMainMapLayout.FloorThickness - 350.0f);
		const float LocalTraceDown = FMath::Max(CachedTowerMainMapLayout.FloorSpacing + CachedTowerMainMapLayout.FloorThickness + 900.0f, 3200.0f);
		TraceStart = FVector(TraceLocation.X, TraceLocation.Y, TraceLocation.Z + LocalTraceUp);
		TraceEnd = FVector(TraceLocation.X, TraceLocation.Y, TraceLocation.Z - LocalTraceDown);
	}
	else
	{
		TraceStart = FVector(TraceLocation.X, TraceLocation.Y, TraceLocation.Z + 8000.f);
		TraceEnd = FVector(TraceLocation.X, TraceLocation.Y, TraceLocation.Z - 16000.f);
	}

	bool bFoundGroundHit = false;
	if (IsUsingTowerMainMapLayout())
	{
		TArray<FHitResult> Hits;
		if (World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, ECC_WorldStatic, Params))
		{
			for (const FHitResult& Hit : Hits)
			{
				if (T66ShouldIgnoreTowerCeilingHit(Hit))
				{
					continue;
				}

				GroundHit = Hit;
				bFoundGroundHit = true;
				break;
			}
		}

		if (!bFoundGroundHit && World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, ECC_Visibility, Params))
		{
			for (const FHitResult& Hit : Hits)
			{
				if (T66ShouldIgnoreTowerCeilingHit(Hit))
				{
					continue;
				}

				GroundHit = Hit;
				bFoundGroundHit = true;
				break;
			}
		}
	}
	else
	{
		bFoundGroundHit =
			World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic, Params)
			|| World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, Params);
	}

	if (!bFoundGroundHit)
	{
		return false;
	}

	float HalfHeight = 0.f;
	if (const UCapsuleComponent* Capsule = Actor->FindComponentByClass<UCapsuleComponent>())
	{
		HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	}

	const FVector GroundedLoc = GroundHit.ImpactPoint + FVector(0.f, 0.f, HalfHeight + 5.f);
	Actor->SetActorLocation(GroundedLoc, false, nullptr, ETeleportType::TeleportPhysics);
	return true;
}

void AT66GameMode::MaintainPlayerTerrainSafety()
{
	UWorld* World = GetWorld();
	if (!World || IsLabRun())
	{
		return;
	}

	UT66GameInstance* GI = GetT66GameInstance();
	if (GI && GI->IsTestRoomRun())
	{
		return;
	}

	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const FT66MapPreset Preset = T66BuildMainMapPreset(GI);
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	const bool bTowerLayout = bUsingMainMapTerrain && IsUsingTowerMainMapLayout();
	float RescueThresholdZ = bUsingMainMapTerrain
		? (T66MainMapTerrain::GetLowestCollisionBottomZ(Preset) - 100.0f)
		: (Preset.ElevationMin - 200.f);
	if (bTowerLayout && CachedTowerMainMapLayout.Floors.Num() > 0)
	{
		const T66TowerMapTerrain::FFloor& LowestFloor = CachedTowerMainMapLayout.Floors.Last();
		RescueThresholdZ = LowestFloor.SurfaceZ - FMath::Max(CachedTowerMainMapLayout.FloorThickness + 1400.0f, 1800.0f);
	}
	const float AnchorTraceZ = bUsingMainMapTerrain
		? T66MainMapTerrain::GetTraceZ(Preset)
		: (Preset.ElevationMax + 3000.f);
	const bool bStageTimerActive = RunState && RunState->GetStageTimerActive();

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		if (!Pawn)
		{
			continue;
		}

		ACharacter* Character = Cast<ACharacter>(Pawn);
		UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;

		if (!bTerrainCollisionReady)
		{
			if (Movement && Movement->MovementMode != MOVE_None)
			{
				Movement->StopMovementImmediately();
				Movement->SetMovementMode(MOVE_None);
			}
			continue;
		}

		const FVector PawnLoc = Pawn->GetActorLocation();
		const int32 TowerFloorNumber = bTowerLayout ? GetTowerFloorIndexForLocation(PawnLoc) : INDEX_NONE;
		const bool bMovementLost = Movement && Movement->MovementMode == MOVE_None;
		const bool bNeedsRescue = bTowerLayout
			? (PawnLoc.Z <= RescueThresholdZ || (bMovementLost && TowerFloorNumber == INDEX_NONE))
			: (PawnLoc.Z <= RescueThresholdZ || bMovementLost);
		if (!bNeedsRescue)
		{
			continue;
		}

		TArray<FVector> RescueAnchors;
		if (bUsingMainMapTerrain)
		{
			if (MainMapRescueAnchorLocations.Num() > 0)
			{
				RescueAnchors = MainMapRescueAnchorLocations;
			}
			else
			{
				const int32 GridSize = T66MainMapTerrain::MakeSettings(Preset).BoardSize;
				RescueAnchors.Reserve(5);
				RescueAnchors.Add(T66MainMapTerrain::GetSpawnLocation(Preset, AnchorTraceZ));
				RescueAnchors.Add(T66MainMapTerrain::GetCellCenter(Preset, GridSize / 2, FMath::Max(0, GridSize / 2 - 1), AnchorTraceZ));
				RescueAnchors.Add(T66MainMapTerrain::GetCellCenter(Preset, GridSize / 2, FMath::Min(GridSize - 1, GridSize / 2 + 1), AnchorTraceZ));
				RescueAnchors.Add(T66MainMapTerrain::GetCellCenter(Preset, FMath::Max(0, GridSize / 2 - 1), GridSize / 2, AnchorTraceZ));
				RescueAnchors.Add(T66MainMapTerrain::GetCellCenter(Preset, FMath::Min(GridSize - 1, GridSize / 2 + 1), GridSize / 2, AnchorTraceZ));
			}
		}
		else if (!bStageTimerActive || PawnLoc.X <= 0.f)
		{
			RescueAnchors.Reserve(4);
			RescueAnchors.Add(T66GameplayLayout::GetStartAreaCenter(AnchorTraceZ));
			RescueAnchors.Add(T66GameplayLayout::GetStartEntryLocation(AnchorTraceZ));
		}
		if (!bUsingMainMapTerrain && bStageTimerActive && PawnLoc.X > 0.f)
		{
			RescueAnchors.Add(T66GameplayLayout::GetBossEntryLocation(AnchorTraceZ));
			RescueAnchors.Add(T66GameplayLayout::GetBossAreaCenter(AnchorTraceZ));
		}

		bool bRecovered = TrySnapActorToTerrain(Pawn);
		if (!bRecovered)
		{
			RescueAnchors.Sort([PawnLoc, bTowerLayout](const FVector& A, const FVector& B)
			{
				return bTowerLayout
					? (FVector::DistSquared(A, PawnLoc) < FVector::DistSquared(B, PawnLoc))
					: (FVector::DistSquared2D(A, PawnLoc) < FVector::DistSquared2D(B, PawnLoc));
			});

			for (const FVector& Anchor : RescueAnchors)
			{
				if (TrySnapActorToTerrainAtLocation(Pawn, Anchor))
				{
					bRecovered = true;
					break;
				}
			}
		}

		if (Movement)
		{
			Movement->StopMovementImmediately();
			Movement->SetMovementMode(bRecovered ? MOVE_Walking : MOVE_None);
		}

		// A rescue teleport is an explicit floor transition: re-derive the hero's
		// stateful floor from the recovered position (Z derivation is valid here
		// because the pawn was just snapped onto a floor surface).
		if (bRecovered && bTowerLayout)
		{
			const int32 RecoveredFloorNumber = GetTowerFloorIndexForLocation(Pawn->GetActorLocation());
			if (RecoveredFloorNumber != INDEX_NONE)
			{
				SetHeroTowerFloorNumber(RecoveredFloorNumber, TEXT("FallRescue"));
			}
		}
	}
}

void AT66GameMode::SnapPlayersToTerrain()
{
	UWorld* World = GetWorld();
	if (!World || !bTerrainCollisionReady)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		if (!Pawn)
		{
			continue;
		}

		const bool bSnapped = TrySnapActorToTerrain(Pawn);
		if (!bSnapped)
		{
			continue;
		}

		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
				if (Movement->MovementMode == MOVE_None)
				{
					Movement->SetMovementMode(MOVE_Walking);
				}
			}
		}
	}
}

// Console command: T66.MainMap [seed]
static FAutoConsoleCommandWithWorldAndArgs T66MainMapCmd(
	TEXT("T66.MainMap"),
	TEXT("Regenerate the difficulty-driven main map terrain. Usage: T66.MainMap [seed]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			if (!World) return;
			AGameModeBase* GM = World->GetAuthGameMode();
			AT66GameMode* T66GM = Cast<AT66GameMode>(GM);
			if (!T66GM)
			{
				UE_LOG(LogT66GameMode, Error, TEXT("T66.MainMap: no T66GameMode active"));
				return;
			}

			int32 Seed = FMath::Rand();
			if (Args.Num() >= 1)
			{
				Seed = FCString::Atoi(*Args[0]);
			}

			UE_LOG(LogT66GameMode, Log, TEXT("T66.MainMap: Regenerating main map terrain (seed=%d)"), Seed);
			T66GM->RegenerateMainMapTerrain(Seed);
		})
);
// ============================================
// The Lab: spawn / reset
// ============================================

