// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionPreviewController.h"
#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

using namespace T66HeroSelectionPrivate;

void UT66HeroSelectionPreviewController::Initialize(UT66HeroSelectionScreen* InOwnerScreen)
{
	OwnerScreen = InOwnerScreen;
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
		.BorderBackgroundColor(FT66Style::IsDotaTheme() ? FLinearColor::Transparent : FallbackColor)
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
}

void UT66HeroSelectionPreviewController::EnsureHeroPreviewVideoResources()
{
	if (HeroPreviewMediaPlayer && HeroPreviewMediaTexture && HeroPreviewVideoBrush.IsValid())
	{
		return;
	}

	HeroPreviewMediaPlayer = NewObject<UMediaPlayer>(this, UMediaPlayer::StaticClass(), NAME_None, RF_Transient);
	if (HeroPreviewMediaPlayer)
	{
		HeroPreviewMediaPlayer->SetLooping(true);
		HeroPreviewMediaPlayer->PlayOnOpen = true;
	}

	HeroPreviewMediaTexture = NewObject<UMediaTexture>(this, UMediaTexture::StaticClass(), NAME_None, RF_Transient);
	if (HeroPreviewMediaTexture && HeroPreviewMediaPlayer)
	{
		HeroPreviewMediaTexture->SetMediaPlayer(HeroPreviewMediaPlayer);
		HeroPreviewMediaTexture->UpdateResource();
	}

	HeroPreviewVideoBrush = MakeShared<FSlateBrush>();
	HeroPreviewVideoBrush->DrawAs = ESlateBrushDrawType::Image;
	HeroPreviewVideoBrush->Tiling = ESlateBrushTileType::NoTile;
	HeroPreviewVideoBrush->ImageSize = FVector2D(640.0f, 360.0f);
	if (HeroPreviewMediaTexture)
	{
		HeroPreviewVideoBrush->SetResourceObject(HeroPreviewMediaTexture);
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

void UT66HeroSelectionPreviewController::WarmKnightPreviewMediaSource(FName PreviewedHeroID, bool bShowingCompanionInfo)
{
	if (KnightPreviewMediaSource)
	{
		return;
	}

	const FString MoviePath = ResolveArthurPreviewMoviePath();
	if (MoviePath.IsEmpty())
	{
		return;
	}

	KnightPreviewMediaSource = NewObject<UFileMediaSource>(this, NAME_None, RF_Transient);
	if (KnightPreviewMediaSource)
	{
		KnightPreviewMediaSource->SetFilePath(MoviePath);
		HandleKnightPreviewMediaSourceLoaded(PreviewedHeroID, bShowingCompanionInfo);
	}
}

void UT66HeroSelectionPreviewController::CloseHeroPreviewMedia()
{
	if (HeroPreviewMediaPlayer)
	{
		HeroPreviewMediaPlayer->Close();
	}

	ActivePreviewVideoHeroID = NAME_None;
	ActiveHeroPreviewClip = ET66HeroSelectionPreviewClip::Overview;
}

void UT66HeroSelectionPreviewController::UpdateHeroPreviewVideo(FName PreviewedHeroID, bool bShowingCompanionInfo)
{
	FString PreviewMoviePath;
	if (!bShowingCompanionInfo && PreviewedHeroID == FName(TEXT("Hero_1")))
	{
		switch (SelectedHeroPreviewClip)
		{
		case ET66HeroSelectionPreviewClip::Ultimate:
		case ET66HeroSelectionPreviewClip::Passive:
		case ET66HeroSelectionPreviewClip::Overview:
		default:
			PreviewMoviePath = ResolveArthurPreviewMoviePath();
			break;
		}
	}

	const bool bShowHeroVideo = !PreviewMoviePath.IsEmpty();

	if (TSharedPtr<SImage> PreviewVideoImage = HeroPreviewVideoImage.Pin())
	{
		PreviewVideoImage->SetVisibility(bShowHeroVideo ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (TSharedPtr<STextBlock> PreviewPlaceholder = HeroPreviewPlaceholderText.Pin())
	{
		PreviewPlaceholder->SetVisibility(
			!bShowingCompanionInfo && !bShowHeroVideo
				? EVisibility::Visible
				: EVisibility::Collapsed);
	}

	if (!HeroPreviewMediaPlayer)
	{
		return;
	}

	if (bShowHeroVideo)
	{
		const FString CurrentUrl = HeroPreviewMediaPlayer->GetUrl();
		const bool bNeedsReopen = !HeroPreviewMediaPlayer->IsPlaying()
			|| ActivePreviewVideoHeroID != PreviewedHeroID
			|| ActiveHeroPreviewClip != SelectedHeroPreviewClip
			|| !CurrentUrl.Equals(PreviewMoviePath, ESearchCase::IgnoreCase);
		if (bNeedsReopen)
		{
			HeroPreviewMediaPlayer->Close();
			if (!HeroPreviewMediaPlayer->OpenFile(PreviewMoviePath))
			{
				UE_LOG(LogT66HeroSelection, Warning, TEXT("Failed to open hero preview video from '%s'"), *PreviewMoviePath);
			}
			ActivePreviewVideoHeroID = PreviewedHeroID;
			ActiveHeroPreviewClip = SelectedHeroPreviewClip;
		}
	}
	else
	{
		HeroPreviewMediaPlayer->Close();
		ActivePreviewVideoHeroID = NAME_None;
		ActiveHeroPreviewClip = ET66HeroSelectionPreviewClip::Overview;
	}
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

	if (UT66HeroSelectionScreen* Screen = GetOwnerScreen())
	{
		if (UWorld* World = Screen->GetWorld())
		{
			for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It)
			{
				AT66CompanionPreviewStage* CompanionStage = *It;
				CompanionStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
				CompanionStage->SetPreviewDifficulty(SelectedDifficulty);
				break;
			}

			PositionPreviewCamera();
		}
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
		PreviewStage->SetPreviewHero(PreviewedHeroID, SelectedBodyType, EffectiveSkinID, PreviewedCompanionID, EffectiveCompanionSkinID);
		PositionPreviewCamera();
		return;
	}

	if (TSharedPtr<SBorder> PreviewColorBox = HeroPreviewColorBox.Pin())
	{
		PreviewColorBox->SetBorderBackgroundColor(FT66Style::IsDotaTheme() ? FLinearColor::Transparent : FallbackColor);
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
	return HeroPreviewVideoBrush.IsValid() && ::IsValid(HeroPreviewMediaTexture)
		? HeroPreviewVideoBrush.Get()
		: nullptr;
}

const FSlateBrush* UT66HeroSelectionPreviewController::GetCompanionInfoPortraitBrush() const
{
	return CompanionInfoPortraitBrush.IsValid()
		? CompanionInfoPortraitBrush.Get()
		: nullptr;
}

AT66HeroPreviewStage* UT66HeroSelectionPreviewController::GetHeroPreviewStage() const
{
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

	for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

void UT66HeroSelectionPreviewController::HandleKnightPreviewMediaSourceLoaded(FName PreviewedHeroID, bool bShowingCompanionInfo)
{
	UpdateHeroPreviewVideo(PreviewedHeroID, bShowingCompanionInfo);
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
