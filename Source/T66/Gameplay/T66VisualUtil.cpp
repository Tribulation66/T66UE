// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66VisualUtil.h"

#include "Components/StaticMeshComponent.h"
#include "Components/MeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UMaterialInterface* FT66VisualUtil::GetPlaceholderColorMaterial()
{
	static TObjectPtr<UMaterialInterface> Cached = nullptr;
	if (!Cached)
	{
		Cached = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor"));
	}
	return Cached.Get();
}

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

void FT66VisualUtil::ApplyT66Color(UStaticMeshComponent* Mesh, UObject* Outer, const FLinearColor& Color)
{
	if (!Mesh) return;

	// Reuse an existing MID if one is already assigned.
	if (UMaterialInstanceDynamic* Existing = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0)))
	{
		if (UTexture* WhiteTexture = GetWhiteFallbackTexture())
		{
			Existing->SetTextureParameterValue(TEXT("DiffuseColorMap"), WhiteTexture);
			Existing->SetTextureParameterValue(TEXT("BaseColorTexture"), WhiteTexture);
		}
		Existing->SetVectorParameterValue(FName("Color"), Color);
		Existing->SetVectorParameterValue(TEXT("BaseColor"), Color);
		Existing->SetVectorParameterValue(FName("Tint"), Color);
		return;
	}

	// Prefer our placeholder material so the color parameter name is stable ("Color").
	if (UMaterialInterface* ColorMat = GetPlaceholderColorMaterial())
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, Outer ? Outer : Mesh))
		{
			Mat->SetVectorParameterValue(FName("Color"), Color);
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
			UE_LOG(LogTemp, Warning, TEXT("[UnlitAudit] Actor=%s Comp=%s Slot=%d has engine/null material: %s"),
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
					UE_LOG(LogTemp, Warning, TEXT("[UnlitAudit] Actor=%s Comp=%s Slot=%d uses engine material: %s"),
						*Actor->GetName(), *MC->GetName(), i,
						Mat ? *Mat->GetPathName() : TEXT("(null)"));
					++EngineMatCount;
				}
			}
		}
	}

	if (EngineMatCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UnlitAudit] Found %d engine/null material slots. Run Scripts/ReparentToFBXUnlit.py to fix."), EngineMatCount);
	}
}
