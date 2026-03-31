// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBRuntime.h"

#include "Interfaces/IPluginManager.h"
#include "SceneViewExtension.h"

#if WITH_EDITOR
#include "ZibraVDBMaterialComponent.h"
#include "ZibraVDBMaterialComponentWarningsCustomization.h"
#include "PropertyCustomizationHelpers.h"
#endif

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

void FZibraVDBRuntimeModule::StartupModule() noexcept
{

#if WITH_EDITOR
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(UZibraVDBMaterialComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FZibraVDBMaterialComponentWarningsCustomization::MakeInstance));
#endif

}

void FZibraVDBRuntimeModule::ShutdownModule() noexcept
{
}

FZibraVDBRuntimeModule::TZibraRenderExtensionPtr FZibraVDBRuntimeModule::GetZibraRenderExtension()
{
	static TZibraRenderExtensionPtr ZibraVDBRenderExtensionSinglethonInstance(
		FSceneViewExtensions::NewExtension<FZibraVDBMaterialRendering>());
	return ZibraVDBRenderExtensionSinglethonInstance;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FZibraVDBRuntimeModule, ZibraVDBRuntime)
