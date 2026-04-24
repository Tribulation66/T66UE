using UnrealBuildTool;
using System.IO;

public class T66Mini : ModuleRules
{
	public T66Mini(ReadOnlyTargetRules Target) : base(Target)
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
			"T66"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"ApplicationCore",
			"EnhancedInput",
			"Niagara",
			"HTTP",
			"Json",
			"JsonUtilities",
			"ImageWrapper",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Steamworks"
		});

		void AddLooseMiniRuntimeDependency(string RelativeProjectPath)
		{
			RuntimeDependencies.Add("$(ProjectDir)/" + RelativeProjectPath.Replace('\\', '/'), StagedFileType.NonUFS);
		}

		AddLooseMiniRuntimeDependency("Content/Mini/Data/...");
		AddLooseMiniRuntimeDependency("SourceAssets/UI/SettingsReference/MiniScreens/...");
		AddLooseMiniRuntimeDependency("SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/stat_chip_clean_wide.png");
		AddLooseMiniRuntimeDependency("SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/title_plaque_heavy_wide.png");
		AddLooseMiniRuntimeDependency("SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/title_plaque_heavy_medium.png");
		AddLooseMiniRuntimeDependency("SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/title_plaque_heavy_compact.png");
		AddLooseMiniRuntimeDependency("SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/footer_bar_heavy.png");
		AddLooseMiniRuntimeDependency("SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/button_green_heavy.png");
		AddLooseMiniRuntimeDependency("SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/button_blue_heavy.png");
		AddLooseMiniRuntimeDependency("SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/button_purple_heavy.png");
		AddLooseMiniRuntimeDependency("SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/summary_row_heavy.png");
		AddLooseMiniRuntimeDependency("SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/metric_chip_heavy.png");
		AddLooseMiniRuntimeDependency("SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/slice_manifest_title_footer.json");
		AddLooseMiniRuntimeDependency("SourceAssets/Mini/...");
		AddLooseMiniRuntimeDependency("SourceAssets/ItemSprites/...");

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "UI"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "UI", "Screens"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "UI", "Components"));
	}
}
