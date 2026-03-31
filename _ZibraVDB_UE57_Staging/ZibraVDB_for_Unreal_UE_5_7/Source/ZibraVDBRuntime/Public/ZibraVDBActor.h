// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Components/PrimitiveComponent.h"
#include "ZibraVDBReflectionsManagerComponent.h"

#if WITH_EDITOR
#include "Settings/EditorStyleSettings.h"
#endif

#include "ZibraVDBActor.generated.h"

class UZibraVDBPlaybackComponent;
class UZibraVdbVolumeBase;
class UZibraVDBAssetComponent;
class UZibraVDBMaterialComponent;

UCLASS(ClassGroup = Rendering,
	HideCategories = (Activation, Collision, Cooking, Tags, AssetUserData, HLOD, Rendering, Input, Actor, Replication),
	Meta = (ComponentWrapperClass))
class ZIBRAVDBRUNTIME_API AZibraVDBActor final : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UZibraVDBAssetComponent* GetAssetComponent() const noexcept;
	UZibraVDBMaterialComponent* GetMaterialComponent() const noexcept;
	UZibraVDBPlaybackComponent* GetPlaybackComponent() const noexcept;
	FBoxSphereBounds GetBBox() const noexcept;

#if WITH_EDITOR
	bool GetReferencedContentObjects(TArray<UObject*>& Objects) const final;

	//~ Begin AActor Interface.
	bool ShouldTickIfViewportsOnly() const final;
	//~ End AActor Interface.
	
	const UEditorStyleSettings* EditorStyleSettings;
#endif

	void AttachReflectionManagerComponent();
	void DetachReflectionManagerComponent();

	void UpdateVisibility() noexcept;
	void LogVerboseRenderParams();
	bool LogCommonVDBInfo();

	//~ Begin AActor Interface.
	void Tick(float DeltaTime) final;
	void PostLoad() final;
	void OnConstruction(const FTransform& Transform) final;
	//~ End AActor Interface.

	bool bCommonInfoLogged = false;

	UPROPERTY(BlueprintReadOnly, Category = Volumetric, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UZibraVDBAssetComponent> AssetComponent;

	UPROPERTY(BlueprintReadOnly, Category = Volumetric, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UZibraVDBMaterialComponent> MaterialComponent;

	UPROPERTY(BlueprintReadOnly, Category = Volumetric, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UZibraVDBPlaybackComponent> PlaybackComponent;

	UPROPERTY(BlueprintReadOnly, Category = Volumetric, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UZibraVdbReflectionsManagerComponent> ReflectionManagerComponent;

	friend class UZibraVDBMaterialComponent;
};
