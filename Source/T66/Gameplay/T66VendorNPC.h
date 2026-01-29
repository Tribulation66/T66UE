// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "T66VendorNPC.generated.h"

UCLASS(Blueprintable)
class T66_API AT66VendorNPC : public AT66HouseNPCBase
{
	GENERATED_BODY()

public:
	AT66VendorNPC();

	/** Try to sell first inventory item. Returns true if sold. */
	UFUNCTION(BlueprintCallable, Category = "Vendor")
	bool TrySellFirstItem();

	virtual bool Interact(APlayerController* PC) override;
};
