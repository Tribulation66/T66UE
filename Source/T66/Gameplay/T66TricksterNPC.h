// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "T66TricksterNPC.generated.h"

/** Trickster NPC (Cowardice Gate announcer). Placeholder: safe bubble + name + visuals. */
UCLASS(Blueprintable)
class T66_API AT66TricksterNPC : public AT66HouseNPCBase
{
	GENERATED_BODY()

public:
	AT66TricksterNPC();
	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void BeginPlay() override;
};

