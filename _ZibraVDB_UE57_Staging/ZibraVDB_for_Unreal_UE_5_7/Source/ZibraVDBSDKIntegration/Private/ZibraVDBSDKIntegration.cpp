// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBSDKIntegration.h"

#include "DynamicRHI.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/App.h"
#include "Misc/Paths.h"
#include "RHI.h"

#if WITH_EDITOR
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#include <Zibra/CE/Addons/FileManagement.h>
#endif

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

DEFINE_LOG_CATEGORY(LogZibraVDBSDKIntegration);

// clang-format off
#if WITH_EDITOR
static TAutoConsoleVariable<int32> CVarZibraVDBEnabled(
	TEXT("ZibraVDB.Enabled"),
	1,
	TEXT("Global switch for ZibraVDB plugin. This CVar can disable plugin only in Editor.\n")
	TEXT(" 0: Plugin disabled\n")
	TEXT(" 1: Plugin enabled (default)\n"),
	ECVF_ReadOnly);
#endif
// clang-format on

bool FZibraVDBSDKIntegration::bPluginEnabled = false;
FText FZibraVDBSDKIntegration::PluginDisabledReason;

// ZibraVDBSDKIntegration can't use FZibraVDBNotifications as it is in different module
// which we can't depend on due to circular dependencies
inline void ShowNotificationLocal(const FText& Message, float Duration)
{
	FString MessageAsString = Message.ToString();
	UE_LOG(LogZibraVDBSDKIntegration, Warning, TEXT("%s"), *MessageAsString);
#if WITH_EDITOR
	FText NotificationMessage = FText::Format(LOCTEXT("ZibraVDBNotificationPrefix", "ZibraVDB: {0}"), Message);
	FNotificationInfo NotificationInfo(NotificationMessage);
	NotificationInfo.ExpireDuration = Duration;
	FSlateNotificationManager::Get().AddNotification(NotificationInfo);
#endif
}

inline FString GetRHIName(ERHIInterfaceType Type)
{
	switch (Type)
	{
		case ERHIInterfaceType::Hidden:
			return TEXT("Hidden");
		case ERHIInterfaceType::Null:
			return TEXT("Null");
		case ERHIInterfaceType::D3D11:
			return TEXT("D3D11");
		case ERHIInterfaceType::D3D12:
			return TEXT("D3D12");
		case ERHIInterfaceType::Vulkan:
			return TEXT("Vulkan");
		case ERHIInterfaceType::Metal:
			return TEXT("Metal");
		case ERHIInterfaceType::Agx:
			return TEXT("Agx");
		case ERHIInterfaceType::OpenGL:
			return TEXT("OpenGL");
		default:
			checkNoEntry();
			return "";
	}
}

inline FString GetSMName(ERHIFeatureLevel::Type Level)
{
	switch (Level)
	{
		case ERHIFeatureLevel::ES3_1:
			return TEXT("ES3_1");
		case ERHIFeatureLevel::SM5:
			return TEXT("SM5");
		case ERHIFeatureLevel::SM6:
			return TEXT("SM6");
		default:
			checkNoEntry();
			return "";
	}
}

void FZibraVDBSDKIntegration::StartupModule() noexcept
{
	const FText InitializationErrorTemlpate = LOCTEXT("InitializationErrorTemlpate", "{0} ZibraVDB will not work.");
#if WITH_EDITOR
	if (CVarZibraVDBEnabled.GetValueOnGameThread() == 0)
	{
		PluginDisabledReason =
			LOCTEXT("PluginDisabledByCVar", "Plugin disabled by \"ZibraVDB.Enabled\" CVar.");
		const FText CompleteError = FText::Format(InitializationErrorTemlpate, PluginDisabledReason);
		UE_LOG(LogZibraVDBSDKIntegration, Display, TEXT("%s"), *CompleteError.ToString());
		return;
	}
#endif

	if (!FApp::CanEverRender())
	{
		PluginDisabledReason =
			LOCTEXT("CanEverRenderReturnedFalse", "FApp::CanEverRender() returned false, ZibraVDB initialization is skipped.");
		const FText CompleteError = FText::Format(InitializationErrorTemlpate, PluginDisabledReason);
		UE_LOG(LogZibraVDBSDKIntegration, Display, TEXT("%s"), *CompleteError.ToString());
		return;
	}

	if (GDynamicRHI == nullptr)
	{
		PluginDisabledReason =
			LOCTEXT("RHIUninitializedError", "Initialization failed due to uninitialized RHI.");
		const FText CompleteError = FText::Format(InitializationErrorTemlpate, PluginDisabledReason);
		ShowNotificationLocal(CompleteError, 15.0f);
		return;
	}

	ERHIInterfaceType RHIType = RHIGetInterfaceType();
	if (RHIType != ERHIInterfaceType::D3D12)
	{
		PluginDisabledReason =
			FText::Format(LOCTEXT("UnsupportedRHIError",
							  "Initialization failed due to unsupported RHI. Current RHI is {0}, while only D3D12 is supported."),
				FText::FromString(GetRHIName(RHIType)));
		const FText CompleteError = FText::Format(InitializationErrorTemlpate, PluginDisabledReason);
		ShowNotificationLocal(CompleteError, 15.0f);
		return;
	}

	if (GMaxRHIFeatureLevel < ERHIFeatureLevel::SM6)
	{
		PluginDisabledReason =
			FText::Format(LOCTEXT("UnsupportedSMError",
							  "Initialization failed due to unsupported RHI. Current RHI is {0}, while only D3D12 is supported."),
				FText::FromString(GetSMName(GMaxRHIFeatureLevel)));
		const FText CompleteError = FText::Format(InitializationErrorTemlpate, PluginDisabledReason);
		ShowNotificationLocal(CompleteError, 15.0f);
		return;
	}

	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ZibraVDB"));

	const FString BaseDir = Plugin->GetBaseDir();

#if WITH_EDITOR
	const FString LibraryPath = FPaths::Combine(BaseDir, TEXT("Source/ZibraVDBSDK/Editor/bin"));
#else
	const FString LibraryPath = FPaths::Combine(BaseDir, TEXT("Source/ZibraVDBSDK/Build/bin"));
#endif

	const FString LibraryName = "ZibraVDBSDK.dll";
	const FString LibraryFullPath = FPaths::Combine(LibraryPath, LibraryName);
	ZibraVDBSDKDLLHandle = FPlatformProcess::GetDllHandle(*LibraryFullPath);

	if (!ZibraVDBSDKDLLHandle)
	{
		PluginDisabledReason =
			FText::Format(LOCTEXT("ZibraVDBSDKDLLLoadFailed", "Could not load ZibraVDB SDK DLL from {0}."),
				FText::FromString(LibraryFullPath));
		const FText CompleteError = FText::Format(InitializationErrorTemlpate, PluginDisabledReason);
		ShowNotificationLocal(CompleteError, 15.0f);
		return;
	}

	if (!ImportSDKMethods())
	{
		PluginDisabledReason =
			LOCTEXT("ZibraVDBSDKMethodLoadFailed", "Failed to import methods from ZibraVDB SDK DLL.");
		const FText CompleteError = FText::Format(InitializationErrorTemlpate, PluginDisabledReason);
		ShowNotificationLocal(CompleteError, 15.0f);
		return;
	}

#if WITH_EDITOR
	const FString OpenVDBHelperLibraryPath = FPaths::Combine(BaseDir, TEXT("Source/ZibraVDBSDK/Editor/OpenVDBHelper/bin"));
	const FString ThirdPartyLibraryPaths = FPaths::Combine(BaseDir, TEXT("Source/ThirdParty"));

	const FString OpenVDBHelperLibraryName = "OpenVDBHelper.dll";
	const FString OpenVDBHelperLibraryFullPath = FPaths::Combine(OpenVDBHelperLibraryPath, OpenVDBHelperLibraryName);

	FPlatformProcess::PushDllDirectory(*ThirdPartyLibraryPaths);
	OpenVDBHelperDLLHandle = FPlatformProcess::GetDllHandle(*OpenVDBHelperLibraryFullPath);
	FPlatformProcess::PopDllDirectory(*ThirdPartyLibraryPaths);

	if (!OpenVDBHelperDLLHandle)
	{
		PluginDisabledReason = FText::Format(LOCTEXT("OpenVDBHelperDLLLoadFailed", "Could not load OpenVDBHelper DLL from {0}."),
				FText::FromString(OpenVDBHelperLibraryFullPath));
		const FText CompleteError = FText::Format(InitializationErrorTemlpate, PluginDisabledReason);
		ShowNotificationLocal(CompleteError, 15.0f);
		return;
	}

	if (!ImportOpenVDBHelperMethods())
	{
		PluginDisabledReason = LOCTEXT("OpenVDBHelperMethodLoadFailed", "Failed to import methods from OpenVDBHelper DLL.");
		const FText CompleteError = FText::Format(InitializationErrorTemlpate, PluginDisabledReason);
		ShowNotificationLocal(CompleteError, 15.0f);
		return;
	}
#endif

	bPluginEnabled = true;
}

void FZibraVDBSDKIntegration::ShutdownModule() noexcept
{
	bPluginEnabled = false;
}

bool FZibraVDBSDKIntegration::IsPluginEnabled()
{
	return bPluginEnabled;
}

const FText& FZibraVDBSDKIntegration::GetPluginDisabledReason()
{
	return PluginDisabledReason;
}

FString FZibraVDBSDKIntegration::ErrorToString(Zibra::CE::ReturnCode errorCode) noexcept
{
	switch (errorCode)
	{
		case Zibra::CE::ZCE_SUCCESS:
			checkNoEntry();
			return TEXT("");
		case Zibra::CE::ZCE_ERROR:
			return TEXT("Unexpected error");
		case Zibra::CE::ZCE_FATAL_ERROR:
			return TEXT("Fatal error");
		case Zibra::CE::ZCE_ERROR_NOT_INITIALIZED:
			return TEXT("ZibraVDB SDK is not initialized");
		case Zibra::CE::ZCE_ERROR_ALREADY_INITIALIZED:
			return TEXT("ZibraVDB SDK is already initialized");
		case Zibra::CE::ZCE_ERROR_INVALID_USAGE:
			return TEXT("ZibraVDB SDK invalid call");
		case Zibra::CE::ZCE_ERROR_INVALID_ARGUMENTS:
			return TEXT("ZibraVDB SDK invalid arguments");
		case Zibra::CE::ZCE_ERROR_NOT_IMPLEMENTED:
			return TEXT("Not implemented");
		case Zibra::CE::ZCE_ERROR_NOT_SUPPORTED:
			return TEXT("Unsupported");
		case Zibra::CE::ZCE_ERROR_NOT_FOUND:
			return TEXT("Not found");
		case Zibra::CE::ZCE_ERROR_OUT_OF_CPU_MEMORY:
			return TEXT("Out of CPU memory");
		case Zibra::CE::ZCE_ERROR_OUT_OF_GPU_MEMORY:
			return TEXT("Out of GPU memory");
		case Zibra::CE::ZCE_ERROR_TIME_OUT:
			return TEXT("Time out");
		case Zibra::CE::ZCE_ERROR_INVALID_SOURCE:
			return TEXT("Invalid file");
		case Zibra::CE::ZCE_ERROR_INCOMPTIBLE_SOURCE:
			return TEXT("Incompatible file");
		case Zibra::CE::ZCE_ERROR_CORRUPTED_SOURCE:
			return TEXT("Corrupted file");
		case Zibra::CE::ZCE_ERROR_IO_ERROR:
			return TEXT("I/O error");
		case Zibra::CE::ZCE_ERROR_LICENSE_TIER_TOO_LOW:
#if WITH_EDITOR
			if (IsLicenseValidated(Zibra::CE::Licensing::ProductType::Decompression))
			{
				return TEXT("Your license does not allow decompression of this effect.");
			}
			else
			{
				return TEXT("Decompression of this file requires active license.");
			}
#else
			return TEXT("Unexpected licensing error. If you ever get this, please report to ZibraAI.");
#endif
		default:
			checkNoEntry();
			return TEXT("Unknown error ") + FString::FromInt(static_cast<int32>(errorCode));
	}
}

#if WITH_EDITOR

Zibra::OpenVDBHelper::SequenceInfo* FZibraVDBSDKIntegration::ReadSequenceInfo(
	const wchar_t* const* FilePaths, size_t FileCount, float* Progress) noexcept
{
	auto* result = Zibra::OpenVDBHelper::CAPI::ReadSequenceInfo(FilePaths, FileCount, Progress);
	return result;
}

void FZibraVDBSDKIntegration::ReleaseSequenceInfo(Zibra::OpenVDBHelper::SequenceInfo* sequenceInfo) noexcept
{
	if (!IsPluginEnabled())
	{
		checkNoEntry();
		return;
	}

	Zibra::OpenVDBHelper::CAPI::ReleaseSequenceInfo(sequenceInfo);
}

std::vector<std::filesystem::path> FZibraVDBSDKIntegration::CalculateFileList(const FString& InputFileMask) noexcept
{
	if (!IsPluginEnabled())
	{
		checkNoEntry();
		return {};
	}

	return Zibra::CE::Addons::FileManagement::CalculateFileList(*InputFileMask);
}

void FZibraVDBSDKIntegration::CheckoutLicenseWithKey(const char* licenseKey) noexcept
{
	if (!IsPluginEnabled())
	{
		checkNoEntry();
		return;
	}

	Zibra::CE::Licensing::CAPI::CheckoutLicenseWithKey(licenseKey);
}

void FZibraVDBSDKIntegration::CheckoutLicenseOffline(const char* license, int licenseSize) noexcept
{
	if (!IsPluginEnabled())
	{
		checkNoEntry();
		return;
	}

	Zibra::CE::Licensing::CAPI::CheckoutLicenseOffline(license, licenseSize);
}

void FZibraVDBSDKIntegration::CheckoutLicenseLicenseServer(const char* licenseServerAddress) noexcept
{
	if (!IsPluginEnabled())
	{
		checkNoEntry();
		return;
	}

	Zibra::CE::Licensing::CAPI::CheckoutLicenseLicenseServer(licenseServerAddress);
}

bool FZibraVDBSDKIntegration::IsLicenseValidated(Zibra::CE::Licensing::ProductType product) noexcept
{
	if (!IsPluginEnabled())
	{
		checkNoEntry();
		return false;
	}

	return Zibra::CE::Licensing::CAPI::IsLicenseValidated(product);
}

FString FZibraVDBSDKIntegration::GetLicenseType(Zibra::CE::Licensing::ProductType product) noexcept
{
	if (!IsPluginEnabled())
	{
		checkNoEntry();
		return {};
	}

	const char* LicenseType = Zibra::CE::Licensing::CAPI::GetProductLicenseType(product);
	return LicenseType ? FString{ANSI_TO_TCHAR(LicenseType)} : FString{};
}

FString FZibraVDBSDKIntegration::GetHardwareID() noexcept
{
	if (!IsPluginEnabled())
	{
		checkNoEntry();
		return {};
	}

	const char* HardwareID = Zibra::CE::Licensing::CAPI::GetHardwareID();
	return HardwareID ? FString{ANSI_TO_TCHAR(HardwareID)} : FString{};
}

void FZibraVDBSDKIntegration::ReleaseLicense() noexcept
{
	if (!IsPluginEnabled())
	{
		checkNoEntry();
		return;
	}

	Zibra::CE::Licensing::CAPI::ReleaseLicense();
}

const char* FZibraVDBSDKIntegration::GetLicenseError() noexcept
{
	if (!IsPluginEnabled())
	{
		checkNoEntry();
		return "";
	}

	return Zibra::CE::Licensing::CAPI::GetLicenseError();
}

#endif

bool FZibraVDBSDKIntegration::ImportSDKMethods()
{
#define ZIB_LOAD_SDK_FUNCTION_POINTER(FunctionName)                                                         \
	FunctionName = reinterpret_cast<ZCE_PFN(FunctionName)>(                                                 \
		FPlatformProcess::GetDllExport(ZibraVDBSDKDLLHandle, ANSI_TO_TCHAR(ZIB_STRINGIFY(FunctionName))));  \
	if (FunctionName == nullptr)                                                                            \
	{                                                                                                       \
		UE_LOG(LogZibraVDBSDKIntegration, Error, TEXT("Loading of ZibraVDB SDK DLL failed on function %s"), \
			ANSI_TO_TCHAR(ZIB_STRINGIFY(FunctionName)));                                                    \
		return false;                                                                                       \
	}

	ZSDK_RUNTIME_FUNCTION_LIST_APPLY(ZIB_LOAD_SDK_FUNCTION_POINTER)
#undef ZIB_LOAD_SDK_FUNCTION_POINTER

	return true;
}

#if WITH_EDITOR
bool FZibraVDBSDKIntegration::ImportOpenVDBHelperMethods()
{
#define ZIB_LOAD_OPENVDBHELPER_FUNCTION_POINTER(FunctionName)                                                \
	FunctionName = reinterpret_cast<ZIB_PFN(FunctionName)>(                                                  \
		FPlatformProcess::GetDllExport(OpenVDBHelperDLLHandle, ANSI_TO_TCHAR(ZIB_STRINGIFY(FunctionName)))); \
	if (FunctionName == nullptr)                                                                             \
	{                                                                                                        \
		UE_LOG(LogZibraVDBSDKIntegration, Error, TEXT("Loading of OpenVDBHelper DLL failed on function %s"), \
			ANSI_TO_TCHAR(ZIB_STRINGIFY(FunctionName)));                                                     \
		return false;                                                                                        \
	}

	ZIB_OPENVDBHELPER_API_APPLY(ZIB_LOAD_OPENVDBHELPER_FUNCTION_POINTER)
#undef ZIB_LOAD_OPENVDBHELPER_FUNCTION_POINTER
	return true;
}
#endif

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FZibraVDBSDKIntegration, ZibraVDBSDKIntegration)
