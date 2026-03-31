// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraLicensingManager.h"

#if WITH_EDITOR

#include "Framework/Notifications/NotificationManager.h"
#include "JsonObjectConverter.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "ZibraLicensingSettings.h"
#include "ZibraVDBSDKIntegration/Public/ZibraVDBSDKIntegration.h"

#include <Zibra/CE/Licensing.h>

#include <stdexcept>

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

DEFINE_LOG_CATEGORY(LogZibraVDBLicensing);

FZibraLicensingManager* FZibraLicensingManager::Instance = nullptr;

const std::string FZibraLicensingManager::KeyFormatRegex =
	"^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$";

#pragma warning(disable : 4702)

FZibraLicensingManager::FZibraLicensingManager() : KeyFormatPattern(KeyFormatRegex)
{
	RestoreLicenseSettings();

#if !ZIBRAVDB_LICENSE_CHECK_ENABLED
	for (size_t i = 0; i < static_cast<size_t>(Product::Count); ++i)
	{
		CurrentStatus[i] = ZibraLicenseStatus::OK;
	}
	return;
#endif

	ManualActivation = false;
	for (size_t i = 0; i < static_cast<size_t>(Product::Count); ++i)
	{
		CurrentStatus[i] = ZibraLicenseStatus::NotInitialized;
		ErrorMessage = "";
	}
	ActivationType = ZibraActivationType::NotActivated;

	if (!FZibraVDBSDKIntegration::IsPluginEnabled())
	{
		return;
	}
	InternalValidateLicense();
}

void FZibraLicensingManager::CreateInstance()
{
	check(!Instance);
	Instance = new FZibraLicensingManager();
}

void FZibraLicensingManager::DestroyInstance()
{
	check(Instance);
	delete Instance;
	Instance = nullptr;
}

FZibraLicensingManager& FZibraLicensingManager::GetInstance()
{
	// Can't have static Instance because Instance construction depends on Regex module being loaded
	check(Instance);
	return *Instance;
}

bool FZibraLicensingManager::ValidateKeyFormat(const FString& Key)
{
	const char* KeyStr = TCHAR_TO_ANSI(*Key);
	return std::regex_match(KeyStr, KeyFormatPattern);
}

void FZibraLicensingManager::ValidateLicenseOffline(const FString& OfflineActivation)
{
#if !ZIBRAVDB_LICENSE_CHECK_ENABLED
	return;
#endif

	if (!FZibraVDBSDKIntegration::IsPluginEnabled())
	{
		if (ManualActivation)
		{
			const FText ErrorText =
				FText::Format(LOCTEXT("CantValidateWhilePluginInitializationFailed",
								  "Can't validate ZibraVDB license since the plugin initialization failed. Reason: {0}"),
					FZibraVDBSDKIntegration::GetPluginDisabledReason());
			AddNotification(ErrorText.ToString());
		}
		return;
	}

	OfflineActivationData = OfflineActivation;
	InternalValidateLicenseOffline();
}

void FZibraLicensingManager::ValidateLicenseLicenseServer(const FString& ServerAddress)
{
#if !ZIBRAVDB_LICENSE_CHECK_ENABLED
	return;
#endif

	if (!FZibraVDBSDKIntegration::IsPluginEnabled())
	{
		if (ManualActivation)
		{
			const FText ErrorText =
				FText::Format(LOCTEXT("CantValidateWhilePluginInitializationFailed",
								  "Can't validate ZibraVDB license since the plugin initialization failed. Reason: {0}"),
					FZibraVDBSDKIntegration::GetPluginDisabledReason());
			AddNotification(ErrorText.ToString());
		}
		return;
	}

	LicenseServerAddress = ServerAddress;
	InternalValidateLicenseLicenseServer();
}

void FZibraLicensingManager::ValidateLicenseKey(const FString& Key)
{
#if !ZIBRAVDB_LICENSE_CHECK_ENABLED
	return;
#endif

	if (!FZibraVDBSDKIntegration::IsPluginEnabled())
	{
		if (ManualActivation)
		{
			const FText ErrorText =
				FText::Format(LOCTEXT("CantValidateWhilePluginInitializationFailed",
								  "Can't validate ZibraVDB license since the plugin initialization failed. Reason: {0}"),
					FZibraVDBSDKIntegration::GetPluginDisabledReason());
			AddNotification(ErrorText.ToString());
		}
		return;
	}

	LicenseKey = Key;
	InternalValidateLicenseKey();
}

FStringView FZibraLicensingManager::GetLicenseKey() const
{
	return LicenseKey;
}

ZibraLicenseStatus FZibraLicensingManager::GetStatus(Product product) const
{
	return CurrentStatus[size_t(product)];
}

FStringView FZibraLicensingManager::GetErrorMessage(Product product) const
{
	switch (CurrentStatus[size_t(product)])
	{
		case ZibraLicenseStatus::ValidationError:
		{
			return ErrorMessage;
		}
		case ZibraLicenseStatus::NoKey:
		{
			static FString NoKeyErrorMessage = "Plugin is not registered.";
			return NoKeyErrorMessage;
		}
		default:
		{
			static FString DefaultErrorMessage = "";
			return DefaultErrorMessage;
		}
	}
}

FStringView FZibraLicensingManager::GetLicenseType(Product product) const
{
	if (!IsLicenseVerified(product))
	{
		return FStringView();
	}
	return LicenseType[size_t(product)];
}

FStringView FZibraLicensingManager::GetLicenseType() const
{
	for (size_t i = 0; i < static_cast<size_t>(Product::Count); ++i)
	{
		if (CurrentStatus[i] == ZibraLicenseStatus::OK)
		{
			return LicenseType[i];
		}
	}
	return FStringView();
}

FString FZibraLicensingManager::GetHardwareId() const
{
	if (!FZibraVDBSDKIntegration::IsPluginEnabled())
	{
		return "";
	}

	return FZibraVDBSDKIntegration::GetHardwareID();
}

bool FZibraLicensingManager::IsLicenseVerified(Product product) const
{
	return CurrentStatus[size_t(product)] == ZibraLicenseStatus::OK;
}

bool FZibraLicensingManager::IsAnyLicenseVerified() const
{
	for (size_t i = 0; i < static_cast<size_t>(Product::Count); ++i)
	{
		if (CurrentStatus[i] == ZibraLicenseStatus::OK)
		{
			return true;
		}
	}
	return false;
}

void FZibraLicensingManager::SetManualActivationFlag()
{
	ManualActivation = true;
}

void FZibraLicensingManager::RemoveLicense()
{
#if !ZIBRAVDB_LICENSE_CHECK_ENABLED
	return;
#endif

	OfflineActivationData = "";
	LicenseServerAddress = "";
	LicenseKey = "";

	for (size_t i = 0; i < static_cast<size_t>(Product::Count); ++i)
	{
		SetCurrentStatus(static_cast<Product>(i), ZibraLicenseStatus::NotInitialized);
	}
	
	if (FZibraVDBSDKIntegration::IsPluginEnabled())
	{
		FZibraVDBSDKIntegration::ReleaseLicense();
	}

	if (ActivationType != ZibraActivationType::NotActivated)
	{
		ActivationType = ZibraActivationType::NotActivated;
		AddNotification("License removed.");
	}
}

void FZibraLicensingManager::InternalValidateLicense()
{
#if !ZIBRAVDB_LICENSE_CHECK_ENABLED
	return;
#endif

	check(FZibraVDBSDKIntegration::IsPluginEnabled());

	if (!OfflineActivationData.IsEmpty())
	{
		InternalValidateLicenseOffline();
		return;
	}

	if (!LicenseServerAddress.IsEmpty())
	{
		InternalValidateLicenseLicenseServer();
		return;
	}

	if (!LicenseKey.IsEmpty())
	{
		InternalValidateLicenseKey();
		return;
	}
}

void FZibraLicensingManager::InternalValidateLicenseOffline()
{
	check(FZibraVDBSDKIntegration::IsPluginEnabled());

	FTCHARToUTF8 Converter(*OfflineActivationData);
	FZibraVDBSDKIntegration::CheckoutLicenseOffline(Converter.Get(), Converter.Length());

	SyncStateWithSDK();
	if (!IsAnyLicenseVerified())
	{
		const FString ErrorString = FZibraVDBSDKIntegration::GetLicenseError();
		AddNotification(ErrorString);
		SetErrorMessage(ErrorString);
		ActivationType = ZibraActivationType::NotActivated;
		FZibraVDBSDKIntegration::ReleaseLicense();
		return;
	}
	ActivationType = ZibraActivationType::OfflineLicense;

	ShowLicenseActivatedMessage();
	BroadcastValidateLicense();
}

void FZibraLicensingManager::InternalValidateLicenseLicenseServer()
{
	check(FZibraVDBSDKIntegration::IsPluginEnabled());

	FTCHARToUTF8 Converter(*LicenseServerAddress);
	FZibraVDBSDKIntegration::CheckoutLicenseLicenseServer(Converter.Get());

	SyncStateWithSDK();
	if (!IsAnyLicenseVerified())
	{
		const FString ErrorString = FZibraVDBSDKIntegration::GetLicenseError();
		AddNotification(ErrorString);
		SetErrorMessage(ErrorString);
		ActivationType = ZibraActivationType::NotActivated;
		FZibraVDBSDKIntegration::ReleaseLicense();
		return;
	}
	ActivationType = ZibraActivationType::LicenseServer;

	ShowLicenseActivatedMessage();
	BroadcastValidateLicense();
}

void FZibraLicensingManager::InternalValidateLicenseKey()
{
	check(FZibraVDBSDKIntegration::IsPluginEnabled());

	FTCHARToUTF8 Converter(*LicenseKey);
	FZibraVDBSDKIntegration::CheckoutLicenseWithKey(Converter.Get());

	SyncStateWithSDK();
	if (!IsAnyLicenseVerified())
	{
		const FString ErrorString = FZibraVDBSDKIntegration::GetLicenseError();
		AddNotification(ErrorString);
		SetErrorMessage(ErrorString);
		ActivationType = ZibraActivationType::NotActivated;
		FZibraVDBSDKIntegration::ReleaseLicense();
		return;
	}
	ActivationType = ZibraActivationType::LicenseKey;

	ShowLicenseActivatedMessage();
	BroadcastValidateLicense();
}

void FZibraLicensingManager::SyncStateWithSDK()
{
	check(FZibraVDBSDKIntegration::IsPluginEnabled());

#if !ZIBRAVDB_LICENSE_CHECK_ENABLED
	return;
#endif

	for (int i = 0; i < static_cast<int>(Product::Count); ++i)
	{
		Product product = static_cast<Product>(i);
		if (FZibraVDBSDKIntegration::IsLicenseValidated(Zibra::CE::Licensing::ProductType(product)))
		{
			SetCurrentStatus(product, ZibraLicenseStatus::OK);
			LicenseType[i] = FZibraVDBSDKIntegration::GetLicenseType(Zibra::CE::Licensing::ProductType(product));
		}
		else
		{
			SetCurrentStatus(product, ZibraLicenseStatus::ValidationError);
			SetErrorMessage(FZibraVDBSDKIntegration::GetLicenseError());
		}
	}
}

void FZibraLicensingManager::ShowLicenseActivatedMessage()
{
	FString MessageString = FString(TEXT("License successfully verified. License Type: ")) + GetLicenseType();

	bool IsCompressionActivated = IsLicenseVerified(Product::Compression);
	bool IsDecompressionActivated = IsLicenseVerified(Product::Decompression);

	if (IsCompressionActivated && !IsDecompressionActivated)
	{
		MessageString += TEXT(" (Only Compression)");
	}
	else if (!IsCompressionActivated && IsDecompressionActivated)
	{
		MessageString += TEXT(" (Only Decompression)");
	}

	if (ManualActivation)
	{
		AddNotification(MessageString);
	}
	else
	{
		UE_LOG(LogZibraVDBLicensing, Display, TEXT("%s"), *MessageString);
	}
}

void FZibraLicensingManager::RestoreLicenseSettings()
{
	UZibraLicensingSettings* Settings = GetMutableDefault<UZibraLicensingSettings>();
	OfflineActivationData = Settings->OfflineActivation;
	if (!OfflineActivationData.IsEmpty())
	{
		Settings->LicenseServerAddress = "";
		Settings->LicenseKey = "";
		return;
	}

	LicenseServerAddress = Settings->LicenseServerAddress;
	if (!LicenseServerAddress.IsEmpty())
	{
		LicenseKey = "";
		return;
	}

	LicenseKey = Settings->LicenseKey;
}

void FZibraLicensingManager::SetCurrentStatus(Product Product, ZibraLicenseStatus Status)
{
	CurrentStatus[size_t(Product)] = Status;
}

void FZibraLicensingManager::SetErrorMessage(const FString& NewErrorMessage)
{
	ErrorMessage = NewErrorMessage;
}

void FZibraLicensingManager::AddNotification(const FString& Message)
{
	const TOptional<FNotificationInfo> Info = CreateNotification(Message);

	if (Info.IsSet())
	{
		FSlateNotificationManager::Get().AddNotification(Info.GetValue());
	}
}

TOptional<FNotificationInfo> FZibraLicensingManager::CreateNotification(const FString& Message)
{
	FString DisplayMessage = FString::Printf(TEXT("ZibraVDB Licensing: %s"), *Message);

	UE_LOG(LogZibraVDBLicensing, Display, TEXT("%s"), *DisplayMessage);
	const FText NotificationErrorText = FText::FromString(DisplayMessage);
	FNotificationInfo Info(NotificationErrorText);

	Info.ExpireDuration = 10.0f;

	return TOptional<FNotificationInfo>(Info);
}

void FZibraLicensingManager::BroadcastValidateLicense()
{
#if ZIBRAVDB_LICENSE_CHECK_ENABLED
	OnLicenseValidation.Broadcast(ManualActivation);
#endif
	ManualActivation = false;
}

#pragma warning(default : 4702)

#undef LOCTEXT_NAMESPACE

#endif
