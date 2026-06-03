// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66NPCBase.h"
#include "T66CasinoNPC.generated.h"

/** Casino NPC: opens the shared casino shell overlay. */
UCLASS(Blueprintable)
class T66_API AT66CasinoNPC : public AT66NPCBase
{
	GENERATED_BODY()

public:
	AT66CasinoNPC();

	virtual bool Interact(APlayerController* PC) override;

	int32 GetWinGoldAmount() const { return WinGoldAmount; }

protected:
	virtual void BeginPlay() override;
	virtual void ApplyNPCData(const FT66NPCData& Data) override;
	virtual bool ShouldApplyCharacterVisual() const override { return false; }

private:
	void ApplyCasinoNPCStaticVisual();

	int32 WinGoldAmount = 10;
};


