// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66MiniCharacterSelectScreen.generated.h"

struct FT66MiniHeroDefinition;
struct FSlateBrush;

UCLASS(Blueprintable)
class T66MINI_API UT66MiniCharacterSelectScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MiniCharacterSelectScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleBackClicked();
	FReply HandleContinueClicked();
	FReply HandleHeroClicked(FName HeroID);
	void RebuildHeroSpriteBrushes(const TArray<FT66MiniHeroDefinition>& Heroes);
	const FSlateBrush* FindHeroSpriteBrush(FName HeroID) const;

	TMap<FName, TSharedPtr<FSlateBrush>> HeroSpriteBrushes;
};
