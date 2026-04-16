// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "UI/T66ScreenBase.h"
#include "T66TemporaryBuffSelectionScreen.generated.h"

class UT66BuffSubsystem;
class UT66HeroSelectionScreen;
class UT66LocalizationSubsystem;

UCLASS(Blueprintable)
class T66_API UT66TemporaryBuffSelectionScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66TemporaryBuffSelectionScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void RefreshScreen_Implementation() override;

private:
	UT66LocalizationSubsystem* GetLocSubsystem() const;
	UT66BuffSubsystem* GetBuffSubsystem() const;
	UT66HeroSelectionScreen* GetLinkedHeroSelectionScreen() const;

	FReply HandleLoadoutSlotClicked(int32 SlotIndex);
	FReply HandleLoadoutSlotCleared(int32 SlotIndex);
	FReply HandleLoadoutSlotPurchased(int32 SlotIndex);
	FReply HandleUseOrBuyBuffForFocusedLoadoutSlot(ET66SecondaryStatType StatType);
	FReply HandleDoneClicked();
	FReply HandleEnterClicked();

	TArray<TSharedPtr<struct FSlateBrush>> BuffIconBrushes;
	TArray<TSharedPtr<struct FSlateBrush>> FooterPartyAvatarBrushes;
};
