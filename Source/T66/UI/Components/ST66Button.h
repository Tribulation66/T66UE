// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Style/T66Style.h"
#include "Widgets/SCompoundWidget.h"

class T66_API ST66Button : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST66Button)
		: _Params()
	{
	}
		SLATE_ARGUMENT(FT66ButtonParams, Params)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void Rebuild(const FT66ButtonParams& InParams);

private:
	FT66ButtonParams Params;
};
