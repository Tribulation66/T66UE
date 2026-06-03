// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66Hero1AxePierceVFXCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CommandLine.h"
#include "MeshDescription.h"
#include "Misc/Parse.h"
#include "Misc/PackageName.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraCommon.h"
#include "NiagaraMeshRendererMeshProperties.h"
#include "NiagaraMeshRendererProperties.h"
#include "NiagaraScript.h"
#include "NiagaraSystem.h"
#include "StaticMeshAttributes.h"
#include "StaticMeshOperations.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

#if WITH_EDITOR
#include "NiagaraConstants.h"
#include "NiagaraEmitterFactoryNew.h"
#include "NiagaraEditorUtilities.h"
#include "NiagaraGraph.h"
#include "NiagaraNodeFunctionCall.h"
#include "NiagaraNodeOutput.h"
#include "NiagaraParameterMapHistory.h"
#include "NiagaraSystemFactoryNew.h"
#include "ViewModels/Stack/NiagaraParameterHandle.h"
#include "ViewModels/Stack/NiagaraStackGraphUtilities.h"
#endif

namespace
{
	const TCHAR* T66Hero1AxePierceNiagaraPath =
		TEXT("/Game/VFXLab/Hero1Axe/Pierce/NS_Hero1AxePierce_MeshSlash.NS_Hero1AxePierce_MeshSlash");
	const TCHAR* T66Hero1AxePierceNiagaraPackagePath =
		TEXT("/Game/VFXLab/Hero1Axe/Pierce/NS_Hero1AxePierce_MeshSlash");
	const TCHAR* T66Hero1AxePierceNiagaraObjectName =
		TEXT("NS_Hero1AxePierce_MeshSlash");
	const TCHAR* T66Hero1AxePierceMeshPackagePath =
		TEXT("/Game/VFXLab/Hero1Axe/Pierce/SM_Hero1AxePierce_BladePlane");
	const TCHAR* T66Hero1AxePierceMeshObjectName =
		TEXT("SM_Hero1AxePierce_BladePlane");

	bool T66UseHero1AxePierceProductionPaths()
	{
		return FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxePierceProduction"));
	}

	FString T66ResolveHero1AxePiercePath(const TCHAR* Path)
	{
		FString Resolved(Path);
		if (T66UseHero1AxePierceProductionPaths())
		{
			Resolved.ReplaceInline(TEXT("/Game/VFXLab/Hero1Axe/Pierce"), TEXT("/Game/VFX/Hero1/Axe/Pierce"), ESearchCase::CaseSensitive);
			Resolved.ReplaceInline(TEXT("/Game/VFXLab/Hero1Axe/Shared"), TEXT("/Game/VFX/Hero1/Axe/Shared"), ESearchCase::CaseSensitive);
		}
		return Resolved;
	}

	// Pierce reuses the shared Hero 1 AOE slash-layer materials. The carrier geometry
	// (vertical blade plane) and runtime PathAnchored scaling make Pierce distinct from
	// the AOE radial crescent; the material vocabulary is intentionally shared.
	struct FT66PierceLayerConfig
	{
		const TCHAR* EmitterName;
		const TCHAR* MaterialPath;
		float LifetimeSeconds;
		const TCHAR* SpawnScaleDefault;
		const TCHAR* ParticleColorDefault;
		FVector RendererScale;
	};

	const FT66PierceLayerConfig T66Hero1AxePierceLayerConfigs[] =
	{
		{
			TEXT("Emitter_AxePierceSlash_Bright"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Bright.M_Hero1AxeAOE_Slash_Bright"),
			0.24f,
			TEXT("0.94,0.96,0.96"),
			TEXT("(R=0.16,G=0.48,B=1.0,A=1.0)"),
			FVector(1.0f, 1.0f, 1.0f)
		},
		{
			TEXT("Emitter_AxePierceSlash_Body"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Body.M_Hero1AxeAOE_Slash_Body"),
			0.30f,
			TEXT("1.0,1.0,1.0"),
			TEXT("(R=1.0,G=0.04,B=0.055,A=1.0)"),
			FVector(1.0f, 1.0f, 1.0f)
		},
		{
			TEXT("Emitter_AxePierceSlash_Dark"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Dark.M_Hero1AxeAOE_Slash_Dark"),
			0.34f,
			TEXT("1.05,1.06,1.06"),
			TEXT("(R=0.018,G=0.045,B=0.20,A=0.58)"),
			FVector(1.0f, 1.0f, 1.0f)
		}
	};

	float T66GetPierceDevSlowFactor()
	{
		float DevSlowFactor = 1.0f;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxePierceDevSlowFactor="), DevSlowFactor))
		{
			return FMath::Clamp(DevSlowFactor, 1.0f, 20.0f);
		}
		if (FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxePierceDevSlow")))
		{
			return 6.0f;
		}
		return 1.0f;
	}

	float T66ScalePierceLifetimeForDevSlow(const float LifetimeSeconds, const float DevSlowFactor)
	{
		return LifetimeSeconds * FMath::Max(1.0f, DevSlowFactor);
	}

	const FT66PierceLayerConfig* T66FindPierceLayerConfig(const FString& EmitterName)
	{
		for (const FT66PierceLayerConfig& Config : T66Hero1AxePierceLayerConfigs)
		{
			if (EmitterName == Config.EmitterName)
			{
				return &Config;
			}
		}
		return nullptr;
	}

	bool T66SavePierceAsset(UObject* Asset)
	{
		if (!Asset)
		{
			return false;
		}

		UPackage* Package = Asset->GetPackage();
		if (!Package)
		{
			return false;
		}

		Package->MarkPackageDirty();

		const FString PackageName = Package->GetName();
		const FString Filename = FPackageName::LongPackageNameToFilename(
			PackageName,
			FPackageName::GetAssetPackageExtension());

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(Package, Asset, *Filename, SaveArgs);
	}

#if WITH_EDITOR
	UNiagaraNodeFunctionCall* T66AddPierceNiagaraModule(
		const TCHAR* AssetPath,
		UNiagaraNodeOutput& TargetOutputNode)
	{
		UNiagaraScript* ModuleScript = Cast<UNiagaraScript>(FSoftObjectPath(AssetPath).TryLoad());
		if (!ModuleScript)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Missing Niagara module %s"), AssetPath);
			return nullptr;
		}

		return FNiagaraStackGraphUtilities::AddScriptModuleToStack(ModuleScript, TargetOutputNode);
	}

	FName T66MakePierceModuleInputParameterName(const FName InputName)
	{
		return FNiagaraParameterHandle::CreateModuleParameterHandle(InputName).GetParameterHandleString();
	}

	bool T66FindPierceModuleInput(
		const FVersionedNiagaraEmitter& VersionedEmitter,
		const UNiagaraNodeOutput& OutputNode,
		UNiagaraNodeFunctionCall& TargetFunctionCallNode,
		const FName InputName,
		FNiagaraVariable& OutInputVariable)
	{
		TArray<FNiagaraVariable> InputVariables;
		TSet<FNiagaraVariable> HiddenVariables;
		const FCompileConstantResolver ConstantResolver(VersionedEmitter, OutputNode.GetUsage());
		FNiagaraStackGraphUtilities::GetStackFunctionInputs(
			TargetFunctionCallNode,
			InputVariables,
			HiddenVariables,
			ConstantResolver,
			FNiagaraStackGraphUtilities::ENiagaraGetStackFunctionInputPinsOptions::AllInputs,
			false);

		const FName ModuleInputName = T66MakePierceModuleInputParameterName(InputName);
		for (const FNiagaraVariable& InputVariable : InputVariables)
		{
			if (InputVariable.GetName() == ModuleInputName || InputVariable.GetName() == InputName)
			{
				OutInputVariable = InputVariable;
				return true;
			}
		}

		UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Missing module input %s."), *InputName.ToString());
		return false;
	}

	bool T66LinkPierceModuleInputToParticleParameter(
		UNiagaraSystem& OwningSystem,
		const FVersionedNiagaraEmitter& VersionedEmitter,
		const UNiagaraNodeOutput& OutputNode,
		UNiagaraNodeFunctionCall& TargetFunctionCallNode,
		const FName InputName,
		const FNiagaraVariableBase& LinkedParticleParameter)
	{
		FNiagaraVariable InputVariable;
		if (!T66FindPierceModuleInput(VersionedEmitter, OutputNode, TargetFunctionCallNode, InputName, InputVariable))
		{
			return false;
		}

		FNiagaraParameterHandle InputHandle = FNiagaraParameterHandle::CreateModuleParameterHandle(InputName);
		FNiagaraParameterHandle AliasedInputHandle =
			FNiagaraParameterHandle::CreateAliasedModuleParameterHandle(InputHandle, &TargetFunctionCallNode);
		UEdGraphPin& OverridePin = FNiagaraStackGraphUtilities::GetOrCreateStackFunctionInputOverridePin(
			TargetFunctionCallNode,
			AliasedInputHandle,
			InputVariable.GetType(),
			FGuid(),
			FGuid());

		TSet<FNiagaraVariableBase> KnownParameters;
		KnownParameters.Add(LinkedParticleParameter);
		FNiagaraStackGraphUtilities::SetLinkedParameterValueForFunctionInput(
			OverridePin,
			LinkedParticleParameter,
			KnownParameters,
			ENiagaraDefaultMode::Custom);
		return true;
	}

	template<typename ValueType>
	void T66SetPierceRapidIterationParameter(
		const FString& UniqueEmitterName,
		UNiagaraScript& TargetScript,
		UNiagaraNodeFunctionCall& TargetFunctionCallNode,
		const FName InputName,
		const FNiagaraTypeDefinition& InputType,
		const ValueType Value)
	{
		static_assert(!TIsUECoreVariant<ValueType, double>::Value, "Double core variant. Must be float type.");
		FNiagaraParameterHandle InputHandle = FNiagaraParameterHandle::CreateModuleParameterHandle(InputName);
		FNiagaraParameterHandle AliasedInputHandle =
			FNiagaraParameterHandle::CreateAliasedModuleParameterHandle(InputHandle, &TargetFunctionCallNode);
		FNiagaraVariable InputVariable(InputType, AliasedInputHandle.GetParameterHandleString());
		const TCHAR* EmitterNameForRapidIteration =
			(TargetScript.GetUsage() == ENiagaraScriptUsage::SystemSpawnScript ||
				TargetScript.GetUsage() == ENiagaraScriptUsage::SystemUpdateScript)
			? nullptr
			: *UniqueEmitterName;
		FNiagaraVariable RapidIterationParameter = FNiagaraUtilities::ConvertVariableToRapidIterationConstantName(
			InputVariable,
			EmitterNameForRapidIteration,
			TargetScript.GetUsage());
		RapidIterationParameter.SetValue(Value);

		constexpr bool bAddParameterIfMissing = true;
		TargetScript.RapidIterationParameters.SetParameterData(
			RapidIterationParameter.GetData(),
			RapidIterationParameter,
			bAddParameterIfMissing);
	}

	bool T66AddPierceLayerEmitter(UNiagaraSystem& SlashSystem, const FT66PierceLayerConfig& Config, const float DevSlowFactor)
	{
		const float ScaledLifetimeSeconds = T66ScalePierceLifetimeForDevSlow(Config.LifetimeSeconds, DevSlowFactor);
		UNiagaraEmitter* SlashEmitter = NewObject<UNiagaraEmitter>(
			GetTransientPackage(),
			Config.EmitterName,
			RF_Transactional);
		if (!SlashEmitter)
		{
			return false;
		}
		SlashEmitter->SetUniqueEmitterName(Config.EmitterName);
		UNiagaraEmitterFactoryNew::InitializeEmitter(SlashEmitter, false);
		SlashEmitter->SetUniqueEmitterName(Config.EmitterName);
		FVersionedNiagaraEmitterData* EmitterData = SlashEmitter->GetLatestEmitterData();
		if (!EmitterData || !EmitterData->GraphSource)
		{
			return false;
		}

		EmitterData->SimTarget = ENiagaraSimTarget::CPUSim;
		EmitterData->bLocalSpace = true;
		EmitterData->bDeterminism = true;
		EmitterData->CalculateBoundsMode = ENiagaraEmitterCalculateBoundMode::Fixed;
		// Normalized local-space bounds for the unit blade (X[0,1], Y/Z lens). The runtime
		// component scale (LineLength / TubeRadius) maps this to the authoritative lane.
		EmitterData->FixedBounds = FBox(FVector(-0.5f, -2.0f, -2.5f), FVector(1.5f, 2.0f, 2.5f));

		UNiagaraNodeOutput* EmitterUpdateOutputNode =
			FNiagaraEditorUtilities::GetScriptOutputNode(*EmitterData->EmitterUpdateScriptProps.Script);
		UNiagaraNodeOutput* ParticleSpawnOutputNode =
			FNiagaraEditorUtilities::GetScriptOutputNode(*EmitterData->SpawnScriptProps.Script);
		UNiagaraNodeOutput* ParticleUpdateOutputNode =
			FNiagaraEditorUtilities::GetScriptOutputNode(*EmitterData->UpdateScriptProps.Script);
		if (!EmitterUpdateOutputNode || !ParticleSpawnOutputNode || !ParticleUpdateOutputNode)
		{
			return false;
		}

		T66AddPierceNiagaraModule(TEXT("/Niagara/Modules/Emitter/EmitterState.EmitterState"), *EmitterUpdateOutputNode);
		UNiagaraNodeFunctionCall* SpawnBurstNode = T66AddPierceNiagaraModule(
			TEXT("/Niagara/Modules/Emitter/SpawnBurst_Instantaneous.SpawnBurst_Instantaneous"),
			*EmitterUpdateOutputNode);
		if (SpawnBurstNode && EmitterData->EmitterUpdateScriptProps.Script)
		{
			T66SetPierceRapidIterationParameter<int32>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Count"),
				FNiagaraTypeDefinition::GetIntDef(),
				1);
			T66SetPierceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Probability"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetPierceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Time"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
		}
		T66AddPierceNiagaraModule(
			TEXT("/Niagara/Modules/Spawn/Location/SystemLocation.SystemLocation"),
			*ParticleSpawnOutputNode);
		UNiagaraNodeFunctionCall* InitialMeshRotationNode = T66AddPierceNiagaraModule(
			TEXT("/Niagara/Modules/Spawn/Orientation/InitialMeshRotation.InitialMeshRotation"),
			*ParticleSpawnOutputNode);
		if (InitialMeshRotationNode && EmitterData->SpawnScriptProps.Script)
		{
			T66SetPierceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->SpawnScriptProps.Script,
				*InitialMeshRotationNode,
				TEXT("Pitch"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
			T66SetPierceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->SpawnScriptProps.Script,
				*InitialMeshRotationNode,
				TEXT("Yaw"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
			T66SetPierceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->SpawnScriptProps.Script,
				*InitialMeshRotationNode,
				TEXT("Roll"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
		}

		const TArray<FNiagaraVariable> ParticleSpawnVars =
		{
			SYS_PARAM_PARTICLES_LIFETIME,
			SYS_PARAM_PARTICLES_SCALE,
			SYS_PARAM_PARTICLES_COLOR
		};
		const TArray<FString> ParticleSpawnDefaults =
		{
			FString::Printf(TEXT("%.3f"), ScaledLifetimeSeconds),
			Config.SpawnScaleDefault,
			Config.ParticleColorDefault
		};
		FNiagaraStackGraphUtilities::AddParameterModuleToStack(
			ParticleSpawnVars,
			*ParticleSpawnOutputNode,
			INDEX_NONE,
			ParticleSpawnDefaults);

		T66AddPierceNiagaraModule(
			TEXT("/Niagara/Modules/Update/Lifetime/UpdateAge.UpdateAge"),
			*ParticleUpdateOutputNode);
		UNiagaraNodeFunctionCall* DynamicMaterialNode = T66AddPierceNiagaraModule(
			TEXT("/Niagara/Modules/Update/Material/DynamicMaterialParameters.DynamicMaterialParameters"),
			*ParticleUpdateOutputNode);
		if (DynamicMaterialNode && EmitterData->UpdateScriptProps.Script)
		{
			T66SetPierceRapidIterationParameter<FNiagaraBool>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Param0WriteEnabled"),
				FNiagaraTypeDefinition::GetBoolDef(),
				FNiagaraBool(true));
			// Drive the shared slash reveal/erosion material with normalized age so the
			// blade reveals forward along its U (lane) axis as the Pierce activation ages.
			if (!T66LinkPierceModuleInputToParticleParameter(
				SlashSystem,
				FVersionedNiagaraEmitter(SlashEmitter, EmitterData->Version.VersionGuid),
				*ParticleUpdateOutputNode,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 1"),
				SYS_PARAM_PARTICLES_NORMALIZED_AGE))
			{
				return false;
			}
			T66SetPierceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 2"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetPierceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 3"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetPierceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 4"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
		}

		UE_LOG(
			LogTemp,
			Display,
			TEXT("[Hero1AxePierceVFX] Pierce layer emitter=%s lifetime=%.2f baseLifetime=%.2f DevSlowFactor=%.2f"),
			Config.EmitterName,
			ScaledLifetimeSeconds,
			Config.LifetimeSeconds,
			DevSlowFactor);

		FNiagaraEditorUtilities::AddEmitterToSystem(SlashSystem, *SlashEmitter, FGuid(), true);
		return true;
	}

	UNiagaraSystem* T66CreatePierceNiagaraSystem(const float DevSlowFactor)
	{
		const FString PierceNiagaraPackagePath = T66ResolveHero1AxePiercePath(T66Hero1AxePierceNiagaraPackagePath);
		UPackage* Package = CreatePackage(*PierceNiagaraPackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		if (UNiagaraSystem* ExistingSystem = FindObject<UNiagaraSystem>(Package, T66Hero1AxePierceNiagaraObjectName))
		{
			ExistingSystem->ClearFlags(RF_Public | RF_Standalone);
			ExistingSystem->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
		}

		UNiagaraSystem* SlashSystem = NewObject<UNiagaraSystem>(
			Package,
			FName(T66Hero1AxePierceNiagaraObjectName),
			RF_Public | RF_Standalone | RF_Transactional);
		if (!SlashSystem)
		{
			return nullptr;
		}

		UNiagaraSystemFactoryNew::InitializeSystem(SlashSystem, true);

		for (const FT66PierceLayerConfig& Config : T66Hero1AxePierceLayerConfigs)
		{
			if (!T66AddPierceLayerEmitter(*SlashSystem, Config, DevSlowFactor))
			{
				UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Failed to add Pierce layer emitter %s"), Config.EmitterName);
				return nullptr;
			}
		}
		FAssetRegistryModule::AssetCreated(SlashSystem);
		return SlashSystem;
	}
#endif

	FVertexInstanceID T66CreatePierceVertexInstance(
		FMeshDescription& MeshDescription,
		TVertexInstanceAttributesRef<FVector2f>& UVs,
		const FVertexID VertexID,
		const FVector2f UV)
	{
		const FVertexInstanceID VertexInstanceID = MeshDescription.CreateVertexInstance(VertexID);
		UVs[VertexInstanceID] = UV;
		return VertexInstanceID;
	}

	// Builds a unit-space vertical blade-plane lane mesh. Local +X is the forward lane
	// axis (UV U), local +Z is the vertical slash height (UV V), and the cross-section is
	// a thin lens in local Y so the blade reads as a 3D vertical slash from gameplay
	// angles instead of a flat billboard. The lane ends taper to points (a blade
	// silhouette), and the whole thing is intentionally NOT the AOE radial crescent.
	UStaticMesh* T66BuildPierceBladeMesh(UMaterialInterface* SlashMaterial)
	{
		const FString PierceMeshPackagePath = T66ResolveHero1AxePiercePath(T66Hero1AxePierceMeshPackagePath);
		UPackage* Package = CreatePackage(*PierceMeshPackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		UStaticMesh* BladeMesh = FindObject<UStaticMesh>(Package, T66Hero1AxePierceMeshObjectName);
		if (!BladeMesh)
		{
			BladeMesh = NewObject<UStaticMesh>(
				Package,
				T66Hero1AxePierceMeshObjectName,
				RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(BladeMesh);
		}
		if (!BladeMesh)
		{
			return nullptr;
		}

		BladeMesh->Modify();
		BladeMesh->bAllowCPUAccess = true;
		BladeMesh->GetStaticMaterials().Reset();
		BladeMesh->GetStaticMaterials().Add(FStaticMaterial(SlashMaterial, TEXT("PierceSlash")));
		BladeMesh->SetLightMapResolution(4);
		BladeMesh->SetLightingGuid();

		FMeshDescription MeshDescription;
		FStaticMeshAttributes Attributes(MeshDescription);
		Attributes.Register();

		TVertexAttributesRef<FVector3f> Positions = Attributes.GetVertexPositions();
		TVertexInstanceAttributesRef<FVector2f> UVs = Attributes.GetVertexInstanceUVs();
		UVs.SetNumChannels(1);
		TPolygonGroupAttributesRef<FName> MaterialSlotNames = Attributes.GetPolygonGroupMaterialSlotNames();

		const FPolygonGroupID PolygonGroupID = MeshDescription.CreatePolygonGroup();
		MaterialSlotNames[PolygonGroupID] = TEXT("PierceBladeProfile");

		constexpr int32 SegmentCount = 48;
		constexpr float Pi = UE_PI;
		// Vertical half-height and lens half-width profiles across the lane (peaking
		// mid-lane, tapering to points at both ends for a blade silhouette).
		constexpr float MaxHalfHeight = 1.30f;
		constexpr float MaxLensHalfWidth = 0.18f;

		struct FT66BladeStation
		{
			FVertexID Bottom;
			FVertexID Front;
			FVertexID Top;
			FVertexID Back;
		};

		TArray<FT66BladeStation> Stations;
		Stations.Reserve(SegmentCount + 1);

		for (int32 Index = 0; Index <= SegmentCount; ++Index)
		{
			const float T = static_cast<float>(Index) / static_cast<float>(SegmentCount);
			const float HeightEnvelope = FMath::Pow(FMath::Max(0.0f, FMath::Sin(T * Pi)), 0.65f);
			const float WidthEnvelope = FMath::Pow(FMath::Max(0.0f, FMath::Sin(T * Pi)), 0.5f);
			const float HalfHeight = MaxHalfHeight * HeightEnvelope;
			const float LensHalfWidth = MaxLensHalfWidth * WidthEnvelope;

			FT66BladeStation Station;
			Station.Bottom = MeshDescription.CreateVertex();
			Station.Front = MeshDescription.CreateVertex();
			Station.Top = MeshDescription.CreateVertex();
			Station.Back = MeshDescription.CreateVertex();
			Positions[Station.Bottom] = FVector3f(T, 0.0f, -HalfHeight);
			Positions[Station.Front] = FVector3f(T, LensHalfWidth, 0.0f);
			Positions[Station.Top] = FVector3f(T, 0.0f, HalfHeight);
			Positions[Station.Back] = FVector3f(T, -LensHalfWidth, 0.0f);
			Stations.Add(Station);
		}

		auto AddQuad = [&MeshDescription, &UVs, PolygonGroupID](
			const FVertexID A, const FVector2f UvA,
			const FVertexID B, const FVector2f UvB,
			const FVertexID C, const FVector2f UvC,
			const FVertexID D, const FVector2f UvD)
		{
			TArray<FVertexInstanceID> FirstTriangle;
			FirstTriangle.Reserve(3);
			FirstTriangle.Add(T66CreatePierceVertexInstance(MeshDescription, UVs, A, UvA));
			FirstTriangle.Add(T66CreatePierceVertexInstance(MeshDescription, UVs, B, UvB));
			FirstTriangle.Add(T66CreatePierceVertexInstance(MeshDescription, UVs, C, UvC));
			MeshDescription.CreatePolygon(PolygonGroupID, FirstTriangle);

			TArray<FVertexInstanceID> SecondTriangle;
			SecondTriangle.Reserve(3);
			SecondTriangle.Add(T66CreatePierceVertexInstance(MeshDescription, UVs, A, UvA));
			SecondTriangle.Add(T66CreatePierceVertexInstance(MeshDescription, UVs, C, UvC));
			SecondTriangle.Add(T66CreatePierceVertexInstance(MeshDescription, UVs, D, UvD));
			MeshDescription.CreatePolygon(PolygonGroupID, SecondTriangle);
		};

		for (int32 Index = 0; Index < SegmentCount; ++Index)
		{
			const float U0 = static_cast<float>(Index) / static_cast<float>(SegmentCount);
			const float U1 = static_cast<float>(Index + 1) / static_cast<float>(SegmentCount);
			const FT66BladeStation& S0 = Stations[Index];
			const FT66BladeStation& S1 = Stations[Index + 1];

			// V across the lens cross-section: bottom=0, front=0.5, top=1, back=0.5.
			AddQuad(
				S0.Bottom, FVector2f(U0, 0.0f), S1.Bottom, FVector2f(U1, 0.0f),
				S1.Front, FVector2f(U1, 0.5f), S0.Front, FVector2f(U0, 0.5f));
			AddQuad(
				S0.Front, FVector2f(U0, 0.5f), S1.Front, FVector2f(U1, 0.5f),
				S1.Top, FVector2f(U1, 1.0f), S0.Top, FVector2f(U0, 1.0f));
			AddQuad(
				S0.Top, FVector2f(U0, 1.0f), S1.Top, FVector2f(U1, 1.0f),
				S1.Back, FVector2f(U1, 0.5f), S0.Back, FVector2f(U0, 0.5f));
			AddQuad(
				S0.Back, FVector2f(U0, 0.5f), S1.Back, FVector2f(U1, 0.5f),
				S1.Bottom, FVector2f(U1, 0.0f), S0.Bottom, FVector2f(U0, 0.0f));
		}

		FStaticMeshOperations::ComputeTriangleTangentsAndNormals(MeshDescription);
		FStaticMeshOperations::ComputeTangentsAndNormals(
			MeshDescription,
			EComputeNTBsFlags::Normals | EComputeNTBsFlags::Tangents);

		UStaticMesh::FBuildMeshDescriptionsParams BuildParams;
		BuildParams.bBuildSimpleCollision = false;
		BuildParams.bAllowCpuAccess = true;
		BuildParams.bFastBuild = true;
		BuildParams.bCommitMeshDescription = true;
		if (!BladeMesh->BuildFromMeshDescriptions({ &MeshDescription }, BuildParams))
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Failed to build Pierce blade mesh buffers."));
			return nullptr;
		}

		const FStaticMeshRenderData* RenderData = BladeMesh->GetRenderData();
		if (!RenderData || !RenderData->LODResources.IsValidIndex(0))
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Pierce blade mesh has no render data after build."));
			return nullptr;
		}

		const FStaticMeshLODResources& LODResources = RenderData->LODResources[0];
		const int32 VertexCount = static_cast<int32>(LODResources.GetNumVertices());
		const int32 IndexCount = LODResources.IndexBuffer.GetNumIndices();
		const int32 SectionCount = LODResources.Sections.Num();
		UE_LOG(LogTemp, Display, TEXT("[Hero1AxePierceVFX] Pierce blade mesh render data: vertices=%d indices=%d sections=%d"),
			VertexCount,
			IndexCount,
			SectionCount);
		if (VertexCount <= 0 || IndexCount <= 0 || SectionCount <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Pierce blade mesh built with no drawable render buffers."));
			return nullptr;
		}

		BladeMesh->MarkPackageDirty();
		return BladeMesh;
	}
}

UT66Hero1AxePierceVFXCommandlet::UT66Hero1AxePierceVFXCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UT66Hero1AxePierceVFXCommandlet::Main(const FString& Params)
{
	const bool bProductionPaths = T66UseHero1AxePierceProductionPaths();
	const float DevSlowFactor = T66GetPierceDevSlowFactor();
	const FString PierceNiagaraPath = T66ResolveHero1AxePiercePath(T66Hero1AxePierceNiagaraPath);
	UE_LOG(LogTemp, Display, TEXT("[Hero1AxePierceVFX] Building vertical blade-plane carrier and PathAnchored Niagara mesh renderer. ProductionPaths=%s DevSlowFactor=%.2f"), bProductionPaths ? TEXT("true") : TEXT("false"), DevSlowFactor);

#if WITH_EDITOR
	UNiagaraSystem* SlashSystem = T66CreatePierceNiagaraSystem(DevSlowFactor);
#else
	UNiagaraSystem* SlashSystem = LoadObject<UNiagaraSystem>(nullptr, *PierceNiagaraPath);
#endif
	if (!SlashSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Missing or failed to create Niagara system %s"), *PierceNiagaraPath);
		return 1;
	}

	TMap<FString, UMaterialInterface*> PierceLayerMaterials;
	UMaterialInterface* BodyMaterial = nullptr;
	for (const FT66PierceLayerConfig& Config : T66Hero1AxePierceLayerConfigs)
	{
		const FString MaterialPath = T66ResolveHero1AxePiercePath(Config.MaterialPath);
		UMaterialInterface* LayerMaterial = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
		if (!LayerMaterial)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Missing shared slash layer material %s"), *MaterialPath);
			return 1;
		}
		PierceLayerMaterials.Add(Config.EmitterName, LayerMaterial);
		if (FCString::Strcmp(Config.EmitterName, TEXT("Emitter_AxePierceSlash_Body")) == 0)
		{
			BodyMaterial = LayerMaterial;
		}
		if (UMaterial* ConcreteMaterial = Cast<UMaterial>(LayerMaterial))
		{
			ConcreteMaterial->Modify();
			ConcreteMaterial->bUsedWithNiagaraMeshParticles = true;
#if WITH_EDITOR
			ConcreteMaterial->PostEditChange();
#endif
			T66SavePierceAsset(ConcreteMaterial);
		}
	}
	if (!BodyMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Failed to identify Pierce body layer material."));
		return 1;
	}

	UStaticMesh* BladeMesh = T66BuildPierceBladeMesh(BodyMaterial);
	if (!BladeMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Failed to create Pierce blade mesh asset."));
		return 1;
	}
	if (!T66SavePierceAsset(BladeMesh))
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Failed to save %s"), *BladeMesh->GetPathName());
		return 1;
	}

	int32 EmitterCount = 0;
	int32 BoundMeshRendererCount = 0;
	SlashSystem->Modify();
	SlashSystem->SetFixedBounds(FBox(FVector(-0.5f, -2.0f, -2.5f), FVector(1.5f, 2.0f, 2.5f)));

	for (FNiagaraEmitterHandle& EmitterHandle : SlashSystem->GetEmitterHandles())
	{
		FVersionedNiagaraEmitterData* EmitterData = EmitterHandle.GetEmitterData();
		const FVersionedNiagaraEmitter VersionedEmitter = EmitterHandle.GetInstance();
		UNiagaraEmitter* Emitter = VersionedEmitter.Emitter;
		if (!EmitterData || !Emitter)
		{
			continue;
		}

		EmitterCount += 1;
		const FString EmitterName = EmitterHandle.GetName().ToString();
		const FT66PierceLayerConfig* Config = T66FindPierceLayerConfig(EmitterName);

		Emitter->Modify();
		EmitterData->bLocalSpace = true;
		EmitterData->CalculateBoundsMode = ENiagaraEmitterCalculateBoundMode::Fixed;
		EmitterData->FixedBounds = FBox(FVector(-0.5f, -2.0f, -2.5f), FVector(1.5f, 2.0f, 2.5f));

		TArray<UNiagaraRendererProperties*> ExistingRenderers = EmitterData->GetRenderers();
		for (UNiagaraRendererProperties* Renderer : ExistingRenderers)
		{
			if (Renderer)
			{
				Emitter->RemoveRenderer(Renderer, VersionedEmitter.Version);
			}
		}

		if (!Config)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Unexpected emitter name %s"), *EmitterName);
			return 1;
		}

		UMaterialInterface* const* LayerMaterial = PierceLayerMaterials.Find(EmitterName);
		if (!LayerMaterial || !(*LayerMaterial))
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Missing material binding for emitter %s"), *EmitterName);
			return 1;
		}

		UNiagaraMeshRendererProperties* MeshRenderer =
			NewObject<UNiagaraMeshRendererProperties>(Emitter, NAME_None, RF_Transactional);
		if (!MeshRenderer)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Failed to allocate mesh renderer for emitter %s."), *EmitterName);
			return 1;
		}

		MeshRenderer->Modify();
		FNiagaraMeshRendererMeshProperties MeshProperties;
		MeshProperties.Mesh = BladeMesh;
		MeshProperties.Scale = Config->RendererScale;
		MeshProperties.Rotation = FRotator::ZeroRotator;
		MeshProperties.PivotOffset = FVector::ZeroVector;
		MeshRenderer->Meshes.Reset();
		MeshRenderer->Meshes.Add(MeshProperties);
		MeshRenderer->SourceMode = ENiagaraRendererSourceDataMode::Particles;
		MeshRenderer->FacingMode = ENiagaraMeshFacingMode::Default;
		MeshRenderer->bOverrideMaterials = true;
		MeshRenderer->OverrideMaterials.Reset();
		FNiagaraMeshMaterialOverride SlashMaterialOverride;
		SlashMaterialOverride.ExplicitMat = *LayerMaterial;
		MeshRenderer->OverrideMaterials.Add(SlashMaterialOverride);
		MeshRenderer->bCastShadows = false;
		MeshRenderer->bEnableCameraDistanceCulling = false;
		MeshRenderer->bEnableFrustumCulling = false;
		MeshRenderer->SortMode = ENiagaraSortMode::None;
		MeshRenderer->SetIsEnabled(true);

		Emitter->AddRenderer(MeshRenderer, VersionedEmitter.Version);
		BoundMeshRendererCount += 1;
		UE_LOG(
			LogTemp,
			Display,
			TEXT("[Hero1AxePierceVFX] Bound Pierce layer emitter=%s rendererScale=%s material=%s"),
			Config->EmitterName,
			*Config->RendererScale.ToString(),
			*(*LayerMaterial)->GetPathName());
	}

	if (EmitterCount <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Niagara system has no editable emitters."));
		return 1;
	}
	if (BoundMeshRendererCount != UE_ARRAY_COUNT(T66Hero1AxePierceLayerConfigs))
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Bound %d of %d Pierce layer emitters to mesh renderers."),
			BoundMeshRendererCount,
			UE_ARRAY_COUNT(T66Hero1AxePierceLayerConfigs));
		return 1;
	}

#if WITH_EDITOR
	SlashSystem->PostEditChange();
	SlashSystem->RequestCompile(true);
#endif

	if (!T66SavePierceAsset(SlashSystem))
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxePierceVFX] Failed to save %s"), *SlashSystem->GetPathName());
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("[Hero1AxePierceVFX] Bound %d Pierce blade mesh renderer(s) to %s. ProductionPaths=%s DevSlowFactor=%.2f"),
		BoundMeshRendererCount,
		*BladeMesh->GetPathName(),
		bProductionPaths ? TEXT("true") : TEXT("false"),
		DevSlowFactor);
	return 0;
}
