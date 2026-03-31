// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66IdolVolumeAttackVFX.h"

#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "UObject/UnrealType.h"
#include "ZibraVDBActor.h"
#include "ZibraVDBAssetComponent.h"
#include "ZibraVDBPlaybackComponent.h"
#include "ZibraVDBVolume4.h"

namespace
{
	struct FT66IdolVolumeProfile
	{
		const TCHAR* Name = TEXT("None");
		const TCHAR* AssetPath = nullptr;
		float Lifetime = 1.0f;
		float BaseScale = 0.20f;
		float BoundsScale = 1.6f;
		float MaxDrawDistance = 1800.f;
		float DensityScale = 0.08f;
		float FlameScale = 0.0f;
		float TemperatureScale = 0.0f;
		FLinearColor ScatteringColor = FLinearColor::White;
		FLinearColor AbsorptionColor = FLinearColor::White;
		FLinearColor FlameColor = FLinearColor::Black;
		FLinearColor TemperatureColor = FLinearColor::Black;
		bool bEnableEmissionMasking = false;
		float MaskCenter = 0.1f;
		float MaskWidth = 0.1f;
		float MaskIntensity = 1.0f;
		float MaskRamp = 1.0f;
	};

	const TCHAR* GroundExplosionAssetPath = TEXT("/Game/VFX/Idols/Fire/Zibra/Volumes/GroundExplosion.GroundExplosion");
	const TCHAR* ShockwaveExplosionAssetPath = TEXT("/Game/VFX/Idols/Fire/Zibra/Volumes/ShockwaveExplosion.ShockwaveExplosion");
	FT66IdolVolumeProfile ResolveProfile(const FName IdolID, const ET66AttackCategory Category)
	{
		if (Category == ET66AttackCategory::AOE)
		{
			if (IdolID == TEXT("Idol_Lava"))
			{
				return {TEXT("LavaGroundBurst"), GroundExplosionAssetPath, 1.05f, 0.25f, 1.7f, 1800.f, 0.18f, 1.35f, 0.78f,
					FLinearColor(0.06f, 0.05f, 0.05f, 1.f), FLinearColor(0.20f, 0.12f, 0.06f, 1.f), FLinearColor(1.18f, 0.48f, 0.06f, 1.f),
					FLinearColor(0.95f, 0.18f, 0.03f, 1.f), false};
			}
			if (IdolID == TEXT("Idol_Earth"))
			{
				return {TEXT("EarthDustBurst"), GroundExplosionAssetPath, 0.95f, 0.24f, 1.7f, 1700.f, 0.16f, 0.0f, 0.0f,
					FLinearColor(0.56f, 0.43f, 0.24f, 1.f), FLinearColor(0.45f, 0.34f, 0.18f, 1.f), FLinearColor::Black, FLinearColor::Black, false};
			}
			if (IdolID == TEXT("Idol_Water"))
			{
				return {TEXT("WaterShockwave"), ShockwaveExplosionAssetPath, 0.90f, 0.22f, 1.6f, 1700.f, 0.05f, 0.0f, 0.20f,
					FLinearColor(0.16f, 0.34f, 0.64f, 1.f), FLinearColor(0.08f, 0.16f, 0.28f, 1.f), FLinearColor::Black,
					FLinearColor(0.24f, 0.68f, 1.00f, 1.f), false};
			}
			if (IdolID == TEXT("Idol_Sand"))
			{
				return {TEXT("SandDustBurst"), GroundExplosionAssetPath, 0.98f, 0.29f, 1.95f, 1750.f, 0.15f, 0.0f, 0.0f,
					FLinearColor(0.72f, 0.60f, 0.32f, 1.f), FLinearColor(0.34f, 0.26f, 0.12f, 1.f), FLinearColor::Black,
					FLinearColor::Black, false};
			}
			if (IdolID == TEXT("Idol_Storm"))
			{
				return {TEXT("StormShockwave"), ShockwaveExplosionAssetPath, 0.96f, 0.27f, 1.85f, 1800.f, 0.08f, 0.0f, 0.16f,
					FLinearColor(0.38f, 0.46f, 0.58f, 1.f), FLinearColor(0.16f, 0.20f, 0.30f, 1.f), FLinearColor::Black,
					FLinearColor(0.78f, 0.88f, 1.00f, 1.f), false};
			}
		}

		if (Category == ET66AttackCategory::DOT)
		{
			if (IdolID == TEXT("Idol_Curse"))
			{
				return {TEXT("CurseMiasma"), GroundExplosionAssetPath, 1.35f, 0.16f, 1.38f, 1450.f, 0.08f, 0.0f, 0.06f,
					FLinearColor(0.30f, 0.16f, 0.40f, 1.f), FLinearColor(0.10f, 0.04f, 0.15f, 1.f), FLinearColor::Black,
					FLinearColor(0.86f, 0.58f, 1.00f, 1.f), false};
			}
			if (IdolID == TEXT("Idol_Lava"))
			{
				return {TEXT("LavaBurnCloud"), GroundExplosionAssetPath, 1.20f, 0.17f, 1.32f, 1450.f, 0.12f, 0.78f, 0.34f,
					FLinearColor(0.08f, 0.05f, 0.05f, 1.f), FLinearColor(0.20f, 0.10f, 0.06f, 1.f), FLinearColor(1.10f, 0.42f, 0.08f, 1.f),
					FLinearColor(0.98f, 0.20f, 0.04f, 1.f), false};
			}
			if (IdolID == TEXT("Idol_Poison"))
			{
				return {TEXT("PoisonSmog"), GroundExplosionAssetPath, 1.40f, 0.17f, 1.36f, 1450.f, 0.11f, 0.0f, 0.10f,
					FLinearColor(0.24f, 0.46f, 0.14f, 1.f), FLinearColor(0.08f, 0.16f, 0.05f, 1.f), FLinearColor::Black,
					FLinearColor(0.68f, 0.98f, 0.54f, 1.f), false};
			}
			if (IdolID == TEXT("Idol_Decay"))
			{
				return {TEXT("DecaySpores"), GroundExplosionAssetPath, 1.35f, 0.16f, 1.34f, 1450.f, 0.10f, 0.0f, 0.06f,
					FLinearColor(0.40f, 0.40f, 0.20f, 1.f), FLinearColor(0.14f, 0.14f, 0.07f, 1.f), FLinearColor::Black,
					FLinearColor(0.72f, 0.84f, 0.42f, 1.f), false};
			}
			if (IdolID == TEXT("Idol_Acid"))
			{
				return {TEXT("AcidFume"), GroundExplosionAssetPath, 1.28f, 0.17f, 1.34f, 1450.f, 0.12f, 0.0f, 0.14f,
					FLinearColor(0.48f, 0.54f, 0.10f, 1.f), FLinearColor(0.16f, 0.20f, 0.03f, 1.f), FLinearColor::Black,
					FLinearColor(0.92f, 1.00f, 0.40f, 1.f), false};
			}
		}

		return {};
	}

	UZibraVDBVolume4* LoadVolumeAsset(const TCHAR* AssetPath)
	{
		return AssetPath ? LoadObject<UZibraVDBVolume4>(nullptr, AssetPath) : nullptr;
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

	FVector ResolveTargetBodyAnchor(AActor* Actor)
	{
		if (!Actor)
		{
			return FVector::ZeroVector;
		}

		FVector Anchor = Actor->GetActorLocation();
		if (const UCapsuleComponent* Capsule = Actor->FindComponentByClass<UCapsuleComponent>())
		{
			Anchor.Z += Capsule->GetScaledCapsuleHalfHeight() * 0.55f;
		}
		else
		{
			Anchor.Z += 54.f;
		}
		return Anchor;
	}

	float ResolveEditorPreviewFraction(const FT66IdolVolumeProfile& Profile)
	{
		if (Profile.AssetPath == ShockwaveExplosionAssetPath)
		{
			return 0.05f;
		}
		if (Profile.AssetPath == GroundExplosionAssetPath)
		{
			return 0.08f;
		}
		return 0.06f;
	}
}

AT66IdolVolumeAttackVFX::AT66IdolVolumeAttackVFX()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	InitialLifeSpan = 3.0f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

void AT66IdolVolumeAttackVFX::InitEffect(
	const FName InIdolID,
	const ET66AttackCategory InCategory,
	AActor* InFollowTarget,
	const FVector& InLocation,
	const float InRadius,
	const float InDuration,
	const FLinearColor& InTint,
	const int32 InDebugRequestId)
{
	IdolID = InIdolID;
	Category = InCategory;
	FollowTarget = InFollowTarget;
	EffectLocation = InLocation;
	BaseRadius = FMath::Max(55.f, InRadius);
	TintColor = InTint;
	DebugRequestId = InDebugRequestId;
	bInitialized = true;

	const FT66IdolVolumeProfile Profile = ResolveProfile(IdolID, Category);
	LifetimeSeconds = InCategory == ET66AttackCategory::DOT
		? FMath::Clamp(FMath::Max(Profile.Lifetime, InDuration), 0.8f, 3.5f)
		: FMath::Max(0.70f, Profile.Lifetime);

	if (UWorld* World = GetWorld(); World && !World->IsGameWorld() && !bSetupSucceeded)
	{
		ApplyBaseTransform();
		FString FailureReason;
		bSetupSucceeded = SetupVolume(FailureReason);
		if (!bSetupSucceeded)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[IDOL VOLUME VFX] EditorPreviewFailed Req=%d Idol=%s Category=%s FailureReason=%s"),
				DebugRequestId,
				*IdolID.ToString(),
				Category == ET66AttackCategory::DOT ? TEXT("DOT") : TEXT("AOE"),
				*FailureReason);
		}
	}
}

bool AT66IdolVolumeAttackVFX::ShouldUseVolume(const FName IdolID, const ET66AttackCategory Category)
{
	return ResolveProfile(IdolID, Category).AssetPath != nullptr;
}

bool AT66IdolVolumeAttackVFX::Preflight(const FName IdolID, const ET66AttackCategory Category, FString& OutBindingSummary, FString& OutFailureReason)
{
	const FT66IdolVolumeProfile Profile = ResolveProfile(IdolID, Category);
	if (!Profile.AssetPath)
	{
		OutBindingSummary = TEXT("No volumetric profile");
		OutFailureReason = TEXT("No Zibra volume profile is registered for this idol/category.");
		return false;
	}

	const UZibraVDBVolume4* Volume = LoadVolumeAsset(Profile.AssetPath);
	const UClass* ActorClass = AZibraVDBActor::StaticClass();
	const bool bReady = Volume && ActorClass;

	OutBindingSummary = FString::Printf(
		TEXT("Profile=%s RuntimeAsset=%s AssetClass=%s ActorClass=%s Category=%s"),
		Profile.Name,
		Volume ? *Volume->GetPathName() : Profile.AssetPath,
		Volume ? *Volume->GetClass()->GetName() : TEXT("None"),
		ActorClass ? *ActorClass->GetName() : TEXT("None"),
		Category == ET66AttackCategory::DOT ? TEXT("DOT") : TEXT("AOE"));

	if (!bReady)
	{
		TArray<FString> MissingItems;
		if (!Volume)
		{
			MissingItems.Add(TEXT("Volume"));
		}
		if (!ActorClass)
		{
			MissingItems.Add(TEXT("ZibraVDBActorClass"));
		}
		OutFailureReason = FString::Printf(TEXT("Missing bindings: %s"), *FString::Join(MissingItems, TEXT(",")));
	}
	else
	{
		OutFailureReason.Reset();
	}

	return bReady;
}

FString AT66IdolVolumeAttackVFX::DescribeProfile(const FName IdolID, const ET66AttackCategory Category)
{
	const FT66IdolVolumeProfile Profile = ResolveProfile(IdolID, Category);
	return Profile.AssetPath ? FString(Profile.Name) : TEXT("None");
}

void AT66IdolVolumeAttackVFX::BeginPlay()
{
	Super::BeginPlay();

	if (!bInitialized)
	{
		LogPlayResult(TEXT("PlayFailed"), false, TEXT("InitEffect was not called before BeginPlay."), TEXT("Init"), TEXT("IdolProcFallback"));
		Destroy();
		return;
	}

	ApplyBaseTransform();

	FString FailureReason;
	bSetupSucceeded = SetupVolume(FailureReason);
	if (!bSetupSucceeded)
	{
		LogPlayResult(TEXT("PlayFailed"), false, FailureReason, TEXT("SetupVolume"), TEXT("IdolProcFallback"));
		Destroy();
		return;
	}

	LogPlayResult(
		TEXT("PlayStarted"),
		true,
		FString::Printf(
			TEXT("Category=%s Profile=%s Location=(%.1f,%.1f,%.1f) Radius=%.1f Lifetime=%.2f"),
			Category == ET66AttackCategory::DOT ? TEXT("DOT") : TEXT("AOE"),
			*DescribeProfile(IdolID, Category),
			GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z,
			BaseRadius,
			LifetimeSeconds));
}

void AT66IdolVolumeAttackVFX::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bSetupSucceeded)
	{
		return;
	}

	ElapsedSeconds += DeltaSeconds;
	if (Category == ET66AttackCategory::DOT)
	{
		ApplyBaseTransform();
	}

	if (!bLoggedActive)
	{
		bLoggedActive = true;
		if (const AZibraVDBActor* ZibraActor = SpawnedZibraActor.Get())
		{
			const UZibraVDBPlaybackComponent* Playback = ZibraActor->GetPlaybackComponent();
			LogPlayResult(
				TEXT("PlayActive"),
				true,
				FString::Printf(
					TEXT("Profile=%s SpawnedZibraActor=%s Animate=%d Visible=%d"),
					*DescribeProfile(IdolID, Category),
					*ZibraActor->GetName(),
					Playback && Playback->Animate ? 1 : 0,
					Playback && Playback->Visible ? 1 : 0));
		}
	}

	if (ElapsedSeconds >= LifetimeSeconds)
	{
		LogPlayResult(
			TEXT("PlayComplete"),
			true,
			FString::Printf(TEXT("Profile=%s Elapsed=%.3f"), *DescribeProfile(IdolID, Category), ElapsedSeconds));
		Destroy();
	}
}

void AT66IdolVolumeAttackVFX::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroySpawnedZibraActor();
	Super::EndPlay(EndPlayReason);
}

bool AT66IdolVolumeAttackVFX::SetupVolume(FString& OutFailureReason)
{
	const FT66IdolVolumeProfile Profile = ResolveProfile(IdolID, Category);
	if (!Profile.AssetPath)
	{
		OutFailureReason = TEXT("No matching Zibra volume profile was found.");
		return false;
	}

	UZibraVDBVolume4* Volume = LoadVolumeAsset(Profile.AssetPath);
	if (!Volume)
	{
		OutFailureReason = FString::Printf(TEXT("Failed to load Zibra volume asset %s"), Profile.AssetPath);
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		OutFailureReason = TEXT("GetWorld() returned null.");
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

	AssetComponent->SetZibraVDBVolume(Volume);
	if (!AssetComponent->HasZibraVDBVolume())
	{
		OutFailureReason = TEXT("Zibra volume assignment failed on AssetComponent.");
		DestroySpawnedZibraActor();
		return false;
	}

	const int32 FrameCount = Volume->FrameCount;
	if (FrameCount <= 0)
	{
		OutFailureReason = TEXT("Loaded Zibra volume reports zero frames.");
		DestroySpawnedZibraActor();
		return false;
	}

	(void)SetBoolProperty(MaterialComponent, TEXT("bUseHeterogeneousVolume"), false);
	(void)SetEnumProperty(MaterialComponent, TEXT("ScatteringColorMode"), 0);
	(void)SetEnumProperty(MaterialComponent, TEXT("FlameColorMode"), 0);
	(void)SetEnumProperty(MaterialComponent, TEXT("TemperatureColorMode"), 0);
	(void)SetLinearColorProperty(MaterialComponent, TEXT("ScatteringColor"), Profile.ScatteringColor);
	(void)SetLinearColorProperty(MaterialComponent, TEXT("AbsorptionColor"), Profile.AbsorptionColor);
	(void)SetFloatProperty(MaterialComponent, TEXT("DensityScale"), Profile.DensityScale);
	(void)SetFloatProperty(MaterialComponent, TEXT("FlameScale"), Profile.FlameScale);
	(void)SetFloatProperty(MaterialComponent, TEXT("TemperatureScale"), Profile.TemperatureScale);
	(void)SetLinearColorProperty(MaterialComponent, TEXT("FlameColor"), Profile.FlameColor);
	(void)SetLinearColorProperty(MaterialComponent, TEXT("TemperatureColor"), Profile.TemperatureColor);
	(void)SetFloatProperty(MaterialComponent, TEXT("DirectionalLightBrightness"), 0.0f);
	(void)SetFloatProperty(MaterialComponent, TEXT("PointLightBrightness"), 0.0f);
	(void)SetFloatProperty(MaterialComponent, TEXT("SpotLightBrightness"), 0.0f);
	(void)SetFloatProperty(MaterialComponent, TEXT("RectLightBrightness"), 0.0f);
	(void)SetFloatProperty(MaterialComponent, TEXT("AmbientLightIntensity"), 0.0f);
	(void)SetFloatProperty(MaterialComponent, TEXT("ShadowIntensity"), 0.0f);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnableDirectionalLightShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnablePointLightShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnableSpotLightShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnableRectLightShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnableDirectionalLightReceivedShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnablePointLightReceivedShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnableSpotLightReceivedShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("EnableRectLightReceivedShadows"), false);
	(void)SetBoolProperty(MaterialComponent, TEXT("SmoothReceivedShadows"), false);
	(void)SetFloatProperty(MaterialComponent, TEXT("RayMarchingMainStepSize"), 1.55f);
	(void)SetIntProperty(MaterialComponent, TEXT("RayMarchingMaxMainSteps"), 192);
	(void)SetFloatProperty(MaterialComponent, TEXT("RayMarchingSelfShadowStepSize"), 2.40f);
	(void)SetIntProperty(MaterialComponent, TEXT("RayMarchingMaxSelfShadowSteps"), 24);
	(void)SetBoolProperty(MaterialComponent, TEXT("AllowSkipDecompression"), true);
	(void)SetEnumProperty(MaterialComponent, TEXT("RayMarchingFiltering"), 0);
	(void)SetEnumProperty(MaterialComponent, TEXT("VolumeResolution"), 1);
	(void)SetFloatProperty(MaterialComponent, TEXT("ZibraVDBBoundsScale"), Profile.BoundsScale);
	(void)SetIntProperty(MaterialComponent, TEXT("VolumeTranslucencySortPriority"), 20);
	(void)SetBoolProperty(MaterialComponent, TEXT("bAllowFrustumCulling"), true);
	(void)SetBoolProperty(MaterialComponent, TEXT("bFullFrustumCulling"), false);
	(void)SetFloatProperty(MaterialComponent, TEXT("MaxDrawDistance"), Profile.MaxDrawDistance);
	(void)SetBoolProperty(MaterialComponent, TEXT("bEnableEmissionMasking"), Profile.bEnableEmissionMasking);
	(void)SetBoolProperty(MaterialComponent, TEXT("bEnableFlameEmissionMasking"), Profile.bEnableEmissionMasking);
	(void)SetBoolProperty(MaterialComponent, TEXT("bEnableTemperatureEmissionMasking"), Profile.bEnableEmissionMasking);
	(void)SetFloatProperty(MaterialComponent, TEXT("MaskCenter"), Profile.MaskCenter);
	(void)SetFloatProperty(MaterialComponent, TEXT("MaskWidth"), Profile.MaskWidth);
	(void)SetFloatProperty(MaterialComponent, TEXT("MaskIntensity"), Profile.MaskIntensity);
	(void)SetFloatProperty(MaterialComponent, TEXT("MaskRamp"), Profile.MaskRamp);

	PlaybackComponent->Animate = true;
	PlaybackComponent->StartFrame = 0.0f;
	PlaybackComponent->CurrentFrame = 0.0f;
	PlaybackComponent->Framerate = static_cast<float>(FrameCount) / FMath::Max(0.10f, LifetimeSeconds);
	PlaybackComponent->Visible = true;

	if (const UWorld* PreviewWorld = GetWorld(); PreviewWorld && !PreviewWorld->IsGameWorld())
	{
		const float PreviewFraction = ResolveEditorPreviewFraction(Profile);
		PlaybackComponent->Animate = false;
		PlaybackComponent->CurrentFrame = FMath::Clamp(static_cast<float>(FrameCount) * PreviewFraction, 0.0f, static_cast<float>(FrameCount - 1));
	}

	const float RadiusScale = BaseRadius / 100.f;
	ZibraActor->SetActorScale3D(FVector(Profile.BaseScale * RadiusScale));
	ZibraActor->SetActorLocation(GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[IDOL VOLUME VFX] Bound Req=%d Idol=%s Category=%s Profile=%s Actor=%s SpawnedActor=%s Asset=%s FrameCount=%d Framerate=%.2f Scale=(%.2f,%.2f,%.2f) Density=%.3f Flame=%.3f Temp=%.3f MaxDrawDistance=%.1f"),
		DebugRequestId,
		*IdolID.ToString(),
		Category == ET66AttackCategory::DOT ? TEXT("DOT") : TEXT("AOE"),
		Profile.Name,
		*GetName(),
		*ZibraActor->GetName(),
		*Volume->GetPathName(),
		FrameCount,
		PlaybackComponent->Framerate,
		ZibraActor->GetActorScale3D().X, ZibraActor->GetActorScale3D().Y, ZibraActor->GetActorScale3D().Z,
		Profile.DensityScale,
		Profile.FlameScale,
		Profile.TemperatureScale,
		Profile.MaxDrawDistance);

	return true;
}

FVector AT66IdolVolumeAttackVFX::ResolveAnchorLocation() const
{
	if (Category == ET66AttackCategory::DOT)
	{
		if (AActor* FollowActor = FollowTarget.Get())
		{
			return ResolveTargetBodyAnchor(FollowActor);
		}
		return EffectLocation + FVector(0.f, 0.f, 52.f);
	}

	return EffectLocation + FVector(0.f, 0.f, 8.f);
}

void AT66IdolVolumeAttackVFX::ApplyBaseTransform()
{
	SetActorLocation(ResolveAnchorLocation());
	SetActorRotation(FRotator::ZeroRotator);
}

void AT66IdolVolumeAttackVFX::DestroySpawnedZibraActor()
{
	if (AZibraVDBActor* ZibraActor = SpawnedZibraActor.Get())
	{
		SpawnedZibraActor.Reset();
		ZibraActor->Destroy();
	}
}

void AT66IdolVolumeAttackVFX::LogPlayResult(const TCHAR* EventName, const bool bSuccess, const FString& Details, const TCHAR* FailurePoint, const TCHAR* FallbackPath) const
{
	if (bSuccess)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[IDOL VOLUME VFX] %s Req=%d Idol=%s Category=%s Profile=%s Actor=%s Success=1 FailurePoint=%s Fallback=%s Details=%s"),
			EventName,
			DebugRequestId,
			*IdolID.ToString(),
			Category == ET66AttackCategory::DOT ? TEXT("DOT") : TEXT("AOE"),
			*DescribeProfile(IdolID, Category),
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
			TEXT("[IDOL VOLUME VFX] %s Req=%d Idol=%s Category=%s Profile=%s Actor=%s Success=0 FailurePoint=%s Fallback=%s Details=%s"),
			EventName,
			DebugRequestId,
			*IdolID.ToString(),
			Category == ET66AttackCategory::DOT ? TEXT("DOT") : TEXT("AOE"),
			*DescribeProfile(IdolID, Category),
			*GetName(),
			FailurePoint,
			FallbackPath,
			*Details);
	}
}
