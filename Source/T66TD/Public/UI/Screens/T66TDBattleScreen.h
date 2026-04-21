// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "UObject/StrongObjectPtr.h"
#include "T66TDBattleScreen.generated.h"

class UTexture2D;

UCLASS(Blueprintable)
class T66TD_API UT66TDBattleScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66TDBattleScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleBackClicked();
	void EnsureRunSelectionState();
	const TSharedPtr<FSlateBrush>& FindOrLoadMapBrush(const FString& RelativePath);
	void ReleaseRetainedSlateState();

	TSharedPtr<SWidget> BattleBoardRoot;
	TSharedPtr<FSlateBrush> MapBackgroundBrush;
	TStrongObjectPtr<UTexture2D> MapBackgroundTexture;
	TMap<FName, TSharedPtr<FSlateBrush>> HeroSpriteBrushes;
	TMap<FString, TSharedPtr<FSlateBrush>> EnemySpriteBrushes;
	TMap<FString, TSharedPtr<FSlateBrush>> BossSpriteBrushes;
};
