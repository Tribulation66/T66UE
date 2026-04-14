// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66MiniCompanionSelectScreen.generated.h"

struct FT66MiniCompanionDefinition;
struct FSlateBrush;

UCLASS(Blueprintable)
class T66MINI_API UT66MiniCompanionSelectScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MiniCompanionSelectScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleBackClicked();
	FReply HandleContinueClicked();
	FReply HandleCompanionClicked(FName CompanionID);
	void RebuildCompanionBrushes(const TArray<FT66MiniCompanionDefinition>& Companions);
	const FSlateBrush* FindCompanionBrush(FName CompanionID) const;

	TMap<FName, TSharedPtr<FSlateBrush>> CompanionBrushes;
};
