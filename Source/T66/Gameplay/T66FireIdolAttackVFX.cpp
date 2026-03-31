// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66FireIdolAttackVFX.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UnrealType.h"
#include "UObject/ConstructorHelpers.h"
#include "ZibraVDBActor.h"
#include "ZibraVDBAssetComponent.h"
#include "ZibraVDBPlaybackComponent.h"
#include "ZibraVDBVolume4.h"

namespace
{
	const TCHAR* BlenderSourceBlendPath = TEXT("C:/UE/T66/SourceAssets/Import/VFX/Idol_Fire_Explosion/FireExplosion_True3D_Source.blend");
	const TCHAR* BlenderProxyMeshPath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	const TCHAR* BlenderProxyMaterialPath = TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Impact.MI_Hero1_Attack_Impact");
	const TCHAR* ZibraShockwaveAssetPath = TEXT("/Game/VFX/Idols/Fire/Zibra/Volumes/ShockwaveExplosion.ShockwaveExplosion");
	const TCHAR* ZibraAcceptedReviewPath = TEXT("C:/UE/T66/Saved/ZibraReview/ShockwaveExplosion_review_f05_crop.png");

	struct FProxyVectorKey
	{
		float Frame = 1.f;
		FVector Value = FVector::ZeroVector;
	};

	FVector EvaluateProxyKeys(const TConstArrayView<FProxyVectorKey> Keys, const float Age01)
	{
		if (Keys.IsEmpty())
		{
			return FVector::ZeroVector;
		}

		const float Frame = FMath::Lerp(1.f, 24.f, FMath::Clamp(Age01, 0.f, 1.f));
		if (Frame <= Keys[0].Frame)
		{
			return Keys[0].Value;
		}

		for (int32 Index = 1; Index < Keys.Num(); ++Index)
		{
			const FProxyVectorKey& Previous = Keys[Index - 1];
			const FProxyVectorKey& Current = Keys[Index];
			if (Frame <= Current.Frame)
			{
				const float Span = FMath::Max(KINDA_SMALL_NUMBER, Current.Frame - Previous.Frame);
				const float Alpha = FMath::Clamp((Frame - Previous.Frame) / Span, 0.f, 1.f);
				return FMath::Lerp(Previous.Value, Current.Value, Alpha);
			}
		}

		return Keys.Last().Value;
	}

	UMaterialInterface* LoadBlenderProxyMaterial()
	{
		static TWeakObjectPtr<UMaterialInterface> CachedMaterial;
		UMaterialInterface* Material = CachedMaterial.Get();
		if (!Material)
		{
			Material = LoadObject<UMaterialInterface>(nullptr, BlenderProxyMaterialPath);
			CachedMaterial = Material;
		}
		return Material;
	}

	UStaticMesh* LoadBlenderProxyMesh()
	{
		static TWeakObjectPtr<UStaticMesh> CachedMesh;
		UStaticMesh* Mesh = CachedMesh.Get();
		if (!Mesh)
		{
			Mesh = LoadObject<UStaticMesh>(nullptr, BlenderProxyMeshPath);
			CachedMesh = Mesh;
		}
		return Mesh;
	}

	UZibraVDBVolume4* LoadZibraShockwaveAsset()
	{
		static TWeakObjectPtr<UZibraVDBVolume4> CachedAsset;
		UZibraVDBVolume4* Asset = CachedAsset.Get();
		if (!Asset)
		{
			Asset = LoadObject<UZibraVDBVolume4>(nullptr, ZibraShockwaveAssetPath);
			CachedAsset = Asset;
		}
		return Asset;
	}

	FString BuildMissingList(const bool bMissingMesh, const bool bMissingMaterial)
	{
		TArray<FString> MissingItems;
		if (bMissingMesh)
		{
			MissingItems.Add(TEXT("ProxyMesh"));
		}
		if (bMissingMaterial)
		{
			MissingItems.Add(TEXT("ProxyMaterial"));
		}
		return MissingItems.IsEmpty() ? TEXT("None") : FString::Join(MissingItems, TEXT(","));
	}

	void BuildFirePalette(const FLinearColor& SeedColor, FLinearColor& OutTint, FLinearColor& OutPrimary, FLinearColor& OutSecondary, FLinearColor& OutOutline)
	{
		const FLinearColor ClampedSeed(
			FMath::Clamp(SeedColor.R, 0.f, 1.f),
			FMath::Clamp(SeedColor.G, 0.f, 1.f),
			FMath::Clamp(SeedColor.B, 0.f, 1.f),
			1.f);

		FLinearColor FireHSV = ClampedSeed.LinearRGBToHSV();
		FireHSV.G = FMath::Clamp(FMath::Max(FireHSV.G * 1.55f, 0.72f), 0.f, 1.f);
		FireHSV.B = FMath::Clamp(FMath::Max(FireHSV.B * 1.30f, 0.84f), 0.f, 1.f);

		OutPrimary = FireHSV.HSVToLinearRGB();
		OutTint = FLinearColor::LerpUsingHSV(OutPrimary, FLinearColor::White, 0.20f);
		OutSecondary = FLinearColor::LerpUsingHSV(OutPrimary, FLinearColor(0.25f, 0.05f, 0.02f, 1.f), 0.46f);
		OutOutline = FLinearColor::LerpUsingHSV(OutSecondary, FLinearColor::Black, 0.72f);
	}

	void BuildSmokePalette(FLinearColor& OutTint, FLinearColor& OutPrimary, FLinearColor& OutSecondary, FLinearColor& OutOutline)
	{
		OutTint = FLinearColor(0.34f, 0.30f, 0.28f, 1.f);
		OutPrimary = FLinearColor(0.18f, 0.14f, 0.12f, 1.f);
		OutSecondary = FLinearColor(0.08f, 0.06f, 0.06f, 1.f);
		OutOutline = FLinearColor(0.02f, 0.02f, 0.02f, 1.f);
	}

	UActorComponent* FindActorComponentByName(AActor* Actor, const FName ComponentName)
	{
		if (!Actor)
		{
			return nullptr;
		}

		TArray<UActorComponent*> Components;
		Actor->GetComponents(Components);
		for (UActorComponent* Component : Components)
		{
			if (Component && Component->GetFName() == ComponentName)
			{
				return Component;
			}
		}

		return nullptr;
	}

	bool SetBoolProperty(UObject* Object, const FName PropertyName, const bool bValue)
	{
		if (!Object)
		{
			return false;
		}

		if (FBoolProperty* Property = FindFProperty<FBoolProperty>(Object->GetClass(), PropertyName))
		{
			Property->SetPropertyValue_InContainer(Object, bValue);
			return true;
		}
		return false;
	}

	bool SetFloatProperty(UObject* Object, const FName PropertyName, const float Value)
	{
		if (!Object)
		{
			return false;
		}

		if (FFloatProperty* Property = FindFProperty<FFloatProperty>(Object->GetClass(), PropertyName))
		{
			Property->SetPropertyValue_InContainer(Object, Value);
			return true;
		}
		return false;
	}

	bool SetIntProperty(UObject* Object, const FName PropertyName, const int32 Value)
	{
		if (!Object)
		{
			return false;
		}

		if (FIntProperty* Property = FindFProperty<FIntProperty>(Object->GetClass(), PropertyName))
		{
			Property->SetPropertyValue_InContainer(Object, Value);
			return true;
		}
		return false;
	}

	bool SetEnumProperty(UObject* Object, const FName PropertyName, const int64 Value)
	{
		if (!Object)
		{
			return false;
		}

		if (FEnumProperty* Property = FindFProperty<FEnumProperty>(Object->GetClass(), PropertyName))
		{
			if (FNumericProperty* UnderlyingProperty = Property->GetUnderlyingProperty())
			{
				UnderlyingProperty->SetIntPropertyValue(Property->ContainerPtrToValuePtr<void>(Object), Value);
				return true;
			}
		}

		if (FByteProperty* Property = FindFProperty<FByteProperty>(Object->GetClass(), PropertyName))
		{
			Property->SetPropertyValue_InContainer(Object, static_cast<uint8>(Value));
			return true;
		}

		return false;
	}

	bool SetLinearColorProperty(UObject* Object, const FName PropertyName, const FLinearColor& Value)
	{
		if (!Object)
		{
			return false;
		}

		if (FStructProperty* Property = FindFProperty<FStructProperty>(Object->GetClass(), PropertyName))
		{
			if (Property->Struct == TBaseStructure<FLinearColor>::Get())
			{
				*Property->ContainerPtrToValuePtr<FLinearColor>(Object) = Value;
				return true;
			}
		}
		return false;
	}

	const FProxyVectorKey FireCoreScaleKeys[] = {
		{1.f, FVector(0.20f, 0.20f, 0.16f)},
		{2.f, FVector(0.50f, 0.50f, 0.36f)},
		{4.f, FVector(0.96f, 0.96f, 0.72f)},
		{7.f, FVector(1.24f, 1.20f, 0.94f)},
		{12.f, FVector(1.46f, 1.42f, 1.08f)},
		{18.f, FVector(1.58f, 1.54f, 1.14f)},
		{24.f, FVector(1.46f, 1.40f, 1.02f)},
	};
	const FProxyVectorKey FireCoreLocationKeys[] = {
		{1.f, FVector(0.00f, 0.00f, 0.88f)},
		{6.f, FVector(0.02f, 0.00f, 1.06f)},
		{12.f, FVector(0.02f, -0.01f, 1.28f)},
		{24.f, FVector(0.00f, -0.02f, 1.46f)},
	};
	const FProxyVectorKey FireShellScaleKeys[] = {
		{1.f, FVector(0.10f, 0.10f, 0.05f)},
		{3.f, FVector(0.56f, 0.56f, 0.16f)},
		{5.f, FVector(0.96f, 0.96f, 0.22f)},
		{8.f, FVector(1.28f, 1.28f, 0.28f)},
		{14.f, FVector(1.42f, 1.42f, 0.32f)},
		{24.f, FVector(1.30f, 1.30f, 0.28f)},
	};
	const FProxyVectorKey FireShellLocationKeys[] = {
		{1.f, FVector(0.00f, 0.00f, 0.55f)},
		{10.f, FVector(0.00f, 0.00f, 0.60f)},
		{24.f, FVector(0.00f, 0.00f, 0.66f)},
	};
	const FProxyVectorKey FireLobeAScaleKeys[] = {
		{1.f, FVector(0.08f, 0.07f, 0.10f)},
		{3.f, FVector(0.28f, 0.24f, 0.34f)},
		{6.f, FVector(0.64f, 0.52f, 0.76f)},
		{10.f, FVector(0.86f, 0.70f, 1.00f)},
		{18.f, FVector(0.94f, 0.78f, 1.06f)},
		{24.f, FVector(0.78f, 0.64f, 0.88f)},
	};
	const FProxyVectorKey FireLobeALocationKeys[] = {
		{1.f, FVector(0.24f, -0.18f, 0.98f)},
		{8.f, FVector(0.42f, -0.32f, 1.24f)},
		{16.f, FVector(0.54f, -0.40f, 1.46f)},
		{24.f, FVector(0.58f, -0.42f, 1.60f)},
	};
	const FProxyVectorKey FireLobeBScaleKeys[] = {
		{1.f, FVector(0.08f, 0.09f, 0.09f)},
		{3.f, FVector(0.24f, 0.28f, 0.28f)},
		{6.f, FVector(0.52f, 0.60f, 0.64f)},
		{10.f, FVector(0.72f, 0.82f, 0.82f)},
		{18.f, FVector(0.78f, 0.88f, 0.88f)},
		{24.f, FVector(0.68f, 0.74f, 0.74f)},
	};
	const FProxyVectorKey FireLobeBLocationKeys[] = {
		{1.f, FVector(-0.28f, 0.20f, 0.90f)},
		{8.f, FVector(-0.42f, 0.34f, 1.16f)},
		{16.f, FVector(-0.52f, 0.44f, 1.32f)},
		{24.f, FVector(-0.56f, 0.48f, 1.46f)},
	};
	const FProxyVectorKey SmokeMainScaleKeys[] = {
		{1.f, FVector(0.10f, 0.10f, 0.08f)},
		{5.f, FVector(0.18f, 0.18f, 0.12f)},
		{8.f, FVector(0.88f, 0.88f, 0.64f)},
		{12.f, FVector(1.26f, 1.22f, 0.92f)},
		{18.f, FVector(1.54f, 1.50f, 1.16f)},
		{24.f, FVector(1.72f, 1.66f, 1.24f)},
	};
	const FProxyVectorKey SmokeMainLocationKeys[] = {
		{1.f, FVector(0.00f, 0.00f, 0.94f)},
		{10.f, FVector(0.00f, 0.00f, 1.18f)},
		{18.f, FVector(0.02f, 0.00f, 1.42f)},
		{24.f, FVector(0.03f, -0.01f, 1.62f)},
	};
	const FProxyVectorKey SmokeCapScaleKeys[] = {
		{1.f, FVector(0.05f, 0.05f, 0.05f)},
		{7.f, FVector(0.12f, 0.10f, 0.12f)},
		{10.f, FVector(0.46f, 0.40f, 0.48f)},
		{14.f, FVector(0.72f, 0.60f, 0.76f)},
		{20.f, FVector(0.86f, 0.74f, 0.88f)},
		{24.f, FVector(0.92f, 0.80f, 0.96f)},
	};
	const FProxyVectorKey SmokeCapLocationKeys[] = {
		{1.f, FVector(0.18f, 0.10f, 1.12f)},
		{10.f, FVector(0.28f, 0.18f, 1.44f)},
		{20.f, FVector(0.36f, 0.24f, 1.78f)},
		{24.f, FVector(0.38f, 0.24f, 1.92f)},
	};
}

AT66FireIdolAttackVFX::AT66FireIdolAttackVFX()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	auto CreateProxyMesh = [this](const TCHAR* Name, const int32 SortPriority) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Mesh->SetupAttachment(SceneRoot);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCastShadow(false);
		Mesh->SetReceivesDecals(false);
		Mesh->SetCanEverAffectNavigation(false);
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->TranslucencySortPriority = SortPriority;
		Mesh->SetHiddenInGame(true);
		Mesh->SetVisibility(false, true);
		return Mesh;
	};

	FireCoreMesh = CreateProxyMesh(TEXT("FireCoreMesh"), 4);
	FireShellMesh = CreateProxyMesh(TEXT("FireShellMesh"), 5);
	FireLobeAMesh = CreateProxyMesh(TEXT("FireLobeAMesh"), 6);
	FireLobeBMesh = CreateProxyMesh(TEXT("FireLobeBMesh"), 6);
	SmokeMainMesh = CreateProxyMesh(TEXT("SmokeMainMesh"), 3);
	SmokeCapMesh = CreateProxyMesh(TEXT("SmokeCapMesh"), 3);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(BlenderProxyMeshPath);
	if (SphereMesh.Succeeded())
	{
		FireCoreMesh->SetStaticMesh(SphereMesh.Object);
		FireShellMesh->SetStaticMesh(SphereMesh.Object);
		FireLobeAMesh->SetStaticMesh(SphereMesh.Object);
		FireLobeBMesh->SetStaticMesh(SphereMesh.Object);
		SmokeMainMesh->SetStaticMesh(SphereMesh.Object);
		SmokeCapMesh->SetStaticMesh(SphereMesh.Object);
	}
}

void AT66FireIdolAttackVFX::ConfigureCandidate(const ET66FireIdolTestCandidate InCandidate)
{
	Candidate = InCandidate;
	LifetimeSeconds = (Candidate == ET66FireIdolTestCandidate::ZibraShockwave) ? 1.15f : 1.00f;
}

void AT66FireIdolAttackVFX::InitEffect(const FVector& InLocation, const float InRadius, const FLinearColor& InTint, const int32 InDebugRequestId, const FName InDebugSourceID)
{
	EffectLocation = InLocation;
	BaseRadius = FMath::Max(70.f, InRadius);
	TintColor = InTint;
	DebugRequestId = InDebugRequestId;
	DebugSourceID = InDebugSourceID;
	bInitialized = true;

	if (UWorld* World = GetWorld(); World && !World->IsGameWorld() && !bSetupSucceeded)
	{
		ApplyBaseTransform();
		FString FailureReason;
		bSetupSucceeded = SetupCandidate(FailureReason);
		if (!bSetupSucceeded)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[FIRE IDOL TEST] EditorPreviewFailed Req=%d Source=%s Candidate=%s FailureReason=%s"),
				DebugRequestId,
				DebugSourceID.IsNone() ? TEXT("Unknown") : *DebugSourceID.ToString(),
				GetCandidateName(Candidate),
				*FailureReason);
		}
		else if (Candidate == ET66FireIdolTestCandidate::Blender3D)
		{
			UpdateBlenderProxy(0.38f);
		}
		else if (Candidate == ET66FireIdolTestCandidate::ZibraShockwave)
		{
			if (AZibraVDBActor* ZibraActor = SpawnedZibraActor.Get())
			{
				if (UZibraVDBPlaybackComponent* PlaybackComponent = ZibraActor->GetPlaybackComponent())
				{
					const int32 FrameCount = ZibraVolumeAsset ? ZibraVolumeAsset->FrameCount : 0;
					PlaybackComponent->Animate = false;
					PlaybackComponent->CurrentFrame = FMath::Clamp(static_cast<float>(FrameCount) * 0.05f, 0.0f, static_cast<float>(FMath::Max(FrameCount - 1, 0)));
				}
			}
		}
	}
}

const TCHAR* AT66FireIdolAttackVFX::GetCandidateName(const ET66FireIdolTestCandidate InCandidate)
{
	switch (InCandidate)
	{
	case ET66FireIdolTestCandidate::Blender3D:
		return TEXT("Blender3D");
	case ET66FireIdolTestCandidate::ZibraShockwave:
		return TEXT("ZibraShockwave");
	default:
		return TEXT("Unknown");
	}
}

ET66FireIdolTestCandidate AT66FireIdolAttackVFX::SanitizeCandidate(const int32 RawValue, bool& bOutWasClamped)
{
	bOutWasClamped = false;
	switch (RawValue)
	{
	case static_cast<int32>(ET66FireIdolTestCandidate::Blender3D):
		return ET66FireIdolTestCandidate::Blender3D;
	case static_cast<int32>(ET66FireIdolTestCandidate::ZibraShockwave):
		return ET66FireIdolTestCandidate::ZibraShockwave;
	default:
		bOutWasClamped = true;
		return ET66FireIdolTestCandidate::ZibraShockwave;
	}
}

bool AT66FireIdolAttackVFX::PreflightCandidate(const ET66FireIdolTestCandidate InCandidate, FString& OutBindingSummary, FString& OutFailureReason)
{
	OutFailureReason.Reset();
	switch (InCandidate)
	{
	case ET66FireIdolTestCandidate::Blender3D:
	{
		const UStaticMesh* Mesh = LoadBlenderProxyMesh();
		const UMaterialInterface* Material = LoadBlenderProxyMaterial();
		const bool bReady = Mesh && Material;
		OutBindingSummary = FString::Printf(
			TEXT("SourceBlend=%s RuntimeForm=GeometryDrivenProxy ImportedAsset=None ProxyMesh=%s ProxyMaterial=%s Limitation=No Unreal-imported asset exists for the Blender source; gameplay uses a 3D mesh proxy that mirrors the authored rig structure."),
			BlenderSourceBlendPath,
			Mesh ? *Mesh->GetPathName() : BlenderProxyMeshPath,
			Material ? *Material->GetPathName() : BlenderProxyMaterialPath);
		if (!bReady)
		{
			OutFailureReason = FString::Printf(TEXT("Missing bindings: %s"), *BuildMissingList(Mesh == nullptr, Material == nullptr));
		}
		return bReady;
	}
	case ET66FireIdolTestCandidate::ZibraShockwave:
	{
		const UZibraVDBVolume4* Volume = LoadZibraShockwaveAsset();
		const UClass* ActorClass = AZibraVDBActor::StaticClass();
		const bool bReady = Volume && ActorClass;
		OutBindingSummary = FString::Printf(
			TEXT("RuntimeAsset=%s AssetClass=%s ActorClass=%s Components=AssetComponent,MaterialComponent,PlaybackComponent AcceptedReview=%s"),
			Volume ? *Volume->GetPathName() : ZibraShockwaveAssetPath,
			Volume ? *Volume->GetClass()->GetName() : TEXT("None"),
			ActorClass ? *ActorClass->GetName() : TEXT("None"),
			ZibraAcceptedReviewPath);
		if (!bReady)
		{
			TArray<FString> MissingItems;
			if (!Volume)
			{
				MissingItems.Add(TEXT("ShockwaveVolume"));
			}
			if (!ActorClass)
			{
				MissingItems.Add(TEXT("ZibraVDBActorClass"));
			}
			OutFailureReason = FString::Printf(TEXT("Missing bindings: %s"), *FString::Join(MissingItems, TEXT(",")));
		}
		return bReady;
	}
	default:
		OutBindingSummary = TEXT("Unknown candidate");
		OutFailureReason = TEXT("Unsupported fire idol candidate enum value.");
		return false;
	}
}

void AT66FireIdolAttackVFX::BeginPlay()
{
	Super::BeginPlay();

	if (!bInitialized)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[FIRE IDOL TEST] PlayFailed Req=%d Candidate=%s Actor=%s FailurePoint=Init Reason=InitEffect was not called before BeginPlay Fallback=None"),
			DebugRequestId,
			GetCandidateName(Candidate),
			*GetName());
		Destroy();
		return;
	}

	ApplyBaseTransform();

	FString FailureReason;
	bSetupSucceeded = SetupCandidate(FailureReason);
	if (!bSetupSucceeded)
	{
		LogPlayResult(TEXT("PlayFailed"), false, FailureReason, TEXT("SetupCandidate"), TEXT("None"));
		Destroy();
		return;
	}

	LogPlayResult(TEXT("PlayStarted"), true, FString::Printf(TEXT("Location=(%.1f,%.1f,%.1f) Radius=%.1f Candidate=%s"), EffectLocation.X, EffectLocation.Y, EffectLocation.Z, BaseRadius, GetCandidateName(Candidate)));
}

void AT66FireIdolAttackVFX::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bSetupSucceeded)
	{
		return;
	}

	ElapsedSeconds += DeltaSeconds;
	const float Age01 = FMath::Clamp(ElapsedSeconds / FMath::Max(0.01f, LifetimeSeconds), 0.f, 1.f);

	if (Candidate == ET66FireIdolTestCandidate::Blender3D)
	{
		UpdateBlenderProxy(Age01);
		if (!bLoggedActive)
		{
			bLoggedActive = true;
			LogPlayResult(TEXT("PlayActive"), true, TEXT("Components=FireCoreMesh,FireShellMesh,FireLobeAMesh,FireLobeBMesh,SmokeMainMesh,SmokeCapMesh"));
		}
	}
	else if (Candidate == ET66FireIdolTestCandidate::ZibraShockwave)
	{
		AZibraVDBActor* ZibraActor = SpawnedZibraActor.Get();
		if (ZibraActor && !bLoggedActive)
		{
			bLoggedActive = true;
			const UZibraVDBPlaybackComponent* PlaybackComponent = ZibraActor->GetPlaybackComponent();
			const UZibraVDBAssetComponent* AssetComponent = ZibraActor->GetAssetComponent();
			LogPlayResult(
				TEXT("PlayActive"),
				true,
				FString::Printf(
					TEXT("SpawnedZibraActor=%s CurrentFrame=%u Animate=%d Visible=%d"),
					*ZibraActor->GetName(),
					AssetComponent ? AssetComponent->GetCurrentFrame() : 0u,
					PlaybackComponent && PlaybackComponent->Animate ? 1 : 0,
					PlaybackComponent && PlaybackComponent->Visible ? 1 : 0));
		}
	}

	if (Age01 >= 1.f)
	{
		if (Candidate == ET66FireIdolTestCandidate::ZibraShockwave && SpawnedZibraActor.IsValid())
		{
			const UZibraVDBAssetComponent* AssetComponent = SpawnedZibraActor->GetAssetComponent();
			LogPlayResult(
				TEXT("PlayComplete"),
				true,
				FString::Printf(TEXT("Elapsed=%.3f LastFrame=%u"), ElapsedSeconds, AssetComponent ? AssetComponent->GetCurrentFrame() : 0u));
		}
		else
		{
			LogPlayResult(TEXT("PlayComplete"), true, FString::Printf(TEXT("Elapsed=%.3f"), ElapsedSeconds));
		}
		Destroy();
	}
}

void AT66FireIdolAttackVFX::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroySpawnedZibraActor();
	Super::EndPlay(EndPlayReason);
}

bool AT66FireIdolAttackVFX::SetupCandidate(FString& OutFailureReason)
{
	HideAllCandidateVisuals();
	switch (Candidate)
	{
	case ET66FireIdolTestCandidate::Blender3D:
		return SetupBlenderProxy(OutFailureReason);
	case ET66FireIdolTestCandidate::ZibraShockwave:
		return SetupZibraShockwave(OutFailureReason);
	default:
		OutFailureReason = TEXT("Unsupported fire idol candidate enum value.");
		return false;
	}
}

bool AT66FireIdolAttackVFX::SetupBlenderProxy(FString& OutFailureReason)
{
	UStaticMesh* ProxyMesh = LoadBlenderProxyMesh();
	BlenderProxyBaseMaterial = LoadBlenderProxyMaterial();
	if (!ProxyMesh || !BlenderProxyBaseMaterial)
	{
		OutFailureReason = FString::Printf(TEXT("Blender proxy bindings failed. Missing=%s"), *BuildMissingList(ProxyMesh == nullptr, BlenderProxyBaseMaterial == nullptr));
		return false;
	}

	auto BindMaterial = [this](UStaticMeshComponent* Mesh, const TCHAR* MidName, TObjectPtr<UMaterialInstanceDynamic>& OutMID) -> bool
	{
		Mesh->SetStaticMesh(LoadBlenderProxyMesh());
		OutMID = UMaterialInstanceDynamic::Create(BlenderProxyBaseMaterial, this, MidName);
		if (!OutMID)
		{
			return false;
		}
		Mesh->SetMaterial(0, OutMID);
		Mesh->SetHiddenInGame(false);
		Mesh->SetVisibility(true, true);
		return true;
	};

	if (!BindMaterial(FireCoreMesh, TEXT("FireCoreMID"), FireCoreMID) ||
		!BindMaterial(FireShellMesh, TEXT("FireShellMID"), FireShellMID) ||
		!BindMaterial(FireLobeAMesh, TEXT("FireLobeAMID"), FireLobeAMID) ||
		!BindMaterial(FireLobeBMesh, TEXT("FireLobeBMID"), FireLobeBMID) ||
		!BindMaterial(SmokeMainMesh, TEXT("SmokeMainMID"), SmokeMainMID) ||
		!BindMaterial(SmokeCapMesh, TEXT("SmokeCapMID"), SmokeCapMID))
	{
		OutFailureReason = TEXT("Failed to create one or more dynamic materials for the Blender3D runtime proxy.");
		return false;
	}

	UpdateBlenderProxy(0.f);

	const UStaticMeshComponent* MeshComponents[] = { FireCoreMesh, FireShellMesh, FireLobeAMesh, FireLobeBMesh, SmokeMainMesh, SmokeCapMesh };
	for (const UStaticMeshComponent* MeshComponent : MeshComponents)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[FIRE IDOL TEST] CandidateBound Req=%d Candidate=%s Actor=%s Component=%s ComponentClass=%s Mesh=%s Material=%s SourceBlend=%s Limitation=Runtime 3D proxy because the Blender source has no imported Unreal asset."),
			DebugRequestId,
			GetCandidateName(Candidate),
			*GetName(),
			MeshComponent ? *MeshComponent->GetName() : TEXT("None"),
			MeshComponent ? *MeshComponent->GetClass()->GetName() : TEXT("None"),
			ProxyMesh ? *ProxyMesh->GetPathName() : BlenderProxyMeshPath,
			BlenderProxyBaseMaterial ? *BlenderProxyBaseMaterial->GetPathName() : BlenderProxyMaterialPath,
			BlenderSourceBlendPath);
	}

	return true;
}

bool AT66FireIdolAttackVFX::SetupZibraShockwave(FString& OutFailureReason)
{
	ZibraVolumeAsset = LoadZibraShockwaveAsset();
	if (!ZibraVolumeAsset)
	{
		OutFailureReason = FString::Printf(TEXT("Failed to load Zibra shockwave asset %s"), ZibraShockwaveAssetPath);
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		OutFailureReason = TEXT("GetWorld() returned null during Zibra setup.");
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AZibraVDBActor* ZibraActor = World->SpawnActor<AZibraVDBActor>(AZibraVDBActor::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
	if (!ZibraActor)
	{
		OutFailureReason = TEXT("World->SpawnActor<AZibraVDBActor> returned null.");
		return false;
	}

	SpawnedZibraActor = ZibraActor;
	ZibraActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	UZibraVDBAssetComponent* AssetComponent = ZibraActor->GetAssetComponent();
	UActorComponent* MaterialComponent = FindActorComponentByName(ZibraActor, TEXT("MaterialComponent"));
	UZibraVDBPlaybackComponent* PlaybackComponent = ZibraActor->GetPlaybackComponent();
	if (!AssetComponent || !MaterialComponent || !PlaybackComponent)
	{
		OutFailureReason = TEXT("Spawned Zibra actor is missing one or more runtime components.");
		DestroySpawnedZibraActor();
		return false;
	}

	AssetComponent->SetZibraVDBVolume(ZibraVolumeAsset);
	if (!AssetComponent->HasZibraVDBVolume())
	{
		OutFailureReason = TEXT("Zibra volume assignment failed on AssetComponent.");
		DestroySpawnedZibraActor();
		return false;
	}

	const int32 FrameCount = ZibraVolumeAsset->FrameCount;
	if (FrameCount <= 0)
	{
		OutFailureReason = TEXT("Loaded Zibra volume reports zero frames.");
		DestroySpawnedZibraActor();
		return false;
	}

	constexpr float ZibraShadowIntensity = 0.0f;
	constexpr float ZibraMainStepSize = 1.55f;
	constexpr int32 ZibraMaxMainSteps = 192;
	constexpr float ZibraSelfShadowStepSize = 2.40f;
	constexpr int32 ZibraMaxSelfShadowSteps = 24;
	constexpr bool bZibraAllowSkipDecompression = true;
	constexpr int64 ZibraFilteringMode = 0; // ERayMarchingFiltering::None
	constexpr int64 ZibraVolumeResolutionMode = 1; // ResolutionDownscaleFactors::Half

	(void)SetBoolProperty(MaterialComponent, TEXT("bUseHeterogeneousVolume"), false);
	(void)SetLinearColorProperty(MaterialComponent, TEXT("ScatteringColor"), FLinearColor(0.04f, 0.04f, 0.045f, 1.0f));
	(void)SetFloatProperty(MaterialComponent, TEXT("DensityScale"), 0.08f);
	(void)SetFloatProperty(MaterialComponent, TEXT("FlameScale"), 1.10f);
	(void)SetFloatProperty(MaterialComponent, TEXT("TemperatureScale"), 0.60f);
	(void)SetLinearColorProperty(MaterialComponent, TEXT("FlameColor"), FLinearColor(1.10f, 0.55f, 0.08f, 1.0f));
	(void)SetLinearColorProperty(MaterialComponent, TEXT("TemperatureColor"), FLinearColor(0.90f, 0.25f, 0.05f, 1.0f));
	(void)SetFloatProperty(MaterialComponent, TEXT("DirectionalLightBrightness"), 0.0f);
	(void)SetFloatProperty(MaterialComponent, TEXT("PointLightBrightness"), 0.0f);
	(void)SetFloatProperty(MaterialComponent, TEXT("SpotLightBrightness"), 0.0f);
	(void)SetFloatProperty(MaterialComponent, TEXT("RectLightBrightness"), 0.0f);
	(void)SetFloatProperty(MaterialComponent, TEXT("AmbientLightIntensity"), 0.0f);
	(void)SetFloatProperty(MaterialComponent, TEXT("ShadowIntensity"), ZibraShadowIntensity);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnableDirectionalLightShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnablePointLightShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnableSpotLightShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnableRectLightShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnableDirectionalLightReceivedShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnablePointLightReceivedShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnableSpotLightReceivedShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnableRectLightReceivedShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("SmoothReceivedShadows"), false);
	(void)SetFloatProperty(MaterialComponent, TEXT("RayMarchingMainStepSize"), ZibraMainStepSize);
	(void)SetIntProperty(MaterialComponent, TEXT("RayMarchingMaxMainSteps"), ZibraMaxMainSteps);
	(void)SetFloatProperty(MaterialComponent, TEXT("RayMarchingSelfShadowStepSize"), ZibraSelfShadowStepSize);
	(void)SetIntProperty(MaterialComponent, TEXT("RayMarchingMaxSelfShadowSteps"), ZibraMaxSelfShadowSteps);
	(void)SetBoolProperty(MaterialComponent, TEXT("AllowSkipDecompression"), bZibraAllowSkipDecompression);
	(void)SetEnumProperty(MaterialComponent, TEXT("RayMarchingFiltering"), ZibraFilteringMode);
	(void)SetEnumProperty(MaterialComponent, TEXT("VolumeResolution"), ZibraVolumeResolutionMode);
	(void)SetFloatProperty(MaterialComponent, TEXT("ZibraVDBBoundsScale"), 1.6f);
	(void)SetIntProperty(MaterialComponent, TEXT("VolumeTranslucencySortPriority"), 20);

	PlaybackComponent->Animate = true;
	PlaybackComponent->StartFrame = 0.f;
	PlaybackComponent->CurrentFrame = 0.f;
	PlaybackComponent->Framerate = static_cast<float>(FrameCount) / FMath::Max(0.10f, LifetimeSeconds);
	PlaybackComponent->Visible = true;

	const float RadiusScale = BaseRadius / 100.f;
	ZibraActor->SetActorScale3D(FVector(0.22f * RadiusScale));
	ZibraActor->SetActorLocation(GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[FIRE IDOL TEST] CandidateBound Req=%d Candidate=%s Actor=%s SpawnedActor=%s Asset=%s AssetClass=%s ActorClass=%s Components=AssetComponent,MaterialComponent,PlaybackComponent FrameCount=%d Framerate=%.2f Scale=(%.2f,%.2f,%.2f) PerformanceProfile=GameplayBalanced Shadows=Off MainStep=%.2f MaxMainSteps=%d SelfShadowStep=%.2f MaxSelfShadowSteps=%d SkipDecompression=%d Filtering=%lld VolumeResolution=%lld AcceptedReview=%s"),
		DebugRequestId,
		GetCandidateName(Candidate),
		*GetName(),
		*ZibraActor->GetName(),
		*ZibraVolumeAsset->GetPathName(),
		*ZibraVolumeAsset->GetClass()->GetName(),
		*ZibraActor->GetClass()->GetName(),
		FrameCount,
		PlaybackComponent->Framerate,
		ZibraActor->GetActorScale3D().X, ZibraActor->GetActorScale3D().Y, ZibraActor->GetActorScale3D().Z,
		ZibraMainStepSize,
		ZibraMaxMainSteps,
		ZibraSelfShadowStepSize,
		ZibraMaxSelfShadowSteps,
		bZibraAllowSkipDecompression ? 1 : 0,
		ZibraFilteringMode,
		ZibraVolumeResolutionMode,
		ZibraAcceptedReviewPath);

	return true;
}

void AT66FireIdolAttackVFX::HideAllCandidateVisuals()
{
	UStaticMeshComponent* MeshComponents[] = { FireCoreMesh, FireShellMesh, FireLobeAMesh, FireLobeBMesh, SmokeMainMesh, SmokeCapMesh };
	for (UStaticMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent)
		{
			continue;
		}

		MeshComponent->SetHiddenInGame(true);
		MeshComponent->SetVisibility(false, true);
	}
}

void AT66FireIdolAttackVFX::ApplyBaseTransform()
{
	SetActorLocation(EffectLocation + FVector(0.f, 0.f, 8.f));
	SetActorRotation(FRotator::ZeroRotator);
}

void AT66FireIdolAttackVFX::UpdateBlenderProxy(const float Age01)
{
	const float RadiusScale = BaseRadius / 100.f;
	const float LocationScale = 100.f * RadiusScale;

	auto ApplyTrack = [RadiusScale, LocationScale, Age01](UStaticMeshComponent* Mesh, const TConstArrayView<FProxyVectorKey> ScaleKeys, const TConstArrayView<FProxyVectorKey> LocationKeys, const FRotator& Rotation)
	{
		if (!Mesh)
		{
			return;
		}

		Mesh->SetRelativeScale3D(EvaluateProxyKeys(ScaleKeys, Age01) * RadiusScale);
		Mesh->SetRelativeLocation(EvaluateProxyKeys(LocationKeys, Age01) * LocationScale);
		Mesh->SetRelativeRotation(Rotation);
	};

	ApplyTrack(FireCoreMesh, FireCoreScaleKeys, FireCoreLocationKeys, FRotator(0.f, Age01 * 44.f, 0.f));
	ApplyTrack(FireShellMesh, FireShellScaleKeys, FireShellLocationKeys, FRotator(0.f, Age01 * 78.f, 0.f));
	ApplyTrack(FireLobeAMesh, FireLobeAScaleKeys, FireLobeALocationKeys, FRotator(Age01 * 12.f, Age01 * 66.f, 0.f));
	ApplyTrack(FireLobeBMesh, FireLobeBScaleKeys, FireLobeBLocationKeys, FRotator(-Age01 * 14.f, -Age01 * 52.f, 0.f));
	ApplyTrack(SmokeMainMesh, SmokeMainScaleKeys, SmokeMainLocationKeys, FRotator(0.f, Age01 * 20.f, 0.f));
	ApplyTrack(SmokeCapMesh, SmokeCapScaleKeys, SmokeCapLocationKeys, FRotator(0.f, -Age01 * 16.f, 0.f));

	FLinearColor FireTint;
	FLinearColor FirePrimary;
	FLinearColor FireSecondary;
	FLinearColor FireOutline;
	BuildFirePalette(TintColor, FireTint, FirePrimary, FireSecondary, FireOutline);

	FLinearColor SmokeTint;
	FLinearColor SmokePrimary;
	FLinearColor SmokeSecondary;
	FLinearColor SmokeOutline;
	BuildSmokePalette(SmokeTint, SmokePrimary, SmokeSecondary, SmokeOutline);

	UpdateProxyMaterial(FireCoreMID, FireTint, FirePrimary, FireSecondary, FireOutline, 2.05f, Age01);
	UpdateProxyMaterial(FireShellMID, FireTint, FirePrimary, FireSecondary, FireOutline, 1.35f, FMath::Clamp(Age01 * 0.94f, 0.f, 1.f));
	UpdateProxyMaterial(FireLobeAMID, FireTint, FirePrimary, FireSecondary, FireOutline, 1.78f, FMath::Clamp(Age01 * 0.88f, 0.f, 1.f));
	UpdateProxyMaterial(FireLobeBMID, FireTint, FirePrimary, FireSecondary, FireOutline, 1.62f, FMath::Clamp(Age01 * 0.90f, 0.f, 1.f));
	UpdateProxyMaterial(SmokeMainMID, SmokeTint, SmokePrimary, SmokeSecondary, SmokeOutline, 0.42f, FMath::Clamp(Age01 * 0.84f, 0.f, 1.f));
	UpdateProxyMaterial(SmokeCapMID, SmokeTint, SmokePrimary, SmokeSecondary, SmokeOutline, 0.30f, FMath::Clamp(Age01 * 0.76f, 0.f, 1.f));
}

void AT66FireIdolAttackVFX::UpdateProxyMaterial(UMaterialInstanceDynamic* Material, const FLinearColor& Tint, const FLinearColor& Primary, const FLinearColor& Secondary, const FLinearColor& Outline, const float GlowStrength, const float Age01) const
{
	if (!Material)
	{
		return;
	}

	Material->SetVectorParameterValue(TEXT("TintColor"), Tint);
	Material->SetVectorParameterValue(TEXT("PrimaryColor"), Primary);
	Material->SetVectorParameterValue(TEXT("SecondaryColor"), Secondary);
	Material->SetVectorParameterValue(TEXT("OutlineColor"), Outline);
	Material->SetScalarParameterValue(TEXT("GlowStrength"), GlowStrength);
	Material->SetScalarParameterValue(TEXT("Age01"), Age01);
}

void AT66FireIdolAttackVFX::DestroySpawnedZibraActor()
{
	if (AZibraVDBActor* ZibraActor = SpawnedZibraActor.Get())
	{
		SpawnedZibraActor.Reset();
		ZibraActor->Destroy();
	}
}

void AT66FireIdolAttackVFX::LogPlayResult(const TCHAR* EventName, const bool bSuccess, const FString& Details, const TCHAR* FailurePoint, const TCHAR* FallbackPath) const
{
	if (bSuccess)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[FIRE IDOL TEST] %s Req=%d Source=%s Candidate=%s Actor=%s Success=1 FailurePoint=%s Fallback=%s Details=%s"),
			EventName,
			DebugRequestId,
			DebugSourceID.IsNone() ? TEXT("Unknown") : *DebugSourceID.ToString(),
			GetCandidateName(Candidate),
			*GetName(),
			FailurePoint,
			FallbackPath,
			*Details);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[FIRE IDOL TEST] %s Req=%d Source=%s Candidate=%s Actor=%s Success=0 FailurePoint=%s Fallback=%s Details=%s"),
			EventName,
			DebugRequestId,
			DebugSourceID.IsNone() ? TEXT("Unknown") : *DebugSourceID.ToString(),
			GetCandidateName(Candidate),
			*GetName(),
			FailurePoint,
			FallbackPath,
			*Details);
	}
}
