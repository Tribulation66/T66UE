// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Containers/ArrayView.h"
#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UI/WidgetGames/T66WidgetGameDescriptor.h"

class UUserWidget;

namespace T66WidgetGames::Registry
{
	T66_API TConstArrayView<FT66WidgetGameDescriptor> GetAllDescriptors();
	T66_API const FT66WidgetGameDescriptor* FindDescriptor(FName GameID);
	T66_API const FT66WidgetGameDescriptor* FindByLegacyID(FName LegacyID);
	T66_API void GetByCategory(ET66WidgetGameCategory Category, TArray<const FT66WidgetGameDescriptor*>& Out);
	T66_API bool IsAvailable(const UObject* WorldContext, const FT66WidgetGameDescriptor& Descriptor);
	T66_API TSubclassOf<UUserWidget> ResolveWidgetClass(const FT66WidgetGameDescriptor& Descriptor);
}
