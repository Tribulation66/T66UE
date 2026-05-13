// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66GalleryDisplayActor.h"

#include "Core/T66CharacterVisualSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"

AT66GalleryDisplayActor::AT66GalleryDisplayActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	PromptSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PromptSphere"));
	PromptSphere->SetupAttachment(SceneRoot);
	PromptSphere->SetSphereRadius(230.f);
	PromptSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PromptSphere->SetGenerateOverlapEvents(true);
	PromptSphere->SetCollisionObjectType(ECC_WorldDynamic);
	PromptSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PromptSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMeshComponent->SetupAttachment(SceneRoot);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMeshComponent->SetGenerateOverlapEvents(false);
	SkeletalMeshComponent->SetCanEverAffectNavigation(false);
	SkeletalMeshComponent->SetHiddenInGame(true, true);
	SkeletalMeshComponent->SetVisibility(false, true);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComponent->SetupAttachment(SceneRoot);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->SetGenerateOverlapEvents(false);
	StaticMeshComponent->SetCanEverAffectNavigation(false);
	StaticMeshComponent->SetHiddenInGame(true, true);
	StaticMeshComponent->SetVisibility(false, true);
}

void AT66GalleryDisplayActor::ConfigureDisplayVisual(const FName InVisualID, const float InActorScale)
{
	VisualID = InVisualID;
	SetActorScale3D(FVector(FMath::Max(0.05f, InActorScale)));
	ApplyConfiguredVisual();
}

void AT66GalleryDisplayActor::ConfigureInteractionPromptTarget(const FText& InPromptTargetName)
{
	InteractionPromptTargetName = InPromptTargetName;
	RefreshInteractionPrompt();
}

UPrimitiveComponent* AT66GalleryDisplayActor::GetInteractionPromptPrimitive() const
{
	return PromptSphere;
}

bool AT66GalleryDisplayActor::Interact(APlayerController* PC)
{
	if (VisualID != FName(TEXT("Gambler")))
	{
		return false;
	}

	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC)
	{
		return false;
	}

	T66PC->OpenCasinoOverlay();
	T66PC->SwitchCasinoOverlayToGambling();
	return true;
}

void AT66GalleryDisplayActor::BeginPlay()
{
	Super::BeginPlay();

	if (PromptSphere)
	{
		PromptSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66GalleryDisplayActor::OnPromptBeginOverlap);
		PromptSphere->OnComponentEndOverlap.AddDynamic(this, &AT66GalleryDisplayActor::OnPromptEndOverlap);

		if (const APawn* LocalPawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			LocalHeroPromptOverlapCount = PromptSphere->IsOverlappingActor(LocalPawn) ? 1 : 0;
		}
	}

	ApplyConfiguredVisual();
	RefreshInteractionPrompt();
}

void AT66GalleryDisplayActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideInteractionPrompt();

	if (PromptSphere)
	{
		PromptSphere->OnComponentBeginOverlap.RemoveDynamic(this, &AT66GalleryDisplayActor::OnPromptBeginOverlap);
		PromptSphere->OnComponentEndOverlap.RemoveDynamic(this, &AT66GalleryDisplayActor::OnPromptEndOverlap);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66GalleryDisplayActor::ApplyConfiguredVisual()
{
	if (VisualID.IsNone())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UT66CharacterVisualSubsystem* Visuals = GameInstance ? GameInstance->GetSubsystem<UT66CharacterVisualSubsystem>() : nullptr;
	if (Visuals && Visuals->ApplyCharacterVisual(VisualID, SkeletalMeshComponent, nullptr, true, true, true, StaticMeshComponent))
	{
		return;
	}

	if (StaticMeshComponent && !StaticMeshComponent->GetStaticMesh())
	{
		if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
		{
			StaticMeshComponent->SetStaticMesh(Cube);
			StaticMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
			StaticMeshComponent->SetRelativeScale3D(FVector(0.9f, 0.9f, 1.6f));
			FT66VisualUtil::ApplyT66Color(StaticMeshComponent, this, FLinearColor(0.80f, 0.18f, 0.28f, 1.f));
			StaticMeshComponent->SetHiddenInGame(false, true);
			StaticMeshComponent->SetVisibility(true, true);
		}
	}
}

void AT66GalleryDisplayActor::RefreshInteractionPrompt()
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !GetWorld())
	{
		HideInteractionPrompt();
		return;
	}

	if (InteractionPromptTargetName.IsEmpty() || LocalHeroPromptOverlapCount <= 0)
	{
		HideInteractionPrompt();
		return;
	}

	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		T66PC->ShowInteractionPrompt(this, InteractionPromptTargetName);
	}
}

void AT66GalleryDisplayActor::HideInteractionPrompt()
{
	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		T66PC->HideInteractionPrompt(this);
	}
}

bool AT66GalleryDisplayActor::IsLocalHeroActor(const AActor* OtherActor) const
{
	return OtherActor && GetWorld() && OtherActor == UGameplayStatics::GetPlayerPawn(this, 0);
}

void AT66GalleryDisplayActor::OnPromptBeginOverlap(
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

	if (!IsLocalHeroActor(OtherActor))
	{
		return;
	}

	++LocalHeroPromptOverlapCount;
	RefreshInteractionPrompt();
}

void AT66GalleryDisplayActor::OnPromptEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;

	if (!IsLocalHeroActor(OtherActor))
	{
		return;
	}

	LocalHeroPromptOverlapCount = FMath::Max(0, LocalHeroPromptOverlapCount - 1);
	RefreshInteractionPrompt();
}
