// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "T66HeroSelectionPreviewController.generated.h"

class UT66GameInstance;
class UT66HeroSelectionScreen;
class SBorder;
class SImage;
class SScaleBox;
class STextBlock;
class SWidget;
class UT66FrontendVideoPlayer;
struct FSlateBrush;

enum class ET66BodyType : uint8;
enum class ET66Difficulty : uint8;

enum class ET66HeroSelectionPreviewClip : uint8
{
	Overview,
	Ultimate,
	Passive,
};

UCLASS(Transient)
class T66_API UT66HeroSelectionPreviewController : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UT66HeroSelectionScreen* InOwnerScreen);
	TSharedRef<SWidget> CreateHeroPreviewWidget(const FLinearColor& FallbackColor);
	void BindPreviewPanelWidgets(
		const TSharedPtr<SImage>& InHeroPreviewVideoImage,
		const TSharedPtr<STextBlock>& InHeroPreviewPlaceholderText,
		const TSharedPtr<SScaleBox>& InCompanionPreviewScaleBox,
		const TSharedPtr<STextBlock>& InCompanionPreviewPlaceholderText);

	void EnsureCompanionPreviewBrush();
	void RefreshCompanionPreviewPanel(UT66GameInstance* GameInstance, FName PreviewedCompanionID, bool bShowingCompanionInfo);
	void ApplyHeroPreviewVideo(
		UT66GameInstance* GameInstance,
		FName PreviewedHeroID,
		FName PreviewedCompanionID,
		ET66BodyType SelectedBodyType,
		ET66Difficulty SelectedDifficulty,
		bool bShowingCompanionInfo,
		const FLinearColor& FallbackColor) const;

	void ResetHeroPreviewStateForHeroSwitch();
	void ResetHeroSkinPreviewOverride();
	void ResetCompanionSkinPreviewOverride();
	void ToggleHeroSkinPreviewOverride(FName SkinID);
	void ToggleCompanionSkinPreviewOverride(FName SkinID);

	ET66HeroSelectionPreviewClip GetSelectedPreviewClip() const;
	bool IsSelectedPreviewClip(ET66HeroSelectionPreviewClip Clip) const;
	void ToggleSelectedPreviewClip(ET66HeroSelectionPreviewClip Clip, bool bShowingCompanionInfo);

	FName GetHeroSkinPreviewOverride() const;
	FName GetCompanionSkinPreviewOverride() const;
	FName ResolveEffectiveHeroSkinID(const UT66GameInstance* GameInstance) const;
	FName ResolveEffectiveCompanionSkinID(const UT66GameInstance* GameInstance, FName PreviewedCompanionID) const;

	const FSlateBrush* GetHeroPreviewVideoBrush() const;
	const FSlateBrush* GetCompanionInfoPortraitBrush() const;

private:
	UT66HeroSelectionScreen* GetOwnerScreen() const;

	TWeakObjectPtr<UT66HeroSelectionScreen> OwnerScreen;
	TWeakPtr<SBorder> HeroPreviewColorBox;
	TWeakPtr<SImage> HeroPreviewVideoImage;
	TWeakPtr<STextBlock> HeroPreviewPlaceholderText;
	TWeakPtr<SScaleBox> CompanionInfoPortraitScaleBox;
	TWeakPtr<STextBlock> CompanionPreviewPlaceholderText;

	TSharedPtr<FSlateBrush> CompanionInfoPortraitBrush;
	UPROPERTY(Transient)
	mutable TObjectPtr<UT66FrontendVideoPlayer> HeroPreviewVideoPlayer;
	ET66HeroSelectionPreviewClip SelectedHeroPreviewClip = ET66HeroSelectionPreviewClip::Overview;
	FName PreviewSkinIDOverride = NAME_None;
	FName PreviewedCompanionSkinIDOverride = NAME_None;
	mutable bool bPendingAutomationPreviewRefresh = false;
};
