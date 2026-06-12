// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "T66CustomHeroBuilderScreen.generated.h"

class SBorder;
class STextBlock;
class SVerticalBox;
class UT66FrontendVideoPlayer;

UCLASS(Blueprintable)
class T66_API UT66CustomHeroBuilderScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66CustomHeroBuilderScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual bool HandleBackAction() override;

private:
	static constexpr int32 StatBudget = 24;
	static constexpr int32 MinStatValue = 1;
	static constexpr int32 MaxStatValue = 8;

	TArray<FName> HeroIDs;
	FName WeaponSourceHeroID = NAME_None;
	FName VisualSourceHeroID = NAME_None;
	ET66BodyType BodyType = ET66BodyType::Chad;
	FT66HeroStatBlock Stats;

	TSharedPtr<SVerticalBox> WeaponListBox;
	TSharedPtr<SVerticalBox> VisualListBox;
	TSharedPtr<SVerticalBox> StatsListBox;
	TSharedPtr<STextBlock> RemainingText;
	TSharedPtr<STextBlock> PreviewNameText;
	TSharedPtr<SBorder> PreviewFallbackBox;

	UPROPERTY(Transient)
	TObjectPtr<UT66FrontendVideoPlayer> PreviewVideoPlayer;

	void RefreshHeroIDs();
	void LoadFromGameInstance();
	void RebuildWeaponList();
	void RebuildVisualList();
	void RebuildStatsList();
	void UpdatePreview();
	void ClampStatsToBudget();
	int32 GetSpentPoints() const;
	int32 GetRemainingPoints() const;

	FReply HandleWeaponHeroClicked(FName HeroID);
	FReply HandleVisualHeroClicked(FName HeroID);
	FReply HandleBodyClicked(ET66BodyType InBodyType);
	FReply HandleAdjustStatClicked(FName StatName, int32 Delta);
	FReply HandleConfirmClicked();
	FReply HandleBackClicked();
	const FSlateBrush* GetPreviewVideoBrush() const;
};
