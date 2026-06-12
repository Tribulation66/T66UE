// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LootPinata.h"

#include "Core/T66GameInstance.h"
#include "Core/T66RngSubsystem.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInterface.h"

AT66LootPinata::AT66LootPinata()
{
	PrimaryActorTick.bCanEverTick = false;
	MaxHealth = 80;
	CurrentHealth = MaxHealth;
	LootBagCount = 8;
	LootExplosionRadius = 260.f;
	LootSpawnHeight = 90.f;
	bRollLootRarity = true;
	FixedLootRarity = ET66Rarity::Yellow;
	Rarity = ET66Rarity::Yellow;

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(180.f, 180.f, 180.f));
	}

	if (VisualMesh)
	{
		if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
		{
			VisualMesh->SetStaticMesh(Sphere);
			VisualMesh->SetRelativeScale3D(FVector(1.05f, 1.05f, 1.25f));
			VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 105.f));
		}
		VisualMesh->SetGenerateOverlapEvents(false);
	}

	LockIndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockIndicatorMesh"));
	LockIndicatorMesh->SetupAttachment(RootComponent);
	LockIndicatorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LockIndicatorMesh->SetCanEverAffectNavigation(false);
	LockIndicatorMesh->SetHiddenInGame(true);
	LockIndicatorMesh->SetVisibility(false, true);
	LockIndicatorMesh->SetRelativeLocation(FVector(0.f, 0.f, 260.f));
	LockIndicatorMesh->SetRelativeScale3D(FVector(0.28f, 0.28f, 0.08f));
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		LockIndicatorMesh->SetStaticMesh(Sphere);
	}
}

void AT66LootPinata::BeginPlay()
{
	CurrentHealth = FMath::Max(1, MaxHealth);
	Super::BeginPlay();
	SetLockedIndicator(false);
}

void AT66LootPinata::ApplyRarityVisuals()
{
	Super::ApplyRarityVisuals();
	if (LockIndicatorMesh)
	{
		FT66VisualUtil::ApplyT66Color(LockIndicatorMesh, this, FLinearColor(1.f, 0.86f, 0.12f, 1.f));
	}
}

bool AT66LootPinata::ShouldShowInteractionPrompt(const AT66HeroBase* /*LocalHero*/) const
{
	return false;
}

bool AT66LootPinata::Interact(APlayerController* /*PC*/)
{
	return false;
}

bool AT66LootPinata::IsAliveAndTargetable() const
{
	return !bConsumed && CurrentHealth > 0 && !IsActorBeingDestroyed();
}

FT66CombatTargetHandle AT66LootPinata::ResolveCombatTargetHandle(UPrimitiveComponent* PreferredComponent, ET66HitZoneType PreferredHitZone) const
{
	FT66CombatTargetHandle Handle;
	Handle.Actor = const_cast<AT66LootPinata*>(this);
	Handle.HitComponent = PreferredComponent ? PreferredComponent : Cast<UPrimitiveComponent>(VisualMesh.Get());
	Handle.HitZoneType = (PreferredHitZone == ET66HitZoneType::None) ? ET66HitZoneType::Body : PreferredHitZone;
	Handle.HitZoneName = FName(TEXT("Pinata"));
	Handle.AimPoint = VisualMesh ? VisualMesh->Bounds.Origin : (GetActorLocation() + FVector(0.f, 0.f, 120.f));
	return Handle;
}

void AT66LootPinata::SetLockedIndicator(const bool bLocked)
{
	if (LockIndicatorMesh)
	{
		LockIndicatorMesh->SetHiddenInGame(!bLocked);
		LockIndicatorMesh->SetVisibility(bLocked, true);
	}
}

bool AT66LootPinata::TakeDamageFromHero(const int32 DamageAmount, FName DamageSourceID, FName EventType)
{
	return TakeDamageFromHeroHitZone(DamageAmount, ResolveCombatTargetHandle(), DamageSourceID, EventType);
}

bool AT66LootPinata::TakeDamageFromHeroHitZone(const int32 DamageAmount, const FT66CombatTargetHandle& /*TargetHandle*/, FName /*DamageSourceID*/, FName /*EventType*/)
{
	if (!IsAliveAndTargetable() || DamageAmount <= 0)
	{
		return false;
	}

	CurrentHealth = FMath::Max(0, CurrentHealth - DamageAmount);
	if (CurrentHealth > 0)
	{
		return false;
	}

	OpenPinata();
	return true;
}

void AT66LootPinata::OpenPinata()
{
	if (bConsumed)
	{
		return;
	}

	bConsumed = true;
	CurrentHealth = 0;
	SetLockedIndicator(false);
	SetActorEnableCollision(false);
	if (VisualMesh)
	{
		VisualMesh->SetHiddenInGame(true);
		VisualMesh->SetVisibility(false, true);
	}

	SpawnLootBags();
	SetLifeSpan(0.1f);
}

ET66Rarity AT66LootPinata::RollLootRarity(FRandomStream& Stream) const
{
	return bRollLootRarity ? FT66RarityUtil::RollDefaultRarity(Stream) : FixedLootRarity;
}

void AT66LootPinata::SpawnLootBags()
{
	UWorld* World = GetWorld();
	if (!World || LootBagCount <= 0)
	{
		return;
	}

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance());
	UT66RngSubsystem* RngSub = T66GI ? T66GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	FRandomStream FallbackStream(FMath::Rand());
	FRandomStream& Stream = RngSub ? RngSub->GetRunStream() : FallbackStream;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	const FVector Origin = GetActorLocation();
	const float AngleStep = LootBagCount > 0 ? (2.f * PI) / static_cast<float>(LootBagCount) : 0.f;
	const float MaxRadius = FMath::Max(0.f, LootExplosionRadius);

	for (int32 BagIndex = 0; BagIndex < LootBagCount; ++BagIndex)
	{
		const float AngleJitter = Stream.FRandRange(-0.25f, 0.25f);
		const float Angle = (static_cast<float>(BagIndex) * AngleStep) + AngleJitter;
		const float Radius = LootBagCount > 1 ? Stream.FRandRange(MaxRadius * 0.35f, MaxRadius) : 0.f;
		const FVector Outward(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
		const FVector SpawnLocation = Origin + (Outward * Radius) + FVector(0.f, 0.f, LootSpawnHeight);

		AT66LootBagPickup* LootBag = World->SpawnActor<AT66LootBagPickup>(
			AT66LootBagPickup::StaticClass(),
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams);
		if (!LootBag)
		{
			continue;
		}

		const ET66Rarity BagRarity = RollLootRarity(Stream);
		const FName ItemID = T66GI
			? T66GI->GetRandomItemIDForLootRarityFromStream(BagRarity, Stream)
			: FName(TEXT("Item_AoeDamage"));

		LootBag->SetLootRarity(BagRarity);
		LootBag->SetItemID(ItemID.IsNone() ? FName(TEXT("Item_AoeDamage")) : ItemID);
		if (LootBag->FallMovement)
		{
			LootBag->FallMovement->Velocity = (Outward * Stream.FRandRange(260.f, 520.f)) + FVector(0.f, 0.f, Stream.FRandRange(360.f, 620.f));
		}
	}
}
