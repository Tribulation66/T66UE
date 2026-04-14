// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class T66MINI_API ST66MiniFriendsPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST66MiniFriendsPanel) {}
		SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(TArray<FText>, Rows)
		SLATE_ARGUMENT(FText, Footer)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
