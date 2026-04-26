// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ArcadeAmplifierPickup.h"

#include "Core/T66AudioSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"

AT66ArcadeAmplifierPickup::AT66ArcadeAmplifierPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 45.f;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComponent->SetSphereRadius(125.f);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = SphereComponent;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeScale3D(FVector(0.34f, 0.34f, 0.34f));
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 42.f));

	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->SetStaticMesh(Cube);
	}
}

void AT66ArcadeAmplifierPickup::BeginPlay()
{
	Super::BeginPlay();

	if (SphereComponent)
	{
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AT66ArcadeAmplifierPickup::OnSphereBeginOverlap);
	}

	RefreshVisuals();
}

void AT66ArcadeAmplifierPickup::ConfigureAmplifier(
	const ET66HeroStatType InStatType,
	const int32 InBonusStatPoints,
	const float InDurationSeconds)
{
	StatType = InStatType;
	BonusStatPoints = FMath::Max(1, InBonusStatPoints);
	DurationSeconds = FMath::Max(1.f, InDurationSeconds);
	RefreshVisuals();
}

void AT66ArcadeAmplifierPickup::OnSphereBeginOverlap(
	UPrimitiveComponent* /*OverlappedComponent*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/,
	const FHitResult& /*SweepResult*/)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled())
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ApplyTemporaryPrimaryStatAmplifier(StatType, BonusStatPoints, DurationSeconds);
			UT66AudioSubsystem::PlayEventFromWorldContext(this, FName(TEXT("Arcade.Amplifier.Pickup")), GetActorLocation(), this);
			Destroy();
		}
	}
}

void AT66ArcadeAmplifierPickup::RefreshVisuals()
{
	if (!VisualMesh)
	{
		return;
	}

	FT66VisualUtil::ApplyT66Color(VisualMesh, this, ResolveStatTint());
}

FLinearColor AT66ArcadeAmplifierPickup::ResolveStatTint() const
{
	switch (StatType)
	{
	case ET66HeroStatType::Damage:
		return FLinearColor(0.95f, 0.20f, 0.14f, 1.f);
	case ET66HeroStatType::AttackSpeed:
		return FLinearColor(0.97f, 0.70f, 0.18f, 1.f);
	case ET66HeroStatType::AttackScale:
		return FLinearColor(0.78f, 0.35f, 0.95f, 1.f);
	case ET66HeroStatType::Armor:
		return FLinearColor(0.25f, 0.55f, 0.95f, 1.f);
	case ET66HeroStatType::Evasion:
		return FLinearColor(0.20f, 0.95f, 0.72f, 1.f);
	case ET66HeroStatType::Luck:
		return FLinearColor(0.20f, 0.86f, 0.26f, 1.f);
	case ET66HeroStatType::Speed:
		return FLinearColor(0.18f, 0.95f, 0.95f, 1.f);
	case ET66HeroStatType::Accuracy:
		return FLinearColor(0.95f, 0.95f, 0.24f, 1.f);
	default:
		return FLinearColor(0.95f, 0.95f, 0.95f, 1.f);
	}
}
