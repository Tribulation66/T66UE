// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "GameFramework/HUD.h"

#include "ZibraVDBHUD.generated.h"

UCLASS()
class ZIBRAVDBRUNTIME_API AZibraVDBHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
	
};
