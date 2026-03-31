// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#if WITH_EDITOR
#include "Containers/UnrealString.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Widgets/Notifications/SNotificationList.h"

#include <regex>
#endif

#include "ZibraLicensingManager.generated.h"

ZIBRAVDBLICENSING_API DECLARE_LOG_CATEGORY_EXTERN(LogZibraVDBLicensing, Log, All);

UENUM()
enum class ZibraLicenseStatus : uint8
{
	NotInitialized UMETA(DisplayName = "Not Initialized"),
	OK UMETA(DisplayName = "OK"),
	ValidationError UMETA(DisplayName = "Validation Error"),
	NoKey UMETA(DisplayName = "No Key"),
};

UENUM()
enum class ZibraActivationType : uint8
{
	NotActivated UMETA(DisplayName = "Not Activated"),
	OfflineLicense UMETA(DisplayName = "Offline License"),
	LicenseServer UMETA(DisplayName = "License Server"),
	LicenseKey UMETA(DisplayName = "License Key")
};

#if WITH_EDITOR
class ZIBRAVDBLICENSING_API FZibraLicensingManager
{
private:
	FZibraLicensingManager();

public:
	enum class Product
	{
		Compression,
		Decompression,
		Count
	};

	static void CreateInstance();
	static void DestroyInstance();
	static FZibraLicensingManager& GetInstance();

	bool ValidateKeyFormat(const FString& Key);

	void ValidateLicenseOffline(const FString& OfflineActivation);
	void ValidateLicenseLicenseServer(const FString& ServerAddress);
	void ValidateLicenseKey(const FString& Key);

	FStringView GetLicenseKey() const;
	ZibraLicenseStatus GetStatus(Product product) const;
	FStringView GetErrorMessage(Product product) const;
	FStringView GetLicenseType(Product product) const;
	FStringView GetLicenseType() const;
	FString GetHardwareId() const;
	bool IsLicenseVerified(Product product) const;
	bool IsAnyLicenseVerified() const;

	void SetManualActivationFlag();

	void RemoveLicense();

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLicenseValidation, bool);
	FOnLicenseValidation OnLicenseValidation;

private:
	void InternalValidateLicense();

	void InternalValidateLicenseOffline();
	void InternalValidateLicenseLicenseServer();
	void InternalValidateLicenseKey();
	void SyncStateWithSDK();
	void ShowLicenseActivatedMessage();

	void RestoreLicenseSettings();

	void SetCurrentStatus(Product Product, ZibraLicenseStatus Status);
	void SetErrorMessage(const FString& ErrorMessage);

	void AddNotification(const FString& Message);
	static TOptional<FNotificationInfo> CreateNotification(const FString& Message);

	void BroadcastValidateLicense();

	static FZibraLicensingManager* Instance;

	std::regex KeyFormatPattern;
	FString OfflineActivationData;
	FString LicenseServerAddress;
	FString LicenseKey;
	ZibraLicenseStatus CurrentStatus[size_t(Product::Count)];
	FString ErrorMessage;
	FString LicenseType[size_t(Product::Count)];
	ZibraActivationType ActivationType;
	bool ManualActivation;

	static const std::string KeyFormatRegex;
	static const FString LicenseValidationURL;
	static const FString ProductName;
	static const FString EngineName;
};
#endif
