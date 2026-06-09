// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66NPCBase.h"
#include "T66VendorNPC.generated.h"

/** Guaranteed floor vendor NPC: opens the vendor-only shop overlay. */
UCLASS(Blueprintable)
class T66_API AT66VendorNPC : public AT66NPCBase
{
	GENERATED_BODY()

public:
	AT66VendorNPC();

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void BeginPlay() override;
	virtual bool ShouldApplyCharacterVisual() const override { return false; }

private:
	void ApplyVendorNPCStaticVisual();
};
