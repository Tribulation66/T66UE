// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendUIRootWidget.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Slate/SRetainerWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66FrontendUIRoot, Log, All);

namespace
{
	static const TCHAR* FrontendCRTMaterialPath = TEXT("/Game/UI/Materials/M_T66_UI_CRTPostProcess.M_T66_UI_CRTPostProcess");
	static const FName TextureParameterName(TEXT("Texture"));
	static const FName CRTEnabledParameterName(TEXT("CRTEnabled"));
	static const FName ScanlineStrengthParameterName(TEXT("ScanlineStrength"));
	static const FName PhosphorMaskStrengthParameterName(TEXT("PhosphorMaskStrength"));
	static const FName BloomStrengthParameterName(TEXT("BloomStrength"));
	static const FName ChromaticAberrationStrengthParameterName(TEXT("ChromaticAberrationStrength"));
	static const FName BarrelDistortionStrengthParameterName(TEXT("BarrelDistortionStrength"));
	static const FName VignetteStrengthParameterName(TEXT("VignetteStrength"));
	static const FName ColorQuantizationBitsParameterName(TEXT("ColorQuantizationBits"));
	static const FName ReferenceResolutionHeightParameterName(TEXT("ReferenceResolutionHeight"));
	static const FName UITextureSizeParameterName(TEXT("UITextureSize"));

	static TWeakObjectPtr<UMaterialInterface> GCachedCRTMaterial;
	static bool bCheckedCRTMaterial = false;

	UMaterialInterface* LoadFrontendCRTMaterial()
	{
		if (!GCachedCRTMaterial.IsValid())
		{
			GCachedCRTMaterial = LoadObject<UMaterialInterface>(nullptr, FrontendCRTMaterialPath);
			if (!bCheckedCRTMaterial && !GCachedCRTMaterial.IsValid())
			{
				UE_LOG(LogT66FrontendUIRoot, Warning, TEXT("Frontend CRT material not found: %s"), FrontendCRTMaterialPath);
			}
		}

		bCheckedCRTMaterial = true;
		return GCachedCRTMaterial.Get();
	}

	FVector2D ResolveViewportSize()
	{
		if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
		{
			const FIntPoint SizeXY = GEngine->GameViewport->Viewport->GetSizeXY();
			if (SizeXY.X > 0 && SizeXY.Y > 0)
			{
				return FVector2D(static_cast<float>(SizeXY.X), static_cast<float>(SizeXY.Y));
			}
		}

		return FVector2D(1920.f, 1080.f);
	}
}

void UT66FrontendUIRootWidget::SetMainScreen(UUserWidget* Widget)
{
	SetLayerWidget(MainScreenWidget, MainScreenBox, Widget);
}

void UT66FrontendUIRootWidget::ClearMainScreen()
{
	SetLayerWidget(MainScreenWidget, MainScreenBox, nullptr);
}

void UT66FrontendUIRootWidget::SetTopBar(UUserWidget* Widget)
{
	SetLayerWidget(TopBarWidget, TopBarBox, Widget);
}

void UT66FrontendUIRootWidget::ClearTopBar()
{
	SetLayerWidget(TopBarWidget, TopBarBox, nullptr);
}

void UT66FrontendUIRootWidget::SetModal(UUserWidget* Widget)
{
	SetLayerWidget(ModalWidget, ModalBox, Widget);
}

void UT66FrontendUIRootWidget::ClearModal()
{
	SetLayerWidget(ModalWidget, ModalBox, nullptr);
}

void UT66FrontendUIRootWidget::SetLoading(UUserWidget* Widget)
{
	SetLayerWidget(LoadingWidget, LoadingBox, Widget);
}

void UT66FrontendUIRootWidget::ClearLoading()
{
	SetLayerWidget(LoadingWidget, LoadingBox, nullptr);
}

void UT66FrontendUIRootWidget::SetPopup(UUserWidget* Widget)
{
	SetLayerWidget(PopupWidget, PopupBox, Widget);
}

void UT66FrontendUIRootWidget::ClearPopup()
{
	SetLayerWidget(PopupWidget, PopupBox, nullptr);
}

bool UT66FrontendUIRootWidget::IsTopBarVisible() const
{
	return TopBarWidget != nullptr && TopBarWidget->GetCachedWidget().IsValid();
}

bool UT66FrontendUIRootWidget::IsPopupVisible() const
{
	return PopupWidget != nullptr && PopupWidget->GetCachedWidget().IsValid();
}

void UT66FrontendUIRootWidget::ApplyRetroFXSettings(const FT66RetroFXSettings& Settings)
{
	CurrentSettings = Settings;
	ApplySettingsToRetainer();
}

void UT66FrontendUIRootWidget::RefreshLayerWidget(UUserWidget* Widget)
{
	if (!Widget)
	{
		return;
	}

	TSharedPtr<SBox> TargetLayerBox;
	if (Widget == MainScreenWidget)
	{
		TargetLayerBox = MainScreenBox;
	}
	else if (Widget == TopBarWidget)
	{
		TargetLayerBox = TopBarBox;
	}
	else if (Widget == ModalWidget)
	{
		TargetLayerBox = ModalBox;
	}
	else if (Widget == LoadingWidget)
	{
		TargetLayerBox = LoadingBox;
	}
	else if (Widget == PopupWidget)
	{
		TargetLayerBox = PopupBox;
	}

	if (!TargetLayerBox.IsValid())
	{
		return;
	}

	// Detach the stale Slate subtree before rebuilding the widget. Without this
	// explicit detach/reattach, state changes can be committed on the UWidget
	// while the frontend retainer continues presenting the previous Slate tree.
	TargetLayerBox->SetContent(SNullWidget::NullWidget);
	TargetLayerBox->Invalidate(EInvalidateWidgetReason::Layout);

	Widget->ReleaseSlateResources(true);
	Widget->InvalidateLayoutAndVolatility();

	if (Widget == MainScreenWidget)
	{
		ReapplyLayerWidget(MainScreenWidget, MainScreenBox);
	}
	else if (Widget == TopBarWidget)
	{
		ReapplyLayerWidget(TopBarWidget, TopBarBox);
	}
	else if (Widget == ModalWidget)
	{
		ReapplyLayerWidget(ModalWidget, ModalBox);
	}
	else if (Widget == LoadingWidget)
	{
		ReapplyLayerWidget(LoadingWidget, LoadingBox);
	}
	else if (Widget == PopupWidget)
	{
		ReapplyLayerWidget(PopupWidget, PopupBox);
	}

	TargetLayerBox->Invalidate(EInvalidateWidgetReason::Layout);
	if (RetainerWidget.IsValid())
	{
		RetainerWidget->Invalidate(EInvalidateWidgetReason::Paint);
		RetainerWidget->RequestRender();
	}
}

void UT66FrontendUIRootWidget::RequestFrontendPaintRefresh()
{
	if (MainScreenBox.IsValid())
	{
		MainScreenBox->Invalidate(EInvalidateWidgetReason::Paint);
	}
	if (TopBarBox.IsValid())
	{
		TopBarBox->Invalidate(EInvalidateWidgetReason::Paint);
	}
	if (ModalBox.IsValid())
	{
		ModalBox->Invalidate(EInvalidateWidgetReason::Paint);
	}
	if (LoadingBox.IsValid())
	{
		LoadingBox->Invalidate(EInvalidateWidgetReason::Paint);
	}
	if (PopupBox.IsValid())
	{
		PopupBox->Invalidate(EInvalidateWidgetReason::Paint);
	}
	if (RetainerWidget.IsValid())
	{
		RetainerWidget->Invalidate(EInvalidateWidgetReason::Paint);
		RetainerWidget->RequestRender();
	}
}

TSharedRef<SWidget> UT66FrontendUIRootWidget::RebuildWidget()
{
	TSharedRef<SWidget> LayerStack =
		SNew(SOverlay)

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(MainScreenBox, SBox)
			.Visibility(EVisibility::Collapsed)
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(TopBarBox, SBox)
			.Visibility(EVisibility::Collapsed)
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(ModalBox, SBox)
			.Visibility(EVisibility::Collapsed)
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(LoadingBox, SBox)
			.Visibility(EVisibility::Collapsed)
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(PopupBox, SBox)
			.Visibility(EVisibility::Collapsed)
		];

	SAssignNew(RetainerWidget, SRetainerWidget)
		.Visibility(EVisibility::SelfHitTestInvisible)
		.RenderOnPhase(true)
		.RenderOnInvalidation(true)
		.Phase(0)
		.PhaseCount(1)
		.StatId(FName(TEXT("T66FrontendUIRootCRT")))
		[
			LayerStack
		];

	RetainerWidget->SetTextureParameter(TextureParameterName);
	EnsureRetainerMaterial();
	if (UWorld* World = GetWorld())
	{
		RetainerWidget->SetWorld(World);
	}

	ReapplyAllLayerWidgets();
	ApplySettingsToRetainer();
	return RetainerWidget.ToSharedRef();
}

void UT66FrontendUIRootWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	RetainerWidget.Reset();
	MainScreenBox.Reset();
	TopBarBox.Reset();
	ModalBox.Reset();
	LoadingBox.Reset();
	PopupBox.Reset();
}

void UT66FrontendUIRootWidget::SetLayerWidget(TObjectPtr<UUserWidget>& StoredWidget, const TSharedPtr<SBox>& LayerBox, UUserWidget* Widget)
{
	StoredWidget = Widget;
	ReapplyLayerWidget(StoredWidget, LayerBox);
	if (RetainerWidget.IsValid())
	{
		RetainerWidget->RequestRender();
	}
}

void UT66FrontendUIRootWidget::ReapplyLayerWidget(const TObjectPtr<UUserWidget>& StoredWidget, const TSharedPtr<SBox>& LayerBox)
{
	if (!LayerBox.IsValid())
	{
		return;
	}

	if (StoredWidget)
	{
		LayerBox->SetContent(StoredWidget->TakeWidget());
		LayerBox->SetVisibility(EVisibility::SelfHitTestInvisible);
	}
	else
	{
		LayerBox->SetContent(SNullWidget::NullWidget);
		LayerBox->SetVisibility(EVisibility::Collapsed);
	}
}

void UT66FrontendUIRootWidget::ReapplyAllLayerWidgets()
{
	ReapplyLayerWidget(MainScreenWidget, MainScreenBox);
	ReapplyLayerWidget(TopBarWidget, TopBarBox);
	ReapplyLayerWidget(ModalWidget, ModalBox);
	ReapplyLayerWidget(LoadingWidget, LoadingBox);
	ReapplyLayerWidget(PopupWidget, PopupBox);
}

void UT66FrontendUIRootWidget::ApplySettingsToRetainer()
{
	if (!RetainerWidget.IsValid())
	{
		return;
	}

	EnsureRetainerMaterial();
	UMaterialInstanceDynamic* Material = RetainerWidget->GetEffectMaterial();
	if (!Material)
	{
		return;
	}

	const float Enabled = CurrentSettings.UIFullScreenCRTEnabled ? 1.0f : 0.0f;
	Material->SetScalarParameterValue(CRTEnabledParameterName, Enabled);
	Material->SetScalarParameterValue(ScanlineStrengthParameterName, CurrentSettings.UIFullScreenCRTEnabled ? FMath::Clamp(CurrentSettings.UICRTScanlineStrength, 0.0f, 1.0f) : 0.0f);
	Material->SetScalarParameterValue(PhosphorMaskStrengthParameterName, CurrentSettings.UIFullScreenCRTEnabled ? FMath::Clamp(CurrentSettings.UICRTPhosphorMaskStrength, 0.0f, 1.0f) : 0.0f);
	Material->SetScalarParameterValue(BloomStrengthParameterName, CurrentSettings.UIFullScreenCRTEnabled ? FMath::Clamp(CurrentSettings.UICRTBloomStrength, 0.0f, 1.0f) : 0.0f);
	Material->SetScalarParameterValue(ChromaticAberrationStrengthParameterName, CurrentSettings.UIFullScreenCRTEnabled ? FMath::Clamp(CurrentSettings.UICRTChromaticAberrationStrength, 0.0f, 1.0f) : 0.0f);
	Material->SetScalarParameterValue(BarrelDistortionStrengthParameterName, CurrentSettings.UIFullScreenCRTEnabled ? FMath::Clamp(CurrentSettings.UICRTBarrelDistortionStrength, 0.0f, 1.0f) : 0.0f);
	Material->SetScalarParameterValue(VignetteStrengthParameterName, CurrentSettings.UIFullScreenCRTEnabled ? FMath::Clamp(CurrentSettings.UICRTVignetteStrength, 0.0f, 1.0f) : 0.0f);
	Material->SetScalarParameterValue(ColorQuantizationBitsParameterName, static_cast<float>(FMath::Clamp(CurrentSettings.UICRTColorQuantizationBits, 4, 8)));
	Material->SetScalarParameterValue(ReferenceResolutionHeightParameterName, static_cast<float>(FMath::Max(CurrentSettings.UICRTReferenceResolutionHeight, 120)));

	const FVector2D ViewportSize = ResolveViewportSize();
	Material->SetVectorParameterValue(UITextureSizeParameterName, FLinearColor(ViewportSize.X, ViewportSize.Y, 0.0f, 0.0f));
	RetainerWidget->RequestRender();
}

void UT66FrontendUIRootWidget::EnsureRetainerMaterial()
{
	if (!RetainerWidget.IsValid())
	{
		return;
	}

	if (!RetainerWidget->GetEffectMaterial())
	{
		RetainerWidget->SetEffectMaterial(LoadFrontendCRTMaterial());
	}
}
