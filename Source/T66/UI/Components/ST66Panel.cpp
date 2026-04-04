// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/ST66Panel.h"
#include "Widgets/SNullWidget.h"

void ST66Panel::Construct(const FArguments& InArgs)
{
	Rebuild(InArgs._Content.Widget, InArgs._Params);
}

void ST66Panel::Rebuild(const TSharedRef<SWidget>& InContent, const FT66PanelParams& InParams)
{
	Params = InParams;
	ChildSlot
	[
		FT66Style::MakePanel(InContent, Params)
	];
}
