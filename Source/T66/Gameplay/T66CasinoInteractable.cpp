// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CasinoInteractable.h"

#include "Core/T66RunStateSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"
#include "Gameplay/T66HeroBase.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/SoftObjectPath.h"

AT66CasinoInteractable::AT66CasinoInteractable()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Casino/Casino.Casino")));

	if (VisualMesh)
	{
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 8.f));
	}

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(GetMinimumInteractionExtent());
	}

	SafeZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SafeZoneSphere"));
	SafeZoneSphere->SetupAttachment(RootComponent);
	SafeZoneSphere->SetSphereRadius(SafeZoneRadius);
	SafeZoneSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SafeZoneSphere->SetGenerateOverlapEvents(true);
	SafeZoneSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	SafeZoneSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ApplyRarityVisuals();
}

void AT66CasinoInteractable::BeginPlay()
{
	Super::BeginPlay();

	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->BossChanged.AddDynamic(this, &AT66CasinoInteractable::HandleBossStateChanged);
	}
	if (SafeZoneSphere)
	{
		SafeZoneSphere->SetSphereRadius(SafeZoneRadius);
		SafeZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66CasinoInteractable::HandleSafeZoneBeginOverlap);
		SafeZoneSphere->OnComponentEndOverlap.AddDynamic(this, &AT66CasinoInteractable::HandleSafeZoneEndOverlap);
	}
	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterCasino(this);
		}
	}

	RefreshInteractionPrompt();
}

void AT66CasinoInteractable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SafeZoneSphere)
	{
		SafeZoneSphere->OnComponentBeginOverlap.RemoveDynamic(this, &AT66CasinoInteractable::HandleSafeZoneBeginOverlap);
		SafeZoneSphere->OnComponentEndOverlap.RemoveDynamic(this, &AT66CasinoInteractable::HandleSafeZoneEndOverlap);
	}
	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterCasino(this);
		}
	}
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->BossChanged.RemoveDynamic(this, &AT66CasinoInteractable::HandleBossStateChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66CasinoInteractable::ApplyRarityVisuals()
{
	if (TryApplyImportedMesh())
	{
		return;
	}

	if (!VisualMesh)
	{
		return;
	}

	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.16f, 0.16f, 0.24f, 1.f));
}

bool AT66CasinoInteractable::ShouldShowInteractionPrompt(const AT66HeroBase* LocalHero) const
{
	if (!LocalHero || !IsLocalHeroOverlapping())
	{
		return false;
	}

	const UT66RunStateSubsystem* RunState = GetRunState();
	return !RunState || !RunState->GetBossActive();
}

bool AT66CasinoInteractable::Interact(APlayerController* PC)
{
	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC)
	{
		return false;
	}

	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		if (RunState->GetBossActive())
		{
			return false;
		}
	}

	T66PC->OpenCasinoOverlay();
	return true;
}

void AT66CasinoInteractable::ConfigureCompactTowerVariant(const float InScaleMultiplier, const float InSafeZoneRadius)
{
	const float ScaleMultiplier = FMath::Max(0.1f, InScaleMultiplier);
	SafeZoneRadius = FMath::Max(0.0f, InSafeZoneRadius);
	SetActorScale3D(FVector(ScaleMultiplier));

	if (SafeZoneSphere)
	{
		SafeZoneSphere->SetSphereRadius(SafeZoneRadius);
	}
}

void AT66CasinoInteractable::HandleBossStateChanged()
{
	RefreshInteractionPrompt();
}

void AT66CasinoInteractable::HandleSafeZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor))
	{
		Hero->AddSafeZoneOverlap(+1);
	}
}

void AT66CasinoInteractable::HandleSafeZoneEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;

	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor))
	{
		Hero->AddSafeZoneOverlap(-1);
	}
}

UT66RunStateSubsystem* AT66CasinoInteractable::GetRunState() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}
