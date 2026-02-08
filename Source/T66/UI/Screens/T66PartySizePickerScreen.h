// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "T66PartySizePickerScreen.generated.h"

/**
 * Party Size Picker Screen
 * Three large centered panels: Solo, Duo, Trio
 */
UCLASS(Blueprintable)
class T66_API UT66PartySizePickerScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66PartySizePickerScreen(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = "Party Size")
	bool bIsNewGame = true;

	UFUNCTION(BlueprintCallable, Category = "Party Size")
	void OnSoloClicked();

	UFUNCTION(BlueprintCallable, Category = "Party Size")
	void OnCoopClicked();

	UFUNCTION(BlueprintCallable, Category = "Party Size")
	void OnBackClicked();

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	void SelectPartySize(ET66PartySize PartySize);

private:
	FReply HandleSoloClicked();
	FReply HandleCoopClicked();
	FReply HandleBackClicked();
};
