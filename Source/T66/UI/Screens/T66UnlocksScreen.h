// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/T66ScreenBase.h"
#include "T66UnlocksScreen.generated.h"

class UT66AchievementsSubsystem;
class UT66CompanionUnlockSubsystem;
class UT66GameInstance;
class UT66LocalizationSubsystem;

UCLASS(Blueprintable)
class T66_API UT66UnlocksScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66UnlocksScreen(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Unlocks")
	void OnBackClicked();

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	enum class EUnlockCategory : uint8
	{
		Heroes,
		Companions,
		Items
	};

	struct FUnlockEntry
	{
		FText Name;
		FText TypeText;
		FText DetailText;
		bool bUnlocked = false;
		int32 Order = 0;
	};

	EUnlockCategory CurrentCategory = EUnlockCategory::Heroes;
	TSharedPtr<SVerticalBox> UnlockListBox;

	UT66LocalizationSubsystem* GetLocSubsystem() const;
	UT66AchievementsSubsystem* GetAchievementsSubsystem() const;
	UT66CompanionUnlockSubsystem* GetCompanionUnlockSubsystem() const;
	UT66GameInstance* GetT66GameInstance() const;

	TArray<FUnlockEntry> BuildEntriesForCategory(EUnlockCategory Category) const;
	void RebuildUnlockList();
	int32 GetTotalUnlockCount() const;
	int32 GetUnlockedCount() const;
	void SwitchToCategory(EUnlockCategory Category);

	FReply HandleCategoryClicked(EUnlockCategory Category);
	FReply HandleBackClicked();
	FReply HandleOpenMiniChadpocalypseClicked();
	FReply HandleOpenChadpocalypseTDClicked();

	UFUNCTION()
	void HandleLanguageChanged(ET66Language NewLanguage);

	UFUNCTION()
	void HandleAchievementsStateChanged();
};
