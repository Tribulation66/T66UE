// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/T66RetroFXSettings.h"
#include "T66FrontendUIRootWidget.generated.h"

class SBox;
class SRetainerWidget;

/**
 * Frontend-only retained root for UIManager-owned screens and chrome.
 *
 * Gameplay HUDs, in-run overlays, world prompts, and the dev console must stay
 * outside this root so the frontend CRT pass cannot leak into active gameplay.
 */
UCLASS()
class T66_API UT66FrontendUIRootWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMainScreen(UUserWidget* Widget);
	void ClearMainScreen();

	void SetTopBar(UUserWidget* Widget);
	void ClearTopBar();

	void SetModal(UUserWidget* Widget);
	void ClearModal();

	void SetLoading(UUserWidget* Widget);
	void ClearLoading();

	void SetPopup(UUserWidget* Widget);
	void ClearPopup();

	bool IsTopBarVisible() const;
	bool IsPopupVisible() const;

	void ApplyRetroFXSettings(const FT66RetroFXSettings& Settings);
	void RefreshLayerWidget(UUserWidget* Widget);
	void RequestFrontendPaintRefresh();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
	void SetLayerWidget(TObjectPtr<UUserWidget>& StoredWidget, const TSharedPtr<SBox>& LayerBox, UUserWidget* Widget);
	void ReapplyLayerWidget(const TObjectPtr<UUserWidget>& StoredWidget, const TSharedPtr<SBox>& LayerBox);
	void ReapplyAllLayerWidgets();
	void ApplySettingsToRetainer();
	void EnsureRetainerMaterial();

	UPROPERTY()
	TObjectPtr<UUserWidget> MainScreenWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> TopBarWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> ModalWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> PopupWidget;

	FT66RetroFXSettings CurrentSettings;

	TSharedPtr<SRetainerWidget> RetainerWidget;
	TSharedPtr<SBox> MainScreenBox;
	TSharedPtr<SBox> TopBarBox;
	TSharedPtr<SBox> ModalBox;
	TSharedPtr<SBox> LoadingBox;
	TSharedPtr<SBox> PopupBox;
};
