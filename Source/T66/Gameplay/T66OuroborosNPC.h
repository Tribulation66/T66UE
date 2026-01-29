// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "T66OuroborosNPC.generated.h"

/** Ouroboros NPC: insta-kills the player on interact. */
UCLASS(Blueprintable)
class T66_API AT66OuroborosNPC : public AT66HouseNPCBase
{
	GENERATED_BODY()

public:
	AT66OuroborosNPC();

	virtual bool Interact(APlayerController* PC) override;
};

