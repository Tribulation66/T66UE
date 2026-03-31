// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"
#include "Input/Reply.h"

class FZibraVDBAssetDeprecatedCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) final;

private:
	FReply OpenCompressionWindow();

	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
};
