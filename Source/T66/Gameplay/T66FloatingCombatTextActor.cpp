// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66FloatingCombatTextActor.h"
#include "UI/T66FloatingCombatTextWidget.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

AT66FloatingCombatTextActor::AT66FloatingCombatTextActor()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawAtDesiredSize(true);
	WidgetComponent->SetDrawSize(FVector2D(420.f, 100.f));
	WidgetComponent->SetWidgetClass(UT66FloatingCombatTextWidget::StaticClass());
	WidgetComponent->InitWidget();
}

void AT66FloatingCombatTextActor::ActivateForTarget(AActor* Target, const FVector& RelativeLocation)
{
	if (!IsValid(Target))
	{
		return;
	}

	SetActorHiddenInGame(false);
	SetActorEnableCollision(false);
	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(true, true);
		if (!WidgetComponent->GetUserWidgetObject())
		{
			WidgetComponent->InitWidget();
		}
	}

	AttachToActor(Target, FAttachmentTransformRules::KeepRelativeTransform);
	SetActorRelativeLocation(RelativeLocation);
}

void AT66FloatingCombatTextActor::DeactivateForPool()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorLocation(FVector(0.f, 0.f, -50000.f));
	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(false, true);
	}
}

void AT66FloatingCombatTextActor::SetDamageNumber(int32 Amount, FName EventType)
{
	if (UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject())
	{
		if (UT66FloatingCombatTextWidget* CombatWidget = Cast<UT66FloatingCombatTextWidget>(UserWidget))
		{
			CombatWidget->SetDamageNumber(Amount, EventType);
		}
	}
}

void AT66FloatingCombatTextActor::SetStatusEvent(FName EventType)
{
	if (UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject())
	{
		if (UT66FloatingCombatTextWidget* CombatWidget = Cast<UT66FloatingCombatTextWidget>(UserWidget))
		{
			CombatWidget->SetStatusEvent(EventType);
		}
	}
}
