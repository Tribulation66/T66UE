// Copyright Tribulation 66. All Rights Reserved.

#include "T66NiagaraIsolationCaptureCommandlet.h"

#include "AdvancedPreviewScene.h"
#include "Camera/CameraTypes.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/StaticMesh.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "FXSystem.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
#include "ImageCore.h"
#include "ImageUtils.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraCommon.h"
#include "NiagaraComponent.h"
#include "NiagaraEmitterInstance.h"
#include "NiagaraSystem.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraSystemInstanceController.h"
#include "NiagaraWorldManager.h"
#include "PreviewScene.h"
#include "DynamicRHI.h"
#include "RHIStats.h"
#include "AssetCompilingManager.h"
#include "ShaderCompiler.h"
#include "RenderingThread.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66NiagaraIsolationCapture, Log, All);

namespace
{
	constexpr int32 T66DefaultResX = 1600;
	constexpr int32 T66DefaultResY = 1200;
	constexpr float T66DefaultOrthoWidth = 1400.0f;
	constexpr float T66DefaultCameraHeight = 1200.0f;
	constexpr float T66DefaultActualTime = 0.42f;
	constexpr float T66DefaultSeekDelta = 1.0f / 60.0f;
	constexpr int32 T66DefaultWarmupTicks = 4;
	constexpr float T66DefaultWarmupTickSeconds = 1.0f / 60.0f;
	constexpr double T66DefaultNonBlackThreshold = 0.002;

	const TCHAR* T66DefaultSystemPath = TEXT("/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash");
	const TCHAR* T66DefaultPreviewTimes = TEXT("0.06;0.18;0.30;0.42;0.54;0.66");

	struct FT66CaptureConfig
	{
		FString SystemPath = T66DefaultSystemPath;
		FString OutputDir;
		FString TargetPath;
		int32 ResX = T66DefaultResX;
		int32 ResY = T66DefaultResY;
		float OrthoWidth = T66DefaultOrthoWidth;
		float CameraHeight = T66DefaultCameraHeight;
		float CameraX = 0.0f;
		float CameraY = 0.0f;
		float CameraPitch = -90.0f;
		float CameraYaw = 0.0f;
		float CameraRoll = 0.0f;
		float ActualTimeSeconds = T66DefaultActualTime;
		float SeekDeltaSeconds = T66DefaultSeekDelta;
		int32 WarmupTicks = T66DefaultWarmupTicks;
		float WarmupTickSeconds = T66DefaultWarmupTickSeconds;
		double NonBlackPixelRatioThreshold = T66DefaultNonBlackThreshold;
		bool bDebugPrimitive = false;
		int32 ArrayProofCount = 0;
		FString ArrayProofParameter = TEXT("User.TravelerPositions");
		float ArrayProofSpacing = 18.0f;
		TArray<float> PreviewTimesSeconds;
	};

	struct FT66RenderDiagnostics
	{
		bool bComponentActive = false;
		bool bComponentRegistered = false;
		bool bComponentVisible = false;
		bool bSystemReady = false;
		bool bControllerValid = false;
		bool bSystemInstancePresent = false;
		bool bSystemInstanceReady = false;
		bool bSystemInstanceHasGpuEmitters = false;
		int32 SystemEmitterHandleCount = 0;
		int32 RuntimeEmitterCount = 0;
		int32 RuntimeTotalParticles = 0;
		float DesiredAge = 0.0f;
		FVector BoundsOrigin = FVector::ZeroVector;
		FVector BoundsExtent = FVector::ZeroVector;
		FVector LocalBoundsMin = FVector::ZeroVector;
		FVector LocalBoundsMax = FVector::ZeroVector;
		bool bLocalBoundsValid = false;
		FString RequestedExecutionState;
		FString ActualExecutionState;
		TArray<FString> SystemEmitterSummaries;
		TArray<FString> RuntimeEmitterSummaries;
	};

	struct FT66ArrayProofDiagnostics
	{
		bool bEnabled = false;
		FString ParameterName;
		int32 RequestedCount = 0;
		int32 UploadedCount = 0;
		int32 ReadbackCount = 0;
		double LastUploadSeconds = 0.0;
		FString FirstPosition;
		FString LastPosition;
	};

	struct FT66CostDiagnostics
	{
		double CaptureWallSeconds = 0.0;
		double EstimatedWallFps = 0.0;
		double GpuFrameMs = -1.0;
		double EstimatedGpuFps = 0.0;
		int32 DrawCallsRHI = -1;
	};

	struct FT66FrameCapture
	{
		float RequestedAgeSeconds = 0.0f;
		float EffectiveAgeSeconds = 0.0f;
		FImage Image;
		double NonBlackPixelRatio = 0.0;
		bool bReadbackSuccess = false;
		bool bSaveSuccess = false;
		FString OutputPath;
	};

	FString T66Quote(const FString& Value)
	{
		return FString::Printf(TEXT("\"%s\""), *Value.Replace(TEXT("\""), TEXT("\\\"")));
	}

	FString T66ToObjectPath(const FString& InPath, FString& OutPackagePath)
	{
		FString Path = InPath;
		Path.TrimStartAndEndInline();
		Path.TrimQuotesInline();

		FString ObjectPath = Path;
		const int32 DotIndex = Path.Find(TEXT("."), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		if (DotIndex == INDEX_NONE)
		{
			OutPackagePath = Path;
			const FString AssetName = FPackageName::GetLongPackageAssetName(Path);
			ObjectPath = FString::Printf(TEXT("%s.%s"), *Path, *AssetName);
		}
		else
		{
			OutPackagePath = Path.Left(DotIndex);
		}
		return ObjectPath;
	}

	bool T66PackageFileExists(const FString& PackagePath)
	{
		if (!FPackageName::IsValidLongPackageName(PackagePath))
		{
			return false;
		}

		const FString AssetFilename = FPackageName::LongPackageNameToFilename(
			PackagePath,
			FPackageName::GetAssetPackageExtension());
		return FPaths::FileExists(AssetFilename);
	}

	void T66ParsePreviewTimes(const FString& Value, TArray<float>& OutTimes)
	{
		OutTimes.Reset();
		TArray<FString> Parts;
		FString Normalized = Value;
		Normalized.ReplaceInline(TEXT(","), TEXT(";"));
		Normalized.ReplaceInline(TEXT("|"), TEXT(";"));
		Normalized.ParseIntoArray(Parts, TEXT(";"), true);
		for (FString Part : Parts)
		{
			Part.TrimStartAndEndInline();
			if (Part.IsEmpty())
			{
				continue;
			}

			OutTimes.Add(FMath::Max(0.0f, FCString::Atof(*Part)));
		}
	}

	FT66CaptureConfig T66ReadConfig()
	{
		FT66CaptureConfig Config;
		FString PreviewTimesString = T66DefaultPreviewTimes;

		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationSystem="), Config.SystemPath);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationOutput="), Config.OutputDir);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationTarget="), Config.TargetPath);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationResX="), Config.ResX);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationResY="), Config.ResY);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationOrthoWidth="), Config.OrthoWidth);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationCameraHeight="), Config.CameraHeight);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationCameraX="), Config.CameraX);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationCameraY="), Config.CameraY);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationCameraPitch="), Config.CameraPitch);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationCameraYaw="), Config.CameraYaw);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationCameraRoll="), Config.CameraRoll);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationActualTime="), Config.ActualTimeSeconds);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationSeekDelta="), Config.SeekDeltaSeconds);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationWarmupTicks="), Config.WarmupTicks);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationWarmupTickSeconds="), Config.WarmupTickSeconds);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationNonBlackThreshold="), Config.NonBlackPixelRatioThreshold);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationPreviewTimes="), PreviewTimesString);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationArrayProofCount="), Config.ArrayProofCount);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationArrayProofParameter="), Config.ArrayProofParameter);
		FParse::Value(FCommandLine::Get(), TEXT("T66NiagaraIsolationArrayProofSpacing="), Config.ArrayProofSpacing);
		Config.bDebugPrimitive = FParse::Param(FCommandLine::Get(), TEXT("T66NiagaraIsolationDebugPrimitive"));

		T66ParsePreviewTimes(PreviewTimesString, Config.PreviewTimesSeconds);
		if (Config.PreviewTimesSeconds.Num() == 0)
		{
			T66ParsePreviewTimes(T66DefaultPreviewTimes, Config.PreviewTimesSeconds);
		}

		Config.ResX = FMath::Clamp(Config.ResX, 64, 8192);
		Config.ResY = FMath::Clamp(Config.ResY, 64, 8192);
		Config.OrthoWidth = FMath::Max(1.0f, Config.OrthoWidth);
		Config.CameraHeight = FMath::Max(1.0f, Config.CameraHeight);
		Config.ActualTimeSeconds = FMath::Max(0.0f, Config.ActualTimeSeconds);
		Config.SeekDeltaSeconds = FMath::Max(1.0f / 240.0f, Config.SeekDeltaSeconds);
		Config.WarmupTicks = FMath::Clamp(Config.WarmupTicks, 0, 120);
		Config.WarmupTickSeconds = FMath::Max(1.0f / 240.0f, Config.WarmupTickSeconds);
		Config.NonBlackPixelRatioThreshold = FMath::Max(0.0, Config.NonBlackPixelRatioThreshold);
		Config.ArrayProofCount = FMath::Clamp(Config.ArrayProofCount, 0, 100000);
		Config.ArrayProofParameter.TrimStartAndEndInline();
		Config.ArrayProofParameter.TrimQuotesInline();
		if (Config.ArrayProofParameter.IsEmpty())
		{
			Config.ArrayProofParameter = TEXT("User.TravelerPositions");
		}
		Config.ArrayProofSpacing = FMath::Clamp(Config.ArrayProofSpacing, 1.0f, 1000.0f);

		if (Config.OutputDir.IsEmpty())
		{
			const FString Stamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
			Config.OutputDir = FPaths::ProjectSavedDir() / TEXT("VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation") / Stamp;
		}
		Config.OutputDir = FPaths::ConvertRelativePathToFull(Config.OutputDir);
		Config.TargetPath.TrimStartAndEndInline();
		Config.TargetPath.TrimQuotesInline();
		return Config;
	}

	TArray<FVector> T66BuildArrayProofPositions(const int32 Count, const float Spacing)
	{
		TArray<FVector> Positions;
		Positions.Reserve(Count);
		if (Count <= 0)
		{
			return Positions;
		}

		const int32 Columns = FMath::Max(1, FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Count) * 2.0f)));
		const int32 Rows = FMath::Max(1, FMath::CeilToInt(static_cast<float>(Count) / static_cast<float>(Columns)));
		const float HalfWidth = static_cast<float>(Columns - 1) * Spacing * 0.5f;
		const float HalfHeight = static_cast<float>(Rows - 1) * Spacing * 0.5f;
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const int32 Column = Index % Columns;
			const int32 Row = Index / Columns;
			const float X = static_cast<float>(Column) * Spacing - HalfWidth;
			const float Y = static_cast<float>(Row) * Spacing - HalfHeight;
			const float Z = FMath::Sin(static_cast<float>(Index) * 0.173f) * 2.0f;
			Positions.Add(FVector(X, Y, Z));
		}
		return Positions;
	}

	void T66ApplyArrayProofOverride(
		UNiagaraComponent* NiagaraComponent,
		const FT66CaptureConfig& Config,
		FT66ArrayProofDiagnostics& Diagnostics)
	{
		if (!NiagaraComponent || Config.ArrayProofCount <= 0)
		{
			return;
		}

		Diagnostics.bEnabled = true;
		Diagnostics.ParameterName = Config.ArrayProofParameter;
		Diagnostics.RequestedCount = Config.ArrayProofCount;
		const TArray<FVector> Positions = T66BuildArrayProofPositions(Config.ArrayProofCount, Config.ArrayProofSpacing);
		const double StartSeconds = FPlatformTime::Seconds();
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
			NiagaraComponent,
			FName(*Config.ArrayProofParameter),
			Positions);
		Diagnostics.LastUploadSeconds = FPlatformTime::Seconds() - StartSeconds;
		Diagnostics.UploadedCount = Positions.Num();

		const TArray<FVector> Readback = UNiagaraDataInterfaceArrayFunctionLibrary::GetNiagaraArrayVector(
			NiagaraComponent,
			FName(*Config.ArrayProofParameter));
		Diagnostics.ReadbackCount = Readback.Num();
		if (Readback.Num() > 0)
		{
			Diagnostics.FirstPosition = Readback[0].ToCompactString();
			Diagnostics.LastPosition = Readback.Last().ToCompactString();
		}
	}

	FT66CostDiagnostics T66CollectCostDiagnostics(const double CaptureWallSeconds)
	{
		FT66CostDiagnostics Diagnostics;
		Diagnostics.CaptureWallSeconds = CaptureWallSeconds;
		if (CaptureWallSeconds > 0.0)
		{
			Diagnostics.EstimatedWallFps = 1.0 / CaptureWallSeconds;
		}

		const uint32 GpuCycles = RHIGetGPUFrameCycles(0);
		if (GpuCycles > 0)
		{
			Diagnostics.GpuFrameMs = FPlatformTime::ToMilliseconds(GpuCycles);
			if (Diagnostics.GpuFrameMs > 0.0)
			{
				Diagnostics.EstimatedGpuFps = 1000.0 / Diagnostics.GpuFrameMs;
			}
		}

		Diagnostics.DrawCallsRHI = GNumDrawCallsRHI[0];
		return Diagnostics;
	}

	template<typename EnumType>
	FString T66EnumToString(const EnumType Value)
	{
		if (const UEnum* Enum = StaticEnum<EnumType>())
		{
			return Enum->GetNameStringByValue(static_cast<int64>(Value));
		}
		return FString::FromInt(static_cast<int32>(Value));
	}

	FT66RenderDiagnostics T66CollectRenderDiagnostics(UNiagaraComponent* NiagaraComponent)
	{
		FT66RenderDiagnostics Diagnostics;
		if (!NiagaraComponent)
		{
			return Diagnostics;
		}

		Diagnostics.bComponentActive = NiagaraComponent->IsActive();
		Diagnostics.bComponentRegistered = NiagaraComponent->IsRegistered();
		Diagnostics.bComponentVisible = NiagaraComponent->IsVisible();
		Diagnostics.bSystemReady = NiagaraComponent->GetAsset() ? NiagaraComponent->GetAsset()->IsReadyToRun() : false;
		Diagnostics.DesiredAge = NiagaraComponent->GetDesiredAge();
		Diagnostics.BoundsOrigin = NiagaraComponent->Bounds.Origin;
		Diagnostics.BoundsExtent = NiagaraComponent->Bounds.BoxExtent;

		if (const UNiagaraSystem* System = NiagaraComponent->GetAsset())
		{
			const TArray<FNiagaraEmitterHandle>& EmitterHandles = System->GetEmitterHandles();
			Diagnostics.SystemEmitterHandleCount = EmitterHandles.Num();
			for (int32 EmitterIndex = 0; EmitterIndex < EmitterHandles.Num(); ++EmitterIndex)
			{
				const FNiagaraEmitterHandle& EmitterHandle = EmitterHandles[EmitterIndex];
				const FVersionedNiagaraEmitterData* EmitterData = EmitterHandle.GetEmitterData();
				const int32 RendererCount = EmitterData ? EmitterData->GetRenderers().Num() : 0;
				Diagnostics.SystemEmitterSummaries.Add(FString::Printf(
					TEXT("index=%d name=%s valid=%s enabled=%s data=%s dataReady=%s simTarget=%s rendererCount=%d"),
					EmitterIndex,
					*EmitterHandle.GetName().ToString(),
					EmitterHandle.IsValid() ? TEXT("true") : TEXT("false"),
					EmitterHandle.GetIsEnabled() ? TEXT("true") : TEXT("false"),
					EmitterData ? TEXT("present") : TEXT("null"),
					(EmitterData && EmitterData->IsReadyToRun()) ? TEXT("true") : TEXT("false"),
					EmitterData ? *T66EnumToString(EmitterData->SimTarget) : TEXT("unknown"),
					RendererCount));
			}
		}

		const FNiagaraSystemInstanceControllerPtr Controller = NiagaraComponent->GetSystemInstanceController();
		Diagnostics.bControllerValid = Controller.IsValid() && Controller->IsValid();
		if (Controller.IsValid())
		{
			const FBox& LocalBounds = Controller->GetLocalBounds();
			if (LocalBounds.IsValid)
			{
				Diagnostics.bLocalBoundsValid = true;
				Diagnostics.LocalBoundsMin = LocalBounds.Min;
				Diagnostics.LocalBoundsMax = LocalBounds.Max;
			}
		}
		FNiagaraSystemInstance* SystemInstance = Diagnostics.bControllerValid ? Controller->GetSystemInstance_Unsafe() : nullptr;
		if (SystemInstance)
		{
			Diagnostics.bSystemInstancePresent = true;
			Diagnostics.bSystemInstanceReady = SystemInstance->IsReadyToRun();
			Diagnostics.bSystemInstanceHasGpuEmitters = SystemInstance->HasGPUEmitters();
			Diagnostics.RequestedExecutionState = T66EnumToString(SystemInstance->GetRequestedExecutionState());
			Diagnostics.ActualExecutionState = T66EnumToString(SystemInstance->GetActualExecutionState());
			const TConstArrayView<FNiagaraEmitterInstanceRef> EmitterInstances = SystemInstance->GetEmitters();
			Diagnostics.RuntimeEmitterCount = EmitterInstances.Num();
			for (int32 EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); ++EmitterIndex)
			{
				const FNiagaraEmitterInstance& EmitterInstance = EmitterInstances[EmitterIndex].Get();
				int32 EnabledRendererCount = 0;
				EmitterInstance.ForEachEnabledRenderer(
					[&EnabledRendererCount](const UNiagaraRendererProperties*)
					{
						++EnabledRendererCount;
					});
				Diagnostics.RuntimeTotalParticles += EmitterInstance.GetNumParticles();
				Diagnostics.RuntimeEmitterSummaries.Add(FString::Printf(
					TEXT("index=%d name=%s simTarget=%s particles=%d execState=%s ready=%s active=%s rendererCount=%d bounds=%s"),
					EmitterIndex,
					*EmitterInstance.GetEmitterHandle().GetName().ToString(),
					*T66EnumToString(EmitterInstance.GetSimTarget()),
					EmitterInstance.GetNumParticles(),
					*T66EnumToString(EmitterInstance.GetExecutionState()),
					EmitterInstance.IsReadyToRun() ? TEXT("true") : TEXT("false"),
					EmitterInstance.IsActive() ? TEXT("true") : TEXT("false"),
					EnabledRendererCount,
					*EmitterInstance.GetBounds().ToString()));
			}
		}

		return Diagnostics;
	}

	void T66FlushCaptureWorld(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		UMaterialInterface::SubmitRemainingJobsForWorld(World);
		FAssetCompilingManager::Get().FinishAllCompilation();
		FAssetCompilingManager::Get().ProcessAsyncTasks();
		FlushRenderingCommands();

		World->SendAllEndOfFrameUpdates();
		if (FNiagaraWorldManager* NiagaraWorldManager = FNiagaraWorldManager::Get(World))
		{
			NiagaraWorldManager->FlushComputeAndDeferredQueues(false);
		}
		FlushRenderingCommands();
	}

	FImage T66ToBGRA8(const FImage& Image)
	{
		if (Image.Format == ERawImageFormat::BGRA8 && Image.GammaSpace == EGammaSpace::sRGB)
		{
			FImage Copy;
			Image.CopyTo(Copy, ERawImageFormat::BGRA8, EGammaSpace::sRGB);
			return Copy;
		}

		FImage Converted;
		Image.CopyTo(Converted, ERawImageFormat::BGRA8, EGammaSpace::sRGB);
		return Converted;
	}

	double T66ComputeNonBlackRatio(const FImage& Image)
	{
		FImage Bgra = T66ToBGRA8(Image);
		const TArrayView64<const FColor> Pixels = Bgra.AsBGRA8();
		if (Pixels.Num() == 0)
		{
			return 0.0;
		}

		int64 NonBlackCount = 0;
		for (const FColor& Pixel : Pixels)
		{
			if (Pixel.R > 4 || Pixel.G > 4 || Pixel.B > 4)
			{
				++NonBlackCount;
			}
		}

		return static_cast<double>(NonBlackCount) / static_cast<double>(Pixels.Num());
	}

	bool T66SaveImage(const FString& Path, const FImage& Image)
	{
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
		return FImageUtils::SaveImageByExtension(*Path, Image);
	}

	bool T66MakeContactSheet(const TArray<FT66FrameCapture>& Frames, int32 ResX, int32 ResY, FImage& OutSheet)
	{
		if (Frames.Num() == 0)
		{
			return false;
		}

		const int32 Columns = FMath::Min(3, Frames.Num());
		const int32 Rows = FMath::CeilToInt(static_cast<float>(Frames.Num()) / static_cast<float>(Columns));
		OutSheet.Init(ResX * Columns, ResY * Rows, ERawImageFormat::BGRA8, EGammaSpace::sRGB);

		TArrayView64<FColor> SheetPixels = OutSheet.AsBGRA8();
		for (FColor& Pixel : SheetPixels)
		{
			Pixel = FColor::Black;
		}

		for (int32 FrameIndex = 0; FrameIndex < Frames.Num(); ++FrameIndex)
		{
			if (!Frames[FrameIndex].bReadbackSuccess)
			{
				continue;
			}

			FImage FrameImage = T66ToBGRA8(Frames[FrameIndex].Image);
			if (FrameImage.SizeX != ResX || FrameImage.SizeY != ResY)
			{
				continue;
			}

			const TArrayView64<const FColor> FramePixels = FrameImage.AsBGRA8();
			const int32 Column = FrameIndex % Columns;
			const int32 Row = FrameIndex / Columns;
			for (int32 Y = 0; Y < ResY; ++Y)
			{
				const int64 SheetBase = static_cast<int64>((Row * ResY) + Y) * OutSheet.SizeX + (Column * ResX);
				const int64 FrameBase = static_cast<int64>(Y) * ResX;
				for (int32 X = 0; X < ResX; ++X)
				{
					SheetPixels[SheetBase + X] = FramePixels[FrameBase + X];
				}
			}
		}

		return true;
	}

	bool T66WriteTextFile(const FString& Path, const FString& Contents)
	{
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
		return FFileHelper::SaveStringToFile(Contents, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	bool T66WriteManifest(
		const FT66CaptureConfig& Config,
		const FString& ObjectPath,
		const FString& PackagePath,
		bool bAssetFilePresent,
		bool bAssetLoadSuccess,
		bool bRenderSuccess,
		const FString& FailureMode,
		double ActualNonBlackRatio,
		bool bTargetPresent,
		const TArray<FT66FrameCapture>& Frames,
		const FT66RenderDiagnostics& Diagnostics,
		const FT66ArrayProofDiagnostics& ArrayDiagnostics,
		const FT66CostDiagnostics& CostDiagnostics)
	{
		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
		Root->SetStringField(TEXT("tool"), TEXT("T66NiagaraIsolationCapture"));
		Root->SetStringField(TEXT("system_path_input"), Config.SystemPath);
		Root->SetStringField(TEXT("system_object_path"), ObjectPath);
		Root->SetStringField(TEXT("system_package_path"), PackagePath);
		Root->SetStringField(TEXT("output_dir"), Config.OutputDir);
		Root->SetStringField(TEXT("command_line"), FCommandLine::Get());
		Root->SetStringField(TEXT("required_executable"), TEXT("UnrealEditor.exe"));
		Root->SetBoolField(TEXT("allow_commandlet_rendering_required"), true);
		Root->SetBoolField(TEXT("null_rhi_allowed"), false);
		Root->SetBoolField(TEXT("asset_file_present"), bAssetFilePresent);
		Root->SetBoolField(TEXT("asset_load_success"), bAssetLoadSuccess);
		Root->SetBoolField(TEXT("render_success"), bRenderSuccess);
		Root->SetStringField(TEXT("failure_mode"), FailureMode);
		Root->SetStringField(TEXT("view_mode"), TEXT("top-down orthographic"));
		Root->SetStringField(TEXT("background"), TEXT("black"));
		Root->SetNumberField(TEXT("res_x"), Config.ResX);
		Root->SetNumberField(TEXT("res_y"), Config.ResY);
		Root->SetNumberField(TEXT("ortho_width"), Config.OrthoWidth);
		Root->SetNumberField(TEXT("camera_height"), Config.CameraHeight);
		Root->SetNumberField(TEXT("camera_x"), Config.CameraX);
		Root->SetNumberField(TEXT("camera_y"), Config.CameraY);
		Root->SetNumberField(TEXT("camera_pitch"), Config.CameraPitch);
		Root->SetNumberField(TEXT("camera_yaw"), Config.CameraYaw);
		Root->SetNumberField(TEXT("camera_roll"), Config.CameraRoll);
		Root->SetNumberField(TEXT("actual_time_seconds_requested"), Config.ActualTimeSeconds);
		Root->SetNumberField(TEXT("actual_time_seconds_effective"), Config.ActualTimeSeconds);
		Root->SetNumberField(TEXT("seek_delta_seconds"), Config.SeekDeltaSeconds);
		Root->SetNumberField(TEXT("warmup_ticks"), Config.WarmupTicks);
		Root->SetNumberField(TEXT("warmup_tick_seconds"), Config.WarmupTickSeconds);
		Root->SetNumberField(TEXT("non_black_pixel_ratio_threshold"), Config.NonBlackPixelRatioThreshold);
		Root->SetNumberField(TEXT("actual_non_black_pixel_ratio"), ActualNonBlackRatio);
		Root->SetBoolField(TEXT("debug_primitive_enabled"), Config.bDebugPrimitive);
		Root->SetBoolField(TEXT("component_active"), Diagnostics.bComponentActive);
		Root->SetBoolField(TEXT("component_registered"), Diagnostics.bComponentRegistered);
		Root->SetBoolField(TEXT("component_visible"), Diagnostics.bComponentVisible);
		Root->SetBoolField(TEXT("system_ready"), Diagnostics.bSystemReady);
		Root->SetBoolField(TEXT("controller_valid"), Diagnostics.bControllerValid);
		Root->SetBoolField(TEXT("system_instance_present"), Diagnostics.bSystemInstancePresent);
		Root->SetBoolField(TEXT("system_instance_ready"), Diagnostics.bSystemInstanceReady);
		Root->SetBoolField(TEXT("system_instance_has_gpu_emitters"), Diagnostics.bSystemInstanceHasGpuEmitters);
		Root->SetNumberField(TEXT("system_emitter_handle_count"), Diagnostics.SystemEmitterHandleCount);
		Root->SetNumberField(TEXT("runtime_emitter_count"), Diagnostics.RuntimeEmitterCount);
		Root->SetNumberField(TEXT("runtime_total_particles"), Diagnostics.RuntimeTotalParticles);
		Root->SetStringField(TEXT("requested_execution_state"), Diagnostics.RequestedExecutionState);
		Root->SetStringField(TEXT("actual_execution_state"), Diagnostics.ActualExecutionState);
		Root->SetNumberField(TEXT("component_desired_age"), Diagnostics.DesiredAge);
		Root->SetStringField(TEXT("component_bounds_origin"), Diagnostics.BoundsOrigin.ToCompactString());
		Root->SetStringField(TEXT("component_bounds_extent"), Diagnostics.BoundsExtent.ToCompactString());
		Root->SetBoolField(TEXT("local_bounds_valid"), Diagnostics.bLocalBoundsValid);
		Root->SetStringField(TEXT("local_bounds_min"), Diagnostics.LocalBoundsMin.ToCompactString());
		Root->SetStringField(TEXT("local_bounds_max"), Diagnostics.LocalBoundsMax.ToCompactString());
		TArray<TSharedPtr<FJsonValue>> SystemEmitterValues;
		for (const FString& Summary : Diagnostics.SystemEmitterSummaries)
		{
			SystemEmitterValues.Add(MakeShared<FJsonValueString>(Summary));
		}
		Root->SetArrayField(TEXT("system_emitters"), SystemEmitterValues);
		TArray<TSharedPtr<FJsonValue>> RuntimeEmitterValues;
		for (const FString& Summary : Diagnostics.RuntimeEmitterSummaries)
		{
			RuntimeEmitterValues.Add(MakeShared<FJsonValueString>(Summary));
		}
		Root->SetArrayField(TEXT("runtime_emitters"), RuntimeEmitterValues);
		Root->SetBoolField(TEXT("target_present"), bTargetPresent);
		Root->SetStringField(TEXT("target_status"), bTargetPresent ? TEXT("present") : TEXT("pending"));
		Root->SetNumberField(TEXT("niagara_components_spawned"), 1);
		Root->SetBoolField(TEXT("single_persistent_niagara_component"), true);
		Root->SetBoolField(TEXT("array_proof_enabled"), ArrayDiagnostics.bEnabled);
		Root->SetStringField(TEXT("array_proof_parameter"), ArrayDiagnostics.ParameterName);
		Root->SetNumberField(TEXT("array_proof_requested_count"), ArrayDiagnostics.RequestedCount);
		Root->SetNumberField(TEXT("array_proof_uploaded_count"), ArrayDiagnostics.UploadedCount);
		Root->SetNumberField(TEXT("array_proof_readback_count"), ArrayDiagnostics.ReadbackCount);
		Root->SetNumberField(TEXT("array_proof_last_upload_seconds"), ArrayDiagnostics.LastUploadSeconds);
		Root->SetStringField(TEXT("array_proof_first_position"), ArrayDiagnostics.FirstPosition);
		Root->SetStringField(TEXT("array_proof_last_position"), ArrayDiagnostics.LastPosition);
		Root->SetNumberField(TEXT("rough_capture_wall_seconds"), CostDiagnostics.CaptureWallSeconds);
		Root->SetNumberField(TEXT("rough_fps_from_wall_time"), CostDiagnostics.EstimatedWallFps);
		Root->SetStringField(TEXT("rough_gpu_frame_time_ms"), CostDiagnostics.GpuFrameMs >= 0.0 ? FString::Printf(TEXT("%.4f"), CostDiagnostics.GpuFrameMs) : TEXT("Unavailable"));
		Root->SetStringField(TEXT("rough_fps_from_gpu_frame_time"), CostDiagnostics.EstimatedGpuFps > 0.0 ? FString::Printf(TEXT("%.2f"), CostDiagnostics.EstimatedGpuFps) : TEXT("Unavailable"));
		Root->SetStringField(TEXT("rough_draw_calls_rhi"), CostDiagnostics.DrawCallsRHI >= 0 ? FString::FromInt(CostDiagnostics.DrawCallsRHI) : TEXT("Unavailable"));
		Root->SetStringField(TEXT("rough_vram"), TEXT("Unavailable"));
		Root->SetStringField(TEXT("rough_thermals"), TEXT("Unavailable"));

		TArray<TSharedPtr<FJsonValue>> TimeValues;
		for (const float Time : Config.PreviewTimesSeconds)
		{
			TimeValues.Add(MakeShared<FJsonValueNumber>(Time));
		}
		Root->SetArrayField(TEXT("preview_times_seconds"), TimeValues);

		TArray<TSharedPtr<FJsonValue>> FrameValues;
		for (const FT66FrameCapture& Frame : Frames)
		{
			TSharedRef<FJsonObject> FrameObject = MakeShared<FJsonObject>();
			FrameObject->SetNumberField(TEXT("requested_age_seconds"), Frame.RequestedAgeSeconds);
			FrameObject->SetNumberField(TEXT("effective_age_seconds"), Frame.EffectiveAgeSeconds);
			FrameObject->SetBoolField(TEXT("readback_success"), Frame.bReadbackSuccess);
			FrameObject->SetBoolField(TEXT("save_success"), Frame.bSaveSuccess);
			FrameObject->SetNumberField(TEXT("non_black_pixel_ratio"), Frame.NonBlackPixelRatio);
			FrameObject->SetStringField(TEXT("output_path"), Frame.OutputPath);
			FrameValues.Add(MakeShared<FJsonValueObject>(FrameObject));
		}
		Root->SetArrayField(TEXT("frames"), FrameValues);

		FString Serialized;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Serialized);
		if (!FJsonSerializer::Serialize(Root, Writer))
		{
			return false;
		}

		return T66WriteTextFile(Config.OutputDir / TEXT("manifest.json"), Serialized);
	}

	void T66WriteMismatchNotes(
		const FT66CaptureConfig& Config,
		bool bTargetPresent,
		bool bRenderSuccess,
		const FString& FailureMode)
	{
		FString Notes;
		Notes += TEXT("# Niagara Isolation Mismatch Notes\n\n");
		Notes += TEXT("Status: diagnostic route output only.\n\n");
		Notes += FString::Printf(TEXT("- Target present: %s\n"), bTargetPresent ? TEXT("YES") : TEXT("NO"));
		Notes += FString::Printf(TEXT("- Render success: %s\n"), bRenderSuccess ? TEXT("YES") : TEXT("NO"));
		Notes += FString::Printf(TEXT("- Failure mode: %s\n"), *FailureMode);
		Notes += TEXT("- Acceptance state: PARTIAL/BLOCKED until same-view target.png exists and target-vs-actual comparison is performed.\n");
		Notes += TEXT("- Manual inspection required: confirm full VFX visibility, black background, no dungeon/hero/enemy geometry, and full active region framed with margin.\n\n");
		if (!bTargetPresent)
		{
			Notes += TEXT("No target.png was provided for this diagnostic run, so this file intentionally does not claim visual match.\n");
		}
		else
		{
			Notes += TEXT("target.png is present. A future pass must compare the target and actual against the AOE mismatch rubric before visual acceptance.\n");
		}
		T66WriteTextFile(Config.OutputDir / TEXT("mismatch_notes.md"), Notes);
	}

	void T66AdvanceForRegistration(UWorld* World, UNiagaraComponent* Component, const FT66CaptureConfig& Config)
	{
		if (!World || !Component)
		{
			return;
		}

		for (int32 TickIndex = 0; TickIndex < Config.WarmupTicks; ++TickIndex)
		{
			World->Tick(LEVELTICK_All, Config.WarmupTickSeconds);
			Component->TickComponent(Config.WarmupTickSeconds, LEVELTICK_All, nullptr);
		}
	}

	bool T66CaptureFrame(
		UWorld* World,
		UNiagaraComponent* NiagaraComponent,
		USceneCaptureComponent2D* CaptureComponent,
		UTextureRenderTarget2D* RenderTarget,
		const FT66CaptureConfig& Config,
		FT66ArrayProofDiagnostics& ArrayDiagnostics,
		float RequestedAge,
		FT66FrameCapture& OutFrame)
	{
		OutFrame.RequestedAgeSeconds = RequestedAge;
		OutFrame.EffectiveAgeSeconds = RequestedAge;
		if (!World || !NiagaraComponent || !CaptureComponent || !RenderTarget)
		{
			return false;
		}

		NiagaraComponent->DeactivateImmediate();
		NiagaraComponent->Activate(true);
		NiagaraComponent->ReinitializeSystem();
		T66ApplyArrayProofOverride(NiagaraComponent, Config, ArrayDiagnostics);
		NiagaraComponent->Activate(true);
		T66AdvanceForRegistration(World, NiagaraComponent, Config);
		NiagaraComponent->SetAgeUpdateMode(ENiagaraAgeUpdateMode::DesiredAge);
		NiagaraComponent->SetCanRenderWhileSeeking(true);
		NiagaraComponent->SetSeekDelta(Config.SeekDeltaSeconds);
		NiagaraComponent->SeekToDesiredAge(RequestedAge);

		World->TimeSeconds = RequestedAge;
		World->UnpausedTimeSeconds = RequestedAge;
		World->RealTimeSeconds = RequestedAge;
		World->DeltaTimeSeconds = Config.SeekDeltaSeconds;
		World->DeltaRealTimeSeconds = Config.SeekDeltaSeconds;
		World->Tick(LEVELTICK_PauseTick, 0.0f);
		NiagaraComponent->TickComponent(Config.SeekDeltaSeconds, LEVELTICK_All, nullptr);
		T66FlushCaptureWorld(World);

		CaptureComponent->CaptureScene();
		FlushRenderingCommands();
#if WITH_EDITOR
		FAssetCompilingManager::Get().FinishAllCompilation();
		if (GShaderCompilingManager)
		{
			GShaderCompilingManager->FinishAllCompilation();
		}
#endif
		CaptureComponent->CaptureScene();
		FlushRenderingCommands();

		FImage Image;
		if (!FImageUtils::GetRenderTargetImage(RenderTarget, Image))
		{
			return false;
		}

		OutFrame.Image = MoveTemp(Image);
		OutFrame.NonBlackPixelRatio = T66ComputeNonBlackRatio(OutFrame.Image);
		OutFrame.bReadbackSuccess = true;
		return true;
	}

	UStaticMeshComponent* T66CreateDebugPrimitive(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		UStaticMesh* DebugMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		UMaterialInterface* DebugMaterial =
			LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (!DebugMesh || !DebugMaterial)
		{
			UE_LOG(LogT66NiagaraIsolationCapture, Warning, TEXT("[T66NiagaraIsolationCapture] Debug primitive asset load failed."));
			return nullptr;
		}

		AActor* DebugActor = World->SpawnActor<AActor>(
			AActor::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator);
		if (!DebugActor)
		{
			return nullptr;
		}

		UStaticMeshComponent* DebugComponent = NewObject<UStaticMeshComponent>(DebugActor, TEXT("T66NiagaraIsolationDebugCube"), RF_Transient);
		if (!DebugComponent)
		{
			return nullptr;
		}

		DebugComponent->SetStaticMesh(DebugMesh);
		DebugComponent->SetMaterial(0, DebugMaterial);
		DebugComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DebugComponent->SetGenerateOverlapEvents(false);
		DebugComponent->CastShadow = false;
		DebugComponent->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.2f));
		DebugActor->SetRootComponent(DebugComponent);
		DebugActor->AddInstanceComponent(DebugComponent);
		DebugComponent->RegisterComponent();
		return DebugComponent;
	}
}

UT66NiagaraIsolationCaptureCommandlet::UT66NiagaraIsolationCaptureCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UT66NiagaraIsolationCaptureCommandlet::Main(const FString& Params)
{
	UE_LOG(LogT66NiagaraIsolationCapture, Display, TEXT("[T66NiagaraIsolationCapture] Starting capture. Params=%s"), *Params);

	const FT66CaptureConfig Config = T66ReadConfig();
	IFileManager::Get().MakeDirectory(*Config.OutputDir, true);

	FString PackagePath;
	const FString ObjectPath = T66ToObjectPath(Config.SystemPath, PackagePath);
	const bool bAssetFilePresent = T66PackageFileExists(PackagePath);

	UNiagaraSystem* NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, *ObjectPath);
	const bool bAssetLoadSuccess = NiagaraSystem != nullptr;
	FString FailureMode = TEXT("none");

	if (!bAssetFilePresent)
	{
		FailureMode = TEXT("asset_file_missing");
	}
	else if (!bAssetLoadSuccess)
	{
		FailureMode = TEXT("asset_load_failed");
	}

	TArray<FT66FrameCapture> Frames;
	double ActualNonBlackRatio = 0.0;
	bool bRenderSuccess = false;
	bool bTargetPresent = false;
	FT66RenderDiagnostics RenderDiagnostics;
	FT66ArrayProofDiagnostics ArrayDiagnostics;
	FT66CostDiagnostics CostDiagnostics;

	if (bAssetLoadSuccess)
	{
#if WITH_EDITOR
		NiagaraSystem->WaitForCompilationComplete(true);
		NiagaraSystem->PollForCompilationComplete();
		FAssetCompilingManager::Get().FinishAllCompilation();
		if (GShaderCompilingManager)
		{
			GShaderCompilingManager->FinishAllCompilation();
		}
#endif

		FAdvancedPreviewScene PreviewScene(
			FPreviewScene::ConstructionValues()
			.SetCreateDefaultLighting(true)
			.SetCreatePhysicsScene(false)
			.SetTransactional(false)
			.SetEditor(true));
		PreviewScene.SetFloorVisibility(false);
		UWorld* World = PreviewScene.GetWorld();
		if (World && !World->FXSystem)
		{
			World->FXSystem = FFXSystemInterface::Create(World->GetFeatureLevel(), World->Scene);
			if (World->Scene)
			{
				World->Scene->SetFXSystem(World->FXSystem);
			}
		}

		AActor* NiagaraActor = World ? World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator) : nullptr;
		UNiagaraComponent* NiagaraComponent = NiagaraActor ? NewObject<UNiagaraComponent>(NiagaraActor, TEXT("T66NiagaraIsolationNiagara"), RF_Transient) : nullptr;
		if (NiagaraActor && NiagaraComponent)
		{
			NiagaraComponent->SetAsset(NiagaraSystem);
			NiagaraComponent->CastShadow = 0;
			NiagaraComponent->bCastDynamicShadow = 0;
			NiagaraComponent->SetAllowScalability(false);
			NiagaraComponent->SetAutoActivate(false);
			NiagaraComponent->SetForceSolo(true);
			NiagaraComponent->SetAgeUpdateMode(ENiagaraAgeUpdateMode::DesiredAge);
			NiagaraComponent->SetCanRenderWhileSeeking(true);
			NiagaraComponent->SetMaxSimTime(FMath::Max(1.0f, Config.ActualTimeSeconds + Config.SeekDeltaSeconds));
			NiagaraComponent->SetRelativeLocation(FVector::ZeroVector);
			NiagaraActor->SetRootComponent(NiagaraComponent);
			NiagaraActor->AddInstanceComponent(NiagaraComponent);
			NiagaraComponent->RegisterComponent();
		}

		if (Config.bDebugPrimitive && World)
		{
			T66CreateDebugPrimitive(World);
		}

		UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(GetTransientPackage(), TEXT("T66NiagaraIsolationRT"), RF_Transient);
		if (RenderTarget)
		{
			RenderTarget->RenderTargetFormat = RTF_RGBA8_SRGB;
			RenderTarget->ClearColor = FLinearColor::Black;
			RenderTarget->InitAutoFormat(Config.ResX, Config.ResY);
			RenderTarget->UpdateResourceImmediate(true);
		}

		const FTransform CaptureTransform(
			FRotator(Config.CameraPitch, Config.CameraYaw, Config.CameraRoll),
			FVector(Config.CameraX, Config.CameraY, Config.CameraHeight));
		ASceneCapture2D* CaptureActor = World ? World->SpawnActor<ASceneCapture2D>(
			ASceneCapture2D::StaticClass(),
			CaptureTransform.GetLocation(),
			CaptureTransform.GetRotation().Rotator()) : nullptr;
		USceneCaptureComponent2D* CaptureComponent = CaptureActor ? CaptureActor->GetCaptureComponent2D() : nullptr;
		if (CaptureComponent && RenderTarget && NiagaraComponent)
		{
			CaptureComponent->TextureTarget = RenderTarget;
			CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
			CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
			CaptureComponent->OrthoWidth = Config.OrthoWidth;
			CaptureComponent->bCaptureEveryFrame = false;
			CaptureComponent->bCaptureOnMovement = false;
			CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
			CaptureComponent->ShowFlags = FEngineShowFlags(ESFIM_Game);
			CaptureComponent->ShowFlags.SetGrid(false);
			CaptureComponent->ShowFlags.SetPostProcessing(false);
			CaptureComponent->ShowFlags.SetAtmosphere(false);
			CaptureComponent->ShowFlags.SetFog(false);
			CaptureComponent->ShowFlags.SetParticles(true);
			CaptureComponent->ShowFlags.SetNiagara(true);
			CaptureComponent->ShowFlags.SetSelection(false);
			CaptureComponent->ShowFlags.SetSelectionOutline(false);
			CaptureComponent->SetWorldTransform(CaptureTransform);
			CaptureComponent->MarkRenderStateDirty();
			T66FlushCaptureWorld(World);
		}

		if (!NiagaraComponent || !CaptureComponent || !RenderTarget || !World)
		{
			FailureMode = TEXT("component_spawn_failed");
		}
		else
		{
			for (int32 Index = 0; Index < Config.PreviewTimesSeconds.Num(); ++Index)
			{
				FT66FrameCapture Frame;
				Frame.OutputPath = Config.OutputDir / FString::Printf(TEXT("preview_%02d.png"), Index);
				const bool bFrameCaptured = T66CaptureFrame(
					World,
					NiagaraComponent,
					CaptureComponent,
					RenderTarget,
					Config,
					ArrayDiagnostics,
					Config.PreviewTimesSeconds[Index],
					Frame);
				if (bFrameCaptured)
				{
					Frame.bSaveSuccess = T66SaveImage(Frame.OutputPath, Frame.Image);
				}
				Frames.Add(MoveTemp(Frame));
			}

			FT66FrameCapture ActualFrame;
			ActualFrame.OutputPath = Config.OutputDir / TEXT("actual.png");
			const double ActualCaptureStartSeconds = FPlatformTime::Seconds();
			if (T66CaptureFrame(
				World,
				NiagaraComponent,
				CaptureComponent,
				RenderTarget,
				Config,
				ArrayDiagnostics,
				Config.ActualTimeSeconds,
				ActualFrame))
			{
				CostDiagnostics = T66CollectCostDiagnostics(FPlatformTime::Seconds() - ActualCaptureStartSeconds);
				ActualFrame.bSaveSuccess = T66SaveImage(ActualFrame.OutputPath, ActualFrame.Image);
				ActualNonBlackRatio = ActualFrame.NonBlackPixelRatio;
				RenderDiagnostics = T66CollectRenderDiagnostics(NiagaraComponent);
				if (ActualFrame.bSaveSuccess)
				{
					T66SaveImage(Config.OutputDir / TEXT("actual_crop.png"), ActualFrame.Image);
				}
			}

			bRenderSuccess =
				ActualFrame.bReadbackSuccess &&
				ActualFrame.bSaveSuccess &&
				ActualFrame.Image.SizeX == Config.ResX &&
				ActualFrame.Image.SizeY == Config.ResY &&
				ActualFrame.NonBlackPixelRatio >= Config.NonBlackPixelRatioThreshold;

			if (!ActualFrame.bReadbackSuccess)
			{
				FailureMode = TEXT("render_target_read_failed");
			}
			else if (!ActualFrame.bSaveSuccess)
			{
				FailureMode = TEXT("image_file_missing");
			}
			else if (ActualFrame.Image.SizeX != Config.ResX || ActualFrame.Image.SizeY != Config.ResY)
			{
				FailureMode = TEXT("image_wrong_dimensions");
			}
			else if (ActualFrame.NonBlackPixelRatio < Config.NonBlackPixelRatioThreshold)
			{
				FailureMode = TEXT("image_below_non_black_threshold");
			}

			FImage ContactSheet;
			if (T66MakeContactSheet(Frames, Config.ResX, Config.ResY, ContactSheet))
			{
				T66SaveImage(Config.OutputDir / TEXT("contact_sheet.png"), ContactSheet);
			}
		}
	}

	if (!Config.TargetPath.IsEmpty() && FPaths::FileExists(Config.TargetPath))
	{
		const FString TargetDest = Config.OutputDir / TEXT("target.png");
		bTargetPresent = IFileManager::Get().Copy(*TargetDest, *Config.TargetPath, true, true) == COPY_OK;
	}

	if (!bRenderSuccess && FailureMode == TEXT("none"))
	{
		FailureMode = TEXT("niagara_not_populated_or_no_tick");
	}

	T66WriteMismatchNotes(Config, bTargetPresent, bRenderSuccess, FailureMode);
	T66WriteManifest(
		Config,
		ObjectPath,
		PackagePath,
		bAssetFilePresent,
		bAssetLoadSuccess,
		bRenderSuccess,
		FailureMode,
		ActualNonBlackRatio,
		bTargetPresent,
		Frames,
		RenderDiagnostics,
		ArrayDiagnostics,
		CostDiagnostics);

	UE_LOG(
		LogT66NiagaraIsolationCapture,
		Display,
		TEXT("[T66NiagaraIsolationCapture] Finished. RenderSuccess=%s FailureMode=%s Output=%s"),
		bRenderSuccess ? TEXT("true") : TEXT("false"),
		*FailureMode,
		*Config.OutputDir);

	return bRenderSuccess ? 0 : 1;
}
