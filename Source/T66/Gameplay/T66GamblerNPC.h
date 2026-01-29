// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "T66GamblerNPC.generated.h"

/** Gambler NPC: opens a coin flip overlay (heads/tails). */
UCLASS(Blueprintable)
class T66_API AT66GamblerNPC : public AT66HouseNPCBase
{
	GENERATED_BODY()

public:
	AT66GamblerNPC();

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyNPCData(const FHouseNPCData& Data) override;

private:
	int32 WinGoldAmount = 10;
};

