// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66SlashEffect.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

AT66SlashEffect::AT66SlashEffect()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = Lifetime;

	DiscMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DiscMesh"));
	RootComponent = DiscMesh;
	DiscMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DiscMesh->CastShadow = false;
	DiscMesh->SetVisibility(false, true);
	DiscMesh->SetHiddenInGame(true);
	DiscMesh->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> PixelVFX(
		TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"));
	if (PixelVFX.Succeeded())
	{
		PixelVFXSystem = PixelVFX.Object;
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UNiagaraSystem> LegacyVFX(
			TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"));
		if (LegacyVFX.Succeeded())
		{
			PixelVFXSystem = LegacyVFX.Object;
		}
	}
}

void AT66SlashEffect::InitEffect(float InRadius, const FLinearColor& InColor)
{
	EffectRadius = FMath::Max(InRadius, 10.f);
	EffectColor = InColor;
}

void AT66SlashEffect::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World || !PixelVFXSystem)
	{
		return;
	}

	const FLinearColor PixelColor(EffectColor.R, EffectColor.G, EffectColor.B, 1.f);
	const float RingRadius = FMath::Max(16.f, EffectRadius * 0.35f);
	const int32 NumParticles = FMath::Clamp(FMath::RoundToInt(EffectRadius / 16.f), 24, 72);
	const FVector Center = GetActorLocation() + FVector(0.f, 0.f, 4.f);
	const FVector2D SpriteSize(2.5f, 2.5f);

	for (int32 Index = 0; Index < NumParticles; ++Index)
	{
		const float Angle = (2.f * PI * static_cast<float>(Index)) / static_cast<float>(NumParticles);
		const FVector Offset(FMath::Cos(Angle) * RingRadius, FMath::Sin(Angle) * RingRadius, 0.f);
		if (UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			PixelVFXSystem,
			Center + Offset,
			FRotator::ZeroRotator,
			FVector(1.f),
			true,
			true,
			ENCPoolMethod::AutoRelease))
		{
			Niagara->SetVariableLinearColor(FName(TEXT("User.Tint")), PixelColor);
			Niagara->SetVariableLinearColor(FName(TEXT("User.Color")), PixelColor);
			Niagara->SetVariableVec2(FName(TEXT("User.SpriteSize")), SpriteSize);
			Niagara->SetAutoDestroy(true);
		}
	}
}
