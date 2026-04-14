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
			"EnhancedInput",
			"Niagara",
			"HTTP",
			"Json",
			"JsonUtilities",
			"ImageWrapper"
		});

		void AddLooseMiniRuntimeDependency(string RelativeProjectPath)
		{
			RuntimeDependencies.Add("$(ProjectDir)/" + RelativeProjectPath.Replace('\\', '/'), StagedFileType.NonUFS);
		}

		AddLooseMiniRuntimeDependency("Content/Mini/Data/...");
		AddLooseMiniRuntimeDependency("SourceAssets/Mini/...");
		AddLooseMiniRuntimeDependency("SourceAssets/ItemSprites/...");

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "UI"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "UI", "Screens"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "UI", "Components"));
	}
}
