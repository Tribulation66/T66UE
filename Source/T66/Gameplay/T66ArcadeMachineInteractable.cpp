// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ArcadeMachineInteractable.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	UMaterialInterface* T66LoadArcadeAuraMaterial()
	{
		static TObjectPtr<UMaterialInterface> CachedMaterial = nullptr;
		if (!CachedMaterial)
		{
			CachedMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Stylized_VFX_StPack/Materials/M_ShieldTransparent.M_ShieldTransparent"));
		}
		return CachedMaterial.Get();
	}
}

AT66ArcadeMachineInteractable::AT66ArcadeMachineInteractable()
{
	ArcadeData.ArcadeID = FName(TEXT("Arcade_Machine"));
	ArcadeData.DisplayName = NSLOCTEXT("T66.Arcade", "MachineDisplayName", "Arcade Cabinet");
	ArcadeData.InteractionVerb = NSLOCTEXT("T66.Arcade", "MachineInteractVerb", "play arcade");
	ArcadeData.ArcadeClass = ET66ArcadeInteractableClass::PopupArcade;
	ArcadeData.ArcadeGameType = ET66ArcadeGameType::Random;
	ArcadeData.RandomGameTypes = {
		ET66ArcadeGameType::WhackAMole,
		ET66ArcadeGameType::Topwar,
		ET66ArcadeGameType::GoldMiner,
		ET66ArcadeGameType::RuneSwipe,
		ET66ArcadeGameType::CartSwitcher,
		ET66ArcadeGameType::CrystalDash,
		ET66ArcadeGameType::PotionPour,
		ET66ArcadeGameType::RelicStack,
		ET66ArcadeGameType::ShieldParry,
		ET66ArcadeGameType::MimicMemory,
		ET66ArcadeGameType::BombSorter,
		ET66ArcadeGameType::LanternLeap,
		ET66ArcadeGameType::BladeSweep,
	};
	ArcadeData.bConsumeOnSuccess = true;
	ArcadeData.bConsumeOnFailure = true;
	ArcadeData.DisplayMeshScale = FVector(1.1f, 0.75f, 1.85f);
	ArcadeData.DisplayMeshOffset = FVector(0.f, 0.f, 95.f);
	ArcadeData.Tint = FLinearColor(0.14f, 0.36f, 0.92f, 1.f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::ArcadeRoundSeconds, 10.f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::ArcadeStartInterval, 0.70f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::ArcadeEndInterval, 0.24f);
	ArcadeRowID = ArcadeData.ArcadeID;

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(320.f, 320.f, 260.f));
	}

	if (VisualMesh)
	{
		VisualMesh->SetRelativeLocation(ArcadeData.DisplayMeshOffset);
	}

	ApplyRarityVisuals();
}

void AT66ArcadeMachineInteractable::BeginPlay()
{
	Super::BeginPlay();
	CreateProtectionAuraComponents();
}

void AT66ArcadeMachineInteractable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ProtectionAuraSphere)
	{
		TArray<AActor*> OverlappingHeroes;
		ProtectionAuraSphere->GetOverlappingActors(OverlappingHeroes, AT66HeroBase::StaticClass());
		for (AActor* OverlappingActor : OverlappingHeroes)
		{
			if (AT66HeroBase* Hero = Cast<AT66HeroBase>(OverlappingActor))
			{
				Hero->AddSafeZoneOverlap(-1);
			}
		}

		ProtectionAuraSphere->OnComponentBeginOverlap.RemoveDynamic(this, &AT66ArcadeMachineInteractable::HandleProtectionAuraBeginOverlap);
		ProtectionAuraSphere->OnComponentEndOverlap.RemoveDynamic(this, &AT66ArcadeMachineInteractable::HandleProtectionAuraEndOverlap);
		ProtectionAuraSphere->DestroyComponent();
		ProtectionAuraSphere = nullptr;
	}

	if (ProtectionAuraVisualMesh)
	{
		ProtectionAuraVisualMesh->DestroyComponent();
		ProtectionAuraVisualMesh = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AT66ArcadeMachineInteractable::CreateProtectionAuraComponents()
{
	if (!bProtectionAuraEnabled || ProtectionAuraRadius <= KINDA_SMALL_NUMBER || !GetRootComponent())
	{
		return;
	}

	if (!ProtectionAuraSphere)
	{
		ProtectionAuraSphere = NewObject<USphereComponent>(this, TEXT("ProtectionAuraSphere"));
		if (ProtectionAuraSphere)
		{
			AddInstanceComponent(ProtectionAuraSphere);
			ProtectionAuraSphere->SetupAttachment(GetRootComponent());
			ProtectionAuraSphere->RegisterComponent();
		}
	}
	ConfigureProtectionAuraCollision();

	if (!ProtectionAuraVisualMesh)
	{
		ProtectionAuraVisualMesh = NewObject<UStaticMeshComponent>(this, TEXT("ProtectionAuraVisualMesh"));
		if (ProtectionAuraVisualMesh)
		{
			AddInstanceComponent(ProtectionAuraVisualMesh);
			ProtectionAuraVisualMesh->SetupAttachment(GetRootComponent());
			ProtectionAuraVisualMesh->RegisterComponent();
		}
	}
	RefreshProtectionAuraVisual();
}

void AT66ArcadeMachineInteractable::ConfigureProtectionAuraCollision()
{
	if (!ProtectionAuraSphere)
	{
		return;
	}

	ProtectionAuraSphere->SetSphereRadius(ProtectionAuraRadius);
	ProtectionAuraSphere->SetRelativeLocation(FVector::ZeroVector);
	ProtectionAuraSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProtectionAuraSphere->SetGenerateOverlapEvents(true);
	ProtectionAuraSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ProtectionAuraSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ProtectionAuraSphere->SetHiddenInGame(true, true);
	ProtectionAuraSphere->SetVisibility(false, true);
	ProtectionAuraSphere->OnComponentBeginOverlap.RemoveDynamic(this, &AT66ArcadeMachineInteractable::HandleProtectionAuraBeginOverlap);
	ProtectionAuraSphere->OnComponentEndOverlap.RemoveDynamic(this, &AT66ArcadeMachineInteractable::HandleProtectionAuraEndOverlap);
	ProtectionAuraSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66ArcadeMachineInteractable::HandleProtectionAuraBeginOverlap);
	ProtectionAuraSphere->OnComponentEndOverlap.AddDynamic(this, &AT66ArcadeMachineInteractable::HandleProtectionAuraEndOverlap);
}

void AT66ArcadeMachineInteractable::RefreshProtectionAuraVisual()
{
	if (!ProtectionAuraVisualMesh)
	{
		return;
	}

	ProtectionAuraVisualMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCylinder());
	ProtectionAuraVisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProtectionAuraVisualMesh->SetGenerateOverlapEvents(false);
	ProtectionAuraVisualMesh->SetCanEverAffectNavigation(false);
	ProtectionAuraVisualMesh->SetCastShadow(false);
	ProtectionAuraVisualMesh->SetReceivesDecals(false);
	ProtectionAuraVisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 6.f));
	ProtectionAuraVisualMesh->SetRelativeScale3D(FVector(
		FMath::Max(0.01f, ProtectionAuraRadius / 50.f),
		FMath::Max(0.01f, ProtectionAuraRadius / 50.f),
		0.018f));
	ProtectionAuraVisualMesh->SetHiddenInGame(false, true);
	ProtectionAuraVisualMesh->SetVisibility(true, true);

	if (UMaterialInterface* AuraMaterial = T66LoadArcadeAuraMaterial())
	{
		if (UMaterialInstanceDynamic* AuraMID = UMaterialInstanceDynamic::Create(AuraMaterial, this))
		{
			const FLinearColor AuraColor(0.12f, 0.78f, 1.f, 0.30f);
			AuraMID->SetVectorParameterValue(TEXT("Color"), AuraColor);
			AuraMID->SetVectorParameterValue(TEXT("BaseColor"), AuraColor);
			AuraMID->SetVectorParameterValue(TEXT("Tint"), AuraColor);
			AuraMID->SetScalarParameterValue(TEXT("Opacity"), 0.30f);
			AuraMID->SetScalarParameterValue(TEXT("Alpha"), 0.30f);
			ProtectionAuraVisualMesh->SetMaterial(0, AuraMID);
		}
		else
		{
			ProtectionAuraVisualMesh->SetMaterial(0, AuraMaterial);
		}
	}
	else
	{
		FT66VisualUtil::ApplyT66Color(ProtectionAuraVisualMesh, this, FLinearColor(0.12f, 0.78f, 1.f, 0.30f));
	}
}

void AT66ArcadeMachineInteractable::HandleProtectionAuraBeginOverlap(
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

void AT66ArcadeMachineInteractable::HandleProtectionAuraEndOverlap(
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
