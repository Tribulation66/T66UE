// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66SpawnPlateau.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

AT66SpawnPlateau::AT66SpawnPlateau()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlateauMesh"));
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCastShadow(true);
	MeshComponent->SetReceivesDecals(true);

	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		MeshComponent->SetStaticMesh(Cylinder);
		// Engine cylinder: 50 radius, 100 height. Scale to PlateauRadius and PlateauHeight.
		const float RadiusScale = PlateauRadius / 50.f;
		const float HeightScale = PlateauHeight / 100.f;
		MeshComponent->SetRelativeScale3D(FVector(RadiusScale, RadiusScale, HeightScale));
	}
}
