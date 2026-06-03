// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66NPCBase.h"
#include "T66SaintNPC.generated.h"

/** Saint NPC: random Saints only offer the Kromer branch; endgame Saints offer the full difficulty-clear choice set. */
UCLASS(Blueprintable)
class T66_API AT66SaintNPC : public AT66NPCBase
{
	GENERATED_BODY()

public:
	AT66SaintNPC();

	virtual bool Interact(APlayerController* PC) override;

	void SetEndgameSaint(const bool bInEndgameSaint) { bEndgameSaint = bInEndgameSaint; }
	bool IsEndgameSaint() const { return bEndgameSaint; }

private:
	UPROPERTY(EditAnywhere, Category = "Saint")
	bool bEndgameSaint = false;
};


