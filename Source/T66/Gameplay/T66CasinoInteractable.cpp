// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CasinoInteractable.h"

#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"
#include "Gameplay/T66HeroBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/SoftObjectPath.h"

AT66CasinoInteractable::AT66CasinoInteractable()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Casino/Casino.Casino")));

	if (VisualMesh)
	{
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 12.f));
	}

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(720.f, 620.f, 360.f));
	}

	ApplyRarityVisuals();
}

void AT66CasinoInteractable::BeginPlay()
{
	Super::BeginPlay();

	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->BossChanged.AddDynamic(this, &AT66CasinoInteractable::HandleBossStateChanged);
	}

	RefreshInteractionPrompt();
}

void AT66CasinoInteractable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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

void AT66CasinoInteractable::HandleBossStateChanged()
{
	RefreshInteractionPrompt();
}

UT66RunStateSubsystem* AT66CasinoInteractable::GetRunState() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}
