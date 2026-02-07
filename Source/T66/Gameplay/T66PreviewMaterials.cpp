// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PreviewMaterials.h"
#include "Materials/MaterialInterface.h"

#if WITH_EDITORONLY_DATA
#include "Materials/Material.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "UObject/SavePackage.h"
#include "HAL/PlatformFileManager.h"
#endif

static const TCHAR* GroundAssetPath = TEXT("/Game/UI/Preview/M_PreviewGround.M_PreviewGround");
static const TCHAR* SkyAssetPath    = TEXT("/Game/UI/Preview/M_PreviewSky.M_PreviewSky");
static const TCHAR* StarAssetPath   = TEXT("/Game/UI/Preview/M_PreviewStar.M_PreviewStar");

// ---------------------------------------------------------------------------
#if WITH_EDITORONLY_DATA
// ---------------------------------------------------------------------------

static bool TrySavePackage(UPackage* Pkg, UObject* Asset, const FString& PkgName)
{
	if (!Pkg) return false;
	const FString File = FPackageName::LongPackageNameToFilename(PkgName, FPackageName::GetAssetPackageExtension());
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(File), true);
	FSavePackageArgs Args;
	Args.TopLevelFlags = RF_Standalone;
	return UPackage::SavePackage(Pkg, Asset, *File, Args);
}

// Helper: create a simple material with one VectorParameter connected to a single output.
// This is the ONLY pattern proven to work in this project's C++ material construction.
static UMaterial* CreateSimpleMaterial(
	const FString& PackageName,
	const FName& AssetName,
	const FName& ParamName,
	const FLinearColor& DefaultColor,
	bool bConnectToEmissive, // true = EmissiveColor (self-lit sky/star), false = BaseColor (lit ground)
	float Roughness = 1.0f)
{
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package) return nullptr;

	UMaterial* Mat = NewObject<UMaterial>(Package, AssetName, RF_Public | RF_Standalone);
	UMaterialEditorOnlyData* EdData = Mat->GetEditorOnlyData();
	if (!EdData)
	{
		UE_LOG(LogTemp, Error, TEXT("[PreviewMaterials] GetEditorOnlyData() null for %s"), *PackageName);
		return Mat;
	}

	// --- The one VectorParameter ---
	UMaterialExpressionVectorParameter* Param = NewObject<UMaterialExpressionVectorParameter>(Mat);
	Param->ParameterName = ParamName;
	Param->DefaultValue  = DefaultColor;
	Param->MaterialExpressionEditorX = -400;
	Param->MaterialExpressionEditorY = 0;
	EdData->ExpressionCollection.Expressions.Add(Param);

	if (bConnectToEmissive)
	{
		// Self-lit: VectorParam → EmissiveColor, black BaseColor to suppress diffuse.
		EdData->EmissiveColor.Expression = Param;

		UMaterialExpressionConstant3Vector* Black = NewObject<UMaterialExpressionConstant3Vector>(Mat);
		Black->Constant = FLinearColor(0.f, 0.f, 0.f, 1.f);
		Black->MaterialExpressionEditorX = -400;
		Black->MaterialExpressionEditorY = 200;
		EdData->ExpressionCollection.Expressions.Add(Black);
		EdData->BaseColor.Expression = Black;

		UMaterialExpressionConstant* Spec0 = NewObject<UMaterialExpressionConstant>(Mat);
		Spec0->R = 0.0f;
		Spec0->MaterialExpressionEditorX = -400;
		Spec0->MaterialExpressionEditorY = 500;
		EdData->ExpressionCollection.Expressions.Add(Spec0);
		EdData->Specular.Expression = Spec0;
	}
	else
	{
		// Lit: VectorParam → BaseColor (illuminated by scene lights).
		EdData->BaseColor.Expression = Param;
	}

	// Roughness
	UMaterialExpressionConstant* Rough = NewObject<UMaterialExpressionConstant>(Mat);
	Rough->R = Roughness;
	Rough->MaterialExpressionEditorX = -400;
	Rough->MaterialExpressionEditorY = 300;
	EdData->ExpressionCollection.Expressions.Add(Rough);
	EdData->Roughness.Expression = Rough;

	// Metallic = 0
	UMaterialExpressionConstant* Metal = NewObject<UMaterialExpressionConstant>(Mat);
	Metal->R = 0.0f;
	Metal->MaterialExpressionEditorX = -400;
	Metal->MaterialExpressionEditorY = 400;
	EdData->ExpressionCollection.Expressions.Add(Metal);
	EdData->Metallic.Expression = Metal;

	Mat->PreEditChange(nullptr);
	Mat->PostEditChange();
	Mat->MarkPackageDirty();

	if (TrySavePackage(Package, Mat, PackageName))
	{
		UE_LOG(LogTemp, Log, TEXT("[PreviewMaterials] Created and saved %s"), *PackageName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PreviewMaterials] Created %s in memory (save failed)"), *PackageName);
	}
	return Mat;
}

#endif // WITH_EDITORONLY_DATA
// ---------------------------------------------------------------------------

UMaterialInterface* T66PreviewMaterials::GetGroundMaterial()
{
	if (UMaterialInterface* F = LoadObject<UMaterialInterface>(nullptr, GroundAssetPath))
		return F;

#if WITH_EDITORONLY_DATA
	// Lit grass: "BaseColor" param → BaseColor output, Roughness 0.85.
	UMaterial* Mat = CreateSimpleMaterial(
		TEXT("/Game/UI/Preview/M_PreviewGround"), FName("M_PreviewGround"),
		FName("BaseColor"), FLinearColor(0.15f, 0.45f, 0.08f, 1.0f),
		/*bConnectToEmissive=*/ false, /*Roughness=*/ 0.85f);

	// Add disconnected VariationColor param (C++ sets it on the MID).
	if (Mat)
	{
		if (UMaterialEditorOnlyData* EdData = Mat->GetEditorOnlyData())
		{
			UMaterialExpressionVectorParameter* Var = NewObject<UMaterialExpressionVectorParameter>(Mat);
			Var->ParameterName = FName("VariationColor");
			Var->DefaultValue  = FLinearColor(0.10f, 0.35f, 0.05f, 1.0f);
			Var->MaterialExpressionEditorX = -400;
			Var->MaterialExpressionEditorY = 600;
			EdData->ExpressionCollection.Expressions.Add(Var);
		}
	}
	return Mat;
#else
	return nullptr;
#endif
}

UMaterialInterface* T66PreviewMaterials::GetSkyMaterial()
{
	if (UMaterialInterface* F = LoadObject<UMaterialInterface>(nullptr, SkyAssetPath))
		return F;

#if WITH_EDITORONLY_DATA
	// Self-lit sky dome: "SkyColor" → EmissiveColor.
	// C++ must set values >> 1.0 (emissive range) to survive auto-exposure.
	return CreateSimpleMaterial(
		TEXT("/Game/UI/Preview/M_PreviewSky"), FName("M_PreviewSky"),
		FName("SkyColor"), FLinearColor(2.0f, 3.0f, 5.0f, 1.0f),
		/*bConnectToEmissive=*/ true);
#else
	return nullptr;
#endif
}

UMaterialInterface* T66PreviewMaterials::GetStarMaterial()
{
	if (UMaterialInterface* F = LoadObject<UMaterialInterface>(nullptr, StarAssetPath))
		return F;

#if WITH_EDITORONLY_DATA
	// Self-lit star dot: "StarColor" → EmissiveColor.
	return CreateSimpleMaterial(
		TEXT("/Game/UI/Preview/M_PreviewStar"), FName("M_PreviewStar"),
		FName("StarColor"), FLinearColor(8.0f, 8.0f, 7.0f, 1.0f),
		/*bConnectToEmissive=*/ true);
#else
	return nullptr;
#endif
}
