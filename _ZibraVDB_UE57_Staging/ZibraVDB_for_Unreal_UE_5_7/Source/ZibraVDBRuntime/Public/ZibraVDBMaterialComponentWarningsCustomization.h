// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class FZibraVDBMaterialComponentWarningsCustomization : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** ILayoutDetails interface */
	virtual void CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder) override;

private:
	/** Associated detail layout builder */
	IDetailLayoutBuilder* DetailLayoutBuilder;
};
