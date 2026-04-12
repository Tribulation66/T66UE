// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Data/T66DataTypes.h"
#include "UI/T66ScreenBase.h"
#include "T66AchievementsScreen.generated.h"

class UT66LocalizationSubsystem;
class UT66AchievementsSubsystem;
class UT66PlayerSettingsSubsystem;

/**
 * Achievements Screen
 * Full-page achievement browser with separate Standard and Special sections.
 */
UCLASS(Blueprintable)
class T66_API UT66AchievementsScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66AchievementsScreen(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void OnBackClicked();

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	TArray<FAchievementData> AllAchievements;
	TSharedPtr<SVerticalBox> AchievementListBox;

	UT66LocalizationSubsystem* GetLocSubsystem() const;
	UT66AchievementsSubsystem* GetAchievementsSubsystem() const;
	UT66PlayerSettingsSubsystem* GetPlayerSettingsSubsystem() const;

	void RefreshAchievements();
	void RebuildAchievementList();
	int32 GetUnlockedAchievementCount() const;
	int32 GetUnlockedAchievementCountForCategory(ET66AchievementCategory Category) const;
	TArray<FAchievementData> GetAchievementsForCategory(ET66AchievementCategory Category) const;
	FText BuildAchievementRewardText(const FAchievementData& Achievement) const;
	FText GetAchievementActionText(const FAchievementData& Achievement) const;

	FReply HandleBackClicked();
	FReply HandleClaimClicked(FName AchievementID);

	UFUNCTION()
	void HandleLanguageChanged(ET66Language NewLanguage);

	UFUNCTION()
	void HandleAchievementsStateChanged();
};
