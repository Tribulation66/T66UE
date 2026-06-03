// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66Hero1AxeAOEVFXCommandlet.h"

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
#include "NiagaraSpriteRendererProperties.h"
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
	const TCHAR* T66Hero1AxeAOESlashNiagaraPath =
		TEXT("/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash");
	const TCHAR* T66Hero1AxeAOESlashNiagaraPackagePath =
		TEXT("/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash");
	const TCHAR* T66Hero1AxeAOESlashNiagaraObjectName =
		TEXT("NS_Hero1AxeAOE_MeshSlash");
	const TCHAR* T66Hero1AxeAOESlashMeshPackagePath =
		TEXT("/Game/VFXLab/Hero1Axe/Shared/SM_Hero1AxeAOE_SlashArc");
	const TCHAR* T66Hero1AxeAOESlashMeshObjectName =
		TEXT("SM_Hero1AxeAOE_SlashArc");

	bool T66UseHero1AxeAOEProductionPaths()
	{
		return FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeAOEProduction"));
	}

	FString T66ResolveHero1AxeAOEPath(const TCHAR* Path)
	{
		FString Resolved(Path);
		if (T66UseHero1AxeAOEProductionPaths())
		{
			Resolved.ReplaceInline(TEXT("/Game/VFXLab/Hero1Axe/AOE"), TEXT("/Game/VFX/Hero1/Axe/AOE"), ESearchCase::CaseSensitive);
			Resolved.ReplaceInline(TEXT("/Game/VFXLab/Hero1Axe/Shared"), TEXT("/Game/VFX/Hero1/Axe/Shared"), ESearchCase::CaseSensitive);
		}
		return Resolved;
	}

	struct FT66SlashLayerConfig
	{
		const TCHAR* EmitterName;
		const TCHAR* MaterialPath;
		const TCHAR* MaterialSlotName;
		float LifetimeSeconds;
		const TCHAR* SpawnScaleDefault;
		const TCHAR* ParticleColorDefault;
		FVector RendererScale;
		float RotationForceZ;
		float LeverRadiusCm;
	};

	const FT66SlashLayerConfig T66Hero1AxeAOESlashLayerConfigs[] =
	{
		{
			TEXT("Emitter_AxeAOESlash_Bright"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Bright.M_Hero1AxeAOE_Slash_Bright"),
			TEXT("SlashBright"),
			0.32f,
			TEXT("0.92,0.82,1.0"),
			TEXT("(R=0.16,G=0.48,B=1.0,A=1.0)"),
			FVector(0.96f, 0.88f, 1.0f),
			0.0f,
			168.0f
		},
		{
			TEXT("Emitter_AxeAOESlash_Body"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Body.M_Hero1AxeAOE_Slash_Body"),
			TEXT("SlashBody"),
			0.38f,
			TEXT("1.07,1.03,1.0"),
			TEXT("(R=1.0,G=0.04,B=0.055,A=1.0)"),
			FVector(1.08f, 1.04f, 1.0f),
			0.0f,
			190.0f
		},
		{
			TEXT("Emitter_AxeAOESlash_Dark"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Dark.M_Hero1AxeAOE_Slash_Dark"),
			TEXT("SlashDark"),
			0.46f,
			TEXT("0.98,0.96,1.0"),
			TEXT("(R=0.018,G=0.045,B=0.20,A=0.58)"),
			FVector(1.02f, 1.0f, 1.0f),
			0.0f,
			206.0f
		}
	};

	struct FT66SupportEmitterConfig
	{
		const TCHAR* EmitterName;
		const TCHAR* MaterialPath;
		float LifetimeSeconds;
		int32 SpawnCount;
		const TCHAR* PositionDefault;
		const TCHAR* VelocityDefault;
		const TCHAR* SpriteSizeDefault;
		const TCHAR* SpriteFacingDefault;
		const TCHAR* SpriteAlignmentDefault;
		const TCHAR* ParticleColorDefault;
		ENiagaraSpriteAlignment Alignment;
		ENiagaraSpriteFacingMode FacingMode;
		int32 SortHint;
	};

	const FT66SupportEmitterConfig T66Hero1AxeAOESupportEmitterConfigs[] =
	{
		{
			TEXT("Emitter_AxeAOEImpact_Flare"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_ImpactFlare.M_Hero1AxeAOE_ImpactFlare"),
			0.24f,
			1,
			TEXT("320.0,0.0,58.0"),
			TEXT("0.0,0.0,32.0"),
			TEXT("X=132.0 Y=132.0"),
			TEXT("0.0,1.0,0.28"),
			TEXT("0.0,1.0,0.0"),
			TEXT("(R=1.0,G=1.0,B=1.0,A=0.90)"),
			ENiagaraSpriteAlignment::Unaligned,
			ENiagaraSpriteFacingMode::FaceCameraPosition,
			20
		},
		{
			TEXT("Emitter_AxeAOESupport_DirectionalSparks"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_DirectionalSpark.M_Hero1AxeAOE_DirectionalSpark"),
			0.34f,
			2,
			TEXT("286.0,0.0,46.0"),
			TEXT("-80.0,0.0,24.0"),
			TEXT("X=4.0 Y=42.0"),
			TEXT("0.0,1.0,0.24"),
			TEXT("0.0,1.0,0.0"),
			TEXT("(R=1.0,G=0.05,B=0.06,A=0.16)"),
			ENiagaraSpriteAlignment::VelocityAligned,
			ENiagaraSpriteFacingMode::FaceCameraPosition,
			18
		},
		{
			TEXT("Emitter_AxeAOESupport_Motes"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Mote.M_Hero1AxeAOE_Mote"),
			0.92f,
			6,
			TEXT("246.0,0.0,72.0"),
			TEXT("-32.0,0.0,18.0"),
			TEXT("X=14.0 Y=14.0"),
			TEXT("0.0,1.0,0.35"),
			TEXT("0.0,1.0,0.0"),
			TEXT("(R=0.18,G=0.42,B=1.0,A=0.11)"),
			ENiagaraSpriteAlignment::Unaligned,
			ENiagaraSpriteFacingMode::FaceCameraPosition,
			8
		},
		{
			TEXT("Emitter_AxeAOESupport_GroundTrace"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_GroundTrace.M_Hero1AxeAOE_GroundTrace"),
			0.58f,
			1,
			TEXT("210.0,0.0,-74.0"),
			TEXT("0.0,0.0,0.0"),
			TEXT("X=360.0 Y=34.0"),
			TEXT("0.0,0.0,1.0"),
			TEXT("1.0,0.0,0.0"),
			TEXT("(R=0.16,G=0.30,B=1.0,A=0.08)"),
			ENiagaraSpriteAlignment::CustomAlignment,
			ENiagaraSpriteFacingMode::CustomFacingVector,
			-2
		}
	};

	float T66GetDevSlowFactor()
	{
		float DevSlowFactor = 1.0f;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEDevSlowFactor="), DevSlowFactor))
		{
			return FMath::Clamp(DevSlowFactor, 1.0f, 20.0f);
		}
		if (FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeAOEDevSlow")))
		{
			return 6.0f;
		}
		return 1.0f;
	}

	float T66ScaleLifetimeForDevSlow(const float LifetimeSeconds, const float DevSlowFactor)
	{
		return LifetimeSeconds * FMath::Max(1.0f, DevSlowFactor);
	}

	float T66ScaleSpawnTimeForDevSlow(const float SpawnTimeSeconds, const float DevSlowFactor)
	{
		return SpawnTimeSeconds * FMath::Max(1.0f, DevSlowFactor);
	}

	float T66ScaleRotationForceForDevSlow(const float RotationForce, const float DevSlowFactor)
	{
		const float SafeFactor = FMath::Max(1.0f, DevSlowFactor);
		return RotationForce / (SafeFactor * SafeFactor);
	}

	FString T66ScaleVectorCsvForDevSlow(const TCHAR* VectorCsv, const float DevSlowFactor)
	{
		const float SafeFactor = FMath::Max(1.0f, DevSlowFactor);
		TArray<FString> Components;
		FString(VectorCsv).ParseIntoArray(Components, TEXT(","), true);
		if (Components.Num() != 3)
		{
			return FString(VectorCsv);
		}

		return FString::Printf(
			TEXT("%.3f,%.3f,%.3f"),
			FCString::Atof(*Components[0]) / SafeFactor,
			FCString::Atof(*Components[1]) / SafeFactor,
			FCString::Atof(*Components[2]) / SafeFactor);
	}

	const FT66SlashLayerConfig* T66FindSlashLayerConfig(const FString& EmitterName)
	{
		for (const FT66SlashLayerConfig& Config : T66Hero1AxeAOESlashLayerConfigs)
		{
			if (EmitterName == Config.EmitterName)
			{
				return &Config;
			}
		}
		return nullptr;
	}

	const FT66SupportEmitterConfig* T66FindSupportEmitterConfig(const FString& EmitterName)
	{
		for (const FT66SupportEmitterConfig& Config : T66Hero1AxeAOESupportEmitterConfigs)
		{
			if (EmitterName == Config.EmitterName)
			{
				return &Config;
			}
		}
		return nullptr;
	}

	bool T66SaveAsset(UObject* Asset)
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
	UNiagaraNodeFunctionCall* T66AddNiagaraModuleFromAssetPath(
		const TCHAR* AssetPath,
		UNiagaraNodeOutput& TargetOutputNode)
	{
		UNiagaraScript* ModuleScript = Cast<UNiagaraScript>(FSoftObjectPath(AssetPath).TryLoad());
		if (!ModuleScript)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Missing Niagara module %s"), AssetPath);
			return nullptr;
		}

		return FNiagaraStackGraphUtilities::AddScriptModuleToStack(ModuleScript, TargetOutputNode);
	}

	FName T66MakeModuleInputParameterName(const FName InputName)
	{
		return FNiagaraParameterHandle::CreateModuleParameterHandle(InputName).GetParameterHandleString();
	}

	bool T66FindModuleInput(
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

		const FName ModuleInputName = T66MakeModuleInputParameterName(InputName);
		for (const FNiagaraVariable& InputVariable : InputVariables)
		{
			if (InputVariable.GetName() == ModuleInputName || InputVariable.GetName() == InputName)
			{
				OutInputVariable = InputVariable;
				return true;
			}
		}

		FString AvailableInputs;
		for (const FNiagaraVariable& InputVariable : InputVariables)
		{
			if (!AvailableInputs.IsEmpty())
			{
				AvailableInputs += TEXT(", ");
			}
			AvailableInputs += FString::Printf(
				TEXT("%s:%s"),
				*InputVariable.GetName().ToString(),
				*InputVariable.GetType().GetName());
		}
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Missing module input %s. Available inputs: %s"),
			*InputName.ToString(),
			*AvailableInputs);
		return false;
	}

	bool T66LinkModuleInputToParticleParameter(
		UNiagaraSystem& OwningSystem,
		const FVersionedNiagaraEmitter& VersionedEmitter,
		const UNiagaraNodeOutput& OutputNode,
		UNiagaraNodeFunctionCall& TargetFunctionCallNode,
		const FName InputName,
		const FNiagaraVariableBase& LinkedParticleParameter)
	{
		FNiagaraVariable InputVariable;
		if (!T66FindModuleInput(VersionedEmitter, OutputNode, TargetFunctionCallNode, InputName, InputVariable))
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

		UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEVFX] Linked %s to %s."),
			*T66MakeModuleInputParameterName(InputName).ToString(),
			*LinkedParticleParameter.GetName().ToString());
		return true;
	}

	template<typename ValueType>
	void T66SetRapidIterationParameter(
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

	bool T66AddSlashLayerEmitter(UNiagaraSystem& SlashSystem, const FT66SlashLayerConfig& Config, const float DevSlowFactor)
	{
		const float ScaledLifetimeSeconds = T66ScaleLifetimeForDevSlow(Config.LifetimeSeconds, DevSlowFactor);
		const float ScaledRotationForceZ = T66ScaleRotationForceForDevSlow(Config.RotationForceZ, DevSlowFactor);
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
		EmitterData->FixedBounds = FBox(FVector(-120.0f, -460.0f, -70.0f), FVector(440.0f, 460.0f, 120.0f));

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

		T66AddNiagaraModuleFromAssetPath(TEXT("/Niagara/Modules/Emitter/EmitterState.EmitterState"), *EmitterUpdateOutputNode);
		UNiagaraNodeFunctionCall* SpawnBurstNode = T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Emitter/SpawnBurst_Instantaneous.SpawnBurst_Instantaneous"),
			*EmitterUpdateOutputNode);
		if (SpawnBurstNode && EmitterData->EmitterUpdateScriptProps.Script)
		{
			T66SetRapidIterationParameter<int32>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Count"),
				FNiagaraTypeDefinition::GetIntDef(),
				1);
			T66SetRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Probability"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Time"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
		}
		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Spawn/Location/SystemLocation.SystemLocation"),
			*ParticleSpawnOutputNode);
		UNiagaraNodeFunctionCall* InitialMeshRotationNode = T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Spawn/Orientation/InitialMeshRotation.InitialMeshRotation"),
			*ParticleSpawnOutputNode);
		if (InitialMeshRotationNode && EmitterData->SpawnScriptProps.Script)
		{
			T66SetRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->SpawnScriptProps.Script,
				*InitialMeshRotationNode,
				TEXT("Pitch"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
			T66SetRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->SpawnScriptProps.Script,
				*InitialMeshRotationNode,
				TEXT("Yaw"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
			T66SetRapidIterationParameter<float>(
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

		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Update/Lifetime/UpdateAge.UpdateAge"),
			*ParticleUpdateOutputNode);
		UNiagaraNodeFunctionCall* DynamicMaterialNode = T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Update/Material/DynamicMaterialParameters.DynamicMaterialParameters"),
			*ParticleUpdateOutputNode);
		if (DynamicMaterialNode && EmitterData->UpdateScriptProps.Script)
		{
			T66SetRapidIterationParameter<FNiagaraBool>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Param0WriteEnabled"),
				FNiagaraTypeDefinition::GetBoolDef(),
				FNiagaraBool(true));
			if (!T66LinkModuleInputToParticleParameter(
				SlashSystem,
				FVersionedNiagaraEmitter(SlashEmitter, EmitterData->Version.VersionGuid),
				*ParticleUpdateOutputNode,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 1"),
				SYS_PARAM_PARTICLES_NORMALIZED_AGE))
			{
				return false;
			}
			T66SetRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 2"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 3"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetRapidIterationParameter<float>(
				SlashEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 4"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
		}

		UNiagaraNodeFunctionCall* MeshRotationForceNode = T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Update/Orientation/MeshRotationForce.MeshRotationForce"),
			*ParticleUpdateOutputNode);
		if (!MeshRotationForceNode || !EmitterData->UpdateScriptProps.Script)
		{
			return false;
		}
		T66SetRapidIterationParameter<float>(
			SlashEmitter->GetUniqueEmitterName(),
			*EmitterData->UpdateScriptProps.Script,
			*MeshRotationForceNode,
			TEXT("X"),
			FNiagaraTypeDefinition::GetFloatDef(),
			0.0f);
		T66SetRapidIterationParameter<float>(
			SlashEmitter->GetUniqueEmitterName(),
			*EmitterData->UpdateScriptProps.Script,
			*MeshRotationForceNode,
			TEXT("Y"),
			FNiagaraTypeDefinition::GetFloatDef(),
			0.0f);
		T66SetRapidIterationParameter<float>(
			SlashEmitter->GetUniqueEmitterName(),
			*EmitterData->UpdateScriptProps.Script,
			*MeshRotationForceNode,
			TEXT("Z"),
			FNiagaraTypeDefinition::GetFloatDef(),
			ScaledRotationForceZ);
		T66SetRapidIterationParameter<float>(
			SlashEmitter->GetUniqueEmitterName(),
			*EmitterData->UpdateScriptProps.Script,
			*MeshRotationForceNode,
			TEXT("Lever Radius (cm)"),
			FNiagaraTypeDefinition::GetFloatDef(),
			Config.LeverRadiusCm);
		T66SetRapidIterationParameter<FNiagaraBool>(
			SlashEmitter->GetUniqueEmitterName(),
			*EmitterData->UpdateScriptProps.Script,
			*MeshRotationForceNode,
			TEXT("Rotate in Mesh Space?"),
			FNiagaraTypeDefinition::GetBoolDef(),
			FNiagaraBool(false));

		UNiagaraNodeFunctionCall* SolveRotationNode = T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Solvers/SolveRotationalForcesAndVelocity.SolveRotationalForcesAndVelocity"),
			*ParticleUpdateOutputNode);
		if (!SolveRotationNode)
		{
			return false;
		}
		UE_LOG(
			LogTemp,
			Display,
			TEXT("[Hero1AxeAOEVFX] MotionOrder emitter=%s MeshRotationForce-before-SolveRotationalForcesAndVelocity ForceZ=%.2f BaseForceZ=%.2f LeverRadius=%.2f Lifetime=%.2f BaseLifetime=%.2f DevSlowFactor=%.2f"),
			Config.EmitterName,
			ScaledRotationForceZ,
			Config.RotationForceZ,
			Config.LeverRadiusCm,
			ScaledLifetimeSeconds,
			Config.LifetimeSeconds,
			DevSlowFactor);

		FNiagaraEditorUtilities::AddEmitterToSystem(SlashSystem, *SlashEmitter, FGuid(), true);
		return true;
	}

	bool T66AddSupportEmitter(UNiagaraSystem& SlashSystem, const FT66SupportEmitterConfig& Config, const float DevSlowFactor)
	{
		const float ScaledLifetimeSeconds = T66ScaleLifetimeForDevSlow(Config.LifetimeSeconds, DevSlowFactor);
		const float ScaledSpawnTimeSeconds = T66ScaleSpawnTimeForDevSlow(0.03f, DevSlowFactor);
		const FString ScaledVelocityDefault = T66ScaleVectorCsvForDevSlow(Config.VelocityDefault, DevSlowFactor);
		UNiagaraEmitter* SupportEmitter = NewObject<UNiagaraEmitter>(
			GetTransientPackage(),
			Config.EmitterName,
			RF_Transactional);
		if (!SupportEmitter)
		{
			return false;
		}
		SupportEmitter->SetUniqueEmitterName(Config.EmitterName);
		UNiagaraEmitterFactoryNew::InitializeEmitter(SupportEmitter, false);
		SupportEmitter->SetUniqueEmitterName(Config.EmitterName);
		FVersionedNiagaraEmitterData* EmitterData = SupportEmitter->GetLatestEmitterData();
		if (!EmitterData || !EmitterData->GraphSource)
		{
			return false;
		}

		EmitterData->SimTarget = ENiagaraSimTarget::CPUSim;
		EmitterData->bLocalSpace = true;
		EmitterData->bDeterminism = true;
		EmitterData->CalculateBoundsMode = ENiagaraEmitterCalculateBoundMode::Fixed;
		EmitterData->FixedBounds = FBox(FVector(-260.0f, -650.0f, -120.0f), FVector(620.0f, 650.0f, 220.0f));

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

		T66AddNiagaraModuleFromAssetPath(TEXT("/Niagara/Modules/Emitter/EmitterState.EmitterState"), *EmitterUpdateOutputNode);
		UNiagaraNodeFunctionCall* SpawnBurstNode = T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Emitter/SpawnBurst_Instantaneous.SpawnBurst_Instantaneous"),
			*EmitterUpdateOutputNode);
		if (SpawnBurstNode && EmitterData->EmitterUpdateScriptProps.Script)
		{
			T66SetRapidIterationParameter<int32>(
				SupportEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Count"),
				FNiagaraTypeDefinition::GetIntDef(),
				Config.SpawnCount);
			T66SetRapidIterationParameter<float>(
				SupportEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Probability"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetRapidIterationParameter<float>(
				SupportEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Time"),
				FNiagaraTypeDefinition::GetFloatDef(),
				ScaledSpawnTimeSeconds);
		}

		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Spawn/Location/SystemLocation.SystemLocation"),
			*ParticleSpawnOutputNode);
		const TArray<FNiagaraVariable> ParticleSpawnVars =
		{
			SYS_PARAM_PARTICLES_LIFETIME,
			SYS_PARAM_PARTICLES_POSITION,
			SYS_PARAM_PARTICLES_VELOCITY,
			SYS_PARAM_PARTICLES_COLOR,
			SYS_PARAM_PARTICLES_SPRITE_SIZE,
			SYS_PARAM_PARTICLES_SPRITE_FACING,
			SYS_PARAM_PARTICLES_SPRITE_ALIGNMENT
		};
		const TArray<FString> ParticleSpawnDefaults =
		{
			FString::Printf(TEXT("%.3f"), ScaledLifetimeSeconds),
			Config.PositionDefault,
			ScaledVelocityDefault,
			Config.ParticleColorDefault,
			Config.SpriteSizeDefault,
			Config.SpriteFacingDefault,
			Config.SpriteAlignmentDefault
		};
		FNiagaraStackGraphUtilities::AddParameterModuleToStack(
			ParticleSpawnVars,
			*ParticleSpawnOutputNode,
			INDEX_NONE,
			ParticleSpawnDefaults);

		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Update/Lifetime/UpdateAge.UpdateAge"),
			*ParticleUpdateOutputNode);
		UNiagaraNodeFunctionCall* DynamicMaterialNode = T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Update/Material/DynamicMaterialParameters.DynamicMaterialParameters"),
			*ParticleUpdateOutputNode);
		if (DynamicMaterialNode && EmitterData->UpdateScriptProps.Script)
		{
			T66SetRapidIterationParameter<FNiagaraBool>(
				SupportEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Param0WriteEnabled"),
				FNiagaraTypeDefinition::GetBoolDef(),
				FNiagaraBool(true));
			if (!T66LinkModuleInputToParticleParameter(
				SlashSystem,
				FVersionedNiagaraEmitter(SupportEmitter, EmitterData->Version.VersionGuid),
				*ParticleUpdateOutputNode,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 1"),
				SYS_PARAM_PARTICLES_NORMALIZED_AGE))
			{
				return false;
			}
		}
		T66AddNiagaraModuleFromAssetPath(
			TEXT("/Niagara/Modules/Solvers/SolveForcesAndVelocity.SolveForcesAndVelocity"),
			*ParticleUpdateOutputNode);

		UE_LOG(
			LogTemp,
			Display,
			TEXT("[Hero1AxeAOEVFX] SupportEmitter emitter=%s spawnCount=%d lifetime=%.2f baseLifetime=%.2f spawnTime=%.2f velocity=%s baseVelocity=%s position=%s spriteSize=%s DevSlowFactor=%.2f"),
			Config.EmitterName,
			Config.SpawnCount,
			ScaledLifetimeSeconds,
			Config.LifetimeSeconds,
			ScaledSpawnTimeSeconds,
			*ScaledVelocityDefault,
			Config.VelocityDefault,
			Config.PositionDefault,
			Config.SpriteSizeDefault,
			DevSlowFactor);

		FNiagaraEditorUtilities::AddEmitterToSystem(SlashSystem, *SupportEmitter, FGuid(), true);
		return true;
	}

	UNiagaraSystem* T66CreateSlashNiagaraSystem(const bool bCarrierOnly, const float DevSlowFactor)
	{
		const FString SlashNiagaraPackagePath = T66ResolveHero1AxeAOEPath(T66Hero1AxeAOESlashNiagaraPackagePath);
		UPackage* Package = CreatePackage(*SlashNiagaraPackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		if (UNiagaraSystem* ExistingSystem = FindObject<UNiagaraSystem>(Package, T66Hero1AxeAOESlashNiagaraObjectName))
		{
			ExistingSystem->ClearFlags(RF_Public | RF_Standalone);
			ExistingSystem->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
		}

		UNiagaraSystem* SlashSystem = NewObject<UNiagaraSystem>(
			Package,
			FName(T66Hero1AxeAOESlashNiagaraObjectName),
			RF_Public | RF_Standalone | RF_Transactional);
		if (!SlashSystem)
		{
			return nullptr;
		}

		UNiagaraSystemFactoryNew::InitializeSystem(SlashSystem, true);

		for (const FT66SlashLayerConfig& Config : T66Hero1AxeAOESlashLayerConfigs)
		{
			if (!T66AddSlashLayerEmitter(*SlashSystem, Config, DevSlowFactor))
			{
				UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Failed to add slash layer emitter %s"), Config.EmitterName);
				return nullptr;
			}
		}
		if (bCarrierOnly)
		{
			UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEVFX] Carrier-only diagnostic mode: support emitters skipped."));
		}
		else
		{
			for (const FT66SupportEmitterConfig& Config : T66Hero1AxeAOESupportEmitterConfigs)
			{
				if (!T66AddSupportEmitter(*SlashSystem, Config, DevSlowFactor))
				{
					UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Failed to add support emitter %s"), Config.EmitterName);
					return nullptr;
				}
			}
		}
		FAssetRegistryModule::AssetCreated(SlashSystem);
		return SlashSystem;
	}
#endif

	FVertexInstanceID T66CreateSlashVertexInstance(
		FMeshDescription& MeshDescription,
		TVertexInstanceAttributesRef<FVector2f>& UVs,
		const FVertexID VertexID,
		const FVector2f UV)
	{
		const FVertexInstanceID VertexInstanceID = MeshDescription.CreateVertexInstance(VertexID);
		UVs[VertexInstanceID] = UV;
		return VertexInstanceID;
	}

	UStaticMesh* T66BuildSlashArcMesh(UMaterialInterface* SlashMaterial)
	{
		const FString SlashMeshPackagePath = T66ResolveHero1AxeAOEPath(T66Hero1AxeAOESlashMeshPackagePath);
		UPackage* Package = CreatePackage(*SlashMeshPackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		UStaticMesh* SlashMesh = FindObject<UStaticMesh>(Package, T66Hero1AxeAOESlashMeshObjectName);
		if (!SlashMesh)
		{
			SlashMesh = NewObject<UStaticMesh>(
				Package,
				T66Hero1AxeAOESlashMeshObjectName,
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
		SlashMesh->GetStaticMaterials().Add(FStaticMaterial(SlashMaterial, TEXT("SlashReveal")));
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
		MaterialSlotNames[PolygonGroupID] = TEXT("SlashLayeredProfile");

		struct FT66SlashProfileSample
		{
			float T;
			float AngleDeg;
			float InnerRadius;
			float OuterRadius;
			float TangentOffset;
		};

		static constexpr FT66SlashProfileSample SlashProfileSamples[] =
		{
			{ 0.00f, -90.0f, 310.0f, 338.0f, -8.0f },
			{ 0.10f, -72.0f, 292.0f, 346.0f, -5.0f },
			{ 0.24f, -47.0f, 260.0f, 360.0f, 0.0f },
			{ 0.40f, -18.0f, 232.0f, 374.0f, 5.0f },
			{ 0.50f, 0.0f, 222.0f, 382.0f, 0.0f },
			{ 0.60f, 18.0f, 232.0f, 374.0f, -5.0f },
			{ 0.76f, 47.0f, 260.0f, 360.0f, 0.0f },
			{ 0.90f, 72.0f, 292.0f, 346.0f, 5.0f },
			{ 1.00f, 90.0f, 310.0f, 338.0f, 8.0f },
		};

		auto InterpolateProfile = [](const float T)
		{
			for (int32 SampleIndex = 0; SampleIndex < UE_ARRAY_COUNT(SlashProfileSamples) - 1; ++SampleIndex)
			{
				const FT66SlashProfileSample& A = SlashProfileSamples[SampleIndex];
				const FT66SlashProfileSample& B = SlashProfileSamples[SampleIndex + 1];
				if (T <= B.T)
				{
					const float LocalT = (T - A.T) / FMath::Max(0.001f, B.T - A.T);
					const float SmoothT = LocalT * LocalT * (3.0f - 2.0f * LocalT);
					return FT66SlashProfileSample
					{
						T,
						FMath::Lerp(A.AngleDeg, B.AngleDeg, SmoothT),
						FMath::Lerp(A.InnerRadius, B.InnerRadius, SmoothT),
						FMath::Lerp(A.OuterRadius, B.OuterRadius, SmoothT),
						FMath::Lerp(A.TangentOffset, B.TangentOffset, SmoothT)
					};
				}
			}
			return SlashProfileSamples[UE_ARRAY_COUNT(SlashProfileSamples) - 1];
		};

		constexpr int32 SegmentCount = 96;
		constexpr float Pi = UE_PI;

		TArray<FVertexID> InnerVertices;
		TArray<FVertexID> OuterVertices;
		InnerVertices.Reserve(SegmentCount + 1);
		OuterVertices.Reserve(SegmentCount + 1);

		for (int32 Index = 0; Index <= SegmentCount; ++Index)
		{
			const float T = static_cast<float>(Index) / static_cast<float>(SegmentCount);
			FT66SlashProfileSample Profile = InterpolateProfile(T);
			const float EdgeEnvelope = FMath::Pow(FMath::Max(0.0f, FMath::Sin(T * Pi)), 0.8f);
			const float OuterRipple = EdgeEnvelope * (FMath::Sin(T * Pi * 7.0f) * 1.8f + FMath::Sin(T * Pi * 13.0f) * 0.7f);
			const float InnerRipple = EdgeEnvelope * (FMath::Sin(T * Pi * 5.0f + 0.9f) * 1.1f);
			const float Angle = FMath::DegreesToRadians(Profile.AngleDeg);
			const float InnerRadius = Profile.InnerRadius + InnerRipple;
			const float OuterRadius = Profile.OuterRadius + OuterRipple;
			const FVector2f Direction(FMath::Cos(Angle), FMath::Sin(Angle));
			const FVector2f Tangent(-Direction.Y, Direction.X);

			const FVertexID InnerID = MeshDescription.CreateVertex();
			const FVertexID OuterID = MeshDescription.CreateVertex();
			const FVector2f InnerPosition = Direction * InnerRadius + Tangent * (Profile.TangentOffset * 0.45f);
			const FVector2f OuterPosition = Direction * OuterRadius + Tangent * Profile.TangentOffset;
			Positions[InnerID] = FVector3f(InnerPosition.X, InnerPosition.Y, 0.0f);
			Positions[OuterID] = FVector3f(OuterPosition.X, OuterPosition.Y, 0.0f);
			InnerVertices.Add(InnerID);
			OuterVertices.Add(OuterID);
		}

		for (int32 Index = 0; Index < SegmentCount; ++Index)
		{
			const float U0 = static_cast<float>(Index) / static_cast<float>(SegmentCount);
			const float U1 = static_cast<float>(Index + 1) / static_cast<float>(SegmentCount);

			TArray<FVertexInstanceID> FirstTriangle;
			FirstTriangle.Reserve(3);
			FirstTriangle.Add(T66CreateSlashVertexInstance(MeshDescription, UVs, InnerVertices[Index], FVector2f(U0, 0.0f)));
			FirstTriangle.Add(T66CreateSlashVertexInstance(MeshDescription, UVs, OuterVertices[Index], FVector2f(U0, 1.0f)));
			FirstTriangle.Add(T66CreateSlashVertexInstance(MeshDescription, UVs, OuterVertices[Index + 1], FVector2f(U1, 1.0f)));
			MeshDescription.CreatePolygon(PolygonGroupID, FirstTriangle);

			TArray<FVertexInstanceID> SecondTriangle;
			SecondTriangle.Reserve(3);
			SecondTriangle.Add(T66CreateSlashVertexInstance(MeshDescription, UVs, InnerVertices[Index], FVector2f(U0, 0.0f)));
			SecondTriangle.Add(T66CreateSlashVertexInstance(MeshDescription, UVs, OuterVertices[Index + 1], FVector2f(U1, 1.0f)));
			SecondTriangle.Add(T66CreateSlashVertexInstance(MeshDescription, UVs, InnerVertices[Index + 1], FVector2f(U1, 0.0f)));
			MeshDescription.CreatePolygon(PolygonGroupID, SecondTriangle);
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
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Failed to build slash arc mesh buffers."));
			return nullptr;
		}

		const FStaticMeshRenderData* RenderData = SlashMesh->GetRenderData();
		if (!RenderData || !RenderData->LODResources.IsValidIndex(0))
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Slash arc mesh has no render data after build."));
			return nullptr;
		}

		const FStaticMeshLODResources& LODResources = RenderData->LODResources[0];
		const int32 VertexCount = static_cast<int32>(LODResources.GetNumVertices());
		const int32 IndexCount = LODResources.IndexBuffer.GetNumIndices();
		const int32 SectionCount = LODResources.Sections.Num();
		UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEVFX] Slash arc mesh render data: vertices=%d indices=%d sections=%d"),
			VertexCount,
			IndexCount,
			SectionCount);
		if (VertexCount <= 0 || IndexCount <= 0 || SectionCount <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Slash arc mesh built with no drawable render buffers."));
			return nullptr;
		}

		SlashMesh->MarkPackageDirty();
		return SlashMesh;
	}
}

UT66Hero1AxeAOEVFXCommandlet::UT66Hero1AxeAOEVFXCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UT66Hero1AxeAOEVFXCommandlet::Main(const FString& Params)
{
	const bool bCarrierOnly = FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeAOECarrierOnly"));
	const bool bProductionPaths = T66UseHero1AxeAOEProductionPaths();
	const float DevSlowFactor = T66GetDevSlowFactor();
	const FString SlashNiagaraPath = T66ResolveHero1AxeAOEPath(T66Hero1AxeAOESlashNiagaraPath);
	UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEVFX] Building mesh slash carrier and Niagara mesh renderer. CarrierOnly=%s ProductionPaths=%s DevSlowFactor=%.2f"), bCarrierOnly ? TEXT("true") : TEXT("false"), bProductionPaths ? TEXT("true") : TEXT("false"), DevSlowFactor);

#if WITH_EDITOR
	UNiagaraSystem* SlashSystem = T66CreateSlashNiagaraSystem(bCarrierOnly, DevSlowFactor);
#else
	UNiagaraSystem* SlashSystem = LoadObject<UNiagaraSystem>(nullptr, *SlashNiagaraPath);
#endif
	if (!SlashSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Missing or failed to create Niagara system %s"), *SlashNiagaraPath);
		return 1;
	}

	TMap<FString, UMaterialInterface*> SlashLayerMaterials;
	TMap<FString, UMaterialInterface*> SupportEmitterMaterials;
	UMaterialInterface* BodyMaterial = nullptr;
	for (const FT66SlashLayerConfig& Config : T66Hero1AxeAOESlashLayerConfigs)
	{
		const FString MaterialPath = T66ResolveHero1AxeAOEPath(Config.MaterialPath);
		UMaterialInterface* LayerMaterial = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
		if (!LayerMaterial)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Missing slash layer material %s"), *MaterialPath);
			return 1;
		}
		SlashLayerMaterials.Add(Config.EmitterName, LayerMaterial);
		if (FCString::Strcmp(Config.EmitterName, TEXT("Emitter_AxeAOESlash_Body")) == 0)
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
			T66SaveAsset(ConcreteMaterial);
		}
	}
	if (!bCarrierOnly)
	{
		for (const FT66SupportEmitterConfig& Config : T66Hero1AxeAOESupportEmitterConfigs)
		{
			const FString MaterialPath = T66ResolveHero1AxeAOEPath(Config.MaterialPath);
			UMaterialInterface* SupportMaterial = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
			if (!SupportMaterial)
			{
				UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Missing support material %s"), *MaterialPath);
				return 1;
			}
			SupportEmitterMaterials.Add(Config.EmitterName, SupportMaterial);
			if (UMaterial* ConcreteMaterial = Cast<UMaterial>(SupportMaterial))
			{
				ConcreteMaterial->Modify();
				ConcreteMaterial->bUsedWithNiagaraSprites = true;
#if WITH_EDITOR
				ConcreteMaterial->PostEditChange();
#endif
				T66SaveAsset(ConcreteMaterial);
			}
		}
	}
	if (!BodyMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Failed to identify body layer material."));
		return 1;
	}

	UStaticMesh* SlashMesh = T66BuildSlashArcMesh(BodyMaterial);
	if (!SlashMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Failed to create slash arc mesh asset."));
		return 1;
	}
	if (!T66SaveAsset(SlashMesh))
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Failed to save %s"), *SlashMesh->GetPathName());
		return 1;
	}

	int32 EmitterCount = 0;
	int32 BoundMeshRendererCount = 0;
	int32 BoundSupportRendererCount = 0;
	SlashSystem->Modify();
	SlashSystem->SetFixedBounds(FBox(FVector(-260.0f, -650.0f, -120.0f), FVector(620.0f, 650.0f, 220.0f)));

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
		const FT66SlashLayerConfig* Config = T66FindSlashLayerConfig(EmitterName);
		const FT66SupportEmitterConfig* SupportConfig = T66FindSupportEmitterConfig(EmitterName);

		Emitter->Modify();
		EmitterData->bLocalSpace = true;
		EmitterData->CalculateBoundsMode = ENiagaraEmitterCalculateBoundMode::Fixed;
		EmitterData->FixedBounds = FBox(FVector(-260.0f, -650.0f, -120.0f), FVector(620.0f, 650.0f, 220.0f));

		TArray<UNiagaraRendererProperties*> ExistingRenderers = EmitterData->GetRenderers();
		for (UNiagaraRendererProperties* Renderer : ExistingRenderers)
		{
			if (Renderer)
			{
				Emitter->RemoveRenderer(Renderer, VersionedEmitter.Version);
			}
		}

		if (Config)
		{
			UMaterialInterface* const* LayerMaterial = SlashLayerMaterials.Find(EmitterName);
			if (!LayerMaterial || !(*LayerMaterial))
			{
				UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Missing material binding for emitter %s"), *EmitterName);
				return 1;
			}

			UNiagaraMeshRendererProperties* MeshRenderer =
				NewObject<UNiagaraMeshRendererProperties>(Emitter, NAME_None, RF_Transactional);
			if (!MeshRenderer)
			{
				UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Failed to allocate mesh renderer for emitter %s."), *EmitterHandle.GetName().ToString());
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
				TEXT("[Hero1AxeAOEVFX] Bound layer emitter=%s lifetime=%.2f baseLifetime=%.2f rendererScale=%s material=%s DevSlowFactor=%.2f"),
				Config->EmitterName,
				T66ScaleLifetimeForDevSlow(Config->LifetimeSeconds, DevSlowFactor),
				Config->LifetimeSeconds,
				*Config->RendererScale.ToString(),
				*(*LayerMaterial)->GetPathName(),
				DevSlowFactor);
			continue;
		}

		if (SupportConfig)
		{
			UMaterialInterface* const* SupportMaterial = SupportEmitterMaterials.Find(EmitterName);
			if (!SupportMaterial || !(*SupportMaterial))
			{
				UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Missing support material binding for emitter %s"), *EmitterName);
				return 1;
			}

			UNiagaraSpriteRendererProperties* SpriteRenderer =
				NewObject<UNiagaraSpriteRendererProperties>(Emitter, NAME_None, RF_Transactional);
			if (!SpriteRenderer)
			{
				UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Failed to allocate sprite renderer for support emitter %s."), *EmitterHandle.GetName().ToString());
				return 1;
			}

			SpriteRenderer->Modify();
			SpriteRenderer->Material = *SupportMaterial;
			SpriteRenderer->SourceMode = ENiagaraRendererSourceDataMode::Particles;
			SpriteRenderer->Alignment = SupportConfig->Alignment;
			SpriteRenderer->FacingMode = SupportConfig->FacingMode;
			SpriteRenderer->SortMode = ENiagaraSortMode::ViewDepth;
			SpriteRenderer->bCastShadows = false;
			SpriteRenderer->bEnableCameraDistanceCulling = false;
			SpriteRenderer->SetIsEnabled(true);

			Emitter->AddRenderer(SpriteRenderer, VersionedEmitter.Version);
			BoundSupportRendererCount += 1;
			UE_LOG(
				LogTemp,
				Display,
				TEXT("[Hero1AxeAOEVFX] Bound support emitter=%s lifetime=%.2f baseLifetime=%.2f spawnCount=%d spriteSize=%s material=%s DevSlowFactor=%.2f"),
				SupportConfig->EmitterName,
				T66ScaleLifetimeForDevSlow(SupportConfig->LifetimeSeconds, DevSlowFactor),
				SupportConfig->LifetimeSeconds,
				SupportConfig->SpawnCount,
				SupportConfig->SpriteSizeDefault,
				*(*SupportMaterial)->GetPathName(),
				DevSlowFactor);
			continue;
		}

		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Unexpected emitter name %s"), *EmitterName);
		return 1;
	}

	if (EmitterCount <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Niagara system has no editable emitters."));
		return 1;
	}
	if (BoundMeshRendererCount != UE_ARRAY_COUNT(T66Hero1AxeAOESlashLayerConfigs))
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Bound %d of %d slash layer emitters to mesh renderers."),
			BoundMeshRendererCount,
			UE_ARRAY_COUNT(T66Hero1AxeAOESlashLayerConfigs));
		return 1;
	}
	const int32 ExpectedSupportRendererCount = bCarrierOnly ? 0 : UE_ARRAY_COUNT(T66Hero1AxeAOESupportEmitterConfigs);
	if (BoundSupportRendererCount != ExpectedSupportRendererCount)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Bound %d of %d support emitters to sprite renderers. CarrierOnly=%s"),
			BoundSupportRendererCount,
			ExpectedSupportRendererCount,
			bCarrierOnly ? TEXT("true") : TEXT("false"));
		return 1;
	}

#if WITH_EDITOR
	SlashSystem->PostEditChange();
	SlashSystem->RequestCompile(true);
#endif

	if (!T66SaveAsset(SlashSystem))
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEVFX] Failed to save %s"), *SlashSystem->GetPathName());
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEVFX] Bound %d layered Niagara mesh renderer(s) and %d support sprite renderer(s) to %s. CarrierOnly=%s DevSlowFactor=%.2f"),
		BoundMeshRendererCount,
		BoundSupportRendererCount,
		*SlashMesh->GetPathName(),
		bCarrierOnly ? TEXT("true") : TEXT("false"),
		DevSlowFactor);
	return 0;
}
