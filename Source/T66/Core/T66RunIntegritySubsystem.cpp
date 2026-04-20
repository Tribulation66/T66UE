// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RunIntegritySubsystem.h"

#include "Core/T66SteamHelper.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Misc/StringBuilder.h"

namespace
{
	FString T66ProjectVersionString()
	{
		FString BuildVersion = FApp::GetBuildVersion();
		if (!BuildVersion.IsEmpty())
		{
			return BuildVersion;
		}

		GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), BuildVersion, GGameIni);
		return BuildVersion;
	}

	void T66AppendSuspiciousPathReasons(const TArray<FString>& CandidateRoots, FT66RunIntegrityContext& Context)
	{
		IFileManager& FileManager = IFileManager::Get();
		static const TCHAR* SuspiciousDirectories[] =
		{
			TEXT("BepInEx"),
			TEXT("MelonLoader"),
			TEXT("UE4SS"),
			TEXT("ue4ss"),
			TEXT("Mods"),
			TEXT("~mods"),
		};
		static const TCHAR* SuspiciousFiles[] =
		{
			TEXT("doorstop_config.ini"),
			TEXT("ue4ss.dll"),
			TEXT("UE4SS.dll"),
			TEXT("winhttp.dll"),
			TEXT("version.dll"),
		};

		for (const FString& Root : CandidateRoots)
		{
			for (const TCHAR* DirName : SuspiciousDirectories)
			{
				const FString SuspiciousDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(Root, DirName));
				if (FileManager.DirectoryExists(*SuspiciousDir))
				{
					Context.PromoteToVerdict(TEXT("modded"), FString::Printf(TEXT("suspicious_dir:%s"), DirName));
				}
			}

			for (const TCHAR* FileName : SuspiciousFiles)
			{
				const FString SuspiciousFile = FPaths::ConvertRelativePathToFull(FPaths::Combine(Root, FileName));
				if (FileManager.FileExists(*SuspiciousFile))
				{
					Context.PromoteToVerdict(TEXT("modded"), FString::Printf(TEXT("suspicious_file:%s"), FileName));
				}
			}
		}
	}
}

void UT66RunIntegritySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetActiveRunContext();
}

void UT66RunIntegritySubsystem::ResetActiveRunContext()
{
	ActiveContext = FT66RunIntegrityContext{};
}

void UT66RunIntegritySubsystem::CaptureFreshRunBaseline()
{
	ResetActiveRunContext();
	MergeCurrentEnvironmentInto(ActiveContext);
	ActiveContext.bBaselineCaptured = true;
	ActiveContext.bFinalized = false;
	ActiveContext.BaselineHash = ComputeCombinedHash(
		ActiveContext.SteamAppId,
		ActiveContext.SteamBuildId,
		ActiveContext.SteamBetaName,
		ActiveContext.ManifestId,
		ActiveContext.ManifestRootHash,
		ActiveContext.ModuleListHash,
		ActiveContext.MountedContentHash);
	ActiveContext.FinalHash = ActiveContext.BaselineHash;
	ActiveContext.MarkPristineIfUnset();
}

void UT66RunIntegritySubsystem::RestoreActiveRunContext(const FT66RunIntegrityContext& SavedContext)
{
	ActiveContext = SavedContext;
	ActiveContext.MarkPristineIfUnset();
}

void UT66RunIntegritySubsystem::MarkRunModifierSelected(const FString& ModifierKind, const FName ModifierId)
{
	ActiveContext.RunModifierKind = ModifierKind;
	ActiveContext.RunModifierId = ModifierId.ToString();
	ActiveContext.PromoteToVerdict(TEXT("run_modifier"), FString::Printf(TEXT("run_modifier:%s:%s"), *ModifierKind, *ActiveContext.RunModifierId));
}

void UT66RunIntegritySubsystem::MarkLoadedSnapshot()
{
	ActiveContext.bLoadedFromSnapshot = true;
	ActiveContext.PromoteToVerdict(TEXT("loaded_snapshot"), TEXT("loaded_snapshot"));
}

void UT66RunIntegritySubsystem::FinalizeCurrentRun()
{
	FT66RunIntegrityContext Current = ActiveContext;
	MergeCurrentEnvironmentInto(Current);
	Current.FinalHash = ComputeCombinedHash(
		Current.SteamAppId,
		Current.SteamBuildId,
		Current.SteamBetaName,
		Current.ManifestId,
		Current.ManifestRootHash,
		Current.ModuleListHash,
		Current.MountedContentHash);
	Current.bFinalized = true;

	if (!ActiveContext.bBaselineCaptured || ActiveContext.BaselineHash.IsEmpty())
	{
		Current.PromoteToVerdict(TEXT("missing_baseline"), TEXT("missing_baseline"));
	}
	else
	{
		if (!ActiveContext.ManifestRootHash.Equals(Current.ManifestRootHash, ESearchCase::CaseSensitive))
		{
			Current.PromoteToVerdict(TEXT("integrity_mismatch"), TEXT("manifest_root_mismatch"));
		}
		if (!ActiveContext.ModuleListHash.Equals(Current.ModuleListHash, ESearchCase::CaseSensitive))
		{
			Current.PromoteToVerdict(TEXT("integrity_mismatch"), TEXT("module_list_mismatch"));
		}
		if (!ActiveContext.MountedContentHash.Equals(Current.MountedContentHash, ESearchCase::CaseSensitive))
		{
			Current.PromoteToVerdict(TEXT("integrity_mismatch"), TEXT("mounted_content_mismatch"));
		}
		if (!ActiveContext.BaselineHash.Equals(Current.FinalHash, ESearchCase::CaseSensitive))
		{
			Current.PromoteToVerdict(TEXT("integrity_mismatch"), TEXT("baseline_hash_mismatch"));
		}
	}

	Current.MarkPristineIfUnset();
	ActiveContext = MoveTemp(Current);
}

bool UT66RunIntegritySubsystem::IsCurrentEnvironmentConsistentWithBaseline() const
{
	if (!ActiveContext.bBaselineCaptured || ActiveContext.BaselineHash.IsEmpty())
	{
		return false;
	}

	FT66RunIntegrityContext Current = ActiveContext;
	MergeCurrentEnvironmentInto(Current);
	const FString CurrentHash = ComputeCombinedHash(
		Current.SteamAppId,
		Current.SteamBuildId,
		Current.SteamBetaName,
		Current.ManifestId,
		Current.ManifestRootHash,
		Current.ModuleListHash,
		Current.MountedContentHash);
	return CurrentHash.Equals(ActiveContext.BaselineHash, ESearchCase::CaseSensitive);
}

void UT66RunIntegritySubsystem::CopyCurrentContextTo(FT66RunIntegrityContext& OutContext) const
{
	OutContext = ActiveContext;
}

void UT66RunIntegritySubsystem::MergeCurrentEnvironmentInto(FT66RunIntegrityContext& Context) const
{
	const FString BaseDir = FPaths::ConvertRelativePathToFull(FPlatformProcess::BaseDir());
	const FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	const FString ExecutablePath = FPaths::ConvertRelativePathToFull(FPlatformProcess::ExecutablePath());
	const FString PaksDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(ProjectDir, TEXT("Content"), TEXT("Paks")));
	const FString PluginsDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(ProjectDir, TEXT("Plugins")));
	const FString BinariesDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(ProjectDir, TEXT("Binaries")));

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66SteamHelper* SteamHelper = GI->GetSubsystem<UT66SteamHelper>())
		{
			Context.SteamAppId = SteamHelper->GetActiveSteamAppId();
			Context.SteamBuildId = SteamHelper->GetActiveSteamBuildId();
			Context.SteamBetaName = SteamHelper->GetCurrentSteamBetaName();
		}
	}

	Context.ManifestId = T66ProjectVersionString();
	if (Context.ManifestId.IsEmpty() && Context.SteamBuildId > 0)
	{
		Context.ManifestId = FString::Printf(TEXT("steam_build_%d"), Context.SteamBuildId);
	}

	TArray<FString> ManifestEntries;
	AppendFileStamp(ExecutablePath, ManifestEntries);
	AppendDirectoryFileStamps(BaseDir, { TEXT("*.exe"), TEXT("*.dll") }, ManifestEntries);
	AppendDirectoryFileStamps(BinariesDir, { TEXT("*.exe"), TEXT("*.dll") }, ManifestEntries);
	AppendDirectoryFileStamps(PaksDir, { TEXT("*.pak"), TEXT("*.utoc"), TEXT("*.ucas") }, ManifestEntries);
	Context.ManifestRootHash = ComputeStableHash(ManifestEntries);

	TArray<FString> ModuleEntries;
	AppendDirectoryFileStamps(BaseDir, { TEXT("*.dll"), TEXT("*.exe") }, ModuleEntries);
	AppendDirectoryFileStamps(BinariesDir, { TEXT("*.dll"), TEXT("*.exe") }, ModuleEntries);
	Context.ModuleListHash = ComputeStableHash(ModuleEntries);

	TArray<FString> MountedEntries;
	AppendDirectoryFileStamps(PaksDir, { TEXT("*.pak"), TEXT("*.utoc"), TEXT("*.ucas") }, MountedEntries);
	AppendDirectoryFileStamps(PluginsDir, { TEXT("*.uplugin"), TEXT("*.pak"), TEXT("*.utoc"), TEXT("*.ucas") }, MountedEntries);
	for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPlugins())
	{
		MountedEntries.Add(FString::Printf(
			TEXT("plugin|%s|%s|%s"),
			*Plugin->GetName(),
			*NormalizeForHash(Plugin->GetBaseDir()),
			Plugin->IsEnabled() ? TEXT("enabled") : TEXT("disabled")));
	}
	Context.MountedContentHash = ComputeStableHash(MountedEntries);

	T66AppendSuspiciousPathReasons(
		{
			BaseDir,
			ProjectDir,
			FPaths::ConvertRelativePathToFull(FPaths::Combine(BaseDir, TEXT(".."))),
			FPaths::ConvertRelativePathToFull(FPaths::Combine(BaseDir, TEXT(".."), TEXT("..")))
		},
		Context);

	Context.MarkPristineIfUnset();
}

FString UT66RunIntegritySubsystem::ComputeStableHash(const TArray<FString>& Parts)
{
	TArray<FString> Sorted = Parts;
	Sorted.Sort();
	const FString Joined = FString::Join(Sorted, TEXT("\n"));
	return FMD5::HashAnsiString(*Joined);
}

FString UT66RunIntegritySubsystem::ComputeCombinedHash(
	const FString& SteamAppId,
	int32 SteamBuildId,
	const FString& SteamBetaName,
	const FString& ManifestId,
	const FString& ManifestRootHash,
	const FString& ModuleListHash,
	const FString& MountedContentHash)
{
	return ComputeStableHash(
		{
			SteamAppId,
			FString::FromInt(SteamBuildId),
			SteamBetaName,
			ManifestId,
			ManifestRootHash,
			ModuleListHash,
			MountedContentHash,
		});
}

void UT66RunIntegritySubsystem::AppendDirectoryFileStamps(const FString& Directory, const TArray<FString>& Patterns, TArray<FString>& OutEntries)
{
	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.DirectoryExists(*Directory))
	{
		return;
	}

	for (const FString& Pattern : Patterns)
	{
		TArray<FString> Files;
		FileManager.FindFilesRecursive(Files, *Directory, *Pattern, true, false, true);
		for (const FString& FilePath : Files)
		{
			AppendFileStamp(FilePath, OutEntries);
		}
	}
}

void UT66RunIntegritySubsystem::AppendFileStamp(const FString& FilePath, TArray<FString>& OutEntries)
{
	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.FileExists(*FilePath))
	{
		return;
	}

	const int64 Size = FileManager.FileSize(*FilePath);
	const FDateTime Timestamp = FileManager.GetTimeStamp(*FilePath);
	OutEntries.Add(FString::Printf(TEXT("%s|%lld|%lld"), *NormalizeForHash(FilePath), Size, Timestamp.ToUnixTimestamp()));
}

FString UT66RunIntegritySubsystem::NormalizeForHash(const FString& Path)
{
	FString Normalized = FPaths::ConvertRelativePathToFull(Path);
	FPaths::NormalizeFilename(Normalized);
	return Normalized.ToLower();
}

void UT66RunIntegritySubsystem::AddUniqueReason(FT66RunIntegrityContext& Context, const FString& Reason)
{
	Context.AddReason(Reason);
}
