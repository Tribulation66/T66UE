// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66IdolVFXPreviewStage.h"

#include "Core/T66IdolManagerSubsystem.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Gameplay/T66IdolProcVFX.h"
#include "Gameplay/T66IdolVolumeAttackVFX.h"
#include "Gameplay/T66IdolVFXPreviewDummy.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FName PierceIdols[] = {
		TEXT("Idol_Light"), TEXT("Idol_Wind"), TEXT("Idol_Steel"),
		TEXT("Idol_Wood"), TEXT("Idol_Bone"), TEXT("Idol_Glass")
	};

	const FName BounceIdols[] = {
		TEXT("Idol_Electric"), TEXT("Idol_Ice"), TEXT("Idol_Sound"),
		TEXT("Idol_Shadow"), TEXT("Idol_Star"), TEXT("Idol_Rubber")
	};

	const FName AOEIdols[] = {
		TEXT("Idol_Fire"), TEXT("Idol_Earth"), TEXT("Idol_Water"),
		TEXT("Idol_Sand"), TEXT("Idol_BlackHole"), TEXT("Idol_Storm")
	};

	const FName DOTIdols[] = {
		TEXT("Idol_Curse"), TEXT("Idol_Lava"), TEXT("Idol_Poison"),
		TEXT("Idol_Decay"), TEXT("Idol_Bleed"), TEXT("Idol_Acid")
	};

	FString ToLabel(const FName IdolID)
	{
		return IdolID.ToString().Replace(TEXT("Idol_"), TEXT(""));
	}
}

AT66IdolVFXPreviewStage::AT66IdolVFXPreviewStage()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	PreviewFloor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewFloor"));
	PreviewFloor->SetupAttachment(SceneRoot);
	PreviewFloor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewFloor->SetCastShadow(false);
	PreviewFloor->SetReceivesDecals(false);
	PreviewFloor->SetCanEverAffectNavigation(false);
	PreviewFloor->SetRelativeLocation(FVector(1050.f, 0.f, -6.f));
	PreviewFloor->SetRelativeScale3D(FVector(28.f, 13.f, 0.12f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		PreviewFloor->SetStaticMesh(PlaneMesh.Object);
	}
}

void AT66IdolVFXPreviewStage::BuildPreview()
{
	ClearPreview();

	SpawnPierceRow(FVector(0.f, -RowSpacing * 1.5f, 0.f));
	SpawnBounceRow(FVector(0.f, -RowSpacing * 0.5f, 0.f));
	SpawnAOERow(FVector(0.f, RowSpacing * 0.5f, 0.f));
	SpawnDOTRow(FVector(0.f, RowSpacing * 1.5f, 0.f));
}

void AT66IdolVFXPreviewStage::ClearPreview()
{
	for (UTextRenderComponent* Label : SpawnedLabels)
	{
		if (Label)
		{
			Label->DestroyComponent();
		}
	}
	SpawnedLabels.Reset();

	UWorld* World = GetWorld();
	for (AActor* Actor : SpawnedPreviewActors)
	{
		if (!Actor)
		{
			continue;
		}

#if WITH_EDITOR
		if (World && !World->IsGameWorld())
		{
			World->EditorDestroyActor(Actor, true);
			continue;
		}
#endif
		Actor->Destroy();
	}
	SpawnedPreviewActors.Reset();
}

void AT66IdolVFXPreviewStage::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoBuildOnBeginPlay)
	{
		BuildPreview();
	}
}

void AT66IdolVFXPreviewStage::SpawnPierceRow(const FVector& RowOrigin)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(PierceIdols); ++Index)
	{
		const FName IdolID = PierceIdols[Index];
		const FVector SlotCenter = RowOrigin + FVector(Index * ColumnSpacing, 0.f, 0.f);
		const FVector Start = GetActorLocation() + SlotCenter + FVector(-145.f, 0.f, 12.f);
		const FVector End = GetActorLocation() + SlotCenter + FVector(155.f, 0.f, 12.f);

		AT66IdolProcVFX* Effect = World->SpawnActor<AT66IdolProcVFX>(AT66IdolProcVFX::StaticClass(), Start, FRotator::ZeroRotator);
		if (!Effect)
		{
			continue;
		}

		FT66IdolProcVFXRequest Request;
		Request.IdolID = IdolID;
		Request.Rarity = PreviewRarity;
		Request.Category = ET66AttackCategory::Pierce;
		Request.Start = Start;
		Request.End = End;
		Request.Impact = End;
		Effect->InitEffect(Request);
		SpawnedPreviewActors.Add(Effect);
		AddSlotLabel(ToLabel(IdolID), SlotCenter + FVector(0.f, 0.f, 150.f));
	}
}

void AT66IdolVFXPreviewStage::SpawnBounceRow(const FVector& RowOrigin)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(BounceIdols); ++Index)
	{
		const FName IdolID = BounceIdols[Index];
		const FVector SlotCenter = RowOrigin + FVector(Index * ColumnSpacing, 0.f, 0.f);
		const FVector P0 = GetActorLocation() + SlotCenter + FVector(-145.f, 0.f, 12.f);
		const FVector P1 = GetActorLocation() + SlotCenter + FVector(-20.f, 70.f, 16.f);
		const FVector P2 = GetActorLocation() + SlotCenter + FVector(135.f, -32.f, 12.f);

		AT66IdolProcVFX* Effect = World->SpawnActor<AT66IdolProcVFX>(AT66IdolProcVFX::StaticClass(), P0, FRotator::ZeroRotator);
		if (!Effect)
		{
			continue;
		}

		FT66IdolProcVFXRequest Request;
		Request.IdolID = IdolID;
		Request.Rarity = PreviewRarity;
		Request.Category = ET66AttackCategory::Bounce;
		Request.ChainPositions = { P0, P1, P2 };
		Effect->InitEffect(Request);
		SpawnedPreviewActors.Add(Effect);
		AddSlotLabel(ToLabel(IdolID), SlotCenter + FVector(0.f, 0.f, 150.f));
	}
}

void AT66IdolVFXPreviewStage::SpawnAOERow(const FVector& RowOrigin)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(AOEIdols); ++Index)
	{
		const FName IdolID = AOEIdols[Index];
		const FVector SlotCenter = RowOrigin + FVector(Index * ColumnSpacing, 0.f, 0.f);
		const FVector WorldCenter = GetActorLocation() + SlotCenter;

		AActor* EffectActor = nullptr;
		if (IdolID == TEXT("Idol_Fire"))
		{
			if (AT66FireIdolAttackVFX* FireEffect = World->SpawnActor<AT66FireIdolAttackVFX>(AT66FireIdolAttackVFX::StaticClass(), WorldCenter, FRotator::ZeroRotator))
			{
				bool bWasClamped = false;
				const ET66FireIdolTestCandidate FireCandidate = AT66FireIdolAttackVFX::SanitizeCandidate(static_cast<int32>(FirePreviewCandidateRaw), bWasClamped);
				FireEffect->ConfigureCandidate(FireCandidate);
				FireEffect->InitEffect(WorldCenter, 155.f, FLinearColor(1.f, 0.45f, 0.08f, 1.f), 0, IdolID);
				EffectActor = FireEffect;
			}
		}
		else if (AT66IdolVolumeAttackVFX::ShouldUseVolume(IdolID, ET66AttackCategory::AOE))
		{
			if (AT66IdolVolumeAttackVFX* VolumeEffect = World->SpawnActor<AT66IdolVolumeAttackVFX>(AT66IdolVolumeAttackVFX::StaticClass(), WorldCenter, FRotator::ZeroRotator))
			{
				const FLinearColor IdolColor = UT66IdolManagerSubsystem::GetIdolColor(IdolID);
				VolumeEffect->InitEffect(IdolID, ET66AttackCategory::AOE, nullptr, WorldCenter, 150.f, 1.0f, IdolColor, 0);
				EffectActor = VolumeEffect;
			}
		}
		else if (AT66IdolProcVFX* ProcEffect = World->SpawnActor<AT66IdolProcVFX>(AT66IdolProcVFX::StaticClass(), WorldCenter, FRotator::ZeroRotator))
		{
			FT66IdolProcVFXRequest Request;
			Request.IdolID = IdolID;
			Request.Rarity = PreviewRarity;
			Request.Category = ET66AttackCategory::AOE;
			Request.Center = WorldCenter;
			Request.Radius = 150.f;
			ProcEffect->InitEffect(Request);
			EffectActor = ProcEffect;
		}

		if (EffectActor)
		{
			SpawnedPreviewActors.Add(EffectActor);
			AddSlotLabel(ToLabel(IdolID), SlotCenter + FVector(0.f, 0.f, 170.f));
		}
	}
}

void AT66IdolVFXPreviewStage::SpawnDOTRow(const FVector& RowOrigin)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(DOTIdols); ++Index)
	{
		const FName IdolID = DOTIdols[Index];
		const FVector SlotCenter = RowOrigin + FVector(Index * ColumnSpacing, 0.f, 0.f);
		const FVector DummyLocation = GetActorLocation() + SlotCenter + FVector(0.f, 0.f, 88.f);

		AT66IdolVFXPreviewDummy* Dummy = World->SpawnActor<AT66IdolVFXPreviewDummy>(AT66IdolVFXPreviewDummy::StaticClass(), DummyLocation, FRotator::ZeroRotator);
		if (Dummy)
		{
			SpawnedPreviewActors.Add(Dummy);
		}

		AActor* EffectActor = nullptr;
		if (AT66IdolVolumeAttackVFX::ShouldUseVolume(IdolID, ET66AttackCategory::DOT))
		{
			if (AT66IdolVolumeAttackVFX* VolumeEffect = World->SpawnActor<AT66IdolVolumeAttackVFX>(AT66IdolVolumeAttackVFX::StaticClass(), DummyLocation, FRotator::ZeroRotator))
			{
				const FLinearColor Tint = UT66IdolManagerSubsystem::GetIdolColor(IdolID);
				VolumeEffect->InitEffect(IdolID, ET66AttackCategory::DOT, Dummy, DummyLocation, 110.f, 1.2f, Tint, 0);
				EffectActor = VolumeEffect;
			}
		}
		else if (AT66IdolProcVFX* ProcEffect = World->SpawnActor<AT66IdolProcVFX>(AT66IdolProcVFX::StaticClass(), DummyLocation, FRotator::ZeroRotator))
		{
			FT66IdolProcVFXRequest Request;
			Request.IdolID = IdolID;
			Request.Rarity = PreviewRarity;
			Request.Category = ET66AttackCategory::DOT;
			Request.Center = DummyLocation;
			Request.Radius = 115.f;
			Request.Duration = 1.2f;
			Request.FollowTarget = Dummy;
			ProcEffect->InitEffect(Request);
			EffectActor = ProcEffect;
		}

		if (EffectActor)
		{
			SpawnedPreviewActors.Add(EffectActor);
			AddSlotLabel(ToLabel(IdolID), SlotCenter + FVector(0.f, 0.f, 195.f));
		}
	}
}

void AT66IdolVFXPreviewStage::AddSlotLabel(const FString& LabelText, const FVector& RelativeLocation)
{
	UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this);
	if (!Label)
	{
		return;
	}

	AddInstanceComponent(Label);
	Label->SetupAttachment(SceneRoot);
	Label->SetRelativeLocation(RelativeLocation);
	Label->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	Label->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	Label->SetWorldSize(56.f);
	Label->SetText(FText::FromString(LabelText));
	Label->SetTextRenderColor(FColor::White);
	Label->RegisterComponent();
	SpawnedLabels.Add(Label);
}
