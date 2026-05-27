// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionPreviewController.h"
#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

#include "UI/T66FrontendVideoCatalog.h"
#include "UI/T66FrontendVideoPlayer.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"

using namespace T66HeroSelectionPrivate;

void UT66HeroSelectionPreviewController::Initialize(UT66HeroSelectionScreen* InOwnerScreen)
{
	OwnerScreen = InOwnerScreen;
}

TSharedRef<SWidget> UT66HeroSelectionPreviewController::CreateHeroPreviewWidget(const FLinearColor& FallbackColor)
{
	TSharedPtr<SBorder> PreviewColorBoxWidget;
	TSharedPtr<SImage> PreviewVideoImageWidget;
	const TSharedRef<SBorder> PreviewColorBoxWidgetRef =
		SAssignNew(PreviewColorBoxWidget, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FallbackColor)
		[
			SNew(SBox)
	];
	const TSharedRef<SImage> PreviewVideoImageWidgetRef =
		SAssignNew(PreviewVideoImageWidget, SImage)
		.Image_UObject(this, &UT66HeroSelectionPreviewController::GetHeroPreviewVideoBrush);
	HeroPreviewColorBoxes.Add(PreviewColorBoxWidget);
	HeroPreviewVideoImages.Add(PreviewVideoImageWidget);
	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			PreviewColorBoxWidgetRef
		]
		+ SOverlay::Slot()
		[
			PreviewVideoImageWidgetRef
		];
}

void UT66HeroSelectionPreviewController::BindPreviewPanelWidgets(
	const TSharedPtr<SImage>& InHeroPreviewVideoImage,
	const TSharedPtr<STextBlock>& InHeroPreviewPlaceholderText,
	const TSharedPtr<SScaleBox>& InCompanionPreviewScaleBox,
	const TSharedPtr<STextBlock>& InCompanionPreviewPlaceholderText)
{
	if (InHeroPreviewVideoImage.IsValid())
	{
		HeroPreviewVideoImages.Add(InHeroPreviewVideoImage);
	}
	HeroPreviewPlaceholderText = InHeroPreviewPlaceholderText;
	CompanionInfoPortraitScaleBox = InCompanionPreviewScaleBox;
	CompanionPreviewPlaceholderText = InCompanionPreviewPlaceholderText;
	if (TSharedPtr<SImage> PreviewVideoImage = InHeroPreviewVideoImage)
	{
		PreviewVideoImage->SetVisibility(EVisibility::Collapsed);
	}
	if (TSharedPtr<STextBlock> PreviewPlaceholder = HeroPreviewPlaceholderText.Pin())
	{
		PreviewPlaceholder->SetVisibility(EVisibility::Collapsed);
	}
}

void UT66HeroSelectionPreviewController::EnsureCompanionPreviewBrush()
{
	if (CompanionInfoPortraitBrush.IsValid())
	{
		return;
	}

	CompanionInfoPortraitBrush = MakeShared<FSlateBrush>();
	CompanionInfoPortraitBrush->DrawAs = ESlateBrushDrawType::Image;
	CompanionInfoPortraitBrush->Tiling = ESlateBrushTileType::NoTile;
	CompanionInfoPortraitBrush->ImageSize = FVector2D(320.f, 204.f);
}

void UT66HeroSelectionPreviewController::RefreshCompanionPreviewPanel(
	UT66GameInstance* GameInstance,
	FName PreviewedCompanionID,
	bool bShowingCompanionInfo)
{
	EnsureCompanionPreviewBrush();
	if (!CompanionInfoPortraitBrush.IsValid())
	{
		return;
	}

	CompanionInfoPortraitBrush->SetResourceObject(nullptr);

	if (GameInstance && !PreviewedCompanionID.IsNone())
	{
		if (UT66UITexturePoolSubsystem* TexPool = GameInstance->GetSubsystem<UT66UITexturePoolSubsystem>())
		{
			if (UT66SkinSubsystem* SkinSubsystem = GameInstance->GetSubsystem<UT66SkinSubsystem>())
			{
				const FName EffectiveCompanionSkinID = ResolveEffectiveCompanionSkinID(GameInstance, PreviewedCompanionID);
				const TSoftObjectPtr<UTexture2D> PortraitSoft = SkinSubsystem->GetSkinPortrait(
					ET66SkinEntityType::Companion,
					PreviewedCompanionID,
					EffectiveCompanionSkinID,
					true);
				if (!PortraitSoft.IsNull())
				{
					T66SlateTexture::BindSharedBrushAsync(
						TexPool,
						PortraitSoft,
						GetOwnerScreen(),
						CompanionInfoPortraitBrush,
						FName(TEXT("HeroSelectionCompanionInfoPortrait")),
						true);
				}
			}
		}
	}

	const bool bShowCompanionPortraitPanel = bShowingCompanionInfo && !PreviewedCompanionID.IsNone();
	if (TSharedPtr<SScaleBox> PortraitScaleBox = CompanionInfoPortraitScaleBox.Pin())
	{
		PortraitScaleBox->SetVisibility(
			bShowCompanionPortraitPanel
				? EVisibility::Visible
				: EVisibility::Collapsed);
	}
	if (TSharedPtr<STextBlock> PreviewPlaceholder = CompanionPreviewPlaceholderText.Pin())
	{
		PreviewPlaceholder->SetVisibility(
			bShowingCompanionInfo && PreviewedCompanionID.IsNone()
				? EVisibility::Visible
				: EVisibility::Collapsed);
		PreviewPlaceholder->SetText(
			PreviewedCompanionID.IsNone()
				? NSLOCTEXT("T66.HeroSelection", "NoCompanionPortraitPlaceholder", "No companion selected.")
				: NSLOCTEXT("T66.HeroSelection", "CompanionPortraitPlaceholder", "Companion portrait unavailable."));
	}
}

void UT66HeroSelectionPreviewController::ApplyHeroPreviewVideo(
	UT66GameInstance* GameInstance,
	FName PreviewedHeroID,
	FName PreviewedCompanionID,
	ET66BodyType SelectedBodyType,
	ET66Difficulty SelectedDifficulty,
	bool bShowingCompanionInfo,
	const FLinearColor& FallbackColor) const
{
	static_cast<void>(SelectedDifficulty);

	bool bVideoOpened = false;
	if (bShowingCompanionInfo && !PreviewedCompanionID.IsNone())
	{
		FName EffectiveCompanionSkinID = ResolveEffectiveCompanionSkinID(GameInstance, PreviewedCompanionID);
		if (EffectiveCompanionSkinID.IsNone())
		{
			EffectiveCompanionSkinID = FName(TEXT("Default"));
		}

		FT66FrontendVideoAsset VideoAsset;
		if (T66FrontendVideoCatalog::ResolveCompanionSelection(PreviewedCompanionID, EffectiveCompanionSkinID, VideoAsset))
		{
			if (!HeroPreviewVideoPlayer)
			{
				HeroPreviewVideoPlayer = NewObject<UT66FrontendVideoPlayer>(const_cast<UT66HeroSelectionPreviewController*>(this));
			}
			if (HeroPreviewVideoPlayer)
			{
				bVideoOpened = HeroPreviewVideoPlayer->OpenVideo(
					VideoAsset,
					FVector2D(712.f, 680.f),
					FName(TEXT("HeroSelectionCompanionPreview")));
			}
		}
	}
	else if (!PreviewedHeroID.IsNone())
	{
		FName EffectiveSkinID = ResolveEffectiveHeroSkinID(GameInstance);
		if (EffectiveSkinID.IsNone())
		{
			EffectiveSkinID = FName(TEXT("Default"));
		}

		FT66FrontendVideoAsset VideoAsset;
		if (T66FrontendVideoCatalog::ResolveHeroSelection(PreviewedHeroID, EffectiveSkinID, SelectedBodyType, VideoAsset))
		{
			if (!HeroPreviewVideoPlayer)
			{
				HeroPreviewVideoPlayer = NewObject<UT66FrontendVideoPlayer>(const_cast<UT66HeroSelectionPreviewController*>(this));
			}
			if (HeroPreviewVideoPlayer)
			{
				bVideoOpened = HeroPreviewVideoPlayer->OpenVideo(
					VideoAsset,
					FVector2D(712.f, 680.f),
					FName(TEXT("HeroSelectionHeroPreview")));
			}
		}
	}

	if (!bVideoOpened && HeroPreviewVideoPlayer)
	{
		HeroPreviewVideoPlayer->CloseVideo();
	}

	for (const TWeakPtr<SBorder>& WeakColorBox : HeroPreviewColorBoxes)
	{
		if (TSharedPtr<SBorder> PreviewColorBox = WeakColorBox.Pin())
		{
			PreviewColorBox->SetBorderBackgroundColor(bVideoOpened ? FLinearColor::Transparent : FallbackColor);
		}
	}
	for (const TWeakPtr<SImage>& WeakVideoImage : HeroPreviewVideoImages)
	{
		if (TSharedPtr<SImage> PreviewVideoImage = WeakVideoImage.Pin())
		{
			PreviewVideoImage->SetImage(GetHeroPreviewVideoBrush());
			PreviewVideoImage->Invalidate(EInvalidateWidgetReason::Paint);
		}
	}
}

void UT66HeroSelectionPreviewController::ResetHeroPreviewStateForHeroSwitch()
{
	SelectedHeroPreviewClip = ET66HeroSelectionPreviewClip::Overview;
	ResetHeroSkinPreviewOverride();
}

void UT66HeroSelectionPreviewController::ResetHeroSkinPreviewOverride()
{
	PreviewSkinIDOverride = NAME_None;
}

void UT66HeroSelectionPreviewController::ResetCompanionSkinPreviewOverride()
{
	PreviewedCompanionSkinIDOverride = NAME_None;
}

void UT66HeroSelectionPreviewController::ToggleHeroSkinPreviewOverride(FName SkinID)
{
	PreviewSkinIDOverride = (PreviewSkinIDOverride == SkinID) ? NAME_None : SkinID;
}

void UT66HeroSelectionPreviewController::ToggleCompanionSkinPreviewOverride(FName SkinID)
{
	PreviewedCompanionSkinIDOverride = (PreviewedCompanionSkinIDOverride == SkinID) ? NAME_None : SkinID;
}

ET66HeroSelectionPreviewClip UT66HeroSelectionPreviewController::GetSelectedPreviewClip() const
{
	return SelectedHeroPreviewClip;
}

bool UT66HeroSelectionPreviewController::IsSelectedPreviewClip(ET66HeroSelectionPreviewClip Clip) const
{
	return SelectedHeroPreviewClip == Clip;
}

void UT66HeroSelectionPreviewController::ToggleSelectedPreviewClip(ET66HeroSelectionPreviewClip Clip, bool bShowingCompanionInfo)
{
	if (bShowingCompanionInfo)
	{
		return;
	}

	SelectedHeroPreviewClip = (SelectedHeroPreviewClip == Clip)
		? ET66HeroSelectionPreviewClip::Overview
		: Clip;
}

FName UT66HeroSelectionPreviewController::GetHeroSkinPreviewOverride() const
{
	return PreviewSkinIDOverride;
}

FName UT66HeroSelectionPreviewController::GetCompanionSkinPreviewOverride() const
{
	return PreviewedCompanionSkinIDOverride;
}

FName UT66HeroSelectionPreviewController::ResolveEffectiveHeroSkinID(const UT66GameInstance* GameInstance) const
{
	FName EffectiveSkinID = PreviewSkinIDOverride.IsNone()
		? (GameInstance && !GameInstance->SelectedHeroSkinID.IsNone() ? GameInstance->SelectedHeroSkinID : FName(TEXT("Default")))
		: PreviewSkinIDOverride;
	if (EffectiveSkinID.IsNone())
	{
		EffectiveSkinID = FName(TEXT("Default"));
	}
	return EffectiveSkinID;
}

FName UT66HeroSelectionPreviewController::ResolveEffectiveCompanionSkinID(
	const UT66GameInstance* GameInstance,
	FName PreviewedCompanionID) const
{
	if (PreviewedCompanionID.IsNone())
	{
		return NAME_None;
	}

	if (!PreviewedCompanionSkinIDOverride.IsNone())
	{
		return PreviewedCompanionSkinIDOverride;
	}

	if (GameInstance)
	{
		if (UT66SkinSubsystem* SkinSub = GameInstance->GetSubsystem<UT66SkinSubsystem>())
		{
			return SkinSub->GetEquippedCompanionSkinID(PreviewedCompanionID);
		}
	}

	return FName(TEXT("Default"));
}

const FSlateBrush* UT66HeroSelectionPreviewController::GetHeroPreviewVideoBrush() const
{
	return HeroPreviewVideoPlayer
		? HeroPreviewVideoPlayer->GetVideoBrush()
		: nullptr;
}

const FSlateBrush* UT66HeroSelectionPreviewController::GetCompanionInfoPortraitBrush() const
{
	return CompanionInfoPortraitBrush.IsValid()
		? CompanionInfoPortraitBrush.Get()
		: nullptr;
}

UT66HeroSelectionScreen* UT66HeroSelectionPreviewController::GetOwnerScreen() const
{
	if (OwnerScreen.IsValid())
	{
		return OwnerScreen.Get();
	}

	return Cast<UT66HeroSelectionScreen>(GetOuter());
}
