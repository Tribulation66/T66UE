// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/T66DataTypes.h"
#include "T66LeaderboardFilterButton.generated.h"

class UButton;
class UImage;
class UMaterialInstanceDynamic;

/**
 * Leaderboard filter button: border-only button with content = static image (Global/Friends)
 * or animated sprite-sheet material (Streamers).
 * Use Blueprint subclasses to set StaticTexture or SpriteSheetMaterial + frame count.
 */
UCLASS(Blueprintable)
class T66_API UT66LeaderboardFilterButton : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Static icon (used for Global and Friends). If set, SpriteSheetMaterial is ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard Filter")
	TObjectPtr<UTexture2D> StaticTexture;

	/**
	 * Sprite-sheet material for animated icon (Streamers).
	 * Material must have scalar parameter "Frame" (0..FrameCount-1) and texture param for the sheet.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard Filter")
	TObjectPtr<UMaterialInterface> SpriteSheetMaterial;

	/** Number of frames in the sprite sheet (horizontal grid). Used only when SpriteSheetMaterial is set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard Filter", meta = (ClampMin = "1", UIMin = "1"))
	int32 FrameCount = 1;

	/** Frames per second for sprite-sheet animation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard Filter", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float FramesPerSecond = 8.0f;

	/** Which filter this button represents (for styling/selection). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard Filter")
	ET66LeaderboardFilter Filter = ET66LeaderboardFilter::Global;

	/** The button (border only). Created at runtime in C++ if not provided by the Blueprint. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UButton> FilterButton;

	/** The image showing the icon (texture or material). Created at runtime in C++ if not provided by the Blueprint. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UImage> IconImage;

	/** Called when the button is clicked. Panel binds to this. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFilterButtonClicked, ET66LeaderboardFilter, Filter);
	UPROPERTY(BlueprintAssignable, Category = "Leaderboard Filter")
	FOnFilterButtonClicked OnFilterButtonClicked;

	UFUNCTION(BlueprintCallable, Category = "Leaderboard Filter")
	void NotifyClicked();

	/** Call after setting StaticTexture or SpriteSheetMaterial so the icon updates (e.g. from Main Menu). */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard Filter")
	void RefreshIcon();

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	void SyncBrush();
	void AdvanceSpriteFrame(float DeltaTime);
	/** Create FilterButton and IconImage at runtime and add them to the root (used when Blueprint has no children). */
	void EnsureButtonAndImageCreated();

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SpriteDynamicMaterial;
	float SpriteTime = 0.0f;
	bool bSyncedBrushOnce = false;
};
