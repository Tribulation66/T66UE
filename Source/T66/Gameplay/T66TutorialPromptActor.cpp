// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TutorialPromptActor.h"

#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Gameplay/T66HeroBase.h"

AT66TutorialPromptActor::AT66TutorialPromptActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(180.f, 420.f, 220.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = TriggerBox;

	Text = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Text"));
	Text->SetupAttachment(RootComponent);
	Text->SetHorizontalAlignment(EHTA_Center);
	Text->SetVerticalAlignment(EVRTA_TextCenter);
	Text->SetTextRenderColor(FColor::White);
	Text->SetWorldSize(120.f);
	Text->SetRelativeLocation(FVector(0.f, 0.f, 900.f)); // sky text
	Text->SetRelativeRotation(FRotator(0.f, 180.f, 0.f)); // face toward start area
}

void AT66TutorialPromptActor::BeginPlay()
{
	Super::BeginPlay();
	if (Text)
	{
		Text->SetText(PromptText);
	}
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AT66TutorialPromptActor::OnTriggerBeginOverlap);
	}
}

void AT66TutorialPromptActor::SetActive(bool bActive)
{
	SetActorHiddenInGame(!bActive);
	SetActorEnableCollision(bActive);
	if (!bActive)
	{
		bTriggered = false;
	}
}

void AT66TutorialPromptActor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bTriggered) return;
	if (!OtherActor) return;
	if (!Cast<AT66HeroBase>(OtherActor)) return;

	bTriggered = true;
	OnPassed.Broadcast();
	// Hide immediately once passed.
	SetActive(false);
}

