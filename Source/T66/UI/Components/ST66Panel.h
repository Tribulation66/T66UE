// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Style/T66Style.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SNullWidget.h"

class T66_API ST66Panel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST66Panel)
		: _Params()
	{
		_Content.Widget = SNullWidget::NullWidget;
	}
		SLATE_ARGUMENT(FT66PanelParams, Params)
		SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void Rebuild(const TSharedRef<SWidget>& InContent, const FT66PanelParams& InParams);

private:
	FT66PanelParams Params;
};
