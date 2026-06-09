// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"

class UT66UITexturePoolSubsystem;
struct FSlateBrush;

namespace T66TemporaryBuffUI
{
	FString GetSecondaryBuffIconSlug(ET66StatType StatType);

	TSharedPtr<FSlateBrush> CreateSecondaryBuffBrush(
		UT66UITexturePoolSubsystem* TexPool,
		UObject* Requester,
		ET66StatType StatType,
		const FVector2D& ImageSize);
}
