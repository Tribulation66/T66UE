// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66VisualUtil.h"

#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/MeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66VisualUtil, Log, All);

static UMaterialInterface* GetEnvironmentUnlitMaterial()
{
	static TObjectPtr<UMaterialInterface> Cached = nullptr;
	if (!Cached)
	{
		Cached = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
	}
	return Cached.Get();
}

static UTexture* GetWhiteFallbackTexture()
{
	static TObjectPtr<UTexture> Cached = nullptr;
	if (!Cached)
	{
		Cached = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
	}
	if (!Cached)
	{
		Cached = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
	}
	return Cached.Get();
}

UMaterialInterface* FT66VisualUtil::GetFlatColorMaterial()
{
	return GetEnvironmentUnlitMaterial();
}

void FT66VisualUtil::ConfigureFlatColorMaterial(UMaterialInstanceDynamic* Material, const FLinearColor& Color)
{
	if (!Material)
	{
		return;
	}

	if (UTexture* WhiteTexture = GetWhiteFallbackTexture())
	{
		Material->SetTextureParameterValue(TEXT("DiffuseColorMap"), WhiteTexture);
		Material->SetTextureParameterValue(TEXT("BaseColorTexture"), WhiteTexture);
	}

	Material->SetVectorParameterValue(FName("Color"), Color);
	Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
	Material->SetVectorParameterValue(FName("Tint"), Color);
}

void FT66VisualUtil::ApplyT66Color(UStaticMeshComponent* Mesh, UObject* Outer, const FLinearColor& Color)
{
	if (!Mesh) return;

	// Reuse an existing MID if one is already assigned.
	if (UMaterialInstanceDynamic* Existing = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0)))
	{
		ConfigureFlatColorMaterial(Existing, Color);
		return;
	}

	if (UMaterialInterface* ColorMat = GetFlatColorMaterial())
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, Outer ? Outer : Mesh))
		{
			ConfigureFlatColorMaterial(Mat, Color);
			Mesh->SetMaterial(0, Mat);
			return;
		}
	}

	// Fallback to M_Environment_Unlit (Unlit, uses Tint parameter).
	if (UMaterialInterface* EnvUnlit = GetEnvironmentUnlitMaterial())
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(EnvUnlit, Outer ? Outer : Mesh))
		{
			if (UTexture* WhiteTexture = GetWhiteFallbackTexture())
			{
				Mat->SetTextureParameterValue(TEXT("DiffuseColorMap"), WhiteTexture);
			}
			Mat->SetVectorParameterValue(FName("Tint"), Color);
			Mesh->SetMaterial(0, Mat);
			return;
		}
	}

	// Last resort fallback to whatever material is on the mesh.
	if (UMaterialInstanceDynamic* Mat = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		if (UTexture* WhiteTexture = GetWhiteFallbackTexture())
		{
			Mat->SetTextureParameterValue(TEXT("DiffuseColorMap"), WhiteTexture);
			Mat->SetTextureParameterValue(TEXT("BaseColorTexture"), WhiteTexture);
		}
		Mat->SetVectorParameterValue(FName("Color"), Color);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
		Mat->SetVectorParameterValue(FName("Tint"), Color);
	}
}

UStaticMesh* FT66VisualUtil::GetBasicShapeCube()
{
	static TObjectPtr<UStaticMesh> Cached = nullptr;
	if (!Cached)
	{
		Cached = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}
	return Cached.Get();
}

UStaticMesh* FT66VisualUtil::GetBasicShapeSphere()
{
	static TObjectPtr<UStaticMesh> Cached = nullptr;
	if (!Cached)
	{
		Cached = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	}
	return Cached.Get();
}

UStaticMesh* FT66VisualUtil::GetBasicShapeCylinder()
{
	static TObjectPtr<UStaticMesh> Cached = nullptr;
	if (!Cached)
	{
		Cached = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	}
	return Cached.Get();
}

UStaticMesh* FT66VisualUtil::GetBasicShapeCone()
{
	static TObjectPtr<UStaticMesh> Cached = nullptr;
	if (!Cached)
	{
		Cached = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
	}
	return Cached.Get();
}

void FT66VisualUtil::GroundMeshToActorOrigin(UStaticMeshComponent* MeshComponent, UStaticMesh* Mesh)
{
	if (!MeshComponent)
	{
		return;
	}

	UStaticMesh* ResolvedMesh = Mesh;
	if (!ResolvedMesh)
	{
		ResolvedMesh = MeshComponent->GetStaticMesh();
	}
	if (!ResolvedMesh)
	{
		return;
	}

	const FBoxSphereBounds Bounds = ResolvedMesh->GetBounds();
	const float ScaleZ = FMath::Abs(MeshComponent->GetRelativeScale3D().Z);
	const float BottomZ = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * ScaleZ;
	const FVector RelativeLocation = MeshComponent->GetRelativeLocation();
	MeshComponent->SetRelativeLocation(FVector(RelativeLocation.X, RelativeLocation.Y, -BottomZ));
}

static bool T66ShouldUseComponentForGrounding(const UPrimitiveComponent* Primitive)
{
	if (!Primitive || !Primitive->IsRegistered())
	{
		return false;
	}

	const ECollisionEnabled::Type CollisionEnabled = Primitive->GetCollisionEnabled();
	const bool bIsVisible = Primitive->IsVisible();
	const bool bHasSolidCollision =
		CollisionEnabled == ECollisionEnabled::QueryAndPhysics ||
		CollisionEnabled == ECollisionEnabled::PhysicsOnly;

	return bIsVisible || bHasSolidCollision;
}

static bool T66TryGetActorGroundingBottomZ(const AActor* Actor, float& OutBottomZ)
{
	if (!Actor)
	{
		return false;
	}

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
	Actor->GetComponents(PrimitiveComponents);

	bool bFoundBottom = false;
	float BottomZ = 0.f;

	for (const UPrimitiveComponent* Primitive : PrimitiveComponents)
	{
		if (!T66ShouldUseComponentForGrounding(Primitive))
		{
			continue;
		}

		const FBoxSphereBounds Bounds = Primitive->CalcBounds(Primitive->GetComponentTransform());
		const float ComponentBottomZ = Bounds.Origin.Z - Bounds.BoxExtent.Z;
		if (!bFoundBottom || ComponentBottomZ < BottomZ)
		{
			bFoundBottom = true;
			BottomZ = ComponentBottomZ;
		}
	}

	if (!bFoundBottom)
	{
		return false;
	}

	OutBottomZ = BottomZ;
	return true;
}

void FT66VisualUtil::SnapToGround(AActor* Actor, UWorld* World)
{
	if (!Actor || !World) return;

	const FVector Here = Actor->GetActorLocation();
	const FVector Start = FVector(Here.X, Here.Y, Here.Z + 8000.f);
	const FVector End   = FVector(Here.X, Here.Y, Here.Z - 16000.f);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66SnapToGround), false, Actor);

	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params) &&
		!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return;
	}

	float CurrentBottomZ = 0.f;
	if (!T66TryGetActorGroundingBottomZ(Actor, CurrentBottomZ))
	{
		Actor->SetActorLocation(Hit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);
		return;
	}

	const float DeltaZ = Hit.ImpactPoint.Z - CurrentBottomZ;
	Actor->AddActorWorldOffset(FVector(0.f, 0.f, DeltaZ), false, nullptr, ETeleportType::TeleportPhysics);
}

static bool IsEngineMaterial(UMaterialInterface* Mat)
{
	if (!Mat) return false;
	return Mat->GetPathName().StartsWith(TEXT("/Engine/"));
}

void FT66VisualUtil::EnsureUnlitMaterials(UMeshComponent* Mesh, UObject* Outer)
{
	// Intentionally log-only. Do NOT replace materials - imported models must keep their
	// native textures. Run Scripts/ReparentToFBXUnlit.py in-editor instead.
	if (!Mesh) return;

	const int32 NumSlots = Mesh->GetNumMaterials();
	for (int32 i = 0; i < NumSlots; ++i)
	{
		UMaterialInterface* Mat = Mesh->GetMaterial(i);
		if (!Mat || IsEngineMaterial(Mat))
		{
			AActor* Owner = Mesh->GetOwner();
			UE_LOG(LogT66VisualUtil, Warning, TEXT("[UnlitAudit] Actor=%s Comp=%s Slot=%d has engine/null material: %s"),
				Owner ? *Owner->GetName() : TEXT("?"),
				*Mesh->GetName(), i,
				Mat ? *Mat->GetPathName() : TEXT("(null)"));
		}
	}
}

void FT66VisualUtil::EnsureAllWorldMeshesUnlit(UWorld* World)
{
	// Log-only audit pass - does NOT replace any materials.
	if (!World) return;

	int32 EngineMatCount = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor) continue;

		TArray<UMeshComponent*> MeshComps;
		Actor->GetComponents<UMeshComponent>(MeshComps);

		for (UMeshComponent* MC : MeshComps)
		{
			if (!MC) continue;
			const int32 NumSlots = MC->GetNumMaterials();
			for (int32 i = 0; i < NumSlots; ++i)
			{
				UMaterialInterface* Mat = MC->GetMaterial(i);
				if (!Mat || IsEngineMaterial(Mat))
				{
					UE_LOG(LogT66VisualUtil, Warning, TEXT("[UnlitAudit] Actor=%s Comp=%s Slot=%d uses engine material: %s"),
						*Actor->GetName(), *MC->GetName(), i,
						Mat ? *Mat->GetPathName() : TEXT("(null)"));
					++EngineMatCount;
				}
			}
		}
	}

	if (EngineMatCount > 0)
	{
		UE_LOG(LogT66VisualUtil, Warning, TEXT("[UnlitAudit] Found %d engine/null material slots. Run Scripts/ReparentToFBXUnlit.py to fix."), EngineMatCount);
	}
}
