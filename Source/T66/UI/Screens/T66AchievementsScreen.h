// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "T66AchievementsScreen.generated.h"

class UT66LocalizationSubsystem;

/**
 * Achievements Screen - Bible 1.19
 * Full-page achievement browser with category toggles
 * Header: Title (left), Total AC (right)
 * Tabs: Black, Red, Yellow, White
 * Content: Scrollable list of achievement rows
 */
UCLASS(Blueprintable)
class T66_API UT66AchievementsScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66AchievementsScreen(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = "Achievements")
	ET66AchievementTier CurrentTier = ET66AchievementTier::Black;

	/** Total Achievement Coins earned (placeholder) */
	UPROPERTY(BlueprintReadWrite, Category = "Achievements")
	int32 TotalAchievementCoins = 1250;

	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void SwitchToTier(ET66AchievementTier Tier);

	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void OnBackClicked();

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	TArray<FAchievementData> AllAchievements;
	TSharedPtr<SVerticalBox> AchievementListBox;

	UT66LocalizationSubsystem* GetLocSubsystem() const;

	void GeneratePlaceholderAchievements();
	void RebuildAchievementList();

	FReply HandleTierClicked(ET66AchievementTier Tier);
	FReply HandleBackClicked();
};
