// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LoanShark.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AT66LoanShark::AT66LoanShark()
{
	PrimaryActorTick.bCanEverTick = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = BaseMoveSpeed;
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Align primitive mesh to ground when capsule is grounded:
	// capsule half-height~88, sphere half-height=50*3=150 => relative Z = 150 - 88 = 62.
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 62.f));
	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		VisualMesh->SetStaticMesh(Sphere);
		// Bigger than regular enemies (0.9 cyl) but smaller than boss (6.0 sphere)
		VisualMesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));
	}

	// Force a visible solid color material (avoids relying on engine default material params).
	if (UMaterialInterface* ColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor")))
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, this))
		{
			Mat->SetVectorParameterValue(FName("Color"), FLinearColor(0.1f, 0.45f, 0.9f, 1.f));
			VisualMesh->SetMaterial(0, Mat);
		}
	}
}

void AT66LoanShark::BeginPlay()
{
	Super::BeginPlay();

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &AT66LoanShark::OnCapsuleBeginOverlap);
	}

	// Load tuning from DataTable if present
	if (UWorld* World = GetWorld())
	{
		if (UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr)
		{
			FLoanSharkData Data;
			if (GI->GetLoanSharkData(FName(TEXT("LoanShark")), Data))
			{
				BaseMoveSpeed = Data.BaseMoveSpeed;
				MoveSpeedPer100Debt = Data.MoveSpeedPer100Debt;
				BaseDamageHearts = Data.BaseDamageHearts;
				DebtPerExtraHeart = FMath::Max(1, Data.DebtPerExtraHeart);
			}
		}
	}
	UpdateTuningFromDebt();
}

void AT66LoanShark::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	// If debt was paid off, GameMode will destroy us; still keep behavior safe.
	UpdateTuningFromDebt();

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	// Safe zone rule: cannot enter NPC safe bubbles. If inside, run away.
	for (TActorIterator<AT66HouseNPCBase> It(GetWorld()); It; ++It)
	{
		AT66HouseNPCBase* NPC = *It;
		if (!NPC) continue;
		const FVector N = NPC->GetActorLocation();
		FVector ToMe = GetActorLocation() - N;
		ToMe.Z = 0.f;
		const float Dist = ToMe.Size();
		if (Dist < NPC->GetSafeZoneRadius())
		{
			if (Dist > 1.f)
			{
				ToMe /= Dist;
				AddMovementInput(ToMe, 1.f);
			}
			return;
		}
	}

	// Chase player
	FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	const float Len = ToPlayer.Size();
	if (Len > 10.f)
	{
		ToPlayer /= Len;
		AddMovementInput(ToPlayer, 1.f);
	}
}

void AT66LoanShark::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;
	if (Hero->IsInSafeZone()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;
	if (RunState->GetCurrentDebt() <= 0) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	if (Now - LastTouchDamageTime < TouchDamageCooldown) return;

	LastTouchDamageTime = Now;
	RunState->ApplyDamage(CurrentDamageHearts);
}

UT66RunStateSubsystem* AT66LoanShark::GetRunState() const
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}

void AT66LoanShark::UpdateTuningFromDebt()
{
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	const int32 Debt = RunState->GetCurrentDebt();
	if (Debt == CachedDebt) return;
	CachedDebt = Debt;

	const float ExtraSpeed = (MoveSpeedPer100Debt * (static_cast<float>(Debt) / 100.f));
	const float NewSpeed = FMath::Max(150.f, BaseMoveSpeed + ExtraSpeed);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = NewSpeed;
	}

	const int32 ExtraHearts = (DebtPerExtraHeart > 0) ? (Debt / DebtPerExtraHeart) : 0;
	CurrentDamageHearts = FMath::Clamp(BaseDamageHearts + ExtraHearts, 1, 999);
}

