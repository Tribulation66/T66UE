// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66ShelvedFeatureScreen.generated.h"

UCLASS(Blueprintable)
class T66_API UT66ShelvedFeatureScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66ShelvedFeatureScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleBackClicked();
};
