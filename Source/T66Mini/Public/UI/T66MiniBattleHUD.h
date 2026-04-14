// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "T66MiniBattleHUD.generated.h"

UCLASS()
class T66MINI_API AT66MiniBattleHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
