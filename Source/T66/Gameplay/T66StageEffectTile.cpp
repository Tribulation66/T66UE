// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StageEffectTile.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

AT66StageEffectTile::AT66StageEffectTile()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(120.f, 120.f, 60.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = TriggerBox;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Visible "panel" slightly above the ground so it doesn't z-fight or get buried.
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 6.f));
	VisualMesh->SetRelativeScale3D(FVector(2.2f, 1.4f, 0.06f)); // rectangular, thin
	VisualMesh->SetCastShadow(false);

	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->SetStaticMesh(Cube);
	}
}

void AT66StageEffectTile::BeginPlay()
{
	Super::BeginPlay();
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AT66StageEffectTile::OnTriggerBeginOverlap);
	}
	ApplyVisuals();
}

void AT66StageEffectTile::ApplyVisuals()
{
	if (!VisualMesh) return;
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, EffectColor);
}

void AT66StageEffectTile::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;
	if (Hero->IsInSafeZone()) return;

	UWorld* World = GetWorld();
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!World || !RunState) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	if (LastTriggerActor.Get() == OtherActor && (Now - LastTriggerTime) < 0.25f)
	{
		return;
	}
	LastTriggerActor = OtherActor;
	LastTriggerTime = Now;

	const float S = FMath::Clamp(Strength, 0.25f, 3.0f);

	switch (EffectType)
	{
		case ET66StageEffectType::Speed:
		{
			// Short burst of speed so it's immediately felt.
			RunState->ApplyStageSpeedBoost(1.6f + 0.2f * (S - 1.f), 1.25f);
			// Small push forward to emphasize the pad.
			if (UCharacterMovementComponent* Move = Hero->GetCharacterMovement())
			{
				FVector V = Move->Velocity;
				FVector Fwd = Hero->GetActorForwardVector();
				Fwd.Z = 0.f;
				Fwd.Normalize();
				const float Boost = 1100.f + (450.f * (S - 1.f));
				V.X += Fwd.X * Boost;
				V.Y += Fwd.Y * Boost;
				Move->Velocity = V;
			}
			break;
		}
		case ET66StageEffectType::Launch:
		{
			const float LaunchZ = 900.f + (350.f * (S - 1.f));
			Hero->LaunchCharacter(FVector(0.f, 0.f, LaunchZ), true, true);
			break;
		}
		case ET66StageEffectType::Slide:
		{
			RunState->ApplyStageSpeedBoost(1.35f + 0.15f * (S - 1.f), 1.75f);
			Hero->ApplyStageSlide(1.75f);
			break;
		}
		default:
			break;
	}
}

