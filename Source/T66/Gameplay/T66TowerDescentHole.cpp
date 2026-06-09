// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TowerDescentHole.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66WeaponManagerSubsystem.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SceneComponent.h"

namespace
{
	static const TCHAR* T66TowerGateTexturePath = TEXT("/Game/World/Tower/Textures/T_TowerDescentGate_Closed.T_TowerDescentGate_Closed");
	static const FName T66TowerDescentNoSurfaceBounceTag(TEXT("T66_NoSurfaceBounce"));
	static constexpr float T66TowerDescentTriggerTopBelowSurface = 180.0f;

	static UStaticMesh* T66GetGateCoverMesh()
	{
		if (UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane")))
		{
			return Plane;
		}
		return FT66VisualUtil::GetBasicShapeCube();
	}

	static void T66ApplyGateCoverMaterial(UStaticMeshComponent* Mesh, UObject* Outer)
	{
		if (!Mesh)
		{
			return;
		}

		UTexture* GateTexture = LoadObject<UTexture>(nullptr, T66TowerGateTexturePath);
		UMaterialInterface* BaseMaterial = FT66VisualUtil::GetFlatColorMaterial();
		if (BaseMaterial && GateTexture)
		{
			if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Outer ? Outer : Mesh))
			{
				Material->SetTextureParameterValue(TEXT("DiffuseColorMap"), GateTexture);
				Material->SetTextureParameterValue(TEXT("BaseColorTexture"), GateTexture);
				Material->SetVectorParameterValue(TEXT("Color"), FLinearColor::White);
				Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
				Material->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
				Mesh->SetMaterial(0, Material);
				return;
			}
		}

		FT66VisualUtil::ApplyT66Color(Mesh, Outer, FLinearColor(0.12f, 0.10f, 0.12f, 1.f));
	}
}

AT66TowerDescentHole::AT66TowerDescentHole()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetBoxExtent(FVector(900.0f, 900.0f, 1200.0f));

	GateCoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateCoverMesh"));
	GateCoverMesh->SetupAttachment(RootComponent);
	GateCoverMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GateCoverMesh->SetCollisionObjectType(ECC_WorldStatic);
	GateCoverMesh->SetCollisionResponseToAllChannels(ECR_Block);
	GateCoverMesh->SetGenerateOverlapEvents(false);
	GateCoverMesh->SetCanEverAffectNavigation(false);
	GateCoverMesh->ComponentTags.AddUnique(T66TowerDescentNoSurfaceBounceTag);
	if (UStaticMesh* CoverMesh = T66GetGateCoverMesh())
	{
		GateCoverMesh->SetStaticMesh(CoverMesh);
	}
	T66ApplyGateCoverMaterial(GateCoverMesh, this);

	Tags.AddUnique(T66TowerDescentNoSurfaceBounceTag);
}

void AT66TowerDescentHole::InitializeHole(
	const int32 InFromFloorNumber,
	const int32 InToFloorNumber,
	const FVector& InTriggerExtent,
	const bool bInRequiresWeaponSelection,
	const bool bInRequiresIdolSelection,
	const bool bInRequiresGuardianDefeated)
{
	FromFloorNumber = InFromFloorNumber;
	ToFloorNumber = InToFloorNumber;
	bRequiresWeaponSelection = bInRequiresWeaponSelection;
	bRequiresIdolSelection = bInRequiresIdolSelection;
	bIdolSelectionSatisfied = !bRequiresIdolSelection;
	bRequiresGuardianDefeated = bInRequiresGuardianDefeated;
	bGateOpen = false;
	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(InTriggerExtent);
		TriggerBox->SetRelativeLocation(FVector(0.0f, 0.0f, -InTriggerExtent.Z - T66TowerDescentTriggerTopBelowSurface));
	}
	if (GateCoverMesh)
	{
		const float CoverHalfX = FMath::Max(250.0f, InTriggerExtent.X * 1.05f);
		const float CoverHalfY = FMath::Max(250.0f, InTriggerExtent.Y * 1.05f);
		GateCoverMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
		GateCoverMesh->SetRelativeScale3D(FVector(CoverHalfX / 50.0f, CoverHalfY / 50.0f, 1.0f));
	}
	RefreshGateVisualState();
}

void AT66TowerDescentHole::SetGuardianEnemy(AT66EnemyBase* InGuardianEnemy)
{
	GuardianEnemy = InGuardianEnemy;
}

void AT66TowerDescentHole::NotifyIdolSelectionSatisfied()
{
	if (!bRequiresIdolSelection)
	{
		return;
	}

	bIdolSelectionSatisfied = true;
}

bool AT66TowerDescentHole::Interact(AT66HeroBase* Hero)
{
	if (!Hero || bGateOpen || !CanOpenGate(Hero))
	{
		return false;
	}

	bGateOpen = true;
	RefreshGateVisualState();
	if (AT66GameMode* GameMode = GetWorld() ? Cast<AT66GameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		GameMode->HandleTowerDescentGateOpened(FromFloorNumber, ToFloorNumber);
	}
	return true;
}

#if !UE_BUILD_SHIPPING
bool AT66TowerDescentHole::AutomationCanOpenForHero(const AT66HeroBase* Hero) const
{
	return CanOpenGate(Hero);
}

AT66EnemyBase* AT66TowerDescentHole::AutomationGetGuardianEnemy() const
{
	return GuardianEnemy.Get();
}
#endif

void AT66TowerDescentHole::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AT66TowerDescentHole::OnTriggerBeginOverlap);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AT66TowerDescentHole::OnTriggerEndOverlap);
	}
	RefreshGateVisualState();
}

void AT66TowerDescentHole::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero)
	{
		return;
	}

	if (!bGateOpen)
	{
		return;
	}

	TriggerDescentForHero(Hero);
}

void AT66TowerDescentHole::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	ActiveActors.Remove(TWeakObjectPtr<AActor>(OtherActor));
}

void AT66TowerDescentHole::RefreshGateVisualState()
{
	if (!GateCoverMesh)
	{
		return;
	}

	GateCoverMesh->SetHiddenInGame(bGateOpen, true);
	GateCoverMesh->SetVisibility(!bGateOpen, true);
	GateCoverMesh->SetCollisionEnabled(bGateOpen ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
}

bool AT66TowerDescentHole::CanOpenGate(const AT66HeroBase* Hero) const
{
	if (!Hero)
	{
		return false;
	}

	if (bRequiresWeaponSelection)
	{
		UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
		const UT66WeaponManagerSubsystem* WeaponManager = GI ? GI->GetSubsystem<UT66WeaponManagerSubsystem>() : nullptr;
		if (!WeaponManager || WeaponManager->GetEquippedWeaponID().IsNone())
		{
			return false;
		}
	}

	if (bRequiresIdolSelection && !bIdolSelectionSatisfied)
	{
		return false;
	}

	if (bRequiresGuardianDefeated && !IsGuardianDefeated())
	{
		return false;
	}

	return true;
}

bool AT66TowerDescentHole::IsGuardianDefeated() const
{
	const AT66EnemyBase* Guardian = GuardianEnemy.Get();
	return !Guardian || Guardian->CurrentHP <= 0 || Guardian->IsHidden();
}

void AT66TowerDescentHole::TriggerDescentForHero(AT66HeroBase* Hero)
{
	if (!Hero)
	{
		return;
	}

	TWeakObjectPtr<AActor> WeakActor(Hero);
	if (ActiveActors.Contains(WeakActor))
	{
		return;
	}

	ActiveActors.Add(WeakActor);

	if (AT66GameMode* GameMode = GetWorld() ? Cast<AT66GameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		GameMode->HandleTowerDescentHoleTriggered(Hero, FromFloorNumber, ToFloorNumber);
	}
}
