// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StageEffects.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"

AT66Shroom::AT66Shroom()
{
	PrimaryActorTick.bCanEverTick = false;

	// Side trigger: box around the mushroom body for lateral contacts.
	SideTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("SideTrigger"));
	SideTrigger->SetBoxExtent(FVector(100.f, 100.f, 60.f));
	SideTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SideTrigger->SetGenerateOverlapEvents(true);
	SideTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	SideTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = SideTrigger;

	// Top trigger: sphere above the mushroom cap for jump-landing detection.
	TopTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("TopTrigger"));
	TopTrigger->SetupAttachment(RootComponent);
	TopTrigger->SetSphereRadius(80.f);
	TopTrigger->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	TopTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TopTrigger->SetGenerateOverlapEvents(true);
	TopTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	TopTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ShroomMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShroomMesh"));
	ShroomMesh->SetupAttachment(RootComponent);
	ShroomMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShroomMesh->SetCastShadow(false);
	ShroomMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	ShroomMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 1.5f));

	ShroomMeshOverride = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Shroom.Shroom")));
}

void AT66Shroom::BeginPlay()
{
	Super::BeginPlay();

	// Load Shroom mesh (imported from Shroom.glb), fall back to cube.
	if (!ShroomMeshOverride.IsNull())
	{
		UStaticMesh* Loaded = ShroomMeshOverride.LoadSynchronous();
		if (Loaded)
		{
			ShroomMesh->SetStaticMesh(Loaded);
		}
	}
	if (!ShroomMesh->GetStaticMesh())
	{
		if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
		{
			ShroomMesh->SetStaticMesh(Cube);
			ShroomMesh->SetRelativeScale3D(FVector(1.8f, 1.8f, 2.0f));
		}
		FT66VisualUtil::ApplyT66Color(ShroomMesh, this, FLinearColor(0.55f, 0.15f, 0.65f, 1.f));
	}

	// Load pixel VFX.
	CachedPixelVFX = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"));

	FT66VisualUtil::SnapToGround(this, GetWorld());

	if (SideTrigger)
	{
		SideTrigger->OnComponentBeginOverlap.AddDynamic(this, &AT66Shroom::OnSideTriggerOverlap);
	}
	if (TopTrigger)
	{
		TopTrigger->OnComponentBeginOverlap.AddDynamic(this, &AT66Shroom::OnTopTriggerOverlap);
	}
}

void AT66Shroom::OnTopTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;

	const float Now = static_cast<float>(GetWorld()->GetTimeSeconds());
	if (LastTopActor.Get() == OtherActor && (Now - LastTopTriggerTime) < CooldownSeconds) return;
	LastTopActor = OtherActor;
	LastTopTriggerTime = Now;

	// Only launch if the hero is falling (coming from above).
	UCharacterMovementComponent* Move = Hero->GetCharacterMovement();
	if (!Move) return;
	if (Move->Velocity.Z > 50.f) return;

	// Launch: dramatic upward + forward in the hero's facing direction.
	FVector Fwd = Hero->GetActorForwardVector();
	Fwd.Z = 0.f;
	Fwd.Normalize();
	const FVector LaunchVel(Fwd.X * LaunchForwardVelocity, Fwd.Y * LaunchForwardVelocity, LaunchZVelocity);
	Hero->LaunchCharacter(LaunchVel, true, true);

	SpawnPixelBurst(GetActorLocation() + FVector(0.f, 0.f, 80.f), FLinearColor(0.40f, 0.90f, 0.30f));
}

void AT66Shroom::OnSideTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;

	const float Now = static_cast<float>(GetWorld()->GetTimeSeconds());
	if (LastSideActor.Get() == OtherActor && (Now - LastSideTriggerTime) < CooldownSeconds) return;
	LastSideActor = OtherActor;
	LastSideTriggerTime = Now;

	// Aggressive knockback: push the hero back in the direction they came from.
	UCharacterMovementComponent* Move = Hero->GetCharacterMovement();
	if (!Move) return;

	FVector ToHero = Hero->GetActorLocation() - GetActorLocation();
	ToHero.Z = 0.f;
	if (ToHero.IsNearlyZero()) ToHero = Hero->GetActorForwardVector() * -1.f;
	ToHero.Normalize();

	const FVector KnockbackVel(ToHero.X * KnockbackForce, ToHero.Y * KnockbackForce, KnockbackForce * 0.35f);
	Hero->LaunchCharacter(KnockbackVel, true, true);

	SpawnPixelBurst(Hero->GetActorLocation(), FLinearColor(0.90f, 0.30f, 0.20f));
}

void AT66Shroom::SpawnPixelBurst(const FVector& Location, const FLinearColor& Color) const
{
	if (!CachedPixelVFX || !GetWorld()) return;

	UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(), CachedPixelVFX, Location, FRotator::ZeroRotator,
		FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
	if (NC)
	{
		NC->SetVariableVec4(FName(TEXT("User.Tint")), FVector4(Color.R, Color.G, Color.B, 1.f));
		NC->SetVariableVec2(FName(TEXT("User.SpriteSize")), FVector2D(4.0, 4.0));
	}
}
