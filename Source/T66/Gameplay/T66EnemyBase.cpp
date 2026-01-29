// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66EnemyAIController.h"
#include "Gameplay/T66ItemPickup.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AT66EnemyBase::AT66EnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AT66EnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	CurrentHP = MaxHP;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 350.f;
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap); // so projectile overlaps and hits
	}

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Cylinder)
	{
		VisualMesh->SetStaticMesh(Cylinder);
		VisualMesh->SetRelativeScale3D(FVector(0.9f, 0.9f, 1.f));
		UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (Mat)
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.8f, 0.2f, 0.2f, 1.f));
		}
	}

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidget->SetDrawSize(FVector2D(120.f, 12.f));
}

void AT66EnemyBase::BeginPlay()
{
	Super::BeginPlay();
	UpdateHealthBar();

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &AT66EnemyBase::OnCapsuleBeginOverlap);
	}
}

void AT66EnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (CurrentHP <= 0) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) return;

	FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	float Len = ToPlayer.Size();
	if (Len > 10.f)
	{
		ToPlayer /= Len;
		AddMovementInput(ToPlayer, 1.f);
	}
}

void AT66EnemyBase::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;

	UWorld* World = GetWorld();
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return;
	UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	if (!RunState) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	if (Now - LastTouchDamageTime < TouchDamageCooldown) return;

	LastTouchDamageTime = Now;
	RunState->ApplyDamage(1);
}

bool AT66EnemyBase::TakeDamageFromHero(int32 Damage)
{
	if (Damage <= 0 || CurrentHP <= 0) return false;
	CurrentHP = FMath::Max(0, CurrentHP - Damage);
	UpdateHealthBar();
	if (CurrentHP <= 0)
	{
		OnDeath();
		return true;
	}
	return false;
}

void AT66EnemyBase::UpdateHealthBar()
{
	// Health bar is updated via a simple progress bar in the widget.
	// For v0 we use a UserWidget that reads CurrentHP/MaxHP; we'll set the percent on the widget.
	// If no widget class set, we skip. For C++ we can create a simple widget or leave placeholder.
	// The WidgetComponent needs a WidgetClass - we can set it in Blueprint or create a minimal one in C++.
	// For v0 we just ensure the component exists; visual can be a Blueprint widget that binds to GetCurrentHP/GetMaxHP.
}

void AT66EnemyBase::OnDeath()
{
	if (OwningDirector)
	{
		OwningDirector->NotifyEnemyDied(this);
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	if (World && T66GI)
	{
		// Spawn one random item pickup
		TArray<FName> ItemIDs;
		if (UDataTable* ItemsDT = T66GI->GetItemsDataTable())
		{
			ItemIDs = ItemsDT->GetRowNames();
		}
		if (ItemIDs.Num() == 0)
		{
			ItemIDs.Add(FName(TEXT("Item_01")));
			ItemIDs.Add(FName(TEXT("Item_02")));
			ItemIDs.Add(FName(TEXT("Item_03")));
		}
		FName ItemID = ItemIDs[FMath::RandRange(0, ItemIDs.Num() - 1)];
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66ItemPickup* Pickup = World->SpawnActor<AT66ItemPickup>(AT66ItemPickup::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
		if (Pickup)
		{
			Pickup->SetItemID(ItemID);
		}
	}

	Destroy();
}
