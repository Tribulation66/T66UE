// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66VisualUtil.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
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

void FT66VisualUtil::ApplyT66Color(UStaticMeshComponent* Mesh, UObject* Outer, const FLinearColor& Color)
{
	if (!Mesh) return;

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

	// Fallback to whatever material is on the mesh.
	if (UMaterialInstanceDynamic* Mat = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Mat->SetVectorParameterValue(FName("Color"), Color);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
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

