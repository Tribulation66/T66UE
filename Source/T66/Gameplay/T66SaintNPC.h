// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "T66SaintNPC.generated.h"

/** Saint NPC: heals hearts to full. */
UCLASS(Blueprintable)
class T66_API AT66SaintNPC : public AT66HouseNPCBase
{
	GENERATED_BODY()

public:
	AT66SaintNPC();

	virtual bool Interact(APlayerController* PC) override;
};

