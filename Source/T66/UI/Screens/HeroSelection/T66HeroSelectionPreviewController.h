// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "T66HeroSelectionPreviewController.generated.h"

class AT66HeroPreviewStage;
class UT66GameInstance;
class UT66HeroSelectionScreen;
class SBorder;
class SImage;
class SScaleBox;
class STextBlock;
class SWidget;
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
	void ApplySelectionDifficultyToPreviewStages(ET66Difficulty SelectedDifficulty) const;
	void ApplyHeroPreviewStage(
		UT66GameInstance* GameInstance,
		FName PreviewedHeroID,
		FName PreviewedCompanionID,
		ET66BodyType SelectedBodyType,
		ET66Difficulty SelectedDifficulty,
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
	AT66HeroPreviewStage* GetHeroPreviewStage() const;

private:
	void PositionPreviewCamera() const;
	UT66HeroSelectionScreen* GetOwnerScreen() const;

	TWeakObjectPtr<UT66HeroSelectionScreen> OwnerScreen;
	TWeakPtr<SBorder> HeroPreviewColorBox;
	TWeakPtr<SImage> HeroPreviewVideoImage;
	TWeakPtr<STextBlock> HeroPreviewPlaceholderText;
	TWeakPtr<SScaleBox> CompanionInfoPortraitScaleBox;
	TWeakPtr<STextBlock> CompanionPreviewPlaceholderText;

	TSharedPtr<FSlateBrush> CompanionInfoPortraitBrush;
	ET66HeroSelectionPreviewClip SelectedHeroPreviewClip = ET66HeroSelectionPreviewClip::Overview;
	FName PreviewSkinIDOverride = NAME_None;
	FName PreviewedCompanionSkinIDOverride = NAME_None;
};
