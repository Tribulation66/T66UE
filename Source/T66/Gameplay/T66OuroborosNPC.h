// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66NPCBase.h"
#include "T66OuroborosNPC.generated.h"

/** Ouroboros NPC: insta-kills the player on interact. */
UCLASS(Blueprintable)
class T66_API AT66OuroborosNPC : public AT66NPCBase
{
	GENERATED_BODY()

public:
	AT66OuroborosNPC();

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "LethalZone")
	TObjectPtr<class UT66LethalZoneComponent> LethalZoneComponent;
};


