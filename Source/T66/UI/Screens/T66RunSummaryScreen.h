// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66RunSummaryScreen.generated.h"

/**
 * Run Summary screen shown on death: 3D preview placeholder, event log, Restart / Main Menu.
 */
UCLASS(Blueprintable)
class T66_API UT66RunSummaryScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66RunSummaryScreen(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Run Summary")
	void OnRestartClicked();

	UFUNCTION(BlueprintCallable, Category = "Run Summary")
	void OnMainMenuClicked();

	UFUNCTION(BlueprintCallable, Category = "Run Summary")
	void OnViewLogClicked();

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleRestartClicked();
	FReply HandleMainMenuClicked();
	FReply HandleViewLogClicked();

	void RebuildLogItems();
	TSharedRef<ITableRow> GenerateLogRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);

	bool bLogVisible = true;

	// Virtualized log list (prevents building a widget per entry).
	TArray<TSharedPtr<FString>> LogItems;
};
