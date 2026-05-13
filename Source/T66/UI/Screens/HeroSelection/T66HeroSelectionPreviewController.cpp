// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionPreviewController.h"
#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

#include "Core/T66CharacterVisualSubsystem.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

using namespace T66HeroSelectionPrivate;

void UT66HeroSelectionPreviewController::Initialize(UT66HeroSelectionScreen* InOwnerScreen)
{
	OwnerScreen = InOwnerScreen;
	CachedHeroPreviewStage.Reset();
	CachedCompanionPreviewStage.Reset();
}

TSharedRef<SWidget> UT66HeroSelectionPreviewController::CreateHeroPreviewWidget(const FLinearColor& FallbackColor)
{
	if (AT66HeroPreviewStage* PreviewStage = GetHeroPreviewStage())
	{
		const TWeakObjectPtr<AT66HeroPreviewStage> WeakPreviewStage(PreviewStage);
		return SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(ST66DragRotateStagePreview)
				.DegreesPerPixel(0.28f)
				.OnRotateYaw(FT66DragPreviewDeltaDelegate::CreateLambda([WeakPreviewStage](const float DeltaYaw)
				{
					if (AT66HeroPreviewStage* Stage = WeakPreviewStage.Get())
					{
						Stage->AddPreviewYaw(DeltaYaw);
					}
				}))
				.OnZoom(FT66DragPreviewDeltaDelegate::CreateLambda([WeakPreviewStage](const float ZoomDelta)
				{
					if (AT66HeroPreviewStage* Stage = WeakPreviewStage.Get())
					{
						Stage->AddPreviewZoom(ZoomDelta);
						T66PositionHeroPreviewCamera(Stage);
					}
				}))
			];
	}

	TSharedPtr<SBorder> PreviewColorBoxWidget;
	const TSharedRef<SBorder> PreviewColorBoxWidgetRef =
		SAssignNew(PreviewColorBoxWidget, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FallbackColor)
		[
			SNew(SBox)
		];
	HeroPreviewColorBox = PreviewColorBoxWidget;
	return PreviewColorBoxWidgetRef;
}

void UT66HeroSelectionPreviewController::BindPreviewPanelWidgets(
	const TSharedPtr<SImage>& InHeroPreviewVideoImage,
	const TSharedPtr<STextBlock>& InHeroPreviewPlaceholderText,
	const TSharedPtr<SScaleBox>& InCompanionPreviewScaleBox,
	const TSharedPtr<STextBlock>& InCompanionPreviewPlaceholderText)
{
	HeroPreviewVideoImage = InHeroPreviewVideoImage;
	HeroPreviewPlaceholderText = InHeroPreviewPlaceholderText;
	CompanionInfoPortraitScaleBox = InCompanionPreviewScaleBox;
	CompanionPreviewPlaceholderText = InCompanionPreviewPlaceholderText;
	if (TSharedPtr<SImage> PreviewVideoImage = HeroPreviewVideoImage.Pin())
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

void UT66HeroSelectionPreviewController::ApplySelectionDifficultyToPreviewStages(ET66Difficulty SelectedDifficulty) const
{
	if (AT66HeroPreviewStage* PreviewStage = GetHeroPreviewStage())
	{
		PreviewStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		PreviewStage->SetPreviewDifficulty(SelectedDifficulty);
	}

	if (GetOwnerScreen())
	{
		if (AT66CompanionPreviewStage* CompanionStage = GetCompanionPreviewStage())
		{
			CompanionStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
			CompanionStage->SetPreviewDifficulty(SelectedDifficulty);
		}

		PositionPreviewCamera();
	}
}

void UT66HeroSelectionPreviewController::ApplyHeroPreviewStage(
	UT66GameInstance* GameInstance,
	FName PreviewedHeroID,
	FName PreviewedCompanionID,
	ET66BodyType SelectedBodyType,
	ET66Difficulty SelectedDifficulty,
	const FLinearColor& FallbackColor) const
{
	if (AT66HeroPreviewStage* PreviewStage = GetHeroPreviewStage())
	{
		if (TSharedPtr<SBorder> PreviewColorBox = HeroPreviewColorBox.Pin())
		{
			PreviewColorBox->SetBorderBackgroundColor(FLinearColor::Transparent);
		}

		FName EffectiveSkinID = ResolveEffectiveHeroSkinID(GameInstance);
		if (EffectiveSkinID.IsNone())
		{
			EffectiveSkinID = FName(TEXT("Default"));
		}

		const FName EffectiveCompanionSkinID = ResolveEffectiveCompanionSkinID(GameInstance, PreviewedCompanionID);
		PreviewStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		PreviewStage->SetPreviewDifficulty(SelectedDifficulty);
		UE_LOG(
			LogT66HeroSelection,
			Verbose,
			TEXT("[BEACH] UpdateHeroDisplay: PreviewSkinIDOverride=%s, GI->SelectedHeroSkinID=%s, EffectiveSkinID=%s"),
			*PreviewSkinIDOverride.ToString(),
			GameInstance ? *GameInstance->SelectedHeroSkinID.ToString() : TEXT("(null GI)"),
			*EffectiveSkinID.ToString());
		UE_LOG(
			LogT66HeroSelection,
			Verbose,
			TEXT("[BEACH] UpdateHeroDisplay: calling SetPreviewHero HeroID=%s BodyType=%d SkinID=%s"),
			*PreviewedHeroID.ToString(),
			static_cast<int32>(SelectedBodyType),
			*EffectiveSkinID.ToString());
		FString AutomationScreenshotPath;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66AutoScreenshot="), AutomationScreenshotPath))
		{
			const FName HeroVisualID = UT66CharacterVisualSubsystem::GetHeroVisualID(PreviewedHeroID, SelectedBodyType, EffectiveSkinID);
			if (!bPendingAutomationPreviewRefresh)
			{
				if (UT66CharacterVisualSubsystem* Visuals = GameInstance ? GameInstance->GetSubsystem<UT66CharacterVisualSubsystem>() : nullptr)
				{
					if (!Visuals->IsCharacterVisualReady(HeroVisualID))
					{
						bPendingAutomationPreviewRefresh = true;
						if (UWorld* World = GetOwnerScreen() ? GetOwnerScreen()->GetWorld() : nullptr)
						{
							FTimerHandle RetryHandle;
							const TWeakObjectPtr<UT66HeroSelectionPreviewController> WeakThis(const_cast<UT66HeroSelectionPreviewController*>(this));
							World->GetTimerManager().SetTimer(
								RetryHandle,
								FTimerDelegate::CreateLambda([
									WeakThis,
									GameInstance,
									PreviewedHeroID,
									PreviewedCompanionID,
									SelectedBodyType,
									SelectedDifficulty,
									FallbackColor]()
								{
									if (UT66HeroSelectionPreviewController* Controller = WeakThis.Get())
									{
										Controller->bPendingAutomationPreviewRefresh = false;
										Controller->ApplyHeroPreviewStage(
											GameInstance,
											PreviewedHeroID,
											PreviewedCompanionID,
											SelectedBodyType,
											SelectedDifficulty,
											FallbackColor);
									}
								}),
								1.25f,
								false);
						}
					}
				}
			}
			PreviewStage->SetPreviewHero(NAME_None, SelectedBodyType, EffectiveSkinID, NAME_None, NAME_None);
		}
		PreviewStage->SetPreviewHero(PreviewedHeroID, SelectedBodyType, EffectiveSkinID, PreviewedCompanionID, EffectiveCompanionSkinID);
		PositionPreviewCamera();
		return;
	}

	if (TSharedPtr<SBorder> PreviewColorBox = HeroPreviewColorBox.Pin())
	{
		PreviewColorBox->SetBorderBackgroundColor(FallbackColor);
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
	return nullptr;
}

const FSlateBrush* UT66HeroSelectionPreviewController::GetCompanionInfoPortraitBrush() const
{
	return CompanionInfoPortraitBrush.IsValid()
		? CompanionInfoPortraitBrush.Get()
		: nullptr;
}

AT66HeroPreviewStage* UT66HeroSelectionPreviewController::GetHeroPreviewStage() const
{
	if (AT66HeroPreviewStage* CachedStage = CachedHeroPreviewStage.Get())
	{
		return CachedStage;
	}

	UT66HeroSelectionScreen* Screen = GetOwnerScreen();
	UWorld* World = Screen ? Screen->GetWorld() : nullptr;
	if (!World)
	{
		return nullptr;
	}

	if (AT66PlayerController* PC = T66GetLocalFrontendHeroPlayerController(Screen))
	{
		PC->EnsureLocalFrontendPreviewScene();
	}

	if (AT66HeroPreviewStage* CachedStage = CachedHeroPreviewStage.Get())
	{
		return CachedStage;
	}

	// UI setup fallback: preview stages are spawned once by the frontend scene and
	// cached here so repeated selection refreshes do not rescan the world.
	for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It)
	{
		CachedHeroPreviewStage = *It;
		return CachedHeroPreviewStage.Get();
	}
	return nullptr;
}

AT66CompanionPreviewStage* UT66HeroSelectionPreviewController::GetCompanionPreviewStage() const
{
	if (AT66CompanionPreviewStage* CachedStage = CachedCompanionPreviewStage.Get())
	{
		return CachedStage;
	}

	UT66HeroSelectionScreen* Screen = GetOwnerScreen();
	UWorld* World = Screen ? Screen->GetWorld() : nullptr;
	if (!World)
	{
		return nullptr;
	}

	if (AT66PlayerController* PC = T66GetLocalFrontendHeroPlayerController(Screen))
	{
		PC->EnsureLocalFrontendPreviewScene();
	}

	if (AT66CompanionPreviewStage* CachedStage = CachedCompanionPreviewStage.Get())
	{
		return CachedStage;
	}

	// UI setup fallback: cached after first resolve for this controller.
	for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It)
	{
		CachedCompanionPreviewStage = *It;
		return CachedCompanionPreviewStage.Get();
	}
	return nullptr;
}

void UT66HeroSelectionPreviewController::PositionPreviewCamera() const
{
	if (UT66HeroSelectionScreen* Screen = GetOwnerScreen())
	{
		T66PositionHeroPreviewCamera(Screen);
	}
}

UT66HeroSelectionScreen* UT66HeroSelectionPreviewController::GetOwnerScreen() const
{
	if (OwnerScreen.IsValid())
	{
		return OwnerScreen.Get();
	}

	return Cast<UT66HeroSelectionScreen>(GetOuter());
}
