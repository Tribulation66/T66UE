using UnrealBuildTool;
using System.IO;

public class T66Idle : ModuleRules
{
	public T66Idle(ReadOnlyTargetRules Target) : base(Target)
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
			"Json",
			"JsonUtilities"
		});

		void AddLooseIdleRuntimeDependency(string RelativeProjectPath)
		{
			RuntimeDependencies.Add("$(ProjectDir)/" + RelativeProjectPath.Replace('\\', '/'), StagedFileType.NonUFS);
		}

		AddLooseIdleRuntimeDependency("Content/Idle/...");
		AddLooseIdleRuntimeDependency("SourceAssets/Idle/...");
		AddLooseIdleRuntimeDependency("UI/screens/minigames/idle_chadpocalypse/reference/idle_chadpocalypse_main_menu_mockup_1920x1080_imagegen_20260503_v2.png");
		AddLooseIdleRuntimeDependency("UI/screens/minigames/idle_chadpocalypse/reference/idle_chadpocalypse_gameplay_mockup_1920x1080_imagegen_20260503_v2.png");

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "UI"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "UI", "Screens"));
	}
}
