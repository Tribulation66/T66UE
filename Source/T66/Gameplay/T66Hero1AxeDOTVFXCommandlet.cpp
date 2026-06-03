// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66Hero1AxeDOTVFXCommandlet.h"

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
	const TCHAR* T66Hero1AxeDOTNiagaraPath =
		TEXT("/Game/VFXLab/Hero1Axe/DOT/NS_Hero1AxeDOT_MeshSlash.NS_Hero1AxeDOT_MeshSlash");
	const TCHAR* T66Hero1AxeDOTNiagaraPackagePath =
		TEXT("/Game/VFXLab/Hero1Axe/DOT/NS_Hero1AxeDOT_MeshSlash");
	const TCHAR* T66Hero1AxeDOTNiagaraObjectName =
		TEXT("NS_Hero1AxeDOT_MeshSlash");
	const TCHAR* T66Hero1AxeDOTMeshPackagePath =
		TEXT("/Game/VFXLab/Hero1Axe/DOT/SM_Hero1AxeDOT_AuraRing");
	const TCHAR* T66Hero1AxeDOTMeshObjectName =
		TEXT("SM_Hero1AxeDOT_AuraRing");

	// Local-space bounds (Unreal centimeters) for the centered aura-ring carrier:
	// the ring lies in the Y-Z plane and the moving DOT shot transports it head-on
	// along +X. Major radius 66 + tube radius 12 -> ring extents ~78 along Y/Z; the
	// thin tube depth along X is ~12. This matches the Hero1Axe_DOT_Base
	// BaseVisualRadius=80 binding convention shared with the AOE radius=132, Pierce
	// half-length=150, and Bounce half-length=80 carriers.
	const FBox T66Hero1AxeDOTLocalBounds(FVector(-16.0f, -84.0f, -84.0f), FVector(16.0f, 84.0f, 84.0f));

	bool T66UseHero1AxeDOTProductionPaths()
	{
		return FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeDOTProduction"));
	}

	FString T66ResolveHero1AxeDOTPath(const TCHAR* Path)
	{
		FString Resolved(Path);
		if (T66UseHero1AxeDOTProductionPaths())
		{
			Resolved.ReplaceInline(TEXT("/Game/VFXLab/Hero1Axe/DOT"), TEXT("/Game/VFX/Hero1/Axe/DOT"), ESearchCase::CaseSensitive);
			Resolved.ReplaceInline(TEXT("/Game/VFXLab/Hero1Axe/Shared"), TEXT("/Game/VFX/Hero1/Axe/Shared"), ESearchCase::CaseSensitive);
		}
		return Resolved;
	}

	// DOT reuses the shared Hero 1 AOE slash-layer materials. The carrier geometry
	// (compact aura ring) and the single hero->target moving-shot transport make DOT
	// distinct from the AOE radial crescent, the Pierce forward vertical lane, and the
	// Bounce horizontal slash; the red/blue/white material vocabulary is intentionally
	// shared. Runtime overrides User.Color/Tint with the hero DOT shot color.
	struct FT66DOTLayerConfig
	{
		const TCHAR* EmitterName;
		const TCHAR* MaterialPath;
		float LifetimeSeconds;
		const TCHAR* SpawnScaleDefault;
		const TCHAR* ParticleColorDefault;
		FVector RendererScale;
	};

	const FT66DOTLayerConfig T66Hero1AxeDOTLayerConfigs[] =
	{
		{
			TEXT("Emitter_AxeDOTRing_Bright"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Bright.M_Hero1AxeAOE_Slash_Bright"),
			0.52f,
			TEXT("0.94,0.96,0.96"),
			TEXT("(R=0.16,G=0.48,B=1.0,A=1.0)"),
			FVector(1.0f, 1.0f, 1.0f)
		},
		{
			TEXT("Emitter_AxeDOTRing_Body"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Body.M_Hero1AxeAOE_Slash_Body"),
			0.60f,
			TEXT("1.0,1.0,1.0"),
			TEXT("(R=0.16,G=0.52,B=1.0,A=1.0)"),
			FVector(1.0f, 1.0f, 1.0f)
		},
		{
			TEXT("Emitter_AxeDOTRing_Dark"),
			TEXT("/Game/VFXLab/Hero1Axe/Shared/M_Hero1AxeAOE_Slash_Dark.M_Hero1AxeAOE_Slash_Dark"),
			0.68f,
			TEXT("1.05,1.06,1.06"),
			TEXT("(R=0.018,G=0.045,B=0.20,A=0.58)"),
			FVector(1.0f, 1.0f, 1.0f)
		}
	};

	float T66GetDOTDevSlowFactor()
	{
		float DevSlowFactor = 1.0f;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeDOTDevSlowFactor="), DevSlowFactor))
		{
			return FMath::Clamp(DevSlowFactor, 1.0f, 20.0f);
		}
		if (FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeDOTDevSlow")))
		{
			return 6.0f;
		}
		return 1.0f;
	}

	float T66ScaleDOTLifetimeForDevSlow(const float LifetimeSeconds, const float DevSlowFactor)
	{
		return LifetimeSeconds * FMath::Max(1.0f, DevSlowFactor);
	}

	const FT66DOTLayerConfig* T66FindDOTLayerConfig(const FString& EmitterName)
	{
		for (const FT66DOTLayerConfig& Config : T66Hero1AxeDOTLayerConfigs)
		{
			if (EmitterName == Config.EmitterName)
			{
				return &Config;
			}
		}
		return nullptr;
	}

	bool T66SaveDOTAsset(UObject* Asset)
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
	UNiagaraNodeFunctionCall* T66AddDOTNiagaraModule(
		const TCHAR* AssetPath,
		UNiagaraNodeOutput& TargetOutputNode)
	{
		UNiagaraScript* ModuleScript = Cast<UNiagaraScript>(FSoftObjectPath(AssetPath).TryLoad());
		if (!ModuleScript)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Missing Niagara module %s"), AssetPath);
			return nullptr;
		}

		return FNiagaraStackGraphUtilities::AddScriptModuleToStack(ModuleScript, TargetOutputNode);
	}

	FName T66MakeDOTModuleInputParameterName(const FName InputName)
	{
		return FNiagaraParameterHandle::CreateModuleParameterHandle(InputName).GetParameterHandleString();
	}

	template<typename ValueType>
	void T66SetDOTRapidIterationParameter(
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

	bool T66AddDOTLayerEmitter(UNiagaraSystem& RingSystem, const FT66DOTLayerConfig& Config, const float DevSlowFactor)
	{
		const float ScaledLifetimeSeconds = T66ScaleDOTLifetimeForDevSlow(Config.LifetimeSeconds, DevSlowFactor);
		UNiagaraEmitter* RingEmitter = NewObject<UNiagaraEmitter>(
			GetTransientPackage(),
			Config.EmitterName,
			RF_Transactional);
		if (!RingEmitter)
		{
			return false;
		}
		RingEmitter->SetUniqueEmitterName(Config.EmitterName);
		UNiagaraEmitterFactoryNew::InitializeEmitter(RingEmitter, false);
		RingEmitter->SetUniqueEmitterName(Config.EmitterName);
		FVersionedNiagaraEmitterData* EmitterData = RingEmitter->GetLatestEmitterData();
		if (!EmitterData || !EmitterData->GraphSource)
		{
			return false;
		}

		EmitterData->SimTarget = ENiagaraSimTarget::CPUSim;
		EmitterData->bLocalSpace = true;
		EmitterData->bDeterminism = true;
		EmitterData->CalculateBoundsMode = ENiagaraEmitterCalculateBoundMode::Fixed;
		// Local-space bounds for the centered aura ring. The moving DOT shot transports
		// this carrier from the hero attack origin to the resolved target impact point.
		EmitterData->FixedBounds = T66Hero1AxeDOTLocalBounds;

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

		T66AddDOTNiagaraModule(TEXT("/Niagara/Modules/Emitter/EmitterState.EmitterState"), *EmitterUpdateOutputNode);
		UNiagaraNodeFunctionCall* SpawnBurstNode = T66AddDOTNiagaraModule(
			TEXT("/Niagara/Modules/Emitter/SpawnBurst_Instantaneous.SpawnBurst_Instantaneous"),
			*EmitterUpdateOutputNode);
		if (SpawnBurstNode && EmitterData->EmitterUpdateScriptProps.Script)
		{
			T66SetDOTRapidIterationParameter<int32>(
				RingEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Count"),
				FNiagaraTypeDefinition::GetIntDef(),
				1);
			T66SetDOTRapidIterationParameter<float>(
				RingEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Probability"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetDOTRapidIterationParameter<float>(
				RingEmitter->GetUniqueEmitterName(),
				*EmitterData->EmitterUpdateScriptProps.Script,
				*SpawnBurstNode,
				TEXT("Spawn Time"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
		}
		// Keep the mesh at local zero. The DOT carrier is attached to a visual
		// projectile that moves from the hero attack origin to the target impact point;
		// adding SystemLocation in a local-space emitter bakes an owner/world offset into
		// the particle and makes the ring appear pinned near the hero instead of
		// following the moving component.
		UNiagaraNodeFunctionCall* InitialMeshRotationNode = T66AddDOTNiagaraModule(
			TEXT("/Niagara/Modules/Spawn/Orientation/InitialMeshRotation.InitialMeshRotation"),
			*ParticleSpawnOutputNode);
		if (InitialMeshRotationNode && EmitterData->SpawnScriptProps.Script)
		{
			T66SetDOTRapidIterationParameter<float>(
				RingEmitter->GetUniqueEmitterName(),
				*EmitterData->SpawnScriptProps.Script,
				*InitialMeshRotationNode,
				TEXT("Pitch"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
			T66SetDOTRapidIterationParameter<float>(
				RingEmitter->GetUniqueEmitterName(),
				*EmitterData->SpawnScriptProps.Script,
				*InitialMeshRotationNode,
				TEXT("Yaw"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.0f);
			T66SetDOTRapidIterationParameter<float>(
				RingEmitter->GetUniqueEmitterName(),
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

		T66AddDOTNiagaraModule(
			TEXT("/Niagara/Modules/Update/Lifetime/UpdateAge.UpdateAge"),
			*ParticleUpdateOutputNode);
		UNiagaraNodeFunctionCall* DynamicMaterialNode = T66AddDOTNiagaraModule(
			TEXT("/Niagara/Modules/Update/Material/DynamicMaterialParameters.DynamicMaterialParameters"),
			*ParticleUpdateOutputNode);
		if (DynamicMaterialNode && EmitterData->UpdateScriptProps.Script)
		{
			T66SetDOTRapidIterationParameter<FNiagaraBool>(
				RingEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Param0WriteEnabled"),
				FNiagaraTypeDefinition::GetBoolDef(),
				FNiagaraBool(true));
			// DOT is a moving projectile carrier, like Bounce and unlike the stationary
			// AOE/Pierce impact reveals. Hold the shared slash material in its active
			// mid-life band while the actor travels; forcing it to age 1.0 starts the
			// carrier in its dissipated/end-mask state and leaves only tiny late fragments
			// in capture.
			T66SetDOTRapidIterationParameter<float>(
				RingEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 1"),
				FNiagaraTypeDefinition::GetFloatDef(),
				0.45f);
			T66SetDOTRapidIterationParameter<float>(
				RingEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 2"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetDOTRapidIterationParameter<float>(
				RingEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 3"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
			T66SetDOTRapidIterationParameter<float>(
				RingEmitter->GetUniqueEmitterName(),
				*EmitterData->UpdateScriptProps.Script,
				*DynamicMaterialNode,
				TEXT("Index 0 Param 4"),
				FNiagaraTypeDefinition::GetFloatDef(),
				1.0f);
		}

		UE_LOG(
			LogTemp,
			Display,
			TEXT("[Hero1AxeDOTVFX] DOT layer emitter=%s lifetime=%.2f baseLifetime=%.2f DevSlowFactor=%.2f"),
			Config.EmitterName,
			ScaledLifetimeSeconds,
			Config.LifetimeSeconds,
			DevSlowFactor);

		FNiagaraEditorUtilities::AddEmitterToSystem(RingSystem, *RingEmitter, FGuid(), true);
		return true;
	}

	UNiagaraSystem* T66CreateDOTNiagaraSystem(const float DevSlowFactor)
	{
		const FString DOTNiagaraPackagePath = T66ResolveHero1AxeDOTPath(T66Hero1AxeDOTNiagaraPackagePath);
		UPackage* Package = CreatePackage(*DOTNiagaraPackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		if (UNiagaraSystem* ExistingSystem = FindObject<UNiagaraSystem>(Package, T66Hero1AxeDOTNiagaraObjectName))
		{
			ExistingSystem->ClearFlags(RF_Public | RF_Standalone);
			ExistingSystem->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
		}

		UNiagaraSystem* RingSystem = NewObject<UNiagaraSystem>(
			Package,
			FName(T66Hero1AxeDOTNiagaraObjectName),
			RF_Public | RF_Standalone | RF_Transactional);
		if (!RingSystem)
		{
			return nullptr;
		}

		UNiagaraSystemFactoryNew::InitializeSystem(RingSystem, true);

		for (const FT66DOTLayerConfig& Config : T66Hero1AxeDOTLayerConfigs)
		{
			if (!T66AddDOTLayerEmitter(*RingSystem, Config, DevSlowFactor))
			{
				UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Failed to add DOT layer emitter %s"), Config.EmitterName);
				return nullptr;
			}
		}
		FAssetRegistryModule::AssetCreated(RingSystem);
		return RingSystem;
	}
#endif

	FVertexInstanceID T66CreateDOTVertexInstance(
		FMeshDescription& MeshDescription,
		TVertexInstanceAttributesRef<FVector2f>& UVs,
		const FVertexID VertexID,
		const FVector2f UV)
	{
		const FVertexInstanceID VertexInstanceID = MeshDescription.CreateVertexInstance(VertexID);
		UVs[VertexInstanceID] = UV;
		return VertexInstanceID;
	}

	// Builds a centered aura-ring (torus) mesh. The ring lies in the local Y-Z plane so
	// the moving DOT shot transports it head-on along +X. Major angle sweeps the ring
	// (UV U); minor angle sweeps the tube cross-section (UV V). The silhouette is a
	// readable circular aura, deliberately NOT the AOE radial crescent, the Pierce
	// forward vertical lane, nor the Bounce horizontal slash.
	UStaticMesh* T66BuildDOTRingMesh(UMaterialInterface* RingMaterial)
	{
		const FString DOTMeshPackagePath = T66ResolveHero1AxeDOTPath(T66Hero1AxeDOTMeshPackagePath);
		UPackage* Package = CreatePackage(*DOTMeshPackagePath);
		if (!Package)
		{
			return nullptr;
		}
		Package->FullyLoad();

		UStaticMesh* RingMesh = FindObject<UStaticMesh>(Package, T66Hero1AxeDOTMeshObjectName);
		if (!RingMesh)
		{
			RingMesh = NewObject<UStaticMesh>(
				Package,
				T66Hero1AxeDOTMeshObjectName,
				RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(RingMesh);
		}
		if (!RingMesh)
		{
			return nullptr;
		}

		RingMesh->Modify();
		RingMesh->bAllowCPUAccess = true;
		RingMesh->GetStaticMaterials().Reset();
		RingMesh->GetStaticMaterials().Add(FStaticMaterial(RingMaterial, TEXT("DOTRing")));
		RingMesh->SetLightMapResolution(4);
		RingMesh->SetLightingGuid();

		FMeshDescription MeshDescription;
		FStaticMeshAttributes Attributes(MeshDescription);
		Attributes.Register();

		TVertexAttributesRef<FVector3f> Positions = Attributes.GetVertexPositions();
		TVertexInstanceAttributesRef<FVector2f> UVs = Attributes.GetVertexInstanceUVs();
		UVs.SetNumChannels(1);
		TPolygonGroupAttributesRef<FName> MaterialSlotNames = Attributes.GetPolygonGroupMaterialSlotNames();

		const FPolygonGroupID PolygonGroupID = MeshDescription.CreatePolygonGroup();
		MaterialSlotNames[PolygonGroupID] = TEXT("DOTRingProfile");

		constexpr int32 MajorSegments = 48;
		constexpr int32 MinorSegments = 12;
		constexpr float Tau = 2.0f * UE_PI;
		// Major radius of the ring center circle and tube radius of the lens cross-
		// section, in Unreal centimeters. Major 66 + tube 12 -> ring extent ~78 along
		// Y/Z, matching the Hero1Axe_DOT_Base BaseVisualRadius=80 binding convention.
		constexpr float MajorRadius = 66.0f;
		constexpr float TubeRadius = 12.0f;

		// Grid of shared vertices: (MajorSegments+1) x (MinorSegments+1) with duplicated
		// seam rows/columns so the UV wrap is clean.
		TArray<FVertexID> GridVertices;
		GridVertices.Reserve((MajorSegments + 1) * (MinorSegments + 1));
		for (int32 MajorIndex = 0; MajorIndex <= MajorSegments; ++MajorIndex)
		{
			const float Theta = Tau * static_cast<float>(MajorIndex) / static_cast<float>(MajorSegments);
			const float CosTheta = FMath::Cos(Theta);
			const float SinTheta = FMath::Sin(Theta);
			for (int32 MinorIndex = 0; MinorIndex <= MinorSegments; ++MinorIndex)
			{
				const float Phi = Tau * static_cast<float>(MinorIndex) / static_cast<float>(MinorSegments);
				const float CosPhi = FMath::Cos(Phi);
				const float SinPhi = FMath::Sin(Phi);
				const float RingRadius = MajorRadius + TubeRadius * CosPhi;
				// Ring in Y-Z plane; tube thickness pushes along +/-X (SinPhi) and along the
				// radial direction (CosPhi) within the Y-Z plane.
				const FVector3f Position(
					TubeRadius * SinPhi,
					RingRadius * CosTheta,
					RingRadius * SinTheta);
				const FVertexID VertexID = MeshDescription.CreateVertex();
				Positions[VertexID] = Position;
				GridVertices.Add(VertexID);
			}
		}

		auto GridIndex = [MinorSegments](const int32 MajorIndex, const int32 MinorIndex)
		{
			return MajorIndex * (MinorSegments + 1) + MinorIndex;
		};

		auto AddQuad = [&MeshDescription, &UVs, PolygonGroupID](
			const FVertexID A, const FVector2f UvA,
			const FVertexID B, const FVector2f UvB,
			const FVertexID C, const FVector2f UvC,
			const FVertexID D, const FVector2f UvD)
		{
			TArray<FVertexInstanceID> FirstTriangle;
			FirstTriangle.Reserve(3);
			FirstTriangle.Add(T66CreateDOTVertexInstance(MeshDescription, UVs, A, UvA));
			FirstTriangle.Add(T66CreateDOTVertexInstance(MeshDescription, UVs, B, UvB));
			FirstTriangle.Add(T66CreateDOTVertexInstance(MeshDescription, UVs, C, UvC));
			MeshDescription.CreatePolygon(PolygonGroupID, FirstTriangle);

			TArray<FVertexInstanceID> SecondTriangle;
			SecondTriangle.Reserve(3);
			SecondTriangle.Add(T66CreateDOTVertexInstance(MeshDescription, UVs, A, UvA));
			SecondTriangle.Add(T66CreateDOTVertexInstance(MeshDescription, UVs, C, UvC));
			SecondTriangle.Add(T66CreateDOTVertexInstance(MeshDescription, UVs, D, UvD));
			MeshDescription.CreatePolygon(PolygonGroupID, SecondTriangle);
		};

		for (int32 MajorIndex = 0; MajorIndex < MajorSegments; ++MajorIndex)
		{
			const float U0 = static_cast<float>(MajorIndex) / static_cast<float>(MajorSegments);
			const float U1 = static_cast<float>(MajorIndex + 1) / static_cast<float>(MajorSegments);
			for (int32 MinorIndex = 0; MinorIndex < MinorSegments; ++MinorIndex)
			{
				const float V0 = static_cast<float>(MinorIndex) / static_cast<float>(MinorSegments);
				const float V1 = static_cast<float>(MinorIndex + 1) / static_cast<float>(MinorSegments);

				const FVertexID A = GridVertices[GridIndex(MajorIndex, MinorIndex)];
				const FVertexID B = GridVertices[GridIndex(MajorIndex + 1, MinorIndex)];
				const FVertexID C = GridVertices[GridIndex(MajorIndex + 1, MinorIndex + 1)];
				const FVertexID D = GridVertices[GridIndex(MajorIndex, MinorIndex + 1)];

				AddQuad(
					A, FVector2f(U0, V0),
					B, FVector2f(U1, V0),
					C, FVector2f(U1, V1),
					D, FVector2f(U0, V1));
			}
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
		if (!RingMesh->BuildFromMeshDescriptions({ &MeshDescription }, BuildParams))
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Failed to build DOT ring mesh buffers."));
			return nullptr;
		}

		const FStaticMeshRenderData* RenderData = RingMesh->GetRenderData();
		if (!RenderData || !RenderData->LODResources.IsValidIndex(0))
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] DOT ring mesh has no render data after build."));
			return nullptr;
		}

		const FStaticMeshLODResources& LODResources = RenderData->LODResources[0];
		const int32 VertexCount = static_cast<int32>(LODResources.GetNumVertices());
		const int32 IndexCount = LODResources.IndexBuffer.GetNumIndices();
		const int32 SectionCount = LODResources.Sections.Num();
		UE_LOG(LogTemp, Display, TEXT("[Hero1AxeDOTVFX] DOT ring mesh render data: vertices=%d indices=%d sections=%d"),
			VertexCount,
			IndexCount,
			SectionCount);
		if (VertexCount <= 0 || IndexCount <= 0 || SectionCount <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] DOT ring mesh built with no drawable render buffers."));
			return nullptr;
		}

		RingMesh->MarkPackageDirty();
		return RingMesh;
	}
}

UT66Hero1AxeDOTVFXCommandlet::UT66Hero1AxeDOTVFXCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UT66Hero1AxeDOTVFXCommandlet::Main(const FString& Params)
{
	const bool bProductionPaths = T66UseHero1AxeDOTProductionPaths();
	const float DevSlowFactor = T66GetDOTDevSlowFactor();
	const FString DOTNiagaraPath = T66ResolveHero1AxeDOTPath(T66Hero1AxeDOTNiagaraPath);
	UE_LOG(LogTemp, Display, TEXT("[Hero1AxeDOTVFX] Building aura-ring carrier and moving-shot Niagara mesh renderer. ProductionPaths=%s DevSlowFactor=%.2f"), bProductionPaths ? TEXT("true") : TEXT("false"), DevSlowFactor);

#if WITH_EDITOR
	UNiagaraSystem* RingSystem = T66CreateDOTNiagaraSystem(DevSlowFactor);
#else
	UNiagaraSystem* RingSystem = LoadObject<UNiagaraSystem>(nullptr, *DOTNiagaraPath);
#endif
	if (!RingSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Missing or failed to create Niagara system %s"), *DOTNiagaraPath);
		return 1;
	}

	TMap<FString, UMaterialInterface*> DOTLayerMaterials;
	UMaterialInterface* BodyMaterial = nullptr;
	for (const FT66DOTLayerConfig& Config : T66Hero1AxeDOTLayerConfigs)
	{
		const FString MaterialPath = T66ResolveHero1AxeDOTPath(Config.MaterialPath);
		UMaterialInterface* LayerMaterial = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
		if (!LayerMaterial)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Missing shared slash layer material %s"), *MaterialPath);
			return 1;
		}
		DOTLayerMaterials.Add(Config.EmitterName, LayerMaterial);
		if (FCString::Strcmp(Config.EmitterName, TEXT("Emitter_AxeDOTRing_Body")) == 0)
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
			T66SaveDOTAsset(ConcreteMaterial);
		}
	}
	if (!BodyMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Failed to identify DOT body layer material."));
		return 1;
	}

	UStaticMesh* RingMesh = T66BuildDOTRingMesh(BodyMaterial);
	if (!RingMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Failed to create DOT ring mesh asset."));
		return 1;
	}
	if (!T66SaveDOTAsset(RingMesh))
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Failed to save %s"), *RingMesh->GetPathName());
		return 1;
	}

	int32 EmitterCount = 0;
	int32 BoundMeshRendererCount = 0;
	RingSystem->Modify();
	RingSystem->SetFixedBounds(T66Hero1AxeDOTLocalBounds);

	for (FNiagaraEmitterHandle& EmitterHandle : RingSystem->GetEmitterHandles())
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
		const FT66DOTLayerConfig* Config = T66FindDOTLayerConfig(EmitterName);

		Emitter->Modify();
		EmitterData->bLocalSpace = true;
		EmitterData->CalculateBoundsMode = ENiagaraEmitterCalculateBoundMode::Fixed;
		EmitterData->FixedBounds = T66Hero1AxeDOTLocalBounds;

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
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Unexpected emitter name %s"), *EmitterName);
			return 1;
		}

		UMaterialInterface* const* LayerMaterial = DOTLayerMaterials.Find(EmitterName);
		if (!LayerMaterial || !(*LayerMaterial))
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Missing material binding for emitter %s"), *EmitterName);
			return 1;
		}

		UNiagaraMeshRendererProperties* MeshRenderer =
			NewObject<UNiagaraMeshRendererProperties>(Emitter, NAME_None, RF_Transactional);
		if (!MeshRenderer)
		{
			UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Failed to allocate mesh renderer for emitter %s."), *EmitterName);
			return 1;
		}

		MeshRenderer->Modify();
		FNiagaraMeshRendererMeshProperties MeshProperties;
		MeshProperties.Mesh = RingMesh;
		MeshProperties.Scale = Config->RendererScale;
		MeshProperties.Rotation = FRotator::ZeroRotator;
		MeshProperties.PivotOffset = FVector::ZeroVector;
		MeshRenderer->Meshes.Reset();
		MeshRenderer->Meshes.Add(MeshProperties);
		MeshRenderer->SourceMode = ENiagaraRendererSourceDataMode::Particles;
		MeshRenderer->FacingMode = ENiagaraMeshFacingMode::Default;
		MeshRenderer->bOverrideMaterials = true;
		MeshRenderer->OverrideMaterials.Reset();
		FNiagaraMeshMaterialOverride RingMaterialOverride;
		RingMaterialOverride.ExplicitMat = *LayerMaterial;
		MeshRenderer->OverrideMaterials.Add(RingMaterialOverride);
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
			TEXT("[Hero1AxeDOTVFX] Bound DOT layer emitter=%s rendererScale=%s material=%s"),
			Config->EmitterName,
			*Config->RendererScale.ToString(),
			*(*LayerMaterial)->GetPathName());
	}

	if (EmitterCount <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Niagara system has no editable emitters."));
		return 1;
	}
	if (BoundMeshRendererCount != UE_ARRAY_COUNT(T66Hero1AxeDOTLayerConfigs))
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Bound %d of %d DOT layer emitters to mesh renderers."),
			BoundMeshRendererCount,
			UE_ARRAY_COUNT(T66Hero1AxeDOTLayerConfigs));
		return 1;
	}

#if WITH_EDITOR
	RingSystem->PostEditChange();
	RingSystem->RequestCompile(true);
#endif

	if (!T66SaveDOTAsset(RingSystem))
	{
		UE_LOG(LogTemp, Error, TEXT("[Hero1AxeDOTVFX] Failed to save %s"), *RingSystem->GetPathName());
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("[Hero1AxeDOTVFX] Bound %d DOT ring mesh renderer(s) to %s. ProductionPaths=%s DevSlowFactor=%.2f"),
		BoundMeshRendererCount,
		*RingMesh->GetPathName(),
		bProductionPaths ? TEXT("true") : TEXT("false"),
		DevSlowFactor);
	return 0;
}
