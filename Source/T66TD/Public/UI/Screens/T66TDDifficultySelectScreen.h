// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "UObject/StrongObjectPtr.h"
#include "T66TDDifficultySelectScreen.generated.h"

class UTexture2D;

UCLASS(Blueprintable)
class T66TD_API UT66TDDifficultySelectScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66TDDifficultySelectScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	struct FMapBrushEntry
	{
		FString Key;
		TSharedPtr<FSlateBrush> Brush;
	};

	FReply HandleBackClicked();
	FReply HandleDifficultyClicked(FName DifficultyID);
	FReply HandleMapClicked(FName MapID);
	FReply HandleStartMatchClicked();
	void EnsureSelectionState();
	const TSharedPtr<FSlateBrush>& FindOrLoadMapBrush(const FString& RelativePath);
	void ReleaseRetainedSlateState();

	TArray<FMapBrushEntry> MapBrushEntries;
	TArray<TStrongObjectPtr<UTexture2D>> RetainedTextures;
};
