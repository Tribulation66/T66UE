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

	enum class EFrontendSection : uint8
	{
		AccountStatus,
		Settings,
		Language,
		Home,
		PowerUp,
		MiniGames,
		Achievements,
		None,
	};
	using ETopBarSection = EFrontendSection;

	static float GetReservedHeight();
	static float GetVisibleContentHeight();
	static EFrontendSection ResolveFrontendSectionForScreen(ET66ScreenType ScreenType);

	void SetActiveSection(EFrontendSection InActiveSection);
	void ClearActiveSectionOverride();
	EFrontendSection GetRenderedActiveSection() const;

	struct FPlateBrushSet
	{
		TSharedPtr<FSlateBrush> NormalBrush;
		TSharedPtr<FSlateBrush> HoverBrush;
		TSharedPtr<FSlateBrush> PressedBrush;
		TSharedPtr<FSlateBrush> DisabledBrush;
		TSharedPtr<FSlateBrush> SelectedBrush;
	};

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void RefreshScreen_Implementation() override;

private:
	UT66LocalizationSubsystem* GetLocSubsystem() const;
	ETopBarSection GetActiveSection() const;
	FText GetChadCouponsValueText() const;
	void NavigateWithTopBar(ET66ScreenType TargetScreen);
	void RequestTopBarAssets();
	void ReleaseTopBarBrushes();

	FReply HandleSettingsClicked();
	FReply HandleLanguageClicked();
	FReply HandleHomeClicked();
	FReply HandlePowerUpClicked();
	FReply HandleMiniGamesClicked();
	FReply HandleAchievementsClicked();
	FReply HandleAccountStatusClicked();
	FReply HandleQuitClicked();

	TSharedPtr<FSlateBrush> TopBarBackdropBrush;
	TSharedPtr<FSlateBrush> TopBarFoliageLeftBrush;
	TSharedPtr<FSlateBrush> TopBarFoliageRightBrush;
	FPlateBrushSet SettingsButtonBrushes;
	FPlateBrushSet LanguageButtonBrushes;
	FPlateBrushSet AccountButtonBrushes;
	FPlateBrushSet HomeButtonBrushes;
	FPlateBrushSet NavButtonBrushes;
	FPlateBrushSet PowerUpButtonBrushes;
	FPlateBrushSet AchievementsButtonBrushes;
	FPlateBrushSet MiniGamesButtonBrushes;
	FPlateBrushSet PortraitButtonBrushes;
	FPlateBrushSet CouponButtonBrushes;
	FPlateBrushSet QuitButtonBrushes;
	TSharedPtr<FSlateBrush> UtilityButtonBrush;
	TSharedPtr<FSlateBrush> AccountButtonBrush;
	TSharedPtr<FSlateBrush> AccountButtonActiveBrush;
	TSharedPtr<FSlateBrush> NavButtonBrush;
	TSharedPtr<FSlateBrush> NavButtonActiveBrush;
	TSharedPtr<FSlateBrush> HomeButtonBrush;
	TSharedPtr<FSlateBrush> HomeButtonActiveBrush;
	TSharedPtr<FSlateBrush> CurrencyButtonBrush;
	TSharedPtr<FSlateBrush> HomeIconBrush;
	TSharedPtr<FSlateBrush> SettingsIconBrush;
	TSharedPtr<FSlateBrush> SocialIconBrush;
	TSharedPtr<FSlateBrush> CurrencyIconBrush;
	TSharedPtr<FSlateBrush> QuitIconBrush;
	EFrontendSection ActiveSectionOverride = EFrontendSection::None;
	FVector2D CachedViewportSize = FVector2D::ZeroVector;
	bool bHasActiveSectionOverride = false;
	bool bViewportResponsiveRebuildQueued = false;
};
