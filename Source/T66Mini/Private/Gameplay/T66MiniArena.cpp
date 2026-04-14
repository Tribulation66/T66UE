// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniArena.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	UStaticMesh* T66MiniLoadPlaneMesh()
	{
		static TWeakObjectPtr<UStaticMesh> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
		}

		return Cached.Get();
	}

	UStaticMesh* T66MiniLoadCubeMesh()
	{
		static TWeakObjectPtr<UStaticMesh> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		}

		return Cached.Get();
	}

	UMaterialInterface* T66MiniLoadArenaMaterial()
	{
		static TWeakObjectPtr<UMaterialInterface> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
		}

		return Cached.Get();
	}

	UTexture* T66MiniLoadWhiteTexture()
	{
		static TWeakObjectPtr<UTexture> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
		}

		return Cached.Get();
	}

	void T66MiniConfigureTexturedSurface(UStaticMeshComponent* MeshComponent, UObject* Outer, UTexture* Texture, const FLinearColor& Tint)
	{
		if (!MeshComponent)
		{
			return;
		}

		UMaterialInterface* BaseMaterial = T66MiniLoadArenaMaterial();
		if (!BaseMaterial)
		{
			return;
		}

		UMaterialInstanceDynamic* Material = MeshComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BaseMaterial);
		if (!Material)
		{
			Material = UMaterialInstanceDynamic::Create(BaseMaterial, Outer ? Outer : MeshComponent);
			if (!Material)
			{
				return;
			}

			MeshComponent->SetMaterial(0, Material);
		}

		UTexture* TextureToUse = Texture ? Texture : T66MiniLoadWhiteTexture();
		if (TextureToUse)
		{
			Material->SetTextureParameterValue(TEXT("DiffuseColorMap"), TextureToUse);
			Material->SetTextureParameterValue(TEXT("BaseColorTexture"), TextureToUse);
		}

		Material->SetVectorParameterValue(FName("Color"), Tint);
		Material->SetVectorParameterValue(TEXT("BaseColor"), Tint);
		Material->SetVectorParameterValue(FName("Tint"), Tint);
	}
}

AT66MiniArena::AT66MiniArena()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
	FloorMesh->SetupAttachment(SceneRoot);
	FloorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FloorMesh->SetStaticMesh(T66MiniLoadPlaneMesh());
	FloorMesh->SetCastShadow(false);

	BackdropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Backdrop"));
	BackdropMesh->SetupAttachment(SceneRoot);
	BackdropMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BackdropMesh->SetStaticMesh(T66MiniLoadPlaneMesh());
	BackdropMesh->SetCastShadow(false);

	for (int32 Index = 0; Index < 4; ++Index)
	{
		UStaticMeshComponent* Border = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Border_%d"), Index));
		Border->SetupAttachment(SceneRoot);
		Border->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Border->SetStaticMesh(T66MiniLoadCubeMesh());
		Border->SetCastShadow(false);
		BorderMeshes.Add(Border);
	}
}

void AT66MiniArena::InitializeArena(const FVector& InOrigin, const float InHalfExtent, UTexture2D* InBackgroundTexture)
{
	ArenaOrigin = InOrigin;
	ArenaHalfExtent = InHalfExtent;
	SetActorLocation(ArenaOrigin);

	const float PlaneScale = ArenaHalfExtent / 50.f;
	const float BackdropScale = (ArenaHalfExtent + 1150.f) / 50.f;
	BackdropMesh->SetRelativeScale3D(FVector(BackdropScale, BackdropScale, 1.f));
	BackdropMesh->SetRelativeLocation(FVector(0.f, 0.f, -12.f));
	T66MiniConfigureTexturedSurface(BackdropMesh, this, nullptr, FLinearColor(0.05f, 0.03f, 0.08f, 1.0f));

	FloorMesh->SetRelativeScale3D(FVector(PlaneScale, PlaneScale, 1.f));
	FloorMesh->SetRelativeLocation(FVector(0.f, 0.f, -4.f));
	T66MiniConfigureTexturedSurface(FloorMesh, this, InBackgroundTexture, FLinearColor::White);

	const FVector BorderScale(ArenaHalfExtent / 50.f, 0.26f, 0.18f);
	if (BorderMeshes.Num() >= 4)
	{
		BorderMeshes[0]->SetRelativeLocation(FVector(0.f, ArenaHalfExtent, 8.f));
		BorderMeshes[0]->SetRelativeRotation(FRotator::ZeroRotator);
		BorderMeshes[0]->SetRelativeScale3D(BorderScale);

		BorderMeshes[1]->SetRelativeLocation(FVector(0.f, -ArenaHalfExtent, 8.f));
		BorderMeshes[1]->SetRelativeRotation(FRotator::ZeroRotator);
		BorderMeshes[1]->SetRelativeScale3D(BorderScale);

		BorderMeshes[2]->SetRelativeLocation(FVector(ArenaHalfExtent, 0.f, 8.f));
		BorderMeshes[2]->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
		BorderMeshes[2]->SetRelativeScale3D(BorderScale);

		BorderMeshes[3]->SetRelativeLocation(FVector(-ArenaHalfExtent, 0.f, 8.f));
		BorderMeshes[3]->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
		BorderMeshes[3]->SetRelativeScale3D(BorderScale);
	}

	for (UStaticMeshComponent* BorderMesh : BorderMeshes)
	{
		T66MiniConfigureTexturedSurface(BorderMesh, this, nullptr, FLinearColor(0.10f, 0.08f, 0.12f, 1.0f));
	}
}
