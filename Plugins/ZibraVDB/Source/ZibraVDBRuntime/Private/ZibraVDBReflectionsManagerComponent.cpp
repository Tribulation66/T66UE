// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.
#include "ZibraVDBReflectionsManagerComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ZibraVDBActor.h"
#include "ZibraVDBCommon.h"
#include "ZibraVDBMaterialComponent.h"
#include "ZibraVDBNotifications.h"
#include "ZibraVDBPlaybackComponent.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

UZibraVdbReflectionsManagerComponent::UZibraVdbReflectionsManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), bInitialized(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
	bTickInEditor = true;

	BaseMaterial = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/ZibraVDB/Materials/ZibraVDBReflectionsOverlayMaterial"), nullptr, LOAD_None, nullptr);
	if (!BaseMaterial)
	{
		UE_LOG(LogZibraVDBRuntime, Error, TEXT("ZibraVDBReflectionsOverlayMaterial UMaterialInterface is not found!"));
		return;
	}
}

void UZibraVdbReflectionsManagerComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateRayMarchParams();
	CheckOwnerVisibilityChanged();
}

void UZibraVdbReflectionsManagerComponent::OnRegister()
{
	Super::OnRegister();
	if (bInitialized)
		return;

	for (int i = 0; i < ActorsToApplyReflections.Num(); i++)
	{
		auto Actor = ActorsToApplyReflections[i];
		if (!Actor)
		{
			UE_LOG(LogZibraVDBRuntime, Error, TEXT("Actor to apply reflections is nullptr!"));
			continue;
		}
		ApplyReflectionsMaterial(Actor);
	}
	PreviousActorsToApplyReflections = ActorsToApplyReflections;
	bInitialized = true;

	bool bLumenHardwareRaytracingEnabled =
		IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.Lumen.HardwareRayTracing"))->GetValueOnGameThread() != 0;

	bGDFEnabled =
		IConsoleManager::Get().FindConsoleVariable(TEXT("r.AOGlobalDistanceField"))->GetBool() &&
		IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.GenerateMeshDistanceFields"))->GetValueOnGameThread() != 0 &&
		IConsoleManager::Get().FindConsoleVariable(TEXT("r.DistanceFieldAO"))->GetBool() && !bLumenHardwareRaytracingEnabled;
}

void UZibraVdbReflectionsManagerComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
	for (int i = 0; i < ActorsToApplyReflections.Num(); i++)
	{
		AActor* Actor = ActorsToApplyReflections[i];
		if (Actor)
		{
			RemoveReflectionsMaterial(Actor);
		}
	}
}

void UZibraVdbReflectionsManagerComponent::AddActorToReflectionsList(AActor* Actor)
{
	ActorsToApplyReflections.Add(Actor);
	ApplyReflectionsMaterial(Actor);
	PreviousActorsToApplyReflections = ActorsToApplyReflections;
}

#if WITH_EDITOR
void UZibraVdbReflectionsManagerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraVdbReflectionsManagerComponent, ActorsToApplyReflections))
	{
		for (AActor* PreviousActor : PreviousActorsToApplyReflections)
		{
			if (PreviousActor && !ActorsToApplyReflections.Contains(PreviousActor))
			{
				RemoveReflectionsMaterial(PreviousActor);
			}
		}
		for (int i = 0; i < ActorsToApplyReflections.Num(); i++)
		{
			AActor* Actor = ActorsToApplyReflections[i];
			if (Actor)
			{
				ApplyReflectionsMaterial(Actor);
			}
		}
		PreviousActorsToApplyReflections = ActorsToApplyReflections;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraVdbReflectionsManagerComponent, ReflectionsResolution))
	{
		const auto ZibraVDB = Cast<AZibraVDBActor>(GetOwner());
		const auto MaterialComponent = ZibraVDB->GetMaterialComponent();
		const float NewScale = ConvertDownscaleFactorToScale(ReflectionsResolution);
		MaterialComponent->IlluminatedVolumeResolutionScale = NewScale;
	}
}
#endif

void UZibraVdbReflectionsManagerComponent::CheckOwnerVisibilityChanged()
{
	// TODO this feels quite heavy, maybe we can optimize it
	AZibraVDBActor* Owner = Cast<AZibraVDBActor>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogZibraVDBRuntime, Error, TEXT("Owner is nullptr!"));
		return;
	}

	bool OwnerHidden = Owner->IsHidden() || !Owner->GetPlaybackComponent()->Visible;
#if WITH_EDITOR
	OwnerHidden = OwnerHidden || Owner->IsHiddenEd();
#endif

	if (OwnerHidden)
	{
		auto ActorsContainsReflections = ActorsToApplyReflections.FilterByPredicate(
			[this](const AActor* CurActor)
			{
				if (!CurActor)
				{
					return false;
				}
				TArray<UStaticMeshComponent*> StaticComps;
				CurActor->GetComponents<UStaticMeshComponent>(StaticComps);
				return StaticComps.ContainsByPredicate([this](const UStaticMeshComponent* CurMesh)
					{ return CurMesh->OverlayMaterial && DynamicMaterialInstances.Contains(CurMesh->OverlayMaterial); });
			});
		for (int i = 0; i < ActorsContainsReflections.Num(); i++)
		{
			AActor* Actor = ActorsContainsReflections[i];
			if (Actor)
			{
				RemoveReflectionsMaterial(Actor);
			}
		}
	}
	else
	{
		auto ActorsAbsentReflections = ActorsToApplyReflections.FilterByPredicate(
			[this](const AActor* CurActor)
			{
				if (!CurActor)
				{
					return false;
				}
				TArray<UStaticMeshComponent*> StaticComps;
				CurActor->GetComponents<UStaticMeshComponent>(StaticComps);
				return StaticComps.ContainsByPredicate(
					[](const UStaticMeshComponent* CurMesh) { return !CurMesh->OverlayMaterial; });
			});
		for (int i = 0; i < ActorsAbsentReflections.Num(); i++)
		{
			AActor* Actor = ActorsAbsentReflections[i];
			if (Actor)
			{
				ApplyReflectionsMaterial(Actor);
			}
		}
	}
}

void UZibraVdbReflectionsManagerComponent::UpdateRayMarchParams()
{
	const auto ZibraVDB = Cast<AZibraVDBActor>(GetOwner());
	if (!ZibraVDB)
	{
		return;
	}
	const auto* MaterialComponent = ZibraVDB->GetMaterialComponent();
	if (!MaterialComponent)
	{
		return;
	}

	FZibraVDBMaterialSceneProxy* Proxy = static_cast<FZibraVDBMaterialSceneProxy*>(MaterialComponent->SceneProxy);
	if (!(Proxy && Proxy->GetRenderingResources().IsValid() && Proxy->GetRenderingResources()->ZibraVDBNativeResources.IsValid()))
	{
		return;
	}

	for (auto* MaterialInstance : DynamicMaterialInstances)
	{
		if (!MaterialInstance)
		{
			continue;
		}
		MaterialInstance->SetScalarParameterValue(
			"DensityScale", MaterialComponent->GetDensityScaleInternal() *
								Proxy->GetRenderingResources()->ZibraVDBNativeResources->RenderParameters.DensityChannelScale);
		MaterialInstance->SetScalarParameterValue("Density", MaterialComponent->GetDensityScaleInternal());
		MaterialInstance->SetVectorParameterValue("AbsorptionColor", MaterialComponent->AbsorptionColor);
		MaterialInstance->SetScalarParameterValue("GDFEnabled", bGDFEnabled);
		MaterialInstance->SetScalarParameterValue("MaxGDFRaymarchSteps", MaxGDFRaymarchSteps);
		MaterialInstance->SetScalarParameterValue("MaxVolumeRaymarchSteps", MaxVolumeRaymarchSteps);
		MaterialInstance->SetScalarParameterValue("MinVolumeRaymarchDistance", MinVolumeRaymarchDistance);
	}
}

void UZibraVdbReflectionsManagerComponent::ApplyReflectionsMaterial(AActor* Target)
{
	const auto ZibraVDB = Cast<AZibraVDBActor>(GetOwner());
	const auto MaterialComponent = ZibraVDB->GetMaterialComponent();
	FZibraVDBMaterialSceneProxy* Proxy = static_cast<FZibraVDBMaterialSceneProxy*>(MaterialComponent->SceneProxy);
	TArray<UStaticMeshComponent*> StaticComps;
	Target->GetComponents<UStaticMeshComponent>(StaticComps);
	for (int j = 0; j < StaticComps.Num(); j++)
	{
		UStaticMeshComponent* CurMesh = StaticComps[j];
		if (!CurMesh)
		{
			UE_LOG(LogZibraVDBRuntime, Error, TEXT("StaticMeshComponent is nullptr!"));
			continue;
		}
#if WITH_EDITOR
		// Scene isn't running in editor
		if (!(GetWorld() && GetWorld()->IsGameWorld()))
		{
			if (CurMesh->OverlayMaterial)
			{
				if (DynamicMaterialInstances.Contains(CurMesh->OverlayMaterial))
				{
					UE_LOG(LogZibraVDBRuntime, Error, TEXT("OverlayMaterial is already set!"));
					continue;
				}
				else
				{
					FZibraVDBNotifications::AddNotification(
						LOCTEXT("ActorAlreadyHasOverlayMaterial", "This actor already has overlay material on it. Please remove it first."));
					ActorsToApplyReflections.Remove(Target);
					break;
				}
			}
		}
#endif

		UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, CurMesh);
		DynamicMaterialInstance->SetFlags(RF_Transient);
		DynamicMaterialInstance->SetTextureParameterValue("IlluminationTex", MaterialComponent->GetIlluminatedVolumeTexture());
		DynamicMaterialInstance->SetTextureParameterValue("WorldToBoxTransform", MaterialComponent->GetTransformBuffer());
		CurMesh->OverlayMaterial = DynamicMaterialInstance;
		CurMesh->MarkRenderStateDirty();

		DynamicMaterialInstances.Add(DynamicMaterialInstance);
	}
}

void UZibraVdbReflectionsManagerComponent::RemoveReflectionsMaterial(AActor* Target)
{
	TArray<UStaticMeshComponent*> StaticComps;
	Target->GetComponents<UStaticMeshComponent>(StaticComps);
	for (int j = 0; j < StaticComps.Num(); j++)
	{
		UStaticMeshComponent* CurMesh = StaticComps[j];
		if (!CurMesh)
		{
			UE_LOG(LogZibraVDBRuntime, Error, TEXT("StaticMeshComponent is nullptr!"));
			continue;
		}
		auto OverlayMaterial = CurMesh->OverlayMaterial;
		if (!OverlayMaterial || !DynamicMaterialInstances.Contains(OverlayMaterial))
		{
			UE_LOG(LogZibraVDBRuntime, Error, TEXT("Can't find the OverlayMaterial on mesh!"));
			continue;
		}

		CurMesh->OverlayMaterial = nullptr;
		CurMesh->MarkRenderStateDirty();
		DynamicMaterialInstances.Remove(static_cast<UMaterialInstanceDynamic*>(OverlayMaterial));
	}
}

#undef LOCTEXT_NAMESPACE
