// Copyright Tribulation 66. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class T66 : ModuleRules
{
	public T66(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"SlateCore",
			"UMG",
			"HTTP",
			"Json"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"Slate",
			"AssetRegistry",
			"Landscape",
			"Foliage",
			"ApplicationCore",
			"Niagara",
			"JsonUtilities",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Steamworks",
			"OnlineSubsystemSteam",
			"ImageWrapper",
			"MediaAssets",
			"Media",
			"ProceduralMeshComponent",
			"Projects"
		});

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			var WebView2Dir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "WebView2"));
			PublicIncludePaths.Add(Path.Combine(WebView2Dir, "include"));

			// Win32 window + COM dependencies for WebView2 hosting.
			PublicSystemLibraries.AddRange(new string[] { "user32.lib", "ole32.lib", "shlwapi.lib" });

			// Ship the loader DLL next to the executable (required for CreateCoreWebView2EnvironmentWithOptions).
			RuntimeDependencies.Add("$(TargetOutputDir)/WebView2Loader.dll", Path.Combine(WebView2Dir, "bin", "Win64", "WebView2Loader.dll"));
			RuntimeDependencies.Add("$(TargetOutputDir)/steam_appid.txt", Path.Combine(ModuleDirectory, "..", "..", "steam_appid.txt"), StagedFileType.NonUFS);

			PublicDefinitions.Add("T66_WITH_WEBVIEW2=1");
		}
		else
		{
			PublicDefinitions.Add("T66_WITH_WEBVIEW2=0");
		}

		void AddLooseRuntimeDependency(string RelativeProjectPath)
		{
			RuntimeDependencies.Add("$(ProjectDir)/" + RelativeProjectPath.Replace('\\', '/'), StagedFileType.NonUFS);
		}

		AddLooseRuntimeDependency("RuntimeDependencies/T66/Fonts/...");
		AddLooseRuntimeDependency("RuntimeDependencies/T66/UI/...");
		AddLooseRuntimeDependency("RuntimeDependencies/T66/UI/PowerUp/Statues/forbidden_chad/...");
		AddLooseRuntimeDependency("SourceAssets/OuterWallTexture.png");
		AddLooseRuntimeDependency("SourceAssets/Arcade/...");
		AddLooseRuntimeDependency("SourceAssets/UI/MasterLibrary/...");
		AddLooseRuntimeDependency("SourceAssets/UI/RunFlowReference/...");
		AddLooseRuntimeDependency("SourceAssets/UI/RunFlowReference/Backgrounds/...");
		AddLooseRuntimeDependency("SourceAssets/UI/SettingsReference/...");
		AddLooseRuntimeDependency("SourceAssets/UI/Worker2Reference/...");

		// Add all subdirectories as include paths
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Core"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Data"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "UI"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "UI", "Screens"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "UI", "Components"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Gameplay"));
	}
}
