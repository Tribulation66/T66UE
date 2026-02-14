// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66FloatingCombatTextActor.h"
#include "UI/T66FloatingCombatTextWidget.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneComponent.h"

AT66FloatingCombatTextActor::AT66FloatingCombatTextActor()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawAtDesiredSize(true);
	WidgetComponent->SetDrawSize(FVector2D(200.f, 40.f));
	WidgetComponent->SetWidgetClass(UT66FloatingCombatTextWidget::StaticClass());
	WidgetComponent->InitWidget();
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
