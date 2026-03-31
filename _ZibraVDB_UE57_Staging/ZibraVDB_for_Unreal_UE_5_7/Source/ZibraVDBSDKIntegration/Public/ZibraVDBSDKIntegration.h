// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include <Zibra/CE/Compression.h>
#include <Zibra/CE/Licensing.h>
#include <Zibra/OpenVDBHelper.h>
#endif

#include <Zibra/CE/Decompression.h>
#include <Zibra/RHI.h>

DECLARE_LOG_CATEGORY_EXTERN(LogZibraVDBSDKIntegration, Log, All);

#define ZIB_STRINGIFY_INTERNAL(x) #x
#define ZIB_STRINGIFY(x) ZIB_STRINGIFY_INTERNAL(x)

#if WITH_EDITOR
#define ZSDK_RUNTIME_FUNCTION_LIST_APPLY(macro) \
	ZRHI_API_APPLY(macro)                       \
	ZCE_DECOMPRESSION_API_APPLY(macro)          \
	ZCE_COMPRESSOR_API_APPLY(macro)             \
	ZCE_LICENSING_API_APPLY(macro)
#else
#define ZSDK_RUNTIME_FUNCTION_LIST_APPLY(macro) \
	ZRHI_API_APPLY(macro)                       \
	ZCE_DECOMPRESSION_API_APPLY(macro)
#endif

#define ZSDK_CONCAT_HELPER(A, B) A##B
#define ZSDK_PFN(name) ZSDK_CONCAT_HELPER(PFN_, name)

#define ZSDK_DEFINE_FUNCTION_POINTER(name) ZSDK_PFN(name) name = nullptr;
ZSDK_RUNTIME_FUNCTION_LIST_APPLY(ZSDK_DEFINE_FUNCTION_POINTER)
#undef ZSDK_DEFINE_FUNCTION_POINTER

#if WITH_EDITOR
#define ZIB_DEFINE_FUNCTION_POINTER(name) ZIB_PFN(name) name = nullptr;
ZIB_OPENVDBHELPER_API_APPLY(ZIB_DEFINE_FUNCTION_POINTER)
#undef ZIB_DEFINE_FUNCTION_POINTER
#endif

#undef ZSDK_PFN
#undef ZSDK_CONCAT_HELPER

class ZIBRAVDBSDKINTEGRATION_API FZibraVDBSDKIntegration final : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() noexcept override final;
	virtual void ShutdownModule() noexcept override final;

	static bool IsPluginEnabled();
	static const FText& GetPluginDisabledReason();

	static FString ErrorToString(Zibra::CE::ReturnCode errorCode) noexcept;

#if WITH_EDITOR
	static Zibra::OpenVDBHelper::SequenceInfo* ReadSequenceInfo(
		const wchar_t* const* FilePaths, size_t FileCount, float* Progress) noexcept;
	static void ReleaseSequenceInfo(Zibra::OpenVDBHelper::SequenceInfo* sequenceInfo) noexcept;

	static std::vector<std::filesystem::path> CalculateFileList(const FString& InputFileMask) noexcept;

	static void CheckoutLicenseWithKey(const char* licenseKey) noexcept;
	static void CheckoutLicenseOffline(const char* license, int licenseSize) noexcept;
	static void CheckoutLicenseLicenseServer(const char* licenseServerAddress) noexcept;
	static bool IsLicenseValidated(Zibra::CE::Licensing::ProductType product) noexcept;
	static FString GetLicenseType(Zibra::CE::Licensing::ProductType product) noexcept;
	static FString GetHardwareID() noexcept;
	static void ReleaseLicense() noexcept;
	static const char* GetLicenseError() noexcept;
#endif

private:
	static bool bPluginEnabled;
	static FText PluginDisabledReason;

	void* ZibraVDBSDKDLLHandle;
	bool ImportSDKMethods();

	static void LoggingBridge(uint32_t verbosity, const char* message);

#if WITH_EDITOR
	void* OpenVDBHelperDLLHandle;
	bool ImportOpenVDBHelperMethods();
#endif
};
