// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66Hero1AxeBounceVFXCommandlet.h"

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
	const TCHAR* T66Hero1AxeBounceNiagaraPath =
		TEXT("/Game/VFXLab/Hero1Axe/Bounce/NS_Hero1AxeBounce_MeshSlash.NS_Hero1AxeBounce_MeshSlash");
	const TCHAR* T66Hero1AxeBounceNiagaraPackagePath =
		TEXT("/Game/VFXLab/Hero1Axe/Bounce/NS_Hero1AxeBounce_MeshSlash");
	const TCHAR* T66Hero1AxeBounceNiagaraObjectName =
		TEXT("NS_Hero1AxeBounce_MeshSlash");
	const TCHAR* T66Hero1AxeBounceMeshPackagePath =
		TEXT("/Game/VFXLab/Hero1Axe/Bounce/SM_Hero1AxeBounce_HorizontalSlash");
	const TCHAR* T66Hero1AxeBounceMeshObjectName =
		TEXT("SM_Hero1AxeBounce_HorizontalSlash");

	// Local-space bounds (Unreal centimeters) for the centered horizontal slash
	// carrier: length along Y, thin vertical lens in Z, shallow forward crescent bow
	// in X. Sized to contain the cm-authored slash mesh below (half-length 80 along Y,
	// matching the Hero1Axe_Bounce_Base BaseVisualRadius=80 binding convention shared
	// with the AOE radius=132 and Pierce half-length=150 carriers).
	const FBox T66Hero1AxeBounceLocalBounds(FVector(-38.0f, -100.0f, -50.0f), FVector(38.0f, 100.0f, 50.0f));

	bool T66UseHero1AxeBounceProductionPaths()
	{
		return FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeBounceProduction"));
	}

	FString T66ResolveHero1AxeBouncePath(const TCHAR* Path)
	{
		FString Resolved(Path);
		if (T66UseHero1AxeBounceProductionPaths())
		{
			Resolved.ReplaceInline(TEXT("/Game/VFXLab/Hero1Axe/Bounce"), TEXT("/Game/VFX/Hero1/Axe/Bounce"), ESearchCase::CaseSensitive);
			Resolved.ReplaceInline(TEXT("/Game/VFXLab/Hero1Axe/Shared"), TEXT("/Game/VFX/Hero1/Axe/Shared"), ESearchCase::CaseSensitive);
		}
		return Resolved;
	}

	// Bounce reuses the shared Hero 1 AOE slash-layer materials. The carrier geometry
	// (compact horizontal slash) and ImpactAnchored per-link placement make Bounce
	// distinct from the AOE radial crescent and the Pierce forward vertical lane; the
	// red/blue/white material vocabulary is intentionally shared.
	struct FT66BounceLayerConfig
	{
		const TCHAR* EmitterName;
		const TCHAR* MaterialPath;
		float LifetimeSeconds;
		const TCHAR* SpawnScaleDefault;
		const TCHAR* ParticleColorDefault;
		FVector RendererScale;
	};

	const FT66BounceLayerConfig T66Hero1AxeBounceLayerConfigs[] =
	{
		{
			TEXT("Emitter_AxeBounceSlash_Bright"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Bright.M_Hero1AxeAOE_Slash_Bright"),
			0.52f,
			TEXT("0.94,0.96,0.96"),
			TEXT("(R=0.16,G=0.48,B=1.0,A=1.0)"),
			FVector(1.0f, 1.0f, 1.0f)
		},
		{
			TEXT("Emitter_AxeBounceSlash_Body"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Body.M_Hero1AxeAOE_Slash_Body"),
			0.60f,
			TEXT("1.0,1.0,1.0"),
			TEXT("(R=1.0,G=0.04,B=0.055,A=1.0)"),
			FVector(1.0f, 1.0f, 1.0f)
		},
		{
			TEXT("Emitter_AxeBounceSlash_Dark"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Dark.M_Hero1AxeAOE_Slash_Dark"),
			0.68f,
			TEXT("1.05,1.06,1.06"),
			TEXT("(R=0.018,G=0.045,B=0.20,A=0.58)"),
			FVector(1.0f, 1.0f, 1.0f)
		}
	};

	float T66GetBounceDevSlowFactor()
	{
		float DevSlowFactor = 1.0f;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeBounceDevSlowFactor="), DevSlowFactor))
		{
			return FMath::Clamp(DevSlowFactor, 1.0f, 20.0f);
		}
		if (FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeBounceDevSlow")))
		{
			return 6.0f;
		}
		return 1.0f;
	}

	float T66ScaleBounceLifetimeForDevSlow(const float LifetimeSeconds, const float DevSlowFactor)
	{
		return LifetimeSeconds * FMath::Max(1.0f, DevSlowFactor);
	}

	const FT66BounceLayerConfig* T66FindBounceLayerConfig(const FString& EmitterName)
	{
		for (const FT66BounceLayerConfig& Config : T66Hero1AxeBounceLayerConfigs)
		{
			if (EmitterName == Config.EmitterName)
			{
				return &Config;
			}
		}
		return nullptr;
	}

	bool T66SaveBounceAsset(UObject* Asset)
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
	UNiagaraNodeFunctionCall* T66AddBounceNiagaraModule(
		const TCHAR* AssetPath,
		UNiagaraNodeOutput& TargetOutputNode)
	{
		UNiagaraScript* ModuleScript = Cast<UNiagaraScript>(FSoftObjectPath(AssetPath).TryLoad());
		if (!ModuleScript)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Missing Niagara module %s"), AssetPath);
			return nullptr;
		}

		return FNiagaraStackGraphUtilities::AddScriptModuleToStack(ModuleScript, TargetOutputNode);
	}

	FName T66MakeBounceModuleInputParameterName(const FName InputName)
	{
		return FNiagaraParameterHandle::CreateModuleParameterHandle(InputName).GetParameterHandleString();
	}

	bool T66FindBounceModuleInput(
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

		const FName ModuleInputName = T66MakeBounceModuleInputParameterName(InputName);
		for (const FNiagaraVariable& InputVariable : InputVariables)
		{
			if (InputVariable.GetName() == ModuleInputName || InputVariable.GetName() == InputName)
			{
				OutInputVariable = InputVariable;
				return true;
			}
		}

		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Missing module input %s."), *InputName.ToString());
		return false;
	}

	bool T66LinkBounceModuleInputToParticleParameter(
		UNiagaraSystem& OwningSystem,
		const FVersionedNiagaraEmitter& VersionedEmitter,
		const UNiagaraNodeOutput& OutputNode,
		UNiagaraNodeFunctionCall& TargetFunctionCallNode,
		const FName InputName,
		const FNiagaraVariableBase& LinkedParticleParameter)
	{
		FNiagaraVariable InputVariable;
		if (!T66FindBounceModuleInput(VersionedEmitter, OutputNode, TargetFunctionCallNode, InputName, InputVariable))
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
	void T66SetBounceRapidIterationParameter(
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

	bool T66AddBounceLayerEmitter(UNiagaraSystem& SlashSystem, const FT66BounceLayerConfig& Config, const float DevSlowFactor)
	{
		const float ScaledLifetimeSeconds = T66ScaleBounceLifetimeForDevSlow(Config.LifetimeSeconds, DevSlowFactor);
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
		// Local-space bounds for the centered horizontal slash. ImpactAnchored runtime
		// placement positions this carrier at each authoritative Bounce impact point.
		EmitterData->FixedBounds = T66Hero1AxeBounceLocalBounds;

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

		T66AddBounceNiagaraModule(TEXT("/Niagara/Modules/Emitter/EmitterState.EmitterState"), *EmitterUpdateOutputNode);
		UNiagaraNodeFunctionCall* SpawnBurstNode = T66AddBounceNiagaraModule(
			TEXT("/Niagara/Modules/Emitter/SpawnBurst_Instantaneous.SpawnBurst_Instantaneous"),
			*EmitterUpdateOutputNode);
		if (SpawnBurstNode && EmitterData->EmitterUpdateScriptProps.Script)
		{
			T66SetBounceRapidIterationParameter<int32>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Count"),
				FNiagaraTypeDefinition::GetIntDef(),
				1);
			T66SetBounceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Probability"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetBounceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Time"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
		}
		// Keep the mesh at local zero. The Bounce carrier is attached to a visual
		// projectile that moves between authoritative chain impact points; adding
		// SystemLocation in a local-space emitter bakes an owner/world offset into the
		// particle and makes the slash appear pinned near the hero instead of following
		// the moving component.
		UNiagaraNodeFunctionCall* InitialMeshRotationNode = T66AddBounceNiagaraModule(
			TEXT("/Niagara/Modules/Spawn/Orientation/InitialMeshRotation.InitialMeshRotation"),
			*ParticleSpawnOutputNode);
		if (InitialMeshRotationNode && EmitterData->SpawnScriptProps.Script)
		{
			T66SetBounceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->SpawnScriptProps.Script,
				*InitialMeshRotationNode,
				TEXT("Pitch"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
			T66SetBounceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->SpawnScriptProps.Script,
				*InitialMeshRotationNode,
				TEXT("Yaw"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
			T66SetBounceRapidIterationParameter<float>(
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

		T66AddBounceNiagaraModule(
			TEXT("/Niagara/Modules/Update/Lifetime/UpdateAge.UpdateAge"),
			*ParticleUpdateOutputNode);
		UNiagaraNodeFunctionCall* DynamicMaterialNode = T66AddBounceNiagaraModule(
			TEXT("/Niagara/Modules/Update/Material/DynamicMaterialParameters.DynamicMaterialParameters"),
			*ParticleUpdateOutputNode);
		if (DynamicMaterialNode && EmitterData->UpdateScriptProps.Script)
		{
			T66SetBounceRapidIterationParameter<FNiagaraBool>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Param0WriteEnabled"),
				FNiagaraTypeDefinition::GetBoolDef(),
				FNiagaraBool(true));
			// Bounce is a moving projectile carrier, unlike the stationary AOE/Pierce
			// impact reveals. Hold the shared slash material in its active mid-life band
			// while the actor travels; forcing it to age 1.0 starts the carrier in its
			// dissipated/end-mask state and leaves only tiny late fragments in capture.
			T66SetBounceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 1"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.45f);
			T66SetBounceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 2"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetBounceRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 3"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetBounceRapidIterationParameter<float>(
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
			TEXT("[Hero1AxeBounceVFX] Bounce layer emitter=%s lifetime=%.2f baseLifetime=%.2f DevSlowFactor=%.2f"),
			Config.EmitterName,
			ScaledLifetimeSeconds,
			Config.LifetimeSeconds,
			DevSlowFactor);

		FNiagaraEditorUtilities::AddEmitterToSystem(SlashSystem, *SlashEmitter, FGuid(), true);
		return true;
	}

	UNiagaraSystem* T66CreateBounceNiagaraSystem(const float DevSlowFactor)
	{
		const FString BounceNiagaraPackagePath = T66ResolveHero1AxeBouncePath(T66Hero1AxeBounceNiagaraPackagePath);
		UPackage* Package = CreatePackage(*BounceNiagaraPackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		if (UNiagaraSystem* ExistingSystem = FindObject<UNiagaraSystem>(Package, T66Hero1AxeBounceNiagaraObjectName))
		{
			ExistingSystem->ClearFlags(RF_Public | RF_Standalone);
			ExistingSystem->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
		}

		UNiagaraSystem* SlashSystem = NewObject<UNiagaraSystem>(
			Package,
			FName(T66Hero1AxeBounceNiagaraObjectName),
			RF_Public | RF_Standalone | RF_Transactional);
		if (!SlashSystem)
		{
			return nullptr;
		}

		UNiagaraSystemFactoryNew::InitializeSystem(SlashSystem, true);

		for (const FT66BounceLayerConfig& Config : T66Hero1AxeBounceLayerConfigs)
		{
			if (!T66AddBounceLayerEmitter(*SlashSystem, Config, DevSlowFactor))
			{
				UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Failed to add Bounce layer emitter %s"), Config.EmitterName);
				return nullptr;
			}
		}
		FAssetRegistryModule::AssetCreated(SlashSystem);
		return SlashSystem;
	}
#endif

	FVertexInstanceID T66CreateBounceVertexInstance(
		FMeshDescription& MeshDescription,
		TVertexInstanceAttributesRef<FVector2f>& UVs,
		const FVertexID VertexID,
		const FVector2f UV)
	{
		const FVertexInstanceID VertexInstanceID = MeshDescription.CreateVertexInstance(VertexID);
		UVs[VertexInstanceID] = UV;
		return VertexInstanceID;
	}

	// Builds a centered, compact horizontal slash mesh. Local +Y is the horizontal
	// slash-length axis (UV U), local +Z is the thin vertical slash height, and local
	// +X is a shallow forward crescent bow plus the thin lens depth, so the carrier
	// reads as a small horizontal slash from gameplay angles instead of a flat
	// billboard. The slash tapers to points at both ends and is deliberately NOT the
	// AOE radial crescent nor the Pierce forward vertical lane.
	UStaticMesh* T66BuildBounceSlashMesh(UMaterialInterface* SlashMaterial)
	{
		const FString BounceMeshPackagePath = T66ResolveHero1AxeBouncePath(T66Hero1AxeBounceMeshPackagePath);
		UPackage* Package = CreatePackage(*BounceMeshPackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		UStaticMesh* SlashMesh = FindObject<UStaticMesh>(Package, T66Hero1AxeBounceMeshObjectName);
		if (!SlashMesh)
		{
			SlashMesh = NewObject<UStaticMesh>(
				Package,
				T66Hero1AxeBounceMeshObjectName,
				RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(SlashMesh);
		}
		if (!SlashMesh)
		{
			return nullptr;
		}

		SlashMesh->Modify();
		SlashMesh->bAllowCPUAccess = true;
		SlashMesh->GetStaticMaterials().Reset();
		SlashMesh->GetStaticMaterials().Add(FStaticMaterial(SlashMaterial, TEXT("BounceSlash")));
		SlashMesh->SetLightMapResolution(4);
		SlashMesh->SetLightingGuid();

		FMeshDescription MeshDescription;
		FStaticMeshAttributes Attributes(MeshDescription);
		Attributes.Register();

		TVertexAttributesRef<FVector3f> Positions = Attributes.GetVertexPositions();
		TVertexInstanceAttributesRef<FVector2f> UVs = Attributes.GetVertexInstanceUVs();
		UVs.SetNumChannels(1);
		TPolygonGroupAttributesRef<FName> MaterialSlotNames = Attributes.GetPolygonGroupMaterialSlotNames();

		const FPolygonGroupID PolygonGroupID = MeshDescription.CreatePolygonGroup();
		MaterialSlotNames[PolygonGroupID] = TEXT("BounceSlashProfile");

		constexpr int32 SegmentCount = 48;
		constexpr float Pi = UE_PI;
		// Horizontal half-length (Y), vertical half-height (Z) of the thin slash, lens
		// half-depth (X), and forward crescent bow depth (X), all in Unreal centimeters.
		// The slash is centered at the origin so ImpactAnchored placement (runtime keeps
		// the carrier at world scale 1.0) sits it on each Bounce impact point at gameplay
		// size. Half-length 80 cm maps directly to the Hero1Axe_Bounce_Base
		// BaseVisualRadius=80 binding, the same authored-footprint contract the AOE
		// (radius 132) and Pierce (half-length 150) carriers use. This keeps Bounce the
		// most compact of the three: a small horizontal slash, not the AOE radial
		// crescent nor the Pierce forward lane.
		constexpr float MaxHalfLength = 80.0f;
		constexpr float MaxHalfHeight = 13.5f;
		constexpr float MaxLensHalfDepth = 6.0f;
		constexpr float BowDepth = 21.0f;

		struct FT66SlashStation
		{
			FVertexID Bottom;
			FVertexID Front;
			FVertexID Top;
			FVertexID Back;
		};

		TArray<FT66SlashStation> Stations;
		Stations.Reserve(SegmentCount + 1);

		for (int32 Index = 0; Index <= SegmentCount; ++Index)
		{
			const float T = static_cast<float>(Index) / static_cast<float>(SegmentCount);
			// Y spans the horizontal slash length, centered at the origin.
			const float Y = FMath::Lerp(-MaxHalfLength, MaxHalfLength, T);
			// Forward crescent bow: peaks mid-length, returns to zero at the tapered ends.
			const float BowX = BowDepth * FMath::Sin(T * Pi);
			const float HeightEnvelope = FMath::Pow(FMath::Max(0.0f, FMath::Sin(T * Pi)), 0.65f);
			const float DepthEnvelope = FMath::Pow(FMath::Max(0.0f, FMath::Sin(T * Pi)), 0.5f);
			const float HalfHeight = MaxHalfHeight * HeightEnvelope;
			const float LensHalfDepth = MaxLensHalfDepth * DepthEnvelope;

			FT66SlashStation Station;
			Station.Bottom = MeshDescription.CreateVertex();
			Station.Front = MeshDescription.CreateVertex();
			Station.Top = MeshDescription.CreateVertex();
			Station.Back = MeshDescription.CreateVertex();
			// Diamond lens cross-section in the X-Z plane at each Y station.
			Positions[Station.Bottom] = FVector3f(BowX, Y, -HalfHeight);
			Positions[Station.Front] = FVector3f(BowX + LensHalfDepth, Y, 0.0f);
			Positions[Station.Top] = FVector3f(BowX, Y, HalfHeight);
			Positions[Station.Back] = FVector3f(BowX - LensHalfDepth, Y, 0.0f);
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
			FirstTriangle.Add(T66CreateBounceVertexInstance(MeshDescription, UVs, A, UvA));
			FirstTriangle.Add(T66CreateBounceVertexInstance(MeshDescription, UVs, B, UvB));
			FirstTriangle.Add(T66CreateBounceVertexInstance(MeshDescription, UVs, C, UvC));
			MeshDescription.CreatePolygon(PolygonGroupID, FirstTriangle);

			TArray<FVertexInstanceID> SecondTriangle;
			SecondTriangle.Reserve(3);
			SecondTriangle.Add(T66CreateBounceVertexInstance(MeshDescription, UVs, A, UvA));
			SecondTriangle.Add(T66CreateBounceVertexInstance(MeshDescription, UVs, C, UvC));
			SecondTriangle.Add(T66CreateBounceVertexInstance(MeshDescription, UVs, D, UvD));
			MeshDescription.CreatePolygon(PolygonGroupID, SecondTriangle);
		};

		for (int32 Index = 0; Index < SegmentCount; ++Index)
		{
			const float U0 = static_cast<float>(Index) / static_cast<float>(SegmentCount);
			const float U1 = static_cast<float>(Index + 1) / static_cast<float>(SegmentCount);
			const FT66SlashStation& S0 = Stations[Index];
			const FT66SlashStation& S1 = Stations[Index + 1];

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
		if (!SlashMesh->BuildFromMeshDescriptions({ &MeshDescription }, BuildParams))
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Failed to build Bounce slash mesh buffers."));
			return nullptr;
		}

		const FStaticMeshRenderData* RenderData = SlashMesh->GetRenderData();
		if (!RenderData || !RenderData->LODResources.IsValidIndex(0))
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Bounce slash mesh has no render data after build."));
			return nullptr;
		}

		const FStaticMeshLODResources& LODResources = RenderData->LODResources[0];
		const int32 VertexCount = static_cast<int32>(LODResources.GetNumVertices());
		const int32 IndexCount = LODResources.IndexBuffer.GetNumIndices();
		const int32 SectionCount = LODResources.Sections.Num();
		UE_LOG(LogTemp, Display, TEXT("[Hero1AxeBounceVFX] Bounce slash mesh render data: vertices=%d indices=%d sections=%d"),
			VertexCount,
			IndexCount,
			SectionCount);
		if (VertexCount <= 0 || IndexCount <= 0 || SectionCount <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Bounce slash mesh built with no drawable render buffers."));
			return nullptr;
		}

		SlashMesh->MarkPackageDirty();
		return SlashMesh;
	}
}

UT66Hero1AxeBounceVFXCommandlet::UT66Hero1AxeBounceVFXCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UT66Hero1AxeBounceVFXCommandlet::Main(const FString& Params)
{
	const bool bProductionPaths = T66UseHero1AxeBounceProductionPaths();
	const float DevSlowFactor = T66GetBounceDevSlowFactor();
	const FString BounceNiagaraPath = T66ResolveHero1AxeBouncePath(T66Hero1AxeBounceNiagaraPath);
	UE_LOG(LogTemp, Display, TEXT("[Hero1AxeBounceVFX] Building centered horizontal slash carrier and ImpactAnchored Niagara mesh renderer. ProductionPaths=%s DevSlowFactor=%.2f"), bProductionPaths ? TEXT("true") : TEXT("false"), DevSlowFactor);

#if WITH_EDITOR
	UNiagaraSystem* SlashSystem = T66CreateBounceNiagaraSystem(DevSlowFactor);
#else
	UNiagaraSystem* SlashSystem = LoadObject<UNiagaraSystem>(nullptr, *BounceNiagaraPath);
#endif
	if (!SlashSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Missing or failed to create Niagara system %s"), *BounceNiagaraPath);
		return 1;
	}

	TMap<FString, UMaterialInterface*> BounceLayerMaterials;
	UMaterialInterface* BodyMaterial = nullptr;
	for (const FT66BounceLayerConfig& Config : T66Hero1AxeBounceLayerConfigs)
	{
		const FString MaterialPath = T66ResolveHero1AxeBouncePath(Config.MaterialPath);
		UMaterialInterface* LayerMaterial = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
		if (!LayerMaterial)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Missing shared slash layer material %s"), *MaterialPath);
			return 1;
		}
		BounceLayerMaterials.Add(Config.EmitterName, LayerMaterial);
		if (FCString::Strcmp(Config.EmitterName, TEXT("Emitter_AxeBounceSlash_Body")) == 0)
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
			T66SaveBounceAsset(ConcreteMaterial);
		}
	}
	if (!BodyMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Failed to identify Bounce body layer material."));
		return 1;
	}

	UStaticMesh* SlashMesh = T66BuildBounceSlashMesh(BodyMaterial);
	if (!SlashMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Failed to create Bounce slash mesh asset."));
		return 1;
	}
	if (!T66SaveBounceAsset(SlashMesh))
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Failed to save %s"), *SlashMesh->GetPathName());
		return 1;
	}

	int32 EmitterCount = 0;
	int32 BoundMeshRendererCount = 0;
	SlashSystem->Modify();
	SlashSystem->SetFixedBounds(T66Hero1AxeBounceLocalBounds);

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
		const FT66BounceLayerConfig* Config = T66FindBounceLayerConfig(EmitterName);

		Emitter->Modify();
		EmitterData->bLocalSpace = true;
		EmitterData->CalculateBoundsMode = ENiagaraEmitterCalculateBoundMode::Fixed;
		EmitterData->FixedBounds = T66Hero1AxeBounceLocalBounds;

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
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Unexpected emitter name %s"), *EmitterName);
			return 1;
		}

		UMaterialInterface* const* LayerMaterial = BounceLayerMaterials.Find(EmitterName);
		if (!LayerMaterial || !(*LayerMaterial))
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Missing material binding for emitter %s"), *EmitterName);
			return 1;
		}

		UNiagaraMeshRendererProperties* MeshRenderer =
			NewObject<UNiagaraMeshRendererProperties>(Emitter, NAME_None, RF_Transactional);
		if (!MeshRenderer)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Failed to allocate mesh renderer for emitter %s."), *EmitterName);
			return 1;
		}

		MeshRenderer->Modify();
		FNiagaraMeshRendererMeshProperties MeshProperties;
		MeshProperties.Mesh = SlashMesh;
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
			TEXT("[Hero1AxeBounceVFX] Bound Bounce layer emitter=%s rendererScale=%s material=%s"),
			Config->EmitterName,
			*Config->RendererScale.ToString(),
			*(*LayerMaterial)->GetPathName());
	}

	if (EmitterCount <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Niagara system has no editable emitters."));
		return 1;
	}
	if (BoundMeshRendererCount != UE_ARRAY_COUNT(T66Hero1AxeBounceLayerConfigs))
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Bound %d of %d Bounce layer emitters to mesh renderers."),
			BoundMeshRendererCount,
			UE_ARRAY_COUNT(T66Hero1AxeBounceLayerConfigs));
		return 1;
	}

#if WITH_EDITOR
	SlashSystem->PostEditChange();
	SlashSystem->RequestCompile(true);
#endif

	if (!T66SaveBounceAsset(SlashSystem))
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeBounceVFX] Failed to save %s"), *SlashSystem->GetPathName());
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("[Hero1AxeBounceVFX] Bound %d Bounce slash mesh renderer(s) to %s. ProductionPaths=%s DevSlowFactor=%.2f"),
		BoundMeshRendererCount,
		*SlashMesh->GetPathName(),
		bProductionPaths ? TEXT("true") : TEXT("false"),
		DevSlowFactor);
	return 0;
}
