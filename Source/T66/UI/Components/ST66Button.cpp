// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/ST66Button.h"

void ST66Button::Construct(const FArguments& InArgs)
{
	Rebuild(InArgs._Params);
}

void ST66Button::Rebuild(const FT66ButtonParams& InParams)
{
	Params = InParams;
	ChildSlot
	[
		FT66Style::MakeButton(Params)
	];
}
