// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66FrontendTopBarWidget.generated.h"

class UT66LocalizationSubsystem;

struct FSlateBrush;

UCLASS()
class T66_API UT66FrontendTopBarWidget : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66FrontendTopBarWidget(const FObjectInitializer& ObjectInitializer);

	static float GetReservedHeight();
	static float GetVisibleContentHeight();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void RefreshScreen_Implementation() override;

	private:
	enum class ETopBarSection : uint8
	{
		AccountStatus,
		Home,
		LoadGame,
		PowerUp,
		MiniGames,
		Achievements,
		None,
	};

	UT66LocalizationSubsystem* GetLocSubsystem() const;
	ETopBarSection GetActiveSection() const;
	FText GetChadCouponsValueText() const;
	void NavigateWithTopBar(ET66ScreenType TargetScreen);
	void RequestTopBarAssets();
	void ReleaseTopBarBrushes();

	FReply HandleSettingsClicked();
	FReply HandleLanguageClicked();
	FReply HandleHomeClicked();
	FReply HandleLoadGameClicked();
	FReply HandleShopClicked();
	FReply HandleMiniGamesClicked();
	FReply HandleAchievementsClicked();
	FReply HandleAccountStatusClicked();
	FReply HandleQuitClicked();

	TSharedPtr<FSlateBrush> TopBarPlateBrush;
	TSharedPtr<FSlateBrush> InactiveTabBrush;
	TSharedPtr<FSlateBrush> ActiveTabBrush;
	TSharedPtr<FSlateBrush> HomeInactiveTabBrush;
	TSharedPtr<FSlateBrush> HomeActiveTabBrush;
	TSharedPtr<FSlateBrush> HomeIconBrush;
	TSharedPtr<FSlateBrush> NavSeparatorBrush;
	TSharedPtr<FSlateBrush> SettingsSlotBrush;
	TSharedPtr<FSlateBrush> UtilitySlotBrush;
	TSharedPtr<FSlateBrush> QuitSlotBrush;
	TSharedPtr<FSlateBrush> SettingsIconBrush;
	TSharedPtr<FSlateBrush> LanguageIconBrush;
	TSharedPtr<FSlateBrush> AchievementCoinsIconBrush;
	TSharedPtr<FSlateBrush> PowerCouponsIconBrush;
	TSharedPtr<FSlateBrush> QuitIconBrush;
	TSharedPtr<FSlateBrush> QuitGlowBrush;
	FVector2D CachedViewportSize = FVector2D::ZeroVector;
	bool bViewportResponsiveRebuildQueued = false;
};
