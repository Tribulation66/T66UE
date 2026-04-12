// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66TemporaryBuffPresetCreateScreen.generated.h"

class SEditableTextBox;
class UT66BuffSubsystem;

UCLASS(Blueprintable)
class T66_API UT66TemporaryBuffPresetCreateScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66TemporaryBuffPresetCreateScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void RefreshScreen_Implementation() override;

private:
	UT66BuffSubsystem* GetBuffSubsystem() const;

	FReply HandleCreateClicked();
	FReply HandleCancelClicked();
	void HandlePresetNameChanged(const FText& NewText);

	TSharedPtr<SEditableTextBox> PresetNameTextBox;
	FString DraftPresetName;
};
